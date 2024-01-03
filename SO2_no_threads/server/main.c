#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common/constants.h"
#include "common/io.h"
#include "operations.h"

typedef struct{
  char op_code;
  char pipe_name[PIPE_SIZE];
  char buffer[1024];
  unsigned int event_id;
  size_t num_rows;
  size_t num_cols;
  size_t num_seats;
  size_t* xs;
  size_t* ys;
  int out_fd;
  int session_id;
}info;

char pipes[SESSION][PIPE_SIZE];
int pipe[SESSION];

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

  char *pipename = argv[1];

  if(unlink(pipename) != 0 && errno != ENOENT){
    return 1;
  }

  if(mkfifo(pipename,0777) != 0){
    return 1;
  }

  int server_pipe = open(pipename,O_RDONLY);
  if(server_pipe < 0){
    return 1;
  }

  int links = 0;
  info old_req; 
  
  while (1) {
    info req;
    req = old_req;
    ssize_t read_len = read(server_pipe,&req,sizeof(info));
    if(old_req.op_code != req.op_code){
      if(req.op_code == '1'){
        strcpy(pipes[links],req.pipe_name);
        pipe[links] = open(pipes[links],O_WRONLY);
        int response = links;
        ssize_t write_len = write(pipe[links],&response,sizeof(response));
        if(write_len < 0){
          return 1;
        }
        links++;
      }
      else if(req.op_code == '2'){
        int response = close(pipe[req.session_id]);
        ssize_t write_len = write(pipe[req.session_id],&answer,sizeof(answer));
        if(write_len < 0){
          return 1;
        }
      }
      else if(req.op_code == '3'){
        int response = ems_create(req.event_id,req.num_rows,req.num_cols);
        ssize_t write_len = write(pipe[req.session_id],&response,sizeof(response));
        if(write_len < 0){
          return 1;
        }
      }
      else if(req.op_code == '4'){
        int response = ems_reserve(req.event_id,req.num_seats,req.xs,req.ys);
        ssize_t write_len = write(pipe[req.session_id],&response,sizeof(response));
        if(write_len < 0){
          return 1;
        } 
      }
    }
  }

  //TODO: Close Server

  ems_terminate();
}