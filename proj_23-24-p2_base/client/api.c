#include "api.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

int current_session;
int server_pipe, req_pipe, resp_pipe;

typedef struct{
  char req_path[40];  
  char resp_path[40]; 
  int session_id;
} Client;

Client client;  // apenas um cliente por agora

// fills the rest of a buffer with \0
void fill_buffer(char* buffer){
  for(int i = 0; i < 512; i++){
    if(buffer[i] == '\0'){
      for(int j = i; j < 512; j++){
        buffer[j] = '\0';
      }
      break;
    }
  }
}

int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  req_pipe = mkfifo(req_pipe_path, 0777);
  resp_pipe = mkfifo(resp_pipe_path, 0777);
  char buffer[512];
  //TODO: create pipes and connect to the server

  if (req_pipe == -1||resp_pipe == -1)  
    return 1;

  if ((server_pipe = open(server_pipe_path,O_WRONLY)) == -1)
    return 1;

  strcpy(client.req_path, req_pipe_path);
  strcpy(client.resp_path, resp_pipe_path);

  buffer[0] = '1';  
  strcat(buffer, client.req_path);
  strcat(buffer, client.resp_path);
  fill_buffer(buffer);

  printf("%s\n", buffer); // debug

  if((write(server_pipe, buffer, 512))== -1)  // escrever no pipe do server
    return 1;

  if((req_pipe = open(req_pipe_path, O_WRONLY)) == -1)
    return 1;    

  if(write(req_pipe, buffer, 512) == -1)      // escrever no request_pipe do cliente
    return 1;

  if ((resp_pipe = open(resp_pipe_path,O_RDONLY)) == -1)
    return 1;

  if((read(resp_pipe,&current_session,sizeof(int)))== -1)
    return 1;

  client.session_id = current_session;
  
  return 0;
}

int ems_quit(void) { 
  //TODO: close pipes
  char buffer[512];
  buffer[0] = '2';
  fill_buffer(buffer);

  if((write(req_pipe, buffer, 512)) == -1)   
    return 1;

  //close pipes
  if((close(server_pipe) == -1) || (close(req_pipe) == -1) || (close(resp_pipe) == -1))
    return 1;

  return 0;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  //TODO: send create request to the server (through the request pipe) and wait for the response (through the response pipe)
  char buffer[512];
  buffer[0] = '3';
  char event_id_str[128], num_rows_str[128], num_cols_str[128];
  int response;

  sprintf(event_id_str, "%d", event_id);
  sprintf(num_rows_str, "%ld", num_rows);
  sprintf(num_cols_str, "%ld", num_cols);

  strcat(buffer, event_id_str);
  strcat(buffer, num_rows_str);
  strcat(buffer, num_cols_str);
  
  fill_buffer(buffer);

  if(write(req_pipe, buffer, 512) == -1)      // escrever no request_pipe do cliente
    return 1;
  
  if(read(resp_pipe, &response, sizeof(int)) == -1)     // le a resposta do server
    return 1;

  return 0;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  //TODO: send reserve request to the server (through the request pipe) and wait for the response (through the response pipe)
  char buffer[512], event_id_str[128], num_seats_str[128], xs_str[128], ys_str[128];
  buffer[0] = '4';
  int response;
  sprintf(event_id_str, "%d", event_id);
  sprintf(num_seats_str, "%ld", num_seats);
  sprintf(xs_str, "%ln", xs);
  sprintf(ys_str, "%ln", ys);

  strcat(buffer, event_id_str);
  strcat(buffer, num_seats_str);
  strcat(buffer, xs_str);
  strcat(buffer, ys_str);

  fill_buffer(buffer);

  if(write(req_pipe, buffer, 512) == -1)      
    return 1;

  if(read(resp_pipe, &response, sizeof(int)) == -1)     
    return 1;

  return 1;
}

int ems_show(int out_fd, unsigned int event_id) {
  //TODO: send show request to the server (through the request pipe) and wait for the response (through the response pipe)
  char buffer[512], event_id_str[128];
  buffer[0] = '5';
  char response[512];

  sprintf(event_id_str, "%d", event_id);
  strcat(buffer, event_id_str);
  fill_buffer(buffer);

  if(write(req_pipe, buffer, 512) == -1)   
    return 1;

  ssize_t numRead;  // Para armazenar o número de bytes lidos
  ssize_t numWritten;  // Para armazenar o número de bytes escritos

  numRead = read(resp_pipe, &response, sizeof(response));
  if (numRead == -1 || numRead == 0) {
    return 1;
  } 

  if (response[0] != 0){  // se retorno diferente de 0, houve erro
    return 1;
  } else {
      numWritten = write(out_fd, &response, (size_t)numRead);
      if (numWritten == -1) {
          return 1;
      }
  return 1;
  }
}

int ems_list_events(int out_fd) {
  //TODO: send list request to the server (through the request pipe) and wait for the response (through the response pipe)
  char buffer[1];
  buffer[0] = '6';
  char response[512];

  if(write(req_pipe, buffer, 1) == -1)   
    return 1;

  ssize_t numRead;  
  ssize_t numWritten; 

  numRead = read(resp_pipe, &response, sizeof(response));
  if (numRead == -1 || numRead == 0) {
      return 1;
  }

  if (response[0] != 0){ 
    return 1;
  } else {
      numWritten = write(out_fd, &response, (size_t)numRead);
      if (numWritten == -1) {
          return 1;
      }
  }
  return 1;
}
