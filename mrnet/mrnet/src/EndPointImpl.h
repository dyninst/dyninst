#if !defined(endpointimpl_h)
#define endpointimpl_h 1

#include <string>
#include "mrnet/h/MR_Network.h"

namespace MRN
{

class NetworkNode;
class EndPointImpl: public EndPoint{
 private:
  unsigned int id;
  std::string hostname;
  unsigned short port;
  //NetworkNode * downstream_node;

 public:
  EndPointImpl(int _id, const char * _hostname, unsigned short _port);
  virtual ~EndPointImpl();
  virtual const char * get_HostName();
  virtual unsigned short get_Port();
  virtual unsigned int get_Id();
  virtual bool compare(const char * _hostname, unsigned short _port);
};

} // namespace MRN

#endif /* endpoint_h 1 */
