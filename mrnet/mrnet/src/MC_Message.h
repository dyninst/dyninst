#if !defined(MC_Message_h)
#define MC_Message_h

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdarg.h>

#include "mrnet/src/MC_Errors.h"
#include "mrnet/src/pdr.h"

class MC_RemoteNode;
class MC_Packet;
enum MC_DataTypes{UNKNOWN_T, CHAR_T, UCHAR_T,
                  CHAR_ARRAY_T, UCHAR_ARRAY_T,
                  STRING_T, STRING_ARRAY_T,
                  INT16_T, UINT16_T,
                  INT16_ARRAY_T, UINT16_ARRAY_T,
                  INT32_T, UINT32_T,
                  INT32_ARRAY_T, UINT32_ARRAY_T,
                  INT64_T, UINT64_T,
                  INT64_ARRAY_T, UINT64_ARRAY_T,
                  FLOAT_T, DOUBLE_T,
                  FLOAT_ARRAY_T, DOUBLE_ARRAY_T};
typedef enum MC_DataTypes MC_DataTypes; 

//enum MC_EncodingTypes{ENCODING_NONE, ENCODING_NBO};
//typedef enum MC_EncodingTypes MC_EncodingTypes; 

enum MC_PacketTypes{TYPE_CTL, TYPE_DATA};
typedef enum MC_PacketTypes MC_PacketTypes; 

typedef union{
    char c;
    int d;
    short int hd;
    long int ld;
    float f;
    double lf;
    void * p; // May need to be allocated by pdr routine
} MC_DataValue;

typedef struct{
    MC_DataValue val;
    MC_DataTypes type;
    unsigned int array_len;
}MC_DataElement;


class MC_Message{
  std::list <MC_Packet *> packets;

 public:
  void add_Packet(MC_Packet *);
  int send(int sock_fd);
  int recv(int sock_fd, std::list <MC_Packet *> &packets, MC_RemoteNode*);
  int size_Packets();
  int size_Bytes();
};

/*******************************************************************************
  Packet Buffer Format:
    _____________________________________________________
    | pkt_type | streamid | tag | srcstr | fmtstr | data|
    -----------------------------------------------------
*******************************************************************************/
class MC_Packet: public MC_Error{
 private:
  std::vector <MC_DataElement *> data_elements;

  unsigned short stream_id;
  int tag;                  /* Application/Protocol Level ID */
  char * src;               /* Null Terminated String */
  char * fmt_str;           /* Null Terminated String */
  char * buf;  /* The entire packed/encoded buffer (header+payload)! */
  unsigned int buf_len;
  int ArgList2DataElementArray(va_list arg_list);
  void DataElementArray2ArgList(va_list arg_list);

 public: 
  MC_Packet(int _tag, const char * fmt, ...);
  MC_Packet(unsigned short stream_id, int _tag, const char * fmt, va_list);
  MC_Packet(unsigned int _buf_len, char * _buf);
  MC_Packet(MC_DataElement *, const char * fmt);

  int ExtractVaList(const char * fmt, va_list arg_list); 
  int ExtractArgList(const char * fmt, ...); 

  static bool_t pdr_packet(PDR *, MC_Packet *);
  int get_Tag();
  int get_StreamId();
  char * get_Buffer();
  unsigned int get_BufferLen();
  const char * get_FormatString();
  unsigned int get_NumElements();
  MC_DataElement * get_Element(unsigned int);
  MC_RemoteNode * inlet_node;
};

int MC_read(int fd, void * buf, int size);
int MC_readmsg(int fd, struct msghdr *msg);
int MC_write(int fd, const void * buf, int size);

MC_DataTypes Fmt2Type(const char * cur_fmt);
#endif /* MC_Message_h */
