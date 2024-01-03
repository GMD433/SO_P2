#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "common/constants.h"
#include "common/io.h"
#include "operations.h"

static char *pipename;
int input_pipe;
session sessions[MAX_SESSION_COUNT];
int free_sessions[MAX_SESSION_COUNT] = {0};
pthread_mutex_t server_thread_lock;

int server_setup(){
  int session_id;
  char request[40] = {'\0'};
  char response[40] = {'\0'};

  if(read(input_pipe,&request,sizeof(request)) == -1){
    fprintf(stderr,"Error on setting up the server");
    return 1;
  }
  session_id = -1;
  
  for (int i = 0; i < MAX_SESSION_COUNT; ++i){
    if (free_sessions[i] == 0){
      sessions[i].request_path_name = request;
      sessions[i].response_path_name = response;
      session_id = i;
      free_sessions[i] = 1;
      printf("\t mounted session %d\n", session_id);
      break;
      }
  }

  int output_pipe = open(response,O_WRONLY);
  if(output_pipe == -1){
    return 1;
  }
  if(session_id != -1){
    sessions[session_id].res_input_pipe = output_pipe;
  }
  if(write(output_pipe,&session_id,sizeof(session_id)) == -1){
    return 1;
  }
  return 0;
}

int server_quit(int session_id){
  int closer;

  if((close(sessions[session_id].input_pipe) == -1) || (close(sessions[session_id].res_input_pipe) == -1)){
    closer = -1;
    if(write(sessions[session_id].res_input_pipe,&closer,sizeof(closer)) == -1){
      return 1;
    }
    return 1;
  }
  free_sessions[session_id] = 0;

  free(sessions[session_id].buffer);
  sessions[session_id].buffer = NULL;
  return 0;
}

int server_create(int session_id){
  create request;
  memcpy(&request, sessions[session_id].buffer,sizeof(create));
  int buf = ems_create(request.event_id,request.num_rows,request.num_cols);
  if(write(sessions[request.session_id].res_input_pipe,&buf,sizeof(buf)) == -1){
    return 1;
  }
  free(sessions[session_id].buffer);
  sessions[session_id].buffer = NULL;
  return 0;
}

int server_reserve(int session_id){
  reserve request;
  memcpy(&request, sessions[session_id].buffer,sizeof(reserve));
  int buf = ems_reserve(request.event_id,*request.num_seats,request.xs,request.ys);
  if(write(sessions[request.session_id].res_input_pipe,&buf,sizeof(buf)) == -1){
    return 1;
  }
  free(sessions[session_id].buffer);
  sessions[session_id].buffer = NULL;
  return 0;
}

void* thread_func(void *session_temp){
  int session_id = *(int*)session_temp;
  while(1){
    if (pthread_mutex_lock(&sessions[session_id].lock) != 0){
      return;
    }
    if (pthread_mutex_unlock(&server_thread_lock) != 0){
      return;
    }
    if (pthread_cond_wait(&sessions[session_id].cond, &sessions[session_id].lock) != 0){
      return;
    }
    if (pthread_mutex_unlock(&sessions[session_id].lock) != 0){
      return;
    }

    char op_code = sessions[session_id].op_code;

    if(op_code == '2'){
      server_quit(session_id);
    }
    else if(op_code == '3'){
      server_create(session_id);
    }
    else if(op_code == '4'){
      server_reserve(session_id);
    }
  }
}

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

  pipename = argv[1];

  printf("Starting server with pipe called %s\n", pipename);

  if (mkfifo(pipename, 0777) == -1) {
    return 1;
  }


  if (pthread_mutex_init(&server_thread_lock, NULL) != 0){
    return 1;
  }
  for (int i = 0; i < MAX_SESSION_COUNT; i++){
    if (pthread_mutex_lock(&server_thread_lock) != 0){
      return 1;
    }
    if (pthread_cond_init(&sessions[i].cond, NULL) != 0){
      return 1;
    }
    if (pthread_mutex_init(&sessions[i].lock, NULL) != 0){
      return 1;
    }
    if (pthread_create(&sessions[i].thread, NULL, &thread_func, (void*)&i) != 0){
      return 1;
    }
  }

  input_pipe = open(pipename, O_RDONLY);
  if (input_pipe == -1) {
      unlink(pipename);
      return 1;
    }
  

  ssize_t rb;
  while (1) {
    char op_code;
    rb = read(input_pipe,&op_code,sizeof(op_code));

    if(rb > 0){
      if(op_code == '1'){
        server_setup();
      }
      else if(op_code == '2'){
        quit request;

        if(read(input_pipe,&request,sizeof(quit)) == -1){
          fprintf(stderr,"Reading request error");
        }
      sessions[request.session_id].op_code = '2';

      if (pthread_cond_signal(&sessions[request.session_id].cond) != 0){
        return 1;
        }
      }
      else if(op_code == '3'){
        create request;

        if(read(input_pipe,&request,sizeof(create)) == -1){
          return 1;
        }
        sessions[request.session_id].buffer = malloc(sizeof(create));
        memcpy(sessions[request.session_id].buffer,&request,sizeof(create));
        sessions[request.session_id].op_code = '3';

        if (pthread_cond_signal(&sessions[request.session_id].cond) != 0){
          return 1;
          }

      }
      else if(op_code == '4'){
        reserve request;

        if(read(input_pipe,&request,sizeof(reserve)) == -1){
          return 1;
        }
        sessions[request.session_id].buffer = malloc(sizeof(reserve));
        memcpy(sessions[request.session_id].buffer,&request,sizeof(reserve));
        sessions[request.session_id].op_code = '4';

        if (pthread_cond_signal(&sessions[request.session_id].cond) != 0){
          return 1;
          }

      }
    }
  }

  //TODO: Close Server

  ems_terminate();
  return 0;
}