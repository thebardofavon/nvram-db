# NVRAM Database with B+ Tree Indexing and WAL

A lightweight database system that stores data in Non-Volatile RAM (NVRAM) with in-memory B+ tree indexing and Write-Ahead Logging (WAL) for durability.

## Features

- **NVRAM Storage**: Persistent data storage in memory-mapped NVRAM
- **B+ Tree Indexing**: Fast key-value lookups using in-memory B+ tree
- **Write-Ahead Logging (WAL)**: Transaction safety with persistent operation logging
- **Memory Management**: Separate memory managers for RAM and NVRAM
- **Multiple Tables**: Support for multiple concurrent database tables
- **Simple CLI**: Interactive command-line interface for database operations

## Implementation Details

### Memory Architecture

The system uses a hybrid memory architecture:
- **Data Storage**: All user data and WAL entries are stored in NVRAM for persistence
- **Indexes**: B+ tree indexes are maintained in RAM for fast access
- **Memory Management**: Custom memory allocators for both RAM and NVRAM

### Write-Ahead Logging

The WAL implementation ensures data durability:
- Each modification operation (insert/delete) is logged before execution
- WAL entries are stored in NVRAM to survive system crashes
- Each table has its own WAL structure for operation tracking

## Building and Running

```bash
# Compile the code
make

# Run the database
./nvram_db
```

Note: The NVRAM path is configured in `free_space.h` and might need adjustment for your system.

## Usage

The database provides an interactive CLI with the following commands:

1. **Create Table**: Create a new database table
2. **Open Table**: Open an existing table for operations
3. **Insert Row**: Add a new row with key and data
4. **Get Row**: Retrieve a row by key
5. **Delete Row**: Remove a row by key
6. **Iterate Table**: List all rows in the current table
7. **Close Table**: Close the current table
8. **Memory Stats**: Display memory usage statistics
9. **Show WAL Data**: Display all logged operations
10. **Exit**: Shutdown the database and exit

## Project Structure

```
src/
├── db_main.c         # Main application with CLI
├── free_space.c      # NVRAM memory manager
├── free_space.h      # NVRAM memory manager interface
├── ram_bptree.c      # B+ tree implementation
├── ram_bptree.h      # B+ tree interface
├── ram_free_space.c  # RAM memory manager
├── ram_free_space.h  # RAM memory manager interface
├── wal.c             # Write-ahead logging implementation
└── wal.h             # WAL interface
```

## Dependencies

- A POSIX-compliant system (Linux/Unix)
- GCC or compatible C compiler
- Memory-mapped file for NVRAM simulation (2GB by default)

## Limitations (To Be Integrated)

- No concurrency control (single-user system)
- Simple key-value data model (integer keys only)
- No complex queries or joins
- B+ tree does not support node splitting (limited number of entries)