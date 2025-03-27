// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <pthread.h>
// #include <unistd.h>
// #include "ram_bptree.h"
// #include "free_space.h"
// #include "wal.h"
// #include "lock_manager.h"

// #define NUM_THREADS 4
// #define NUM_OPERATIONS 5

// // Shared table for all threads
// Table *shared_table = NULL;
// pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// // Structure to pass data to thread
// typedef struct {
//     int thread_id;
//     int start_key;
// } ThreadData;

// // Thread function for concurrent database operations
// void* thread_worker(void* arg) {
//     ThreadData* data = (ThreadData*)arg;
//     int thread_id = data->thread_id;
//     int start_key = data->start_key;
    
//     // Begin a transaction
//     int txn_id = db_begin_transaction();
    
//     pthread_mutex_lock(&print_mutex);
//     printf("Thread %d: Started transaction %d\n", thread_id, txn_id);
//     pthread_mutex_unlock(&print_mutex);
    
//     if (txn_id < 0) {
//         pthread_mutex_lock(&print_mutex);
//         printf("Thread %d: Failed to start transaction\n", thread_id);
//         pthread_mutex_unlock(&print_mutex);
//         return NULL;
//     }
    
//     // Perform multiple operations in the transaction
//     for (int i = 0; i < NUM_OPERATIONS; i++) {
//         int key = start_key + i;
//         char data[64];
//         snprintf(data, sizeof(data), "Data from thread %d, operation %d", thread_id, i);
        
//         // Random delay to increase chance of lock conflicts
//         usleep(rand() % 100000);  // 0-100ms
        
//         // Insert row
//         pthread_mutex_lock(&print_mutex);
//         printf("Thread %d: Attempting to insert key %d\n", thread_id, key);
//         pthread_mutex_unlock(&print_mutex);
        
//         bool success = db_put_row(shared_table, txn_id, key, data, strlen(data) + 1);
        
//         pthread_mutex_lock(&print_mutex);
//         if (success) {
//             printf("Thread %d: Successfully inserted key %d\n", thread_id, key);
//         } else {
//             printf("Thread %d: Failed to insert key %d (likely locked by another thread)\n", thread_id, key);
//         }
//         pthread_mutex_unlock(&print_mutex);
        
//         // Try to read some data (potentially from another thread)
//         int read_key = (thread_id + 1) % NUM_THREADS * 100 + i;  // Try to read data from another thread
        
//         pthread_mutex_lock(&print_mutex);
//         printf("Thread %d: Attempting to read key %d\n", thread_id, read_key);
//         pthread_mutex_unlock(&print_mutex);
        
//         size_t size;
//         void* read_data = db_get_row(shared_table, txn_id, read_key, &size);
        
//         pthread_mutex_lock(&print_mutex);
//         if (read_data) {
//             printf("Thread %d: Read key %d: %s\n", thread_id, read_key, (char*)read_data);
//         } else {
//             printf("Thread %d: Key %d not found or locked\n", thread_id, read_key);
//         }
//         pthread_mutex_unlock(&print_mutex);
//     }
    
//     // Random chance of committing or aborting (80% commit, 20% abort)
//     bool should_commit = (rand() % 100) < 80;
    
//     if (should_commit) {
//         pthread_mutex_lock(&print_mutex);
//         printf("Thread %d: Committing transaction %d\n", thread_id, txn_id);
//         pthread_mutex_unlock(&print_mutex);
        
//         if (db_commit_transaction(txn_id)) {
//             pthread_mutex_lock(&print_mutex);
//             printf("Thread %d: Transaction %d committed successfully\n", thread_id, txn_id);
//             pthread_mutex_unlock(&print_mutex);
//         } else {
//             pthread_mutex_lock(&print_mutex);
//             printf("Thread %d: Failed to commit transaction %d\n", thread_id, txn_id);
//             pthread_mutex_unlock(&print_mutex);
//         }
//     } else {
//         pthread_mutex_lock(&print_mutex);
//         printf("Thread %d: Aborting transaction %d\n", thread_id, txn_id);
//         pthread_mutex_unlock(&print_mutex);
        
//         if (db_abort_transaction(txn_id)) {
//             pthread_mutex_lock(&print_mutex);
//             printf("Thread %d: Transaction %d aborted successfully\n", thread_id, txn_id);
//             pthread_mutex_unlock(&print_mutex);
//         } else {
//             pthread_mutex_lock(&print_mutex);
//             printf("Thread %d: Failed to abort transaction %d\n", thread_id, txn_id);
//             pthread_mutex_unlock(&print_mutex);
//         }
//     }
    
//     free(data);
//     return NULL;
// }

// // Function to demonstrate deadlock scenario
// void demonstrate_deadlock() {
//     printf("\n--- Demonstrating Deadlock Detection/Prevention ---\n");
    
//     // Create two threads that will attempt to acquire locks in opposite order
//     pthread_t thread1, thread2;
//     ThreadData *data1 = malloc(sizeof(ThreadData));
//     ThreadData *data2 = malloc(sizeof(ThreadData));
    
