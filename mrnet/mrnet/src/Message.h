#if !defined(Message_h)
#define Message_h

#include <list>
#include <vector>

#include <sys/socket.h>
#include <stdarg.h>

#include "mrnet/src/Errors.h"
#include "mrnet/src/pdr.h"

namespace MRN
{

class RemoteNode;
class Packet;
typedef enum {UNKNOWN_T, CHAR_T, UCHAR_T,
	      CHAR_ARRAY_T, UCHAR_ARRAY_T,
	      STRING_T, STRING_ARRAY_T,
	      INT16_T, UINT16_T,
	      INT16_ARRAY_T, UINT16_ARRAY_T,
	      INT32_T, UINT32_T,
	      INT32_ARRAY_T, UINT32_ARRAY_T,
	      INT64_T, UINT64_T,
	      INT64_ARRAY_T, UINT64_ARRAY_T,
	      FLOAT_T, DOUBLE_T,
	      FLOAT_ARRAY_T, DOUBLE_ARRAY_T} DataType;

//typedef enum {TYPE_CTL, TYPE_DATA} PacketType;

typedef union{
    char c;
    unsigned char uc;
    int16_t hd;
    uint16_t uhd;
    int32_t d;
    uint32_t ud;
    int64_t ld;
    uint64_t uld;
    float f;
    double lf;
    void * p; // May need to be allocated by pdr routine
} DataValue;

struct DataElement{
    DataType type;
    DataValue val;
    uint32_t array_len;
};

class Message{
  std::list <Packet *> packets;

 public:
  void add_Packet(Packet *);
  int send(int sock_fd);
  int recv(int sock_fd, std::list <Packet *> &packets, RemoteNode*);
  int size_Packets();
  int size_Bytes();
};

/*******************************************************************************
  Packet Buffer Format:
    __________________________________________
    | streamid | tag | srcstr | fmtstr | data|
    ------------------------------------------
*******************************************************************************/
class Packet: public Error{
 private:
  std::vector <DataElement *> data_elements;

  unsigned short stream_id;
  int tag;                  /* Application/Protocol Level ID */
  char * src;               /* Null Terminated String */
  char * fmt_str;           /* Null Terminated String */
  char * buf;  /* The entire packed/encoded buffer (header+payload)! */
  unsigned int buf_len;
  int ArgList2DataElementArray(va_list arg_list);
  void DataElementArray2ArgList(va_list arg_list);

 public: 
  Packet(int _tag, const char * fmt, ...);
  Packet(unsigned short stream_id, int _tag, const char * fmt, va_list);
  Packet(unsigned int _buf_len, char * _buf);
  Packet(int _tag, unsigned short _stream_id, DataElement *, const char * fmt);

  int ExtractVaList(const char * fmt, va_list arg_list); 
  int ExtractArgList(const char * fmt, ...); 

  static bool_t pdr_packet(PDR *, Packet *);
  int get_Tag();
  int get_StreamId();
  char * get_Buffer();
  unsigned int get_BufferLen();
  const char * get_FormatString();
  unsigned int get_NumDataElements();
  DataElement * get_DataElement(unsigned int);
  RemoteNode * inlet_node;
};

int read(int fd, void * buf, int size);
int readmsg(int fd, struct msghdr *msg);
int write(int fd, const void * buf, int size);

DataType Fmt2Type(const char * cur_fmt);

} // namespace MRN

#endif /* Message_h */
