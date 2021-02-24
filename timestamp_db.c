#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "timestamp_db.h"

int getItem(char *db_mem, size_t item_index, db_item_s *item);
int memGetLine(char *str, char **ret_line);

int openTimestampDB(db_handler_s *db_handler, char *db_name, db_access_mode_enum mode, int write_size, int debug) {
    if(db_name == NULL) {
        fprintf(stderr, "Error: null name assign to db\n");
        return ERR_INVALID_ARG;
    }

    size_t name_len = strlen(db_name);
    db_handler->name = (char*)malloc(sizeof(char) * ( name_len + 1));
    snprintf(db_handler->name, name_len + 1, "%s", db_name); // + 1 for \0

    snprintf(db_handler->db_log_path, sizeof(db_handler->db_log_path), "./database/%s.log", db_name);

    int size = 0;
    if(mode == READ) {
        FILE *fptr = fopen(db_handler->db_log_path, "r"); // use write for create new document not exist
        if(fptr == NULL) {
            fprintf(stderr, "ERROR: DB %s is not created yet\n", db_handler->db_log_path);
            return ERR_INVALID_ARG;
        }
        fseek(fptr, 0, SEEK_END);
        size = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);

        db_handler->db_mem_addr = (char*)malloc(sizeof(char) * size);
        if(db_handler->db_mem_addr == NULL ) {
            perror("Error");
            return ERR_MEM;
        }
        db_handler->write_pos = db_handler->db_mem_addr;

        char *db_buf = NULL;
        db_buf = (char*)malloc(sizeof(char) * size);
        if(db_buf == NULL) {
            return ERR_MEM;
        }
        memset(db_buf, 0, size);

        fread(db_buf, sizeof(char), size, fptr);
        snprintf(db_handler->db_mem_addr, size, "%s", db_buf);
        if(db_handler->debug) {
            fprintf(stderr, "After Loading DB:\n%s\n", db_handler->db_mem_addr);
        }

        free(db_buf);
        fclose(fptr);
    }

    else if(mode == WRITE) {
        size = write_size;
        db_handler->db_mem_addr = (char*)malloc(sizeof(char) * size);
        if(db_handler->db_mem_addr == NULL ) {
            perror("Error");
            return ERR_MEM;
        }
        db_handler->write_pos = db_handler->db_mem_addr;
    }

    else {
        return ERR_INVALID_ARG;
    }
    db_handler->mode = mode;

    db_handler->size = size;
    db_handler->capacity = size;
    db_handler->debug = debug;
    db_handler->status = 1;
    db_handler->item_count = 0;
    db_handler->write_size = 0;
    // db_handler->last_event_time = {0};

    pthread_mutex_init(&(db_handler->lock), NULL);

    if(db_handler->debug) {
        fprintf(stderr, "db_size: %d, db_capacity: %d, db_name: %s, db_mem: %p, write pos: %p, db_log_path: %s, item count: %ld, mode: %s, status: %s\n", \
        db_handler->size, db_handler->capacity, db_handler->name, db_handler->db_mem_addr, \
        db_handler->write_pos, db_handler->db_log_path, db_handler->item_count, \
        (db_handler->mode == READ) ? "read": "write", (db_handler->status == 1) ? "open" : "close");
    }

    return 0;
}

// if mode = write, need to flush out meoery content
int closeTimestampDB(db_handler_s *db_handler) {
    if(db_handler == NULL) {
        fprintf(stderr, "Error: close a null db handler\n");
        return ERR_INVALID_ARG;
    }

    if(db_handler->status != 1) {
        fprintf(stderr, "Error: invalid operation on db\n");
        return ERR_NOT_OPEN_DB;
    }

    if(db_handler->mode == WRITE) {
        FILE *fptr = fopen(db_handler->db_log_path, "a");
        fwrite(db_handler->db_mem_addr, sizeof(char), db_handler->write_size, fptr);
        fclose(fptr);
    }

    if(db_handler->db_mem_addr) {
        free(db_handler->db_mem_addr);
        db_handler->db_mem_addr = NULL;
    }
    db_handler->write_pos = NULL;

    if(db_handler->name) {
        free(db_handler->name);
        db_handler->name = NULL;
    }
    db_handler->status = 0;

    pthread_mutex_destroy(&(db_handler->lock));
}

