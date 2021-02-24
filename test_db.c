#define GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "timestamp_db.h"

#define MAXRET 10

int main(int argc, char const *argv[])
{   
    // basic operation
    db_handler_s db_1;
    // db_item_s ret[MAXRET];

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
        item.data = (char*)malloc(sizeof(char) * (strlen(linptr)));
        if(item.data == NULL) {
            return -1;
        }
        linptr[strlen(linptr)] = '\0';
        snprintf(item.data, strlen(linptr), "%s", linptr);
        if(clock_gettime(CLOCK_MONOTONIC, &rtime) != 0 ) {
            fprintf(stderr, "Error: insert item get time error\n");
            return -1;
        }
        item.timestamp = rtime.tv_sec;
        sleep(1);
        writeTimestampDB(&db_1, &item);
        free(item.data);
    }
    closeTimestampDB(&db_1);
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
