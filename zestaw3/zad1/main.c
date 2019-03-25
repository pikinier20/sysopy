#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ftw.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

static int nftwsearch(const char *actualpath, const struct stat *buf, int ft, struct FTW *ftwbuf) {
    if(ftwbuf->level == 0) return 0;
    pid_t child_pid;
    int s;
    if(ft == FTW_D) {
        child_pid = vfork();
        if(child_pid == 0 ) {
            printf("\n\n%s\n", actualpath);
            printf("Child PID:%d\n",(int)getpid());
            execlp("ls", "ls", "-l", actualpath, NULL);
            
        } else {
            wait(&s);
        }
    }
    return 0;
}


int main(int argc, char ** argv) {

    char actualpath[PATH_MAX+1];
    realpath(argv[1], actualpath);
    nftw(argv[1], nftwsearch, 1000, 1);

    return 0;
}