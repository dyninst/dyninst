/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__streammanager_h)
#define __streammanager_h

#include <vector>
#include <list>
#include "mrnet/src/Filter.h"

namespace MRN
{

class RemoteNode;
class StreamManager{
 private:
    unsigned short stream_id;
    Filter * downstream_aggregator;
    Filter * upstream_aggregator;
    Filter * sync;

    const RemoteNode * upstream_node;
    std::list <const RemoteNode *> downstream_nodes;

    static std::map<unsigned int, StreamManager*> allStreamManagersById;
    static XPlat::Mutex all_stream_managers_mutex;

 public:
    StreamManager(int istream_id,
                  const RemoteNode * iupstream_node,
                  const std::list <const RemoteNode *> &idownstream_nodes,
                  int isync_id,
                  int ids_agg_id,
                  int ius_agg_id );
    ~StreamManager();

    int push_packet(Packet&, std::vector<Packet> &, bool going_upstream=true) const;

    const std::list<const RemoteNode*>&
        get_DownStreamNodes( void ) const   { return downstream_nodes; }
    const RemoteNode* get_UpStreamNode( void )    { return upstream_node; }

    static void set_StreamManagerById( unsigned int, StreamManager * );
    static StreamManager * get_StreamManagerById( unsigned int );
    static void delete_StreamManagerById( unsigned int );
};

} // namespace MRN

#endif /* __streammanager_h  */