//     data1->thread_id = 100;
//     data1->start_key = 1000;
    
//     data2->thread_id = 200;
//     data2->start_key = 2000;
    
//     // Create threads
//     pthread_create(&thread1, NULL, thread_worker, data1);
//     pthread_create(&thread2, NULL, thread_worker, data2);
    
//     // Wait for threads to complete
//     pthread_join(thread1, NULL);
//     pthread_join(thread2, NULL);
    
//     printf("--- Deadlock Demonstration Complete ---\n\n");
// }

// // Main function
// int main() {
//     // Initialize the database
//     db_init();
    
//     // Create a test table
//     int table_id = db_create_table("concurrent_test");
//     if (table_id < 0) {
//         printf("Failed to create table\n");
//         return 1;
//     }
    
//     // Open the table
//     shared_table = db_open_table("concurrent_test");
//     if (!shared_table) {
//         printf("Failed to open table\n");
//         return 1;
//     }
    
//     printf("=== Starting Multithreaded Database Test ===\n");
    
//     // Create thread data
//     ThreadData thread_data[NUM_THREADS];
//     pthread_t threads[NUM_THREADS];
    
//     // Create and start threads
//     for (int i = 0; i < NUM_THREADS; i++) {
//         thread_data[i].thread_id = i;
//         thread_data[i].start_key = i * 100;  // Different key ranges for each thread
        
//         if (pthread_create(&threads[i], NULL, thread_worker, &thread_data[i]) != 0) {
//             perror("Failed to create thread");
//             return 1;
//         }
//     }
    
//     // Wait for all threads to complete
//     for (int i = 0; i < NUM_THREADS; i++) {
//         pthread_join(threads[i], NULL);
//     }
    
//     // Show the WAL data after concurrent operations
//     printf("\n=== Write-Ahead Log After Concurrent Operations ===\n");
//     wal_show_data();
    
//     // Demonstrate table contents
//     printf("\n=== Table Contents After Concurrent Operations ===\n");
//     int txn_id = db_begin_transaction();
//     int key = -1;
//     while ((key = db_get_next_row(shared_table, key)) != -1) {
//         size_t size;
//         void *data = db_get_row(shared_table, txn_id, key, &size);
//         if (data) {
//             printf("Row %d: %s\n", key, (char*)data);
//         }
//     }
//     db_commit_transaction(txn_id);
    
//     // Optionally demonstrate deadlock scenario
//     demonstrate_deadlock();
    
//     // Close the table and shutdown
//     db_close_table(shared_table);
//     db_shutdown();
    
//     printf("\n=== Multithreaded Database Test Complete ===\n");
//     return 0;
// }

// // OLD_MAKE_FILE

// # # CC = gcc
// # # CFLAGS = -Wall -Wextra -g -I./src
// # # TARGET = nvram_db

// # # # Source files
// # # SRC = src/db_main.c src/free_space.c src/ram_free_space.c src/ram_bptree.c src/wal.c src/lock_manager.c

// # # # Object files
// # # OBJ = $(SRC:.c=.o)

// # # # Default target
// # # all: $(TARGET)

// # # # Link object files to create executable
// # # $(TARGET): $(OBJ)
// # # 	$(CC) $(CFLAGS) -o $@ $^

// # # # Compile source files to object files
// # # %.o: %.c
// # # 	$(CC) $(CFLAGS) -c $< -o $@

// # # # Clean up
// # # clean:
// # # 	rm -f $(OBJ) $(TARGET)

// # # # Run the database
// # # run: $(TARGET)
// # # 	./$(TARGET)

// # # .PHONY: all clean run

// # CC = gcc
// # CFLAGS = -Wall -Wextra -g -I./src -pthread
// # TARGET = nvram_db
// # THREAD_TEST = thread_test

// # # Source files
// # SRC = src/db_main.c src/free_space.c src/ram_free_space.c src/ram_bptree.c src/wal.c src/lock_manager.c
// # TEST_SRC = src/thread_test.c src/free_space.c src/ram_bptree.c src/wal.c src/lock_manager.c

// # # Object files
// # OBJ = $(SRC:.c=.o)
// # TEST_OBJ = $(TEST_SRC:.c=.o)

// # # Default target
// # all: $(TARGET) $(THREAD_TEST)

// # # Link object files to create executable
// # $(TARGET): $(OBJ)
// # 	$(CC) $(CFLAGS) -o $@ $^

// # # Build the thread test
// # $(THREAD_TEST): $(TEST_OBJ)
// # 	$(CC) $(CFLAGS) -o $@ $^

// # # Compile source files to object files
// # %.o: %.c
// # 	$(CC) $(CFLAGS) -c $< -o $@

// # # Clean up
// # clean:
// # 	rm -f $(OBJ) $(TEST_OBJ) $(TARGET) $(THREAD_TEST)

// # # Run the database
// # run: $(TARGET)
// # 	./$(TARGET)

// # # Run the thread test
// # run-test: $(THREAD_TEST)
// # 	./$(THREAD_TEST)

// # .PHONY: all clean run run-test

