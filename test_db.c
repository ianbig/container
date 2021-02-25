#define GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "timestamp_db.h"

// void* writer_1() {
//     const char test_path[] = "./test_case/test_name_1";
//     db_handler_s db_1;
//     openTimestampDB(&db_1, "test", WRITE, 1024, 1);

//     char *linptr = NULL;
//     size_t size = 0;
//     FILE *fptr;
//     db_item_s item;

//     fptr = fopen(test_path, "r");
//     if(fptr == NULL) {
//         fprintf(stderr, "Error: unable to open file\n");
//         return NULL;
//     }

//     sleep(1);
//     while(getline(&linptr, &size, fptr) != -1) {
//         struct timespec rtime;
//         if(clock_gettime(CLOCK_MONOTONIC, &rtime) != 0 ) {
//             fprintf(stderr, "Error: insert item get time error\n");
//             return NULL;
//         }
        
//         item.timestamp = rtime.tv_sec;
//         linptr[strlen(linptr)] = '\0';
//         memset(item.data, 0, sizeof(item.data));
//         snprintf(item.data, strlen(linptr), "%s", linptr);

        
//         sleep(1);
//         writeTimestampDB(&db_1, &item);
//     }
//     closeTimestampDB(&db_1);
//     free(linptr);

//     return NULL;
// }

// void* writer_2() {
//     const char test_path[] = "./test_case/test_name_2";
//     db_handler_s db_1;
//     openTimestampDB(&db_1, "test", WRITE, 1024, 1);

//     char *linptr = NULL;
//     size_t size = 0;
//     FILE *fptr;
//     db_item_s item;

//     fptr = fopen(test_path, "r");
//     if(fptr == NULL) {
//         fprintf(stderr, "Error: unable to open file\n");
//         return NULL;
//     }

//     sleep(1);
//     while(getline(&linptr, &size, fptr) != -1) {
//         struct timespec rtime;
//         if(clock_gettime(CLOCK_MONOTONIC, &rtime) != 0 ) {
//             fprintf(stderr, "Error: insert item get time error\n");
//             return NULL;
//         }
        
//         item.timestamp = rtime.tv_sec;
//         linptr[strlen(linptr)] = '\0';
//         memset(item.data, 0, sizeof(item.data));
//         snprintf(item.data, strlen(linptr), "%s", linptr);

        
//         sleep(1);
//         writeTimestampDB(&db_1, &item);
//     }
//     closeTimestampDB(&db_1);
//     free(linptr);

//     return NULL;
// }

int main(int argc, char const *argv[])
{   
    // basic operation
    db_handler_s db_1;

    if( openTimestampDB(&db_1, "test", WRITE, 1024, 1) != 0) {
        fprintf(stderr, "Error: Unknown error\n");
        return -1;
    }

    char *linptr = NULL;
    size_t size = 0;
    FILE *fptr;
    db_item_s item;

    fptr = fopen("./test_case/test_name_1", "r");
    if(fptr == NULL) {
        fprintf(stderr, "Error: unable to open file\n");
        return -1;
    }

    sleep(1);
    while(getline(&linptr, &size, fptr) != -1) {
        struct timespec rtime;
        if(clock_gettime(CLOCK_MONOTONIC, &rtime) != 0 ) {
            fprintf(stderr, "Error: insert item get time error\n");
            return -1;
        }
        
        item.timestamp = rtime.tv_sec;
        linptr[strlen(linptr)] = '\0';
        memset(item.data, 0, sizeof(item.data));
        snprintf(item.data, strlen(linptr), "%s", linptr);

        
        sleep(1);
        writeTimestampDB(&db_1, &item);
    }
    closeTimestampDB(&db_1);
    free(linptr);


    db_item_s ret;
    openTimestampDB(&db_1, "test", READ, 0, 0);

    int retval = 0;
    while(retval != DB_ITEM_READ_FINISH) {
        retval = readTimestampDB(&db_1, &ret);
        if( retval == 0 ) {
            fprintf(stderr, "timestamp: %ld %s\n", ret.timestamp, ret.data);
        }
        sleep(0.5);
    }

    closeTimestampDB(&db_1);
    return 0;
}
    // multithread
//     pthread_t pid_1, pid_2;
//     pthread_create(&pid_1, NULL, writer_1, NULL);
//     pthread_create(&pid_2, NULL, writer_2, NULL);

//     pthread_join(pid_1, NULL);
//     pthread_join(pid_2, NULL);
// }
