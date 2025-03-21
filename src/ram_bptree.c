#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "free_space.h"     // For NVRAM allocation
#include "ram_free_space.h"  // For RAM allocation
#include "ram_bptree.h"
#include "wal.h"           // Include WAL header

// Maximum number of tables
#define MAX_TABLES 10
#define MAX_TABLE_NAME 64

// B+ Tree node structure (in RAM)
struct BPTreeNode {
    bool is_leaf;                  // Is this a leaf node?
    int num_keys;                  // Number of keys currently stored
    int keys[BP_ORDER - 1];        // Array of keys (row IDs)
    
    union {
        BPTreeNode *children[BP_ORDER];  // Internal node: pointers to children
        struct {
            NVRAMPtr data_ptrs[BP_ORDER - 1];  // Leaf node: pointers to data in NVRAM
            size_t data_sizes[BP_ORDER - 1];   // Size of each data item
        };
    };
    
    BPTreeNode *next_leaf;         // Pointer to next leaf (for range queries)
};

// B+ Tree structure (in RAM)
struct BPTree {
    BPTreeNode *root;              // Root node of the tree
    int height;                    // Height of the tree
    int node_count;                // Number of nodes
    int record_count;              // Number of records
};

// Table structure (in RAM)
struct Table {
    char name[MAX_TABLE_NAME];     // Table name
    int table_id;                  // Unique ID
    BPTree *index;                 // B+ Tree index
    bool is_open;                  // Is table open
};

// Global state
static Table *tables[MAX_TABLES] = {NULL};
static int next_table_id = 0;
static bool is_initialized = false;

// Helper function to allocate a new node in RAM
static BPTreeNode* create_node(bool is_leaf) {
    BPTreeNode *node = (BPTreeNode*)ram_allocate_memory(sizeof(BPTreeNode));
    if (!node) return NULL;
    
    // Initialize node
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    node->next_leaf = NULL;
    
    // Clear memory
    memset(node->keys, 0, sizeof(node->keys));
    
    if (is_leaf) {
        // Clear data pointers and sizes
        memset(node->data_ptrs, 0, sizeof(node->data_ptrs));
        memset(node->data_sizes, 0, sizeof(node->data_sizes));
    } else {
        // Clear children pointers
        memset(node->children, 0, sizeof(node->children));
    }
    
    return node;
}

// Helper function to create a new B+ Tree
static BPTree* create_tree() {
    BPTree *tree = (BPTree*)ram_allocate_memory(sizeof(BPTree));
    if (!tree) return NULL;
    
    // Create root node (initially a leaf)
    tree->root = create_node(true);
    if (!tree->root) {
        ram_free_memory(tree, sizeof(BPTree));
        return NULL;
    }
    
    tree->height = 1;
    tree->node_count = 1;
    tree->record_count = 0;
    
    return tree;
}

// Find the leaf node where a key should be located
static BPTreeNode* find_leaf(BPTree *tree, int key) {
    if (!tree || !tree->root) return NULL;
    
    BPTreeNode *node = tree->root;
    while (!node->is_leaf) {
        int i;
        for (i = 0; i < node->num_keys; i++) {
            if (key < node->keys[i]) break;
        }
        node = node->children[i];
    }
    
    return node;
}

// Find position of key in leaf node. Returns index if found, -1 if not found
static int find_key_in_leaf(BPTreeNode *leaf, int key) {
    for (int i = 0; i < leaf->num_keys; i++) {
        if (leaf->keys[i] == key) {
            return i;
        }
    }
    return -1;  // Key not found
}

// Initialize database system
void db_init() {
    if (is_initialized) return;
    
    // Initialize NVRAM free space manager
    init_free_space();
    
    // Initialize RAM free space manager
    ram_init_free_space();
    
    // Initialize tables array
    for (int i = 0; i < MAX_TABLES; i++) {
        tables[i] = NULL;
    }
    
    is_initialized = true;
    printf("Database system initialized\n");
}



// Open an existing table
Table* db_open_table(const char *name) {
    if (!is_initialized) {
        printf("Error: Database not initialized\n");
        return NULL;
    }
    
    for (int i = 0; i < MAX_TABLES; i++) {
        if (tables[i] && strcmp(tables[i]->name, name) == 0) {
            tables[i]->is_open = true;
            return tables[i];
        }
    }
    
    printf("Error: Table '%s' not found\n", name);
    return NULL;
}

