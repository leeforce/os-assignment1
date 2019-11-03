#include<stdio.h>
#include<sys/ptrace.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/user.h>
#include<sys/reg.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<linux/kernel.h>

int main()
{
    mycall();
    return 0;
}