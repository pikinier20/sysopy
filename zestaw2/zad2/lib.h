#ifndef LIB_H
#define LIB_H

extern char op;
extern time_t time_from_epoch;

void traverse_directory(char *dirpath);

void nftw_wrapper(char *path);

#endif