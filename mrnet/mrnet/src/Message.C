#include "mrnet/src/Types.h"
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <poll.h>

#include "mrnet/src/Message.h"
#include "mrnet/src/utils.h"

void Message::add_Packet(Packet *packet)
{
  assert(packet);
  packets.push_back(packet);
}

int Message::recv(int sock_fd, std::list <Packet *> &packets_in,
                     RemoteNode *remote_node)
{
  int i;
  int32_t buf_len;
  uint32_t no_packets=0, *packet_sizes;
  struct msghdr msg;
  char * buf=NULL;
  PDR pdrs;
  enum pdr_op op = PDR_DECODE;


  mrn_printf(3, MCFL, stderr, "Receiving packets to message (%p)\n", this);

    //
    // packet count
    //

  /* find out how many packets are coming */
  buf_len = pdr_sizeof((pdrproc_t)(pdr_uint32), &no_packets);
  assert(buf_len);
  buf = (char *)malloc( buf_len);
  assert(buf);

  mrn_printf(3, MCFL, stderr, "Calling read(%d, %p, %d)\n", sock_fd, buf, buf_len);
  int retval;
  if( (retval=read(sock_fd, buf, buf_len)) != buf_len){
    mrn_printf(3, MCFL, stderr, "read returned %d\n", retval);
    _perror("read()");
    free(buf);
    if( errno == 0 ){
        return 0;
    }
    else{
        return -1;
    }
  }


    //
    // pdrmem initialize
    //

  pdrmem_create(&pdrs, buf, buf_len, op);
  if( !pdr_uint32(&pdrs, &no_packets) ){
    mrn_printf(1, MCFL, stderr, "pdr_uint32() failed\n");
    free(buf);
    return -1;
  }
  free(buf);
  mrn_printf(3, MCFL, stderr, "pdr_uint32() succeeded. Receive %d packets\n", no_packets);

  if( no_packets == 0 )
  {
    fprintf(stderr, "warning: Receiving %d packets\n", no_packets);
  }


    //
    // packet size vector
    //

  /* recv an vector of packet_sizes */
  //buf_len's value is hardcode, breaking pdr encapsulation barrier :(
  buf_len = sizeof(uint32_t) * no_packets + 4; // 4 byte pdr overhead
  buf = (char *)malloc( buf_len);
  assert(buf);


  packet_sizes = (uint32_t *)malloc( sizeof(uint32_t) * no_packets);
  if( packet_sizes == NULL )
  {
        fprintf( stderr, "recv: packet_size malloc is NULL for %d packets\n",
            no_packets );
  }
  assert(packet_sizes);

  mrn_printf(3, MCFL, stderr, "Calling read(%d, %p, %d) for %d buffer lengths.\n",
             sock_fd, buf, buf_len, no_packets);
    int readRet = read(sock_fd, buf, buf_len);
  if( readRet != buf_len )
  {
    mrn_printf(1, MCFL, stderr, "read() failed\n");
    free(buf);
    if( errno == 0 ){
        return 0;
    }
    else{
        return -1;
    }
  }


    //
    // packets
    //

  pdrmem_create(&pdrs, buf, buf_len, op);
  if( !pdr_vector(&pdrs, (char *)(packet_sizes), no_packets, sizeof(uint32_t),
                 (pdrproc_t)pdr_uint32) ){
    mrn_printf(1, MCFL, stderr, "pdr_vector() failed\n");
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

  mrn_printf(3, MCFL, stderr, "Reading %d packets of size: [", no_packets);
  int total_bytes=0;
  for(i=0; i<msg.msg_iovlen; i++){
    msg.msg_iov[i].iov_base = malloc(packet_sizes[i]);
    assert(msg.msg_iov[i].iov_base);
    msg.msg_iov[i].iov_len = packet_sizes[i];
    total_bytes += packet_sizes[i];
    _fprintf((stderr, "%d, ", packet_sizes[i]));
  }
  _fprintf((stderr, "]\n"));

  if(readmsg(sock_fd, &msg) != total_bytes){
    mrn_printf(1, MCFL, stderr, "%s", "");
    _perror("readmsg()");
    return -1;
  }

    //
    // post-processing
    //

  for(i=0; i<msg.msg_iovlen; i++){
    Packet * new_packet = new Packet(msg.msg_iov[i].iov_len,
					   (char *)msg.msg_iov[i].iov_base);
    if(new_packet->fail()){
      mrn_printf(1, MCFL, stderr, "packet creation failed\n");
      return -1;
    }
    new_packet->inlet_node = remote_node;
    packets_in.push_back(new_packet);
  }

  mrn_printf(3, MCFL, stderr, "Msg(%p)::recv() succeeded\n", this);
  return 0;
}

int Message::send(int sock_fd)
{
  /* send an array of packet_sizes */
  unsigned int i;
  uint32_t *packet_sizes, no_packets;
  struct iovec *iov;
  char * buf;
  int buf_len;
  PDR pdrs;
  enum pdr_op op = PDR_ENCODE;


  mrn_printf(3, MCFL, stderr, "Sending packets from message %p\n", this);
  if(packets.size() == 0){  //nothing to do
    mrn_printf(3, MCFL, stderr, "Nothing to send!\n");
    return 0;
  }
  no_packets = packets.size();

  /* send packet buffers */
  iov = (struct iovec *)malloc(sizeof(struct iovec)*no_packets);
  assert(iov);

  //Process packets in list to prepare for send()
  packet_sizes = (uint32_t *)malloc( sizeof(uint32_t) * no_packets );
  assert(packet_sizes);
  std::list<Packet *>::iterator iter=packets.begin();
  mrn_printf(3, MCFL, stderr, "Writing %d packets of size: [ ", no_packets);
  int total_bytes =0;
  for( i=0; iter != packets.end(); iter++, i++){

      Packet* curPacket = *iter;

    iov[i].iov_base = curPacket->get_Buffer();
    iov[i].iov_len = curPacket->get_BufferLen();
    packet_sizes[i] = curPacket->get_BufferLen();

    total_bytes += iov[i].iov_len;
    _fprintf((stderr, "%d, ", (int)iov[i].iov_len));
  }
  _fprintf((stderr, "]\n"));
  
  /* put how many packets are going */

  buf_len = pdr_sizeof((pdrproc_t)(pdr_uint32), &no_packets);
  assert(buf_len);
  buf = (char *)malloc( buf_len );
  assert(buf);
  pdrmem_create(&pdrs, buf, buf_len, op);


  if( !pdr_uint32(&pdrs, &no_packets) ){
    mrn_printf(1, MCFL, stderr, "pdr_uint32() failed\n");
    fprintf(stderr, "pdr_uint32() failed\n");
    free(buf);
    return -1;
  }


  mrn_printf(3, MCFL, stderr, "Calling write(%d, %p, %d)\n", sock_fd, buf, buf_len);
  if( write(sock_fd, buf, buf_len) != buf_len){
    mrn_printf(3, MCFL, stderr, "%s", "");
    fprintf(stderr, "write failed: %s", "");
    _perror("write()");
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
    mrn_printf(1, MCFL, stderr, "pdr_vector() failed\n");
    fprintf(stderr, "pdr_vector() failed\n");
    free(buf);
    return -1;
  }

  mrn_printf(3, MCFL, stderr, "Calling write(%d, %p, %d)\n", sock_fd, buf, buf_len);
  int mcwret = write(sock_fd, buf, buf_len);
  if( mcwret != buf_len){
    mrn_printf(1, MCFL, stderr, "%s", "");
    fprintf(stderr, "write (2) failed %s", "");
    _perror("write()");
    free(buf);
    return -1;
  }
  free(buf);


    if( no_packets > IOV_MAX )
    {
        fprintf( stderr, "splitting writev\n" );
    }

    uint32_t nPacketsLeftToSend = no_packets;
    struct iovec* currIov = iov;
    while( nPacketsLeftToSend > 0 )
    {
        uint32_t iovlen = 
            (nPacketsLeftToSend > IOV_MAX) ? IOV_MAX : nPacketsLeftToSend;

        // count the bytes in the packets to be sent
        int nBytesToSend = 0;
        for( i = 0; i < iovlen; i++ )
        {
            nBytesToSend += currIov[i].iov_len;
        }

        mrn_printf(3, MCFL, stderr, "Calling writev(%d vectors, %d total bytes)\n",
                    iovlen, nBytesToSend);

        int ret = writev(sock_fd, currIov, iovlen);
        if( ret != nBytesToSend )
        {
            mrn_printf(3, MCFL, stderr,
                "writev() returned %d of %d bytes\n", ret, nBytesToSend);
            fprintf(stderr, "writev() returned %d of %d bytes, errno = %d, iovlen = %d\n", ret, nBytesToSend, errno, iovlen );
            for(i=0; i<iovlen; i++){
              mrn_printf(3, MCFL, stderr, "vector[%d].size = %d\n",
                    i, currIov[i].iov_len);
              fprintf(stderr, "vector[%d].size = %d\n", i, (int)currIov[i].iov_len);
            }
            _perror("writev()");
            return -1;
        }

        // advance
        nPacketsLeftToSend -= iovlen;
        currIov += iovlen;
    }

  packets.clear();


  mrn_printf(3, MCFL, stderr, "msg(%p)::send() succeeded\n", this);
  return 0;
}

int Message::size_Packets()
{
  return packets.size();
}

int Message::size_Bytes()
{
  assert(0);
  return 0;
}
/**************
 * Packet
 **************/
Packet::Packet(int _tag, const char *fmt, ...)
  :tag(_tag)
{
  va_list arg_list;
  PDR pdrs;

  mrn_printf(3, MCFL, stderr, "In Packet(%p) constructor\n", this);
  fmt_str = strdup(fmt);
  assert(fmt_str);
  src = strdup(getNetworkName().c_str());
  assert(src);

  va_start(arg_list, fmt);
  if( ArgList2DataElementArray(arg_list) == -1){
    mrn_printf(1, MCFL, stderr, "ArgList2DataElementArray() failed\n");
    va_end(arg_list);
    MRN_errno = MRN_EPACKING;
    return;
  }
  va_end(arg_list);

  buf_len = pdr_sizeof((pdrproc_t)(Packet::pdr_packet), this);
  assert(buf_len);
  buf = (char *) malloc (buf_len);
  assert(buf);

  pdrmem_create(&pdrs, buf, buf_len, PDR_ENCODE);
  if(!Packet::pdr_packet(&pdrs, this)){
    mrn_printf(1, MCFL, stderr, "pdr_packet() failed\n");
    MRN_errno = MRN_EPACKING;
    return;
  }

  mrn_printf(3, MCFL, stderr, "Packet(%p) constructor succeeded\n", this);
  return;
}

Packet::Packet(unsigned short _stream_id, int _tag, const char *fmt, va_list arg_list)
  :stream_id(_stream_id), tag(_tag)
{
  PDR pdrs;
  mrn_printf(3, MCFL, stderr, "In Packet(%p) constructor\n", this);

  fmt_str = strdup(fmt);

  std::string tmp = getNetworkName();
  src = strdup(tmp.c_str());
  //src = strdup(getNetworkName().c_str());

  if( ArgList2DataElementArray(arg_list) == -1){
    mrn_printf(1, MCFL, stderr, "ArgList2DataElementArray() failed\n");
    MRN_errno = MRN_EPACKING;
    return;
  }

  buf_len = pdr_sizeof((pdrproc_t)(Packet::pdr_packet), this);
  assert(buf_len);
  buf = (char *) malloc (buf_len); 
  assert(buf);

  pdrmem_create(&pdrs, buf, buf_len, PDR_ENCODE);

  if(!Packet::pdr_packet(&pdrs, this)){
    mrn_printf(1, MCFL, stderr, "pdr_packet() failed\n");
    MRN_errno = MRN_EPACKING;
    return;
  }

  mrn_printf(3, MCFL, stderr, "Packet(%p) constructor succeeded\n", this);
  return;
}

Packet::Packet(unsigned int _buf_len, char * _buf)
 :src(NULL), fmt_str(NULL), buf(_buf), buf_len(_buf_len)
{
  PDR pdrs;
  mrn_printf(3, MCFL, stderr, "In Packet(%p) constructor\n", this);

  pdrmem_create(&pdrs, buf, buf_len, PDR_DECODE);


  if(!Packet::pdr_packet(&pdrs, this)){
    mrn_printf(1, MCFL, stderr, "pdr_packet() failed\n");
    MRN_errno = MRN_EPACKING;
  }

  mrn_printf(3, MCFL, stderr, "Packet(%p) constructor succeeded: src:%s, "
                     "tag:%d, fmt:%s\n", this, src, tag, fmt_str);
}

Packet::Packet(int _tag,
                     unsigned short _sid,
                     DataElement *_data_elements, const char * _fmt_str)
  : stream_id(_sid), tag(_tag),
    src(strdup("<agg>")),
    fmt_str(strdup(_fmt_str))
{
  char *cur_fmt, * fmt = strdup(_fmt_str), *buf_ptr;
  int i=0;
  DataElement * cur_elem;
  
  mrn_printf(3, MCFL, stderr, "In Packet, packet(%p)\n", this);
  cur_fmt = strtok_r(fmt, " \t\n%", &buf_ptr);
  do{
    if(cur_fmt == NULL){
      break;
    }

    cur_elem = &(_data_elements[i++]);
    assert(cur_elem->type == Fmt2Type(cur_fmt));
    data_elements.push_back(cur_elem);

    cur_fmt = strtok_r(NULL, " \t\n%", &buf_ptr);
  }while(cur_fmt != NULL);

  // data_elements copied, now pack the message
  PDR pdrs;
  buf_len = pdr_sizeof((pdrproc_t)(Packet::pdr_packet), this);
  assert(buf_len);
  buf = (char *) malloc (buf_len); 
  assert(buf);

  pdrmem_create(&pdrs, buf, buf_len, PDR_ENCODE);

  if(!Packet::pdr_packet(&pdrs, this)){
    mrn_printf(1, MCFL, stderr, "pdr_packet() failed\n");
    MRN_errno = MRN_EPACKING;
    return;
  }

  mrn_printf(3, MCFL, stderr, "Packet succeeded, packet(%p)\n", this);
  return;
}

int Packet::ExtractArgList(const char * fmt, ...){
  va_list arg_list;

  mrn_printf(3, MCFL, stderr, "In ExtractArgList(%p)\n", this);

  if(strcmp(fmt_str,fmt)){
    mrn_printf(1, MCFL, stderr, "Format string mismatch: %s, %s\n", fmt_str, fmt);
    MRN_errno = MRN_EFMTSTR_MISMATCH;
    return -1;
  }

  va_start(arg_list, fmt);
  DataElementArray2ArgList(arg_list); 
  va_end(arg_list);

  mrn_printf(3, MCFL, stderr, "ExtractArgList(%p) succeeded\n", this);
  return 0;
}

int Packet::ExtractVaList(const char * fmt, va_list arg_list){
  mrn_printf(3, MCFL, stderr, "In ExtractVaList(%p)\n", this);

  if(strcmp(fmt_str,fmt)){
    mrn_printf(1, MCFL, stderr, "Format string mismatch: %s, %s\n", fmt_str, fmt);
    MRN_errno = MRN_EFMTSTR_MISMATCH;
    return -1;
  }

  DataElementArray2ArgList(arg_list); 

  mrn_printf(3, MCFL, stderr, "ExtractVaList(%p) succeeded\n", this);
  return 0;
}

bool_t Packet::pdr_packet(PDR * pdrs, Packet * pkt){
  char *cur_fmt, * fmt, *buf_ptr;
  unsigned int i;
  bool_t retval=0;
  DataElement * cur_elem;

  mrn_printf(3, MCFL, stderr, "In pdr_packet. op: %d\n", pdrs->p_op);

  /* Process Packet Header into/out of the pdr mem */
/*******************************************************************************
  Packet Buffer Format:
    ___________________________________________________
    | streamid | tag | srcstr |  fmtstr | packed_data |
    ---------------------------------------------------
*******************************************************************************/
  //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
  //printf(3, MCFL, stderr, "Calling pdr_uint16()\n");
  if( pdr_uint16(pdrs, &(pkt->stream_id)) == FALSE){
    mrn_printf(1, MCFL, stderr, "pdr_uint16() failed\n");
    return FALSE;
  }
  //printf(1, MCFL, stderr, "Calling pdr_uint32()\n");
  //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
  if( pdr_int32(pdrs, &(pkt->tag)) == FALSE){
    mrn_printf(1, MCFL, stderr, "pdr_uint32() failed\n");
    return FALSE;
  }
  //printf(3, MCFL, stderr, "Calling pdr_wrapstring(%s)\n", pkt->src);
  //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
  if( pdr_wrapstring(pdrs, &(pkt->src)) == FALSE){
    mrn_printf(1, MCFL, stderr, "pdr_wrapstring() failed\n");
    return FALSE;
  }
  //printf(3, MCFL, stderr, "Calling pdr_wrapstring(%s)\n", pkt->fmt_str);
  //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
  if( pdr_wrapstring(pdrs, &(pkt->fmt_str)) == FALSE){
    mrn_printf(1, MCFL, stderr, "pdr_wrapstring() failed\n");
    return FALSE;
  }

  if(!pkt->get_FormatString()){
    mrn_printf(3, MCFL, stderr, "No data in message. just header info\n");
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
      cur_elem = new DataElement;
      cur_elem->type = Fmt2Type(cur_fmt);
      pkt->data_elements.push_back(cur_elem);
    }
    mrn_printf(3, MCFL, stderr, "Handling packet[%d], cur_fmt: \"%s\", type: %d\n",
               i, cur_fmt, cur_elem->type);

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
      mrn_printf(3, MCFL, stderr, "floats value: %f\n", cur_elem->val.f);
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
	//printf(3, MCFL, stderr, "ENCODING string %s (%p)\n", (char*)cur_elem->val.p, cur_elem->val.p);
  //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
      }
      retval = pdr_wrapstring(pdrs, (char**)&(cur_elem->val.p));
      break;

    case STRING_ARRAY_T:
        if( pdrs->p_op == PDR_DECODE )
        {
            cur_elem->val.p = NULL;
        }
        retval = pdr_array( pdrs, (char**)(&cur_elem->val.p),
                                    &(cur_elem->array_len),
                                    INT32_MAX,
                                    sizeof(char*),
                                    (pdrproc_t)pdr_wrapstring );
        break;

    }
    if(!retval){
      mrn_printf(1, MCFL, stderr, "pdr_xxx() failed for elem[%d] of type %d\n",
      i, cur_elem->type);
      return retval;
    }
    cur_fmt = strtok_r(NULL, " \t\n%", &buf_ptr);
    i++;
  }while(cur_fmt != NULL);

  mrn_printf(3, MCFL, stderr, "pdr_packet() succeeded\n");
  return TRUE;
}

