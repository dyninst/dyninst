#if !defined(packet_h)
#define packet_h 1

#include <stdarg.h>
#include <vector>

#include "mrnet/src/DataElement.h"
#include "mrnet/src/Errors.h"
#include "mrnet/src/pdr.h"
#include "mrnet/src/refCounter.h"

namespace MRN
{
class RemoteNode;

/************************************************************************
  Packet Buffer Format:
    __________________________________________
    | streamid | tag | srcstr | fmtstr | data|
    ------------------------------------------
**************************************************************************/
class PacketData: public Error{
 private:
    std::vector < DataElement >data_elements;
    unsigned short stream_id;
    int tag;                /* Application/Protocol Level ID */
    char *src;              /* Null Terminated String */
    char *fmt_str;          /* Null Terminated String */
    char *buf;              /* The entire packed buffer (header+payload)! */
    unsigned int buf_len;

    RemoteNode *inlet_node;

    void ArgList2DataElementArray( va_list arg_list );
    void DataElementArray2ArgList( va_list arg_list );

 public:
    PacketData( unsigned short _stream_id, int _tag, const char *fmt,
            va_list );
    PacketData( unsigned int _buf_len, char *_buf );
    PacketData(const PacketData&);
    PacketData& operator=(const PacketData&);
    ~PacketData();
    bool operator==(const PacketData&)const;
    bool operator!=(const PacketData&)const;

    int ExtractVaList( const char *fmt, va_list arg_list );

    static bool_t pdr_packet( PDR *, PacketData * );

    int PacketData::get_Tag(  ) {
        return tag;
    }

    unsigned short PacketData::get_StreamId(  ) {
        return stream_id;
    }

    char *PacketData::get_Buffer(  ) {
        return buf;
    }

    unsigned int PacketData::get_BufferLen(  ) {
        return buf_len;
    }

    const char *PacketData::get_FormatString(  ) {
        return fmt_str;
    }

    RemoteNode * PacketData::get_InletNode(  ) {
        return inlet_node;
    }

    void PacketData::set_InletNode( RemoteNode * rn ) {
        inlet_node = rn;
    }

    unsigned int PacketData::get_NumDataElements(  ) {
        return data_elements.size(  );
    }

    DataElement & PacketData::get_DataElement( unsigned int i ) {
        return data_elements[i];
    }

    DataElement & PacketData::operator[] ( unsigned int i ) {
        return data_elements[i];
    }
};

class Packet:public Error
{
 private:
    refCounter< PacketData > data;

 public:
    static Packet * NullPacket;
    static void initialize_static_stuff() {
        NullPacket = new Packet( 0, NULL );
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
        //refCounter<PacketData> newRefCtr( pdata );
        //data = newRefCtr;
        return;
    }

    Packet( unsigned short _stream_id, int _tag, const char *fmt,
            va_list v ) : data( PacketData( _stream_id, _tag, fmt, v) ){}

    Packet( unsigned int _buf_len, char *_buf )
        : data( PacketData(_buf_len, _buf) ) {}

    Packet(const Packet &src): data(src.data) { }
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

    int ExtractVaList( const char *fmt, va_list arg_list ){
        return data.getData().ExtractVaList( fmt, arg_list );
    }

    int ExtractArgList( const char *fmt, ... ){
        va_list arg_list;

        va_start( arg_list, fmt );
        int ret = data.getData().ExtractVaList( fmt, arg_list );
        va_end( arg_list );
        return ret;
    }

    int get_Tag(  ){
        return data.getData().get_Tag();
    }
    unsigned short get_StreamId(  ){
        return data.getData().get_StreamId();
    }
    char *get_Buffer(  ){
        return data.getData().get_Buffer();
    }
    unsigned int get_BufferLen(  ){
        return data.getData().get_BufferLen();
    }
    const char *get_FormatString(  ){
        return data.getData().get_FormatString();
    }
    RemoteNode * get_InletNode(  ){
        return data.getData().get_InletNode();
    }
    void set_InletNode( RemoteNode *rn  ){
        return data.getData().set_InletNode( rn );
    }
    unsigned int get_NumDataElements(  ){
        return data.getData().get_NumDataElements();
    }
    DataElement & get_DataElement( unsigned int pos ){
        return data.getData()[ pos ];
    }
    DataElement & operator[] ( unsigned int pos ){
        return data.getData()[ pos ];
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
