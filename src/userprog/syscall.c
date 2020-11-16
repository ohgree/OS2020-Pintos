#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "lib/kernel/stdio.h"
#include "lib/string.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"

struct file {
    struct inode *inode;
    off_t pos;
    bool deny_write;
};

struct lock file_lock;

static void syscall_handler (struct intr_frame *);

void syscall_init (void) {
    lock_init(&file_lock);
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f UNUSED) {
    /*hex_dump(f->esp, f->esp, 100, 1);*/
    switch(ESP_WORD(0)) {
        case SYS_HALT:      /* Halt the operating system. */
            halt();
            break;
        case SYS_EXIT:      /* Terminate this process. */
            /*user_vaddr_check(_ESP(WORD_SIZE*1));*/
            /*printf("%x < %x ?\n", _ESP(WORD_SIZE*1), PHYS_BASE);*/
            /*printf(": %s\n", is_user_vaddr(_ESP(WORD_SIZE*1)) ? "true" : "false");*/
            if(!is_user_vaddr(_ESP(WORD_SIZE*1))) {
                exit(-1);
            }
            exit(ESP_WORD(1));
            break;
        case SYS_EXEC:      /* Start another process. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            f->eax = exec((const char*)ESP_WORD(1));
            break;
        case SYS_WAIT:      /* Wait for a child process to die. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            f->eax = wait((pid_t)ESP_WORD(1));
            break;
        case SYS_CREATE:    /* Create a file. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            user_vaddr_check(_ESP(WORD_SIZE*2));
            f->eax = create(
                    (const char*)ESP_WORD(1),
                    (unsigned int)ESP_WORD(2)
                    );
            break;
        case SYS_REMOVE:    /* Delete a file. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            f->eax = remove((const char*)ESP_WORD(1));
            break;
        case SYS_OPEN:      /* Open a file. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            f->eax = open((const char*)ESP_WORD(1));
            break;
        case SYS_FILESIZE:  /* Obtain a file's size. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            f->eax = filesize((int)ESP_WORD(1));
            break;
        case SYS_READ:      /* Read from a file. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            user_vaddr_check(_ESP(WORD_SIZE*2));
            user_vaddr_check(_ESP(WORD_SIZE*3));
            f->eax = read(
                    (int)ESP_WORD(1),
                    (void*)ESP_WORD(2),
                    (unsigned int)ESP_WORD(3)
                    );
            break;
        case SYS_WRITE:     /* Write to a file. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            user_vaddr_check(_ESP(WORD_SIZE*2));
            user_vaddr_check(_ESP(WORD_SIZE*3));
            f->eax = write(
                    ESP_WORD(1),
                    (void*)ESP_WORD(2),
                    (unsigned int)ESP_WORD(3)
                    );
            break;
        case SYS_SEEK:      /* Change position in a file. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            user_vaddr_check(_ESP(WORD_SIZE*2));
            seek((int)ESP_WORD(1), (unsigned int)ESP_WORD(2));
            break;
        case SYS_TELL:      /* Report current position in a file. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            f->eax = tell((int)ESP_WORD(1));
            break;
        case SYS_CLOSE:     /* Close a file. */
            user_vaddr_check(_ESP(WORD_SIZE*1));
            close((int)ESP_WORD(1));
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
}

void halt(void) {
    shutdown_power_off();
}

void exit(int status) {
    printf("%s: exit(%d)\n", thread_name(), status);
    thread_current()->exit_status = status;

    for(int i = STDOUT_FILENO + 2 ; i < MAX_FD_SIZE ; i++) {
        if(thread_current()->fd[i])
            close(i);
    }
    thread_exit();
}

pid_t exec(const char* cmd_line) {
    user_vaddr_check(cmd_line);
    return process_execute(cmd_line);
}

int wait(pid_t pid) {
    return process_wait(pid);
}

bool create(const char* file, unsigned initial_size) {
    user_vaddr_check(file);
    if(!file) exit(-1);
    return filesys_create(file, initial_size);
}

bool remove(const char* file) {
    user_vaddr_check(file);
    if(!file) exit(-1);
    return filesys_remove(file);
}

int open(const char* file) {
    int ret = -1;
    struct file* fp;

    user_vaddr_check(file);
    if(!file) exit(-1);

    lock_acquire(&file_lock);

    if((fp = filesys_open(file))) {
        for(int i = STDOUT_FILENO + 2 ; i < MAX_FD_SIZE ; i++) {
            if(!thread_current()->fd[i]) {
                if(!strcmp(thread_current()->name, file)) {
                    file_deny_write(fp);
                }
                thread_current()->fd[i] = fp;
                ret = i;
                break;
            }
        }
    }

    lock_release(&file_lock);

    return ret;
}

int filesize(int fd) {
    if(!thread_current()->fd[fd]) exit(-1);
    return file_length(thread_current()->fd[fd]);
}

int read(int fd, void* buffer, unsigned size) {
    int i = 0;
    user_vaddr_check(buffer);
    if(!buffer){
        /*printf("NULL buffer!\n");*/
        exit(-1);
    }
    /*printf("\nBuffer addr: %x\n\n", buffer);*/
    lock_acquire(&file_lock);
    // stdin
    if(fd == STDIN_FILENO) {
        for(i = 0 ; i < (int)size ; i++) {
            //buffer read
            ((uint8_t*)buffer)[i] = input_getc();
            if(((char*)buffer)[i] == '\0')
                break;
        }
    } else if(fd > STDOUT_FILENO + 1) {
        if(!thread_current()->fd[fd]) exit(-1);
        i = file_read(thread_current()->fd[fd], buffer, size);
    }

    lock_release(&file_lock);

    return i;
}

int write(int fd, const void* buffer, unsigned size) {
    /*printf("written\n");*/
    int ret = -1;
    user_vaddr_check(buffer);
    if(!buffer) exit(-1);

    lock_acquire(&file_lock);
    // stdout
    if(fd == STDOUT_FILENO) {
        putbuf(buffer, size);
        ret = size; 
    } else if(fd > STDOUT_FILENO + 1) {
        if(!thread_current()->fd[fd]) exit(-1);

        if(thread_current()->fd[fd]->deny_write) {
            file_deny_write(thread_current()->fd[fd]);
        }
        ret = file_write(thread_current()->fd[fd], buffer, size);
    }

    lock_release(&file_lock);
    return ret;
}

void seek(int fd, unsigned position) {
    if(!thread_current()->fd[fd]) exit(-1);
    file_seek(thread_current()->fd[fd], position);
}

unsigned tell(int fd) {
    if(!thread_current()->fd[fd]) exit(-1);
    return file_tell(thread_current()->fd[fd]);
}

void close(int fd) {
    struct file* fp;
    if(!thread_current()->fd[fd]) exit(-1);

    fp = thread_current()->fd[fd];
    thread_current()->fd[fd] = NULL;

    return file_close(fp);
}

void user_vaddr_check(const void* vaddr) {
    if(!is_user_vaddr((unsigned)vaddr)) {
        exit(-1);
    }
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
