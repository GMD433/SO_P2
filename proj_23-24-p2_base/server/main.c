#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common/constants.h"
#include "common/io.h"
#include "operations.h"

typedef struct{
  char req_path[40];  
  char resp_path[40]; 
  int session_id;
} Client;


int req_pipe, resp_pipe, server_pipe;

int main(int argc, char* argv[]) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s\n <pipe_path> [delay]\n", argv[0]);
    return 1;
  }

  char* endptr;
  unsigned int state_access_delay_us = STATE_ACCESS_DELAY_US;
  if (argc == 3) {
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_us = (unsigned int)delay;
  }

  if (ems_init(state_access_delay_us)) {
    fprintf(stderr, "Failed to initialize EMS\n");
    return 1;
  }

  char* server_pipename = argv[1];
  //TODO: Intialize server, create worker threads
  if (unlink(server_pipename) != 0 /*&& errno != ENOENT*/) {
      printf("Unlinking pipe \n");
      return -1;
    }

  if (mkfifo(server_pipename, 0777) != 0) {
    printf("Error creating pipe \n");
    return -1;
  }

  server_pipe = open(server_pipename, O_RDONLY);
  if (server_pipe == -1) {
    printf("Error opening pipe\n");
    return -1;
  }

  Client previous_client;
  while (1) {
    //TODO: Read from pipe
    //TODO: Write new client to the producer-consumer buffer
    //Client newClient;
    char buffer[512];
    if (read(server_pipe, buffer, 512) == -1) {
      printf("Error reading from pipe\n");
      return -1;
    }
    ssize_t op_code = buffer[0];
    switch (op_code){
      case '1':
        char req_pipename[41], resp_pipename[41];
        strncpy(req_pipename, buffer + 1, 40);
        strncpy(resp_pipename, buffer + 41, 40);
        ems_setup(req_pipename, resp_pipename, server_pipename);
        break;

      case '2':
        ems_quit();
        break;

      case '3':
        char event_id = buffer[1], num_rows = buffer[2], num_cols = buffer[3];
        ems_create(event_id, num_rows, num_cols);
        break;

      case '4':
        char event_id = buffer[1], num_seats = buffer[2], xs = buffer[3], ys = buffer[4];
        ems_reserve(event_id, num_seats, xs, ys);
        break;

      case '5':
        char event_id = buffer[1];
        ems_show(out_fd, event_id);
        break;
        
      case '6':
        ems_list_events(out_fd);
        break;
      }
  }
  //TODO: Close Server
  ems_terminate();
}