#include <stdio.h>

#include "mrnet/src/ChildNode.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/*===================================================*/
/*  ChildNode CLASS METHOD DEFINITIONS            */
/*===================================================*/
ChildNode::ChildNode(bool _threaded, std::string _host, unsigned short _port)
    :hostname(_host), port(_port), threaded(_threaded)
{
}

ChildNode::~ChildNode(void)
{
}


int ChildNode::getConnections( int** conns, unsigned int* nConns )
{
    int ret = 0;

    if( (conns != NULL) && (nConns != NULL) ) {
        *nConns = 1;
        *conns = new int[*nConns];
        (*conns)[0] = upstream_node->get_sockfd();
    }
    else {
        ret = -1;
    }
    return ret;
}

} // namespace MRN
