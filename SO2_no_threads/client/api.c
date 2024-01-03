#include "api.h"
#include "common/constants.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int session;
int request_pipe;
int response_pipe;
int server_pipe;

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

int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  if (unlink(req_pipe_path) != 0 && errno != ENOENT) {
        printf("Unlinking pipe \n");
        return -1;
    }
  int pipe_in = mkfifo(req_pipe_path, 0777);
  if (pipe_in != 0){
      return 1;
  }
  server_pipe = open(server_pipe_path, O_WRONLY);

  info request;
  request.op_code = '1';
  strcpy(request.pipe_name, resp_pipe_path);
  ssize_t write_size = write(server_pipe, &request, sizeof(request));
  if (write_size == -1){ 
    return -1;
  }
  int pipe_out =mkfifo(resp_pipe_path,077);
  if(pipe_out != 0){
    return 1;
  }

  response_pipe = open(resp_pipe_path,O_RDONLY);
  ssize_t resp_size = read(response_pipe,&session,sizeof(session));
  if(resp_size == -1){
    return 1;
  }

  return 0;
}

int ems_quit(void) { 
  info request;
  request.op_code = '2';
  request.session_id = session;
  ssize_t write_len =write(server_pipe,&request,sizeof(request));
  if(write_len < 0){
    return 1;
  }
  if(close(server_pipe) < 0){
    return 1;
  }
  int response;
  ssize_t read_len = read(response_pipe,&response,sizeof(response));
  if(read_len < 0){
    return 1;
  }
  if(close(request_pipe) < 0 || close(response_pipe) < 0){
    return 1;
  }

  return 0;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  info request;
  request.op_code = '3';
  request.event_id = event_id;
  request.num_rows = num_rows;
  request.num_cols = num_cols;
  ssize_t write_len =write(server_pipe,&request,sizeof(request));
  if(write_len < 0){
    return 1;
  }
  int response;
  ssize_t read_len = read(response_pipe,&response,sizeof(response));
  if(read_len < 0){
    return 1;
  }

  return response;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  info request;
  request.op_code = '4';
  request.event_id = event_id;
  request.num_seats = num_seats;
  request.xs = xs;
  request.ys = ys;
  ssize_t write_len =write(server_pipe,&request,sizeof(request));
  if(write_len < 0){
    return 1;
  }
  int response;
  ssize_t read_len = read(response_pipe,&response,sizeof(response));
  if(read_len < 0){
    return 1;
  }

  return response;
}

int ems_show(int out_fd, unsigned int event_id) {
  //TODO: send show request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_list_events(int out_fd) {
  //TODO: send list request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}
