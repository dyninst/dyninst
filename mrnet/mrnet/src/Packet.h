/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(packet_h)
#define packet_h 1

#include <stdarg.h>
#include <vector>

#include "mrnet/src/Error.h"
#include "mrnet/src/pdr.h"
#include "mrnet/src/refCounter.h"
#include "mrnet/src/DataElement.h"

#include "xplat/Mutex.h"

namespace MRN
{
class RemoteNode;
class DataElement;

/************************************************************************
  Packet Buffer Format:
    __________________________________________
    | streamid | tag | srcstr | fmtstr | data|
    ------------------------------------------
**************************************************************************/
class PacketData: public Error{
 private:
    uint16_t stream_id;
    int32_t tag;            /* Application/Protocol Level ID */
    char *src;              /* Null Terminated String */
    char *fmt_str;          /* Null Terminated String */
    char *buf;              /* The entire packed buffer (header+payload)! */
    unsigned int buf_len;
    const RemoteNode *inlet_node;
    bool destroy_data;
    std::vector < const DataElement * >data_elements;
    mutable XPlat::Mutex data_sync;

    void ArgList2DataElementArray( va_list arg_list );
    void DataElementArray2ArgList( va_list arg_list ) const;
    static bool_t pdr_packet( PDR *, PacketData * );

 public:
    PacketData( unsigned short _stream_id, int _tag, const char *fmt, va_list );
    PacketData( unsigned int ibuf_len, char *ibuf, const RemoteNode * rn );
    PacketData( const PacketData& );
    PacketData& operator=(const PacketData&);
    ~PacketData();

    void set_DestroyData( bool b ) {
        data_sync.Lock();
        destroy_data = b;
        data_sync.Unlock();
    }

    bool operator==(const PacketData&)const;
    bool operator!=(const PacketData&)const;

    int ExtractVaList( const char *fmt, va_list arg_list ) const;

    int get_Tag( void ) const {
        data_sync.Lock();
        int ret = tag;
        data_sync.Unlock();
        return ret;
    }

    unsigned short get_StreamId(  ) const {
        data_sync.Lock();
        unsigned short ret = stream_id;
        data_sync.Unlock();
        return ret;
    }

    const char *get_Buffer(  ) const {
        data_sync.Lock();
        const char * ret = buf;
        data_sync.Unlock();
        return ret;
    }

    unsigned int get_BufferLen(  ) const {
        data_sync.Lock();
        unsigned int ret = buf_len;
        data_sync.Unlock();
        return ret;
    }

    const char *get_FormatString(  ) const {
        data_sync.Lock();
        const char * ret = fmt_str;
        data_sync.Unlock();
        return ret;
    }

    const RemoteNode * get_InletNode(  ) const {
        data_sync.Lock();
        const RemoteNode * ret = inlet_node;
        data_sync.Unlock();
        return ret;
    }

    unsigned int get_NumDataElements(  ) const {
        data_sync.Lock();
        unsigned int ret = data_elements.size(  );
        data_sync.Unlock();
        return ret;
    }

    const DataElement * get_DataElement( unsigned int i ) const {
        data_sync.Lock();
        const DataElement * ret = data_elements[i];
        data_sync.Unlock();
        return ret;
    }

    const DataElement * operator[] ( unsigned int i ) const {
        data_sync.Lock();
        const DataElement * ret = data_elements[i];
        data_sync.Unlock();
        return ret;
    }
};

class Packet:public Error
{
    friend class Message;

 private:
    refCounter< PacketData > data;

    Packet( unsigned int _buf_len, char *_buf, const RemoteNode * rn )
        : data( PacketData(_buf_len, _buf, rn ) ) {}

 public:
    static Packet * NullPacket;
    static void initialize_static_stuff() {
        NullPacket = new Packet( 0, NULL, (const RemoteNode*)NULL);
    }

    static void free_static_stuff() {
        delete NullPacket;
        NullPacket = NULL;
    }

    Packet( ): data( NullPacket->data ) { }

    Packet( unsigned short _stream_id, int _tag, const char *fmt, ... )
        :data( NullPacket->data ) {
        va_list arg_list;

        va_start( arg_list, fmt );
        *this = Packet( _stream_id, _tag, fmt, arg_list );
        va_end( arg_list );
        return;
    }

    Packet( unsigned short _stream_id, int _tag, const char *fmt,
            va_list v ) : data( PacketData( _stream_id, _tag, fmt, v) ){}

    Packet(const Packet &src): Error(), data(src.data) { }

    Packet & operator=(const Packet &src) {
        if( this != &src ){
            data = src.data;
        }
        return *this;
    }

    bool operator==(const Packet &src) const {
        const PacketData &me = data.getData();
        const PacketData &them = src.data.getData();
        return (me == them);
    }

    bool operator!=(const Packet &src) const {
        const PacketData &me = data.getData();
        const PacketData &them = src.data.getData();
        return (me != them);
    }

    int ExtractVaList( const char *fmt, va_list arg_list ) const{
        int ret = data.getData().ExtractVaList( fmt, arg_list );
        return ret;
    }

    int ExtractArgList( const char *fmt, ... ) const{
        va_list arg_list;

        va_start( arg_list, fmt );
        int ret = data.getData().ExtractVaList( fmt, arg_list );
        va_end( arg_list );
        return ret;
    }

    int get_Tag(  ) const {
        int ret = data.getData().get_Tag();
        return ret;
    }
    unsigned short get_StreamId(  ) const {
        unsigned short ret = data.getData().get_StreamId();
        return ret;
    }
    const char *get_Buffer(  ) const {
        const char * ret = data.getData().get_Buffer();
        return ret;
    }
    unsigned int get_BufferLen(  ) const {
        unsigned int ret = data.getData().get_BufferLen();
        return ret;
    }
    const char *get_FormatString(  ) const {
        const char * ret = data.getData().get_FormatString();
        return ret;
    }
    const RemoteNode * get_InletNode(  ) const {
        const RemoteNode * ret = data.getData().get_InletNode();
        return ret;
    }
    unsigned int get_NumDataElements(  ) const {
        unsigned int ret = data.getData().get_NumDataElements();
        return ret;
    }
    const DataElement * get_DataElement( unsigned int pos ) const {
        const DataElement * ret = data.getData()[ pos ];
        return ret;
    }
    const DataElement * operator[] ( unsigned int pos ) const {
        const DataElement * ret = data.getData()[ pos ];
        return ret;
    }

    void set_DestroyData( bool b ) {
        data.getData().set_DestroyData( b );
    }
};

class Packet_counter {
 private:
   static int count;
  
 public:
   Packet_counter() {
      if (count++ == 0)
         Packet::initialize_static_stuff();
   }
  ~Packet_counter() {
      if (--count == 0)
         Packet::free_static_stuff();
   }
};
static Packet_counter pc;
}                               /* namespace MRN */
#endif                          /* packet_h */
