#if !defined(mc_endpointimpl_h)
#define mc_endpointimpl_h 1

#include "mrnet/h/MC_Network.h"
#include <string>

class MC_NetworkNode;
class MC_EndPointImpl: public MC_EndPoint{
 private:
  unsigned int id;
  std::string hostname;
  unsigned short port;
  //MC_NetworkNode * downstream_node;

 public:
  MC_EndPointImpl(int _id, const char * _hostname, unsigned short _port);
  virtual ~MC_EndPointImpl();
  virtual const char * get_HostName();
  virtual unsigned short get_Port();
  virtual unsigned int get_Id();
  virtual bool compare(const char * _hostname, unsigned short _port);
};

#endif /* mc_endpoint_h 1 */
