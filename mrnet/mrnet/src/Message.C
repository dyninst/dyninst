#include "mrnet/src/Types.h"
#include <stdarg.h>
#include <errno.h>

#include "mrnet/src/MC_Message.h"
#include "mrnet/src/utils.h"

void MC_Message::add_Packet(MC_Packet *packet)
{
  assert(packet);
  packets.push_back(packet);
}

int MC_Message::recv(int sock_fd, list <MC_Packet *> &packets, MC_RemoteNode *remote_node)
{
  int i;
  int32_t buf_len;
  uint32_t no_packets=0, *packet_sizes;
  struct msghdr msg;
  char * buf=NULL;
  PDR pdrs;
  enum pdr_op op = PDR_DECODE;

  mc_printf((stderr, "Receiving packets to message (%p)\n", this));

  /* find out how many packets are coming */
  buf_len = pdr_sizeof((pdrproc_t)(pdr_uint32), &no_packets);
  assert(buf_len);
  buf = (char *)malloc( buf_len);
  assert(buf);

  mc_printf((stderr, "Calling MC_read(%d, %p, %d)\n", sock_fd, buf, buf_len));
  int retval;
  if( (retval=MC_read(sock_fd, buf, buf_len)) != buf_len){
    mc_printf((stderr, "MC_read returned %d\n", retval));
    _perror("MC_read()");
    free(buf);
    return -1;
  }

  pdrmem_create(&pdrs, buf, buf_len, op);
  if( !pdr_uint32(&pdrs, &no_packets) ){
    mc_printf((stderr, "pdr_uint32() failed\n"));
    free(buf);
    return -1;
  }
  free(buf);
  mc_printf((stderr, "pdr_uint32() succeeded. Receive %d packets\n", no_packets));

  /* recv an vector of packet_sizes */
  //buf_len's value is hardcode, breaking pdr encapsulation barrier :(
  buf_len = sizeof(uint32_t) * no_packets + 4; // 4 byte pdr overhead
  buf = (char *)malloc( buf_len);
  assert(buf);

  packet_sizes = (uint32_t *)malloc( sizeof(uint32_t) * no_packets);
  assert(packet_sizes);

  mc_printf((stderr, "Calling MC_read(%d, %p, %d) for %d buffer lengths.\n",
             sock_fd, buf, buf_len, no_packets));
  if( MC_read(sock_fd, buf, buf_len) != buf_len){
    mc_printf((stderr, "MC_read() failed\n"));
    free(buf);
    return -1;
  }

  pdrmem_create(&pdrs, buf, buf_len, op);
  if( !pdr_vector(&pdrs, (char *)(packet_sizes), no_packets, sizeof(uint32_t),
                 (pdrproc_t)pdr_uint32) ){
    mc_printf((stderr, "pdr_vector() failed\n"));
    free(buf);
    return -1;
  }
  free(buf);

  /* recv packet buffers */
  msg.msg_name = NULL;
  msg.msg_iov = (struct iovec *)malloc(sizeof(struct iovec)*no_packets);
  assert(msg.msg_iov);
  msg.msg_iovlen = no_packets;
  msg.msg_control = NULL;  /* ancillary data, see below */
  msg.msg_controllen = 0;

  mc_printf((stderr, "Reading %d packets of size: [", no_packets));
  int total_bytes=0;
  for(i=0; i<msg.msg_iovlen; i++){
    msg.msg_iov[i].iov_base = malloc(packet_sizes[i]);
    assert(msg.msg_iov[i].iov_base);
    msg.msg_iov[i].iov_len = packet_sizes[i];
    total_bytes += packet_sizes[i];
    _fprintf((stderr, "%d, ", packet_sizes[i]));
  }
  _fprintf((stderr, "]\n"));

  if(MC_readmsg(sock_fd, &msg) != total_bytes){
    mc_printf((stderr, "%s", ""));
    _perror("MC_readmsg()");
    return -1;
  }

  for(i=0; i<msg.msg_iovlen; i++){
    MC_Packet * new_packet = new MC_Packet(msg.msg_iov[i].iov_len,
					   (char *)msg.msg_iov[i].iov_base);
    if(new_packet->fail()){
      mc_printf((stderr, "packet creation failed\n"));
      return -1;
    }
    new_packet->inlet_node = remote_node;
    packets.push_back(new_packet);
  }

  mc_printf((stderr, "Msg(%p)::recv() succeeded\n", this));
  return 0;
}

