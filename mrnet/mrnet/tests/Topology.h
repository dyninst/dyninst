#if !defined(__topology_h)
#define __topology_h

#include <stdio.h>

#include <string>
#include <vector>

namespace MRN {
class Topology{
 private:
  std::vector< unsigned int > levels;
  unsigned int num_nodes, num_internalnodes, num_leaves;
  bool _error;

 public:
  Topology(std::string &);
  bool error() const { return _error; }
  unsigned int level( unsigned int l) const { return levels[l]; }
  unsigned int get_NumLevels() const { return levels.size(); }
  unsigned int get_NumNodes() const { return num_nodes; }
  unsigned int get_NumInternalNodes() const { return num_internalnodes; }
  unsigned int get_NumLeaves() const { return num_leaves; }
};
} /* namespace MRN */

#endif /* __topology_h */
