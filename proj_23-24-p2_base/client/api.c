#include "api.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

//#include "common_op_code.h"

int current_session;
int pipe_write, pipe_read;

typedef struct{
  bool active;
  char op_code;
  char req_path[40];  
  char resp_path[40]; 
  char buffer[512];
  int session_id;
  size_t len;
  unsigned int event_id;
  size_t num_rows; 
  size_t num_cols;
  size_t num_seats; 
  size_t* xs; 
  size_t* ys;
} Client;

typedef struct{
  char buffer[512];
  size_t response_len;
}info;

Client client;  // apenas um cliente por agora

int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  int req_pipe = mkfifo(req_pipe_path, 0777);
  int resp_pipe = mkfifo(resp_pipe_path, 0777);
  //TODO: create pipes and connect to the server

  if (req_pipe == -1||resp_pipe == -1)  
    return 1;

  if ((pipe_write = open(server_pipe_path,O_WRONLY)) == -1){
    return 1;
  }

  client.op_code = '1';
  strcpy(client.req_path, req_pipe_path);
  strcpy(client.resp_path, resp_pipe_path);

  ssize_t write_len1 = write(pipe_write, &client,sizeof(client));
  if (write_len1 == -1){
    return 1;
  }

  if ((pipe_read = open(resp_pipe_path,O_RDONLY)) == -1){
    return 1;
  } 

  ssize_t read_len1 = read(pipe_read,&current_session,sizeof(int));
  if(read_len1 == -1){
    return 1;
  }
  printf("Session: %d\n", current_session); 
  return 0;
}

int ems_quit(void) { 
  //TODO: close pipes
  client.op_code = '2';
  client.session_id = current_session;
  printf("Processing \n");
  ssize_t write_len2 = write(pipe_write,&client,sizeof(client));
  if (write_len2 == -1){
    return 1;
  }

  if(close(pipe_write) == -1){
    printf("Error\n");
    return 1;
  }

  int response;
  ssize_t read_len2 = read(pipe_read, &response, sizeof(int));
  printf("response: %d \n", response);

  if(close(pipe_read) == -1 || response == -1 || read_len2 == -1){
    printf("Another error\n");
    return 1;
  }
  return 0;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  //TODO: send create request to the server (through the request pipe) and wait for the response (through the response pipe)
  client.op_code = '3';
  client.session_id = current_session;
  client.event_id = event_id;
  client.num_rows = num_rows;
  client.num_cols = num_cols;

  ssize_t write_len3 = write(pipe_write,&client,sizeof(client));   // escrever no pipe
  if (write_len3 == -1){
    return 1;
  }
  return 1;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  //TODO: send reserve request to the server (through the request pipe) and wait for the response (through the response pipe)
  client.op_code = '4';
  client.session_id = current_session;
  client.event_id = event_id;
  client.num_seats = num_seats;
  client.xs = xs;
  client.ys = ys;

  ssize_t write_len4 = write(pipe_write,&client,sizeof(client)); 
  if (write_len4 == -1){
    return 1;
  }

  return 1;
}

int ems_show(int out_fd, unsigned int event_id) {
  //TODO: send show request to the server (through the request pipe) and wait for the response (through the response pipe)
  client.op_code = '5';
  client.session_id = current_session;
  client.event_id = event_id;

  ssize_t write_len5 = write(pipe_write,&client,sizeof(client));  
  if (write_len5 == -1){
    return 1;
  }

  //write(out_fd, encontrar o formato da primeira entrega));
  return 1;
}

int ems_list_events(int out_fd) {
  //TODO: send list request to the server (through the request pipe) and wait for the response (through the response pipe)
  client.op_code = '6';
  client.session_id = current_session;

  ssize_t write_len6 = write(pipe_write,&client,sizeof(client)); 
  if (write_len6 == -1){
    return 1;
  }
  //write(out_fd, encontrar o formato da primeira entrega));
  return 1;
}
