#include "api.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "common/constants.h"
#include "common/io.h"

int input_pipe;
int res_input_pipe;
int output_pipe;
int session_id;
static char pipename[PIPE_SIZE + 1] = {0};

//sends the job as a request
int send_job(int op,void *info,size_t len){
  size_t req_len;
  void* req;
  req_len = sizeof(char) + len;
  req = malloc(req_len);
  char op_code =(char)('0' + op);
  memcpy(req,&op_code,sizeof(op_code));
  memcpy(req +  sizeof(op_code),info,len);
  if(write(output_pipe,req,req_len) < 0){
    return 1;
  }
  return 0;
}

int show_fd(size_t rows,size_t cols,unsigned int* seats, int out_fd){
  for (size_t i = 1; i <= rows; i++) {
    for (size_t j = 1; j <= cols; j++) {
      char buffer[16];
      char seat = '0' + seats[j];
      sprintf(buffer, "%u",seat);

      if (print_str(out_fd, buffer)) {
        perror("Error writing to file descriptor");
        return 1;
      }

      if (j < cols) {
        if (print_str(out_fd, " ")) {
          perror("Error writing to file descriptor");
          return 1;
        }
      }
    }

    if (print_str(out_fd, "\n")) {
      perror("Error writing to file descriptor");
      return 1;
    }
  }
  return 0;
}

int list_fd(size_t n_ids,unsigned int* ids,int out_fd){
  int id_n = 0;
  while(n_ids != 0){
    char buff[] = "Event: ";
    if (print_str(out_fd, buff)) {
      perror("Error writing to file descriptor");
      return 1;
    }

    char id[16];
    sprintf(id, "%u\n",ids[id_n]);
    if (print_str(out_fd, id)) {
      perror("Error writing to file descriptor");
      return 1;
    }
    id_n++;
    n_ids--;
  }
  return 0;
}

int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  strncpy(pipename,req_pipe_path,PIPE_SIZE);
  unlink(pipename);

  if (mkfifo(pipename, 0777) == -1) {
    return 1;
  }

  output_pipe = open(server_pipe_path,O_WRONLY);
  if(output_pipe == -1){
    return 1;
  }

  setup req;

  memset(&req.pipename,'\0',sizeof(char)*PIPE_SIZE);
  memcpy(&req.pipename,req_pipe_path,strlen(req_pipe_path));

  send_job(OP_CODE_SETUP,&req,sizeof(setup));

  input_pipe = open(req_pipe_path,O_WRONLY);
  if(input_pipe == -1){
    close(output_pipe);
    unlink(pipename);
    return 1;
  }

  write(input_pipe,&req,sizeof(req));
  if(session_id == -1){
    close(input_pipe);
    close(output_pipe);
    unlink(pipename);
    return 1;
  }


  res_input_pipe = open(resp_pipe_path,O_RDONLY);
  if(res_input_pipe == -1){
    close(input_pipe);
    close(output_pipe);
    unlink(pipename);
    return 1;
  }

  read(res_input_pipe,&session_id,sizeof(session_id));

  return 0;
}

int ems_quit(void) {
  int response = 0;

  void* req = malloc(sizeof(char) + sizeof(quit));

  char op_code = '0' + OP_CODE_QUIT;
  
  memcpy(req,&op_code,sizeof(char));

  quit request;

  request.session_id = session_id;
  memcpy(req +  sizeof(char),&request,sizeof(quit));

  if(write(output_pipe,req,sizeof(char) + sizeof(quit)) == -1){
    return 1;
  }

  read(res_input_pipe,&response,sizeof(response));
  if (close(output_pipe) == -1){ 
      return 1;
  }
  if (close(input_pipe) == -1){
    return 1;
  }
  if (close(res_input_pipe) == -1){
    return 1;
  }
  if (unlink(pipename) == -1){
    return 1;
  }


  return response;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  int response;

  create request;
  request.session_id = session_id;
  request.event_id = event_id;
  request.num_rows = num_rows;
  request.num_cols = num_cols;


  send_job(OP_CODE_CREATE,&request,sizeof(create));


  read(res_input_pipe,&response,sizeof(response));


  return response;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  int response;

  reserve request;
  request.session_id = session_id;
  request.event_id = event_id;
  request.num_seats = num_seats;
  request.xs = xs;
  request.ys = ys;

  send_job(OP_CODE_RESERVE,&request,sizeof(reserve));

  read(res_input_pipe,&response,sizeof(response));

  return response;
}

int ems_show(int out_fd, unsigned int event_id) {
  int response;
  size_t rows;
  size_t cols;
  unsigned int* seats;

  show request;
  request.session_id = session_id;
  request.event_id = event_id;

  send_job(OP_CODE_SHOW,&request,sizeof(show));

  read(res_input_pipe,&response,sizeof(response));
  read(res_input_pipe,&rows,sizeof(rows));
  read(res_input_pipe,&cols,sizeof(cols));
  read(res_input_pipe,&seats,sizeof(seats));

  show_fd(rows,cols,seats,out_fd);
  return 0;
}

int ems_list_events(int out_fd) {
  int response;
  size_t n_ids;
  unsigned int* ids;

  list request;
  request.session_id = session_id;

  send_job(OP_CODE_SHOW,&request,sizeof(show));

  read(res_input_pipe,&response,sizeof(response));
  read(res_input_pipe,&n_ids,sizeof(n_ids));
  read(res_input_pipe,&ids,sizeof(ids));

  list_fd(n_ids,ids,out_fd);
  return 0;
}
