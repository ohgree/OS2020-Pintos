#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "lib/kernel/stdio.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

void halt(void) {
    shutdown_power_off();
}
void exit(int status) {
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
    int i;
    if(fd == 0) {
        for(i = 0 ; i < size ; i++) {
            //buffer read
            if(!((char*)buffer)[i]) break;
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

    void
syscall_init (void) 
{
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

    static void
syscall_handler (struct intr_frame *f UNUSED) 
{
    /*printf("system call: %d\n", *(uint32_t*)(f->esp));*/

    /*hex_dump(f->esp, f->esp, 100, 1);*/

    switch(_ESP(0)) {
        case SYS_HALT:      /* Halt the operating system. */
            break;
        case SYS_EXIT:      /* Terminate this process. */
            exit(_ESP(WORD_SIZE));
            break;
        case SYS_EXEC:      /* Start another process. */
            break;
        case SYS_WAIT:      /* Wait for a child process to die. */
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
            break;
        case SYS_WRITE:     /* Write to a file. */
            write(
                    _ESP(WORD_SIZE),
                    (void*)_ESP(WORD_SIZE * 2),
                    (unsigned int)_ESP(WORD_SIZE * 3)
                 );
            break;
        case SYS_SEEK:      /* Change position in a file. */
            break;
        case SYS_TELL:      /* Report current position in a file. */
            break;
        case SYS_CLOSE:     /* Close a file. */
            break;
        default: break;
    }

    /*thread_exit ();*/
}