int MC_Message::send(int sock_fd)
{
  /* send an array of packet_sizes */
  unsigned int i;
  uint32_t *packet_sizes, no_packets;
  struct iovec *iov;
  uint32_t iovlen;
  char * buf;
  int buf_len;
  PDR pdrs;
  enum pdr_op op = PDR_ENCODE;


  mc_printf((stderr, "Sending packets from message %p\n", this));
  if(packets.size() == 0){  //nothing to do
    mc_printf((stderr, "Nothing to send!\n"));
    return 0;
  }
  no_packets = packets.size();

  /* send packet buffers */
  iov = (struct iovec *)malloc(sizeof(struct iovec)*no_packets);
  assert(iov);
  iovlen = no_packets;

  //Process packets in list to prepare for send()
  packet_sizes = (uint32_t *)malloc( sizeof(uint32_t) * no_packets );
  assert(packet_sizes);
  list<MC_Packet *>::iterator iter=packets.begin();
  mc_printf((stderr, "Writing %d packets of size: [ ", no_packets));
  int total_bytes =0;
  for(i=0; iter != packets.end(); iter++, i++){
    iov[i].iov_base = (*iter)->get_Buffer();
    iov[i].iov_len = (*iter)->get_BufferLen();
    packet_sizes[i] = (*iter)->get_BufferLen();
    total_bytes += iov[i].iov_len;
    _fprintf((stderr, "%d, ", iov[i].iov_len));
  }
  _fprintf((stderr, "]\n"));
  
  /* put how many packets are going */

  buf_len = pdr_sizeof((pdrproc_t)(pdr_uint32), &no_packets);
  assert(buf_len);
  buf = (char *)malloc( buf_len );
  assert(buf);
  pdrmem_create(&pdrs, buf, buf_len, op);


  if( !pdr_uint32(&pdrs, &no_packets) ){
    mc_printf((stderr, "pdr_uint32() failed\n"));
    free(buf);
    return -1;
  }

  mc_printf((stderr, "Calling MC_write(%d, %p, %d)\n", sock_fd, buf, buf_len));
  if( MC_write(sock_fd, buf, buf_len) != buf_len){
    mc_printf((stderr, "%s", ""));
    _perror("MC_write()");
    free(buf);
    return -1;
  }
  free(buf);

  /* send a vector of packet_sizes */
  buf_len = no_packets*sizeof(uint32_t) + 4; //4 extra bytes overhead
  buf = (char *)malloc(buf_len);
  assert(buf);
  pdrmem_create(&pdrs, buf, buf_len, op);

  if( !pdr_vector(&pdrs, (char *)(packet_sizes), no_packets, sizeof(uint32_t),
                 (pdrproc_t)pdr_uint32) ){
    mc_printf((stderr, "pdr_vector() failed\n"));
    free(buf);
    return -1;
  }

  mc_printf((stderr, "Calling MC_write(%d, %p, %d)\n", sock_fd, buf, buf_len));
  if( MC_write(sock_fd, buf, buf_len ) != buf_len){
    mc_printf((stderr, "%s", ""));
    _perror("MC_write()");
    free(buf);
    return -1;
  }
  free(buf);

  mc_printf((stderr, "Calling writev(%d vectors, %d total bytes)\n",
             iovlen, total_bytes));
  int ret;
  if( (ret=writev(sock_fd, iov, iovlen)) != total_bytes){
    mc_printf((stderr, "writev() returned %d of %d bytes\n", ret, total_bytes));
    for(i=0; i<iovlen; i++){
      mc_printf((stderr, "vector[%d].size = %d\n", i, iov[i].iov_len));
    }
    _perror("writev()");
    return -1;
  }

  packets.clear();

  mc_printf((stderr, "msg(%p)::send() succeeded\n", this));
  return 0;
}

int MC_Message::size_Packets()
{
  return packets.size();
}

int MC_Message::size_Bytes()
{
  assert(0);
  return 0;
}
/**************
 * MC_Packet
 **************/
