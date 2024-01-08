#ifndef CLIENT_API_H
#define CLIENT_API_H

#define PIPE_SIZE 40

#include <stddef.h>

//sends the job as a request
/// @param op Operation code integer
/// @param info Information to be sent to the server.
/// @param len Size of the information to be sent.
/// @return 0 if the connection was established successfully, 1 otherwise.
int send_job(int op,void *info,size_t len);

//prints to a file descriptor the arrangement of seats on the client side
/// @param rows Number of rows of the event.
/// @param cols Number of cols of the event.
/// @param seats Array of the seats of the event.
/// @param out_fd File descriptor where the arrangement will be printed.
/// @return 0 if the connection was established successfully, 1 otherwise.
int show_fd(size_t rows,size_t cols,unsigned int* seats, int out_fd);

//prints to a file descriptor the list of events on the client side
/// @param n_ids Number of ids of the event.
/// @param ids Ids of the event.
/// @param out_fd File descriptor where the arrangement will be printed.
/// @return 0 if the connection was established successfully, 1 otherwise.
int list_fd(size_t n_ids,unsigned int* ids,int out_fd);

/// Connects to an EMS server.
/// @param req_pipe_path Path to the name pipe to be created for requests.
/// @param resp_pipe_path Path to the name pipe to be created for responses.
/// @param server_pipe_path Path to the name pipe where the server is listening.
/// @return 0 if the connection was established successfully, 1 otherwise.
int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path);

/// Disconnects from an EMS server.
/// @return 0 in case of success, 1 otherwise.
int ems_quit(void);

/// Creates a new event with the given id and dimensions.
/// @param event_id Id of the event to be created.
/// @param num_rows Number of rows of the event to be created.
/// @param num_cols Number of columns of the event to be created.
/// @return 0 if the event was created successfully, 1 otherwise.
int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols);

/// Creates a new reservation for the given event.
/// @param event_id Id of the event to create a reservation for.
/// @param num_seats Number of seats to reserve.
/// @param xs Array of rows of the seats to reserve.
/// @param ys Array of columns of the seats to reserve.
/// @return 0 if the reservation was created successfully, 1 otherwise.
int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys);

/// Prints the given event to the given file.
/// @param out_fd File descriptor to print the event to.
/// @param event_id Id of the event to print.
/// @return 0 if the event was printed successfully, 1 otherwise.
int ems_show(int out_fd, unsigned int event_id);

/// Prints all the events to the given file.
/// @param out_fd File descriptor to print the events to.
/// @return 0 if the events were printed successfully, 1 otherwise.
int ems_list_events(int out_fd);

#endif  // CLIENT_API_H
