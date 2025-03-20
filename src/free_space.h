#ifndef FREE_SPACE_H
#define FREE_SPACE_H

#include <stddef.h>

#define FILEPATH "/home/anushka/Documents/GitHub/nvram-db/NVRAM/nvram.img"
#define FILESIZE (2L * 1024 * 1024 * 1024) // 2GB

// Initialize free space management system
void init_free_space();

// Allocate memory from NVRAM using first-fit
void *allocate_memory(size_t size);

// Free allocated memory and merge adjacent blocks
void free_memory(void *ptr, size_t size);

// Cleanup function to release resources
void cleanup_free_space();

#endif // FREE_SPACE_H
