//pipe.h
//Pipe implementation in kernel
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef PIPE_H
#define PIPE_H

#include <sys/types.h>

//Note - pipe reference counts are complex because we need to know if there are readers/writers when writing/reading.
//The pipes store a "refs" count that just counts how many inodes refer to that pipe as a named pipe.
//They also store "refs_r" and "refs_w" counts of how many open file descriptors can read or write the pipe.

//Makes a new pipe. Returns its ID. The pipe starts with one reference.
id_t pipe_new(void);

//Increments the reference-count of an existing pipe. 
//If access is zero, changes the overall reference-count.
//Otherwise, changes the count of readers and/or writers.
//Returns 0 or a negative error number.
int pipe_incr(id_t id, int access);

//Decrements the reference-count of a given pipe.
//If access is zero, changes the overall reference-count.
//Otherwise, changes the count of readers and/or writers.
//Returns 0 or a negative error number.
int pipe_decr(id_t id, int access);

//Writes to the given pipe.
ssize_t pipe_write(id_t id, const void *buf, size_t nbytes);

//Reads from the given pipe.
ssize_t pipe_read(id_t id, void *buf, size_t nbytes);

#endif //PIPE_H
