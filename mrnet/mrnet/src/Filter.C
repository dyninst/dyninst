#include "mrnet/src/MC_Filter.h"
#include "mrnet/src/MC_CommunicationNode.h"

/*======================================*
 *    MC_Filter Class Definition        *
 *======================================*/
MC_Filter::MC_Filter(unsigned short _filter_id)
  :filter_id(_filter_id)
{
}

MC_Filter::~MC_Filter()
{
}

/*==========================================*
 *    MC_Aggregator Class Definition        *
 *==========================================*/
MC_Aggregator::MC_Aggregator(unsigned short _filter_id)
  :MC_Filter(_filter_id)
{
  aggr_spec = MC_InternalNode::AggrSpecById[filter_id];
}

MC_Aggregator::~MC_Aggregator()
{
}

int MC_Aggregator::push_packets(std::list <MC_Packet *> &packets_in,
			        std::list <MC_Packet *> &packets_out)
{
  std::list <MC_Packet *>::iterator iter;
  MC_DataElement **in, **out;
  int in_count=packets_in.size(), out_count=0;
  unsigned int i, j;

  //Allocate an array of dataelements for each packet. This represents a
  //set of data possibly each downstream_node
  in = new MC_DataElement * [in_count];
  for(i=0,iter = packets_in.begin(); iter != packets_in.end(); iter++, i++){
    assert(aggr_spec->format_str == (*iter)->get_FormatString());

    //for each "downstream node" a packet contains an array of elements
    in[i] = new MC_DataElement[(*iter)->get_NumElements()] ;
    for(j=0; j<(*iter)->get_NumElements(); j++){
      in[i][j] = *((*iter)->get_Element(j));
    }
  }

  aggr_spec->filter(in, in_count, &out, &out_count);
  return 0;
}

/*============================================*
 *    MC_Synchronizer Class Definition        *
 *============================================*/
MC_Synchronizer::MC_Synchronizer(unsigned short _filter_id,
                                 std::list <MC_RemoteNode *> &nodes)
  :MC_Filter(_filter_id), downstream_nodes(nodes)
{
  sync = MC_InternalNode::SyncById[filter_id];
}

MC_Synchronizer::~MC_Synchronizer()
{
}

int MC_Synchronizer::push_packets(std::list <MC_Packet *> &packets_in,
			          std::list <MC_Packet *> &packets_out)
{
  sync(packets_in, packets_out, downstream_nodes);
  return 0;
}

/*==========================================*
 *    Default Aggregator Definitions        *
 *==========================================*/
void aggr_Int_Sum(MC_DataElement **in_elems, int in_count,
                  MC_DataElement ***out_elems, int *out_count)
{
  int i;
  int sum=0;

  for(i=0; i<in_count; i++){
    sum += in_elems[i][0].val.d;
  }

  *out_count = 1;
  (*out_elems) = new MC_DataElement* [1];
  (*out_elems)[0] = new MC_DataElement;
  (*out_elems)[0][0].val.d = sum;
  (*out_elems)[0][0].type = INT32_T;
}

void aggr_Float_Avg(MC_DataElement **in_elems, int in_count,
                    MC_DataElement ***out_elems, int *out_count)
{
  int i;
  float avg=0;

  for(i=0; i<in_count; i++){
    avg += in_elems[i][0].val.f;
  }

  avg /= (float)in_count;

  *out_count = 1;
  (*out_elems) = new MC_DataElement* [1];
  (*out_elems)[0] = new MC_DataElement;
  (*out_elems)[0][0].val.f = avg;
  (*out_elems)[0][0].type = FLOAT_T;
}


void aggr_CharArray_Concat(MC_DataElement **in_elems, int in_count,
                           MC_DataElement ***out_elems, int *out_count)
{
  int i, result_array_size=0;
  char *result_array;

  for(i=0; i<in_count; i++){
    result_array_size += in_elems[i][0].array_len;
  }

  int pos=0;
  for(i=0; i<in_count; i++){
    memcpy(result_array+pos, in_elems[i][0].val.p, in_elems[i][0].array_len);
    pos += in_elems[i][0].array_len;
  }

  result_array = (char *)malloc(result_array_size * sizeof(char));

  *out_count = 1;
  (*out_elems) = new MC_DataElement* [1];
  (*out_elems)[0] = new MC_DataElement;
  (*out_elems)[0][0].val.p = result_array;
  (*out_elems)[0][0].type = CHAR_ARRAY_T;
}

/*============================================*
 *    Default Synchronizer Definitions        *
 *============================================*/
void sync_WaitForAll(std::list <MC_Packet *> &packets_in,
                     std::list <MC_Packet *> &packets_out,
                     std::list <MC_RemoteNode *> &downstream_nodes)
{
  static bool initialized=false;
  static std::map <MC_RemoteNode *, std::list<MC_Packet*> *> PacketListByNode;

  if(!initialized){
    std::list <MC_RemoteNode *>::iterator iter;
    for(iter=downstream_nodes.begin(); iter!=downstream_nodes.end(); iter++){
      PacketListByNode[(*iter)] = new std::list<MC_Packet*>;
    }
  }

  //place all incoming packets in appropriate list
  std::list <MC_Packet *>::iterator iter;
  for(iter = packets_in.begin(); iter != packets_in.end(); iter++){
    (PacketListByNode[(*iter)->inlet_node])->push_back(*iter);
  }

  //check to see if all lists have at least one packet, "a wave"
  std::map <MC_RemoteNode *, std::list<MC_Packet*> *>::iterator iter2;
  for(iter2=PacketListByNode.begin(); iter2 != PacketListByNode.end(); iter2++){
    if( ((*iter2).second)->size() == 0 ){
      //all lists not ready!
      return;
    }
  }

  //if we get here, all lists ready. push front of all lists onto "packets_out"
  for(iter2=PacketListByNode.begin(); iter2 != PacketListByNode.end(); iter2++){
    packets_out.push_back( ((*iter2).second)->front() );
    ((*iter2).second)->pop_front();
  }
}

void sync_DontWait(std::list <MC_Packet *> &packets_in,
                   std::list <MC_Packet *> &packets_out,
                   std::list <MC_RemoteNode *> &)
{
  packets_out = packets_in;
}

void sync_TimeOut(std::list <MC_Packet *> &packets_in,
                  std::list <MC_Packet *> &packets_out,
                  std::list <MC_RemoteNode *> &)
{
}

