#define MAX_RESERVATION_SIZE 256
#define STATE_ACCESS_DELAY_US 500000  // 500ms
#define MAX_JOB_FILE_NAME_SIZE 256
#define MAX_SESSION_COUNT 8

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

///Op code bitmap.
enum {
    OP_CODE_SETUP = 1,
    OP_CODE_QUIT = 2,
    OP_CODE_CREATE = 3,
    OP_CODE_RESERVE = 4,
    OP_CODE_SHOW = 5,
    OP_CODE_LIST_EVENTS = 6,
};

///__attribute__((__packed__)) is used to avoid padding problems when reading and writing into the structs.
typedef struct __attribute__((__packed__)){
    char pipename[40];
}setup;

typedef struct __attribute__((__packed__)){
    int session_id;
}quit;

typedef struct __attribute__((__packed__)){
    int session_id;
    unsigned int event_id;
    size_t num_rows;
    size_t num_cols;
}create;

typedef struct __attribute__((__packed__)){
    int session_id;
    unsigned int event_id;
    size_t num_seats;
    size_t* xs;
    size_t* ys;
}reserve;

typedef struct __attribute__((__packed__)){
    int session_id;
    unsigned int event_id;
    size_t rows;
    size_t cols;
    unsigned int* seats;

}show;

typedef struct __attribute__((__packed__)){
    int session_id;
    size_t num_events;
    unsigned int* ids;

}list;
