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
int session_count = 0;
int input_pipe;
session sessions[MAX_SESSION_COUNT];
pthread_mutex_t server_thread_lock;
void* error = NULL;


int session_close(int session_id){//thread function to close a session
  printf("Session closing for client %d",session_id);
  int closer;

  if((close(sessions[session_id].input_pipe) == -1) || (close(sessions[session_id].res_input_pipe) == -1)){
    closer = -1;
    if(write(sessions[session_id].res_input_pipe,&closer,sizeof(closer)) == -1){
      return 1;
    }
    return 1;
  }

  free(sessions[session_id].buffer);
  sessions[session_id].buffer = NULL;
  return 0;
}

int event_create(int session_id){//thread function to create an event
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

int event_reserve(int session_id){//thread function to reserve an event
  reserve request;
  memcpy(&request, sessions[session_id].buffer,sizeof(reserve));
  int buf = ems_reserve(request.event_id,request.num_seats,request.xs,request.ys);
  if(write(sessions[request.session_id].res_input_pipe,&buf,sizeof(buf)) == -1){
    return 1;
  }
  free(sessions[session_id].buffer);
  sessions[session_id].buffer = NULL;
  return 0;
}

int event_show(int session_id){//thread function to send the show information to the client
  show request;
  memcpy(&request, sessions[session_id].buffer,sizeof(show));
  size_t buf = get_rows(request.event_id);
  if(write(sessions[request.session_id].res_input_pipe,&buf,sizeof(buf)) == -1){
    return 1;
  }
  size_t buf1 = get_cols(request.event_id);
  if(write(sessions[request.session_id].res_input_pipe,&buf1,sizeof(buf1)) == -1){
    return 1;
  }
  unsigned int* buf2 = get_seats(request.event_id);
  if(write(sessions[request.session_id].res_input_pipe,&buf2,sizeof(buf2)) == -1){
    return 1;
  }
  free(sessions[session_id].buffer);
  sessions[session_id].buffer = NULL;
  return 0;
}

int event_list(int session_id){//thread function to send the list event information to the client
  list request;
  memcpy(&request, sessions[session_id].buffer,sizeof(show));
  size_t buf = get_n_events();
  if(write(sessions[request.session_id].res_input_pipe,&buf,sizeof(buf)) == -1){
    return 1;
  }
  unsigned int* buf1 = get_ids();
  if(write(sessions[request.session_id].res_input_pipe,&buf1,sizeof(buf1)) == -1){
    return 1;
  }
  free(sessions[session_id].buffer);
  sessions[session_id].buffer = NULL;
  return 0;
}

void* handle_client(void *session_temp){//main thread function that will handle client threading
  int session_id = *(int*)session_temp;
  //pthread_detach(pthread_self());
  while(1){
    if (pthread_mutex_lock(&sessions[session_id].lock) != 0){
      return error;
    }
    if (pthread_mutex_unlock(&server_thread_lock) != 0){
      return error;
    }
    if (pthread_cond_wait(&sessions[session_id].cond, &sessions[session_id].lock) != 0){
      return error;
    }
    if (pthread_mutex_unlock(&sessions[session_id].lock) != 0){
      return error;
    }

    char op_code = sessions[session_id].op_code;

    if(op_code == '2'){
      session_close(session_id);
      session_count--;
      pthread_exit(NULL);
      printf("Session closed for client %d",session_id);
    }
    else if(op_code == '3'){
      event_create(session_id);
      printf("Event created for client %d",session_id);
    }
    else if(op_code == '4'){
      event_reserve(session_id);
      printf("Event reserved for client %d",session_id);
    }
    else if(op_code == '5'){
      event_show(session_id);
      printf("Event information to be shown sent for client %d",session_id);
    }
    else if(op_code == '6'){
      event_list(session_id);
      printf("Event information to be listed sent for client %d",session_id);
    }
  }
}

int session_setup(){ //function to setup the session
  int session_id;
  char request[40] = {'\0'};
  char response[40] = {'\0'};

  if(read(input_pipe,&request,sizeof(request)) == -1){
    fprintf(stderr,"Error on setting up the server");
    return 1;
  }
  if(session_count <= MAX_SESSION_COUNT){
    if (pthread_mutex_lock(&server_thread_lock) != 0){
      return 1;
    }
    if (pthread_cond_init(&sessions[session_count].cond, NULL) != 0){
      return 1;
    }
    if (pthread_mutex_init(&sessions[session_count].lock, NULL) != 0){
      return 1;
    }
    if (pthread_create(&sessions[session_count].thread, NULL, &handle_client, (void*)&session_count) != 0){
      return 1;
    }
    sessions[session_count].request_path_name = request;
    sessions[session_count].response_path_name = response;
    session_id = session_count;
    printf("Started session with id %d\n", session_id);
    session_count++;
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

  printf("Started server with pipe called %s\n", pipename);

  if (mkfifo(pipename, 0777) == -1) {
    printf("#Unable to create pipe");
    return 1;
  }


  if (pthread_mutex_init(&server_thread_lock, NULL) != 0){
    return 1;
  }

  input_pipe = open(pipename, O_RDONLY);
  if (input_pipe == -1) {
      unlink(pipename); 
      printf("#Unable to open pipe");
      return 1;
    }
  

  ssize_t rb;
  while (1) {
    char op_code;
    rb = read(input_pipe,&op_code,sizeof(op_code));

    if(rb > 0){
      if(op_code == '1'){
      printf("#Client %d requested op 1\n",session_count);
        session_setup(); //we don't use threads when setting up a session
      }
      else if(op_code == '2'){
        printf("#Client %d requested op 2\n",session_count);
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
        printf("#Client %d requested op 3\n",request.session_id);

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
        printf("#Client %d requested op 4\n",request.session_id);

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

      else if(op_code == '5'){
        show request;
        printf("#Client %d requested op 5\n",request.session_id);
        if(read(input_pipe,&request,sizeof(reserve)) == -1){
          return 1;
        }
        sessions[request.session_id].buffer = malloc(sizeof(reserve));
        memcpy(sessions[request.session_id].buffer,&request,sizeof(reserve));
        sessions[request.session_id].op_code = '5';
      }
      else if(op_code == '6'){
        list request;
        printf("#Client %d requested op 6\n",request.session_id);
        if(read(input_pipe,&request,sizeof(reserve)) == -1){
          return 1;
        }
        sessions[request.session_id].buffer = malloc(sizeof(reserve));
        memcpy(sessions[request.session_id].buffer,&request,sizeof(reserve));
        sessions[request.session_id].op_code = '6';
      }
    }
  }

  close(input_pipe);

  ems_terminate();
  return 0;
}