
#ifndef SHOWERROR_H
#define SHOWERROR_H

#include "comm.h"
#include "dyninst.h"
#include "resource.h"

extern resource *machineResource;
extern pdRPC *tp;

#define showErrorCallback(a,b) tp->showErrorCallback(a,b,machineResource->part_name())

#endif /* SHOWERROR_H */
