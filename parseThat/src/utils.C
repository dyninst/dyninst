#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#include "config.h"
#include "utils.h"
#include "log.h"

#define MAX_RETRY 10
#define INITIAL_STRLEN 1024

char *chomp(char *str)
{
    unsigned tail = strlen(str);
    if (tail && str[tail - 1] == '\n')
	str[tail - 1] = '\0';
    return str;
}

char *fgets_static(FILE *stream)
{
    static int len = 0;
    static char *buf = NULL;

    if (!buf) {
	buf = (char *)realloc(NULL, INITIAL_STRLEN);
	if (!buf) {
	    dlog(ERR, "Could not allocate %d bytes in fgets_static().\n", INITIAL_STRLEN);
	    return NULL;
	}
	len = INITIAL_STRLEN;
    }

    // Initialize the buffer for safety.
    *buf = '\0';

    int i = 0;
    int tail = 0;
    char *retval = NULL;
    while (!feof(stream)) {
        errno = 0;
        retval = fgets(buf + tail, len - tail, stream);

	if (errno == EINTR || errno == ECHILD) {
	    if (config.state != NORMAL) {
		errno = 0;
		return NULL;

	    } else if (errno == EINTR && ++i > MAX_RETRY) {
		dlog(ERR, "fgets() has been interrupted %d times.  Giving up.\n", MAX_RETRY);
		return NULL;
	    }
	    continue;

	} else if (errno) {
	    dlog(ERR, "Error in fgets(): %s\n", strerror(errno));
	    return NULL;

        } else if (!retval) {
	    return NULL;
	}

	tail = strlen(buf);
	if (tail && buf[tail - 1] != '\n' && !feof(stream)) {
	    char *newbuf = (char *)realloc(buf, len * 2);
	    if (!newbuf) {
		dlog(ERR, "Could not allocate %d bytes in fgets_static().\n", len * 2);
		return NULL;
	    }
	    buf = newbuf;
	    len *= 2;

	    continue;
	}
	break;
    }
    return (*buf ? buf : NULL);
}

char *sprintf_static(const char *fmt, ...)
{
    static int len = 0;
    static char *buf = NULL;

    if (!buf) {
	buf = (char *)realloc(NULL, INITIAL_STRLEN);
	if (!buf) {
	    dlog(ERR, "Could not allocate %d bytes in sprintf_static().\n", INITIAL_STRLEN);
	    return NULL;
	}
	len = INITIAL_STRLEN;
    }

    int retval = len;
    va_list ap;

    while (1) {
	va_start(ap, fmt);
	retval = vsnprintf(buf, len, fmt, ap);
	va_end(ap);

	if (retval < 0) {
	    dlog(ERR, "vsnprintf() returned %d in sprintf_static().\n", retval);
	    return NULL;

	} else if (len < retval) {
	    char *newbuf = (char *)realloc(buf, ++retval);
	    if (!newbuf) {
		dlog(ERR, "Could not allocate %d bytes in sprintf_static().\n", retval);
		return NULL;
	    }
	    buf = newbuf;
	    len = retval;

	    continue;
	}
	break;
    }
    return buf;
}

char *strcat_static(char *s1, char *s2)
{
    static int len = 0;
    static char *buf = NULL;

    int maxlen = strlen(s1) + strlen(s2) + 1;
    bool same_str = (buf == s1);

    if (maxlen > len) {
	char *newbuf = (char *)realloc(buf, maxlen);
	if (!newbuf) {
	    dlog(ERR, "Could not allocate %d bytes in strcat_strlen().\n", maxlen);
	    return NULL;
	}
	buf = newbuf;
	len = maxlen;
    }

    if (!same_str) strcpy(buf, s1);
    return strcat(buf, s2);
}

bool checkStr(const char *s)
{
    if (!(*s)) return true;
    while (*s) {
	if (*s == '\"' || isspace(*s))
	    return true;
	++s;
    }
    return false;
}

char *encodeStr(const char *s)
{
    static unsigned len = 0;
    static char *buf = NULL;

    unsigned maxlen = 2 + (strlen(s) * 2) + 1;
    if (len < maxlen) {
	char *newbuf = (char *)realloc(buf, maxlen);
	if (!newbuf) {
	    dlog(ERR, "Could not allocate %d bytes in encodeStr().\n", maxlen);
	    return NULL;
	}
	buf = newbuf;
	len = maxlen;
    }

    if (!checkStr(s))
	return strcpy(buf, s);

    buf[0] = '\"';
    unsigned j = 0;
    for (unsigned i = 0; s[i] != '\0'; ++i) {
	if (s[i] == '\"' || s[i] == '\\')
	    buf[++j] = '\\';
	buf[++j] = s[i];
    }
    buf[++j] = '\"';
    buf[++j] = '\0';

    return buf;
}

char *decodeStr(const char *s, char **end_ptr)
{
    static unsigned len = 0;
    static char *buf = NULL;

    unsigned newlen = strlen(s) + 1;
    if (newlen > len) {
	char *newbuf = (char *)realloc(buf, newlen);
	if (!newbuf) {
	    dlog(ERR, "Could not allocate %d bytes in decodeStr().\n", newlen);
	    return NULL;
	}
	len = newlen;
	buf = newbuf;
    }

    unsigned i;
    if (s[0] != '\"') {
	for (i = 0; s[i] && !isspace(s[i]); ++i);
	if (i > 0) strncpy(buf, s, i);
	buf[i] = '\0';

	if (end_ptr) *end_ptr = const_cast<char *>(s + i);
	return buf;
    }

    unsigned j = 0;
    for (i = 1; s[i] != '\"'; ++i) {
	if (s[i] == '\\') ++i;
	buf[j++] = s[i];
    }
    buf[j] = '\0';

    if (end_ptr) *end_ptr = const_cast<char *>(s + i + 1);
    return buf;
}