// Get a row by its key
NVRAMPtr db_get_row(Table *table, int key, size_t *size) {
    if (!table || !table->is_open) {
        printf("Error: Invalid or closed table\n");
        return NULL;
    }
    
    // Find leaf node containing key
    BPTreeNode *leaf = find_leaf(table->index, key);
    if (!leaf) return NULL;
    
    // Find key in leaf
    int pos = find_key_in_leaf(leaf, key);
    if (pos == -1) {
        // Key not found
        return NULL;
    }
    
    // Return data pointer and size
    if (size) *size = leaf->data_sizes[pos];
    return leaf->data_ptrs[pos];
}


// Get the next row for iteration
int db_get_next_row(Table *table, int current_key) {
    if (!table || !table->is_open) {
        printf("Error: Invalid or closed table\n");
        return -1;
    }
    
    // Special case: if current_key is -1, return the first key
    if (current_key == -1) {
        BPTreeNode *node = table->index->root;
        
        // Navigate to leftmost leaf
        while (!node->is_leaf) {
            node = node->children[0];
        }
        
        if (node->num_keys > 0) {
            return node->keys[0];
        } else {
            return -1;  // Empty tree
        }
    }
    
    // Find leaf containing current key
    BPTreeNode *leaf = find_leaf(table->index, current_key);
    if (!leaf) return -1;
    
    // Find position of current key
    int pos = find_key_in_leaf(leaf, current_key);
    if (pos == -1) {
        // Current key not found
        return -1;
    }
    
    // Check if there's a next key in the same leaf
    if (pos + 1 < leaf->num_keys) {
        return leaf->keys[pos + 1];
    }
    
    // Otherwise, move to next leaf
    if (leaf->next_leaf && leaf->next_leaf->num_keys > 0) {
        return leaf->next_leaf->keys[0];
    }
    
    // No more keys
    return -1;
}

// Close a table
void db_close_table(Table *table) {
    if (table) {
        table->is_open = false;
        printf("Table '%s' closed\n", table->name);
    }
}

// Helper function to free a B+ Tree node recursively
static void free_node(BPTreeNode *node) {
    if (!node) return;
    
    if (!node->is_leaf) {
        // Free children recursively
        for (int i = 0; i <= node->num_keys; i++) {
            free_node(node->children[i]);
        }
    }
    
    ram_free_memory(node, sizeof(BPTreeNode));
}

// Helper function to free a B+ Tree
static void free_tree(BPTree *tree) {
    if (tree) {
        free_node(tree->root);
        ram_free_memory(tree, sizeof(BPTree));
    }
}

// Shutdown database system
void db_shutdown() {

    if (!is_initialized) return;
    
    // Close and free all tables
    for (int i = 0; i < MAX_TABLES; i++) {
        if (tables[i]) {
            // Free NVRAM data for all records
            if (tables[i]->index) {
                // We would need to traverse all leaves and free NVRAM data
                // For brevity, this code is omitted
                free_tree(tables[i]->index);
            }
            ram_free_memory(tables[i], sizeof(Table));
            tables[i] = NULL;
        }
    }
    
    // Clean up NVRAM
    cleanup_free_space();
    
    // Clean up RAM
    ram_cleanup_free_space();
    
    is_initialized = false;
    printf("Database system shut down\n");
}

// Create a new table
int db_create_table(const char *name) {
    if (!is_initialized) {
        printf("Error: Database not initialized\n");
        return -1;
    }
    
    // Find a free slot in tables array
    int slot = -1;
    for (int i = 0; i < MAX_TABLES; i++) {
        if (tables[i] == NULL) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf("Error: Maximum number of tables reached\n");
        return -1;
    }
    
    // Check if table with same name already exists
    for (int i = 0; i < MAX_TABLES; i++) {
        if (tables[i] && strcmp(tables[i]->name, name) == 0) {
            printf("Error: Table '%s' already exists\n", name);
            return -1;
        }
    }
    
    // Create table structure
    Table *table = (Table*)ram_allocate_memory(sizeof(Table));
    if (!table) {
        printf("Error: Failed to allocate memory for table\n");
        return -1;
    }
    
    // Create B+ Tree index
    BPTree *tree = create_tree();
    if (!tree) {
        printf("Error: Failed to create index for table\n");
        ram_free_memory(table, sizeof(Table));
        return -1;
    }
    
    // Initialize table
    strncpy(table->name, name, MAX_TABLE_NAME - 1);
    table->name[MAX_TABLE_NAME - 1] = '\0';
    table->table_id = next_table_id++;
    table->index = tree;
    table->is_open = true;
    
    // Create WAL table in NVRAM
    void *wal_table_ptr = allocate_memory(sizeof(WALTable));
    if (!wal_table_ptr) {
        printf("Error: Failed to allocate NVRAM for WAL table\n");
        free_tree(tree);
        ram_free_memory(table, sizeof(Table));
        return -1;
    }
    
    // Initialize WAL table
    if (!wal_create_table(table->table_id, wal_table_ptr)) {
        printf("Error: Failed to create WAL table\n");
        free_memory(wal_table_ptr, sizeof(WALTable));
        free_tree(tree);
        ram_free_memory(table, sizeof(Table));
        return -1;
    }
    
    // Add to tables array
    tables[slot] = table;
    
    printf("Table '%s' created with ID %d\n", name, table->table_id);
    return table->table_id;
}

