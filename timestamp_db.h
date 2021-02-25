#ifndef TIMESTAMP_DB_H
#define TIMESTAMP_DB_H

#include <time.h>
#include <stdio.h>
#include <pthread.h>

#define ERR_NOT_OPEN_DB -1
#define ERR_INVALID_DATA_TYPE -2
#define ERR_MEM -3
#define ERR_INVALID_ARG -4
#define ERR_SPACE_NOT_ENOUGH -5
#define ERR_NO_ITEM -6
#define ERR_INVALID_OPERATION -7
#define DB_ITEM_TIME_NOT_REACH 1
#define DB_ITEM_READ_FINISH 2

#define MAXTIMEBYTES 50
#define MAXFILENAME 100
#define MAXLINE 1024
#define MAXDATASIZE 1024

typedef enum {
	READ,
	WRITE
} db_access_mode_enum;

typedef struct {
	char db_log_path[MAXFILENAME];
	char *db_mem_addr;
	unsigned int capacity;
	unsigned int size;
	int debug;
	int status;
	size_t item_count;
	char *name;
	char *write_pos;
	unsigned int write_size;
	pthread_mutex_t lock;
	size_t next_read_item_index;
	time_t last_item_timestamp_db;
	time_t last_item_timestamp_read;
	db_access_mode_enum mode;
	// for mem getline
	char *pre_str;
	char *start_ptr;
} db_handler_s;

typedef struct {
	time_t timestamp;
	char data[MAXDATASIZE];
} db_item_s;

// ret normal: 0, ERR_INVALID_ARG, ERR_MEM
int openTimestampDB(db_handler_s *db_handler, char *db_name, db_access_mode_enum mode, int write_size, int debug);
// potential bug: db_handler not init
int writeTimestampDB(db_handler_s *db_handler, db_item_s *data);
int readTimestampDB(db_handler_s *db_handler, db_item_s ret_data[]);
int closeTimestampDB(db_handler_s *db_handler);

#endif
