#include "api.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int current_session_id = 0;  //use this variable to generate unique session IDs
int freq, fresp, fserver, session_id;

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
  //TODO: create pipes and connect to the server
  //int freq, fresp, fserver, session_id;  -> ´é para ser global ou local?
  if ((mkfifo(req_pipe_path, 0777) == -1)||(mkfifo(resp_pipe_path, 0777) == -1))   // create the FIFO (named pipe)
    return 1;

  if (((freq = open(req_pipe_path, O_RDONLY)) < 0)||((fresp = open(resp_pipe_path, O_WRONLY)) < 0))  // open the FIFO
    return 1;

  if ((fserver = open(server_pipe_path, O_RDONLY)) < 0)
    return 1;
    
  return 0;
}

int ems_quit(void) { 
  //TODO: close pipes
  if ((close(freq) == -1)||(close(fresp) == -1))
    return 1;
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
