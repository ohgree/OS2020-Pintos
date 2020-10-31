#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

#define MAX_COMMAND_LEN 512
#define MAX_ARG_LEN 128
#define WORD_SIZE 4
#define WHITESPACE_CHARS " \t\n\r"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
void parse_arg(const char* src, char* dest, char** next_ptr);


#endif /* userprog/process.h */
