/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(__backendnode_h)
#define __backendnode_h 1

#include <string>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/Message.h"

namespace MRN
{

class BackEndNode: public ChildNode, public CommunicationNode{
 private:
    Network * network;
    int proc_newStream( Packet & pkt );

 public:
    BackEndNode(Network *, std::string _hostname, unsigned short _backend_id,
                std::string _phostname, unsigned short _pport, 
                unsigned short _pid);
    virtual ~BackEndNode(void);
    virtual int proc_PacketsFromUpStream(std::list <Packet> &);
    virtual int proc_DataFromUpStream(Packet &);
    int send(Packet &);
    int flush();
    int recv( bool blocking=true );
};

} // namespace MRN

#endif /* __backendnode_h */
