/*
 * Created by luckpig on 2017/3/16.
 * Usage
 * write pid type value address
 *
 * type
 * 'b' for 1 byte integer
 * 's' for 2 bytes integer
 * 'i' for 4 bytes integer
 * 'li'for 8 bytes integer
 * 'f' for float
 * 'd' for double
*/

#include <sys/ptrace.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>

#define argPid argv[1]
#define argType argv[2]
#define argValue argv[3]
#define argAddress argv[4]

union data_t {
    unsigned char bytes[8];
    unsigned char c;
    unsigned short s;
    unsigned int i;
    unsigned long long int li;
    float f;
    double d;
};

int main(int argc, char *argv[]) {
    pid_t pid = strtol(argPid, NULL, NULL);
    off64_t address = strtoll(argAddress, NULL, 16);
    char memPath[20];
    union data_t data;
    size_t dataSize = 0;
    int memFd;

    //construct path to process's mem file
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);

    //store value
    if (strcmp(argType, "b") == 0) {
        dataSize = 1;
        data.c = strtoull(argValue, NULL, NULL);
    } else if (strcmp(argType, "s") == 0) {
        dataSize = 2;
        data.s = strtoull(argValue, NULL, NULL);
    } else if (strcmp(argType, "i") == 0) {
        dataSize = 4;
        data.i = strtoull(argValue, NULL, NULL);
    } else if (strcmp(argType, "li") == 0) {
        dataSize = 8;
        data.li = strtoull(argValue, NULL, NULL);
    } else if (strcmp(argType, "f") == 0) {
        dataSize = 4;
        data.f = strtof(argValue, NULL);
    } else if (strcmp(argType, "d") == 0) {
        dataSize = 8;
        data.d = strtod(argValue, NULL);
    }

    //use ptrace to attach to process
    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    waitpid(pid, NULL, NULL);

    //open mem file and write value to specified address
    memFd = open(memPath, O_WRONLY);
    pwrite64(memFd, data.bytes, dataSize, address);
    close(memFd);

    //when complete, detach from process
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

