#include "log.h"
#include "config.h"

void dlog(logLevel level, char *format, ...)
{
    va_list argp;

    if (config.curr_rec.enabled) {
	va_start(argp, format);
	vfprintf(config.curr_rec.fd, format, argp);
	va_end(argp);
    }

    if (level <= config.verbose) {
	va_start(argp, format);
	vfprintf(config.outfd, format, argp);
	va_end(argp);
    }
}
