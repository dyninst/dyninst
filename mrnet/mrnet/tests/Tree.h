#if !defined(__tree_h)
#define __tree_h

#include <stdio.h>

#include <string>
#include <map>
#include <list>

#include "Topology.h"

class Node{
 private:
    std::string _name;
    std::list<Node*> children;
    bool visited;

 public:
    Node( std::string n);
    void add_Child( Node * );
    void print_ToFile(FILE *);
    std::string name(){ return _name; } 

    unsigned int visit();
};

class BalancedTree;
class GenericTree;

class Tree{
    friend class BalancedTree;
    friend class GenericTree;
 private:
    Node * root;
    static std::map<std::string, Node *> NodesByName;
    bool _contains_cycle, _contains_unreachable_nodes;

 public:
    static Node * get_Node( std::string &);

    Tree();
    void create_TopologyFile( FILE * );
    bool validate();
    bool contains_Cycle() { return _contains_cycle; }
    bool contains_UnreachableNodes() { return _contains_unreachable_nodes; }
};

class BalancedTree: public Tree{
 private:
    unsigned int _fanout, _num_leaves, _depth;

 public:
    BalancedTree( std::vector<std::string> &hosts, std::string &topology_spec );
};

class GenericTree: public Tree{
 private:

 public:
    GenericTree( std::vector<std::string> &hosts, std::string &topology_spec );
};

#endif /* __tree_h */
