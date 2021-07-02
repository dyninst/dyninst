/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "config.h"
#include "utils.h"
#include "log.h"
#include "ipc.h"

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

char *strcat_static(const char *s1, const char *s2)
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

char *strcat_static(char *s1, char *s2)
{
  return strcat_static((const char *) s1, (const char *) s2);
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
	    dlog(ERR, "Could not allocate %u bytes in encodeStr().\n", maxlen);
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
	    dlog(ERR, "Could not allocate %u bytes in decodeStr().\n", newlen);
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

static struct rusage ru_start;
static void mark_usage(struct rusage *ru)
{
    getrusage(RUSAGE_SELF, ru);

    struct stat s;
    if (!ru->ru_maxrss && stat("/proc/self/status", &s) == 0) {
        unsigned long vmRSS  = 0;

        FILE *fp = fopen("/proc/self/status", "r");
        if (!fp) return;

        char buf[1024] = {0};
        char *ptr = buf, *end = buf + sizeof(buf) - 1;
        while (!feof(fp) && !ferror(fp)) {
            int i = fread(ptr, sizeof(char), end - ptr, fp);
            ptr[i+1] = '\0';

            ptr = strstr(buf, "VmRSS:");
            if (ptr) sscanf(ptr, "VmRSS: %lu", &vmRSS);

            if (!feof(fp) && !ferror(fp)) {
                ptr = strrchr(buf, '\n');
                if (!ptr++) break;

                for (i = 0; ptr + i < end; ++i) buf[i] = ptr[i];
                ptr = buf + i;
            }
        }
        fclose(fp);

        if (vmRSS)
            ru->ru_maxrss = vmRSS;
    }
}

void track_usage()
{
    mark_usage(&ru_start);
}

#ifndef timersub
#define timersub(b, a, r) \
do { \
    (r)->tv_sec = (b)->tv_sec - (a)->tv_sec;\
    (r)->tv_usec = (b)->tv_usec -(a)->tv_usec;\
    if((r)->tv_usec < 0) {\
        (r)->tv_sec--;\
        (r)->tv_usec += 1000000;\
    } \
} while(0)
#endif

#ifndef timeradd
#define timeradd(b, a, r) \
do { \
    (r)->tv_sec = (b)->tv_sec + (a)->tv_sec;\
    (r)->tv_usec = (b)->tv_usec + (a)->tv_usec;\
    if((r)->tv_usec > 1000000) {\
        (r)->tv_sec += (r)->tv_usec / 1000000;\
        (r)->tv_usec = (r)->tv_usec % 1000000;\
    } \
} while(0)
#endif

void report_usage()
{
    struct rusage ru_end;
    mark_usage(&ru_end);

    timeval total_cpu;
    timersub(&ru_end.ru_utime, &ru_start.ru_utime, &ru_end.ru_utime);
    timersub(&ru_end.ru_stime, &ru_start.ru_stime, &ru_end.ru_stime);
    timeradd(&ru_end.ru_utime, &ru_end.ru_stime, &total_cpu);
    long total_mem = (ru_end.ru_maxrss - ru_start.ru_maxrss);
    char cpubuf[32], membuf[32];
    snprintf(cpubuf, sizeof(cpubuf), "CPU: %ld.%06ld",
             total_cpu.tv_sec, total_cpu.tv_usec);
    snprintf(membuf, sizeof(membuf), "MEMORY: %ld", total_mem);

    if (config.printipc || !config.no_fork) {
        sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, cpubuf);
        sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, membuf);

    } else if (config.no_fork) {
        // Directly print if --suppress-ipc and -S options are used.
        fprintf(config.outfd, "%s\n%s\n", cpubuf, membuf);
    }
}