MC_Packet::MC_Packet(int _tag, const char *fmt, ...)
  :tag(_tag)
{
  va_list arg_list;
  PDR pdrs;

  mc_printf((stderr, "In MC_Packet(%p) constructor\n", this));
  fmt_str = strdup(fmt);
  assert(fmt_str);
  src = strdup(getNetworkName().c_str());
  assert(src);

  va_start(arg_list, fmt);
  if( ArgList2DataElementArray(arg_list) == -1){
    mc_printf((stderr, "ArgList2DataElementArray() failed\n"));
    va_end(arg_list);
    mc_errno = MC_EPACKING;
    return;
  }
  va_end(arg_list);

  buf_len = pdr_sizeof((pdrproc_t)(MC_Packet::pdr_packet), this);
  assert(buf_len);
  buf = (char *) malloc (buf_len);
  assert(buf);

  pdrmem_create(&pdrs, buf, buf_len, PDR_ENCODE);
  if(!MC_Packet::pdr_packet(&pdrs, this)){
    mc_printf((stderr, "pdr_packet() failed\n"));
    mc_errno = MC_EPACKING;
    return;
  }

  mc_printf((stderr, "MC_Packet(%p) constructor succeeded\n", this));
  return;
}

MC_Packet::MC_Packet(unsigned short _stream_id, int _tag, const char *fmt, va_list arg_list)
  :stream_id(_stream_id), tag(_tag)
{
  PDR pdrs;
  mc_printf((stderr, "In MC_Packet(%p) constructor\n", this));

  fmt_str = strdup(fmt);
  src = strdup(getNetworkName().c_str());

  if( ArgList2DataElementArray(arg_list) == -1){
    mc_printf((stderr, "ArgList2DataElementArray() failed\n"));
    mc_errno = MC_EPACKING;
    return;
  }

  buf_len = pdr_sizeof((pdrproc_t)(MC_Packet::pdr_packet), this);
  assert(buf_len);
  buf = (char *) malloc (buf_len); 
  assert(buf);

  pdrmem_create(&pdrs, buf, buf_len, PDR_ENCODE);

  if(!MC_Packet::pdr_packet(&pdrs, this)){
    mc_printf((stderr, "pdr_packet() failed\n"));
    mc_errno = MC_EPACKING;
    return;
  }

  mc_printf((stderr, "MC_Packet(%p) constructor succeeded\n", this));
  return;
}

MC_Packet::MC_Packet(unsigned int _buf_len, char * _buf)
 :src(NULL), fmt_str(NULL), buf(_buf), buf_len(_buf_len)
{
  PDR pdrs;
  mc_printf((stderr, "In MC_Packet(%p) constructor\n", this));

  pdrmem_create(&pdrs, buf, buf_len, PDR_DECODE);


  if(!MC_Packet::pdr_packet(&pdrs, this)){
    mc_printf((stderr, "pdr_packet() failed\n"));
    mc_errno = MC_EPACKING;
  }

  mc_printf((stderr, "MC_Packet(%p) constructor succeeded: src:%s, "
                     "tag:%d, fmt:%s\n", this, src, tag, fmt_str));
}

int MC_Packet::ExtractArgList(const char * fmt, ...){
  va_list arg_list;

  mc_printf((stderr, "In ExtractArgList(%p)\n", this));

  if(strcmp(fmt_str,fmt)){
    mc_printf((stderr, "Format string mismatch: %s, %s\n", fmt_str, fmt));
    mc_errno = MC_EFMTSTR_MISMATCH;
    return -1;
  }

  va_start(arg_list, fmt);
  DataElementArray2ArgList(arg_list); 
  va_end(arg_list);

  mc_printf((stderr, "ExtractArgList(%p) succeeded\n", this));
  return 0;
}

int MC_Packet::ExtractVaList(const char * fmt, va_list arg_list){
  mc_printf((stderr, "In ExtractVaList(%p)\n", this));

  if(strcmp(fmt_str,fmt)){
    mc_printf((stderr, "Format string mismatch: %s, %s\n", fmt_str, fmt));
    mc_errno = MC_EFMTSTR_MISMATCH;
    return -1;
  }

  DataElementArray2ArgList(arg_list); 

  mc_printf((stderr, "ExtractVaList(%p) succeeded\n", this));
  return 0;
}

