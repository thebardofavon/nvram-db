#ifndef RAM_BPTREE_H
#define RAM_BPTREE_H

#include <stddef.h>
#include <stdbool.h>

// Define the order of the B+ Tree (maximum number of children)
#define BP_ORDER 5

// Pointer to data in NVRAM
typedef void* NVRAMPtr;

// Forward declarations
typedef struct BPTreeNode BPTreeNode;
typedef struct BPTree BPTree;
typedef struct Table Table;

// Standard database operations
// All structures except the actual data are in RAM

// Create a new table
int db_create_table(const char *name);

// Open an existing table
Table* db_open_table(const char *name);

// Get a row by its key
NVRAMPtr db_get_row(Table *table, int key, size_t *size);

// Insert or update a row
bool db_put_row(Table *table, int key, void *data, size_t size);

// Delete a row
bool db_delete_row(Table *table, int key);

// Get the next row for iteration
int db_get_next_row(Table *table, int current_key);

// Close a table
void db_close_table(Table *table);

// Initialize database system
void db_init();

// Shutdown database system
void db_shutdown();

#endif // RAM_BPTREE_H