// Insert key-value pair (simplified, doesn't handle node splitting)
bool db_put_row(Table *table, int key, void *data, size_t size) {
    if (!table || !table->is_open) {
        printf("Error: Invalid or closed table\n");
        return false;
    }
    
    // Allocate space in NVRAM for data
    NVRAMPtr nvram_data = allocate_memory(size);
    if (!nvram_data) {
        printf("Error: Failed to allocate NVRAM space for data\n");
        return false;
    }
    
    // Copy data to NVRAM
    memcpy(nvram_data, data, size);
    
    // Add WAL entry for the insertion
    void *wal_entry_ptr = allocate_memory(sizeof(WALEntry));
    if (!wal_entry_ptr) {
        printf("Error: Failed to allocate NVRAM for WAL entry\n");
        free_memory(nvram_data, size);
        return false;
    }
    
    // Add entry to WAL (1 for insertion)
    if (!wal_add_entry(table->table_id, key, nvram_data, 1, wal_entry_ptr)) {
        printf("Error: Failed to add WAL entry\n");
        free_memory(wal_entry_ptr, sizeof(WALEntry));
        free_memory(nvram_data, size);
        return false;
    }
    
    // Find leaf node where key should be inserted
    BPTreeNode *leaf = find_leaf(table->index, key);
    if (!leaf) {
        printf("Error: Failed to find leaf node\n");
        free_memory(nvram_data, size);
        return false;
    }
    
    // Check if key already exists
    int pos = find_key_in_leaf(leaf, key);
    if (pos != -1) {
        // Update existing row
        // Free old data
        free_memory(leaf->data_ptrs[pos], leaf->data_sizes[pos]);
        
        // Update with new data
        leaf->data_ptrs[pos] = nvram_data;
        leaf->data_sizes[pos] = size;
        return true;
    }
    
    // Check if leaf is full
    if (leaf->num_keys >= BP_ORDER - 1) {
        printf("Error: Leaf node is full (splitting not implemented)\n");
        free_memory(nvram_data, size);
        return false;
    }
    
    // Find position to insert
    int i = leaf->num_keys - 1;
    while (i >= 0 && leaf->keys[i] > key) {
        leaf->keys[i + 1] = leaf->keys[i];
        leaf->data_ptrs[i + 1] = leaf->data_ptrs[i];
        leaf->data_sizes[i + 1] = leaf->data_sizes[i];
        i--;
    }
    
    // Insert key and data
    leaf->keys[i + 1] = key;
    leaf->data_ptrs[i + 1] = nvram_data;
    leaf->data_sizes[i + 1] = size;
    leaf->num_keys++;
    
    // Update record count
    table->index->record_count++;
    
    return true;
}

// Delete a row
bool db_delete_row(Table *table, int key) {
    if (!table || !table->is_open) {
        printf("Error: Invalid or closed table\n");
        return false;
    }
    
    // Find leaf node containing key
    BPTreeNode *leaf = find_leaf(table->index, key);
    if (!leaf) return false;
    
    // Find key in leaf
    int pos = find_key_in_leaf(leaf, key);
    if (pos == -1) {
        // Key not found
        return false;
    }
    
    // Add WAL entry for deletion before actually deleting data
    void *wal_entry_ptr = allocate_memory(sizeof(WALEntry));
    if (!wal_entry_ptr) {
        printf("Error: Failed to allocate NVRAM for WAL entry\n");
        return false;
    }
    
    // Add entry to WAL (0 for deletion)
    if (!wal_add_entry(table->table_id, key, NULL, 0, wal_entry_ptr)) {
        printf("Error: Failed to add WAL entry\n");
        free_memory(wal_entry_ptr, sizeof(WALEntry));
        return false;
    }
    
    // Free NVRAM data
    free_memory(leaf->data_ptrs[pos], leaf->data_sizes[pos]);
    
    // Remove key and shift others
    for (int i = pos; i < leaf->num_keys - 1; i++) {
        leaf->keys[i] = leaf->keys[i + 1];
        leaf->data_ptrs[i] = leaf->data_ptrs[i + 1];
        leaf->data_sizes[i] = leaf->data_sizes[i + 1];
    }
    leaf->num_keys--;
    
    // Update record count
    table->index->record_count--;
    
    return true;
}