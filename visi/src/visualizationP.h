#ifndef _visualizationP_H
#define _visualizationP_H

#include "visi.xdr.SRVR.h"

class visi_visualization : public visualization {
    public:
        visi_visualization(int fd) : visualization(fd, NULL, NULL, false) {;};
        virtual void handle_error();
};
#endif
