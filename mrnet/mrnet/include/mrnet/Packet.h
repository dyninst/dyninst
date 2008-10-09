/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(packet_h)
#define packet_h 1

#include <stdarg.h>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "Error.h"
#include "pdr.h"
#include "refCounter.h"
#include "DataElement.h"

#include "xplat/Mutex.h"

namespace MRN
{

class Packet;
typedef boost::shared_ptr<Packet> PacketPtr;

/************************************************************************
  Packet Buffer Format:
    ____________________________________________
    | streamid | tag | src_rank | fmtstr | data|
    --------------------------------------------
**************************************************************************/
class Packet: public Error{
    friend class ParentNode;
    friend class BackEndNode;
    friend class Stream;
    friend class Message;

 public:
    Packet( unsigned short _stream_id, int _tag, const char *fmt, ... );
    Packet( unsigned short _stream_id, int _tag, const void **data, const char *fmt);
    ~Packet();

    int unpack( const char *ifmt, ... );
    const DataElement * operator[] ( unsigned int i ) const;

    int get_Tag( void ) const;
    unsigned short get_StreamId( void ) const;
    const char *get_FormatString( void ) const;
    Rank get_InletNodeRank( void ) const;

    bool operator==(const Packet &)const;
    bool operator!=(const Packet &)const;

    void set_DestroyData( bool b );

    static PacketPtr NullPacket;

 private:
    Packet( bool, unsigned short istream_id, int itag, const char *ifmt, va_list iargs );
    Packet( unsigned int ibuf_len, char *ibuf, Rank iinlet_rank );

    int ExtractVaList( const char *fmt, va_list arg_list ) const;
    int ExtractArgList( const char *fmt, ... ) const;

    const char *get_Buffer( void ) const;
    unsigned int get_BufferLen( void ) const;


    unsigned int get_NumDataElements( void ) const;
    const DataElement * get_DataElement( unsigned int i ) const;

    void ArgList2DataElementArray( va_list arg_list );
    void DataElementArray2ArgList( va_list arg_list ) const;
    void ArgVec2DataElementArray( const void **data );
    void DataElementArray2ArgVec( void **data ) const;
    static bool_t pdr_packet( PDR *, Packet * );

    //Data Members
    uint16_t stream_id;
    int32_t tag;            /* Application/Protocol Level ID */
    Rank src_rank;          /* Null Terminated String */
    char *fmt_str;          /* Null Terminated String */
    char *buf;              /* The entire packed buffer (header+payload)! */
    unsigned int buf_len;
    Rank inlet_rank;
    bool destroy_data;
    std::vector < const DataElement * >data_elements;
    mutable XPlat::Mutex data_sync;
};


}                               /* namespace MRN */
#endif                          /* packet_h */
