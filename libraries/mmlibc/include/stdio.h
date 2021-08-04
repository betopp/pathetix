//mmlibc/include/stdio.h
//Standard I/O definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STDIO_H
#define _STDIO_H

#include <mmbits/typedef_file.h>
#include <mmbits/typedef_fpos.h>
#include <mmbits/typedef_off.h>
#include <mmbits/typedef_size.h>
#include <mmbits/typedef_ssize.h>
#include <mmbits/typedef_va_list.h>

#define BUFSIZ 4096
#define L_ctermid 256

#include <mmbits/define_buffer_options.h>
#include <mmbits/define_whence_options.h>

#define FILENAME_MAX 256
#include <mmbits/define_fopen_max.h>
#define FOPEN_MAX _FOPEN_MAX

#include <mmbits/define_eof.h>
#include <mmbits/define_null.h>

#include <mmbits/define_stdinouterr.h>

FILE *fopen(const char *pathname, const char *mode);
FILE *fmemopen(void *buf, size_t size, const char *mode);
int fclose(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
int fflush(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
int fseeko(FILE *stream, off_t offset, int whence);
void rewind(FILE *stream);
int fsetpos(FILE *stream, const fpos_t *pos);
long ftell(FILE *stream);
off_t ftello(FILE *stream);
int fgetpos(FILE *stream, fpos_t *pos);
int fileno(FILE *stream);
void clearerr(FILE *stream);
int putc(int c, FILE *stream);
int putchar(int c);
int putc_unlocked(int c, FILE *stream);
int putchar_unlocked(int c);
int puts(const char *s);
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
int getc(FILE *stream);
int getchar(void);
int getc_unlocked(FILE *stream);
int getchar_unlocked(void);
char *fgets(char *s, int size, FILE *stream);
int fgetc(FILE *stream);
int ungetc(int c, FILE *stream);
int vdprintf(int fd, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int vprintf(const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int dprintf(int fd, const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int vscanf(const char *format, va_list ap);
int fscanf(FILE *stream, const char *format, ...);
int scanf(const char *format, ...);
int vsscanf(const char *str, const char *format, va_list ap);
int sscanf(const char *str, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vfscanf(FILE *stream, const char *format, va_list ap);
char *ctermid(char *s);
FILE *fdopen(int fd, const char *mode);
void flockfile(FILE *filehandle);
FILE *freopen(const char *pathname, const char *mode, FILE *stream);
int ftrylockfile(FILE *filehandle);
void funlockfile(FILE *filehandle);
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
ssize_t  getline(char **lineptr, size_t *n, FILE *stream);
FILE *open_memstream(char **ptr, size_t *sizeloc);
int pclose(FILE *stream);
void perror(const char *s);
FILE *popen(const char *command, const char *type);
int remove(const char *pathname);
int rename(const char *oldn, const char *newn);
int renameat(int oldfd, const char *oldn, int newfd, const char *newn);
void setbuf(FILE *stream, char *buf);
int setvbuf(FILE *stream, char *buf, int mode, size_t size);
FILE *tmpfile(void);


//Todo - maybe make an internal libc header that defines these?
//We use them from multiple source files in the libc.

//Types of FILE we support (i.e. who owns the buffer, who owns the backing FD if any)
typedef enum _FILE_streamtype_e
{
	_FILE_STREAMTYPE_INVALID = 0,
	_FILE_STREAMTYPE_BUFFD, //Buffered file descriptor backing (i.e. a normal fopen or something)
	_FILE_STREAMTYPE_RAWFD, //Directly using a file descriptor, no buffering
	_FILE_STREAMTYPE_STRN, //Writing to someone else's string buffer in memory
	_FILE_STREAMTYPE_MAX
} _FILE_streamtype_t;

//Actual contents of "FILE" structure (called a stream in POSIX)
struct _FILE_s
{
	_FILE_streamtype_t streamtype; //Type of FILE* this is
	
	int fd; //File descriptor underlying the stream
	int omode; //File mode used when opening the stream
	
	char *buf_ptr; //Buffer for the stream
	size_t buf_size; //Total size of buffer, in bytes
	
	size_t buf_rpos; //Read pointer - next location in buffer to read
	size_t buf_wpos; //Write pointer - next location in buffer to write
	int buf_out; //Whether the buffer is waiting to be read (0) or written (1)
	
	int buf_mode; //Buffer mode requested
	
	int error; //Error indicator
	int eof; //End-of-file indicator
};

#endif //_STDIO_H
