#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/user/syscall.h"

#define WORD_SIZE 4
#define _ESP(x) (f->esp+x)
#define ESP_WORD(x) (*(uint32_t*)_ESP(WORD_SIZE * x))

void syscall_init (void);

void halt(void);
void exit(int status);
pid_t exec(const char* cmd_line);
int wait(pid_t pid);
bool create(const char* file, unsigned initial_size);
bool remove(const char* file);
int open(const char* file);
int filesize(int fd);
int read(int fd, void* buffer, unsigned size);
int write(int fd, const void* buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
void user_vaddr_check(const void* vaddr);

// Additional system calls
int max_of_four_int(int a, int b, int c, int d);
int fibonacci(int num);


#endif /* userprog/syscall.h */
