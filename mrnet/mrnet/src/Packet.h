#if !defined(packet_h)
#define packet_h 1

#include <stdarg.h>
#include <vector>

#include "mrnet/src/DataElement.h"
#include "mrnet/src/Errors.h"
#include "mrnet/src/pdr.h"

namespace MRN
{
class RemoteNode;

/************************************************************************
  Packet Buffer Format:
    __________________________________________
    | streamid | tag | srcstr | fmtstr | data|
    ------------------------------------------
**************************************************************************/
class Packet:public Error
{
 private:
    std::vector < DataElement * >data_elements;

    unsigned short stream_id;
    int tag;                /* Application/Protocol Level ID */
    char *src;              /* Null Terminated String */
    char *fmt_str;          /* Null Terminated String */
    char *buf;              /* The entire packed/encoded buffer (header+payload)! */
    unsigned int buf_len;
    int ArgList2DataElementArray( va_list arg_list );
    void DataElementArray2ArgList( va_list arg_list );

 public:
    Packet( unsigned short _stream_id, int _tag, const char *fmt, ... );
    Packet( unsigned short _stream_id, int _tag, const char *fmt,
            va_list );
    Packet( unsigned short _stream_id, int _tag, DataElement *,
            const char *fmt );
    Packet( unsigned int _buf_len, char *_buf );

    int ExtractVaList( const char *fmt, va_list arg_list );
    int ExtractArgList( const char *fmt, ... );

    static bool_t pdr_packet( PDR *, Packet * );
    int get_Tag(  );
    unsigned short get_StreamId(  );
    char *get_Buffer(  );
    unsigned int get_BufferLen(  );
    const char *get_FormatString(  );
    unsigned int get_NumDataElements(  );
    DataElement *get_DataElement( unsigned int );
    DataElement *operator[] ( unsigned int );
    RemoteNode *inlet_node;
};

inline int Packet::get_Tag(  )
{
    return tag;
}

inline unsigned short Packet::get_StreamId(  )
{
    return stream_id;
}

inline char *Packet::get_Buffer(  )
{
    return buf;
}

inline unsigned int Packet::get_BufferLen(  )
{
    return buf_len;
}

inline const char *Packet::get_FormatString(  )
{
    return fmt_str;
}

inline unsigned int Packet::get_NumDataElements(  )
{
    return data_elements.size(  );
}

inline DataElement *Packet::get_DataElement( unsigned int i )
{
    return data_elements[i];
}

inline DataElement *Packet::operator[] ( unsigned int i )
{
    return data_elements[i];
}

}                               /* namespace MRN */
#endif                          /* packet_h */
