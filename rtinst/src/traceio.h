#define TRACE_BUF_SIZE   65535

#define PACKET_SIZE 5  /* Max buffer size for CMMD_send_packet() */
#define MAX_NODES 64
#define HANDLER_DATALEN    TRACE_BUF_SIZE

#define BITS_FOR_NODENUM 6
#define EXTRACT_NODENUM(i) (i & (MAX_NODES - 1))
#define EXTRACT_WORDNUM(i) ((unsigned)i >> BITS_FOR_NODENUM)

#define TRACE(dataPtr, dataLen)                         \
  do {                                                  \
        mustRetry = 0;                                  \
        currPtr = freePtr;                              \
        freePtr += dataLen;                             \
        if (endPtr < freePtr) {                         \
            must_end_timeslice();                       \
            continue;     /* go back and try again */   \
        }                                               \
        bcopy (dataPtr, currPtr, dataLen);              \
        currPtr = freePtr;                              \
  } while (mustRetry);


extern char *currPtr;           /* current pointer in buffer  */
extern char *freePtr;           /* pointer to next free byte in buffer */
extern char *endPtr;            /* last byte in trace buffer */
extern char *traceBuffer;       /* beginning of trace buffer */
extern int mustRetry;           /* signal variable from consumer -> producer */
                                /* or to put it another way, */
                                /* Handler -> Trace macro */

