
#ifndef  MACHINE_TYPE_ENUM
#define  MACHINE_TYPE_ENUM

/*
 * $Log: machineType.h,v $
 * Revision 1.3  1994/08/20 23:13:02  markc
 * Added new machine type.
 * Cast stringHandle to (char*) to print in tunableConst.C
 *
 * Revision 1.2  1994/08/17  18:23:51  markc
 * Added new classes: Cstring KeyList, KList
 * Added new function: RPCgetArg
 * Changed typedefs in machineType.h to #defines
 *
 * Revision 1.1  1994/07/07  03:20:39  markc
 * Added removeAll function to list class.
 * Added machineType headers to specify pvm, cm5, ...
 *
 */

#define metPVM 0
#define metCM5 1
#define metUNIX 2

#endif
