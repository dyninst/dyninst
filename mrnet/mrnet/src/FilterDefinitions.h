#if !defined( filterdefinitions_h )
#define filterdefinitions_h 1

#include <list>
#include <vector>

namespace MRN
{

class Packet;
class RemoteNode;

extern unsigned short TFILTER_NULL;
extern const char * TFILTER_NULL_FORMATSTR;

extern unsigned short TFILTER_SUM;
extern const char * TFILTER_SUM_FORMATSTR;
void tfilter_Sum( std::vector < Packet >&, std::vector < Packet >&, void** );

extern unsigned short TFILTER_AVG;
extern const char * TFILTER_AVG_FORMATSTR;
void tfilter_Avg( std::vector < Packet >&, std::vector < Packet >&, void** );

extern unsigned short TFILTER_MIN;
extern const char * TFILTER_MIN_FORMATSTR;
void tfilter_Min( std::vector < Packet >&, std::vector < Packet >&, void** );

extern unsigned short TFILTER_MAX;
extern const char * TFILTER_MAX_FORMATSTR;
void tfilter_Max( std::vector < Packet >&, std::vector < Packet >&, void** );

extern unsigned short TFILTER_ARRAY_CONCAT;
extern const char * TFILTER_ARRAY_CONCAT_FORMATSTR;
void tfilter_ArrayConcat(std::vector < Packet >&,
                         std::vector < Packet >&,
                         void**);

extern unsigned short TFILTER_INT_EQ_CLASS;
extern const char * TFILTER_INT_EQ_CLASS_FORMATSTR;
void tfilter_IntEqClass( std::vector < Packet >&,
                         std::vector < Packet >&,
                         void** );


extern unsigned short SFILTER_DONTWAIT;

extern unsigned short SFILTER_WAITFORALL;
void sfilter_WaitForAll( std::vector < Packet >&, std::vector < Packet >&,
                         std::list < RemoteNode * >&, void ** );

extern unsigned short SFILTER_TIMEOUT;
void sfilter_TimeOut( std::vector < Packet >&, std::vector < Packet >&,
                      std::list < RemoteNode * >&, void ** );

} // namespace MRN

#endif  /* filterdefinitions_h */
