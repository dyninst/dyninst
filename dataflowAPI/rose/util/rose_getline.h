#ifndef Rose_getline_H
#define Rose_getline_H

#ifdef _MSC_VER
    #include <windows.h>
    #undef max
    typedef LONG_PTR ssize_t;
#else
    #include <unistd.h>
#endif

/** Reads a line of text from a stream.
 *
 *  This function reads an entire line from @p stream, storing the text (including the newline and a terminating null
 *  character) in a buffer and storing the buffer address in @p lineptr.  This documentation is copied from the GNU source
 *  code with a few formatting modifications and name changes. The original GNU getline() function is reportedly not available
 *  on Mac OSX v10.6 (Snow Leopard) for licensing reasons, so we provide our own implementation.
 *
 *  Before calling rose_getline(), you should place in @p lineptr the address of a buffer @p n bytes long, allocated with
 *  malloc().  If this buffer is long enough to hold the line, rose_getline() stores the line in this buffer.  Otherwise,
 *  rose_getline() makes the buffer bigger using realloc(), storing the new buffer address back in @p lineptr and the
 *  increased size back in @p n.
 *
 *  If you set @p lineptr to a null pointer, and @p n to zero, before the call, then rose_getline() allocates the initial
 *  buffer for you by calling malloc().
 *
 *  In either case, when rose_getline() returns, @p lineptr points to the text of the line.
 *
 *  When rose_getline() is successful, it returns the number of characters read (including the newline, but not including the
 *  terminating null).  This value enables you to distinguish null characters that are part of the line from the null
 *  character inserted as a terminator.
 *
 *  This is the recommended way to read lines from a stream.  The alternative standard functions are unreliable.
 *
 *  If an error occurs or end of file is reached without any bytes read, getline() returns -1. */
ssize_t rose_getline(char **lineptr, size_t *n, FILE *stream);

#endif
