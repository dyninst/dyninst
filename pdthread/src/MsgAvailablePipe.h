#ifndef MSG_AVAILABLE_PIPE_H
#define MSG_AVAILABLE_PIPE_H

#include "pdutil/h/pddesc.h"


namespace pdthr
{

class MsgAvailablePipe
{
private:
    PdFile rfd;
    PdFile wfd;

public:
    MsgAvailablePipe( void );
    ~MsgAvailablePipe( void );

    PdFile& GetReadEndpoint( void )     { return rfd; }

    bool RaiseMsgAvailable( void );
    bool Drain( void );
};

} // namespace pdthr

#endif // MSG_AVAILABLE_PIPE_H