int Packet::get_Tag()
{
  return tag;
}

int Packet::get_StreamId()
{
  return stream_id;
}

char * Packet::get_Buffer()
{
  return buf;
}

unsigned int Packet::get_BufferLen()
{
  return buf_len;
}

const char * Packet::get_FormatString()
{
  return fmt_str;
}

int Packet::ArgList2DataElementArray(va_list arg_list)
{
  char *cur_fmt, * fmt = strdup(fmt_str), *buf_ptr;
  DataElement * cur_elem;
  
  mrn_printf(3, MCFL, stderr, "In ArgList2DataElementArray, packet(%p)\n", this);

  cur_fmt = strtok_r(fmt, " \t\n%", &buf_ptr);
  do{
    if(cur_fmt == NULL){
      break;
    }

    cur_elem = new DataElement;
    cur_elem->type = Fmt2Type(cur_fmt);
    mrn_printf(3, MCFL, stderr, "Handling new packet, cur_fmt: \"%s\", type: %d\n",
               cur_fmt, cur_elem->type);
    switch(cur_elem->type){
    case UNKNOWN_T:
      assert(0);
    case CHAR_T:
      cur_elem->val.c = (char)va_arg(arg_list, int);
      break;
    case UCHAR_T:
      cur_elem->val.c = (char)va_arg(arg_list, unsigned int);
      break;

    case INT16_T:
      cur_elem->val.hd = (short int)va_arg(arg_list, int);
      break;
    case UINT16_T:
      cur_elem->val.hd = (short int)va_arg(arg_list, unsigned int);
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
      cur_elem->val.f = (float)va_arg(arg_list, double);
      mrn_printf(3, MCFL, stderr, "floats value: %f\n", cur_elem->val.f);
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
    case STRING_ARRAY_T:
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

  mrn_printf(3, MCFL, stderr, "ArgList2DataElementArray succeeded, packet(%p)\n", this);
  return 0;
}

void Packet::DataElementArray2ArgList(va_list arg_list)
{
  char *cur_fmt, * fmt = strdup(fmt_str), *buf_ptr;
  int i=0;
  DataElement * cur_elem;
  void * tmp_ptr;
  
  mrn_printf(3, MCFL, stderr, "In DataElementArray2ArgList, packet(%p)\n", this);
  cur_fmt = strtok_r(fmt, " \t\n%", &buf_ptr);
  do{
    if(cur_fmt == NULL){
      break;
    }

    cur_elem = data_elements[i];
    assert(cur_elem->type == Fmt2Type(cur_fmt));
    //printf(3, MCFL, stderr, "packet[%d], cur_fmt: \"%s\", cur_type: %d\n",
               //i, cur_fmt, cur_elem->type);
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
    case STRING_ARRAY_T:
      tmp_ptr = (void *)va_arg(arg_list, void **);
      assert( tmp_ptr != NULL );
      *((void **)tmp_ptr) = cur_elem->val.p;
      tmp_ptr = (void *)va_arg(arg_list, int *);
      assert( tmp_ptr != NULL );
      *((int *)tmp_ptr) = cur_elem->array_len;
      break;
    case STRING_T:
      //printf(3, MCFL, stderr, "Extracting %s\n", (char *)cur_elem->val.p);
      tmp_ptr = (void *)va_arg(arg_list, char **);
      *((char **)tmp_ptr) = (char *)cur_elem->val.p;
      break;
    default:
      assert(0);
    }
    i++;
    cur_fmt = strtok_r(NULL, " \t\n%", &buf_ptr);
  }while(cur_fmt != NULL);

  mrn_printf(3, MCFL, stderr, "DataElementArray2ArgList succeeded, packet(%p)\n", this);
  return;
}
unsigned int Packet::get_NumElements()
{
  return data_elements.size();
}

DataElement * Packet::get_Element(unsigned int i)
{
  //assert(i > -1 && i< data_elements.size());
  return data_elements[i];
}

/*********************************************************
 *  Converts fmt_string to enumerated type DataTypes
 *********************************************************/
DataTypes Fmt2Type(const char * cur_fmt)
{
  if( !strcmp(cur_fmt, "c") )
    return CHAR_T;
  else if( !strcmp(cur_fmt, "uc") )
    return UCHAR_T;
  else if( !strcmp(cur_fmt, "ac") )
    return CHAR_ARRAY_T;
  else if( !strcmp(cur_fmt, "auc") )
    return UCHAR_ARRAY_T;
  else if( !strcmp(cur_fmt, "hd") )
    return INT16_T;
  else if( !strcmp(cur_fmt, "uhd") )
    return UINT16_T;
  else if( !strcmp(cur_fmt, "d") )
    return INT32_T;
  else if( !strcmp(cur_fmt, "ud") )
    return UINT32_T;
  else if( !strcmp(cur_fmt, "ahd") )
    return INT16_ARRAY_T;
  else if( !strcmp(cur_fmt, "ld") )
    return INT64_T;
  else if( !strcmp(cur_fmt, "uld") )
    return UINT64_T;
  else if( !strcmp(cur_fmt, "auhd") )
    return UINT16_ARRAY_T;
  else if( !strcmp(cur_fmt, "ad") )
    return INT32_ARRAY_T;
  else if( !strcmp(cur_fmt, "aud") )
    return UINT32_ARRAY_T;
  else if( !strcmp(cur_fmt, "ald") )
    return INT64_ARRAY_T;
  else if( !strcmp(cur_fmt, "auld") )
    return UINT64_ARRAY_T;
  else if( !strcmp(cur_fmt, "f") )
    return FLOAT_T;
  else if( !strcmp(cur_fmt, "af") )
    return FLOAT_ARRAY_T;
  else if( !strcmp(cur_fmt, "lf") )
    return DOUBLE_T;
  else if( !strcmp(cur_fmt, "alf") )
    return DOUBLE_ARRAY_T;
  else if( !strcmp(cur_fmt, "s") )
    return STRING_T;
  else if( !strcmp(cur_fmt, "as") )
    return STRING_ARRAY_T;
  else
    return UNKNOWN_T;
}

/*********************************************************
 *  Functions used to implement sending and recieving of
 *  some basic data types
 *********************************************************/

int write(int fd, const void *buf, int count)
{
  int ret = send(fd, buf, count, 0);

  //should do a recursive call checking for syscall interuption
  return ret;
}

int read(int fd, void *buf, int count)
{


  int bytes_recvd=0, retval;
  while(bytes_recvd != count){


    retval = recv(fd, ((char*)buf)+bytes_recvd, count-bytes_recvd, MSG_WAITALL);

    if(retval == -1){
      if(errno == EINTR){
          continue;
      }
      else{
          mrn_printf(3, MCFL, stderr, "premature return from read(). Got %d of %d "
                    " bytes. errno: %d ", bytes_recvd, count, errno);
            if( errno != 0 )
            {
            perror("");
            }
          return -1;
      }
    }
    else if( (retval == 0) && (errno == EINTR) )
    {
        // this situation has been seen to occur on Linux
        // when the remote endpoint has gone away
        return -1;
    }
    else{
      bytes_recvd += retval;
      if(bytes_recvd < count && errno == EINTR){
          continue;
      }
      else{
	//bytes_recvd is either count, or error other than "eintr" occured
          if(bytes_recvd != count){
              mrn_printf(3, MCFL, stderr, "premature return from read(). %d of %d "
                        " bytes. errno: %d ", bytes_recvd, count, errno);
                if( errno != 0 )
                {
                perror("");
                }
          }
          return bytes_recvd;
      }
    }
  }
  assert(0);
  return -1;
}

int readmsg(int fd, struct msghdr *msg){
  //should do a recursive call checking for syscall interuption
  return recvmsg(fd, msg, MSG_WAITALL); 
}
