#include "mrnet/src/Filter.h"
#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/utils.h"

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
  aggr_spec = MC_ParentNode::AggrSpecById[filter_id];
}

MC_Aggregator::~MC_Aggregator()
{
}

int MC_Aggregator::push_packets(std::list <MC_Packet *> &packets_in,
			        std::list <MC_Packet *> &packets_out)
{
  std::list <MC_Packet *>::iterator iter;
  MC_DataElement **in, **out;
  unsigned int in_count=packets_in.size(), out_count=0;
  unsigned int i, j;

  mc_printf(MCFL, stderr, "In aggr.push_packets()\n");

  if(aggr_spec->filter == NULL){ //do nothing
    packets_out = packets_in;
    packets_in.clear();
    mc_printf(MCFL, stderr, "NULL FILTER: returning %d packets\n",
              packets_out.size());
    return 0;
  }

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

  //put out dataelements in packets and push into packets_out list
  for(i=0; i<out_count; i++){
    MC_Packet * cur_packet = new MC_Packet(out[i], aggr_spec->format_str.c_str());
    packets_out.push_back(cur_packet);
  }

  mc_printf(MCFL, stderr, "Leaving aggr.push_packets()\n");
  return 0;
}

/*============================================*
 *    MC_Synchronizer Class Definition        *
 *============================================*/
MC_Synchronizer::MC_Synchronizer(unsigned short _filter_id,
                                 std::list <MC_RemoteNode *> &nodes)
  :MC_Filter(_filter_id), downstream_nodes(nodes), object_local_storage(NULL)
{
  sync = MC_ParentNode::SyncById[filter_id];
}

MC_Synchronizer::~MC_Synchronizer()
{
}

int MC_Synchronizer::push_packets(std::list <MC_Packet *> &packets_in,
			          std::list <MC_Packet *> &packets_out)
{
  mc_printf(MCFL, stderr, "In sync.push_packets(). Pushing %d packets\n",
            packets_in.size());
  sync(packets_in, packets_out, downstream_nodes, &object_local_storage);
  mc_printf(MCFL, stderr, "Leaving sync.push_packets(). Returning %d packets\n",
            packets_out.size());
  return 0;
}

/*==========================================*
 *    Default Aggregator Definitions        *
 *==========================================*/
void aggr_Int_Sum(MC_DataElement **in_elems, unsigned int in_count,
                  MC_DataElement ***out_elems, unsigned int *out_count)
{
  int sum=0;

  for(unsigned int i=0; i<in_count; i++){
    sum += in_elems[i][0].val.d;
  }

  *out_count = 1;
  (*out_elems) = new MC_DataElement* [1];
  (*out_elems)[0] = new MC_DataElement;
  (*out_elems)[0][0].val.d = sum;
  (*out_elems)[0][0].type = INT32_T;
}

void aggr_Float_Avg(MC_DataElement **in_elems, unsigned int in_count,
                    MC_DataElement ***out_elems, unsigned int *out_count)
{
  float avg=0;

  mc_printf(MCFL, stderr, "averaging: [");
  for(unsigned int i=0; i<in_count; i++){
    _fprintf((stderr, "%f, ", in_elems[i][0].val.f));
    avg += in_elems[i][0].val.f;
  }
  _fprintf((stderr, "]\n"));

  avg /= (float)in_count;

  *out_count = 1;
  (*out_elems) = new MC_DataElement* [1];
  (*out_elems)[0] = new MC_DataElement;
  (*out_elems)[0][0].val.f = avg;
  (*out_elems)[0][0].type = FLOAT_T;
}


void aggr_CharArray_Concat(MC_DataElement **in_elems, unsigned int in_count,
                           MC_DataElement ***out_elems, unsigned int *out_count)
{
  int result_array_size=0;
  char *result_array;

  for(unsigned int i=0; i<in_count; i++){
    result_array_size += in_elems[i][0].array_len;
  }
  result_array = (char *)malloc(result_array_size * sizeof(char));

  int pos=0;
  for(unsigned int i=0; i<in_count; i++){
    memcpy(result_array+pos, in_elems[i][0].val.p, in_elems[i][0].array_len);
    pos += in_elems[i][0].array_len;
  }

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
                     std::list <MC_RemoteNode *> &downstream_nodes,
                     void **object_local_storage)
{
  std::map <MC_RemoteNode *, std::list<MC_Packet*> *> * PacketListByNode;

  mc_printf(MCFL, stderr, "In sync_WaitForAll()\n");
  if(*object_local_storage == NULL){
    PacketListByNode = new std::map <MC_RemoteNode *, std::list<MC_Packet*> *>;
    *object_local_storage = PacketListByNode;

    std::list <MC_RemoteNode *>::iterator iter;
    mc_printf(MCFL, stderr, "Creating Map of %d downstream_nodes\n",
              downstream_nodes.size());
    for(iter=downstream_nodes.begin(); iter!=downstream_nodes.end(); iter++){
      (*PacketListByNode)[(*iter)] = new std::list<MC_Packet*>;
    }
  }
  else{
    PacketListByNode = (std::map <MC_RemoteNode *, std::list<MC_Packet*> *>*)
                       *object_local_storage;
  }

  //place all incoming packets in appropriate list
  std::list <MC_Packet *>::iterator iter;
  mc_printf(MCFL, stderr, "Placing %d incoming packets\n", packets_in.size());
  for(iter = packets_in.begin(); iter != packets_in.end(); iter++){
    ((*PacketListByNode)[(*iter)->inlet_node])->push_back(*iter);
  }
  packets_in.clear();

  //check to see if all lists have at least one packet, "a wave"
  mc_printf(MCFL, stderr, "Checking if all downstream_nodes are ready ...");
  std::map <MC_RemoteNode *, std::list<MC_Packet*> *>::iterator iter2;
  for(iter2=PacketListByNode->begin();
      iter2 != PacketListByNode->end(); iter2++){
    if( ((*iter2).second)->size() == 0 ){
      //all lists not ready!
      _fprintf((stderr, "no!\n"));
      return;
    }
  }
  _fprintf((stderr, "yes!\n"));

  mc_printf(MCFL, stderr, "Placing outgoing packets\n");
  //if we get here, all lists ready. push front of all lists onto "packets_out"
  for(iter2=PacketListByNode->begin();
      iter2 != PacketListByNode->end(); iter2++){
    packets_out.push_back( ((*iter2).second)->front() );
    ((*iter2).second)->pop_front();
  }
  mc_printf(MCFL, stderr, "Returning %d outgoing packets\n", packets_out.size());
}

void sync_DontWait(std::list <MC_Packet *> &packets_in,
                   std::list <MC_Packet *> &packets_out,
                   std::list <MC_RemoteNode *> &,
                   void **object_local_storage)
{
  packets_out = packets_in;
  packets_in.clear();
}

void sync_TimeOut(std::list <MC_Packet *> &packets_in,
                  std::list <MC_Packet *> &packets_out,
                  std::list <MC_RemoteNode *> &,
                  void **object_local_storage)
{
}