bool_t MC_Packet::pdr_packet(PDR * pdrs, MC_Packet * pkt){
  char *cur_fmt, * fmt, *buf_ptr;
  unsigned int i;
  bool_t retval;
  MC_DataElement * cur_elem;

  mc_printf((stderr, "In pdr_packet. op: %d\n", pdrs->p_op));

  /* Process Packet Header into/out of the pdr mem */
/*******************************************************************************
  Packet Buffer Format:
    ___________________________________________________
    | streamid | tag | srcstr |  fmtstr | packed_data |
    ---------------------------------------------------
*******************************************************************************/
  //mc_printf((stderr, "pdrs->space: %d\n", pdrs->space));
  //mc_printf((stderr, "Calling pdr_uint16()\n"));
  if( pdr_uint16(pdrs, &(pkt->stream_id)) == FALSE){
    mc_printf((stderr, "pdr_uint16() failed\n"));
    return FALSE;
  }
  //mc_printf((stderr, "Calling pdr_uint32()\n"));
  //mc_printf((stderr, "pdrs->space: %d\n", pdrs->space));
  if( pdr_int32(pdrs, &(pkt->tag)) == FALSE){
    mc_printf((stderr, "pdr_uint32() failed\n"));
    return FALSE;
  }
  //mc_printf((stderr, "Calling pdr_wrapstring(%s)\n", pkt->src));
  //mc_printf((stderr, "pdrs->space: %d\n", pdrs->space));
  if( pdr_wrapstring(pdrs, &(pkt->src)) == FALSE){
    mc_printf((stderr, "pdr_wrapstring() failed\n"));
    return FALSE;
  }
  //mc_printf((stderr, "Calling pdr_wrapstring(%s)\n", pkt->fmt_str));
  //mc_printf((stderr, "pdrs->space: %d\n", pdrs->space));
  if( pdr_wrapstring(pdrs, &(pkt->fmt_str)) == FALSE){
    mc_printf((stderr, "pdr_wrapstring() failed\n"));
    return FALSE;
  }

  if(!pkt->get_FormatString()){
    mc_printf((stderr, "No data in message. just header info\n"));
    return TRUE;
  }

  fmt=strdup(pkt->get_FormatString());
  cur_fmt = strtok_r(fmt, " \t\n%", &buf_ptr);
  i=0;

  do{
    if(cur_fmt == NULL){
      break;
    }

    if( pdrs->p_op == PDR_ENCODE ){
      cur_elem = pkt->data_elements[i];
    }
    else if ( pdrs->p_op == PDR_DECODE ){
      cur_elem = new MC_DataElement;
      cur_elem->type = Fmt2Type(string(cur_fmt));
      pkt->data_elements.push_back(cur_elem);
    }
    mc_printf((stderr, "Handling packet[%d], cur_fmt: \"%s\", type: %d\n",
               i, cur_fmt, cur_elem->type));

    switch(cur_elem->type){
    case UNKNOWN_T:
      assert(0);
    case CHAR_T:
    case UCHAR_T:
      retval = pdr_uchar(pdrs, (uchar_t *)(&(cur_elem->val.c)) );
      break;

    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
      if( pdrs->p_op == PDR_DECODE ){
	cur_elem->val.p = NULL;
      }
      retval= pdr_array(pdrs, (char **)(&cur_elem->val.p), &(cur_elem->array_len),
			            INT32_MAX, sizeof(uchar_t), (pdrproc_t)pdr_uchar);
      break;

    case INT16_T:
    case UINT16_T:
      retval = pdr_uint16(pdrs, (uint16_t*)(&(cur_elem->val.d)) );
      break;
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
      if( pdrs->p_op == PDR_DECODE ){
	cur_elem->val.p = NULL;
      }
      retval=pdr_array(pdrs, (char **)(&cur_elem->val.p), &(cur_elem->array_len),
			            INT32_MAX, sizeof(uint16_t), (pdrproc_t)pdr_uint16);
      break;

    case INT32_T:
    case UINT32_T:
      retval = pdr_uint32(pdrs, (uint32_t*)(&(cur_elem->val.d)) );
      break;
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
      if( pdrs->p_op == PDR_DECODE ){
	cur_elem->val.p = NULL;
      }
      retval=pdr_array(pdrs, (char **)(&cur_elem->val.p), &(cur_elem->array_len),
			            INT32_MAX, sizeof(uint32_t), (pdrproc_t)pdr_uint32);
      break;

    case INT64_T:
    case UINT64_T:
      retval = pdr_uint64(pdrs, (uint64_t*)(&(cur_elem->val.d)) );
      break;
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
      if( pdrs->p_op == PDR_DECODE ){
	cur_elem->val.p = NULL;
      }
      retval=pdr_array(pdrs, (char **)(&cur_elem->val.p), &(cur_elem->array_len),
			            INT32_MAX, sizeof(uint64_t), (pdrproc_t)pdr_uint64);
      break;

    case FLOAT_T:
      retval = pdr_float(pdrs, (float *)(&(cur_elem->val.f)) );
      break;
    case DOUBLE_T:
      retval = pdr_double(pdrs, (double *)(&(cur_elem->val.lf)) );
      break;
    case FLOAT_ARRAY_T:
      if( pdrs->p_op == PDR_DECODE ){
	cur_elem->val.p = NULL;
      }
      retval=pdr_array(pdrs, (char **)(&cur_elem->val.p), &(cur_elem->array_len),
			            INT32_MAX, sizeof(float), (pdrproc_t)pdr_float);
      break;
    case DOUBLE_ARRAY_T:
      if( pdrs->p_op == PDR_DECODE ){
	cur_elem->val.p = NULL;
      }
      retval=pdr_array(pdrs, (char **)(&cur_elem->val.p), &(cur_elem->array_len),
			            INT32_MAX, sizeof(double), (pdrproc_t)pdr_double);
      break;
    case STRING_T:
      if( pdrs->p_op == PDR_DECODE ){
	cur_elem->val.p = NULL;
      }
      else{
	//mc_printf((stderr, "ENCODING string %s (%p)\n", (char*)cur_elem->val.p, cur_elem->val.p));
  //mc_printf((stderr, "pdrs->space: %d\n", pdrs->space));
      }
      retval = pdr_wrapstring(pdrs, (char**)&(cur_elem->val.p));
      break;

    }
    if(!retval){
      mc_printf((stderr, "pdr_xxx() failed for elem[%d] of type %d\n",
      i, cur_elem->type));
      return retval;
    }
    cur_fmt = strtok_r(NULL, " \t\n%", &buf_ptr);
    i++;
  }while(cur_fmt != NULL);

  mc_printf((stderr, "pdr_packet() succeeded\n"));
  return TRUE;
}

