#if !defined(__mc_filter_h)
#define __mc_filter_h 1

#include <string>
#include <list>
#include <map>
#include "mrnet/src/Message.h"

class MC_Filter{
 protected:
  unsigned short filter_id;

 public:
  MC_Filter(unsigned short _filter_id);
  virtual ~MC_Filter();
  virtual int push_packets(std::list <MC_Packet *> &packets_in,
			   std::list <MC_Packet *> &packets_out)=0;
};

class MC_Aggregator: public MC_Filter{
 public:
  typedef struct{
    std::string format_str;
    void(*filter)(MC_DataElement **, unsigned int, MC_DataElement ***, unsigned int*);
  }AggregatorSpec;

 private:
  AggregatorSpec * aggr_spec;

 public:
  MC_Aggregator(unsigned short _filter_id);
  virtual ~MC_Aggregator();
  virtual int push_packets(std::list <MC_Packet *> &packets_in,
			   std::list <MC_Packet *> &packets_out);
};

class MC_RemoteNode;
class MC_Synchronizer: public MC_Filter{
 private:
  std::map <MC_RemoteNode *, std::list<MC_Packet*> *> PacketListByNode;
  void(*sync)(std::list <MC_Packet *>&, std::list <MC_Packet *>&,
              std::list <MC_RemoteNode *> &, void **);
  std::list <MC_RemoteNode *> downstream_nodes;
  void * object_local_storage;

 public:
  MC_Synchronizer(unsigned short _filter_id, std::list <MC_RemoteNode *> &);
  virtual ~MC_Synchronizer();
  virtual int push_packets(std::list <MC_Packet *> &packets_in,
			   std::list <MC_Packet *> &packets_out);
};

#define AGGR_NULL 0
#define AGGR_NULL_FORMATSTR ""

#define AGGR_INT_SUM_ID 2000
#define AGGR_INT_SUM_FORMATSTR "%d"
void aggr_Int_Sum(MC_DataElement **, unsigned int, MC_DataElement ***, unsigned int*);

#define AGGR_FLOAT_AVG_ID 2001
#define AGGR_FLOAT_AVG_FORMATSTR "%f"
void aggr_Float_Avg(MC_DataElement **, unsigned int, MC_DataElement ***, unsigned int*);

#define AGGR_FLOAT_MAX_ID 2006
#define AGGR_FLOAT_MAX_FORMATSTR "%lf"
void aggr_Float_Max(MC_DataElement **, unsigned int, MC_DataElement ***, unsigned int*);

#define AGGR_CHARARRAY_CONCAT_ID 2007
#define AGGR_CHARARRAY_CONCAT_FORMATSTR "%ac"
void aggr_CharArray_Concat(MC_DataElement **, unsigned int, MC_DataElement ***, unsigned int*);

#define SYNC_WAITFORALL 2003
void sync_WaitForAll(std::list <MC_Packet *>&, std::list <MC_Packet *>&,
                     std::list <MC_RemoteNode *> &, void **);

#define SYNC_DONTWAIT 2004
void sync_DontWait(std::list <MC_Packet *>&, std::list <MC_Packet *>&,
                   std::list <MC_RemoteNode *> &, void **);

#define SYNC_TIMEOUT 2005
void sync_TimeOut(std::list <MC_Packet *>&, std::list <MC_Packet *>&,
                  std::list <MC_RemoteNode *> &, void **);

#endif /* __mc_filter_h */
