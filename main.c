/* The follwing program is based on an example
   from the Linux Journal Article "Playing with ptrace, Part 1
   It was modified to run on a modern x86_64 processor (like pu1, pu2, pu3, etc.)
*/
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <x86_64-linux-gnu/asm/unistd_64.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define MAX_STRING_SIZE 50       // maximum number of bytes to be read from tracee


char syscall_buf[MAX_STRING_SIZE];

/*
__NR_write is the number of the write() system call
  see x86_64-linux-gnu/asm/unistd_64.h
*/

/*

#define __NR_read 0
#define __NR_write 1
#define __NR_open 2
#define __NR_close 3

#define __NR_openat 257
#define __NR_exit_group 231

*/

const int long_size = sizeof(void*);

void getdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * long_size,
                          NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * long_size,
                          NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}

// returns 0 if generic_call was called upon entry of a system call, and
// return 1 if called upon exit from a system call
int generic_call(int child, unsigned long params[4], long* syscall_nr, long* syscall_ret)
{
  int insyscall = 0;  // indicates whether we're called upon entry (=0) into or
                      // exit from (=1) a system call
  long orig_rax, rax;
  int nargs=0;

  rax = ptrace(PTRACE_PEEKUSER, child, 8 * RAX, NULL);

  if( rax == -ENOSYS )
    insyscall = 0;
  else
    insyscall = 1;

  orig_rax = ptrace(PTRACE_PEEKUSER,child, 8 * ORIG_RAX, NULL);

  params[0] = ptrace(PTRACE_PEEKUSER, child, 8 * RDI, NULL);
  params[1] = ptrace(PTRACE_PEEKUSER,child, 8 * RSI, NULL);
  params[2] = ptrace(PTRACE_PEEKUSER,child, 8 * RDX, NULL);
  params[3] = ptrace(PTRACE_PEEKUSER,child, 8 * R10, NULL);

  rax = ptrace(PTRACE_PEEKUSER, child, 8 * RAX, NULL);

  *syscall_ret = rax;

  if( orig_rax == __NR_write || orig_rax == __NR_read )
     getdata(child, params[1], syscall_buf, MAX(0, MIN(params[2], MAX_STRING_SIZE)));

  if( orig_rax == __NR_open )
     getdata(child, params[0], syscall_buf, MAX_STRING_SIZE);

  if( orig_rax == __NR_openat )
     getdata(child, params[1], syscall_buf, MAX_STRING_SIZE);

  if( orig_rax == __NR_connect )
     getdata(child, params[1], syscall_buf, params[2]);

   ptrace(PTRACE_SYSCALL,child, NULL, NULL);

   *syscall_nr = orig_rax;

   return insyscall;
}


int main(int argc, char* argv[])
{
    pid_t child, orig_child;
    long syscall_nr, syscall_ret;
    unsigned long params[4];
    char tbuf[100];
    int i;
    int status;
    int maxlength;
    int ret;
    int syscall_count= 0;
    void (*display_func)(FILE*, int);

    if(argc <= 1)
    {
       fprintf(stderr, "Usage %s command_name command_arg_1 command_arg_2 ... \n", argv[0]);
       exit(-1);
    }

    child = fork();

    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        char com_buf[1000];
        char* myargv[100];

        myargv[0] = argv[1];
        i = 2;
        for(; i < argc; i++)
        {
          myargv[i-1] = argv[i];
        }

        myargv[i-1]=NULL;


        kill(getpid(), SIGSTOP);

        execvp(myargv[0], myargv);
        _exit(-1);
    }
    else
    {

       ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACEEXEC | PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEVFORK | PTRACE_O_TRACEVFORKDONE | PTRACE_O_TRACEEXIT | PTRACE_O_TRACESYSGOOD  );
       ptrace(PTRACE_SYSCALL, child, 0, 0);

       while(1)
       {

        int child =  wait(&status);

           if( child <=  0)

           {
             fprintf(stderr, "Child exited\n");
             break;
           }

           ret = generic_call(child, params, &syscall_nr, &syscall_ret);


           switch(syscall_nr)
            {

             case __NR_read:

                // ADD CODE: handle read() system call

               break;

             case __NR_write:

                // ADD CODE: handle write() system call

               break;

             case __NR_openat:

                // ADD CODE: handle openat() system call

               break;

             case __NR_close:

                // ADD CODE: handle openat() system call

               break;


             case __NR_exit:

                // ADD CODE: handle _exit() system call

               break;

             default:

                // ADD CODE: handle any other system call

               break;


           }
        }

      }


      return 0;
}