// write out to log if full
int writeTimestampDB(db_handler_s *db_handler, db_item_s *item) {
    if(db_handler == NULL) {
        return ERR_MEM;
    }

    if(db_handler->status != 1) {
        return ERR_NOT_OPEN_DB;
    }

    if(db_handler->mode != WRITE) {
        fprintf(stderr, "Error: invalid write with read mode\n");
        return ERR_INVALID_OPERATION;
    } 

    char timestamp[MAXTIMEBYTES] = {0};
    snprintf(timestamp, sizeof(timestamp), "%ld", item->timestamp);
    int space_avail = db_handler->capacity - (strlen(item->data) + strlen(timestamp) + 1); // 1 bytes for \n

    if( space_avail < 1) {
        fprintf(stderr, "Error: Input Item size: %ld, DB remaining size: %d\n", \
            strlen(item->data) + strlen(timestamp), db_handler->capacity);
        return ERR_SPACE_NOT_ENOUGH;
    } // preserve 2 bytes for /0 \n
    
    pthread_mutex_lock(&db_handler->lock);
    snprintf(db_handler->write_pos, db_handler->capacity, "%s:%s\n", timestamp, item->data);
    db_handler->capacity = db_handler->capacity - strlen(db_handler->write_pos);
    db_handler->write_size += strlen(db_handler->write_pos);
    db_handler->write_pos += strlen(db_handler->write_pos);
    db_handler->item_count += 1;
    pthread_mutex_unlock(&db_handler->lock);

    if(db_handler->debug) {
       fprintf(stderr, "Input data %s with timestamp: %ld\n", item->data, item->timestamp);
       fprintf(stderr, "Inside DB:\n%s", db_handler->db_mem_addr);
       fprintf(stderr, "total write size: %d\n", db_handler->write_size);
       fprintf(stderr, "remaining size: %d\n\n", db_handler->capacity);
    }
    
    return 0;
}

// fill in mem with db log content
// int readTimestampDB(db_handler_s *db_handler, db_item_s *ret_data) {
//     int i = 0;
//     int item_count = 0;
//     db_item_s item;

//     if(db_handler == NULL) {
//         fprintf(stderr, "Error: assign null argument\n");
//         return ERR_INVALID_ARG;
//     }

//     if(db_handler->status != 1) {
//         return ERR_NOT_OPEN_DB;
//     }

//     while(i < db_handler->item_count) {
//         if( getItem(db_handler->db_mem_addr, i, &item) == ERR_NO_ITEM ) {
//             i++;
//             continue;
//         }

//         // free(item.data);
//         memset(&item, 0, sizeof(item));
//         i++;
//     }
//     return 0;
// }

int getItem(char *db_mem, size_t ret_item_index, db_item_s *item) {
    int item_index = 0;
    char *get_line = NULL;
    int ret = 0;

    while( (ret = memGetLine(db_mem, &get_line)) == 0) {
        if(item_index != ret_item_index) {
            item_index++;
            continue;
        }
        break;
    }

    if(ret == -2) {
        return ERR_NO_ITEM;
    }

    char *timestamp_ptr;
    char *data_ptr;
    char *mv_line = get_line;

    timestamp_ptr = strtok(mv_line, ":");
    sscanf(mv_line, "%ld", &item->timestamp);
    mv_line += strlen(mv_line) + 1; //jump thr /0

    data_ptr = strtok(mv_line, "\n");
    item->data = (char*)malloc(sizeof(char) * strlen(mv_line));
    if(item->data == NULL) {
        return ERR_MEM;
    }
    sscanf(mv_line, "%s", item->data);
    free(get_line);

    return 0;
}

// // array parsed would not be able to getline again
// // ret_line must be null
int memGetLine(char * const str, char **ret_line) {
    static char *pre_str = NULL;
    static char *start_ptr;
    char *end_ptr;

    if(str == NULL) {
        return -2;
    }

    if(*ret_line == NULL) {
        start_ptr = str;
    }

    if(pre_str != str) {
        start_ptr = str;
        pre_str = str;
    }

    if( (end_ptr = strchr(start_ptr, '\n')) == NULL) {
        pre_str = NULL;
        return -1;
    }

    size_t line_length = end_ptr - start_ptr + 1;
    if(*ret_line != NULL) {
        free(*ret_line);
        *ret_line = NULL;
    }
    *ret_line = (char*)malloc(sizeof(char) * line_length);

    memmove(*ret_line, start_ptr, line_length);
    start_ptr += line_length;

    return 0;
}