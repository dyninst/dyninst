#if !defined(__filter_h)
#define __filter_h 1

#include <string>
#include <list>
#include <map>
#include "mrnet/src/Message.h"
#include "mrnet/src/pthread_sync.h"
#include "mrnet/src/FilterDefinitions.h"

class Filter{
 protected:
  unsigned short filter_id;

 public:
  Filter(unsigned short _filter_id);
  virtual ~Filter();
  virtual int push_packets(std::list <Packet *> &packets_in,
			   std::list <Packet *> &packets_out)=0;
};

class Aggregator: public Filter{
 public:
  typedef struct{
    std::string format_str;
    void(*filter)(DataElement **, unsigned int, DataElement ***, unsigned int*);
  }AggregatorSpec;

 private:
  AggregatorSpec * aggr_spec;

 public:
  Aggregator(unsigned short _filter_id);
  virtual ~Aggregator();
  virtual int push_packets(std::list <Packet *> &packets_in,
			   std::list <Packet *> &packets_out);
};

class RemoteNode;
class Synchronizer: public Filter{
 private:
  std::map <RemoteNode *, std::list<Packet*> *> PacketListByNode;
  void(*sync)(std::list <Packet *>&, std::list <Packet *>&,
              std::list <RemoteNode *> &, void **);
  std::list <RemoteNode *> downstream_nodes;
  void * object_local_storage;
  pthread_sync fsync;

 public:
  Synchronizer(unsigned short _filter_id, std::list <RemoteNode *> &);
  virtual ~Synchronizer();
  virtual int push_packets(std::list <Packet *> &packets_in,
			   std::list <Packet *> &packets_out);
};

#endif /* __filter_h */
