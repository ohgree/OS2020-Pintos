#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "lib/kernel/stdio.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

void syscall_init (void) {
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f UNUSED) {
    switch(ESP_WORD(0)) {
        case SYS_HALT:      /* Halt the operating system. */
            halt();
            break;
        case SYS_EXIT:      /* Terminate this process. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            exit(ESP_WORD(1));
            break;
        case SYS_EXEC:      /* Start another process. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            f->eax = exec((const char*)ESP_WORD(1));
            break;
        case SYS_WAIT:      /* Wait for a child process to die. */
            f->eax = wait((pid_t)ESP_WORD(1));
            break;
        case SYS_CREATE:    /* Create a file. */
            break;
        case SYS_REMOVE:    /* Delete a file. */
            break;
        case SYS_OPEN:      /* Open a file. */
            break;
        case SYS_FILESIZE:  /* Obtain a file's size. */
            break;
        case SYS_READ:      /* Read from a file. */
            user_vaddr_check(_ESP(1));
            user_vaddr_check(_ESP(2));
            user_vaddr_check(_ESP(3));
            f->eax = read(
                    (int)ESP_WORD(1),
                    (void*)ESP_WORD(2),
                    (unsigned int)ESP_WORD(3)
                    );
            break;
        case SYS_WRITE:     /* Write to a file. */
            f->eax = write(
                    ESP_WORD(1),
                    (void*)ESP_WORD(2),
                    (unsigned int)ESP_WORD(3)
                    );
            break;
        case SYS_SEEK:      /* Change position in a file. */
            break;
        case SYS_TELL:      /* Report current position in a file. */
            break;
        case SYS_CLOSE:     /* Close a file. */
            break;
            // Additional system calls
        case SYS_FIBONACCI:
            f->eax = fibonacci(ESP_WORD(1));
            break;
        case SYS_MAX_OF_FOUR:
            f->eax = max_of_four_int(
                    ESP_WORD(1),
                    ESP_WORD(2),
                    ESP_WORD(3),
                    ESP_WORD(4)
                    );
            break;
        default: break;
    }

    /*thread_exit ();*/
}
void halt(void) {
    shutdown_power_off();
}
void exit(int status) {
    printf("%s: exit(%d)\n", thread_name(), status);
    thread_current()->exit_status = status;
    thread_exit();
}
pid_t exec(const char* cmd_line) {
    return process_execute(cmd_line);
}
int wait(pid_t pid) {
    return process_wait(pid);
}
bool create(const char* file, unsigned initial_size);
bool remove(const char* file);
int open(const char* file);
int filesize(int fd);
int read(int fd, void* buffer, unsigned size) {
    // stdin
    int i = 0;
    if(fd == 0) {
        for(i = 0 ; i < (int)size ; i++) {
            //buffer read
            ((uint8_t*)buffer)[i] = input_getc();
            if(((char*)buffer)[i] == '\n') break;
        }
    }
    return i;
}
int write(int fd, const void* buffer, unsigned size) {
    // stdout
    if(fd == 1) {
        putbuf(buffer, size);
        return size; 
    }
    return -1;
}
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);

void user_vaddr_check(const void* vaddr) {
    if(!is_user_vaddr(vaddr))
        exit(-1);
}

int max_of_four_int(int a, int b, int c, int d) {
    int max = a;
    if(max < b)
        max = b;
    if(max < c)
        max = c;
    if(max < d)
        max = d;

    return max;
}

int fibonacci(int num) {
    int prev = 0, next = 1, t;
    for(int i=0 ; i < num ; i++) {
        t = next;
        next = prev + next;
        prev = t;
    }

    return prev;
}
