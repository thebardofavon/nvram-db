#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "../include/ram_bptree.h"
#include "../include/free_space.h"
#include "../include/wal.h"
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Client handling function
void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg);

    Table *current_table = NULL;
    int current_txn_id = -1;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        int n = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0)
        {
            break; // Connection closed or error
        }
        buffer[n] = '\0';

        char *newline = strchr(buffer, '\n');
        if (newline)
        {
            *newline = '\0';
            char command[32];
            sscanf(buffer, "%s", command);

            if (strcmp(command, "CREATE") == 0 && strstr(buffer, "TABLE"))
            {
                char table_name[64];
                sscanf(buffer, "CREATE TABLE %s", table_name);
                int table_id = db_create_table(table_name);
                if (table_id >= 0)
                {
                    send(client_socket, "Table created\n", 14, 0);
                }
                else
                {
                    send(client_socket, "Failed to create table\n", 23, 0);
                }
            }
            else if (strcmp(command, "USE") == 0 && strstr(buffer, "TABLE"))
            {
                char table_name[64];
                sscanf(buffer, "USE TABLE %s", table_name);
                current_table = db_open_table(table_name);
                if (current_table)
                {
                    send(client_socket, "Table opened\n", 13, 0);
                }
                else
                {
                    send(client_socket, "Table not found\n", 16, 0);
                }
            }
            else if (strcmp(command, "BEGIN") == 0 && strstr(buffer, "TRANSACTION"))
            {
                current_txn_id = db_begin_transaction();
                if (current_txn_id >= 0)
                {
                    send(client_socket, "Transaction started\n", 20, 0);
                }
                else
                {
                    send(client_socket, "Failed to start transaction\n", 28, 0);
                }
            }
            else if (strcmp(command, "COMMIT") == 0)
            {
                if (current_txn_id >= 0)
                {
                    if (db_commit_transaction(current_txn_id))
                    {
                        send(client_socket, "Transaction committed\n", 22, 0);
                        current_txn_id = -1;
                    }
                    else
                    {
                        send(client_socket, "Failed to commit transaction\n", 29, 0);
                    }
                }
                else
                {
                    send(client_socket, "No active transaction\n", 22, 0);
                }
            }
            else if (strcmp(command, "ABORT") == 0)
            {
                if (current_txn_id >= 0)
                {
                    if (db_abort_transaction(current_txn_id))
                    {
                        send(client_socket, "Transaction aborted\n", 20, 0);
                        current_txn_id = -1;
                    }
                    else
                    {
                        send(client_socket, "Failed to abort transaction\n", 28, 0);
                    }
                }
                else
                {
                    send(client_socket, "No active transaction\n", 22, 0);
                }
            }
            else if (strcmp(command, "INSERT") == 0 && strstr(buffer, "ROW"))
            {
                if (!current_table)
                {
                    send(client_socket, "No table selected\n", 18, 0);
                    continue;
                }
                if (current_txn_id < 0)
                {
                    send(client_socket, "No active transaction\n", 22, 0);
                    continue;
                }
                int key;
                char data[256];
                char *ptr = strstr(buffer, "ROW") + 3;
                while (*ptr == ' ')
                    ptr++;
                key = atoi(ptr);
                while (*ptr != ' ' && *ptr != '\0')
                    ptr++;
                while (*ptr == ' ')
                    ptr++;
                if (*ptr == '\'')
                {
                    ptr++;
                    char *data_start = ptr;
                    while (*ptr != '\'' && *ptr != '\0')
                        ptr++;
                    if (*ptr == '\'')
                    {
                        size_t data_len = ptr - data_start;
                        strncpy(data, data_start, data_len);
                        data[data_len] = '\0';
                        int status = db_put_row(current_table, current_txn_id, key, data, strlen(data) + 1);
                        if (status == 0)
                        {
                            send(client_socket, "Row inserted\n", 13, 0);
                        }
                        else if (status == 1)
                        {
                            send(client_socket, "Row already exists\n", 19, 0);
                        }
                        else
                        {
                            send(client_socket, "Failed to insert row\n", 21, 0);
                        }
                    }
                    else
                    {
                        send(client_socket, "Invalid format\n", 15, 0);
                    }
                }
                else
                {
                    send(client_socket, "Invalid format\n", 15, 0);
                }
            }
            else if (strcmp(command, "GET") == 0 && strstr(buffer, "ROW"))
            {
                if (!current_table)
                {
                    send(client_socket, "No table selected\n", 18, 0);
                    continue;
                }
                if (current_txn_id < 0)
                {
                    send(client_socket, "No active transaction\n", 22, 0);
                    continue;
                }
                int key;
                sscanf(buffer, "GET ROW %d", &key);
                size_t size;
                void *data = db_get_row(current_table, current_txn_id, key, &size);
                if (data)
                {
                    char response[256];
                    snprintf(response, sizeof(response), "Row %d: %s\n", key, (char *)data);
                    send(client_socket, response, strlen(response), 0);
                }
                else
                {
                    send(client_socket, "Row not found\n", 14, 0);
                }
            }
            else if (strcmp(command, "DELETE") == 0 && strstr(buffer, "ROW"))
            {
                if (!current_table)
                {
                    send(client_socket, "No table selected\n", 18, 0);
                    continue;
                }
                if (current_txn_id < 0)
                {
                    send(client_socket, "No active transaction\n", 22, 0);
                    continue;
                }
                int key;
                sscanf(buffer, "DELETE ROW %d", &key);
                if (db_delete_row(current_table, current_txn_id, key))
                {
                    send(client_socket, "Row deleted\n", 12, 0);
                }
                else
                {
                    send(client_socket, "Failed to delete row\n", 21, 0);
                }
            }
            else if (strcmp(command, "SHOW") == 0 && strstr(buffer, "WAL"))
            {
                wal_show_data();
                send(client_socket, "WAL data displayed in server console\n", 37, 0);
            }
            else if (strcmp(command, "EXIT") == 0)
            {
                send(client_socket, "Goodbye\n", 8, 0);
                break;
            }
            else
            {
                send(client_socket, "Invalid command\n", 16, 0);
            }
        }
    }
    if (current_txn_id >= 0)
    {
        db_abort_transaction(current_txn_id);
    }
    close(client_socket);
    return NULL;
}

void db_init_with_recovery() {
    // Initialize database structures
    db_init();
    
    // Recover WAL if needed
    wal_recover();
    
    printf("Database initialization with WAL recovery complete\n");
}


int main()
{
    db_init_with_recovery();

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0)
        {
            perror("accept");
            continue;
        }

        pthread_t thread;
        int *sock_ptr = malloc(sizeof(int));
        *sock_ptr = client_socket;
        if (pthread_create(&thread, NULL, handle_client, sock_ptr) != 0)
        {
            perror("pthread_create");
            close(client_socket);
            free(sock_ptr);
        }
        else
        {
            pthread_detach(thread);
        }
    }

    close(server_socket);
    db_shutdown();
    return 0;
}