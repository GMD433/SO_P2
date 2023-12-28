#include "api.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int current_session_id = 0;  //use this variable to generate unique session IDs
int freq, fresp, fserver, session_id;
int current_session;
int pipe_write, pipe_read;

typedef struct{
  bool active;
  char op_code;
  char[40] pipe_name;
  char[512] buffer;
  int session_id;
  size_t len;
} client;

typedef struct{
  char[512] buffer;
  size_t response_len;
}info;

// criar estrutura em cada função
int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  int req_pipe = mkfifo(req_pipe_path, 0777;
  int resp_pipe = mkfifo(resp_pipe_path, 0777;
  //TODO: create pipes and connect to the server
  //int freq, fresp, fserver, session_id;  -> ´é para ser global ou local?
  if (req_pipe == -1||resp_pipe == -1)   // create the FIFO (named pipe)
    return 1;
  
  pipe_write = open(server_pipe_path,O_WRONLY);
  client new_client;
  new_client.op_code = '1';
  strcpy(new_client.pipe_name, req_pipe_path);
  ssize_t write_len = write(pipe_write, &new_client,sizeof(client));
  if (write_len == -1){
    return 1
  }
  pipe_read = open(resp_pipe_path,O_RONLY);
  ssize_t read_len = read(pipe_read,&current_session,sizeof(int));
  if(read_len == -1){
    return 1
  }
  printf("Session: %d\n", active_session); 
  return 0;
}

int ems_quit(void) { 
  //TODO: close pipes
  client client;
  client.op_code = '2';
  client.session_id = current_session;
  printf("Processing \n");
  ssize_t write_len =write(pipe_write,&client,sizeof(client));
  if (write_len == -1){
    return 1
  }
  if(close(pipe_write) == -1){
    printf("Error\n");
    return 1;
  }
  int response;
  ssize_t read_len = read(pipe_read, &response, sizeof(int));
  printf("response: %d \n", response);
  if(close(pipe_read) == -1 || response == -1 || read_len == -1){
    printf("Another error\n");
    return 1;
  }
  return 0;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  //TODO: send create request to the server (through the request pipe) and wait for the response (through the response pipe)

  return 1;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  //TODO: send reserve request to the server (through the request pipe) and wait for the response (through the response pipe)
  // read(req_pipe)
  //info do pipe, ler o pipe, return tamanho do pipe
  return 1;
}

int ems_show(int out_fd, unsigned int event_id) {
  //TODO: send show request to the server (through the request pipe) and wait for the response (through the response pipe)
  //write();  ver se é assim
  return 1;
}

int ems_list_events(int out_fd) {
  //TODO: send list request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}
