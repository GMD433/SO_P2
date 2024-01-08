#ifndef SERVER_OPERATIONS_H
#define SERVER_OPERATIONS_H

#include <stddef.h>

/// Session struct.
typedef struct{
    char const *request_path_name;
    char const *response_path_name;
    int input_pipe;
    int res_input_pipe;
    char op_code;
    void *buffer;
    pthread_mutex_t lock;
    pthread_t thread;
    pthread_cond_t cond;
} session;

/// Initializes the EMS state.
/// @param delay_us Delay in microseconds.
/// @return 0 if the EMS state was initialized successfully, 1 otherwise.
int ems_init(unsigned int delay_us);

/// Destroys the EMS state.
int ems_terminate();

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
int ems_reserve(unsigned int event_id, size_t num_seats, size_t *xs, size_t *ys);

/// Prints the given event.
/// @param out_fd File descriptor to print the event to.
/// @param event_id Id of the event to print.
/// @return 0 if the event was printed successfully, 1 otherwise.
int ems_show(int out_fd, unsigned int event_id);

/// Prints all the events.
/// @param out_fd File descriptor to print the events to.
/// @return 0 if the events were printed successfully, 1 otherwise.
int ems_list_events(int out_fd);

/// Gets the rows of an event.
/// @param event_id The id of the event.
/// @return the rows if successfull,error if not.
size_t get_rows(unsigned int event_id);

/// Gets the columns of an event.
/// @param event_id The id of the event.
/// @return the columns if successfull,error if not.
size_t get_cols(unsigned int event_id);

/// Gets the seats of an event.
/// @param event_id The id of the event.
/// @return the seats array if successfull,error if not.
unsigned int* get_seats(unsigned int event_id);

/// Gets the number of events.
/// @return the number of events if successfull,error if not.
size_t get_n_events();

/// Gets the ids of events.
/// @return the ids of events if successfull,error if not.
unsigned int* get_ids();

#endif  // SERVER_OPERATIONS_H
