#define GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "timestamp_db.h"

#define MAXRET 10

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
        FILE *fptr = fopen(db_handler->db_log_path, "w"); // use write for create new document not exist
        if(fptr == NULL) {
            fprintf(stderr, "ERROR: %s not exist\n", db_handler->db_log_path);
            return ERR_INVALID_ARG;
        }
        fseek(fptr, SEEK_END, 0);
        size = ftell(fptr);
        fclose(fptr);
    }

    else if(mode == WRITE) {
        size = write_size;
    }

    else {
        return ERR_INVALID_ARG;
    }
    

    db_handler->db_mem_addr = (char*)malloc(sizeof(char) * size);
    if(db_handler->db_mem_addr == NULL ) {
        perror("Error");
        return ERR_MEM;
    }
    db_handler->write_pos = db_handler->db_mem_addr;
    

    db_handler->size = size;
    db_handler->capacity = size;
    db_handler->debug = debug;
    db_handler->status = 1;
    db_handler->item_count = 0;
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



int main(int argc, char const *argv[])
{   
    // basic operation
    db_handler_s db_1;
    // db_item_s ret[MAXRET];

    if( openTimestampDB(&db_1, "test", READ, 1024, 1) != 0) {
        fprintf(stderr, "Error: Unknown error\n");
        return -1;
    }

    // char *linptr = NULL;
    // size_t size = 0;
    // FILE *fptr;
    // db_item_s item;

    // fptr = fopen("./test_case/test_name_1", "r");
    // if(fptr == NULL) {
    //     fprintf(stderr, "Error: unable to open file\n");
    //     return -1;
    // }

    // time_t start = time(NULL);
    // sleep(1);
    // while(getline(&linptr, &size, fptr) != -1) {
    //     item.data = (char*)malloc(sizeof(char) * (strlen(linptr)));
    //     if(item.data == NULL) {
    //         return -1;
    //     }
    //     linptr[strlen(linptr)] = '\0';
    //     snprintf(item.data, strlen(linptr), "%s", linptr);
    //     item.timestamp = time(NULL);
    //     sleep(1);
    //     writeTimestampDB(&db_1, &item);
    //     free(item.data);
    // }
    // free(linptr);

    // time_t end = time(NULL);

    // fprintf(stderr, "start: %ld, end: %ld\n", start, end);

    // readTimestampDB(&db_1, start, end, ret);

    // closeTimestampDB(&db_1);
    // for(int i = 0; i < MAXRET; i++) {
    //     if(ret[i].data != NULL) {
    //         free(ret[i].data);
    //     }
    // }
}