int MC_Packet::get_Tag()
{
  return tag;
}

int MC_Packet::get_StreamId()
{
  return stream_id;
}

char * MC_Packet::get_Buffer()
{
  return buf;
}

unsigned int MC_Packet::get_BufferLen()
{
  return buf_len;
}

const char * MC_Packet::get_FormatString()
{
  return fmt_str;
}

int MC_Packet::ArgList2DataElementArray(va_list arg_list)
{
  char *cur_fmt, * fmt = strdup(fmt_str), *buf_ptr;
  MC_DataElement * cur_elem;
  
  mc_printf((stderr, "In ArgList2DataElementArray, packet(%p)\n", this));

  cur_fmt = strtok_r(fmt, " \t\n%", &buf_ptr);
  do{
    if(cur_fmt == NULL){
      break;
    }

    cur_elem = new MC_DataElement;
    cur_elem->type = Fmt2Type(cur_fmt);
    //mc_printf((stderr, "Handling new packet, cur_fmt: \"%s\", type: %d\n",
               //cur_fmt, cur_elem->type));
    switch(cur_elem->type){
    case UNKNOWN_T:
      assert(0);
    case CHAR_T:
      cur_elem->val.c = (char)va_arg(arg_list, char);
      break;
    case UCHAR_T:
      cur_elem->val.c = (char)va_arg(arg_list, unsigned char);
      break;

    case INT16_T:
      cur_elem->val.hd = (short int)va_arg(arg_list, short int);
      break;
    case UINT16_T:
      cur_elem->val.hd = (short int)va_arg(arg_list, unsigned short int);
      break;

    case INT32_T:
      cur_elem->val.d = (int)va_arg(arg_list, int);
      break;
    case UINT32_T:
      cur_elem->val.d = (int)va_arg(arg_list, unsigned int);
      break;

    case INT64_T:
      cur_elem->val.ld = (long int)va_arg(arg_list, long int);
      break;
    case UINT64_T:
      cur_elem->val.ld = (long int)va_arg(arg_list, unsigned long int);
      break;

    case FLOAT_T:
      cur_elem->val.f = (float)va_arg(arg_list, float);
      break;
    case DOUBLE_T:
      cur_elem->val.lf = (double)va_arg(arg_list, double);
      break;

    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
      cur_elem->val.p = (void *)va_arg(arg_list, void *);
      cur_elem->array_len = (unsigned int)va_arg(arg_list, unsigned int);
      break;
    case STRING_T:
      cur_elem->val.p = (void *)va_arg(arg_list, void *);
      cur_elem->array_len = strlen( (char*) cur_elem->val.p );
      break;
    default:
      assert(0);
      break;
    }
    data_elements.push_back(cur_elem);
    cur_fmt = strtok_r(NULL, " \t\n%", &buf_ptr);
  }while(cur_fmt != NULL);

  mc_printf((stderr, "ArgList2DataElementArray succeeded, packet(%p)\n", this));
  return 0;
}

