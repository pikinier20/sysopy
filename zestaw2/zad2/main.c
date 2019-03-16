#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/times.h>
#include <string.h>
#include <zconf.h>
#include <errno.h>
#include "lib.h"

int main(int argc, char **argv){
    traverse_directory("/home",0,'>');
    return 0;
}