
#ifndef _XCOFFLIB_

#define _XCOFFLIB_

#include <xcoff.h>


typedef struct {
	
	struct xcoffhdr *XCOFFhdr;
	struct scnhdr *sechdr;
	struct ldhdr *loadhdr;
	int fd;
	char *name;
	char *buffer;
	unsigned int filesize;
	int opened;
	char* textSectionData;

} XCOFF;


#endif