void MC_Packet::DataElementArray2ArgList(va_list arg_list)
{
  char *cur_fmt, * fmt = strdup(fmt_str), *buf_ptr;
  int i=0;
  MC_DataElement * cur_elem;
  void * tmp_ptr;
  
  mc_printf((stderr, "In DataElementArray2ArgList, packet(%p)\n", this));
  cur_fmt = strtok_r(fmt, " \t\n%", &buf_ptr);
  do{
    if(cur_fmt == NULL){
      break;
    }

    cur_elem = data_elements[i];
    assert(cur_elem->type == Fmt2Type(cur_fmt));
    //mc_printf((stderr, "packet[%d], cur_fmt: \"%s\", cur_type: %d\n",
               //i, cur_fmt, cur_elem->type));
    switch(cur_elem->type){
    case UNKNOWN_T:
      assert(0);
    case CHAR_T:
      tmp_ptr = (void *)va_arg(arg_list, char *);
      *((char *)tmp_ptr) = cur_elem->val.c;
      break;
    case UCHAR_T:
      tmp_ptr = (void *)va_arg(arg_list, unsigned char *);
      *((unsigned char *)tmp_ptr) = cur_elem->val.c;
      break;

    case INT16_T:
      tmp_ptr = (void *)va_arg(arg_list, short int *);
      *((short int *)tmp_ptr) = cur_elem->val.hd;
      break;
    case UINT16_T:
      tmp_ptr = (void *)va_arg(arg_list, unsigned short int *);
      *((unsigned short int *)tmp_ptr) = cur_elem->val.hd;
      break;

    case INT32_T:
      tmp_ptr = (void *)va_arg(arg_list, int *);
      *((int *)tmp_ptr) = cur_elem->val.d;
      break;
    case UINT32_T:
      tmp_ptr = (void *)va_arg(arg_list, unsigned int *);
      *((unsigned int *)tmp_ptr) = cur_elem->val.d;
      break;

    case INT64_T:
      tmp_ptr = (void *)va_arg(arg_list, long int *);
      *((long int *)tmp_ptr) = cur_elem->val.ld;
      break;
    case UINT64_T:
      tmp_ptr = (void *)va_arg(arg_list, unsigned long int *);
      *((unsigned long int *)tmp_ptr) = cur_elem->val.ld;
      break;

    case FLOAT_T:
      tmp_ptr = (void *)va_arg(arg_list, float *);
      *((float *)tmp_ptr) = cur_elem->val.f;
      break;
    case DOUBLE_T:
      tmp_ptr = (void *)va_arg(arg_list, double *);
      *((double *)tmp_ptr) = cur_elem->val.lf;
      break;

    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
      tmp_ptr = (void *)va_arg(arg_list, void **);
      *((void **)tmp_ptr) = cur_elem->val.p;
      tmp_ptr = (void *)va_arg(arg_list, int *);
      *((int *)tmp_ptr) = cur_elem->array_len;
      break;
    case STRING_T:
      //mc_printf((stderr, "Extracting %s\n", (char *)cur_elem->val.p));
      tmp_ptr = (void *)va_arg(arg_list, char **);
      *((char **)tmp_ptr) = (char *)cur_elem->val.p;
      break;
    default:
      assert(0);
    }
    i++;
    cur_fmt = strtok_r(NULL, " \t\n%", &buf_ptr);
  }while(cur_fmt != NULL);

  mc_printf((stderr, "DataElementArray2ArgList succeeded, packet(%p)\n", this));
  return;
}
unsigned int MC_Packet::get_NumElements()
{
  return data_elements.size();
}

