#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ram_bptree.h"
#include "free_space.h"

// Main function to test the database
int main() {
    // Initialize the database system
    db_init();
    
    int choice, table_id;
    char table_name[64];
    Table *current_table = NULL;
    
    while (1) {
        printf("\n=== NVRAM Database with RAM B+ Tree ===\n");
        printf("1. Create Table\n");
        printf("2. Open Table\n");
        printf("3. Insert Row\n");
        printf("4. Get Row\n");
        printf("5. Delete Row\n");
        printf("6. Iterate Table\n");
        printf("7. Close Table\n");
        printf("8. Memory Stats\n");
        printf("9. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                printf("Enter table name: ");
                scanf("%s", table_name);
                table_id = db_create_table(table_name);
                if (table_id >= 0) {
                    printf("Table created with ID: %d\n", table_id);
                    current_table = db_open_table(table_name);
                }
                break;
            }
            
            case 2: {
                printf("Enter table name: ");
                scanf("%s", table_name);
                current_table = db_open_table(table_name);
                if (current_table) {
                    printf("Table '%s' opened successfully\n", table_name);
                }
                break;
            }
            
            case 3: {
                if (!current_table) {
                    printf("No table is currently open\n");
                    break;
                }
                
                int key;
                char data[256];
                printf("Enter row ID (integer key): ");
                scanf("%d", &key);
                printf("Enter data: ");
                scanf(" %[^\n]", data);
                
                if (db_put_row(current_table, key, data, strlen(data) + 1)) {
                    printf("Row inserted successfully\n");
                } else {
                    printf("Failed to insert row\n");
                }
                break;
            }
            
            case 4: {
                if (!current_table) {
                    printf("No table is currently open\n");
                    break;
                }
                
                int key;
                size_t size;
                printf("Enter row ID to retrieve: ");
                scanf("%d", &key);
                
                void *data = db_get_row(current_table, key, &size);
                if (data) {
                    printf("Row %d: %s\n", key, (char*)data);
                } else {
                    printf("Row %d not found\n", key);
                }
                break;
            }
            
            case 5: {
                if (!current_table) {
                    printf("No table is currently open\n");
                    break;
                }
                
                int key;
                printf("Enter row ID to delete: ");
                scanf("%d", &key);
                
                if (db_delete_row(current_table, key)) {
                    printf("Row %d deleted successfully\n", key);
                } else {
                    printf("Failed to delete row %d\n", key);
                }
                break;
            }
            
            case 6: {
                if (!current_table) {
                    printf("No table is currently open\n");
                    break;
                }
                
                printf("Table contents:\n");
                int key = -1;  // Start from the beginning
                int count = 0;
                
                while ((key = db_get_next_row(current_table, key)) != -1) {
                    size_t size;
                    void *data = db_get_row(current_table, key, &size);
                    if (data) {
                        printf("  Row %d: %s\n", key, (char*)data);
                        count++;
                    }
                }
                
                if (count == 0) {
                    printf("  [Table is empty]\n");
                }
                break;
            }
            
            case 7: {
                if (!current_table) {
                    printf("No table is currently open\n");
                    break;
                }
                
                db_close_table(current_table);
                current_table = NULL;
                break;
            }
            
            case 8: {
                // Display memory statistics
                size_t ram_total_free, ram_largest_block;
                int ram_free_blocks;
                ram_get_stats(&ram_total_free, &ram_largest_block, &ram_free_blocks);
                
                printf("\nRAM Memory Statistics:\n");
                printf("  Total Free Space: %zu bytes\n", ram_total_free);
                printf("  Largest Free Block: %zu bytes\n", ram_largest_block);
                printf("  Number of Free Blocks: %d\n", ram_free_blocks);
                printf("\n");
                break;
            }
                
            case 9: {
                // Shutdown database system
                db_shutdown();
                printf("Database system shutdown. Exiting...\n");
                return 0;
            }
            
            default:
                printf("Invalid choice, please try again\n");
        }
    }
    
    return 0;
}