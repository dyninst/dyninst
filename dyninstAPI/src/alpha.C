
/* 
 * $Log: alpha.C,v $
 * Revision 1.2  2000/11/15 22:56:06  bernat
 *
 * Added aixDL.C as a target for rs6000-ibm-aix4.2 builds
 * Added fileDescriptor object/getExecFileDescriptor call
 *
 * Added shared object parsing for AIX
 * Added inter-module function call insertion for AIX
 * Added inter-module function call determination for AIX
 *
 * Revision 1.1  1998/08/25 19:35:01  buck
 * Initial commit of DEC Alpha port.
 *
 *
 */

class ptraceKludge { };

fileDescriptor *getExecFileDescriptor(string filename,
				     int &,
				     bool)
{
  fileDescriptor *desc = new fileDescriptor(filename);
  return desc;
}