MC_DataElement * MC_Packet::get_Element(unsigned int i)
{
  assert(i > 0 && i< data_elements.size());
  return data_elements[i];
}

/*********************************************************
 *  Converts fmt_string to enumerated type MC_DataTypes
 *********************************************************/
MC_DataTypes Fmt2Type(string cur_fmt)
{
  if( cur_fmt == "c" )
    return CHAR_T;
  else if( cur_fmt == "uc" )
    return UCHAR_T;
  else if( cur_fmt == "ac" )
    return CHAR_ARRAY_T;
  else if( cur_fmt == "auc" )
    return UCHAR_ARRAY_T;
  else if( cur_fmt == "hd" )
    return INT16_T;
  else if( cur_fmt == "uhd" )
    return UINT16_T;
  else if( cur_fmt == "d" )
    return INT32_T;
  else if( cur_fmt == "ud" )
    return UINT32_T;
  else if( cur_fmt == "ahd" )
    return INT16_ARRAY_T;
  else if( cur_fmt == "ld" )
    return INT64_T;
  else if( cur_fmt == "uld" )
    return UINT64_T;
  else if( cur_fmt == "auhd" )
    return UINT16_ARRAY_T;
  else if( cur_fmt == "ad" )
    return INT32_ARRAY_T;
  else if( cur_fmt == "aud" )
    return UINT32_ARRAY_T;
  else if( cur_fmt == "ald" )
    return INT64_ARRAY_T;
  else if( cur_fmt == "auld" )
    return UINT64_ARRAY_T;
  else if( cur_fmt == "f" )
    return FLOAT_T;
  else if( cur_fmt == "af" )
    return FLOAT_ARRAY_T;
  else if( cur_fmt == "lf" )
    return DOUBLE_T;
  else if( cur_fmt == "alf" )
    return DOUBLE_ARRAY_T;
  else if( cur_fmt == "s" )
    return STRING_T;
  else
    return UNKNOWN_T;
}

/*********************************************************
 *  Functions used to implement sending and recieving of
 *  some basic data types
 *********************************************************/

int MC_write(int fd, const void *buf, int count)
{
  //should do a recursive call checking for syscall interuption
  return send(fd, buf, count, 0);
}

int MC_read(int fd, void *buf, int count)
{
  int bytes_recvd=0, retval;
  while(bytes_recvd != count){
    retval = recv(fd, ((char*)buf)+bytes_recvd, count-bytes_recvd, MSG_WAITALL);

    if(retval == -1){
      if(errno == EINTR){
	continue;
      }
      else{
        return -1;
      }
    }
    else{
      bytes_recvd += retval;
      if(bytes_recvd < count && errno == EINTR){
	continue;
      }
      else{
	//bytes_recvd is either count, or error other than "eintr" occured
	if(bytes_recvd != count){
	  mc_printf((stderr, "premature return from MC_read(). Got %d of %d "
                             " bytes. errno: %d\n", bytes_recvd, count, errno));
	}
	return bytes_recvd;
      }
    }
  }
  assert(0);
  return -1;
}

int MC_readmsg(int fd, struct msghdr *msg){
  //should do a recursive call checking for syscall interuption
  return recvmsg(fd, msg, MSG_WAITALL); 
}
