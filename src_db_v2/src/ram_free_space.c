// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// // Structure for free RAM block
// typedef struct RAMFreeBlock {
//     size_t size;
//     void *address;
//     struct RAMFreeBlock *next;
// } RAMFreeBlock;

// // Global variables
// static RAMFreeBlock *ram_free_list = NULL;
// static size_t ram_total_memory = 4;
// static size_t ram_used_memory = 0;
// static void *ram_pool = NULL;

// // Configuration
// #define RAM_POOL_SIZE (11 * 1024 * 1024)  // 10MB RAM pool

// // Initialize RAM free space list with a memory pool
// void ram_init_free_space() {
//     // Allocate memory pool
//     ram_pool = malloc(RAM_POOL_SIZE);
//     if (!ram_pool) {
//         fprintf(stderr, "Error: Failed to allocate RAM pool\n");
//         exit(1);
//     }
    
//     // Create initial free block covering the entire pool
//     ram_free_list = (RAMFreeBlock *)malloc(sizeof(RAMFreeBlock));
//     if (!ram_free_list) {
//         fprintf(stderr, "Error: Failed to allocate free list header\n");
//         free(ram_pool);
//         exit(1);
//     }
    
//     ram_free_list->size = RAM_POOL_SIZE;
//     ram_free_list->address = ram_pool;
//     ram_free_list->next = NULL;
    
//     ram_total_memory = RAM_POOL_SIZE;
//     ram_used_memory = 0;
    
//     printf("RAM free space manager initialized with %zu bytes\n", RAM_POOL_SIZE);
// }

// // Allocate memory using first-fit algorithm for RAM
// void *ram_allocate_memory(size_t size) {
//     if (size == 0) return NULL;
    
//     // Add overhead for size tracking
//     size_t alloc_size = size + sizeof(size_t);
    
//     // Align to 8-byte boundary for better performance
//     if (alloc_size % 8 != 0) {
//         alloc_size += 8 - (alloc_size % 8);
//     }
    
//     RAMFreeBlock *current = ram_free_list;
//     RAMFreeBlock *prev = NULL;
    
//     // First-fit algorithm
//     while (current) {
//         if (current->size >= alloc_size) {
//             // Found a suitable block
//             void *alloc_address = current->address;
            
//             // Store size at the beginning of allocated block
//             *(size_t*)alloc_address = size;
            
//             // Adjust free block or remove it
//             if (current->size == alloc_size) {
//                 // Exact fit - remove this block from free list
//                 if (prev) {
//                     prev->next = current->next;
//                 } else {
//                     ram_free_list = current->next;
//                 }
//                 free(current);
//             } else {
//                 // Split block - adjust this block
//                 current->address = (char*)current->address + alloc_size;
//                 current->size -= alloc_size;
//             }
            
//             ram_used_memory += alloc_size;
            
//             // Return pointer to usable memory (after size field)
//             return (char*)alloc_address + sizeof(size_t);
//         }
        
//         prev = current;
//         current = current->next;
//     }
    
//     // No suitable block found
//     fprintf(stderr, "Error: RAM allocation failed - out of memory\n");
//     return NULL;
// }

// // Free allocated RAM memory
// void ram_free_memory(void *ptr, size_t expected_size) {
//     if (!ptr) return;
    
//     // Get actual block address (before the user data)
//     void *block_ptr = (char*)ptr - sizeof(size_t);
    
//     // Get stored size
//     size_t actual_size = *(size_t*)block_ptr;
    
//     // Verify size if expected_size is provided
//     if (expected_size > 0 && actual_size != expected_size) {
//         fprintf(stderr, "Warning: Expected size (%zu) does not match actual size (%zu)\n", 
//                 expected_size, actual_size);
//     }
    
//     // Calculate total block size including overhead
//     size_t block_size = actual_size + sizeof(size_t);
    
//     // Align to 8-byte boundary
//     if (block_size % 8 != 0) {
//         block_size += 8 - (block_size % 8);
//     }
    
//     // Create new free block
//     RAMFreeBlock *new_block = (RAMFreeBlock*)malloc(sizeof(RAMFreeBlock));
//     if (!new_block) {
//         fprintf(stderr, "Error: Failed to allocate free block header\n");
//         return;
//     }
    
//     new_block->size = block_size;
//     new_block->address = block_ptr;
    
//     // Insert in address-ordered position
//     RAMFreeBlock *current = ram_free_list;
//     RAMFreeBlock *prev = NULL;
    
//     while (current && current->address < block_ptr) {
//         prev = current;
//         current = current->next;
//     }
    
//     // Insert new block
//     if (prev) {
//         prev->next = new_block;
//     } else {
//         ram_free_list = new_block;
//     }
//     new_block->next = current;
    
//     // Update memory usage stats
//     ram_used_memory -= block_size;
    
//     // Merge with adjacent blocks
    
//     // Check if we can merge with the previous block
//     if (prev && (char*)prev->address + prev->size == new_block->address) {
//         prev->size += new_block->size;
//         prev->next = new_block->next;
//         free(new_block);
//         new_block = prev;
//     }
    
//     // Check if we can merge with the next block
//     if (new_block->next && (char*)new_block->address + new_block->size == new_block->next->address) {
//         new_block->size += new_block->next->size;
//         RAMFreeBlock *temp = new_block->next;
//         new_block->next = temp->next;
//         free(temp);
//     }
// }

// // Get current RAM free space statistics
// void ram_get_stats(size_t *total_free, size_t *largest_block, int *free_blocks) {
//     size_t free_mem = 0;
//     size_t max_block = 0;
//     int block_count = 0;
    
//     RAMFreeBlock *current = ram_free_list;
//     while (current) {
//         free_mem += current->size;
//         if (current->size > max_block) {
//             max_block = current->size;
//         }
//         block_count++;
//         current = current->next;
//     }
    
//     if (total_free) *total_free = free_mem;
//     if (largest_block) *largest_block = max_block;
//     if (free_blocks) *free_blocks = block_count;
// }

// // Cleanup RAM resources
// void ram_cleanup_free_space() {
//     // Free all nodes in the free list
//     RAMFreeBlock *current = ram_free_list;
//     while (current) {
//         RAMFreeBlock *temp = current;
//         current = current->next;
//         free(temp);
//     }
    
//     // Free the memory pool
//     if (ram_pool) {
//         free(ram_pool);
//         ram_pool = NULL;
//     }
    
//     ram_free_list = NULL;
//     ram_total_memory = 0;
//     ram_used_memory = 0;
    
//     printf("RAM free space manager cleaned up\n");
// }