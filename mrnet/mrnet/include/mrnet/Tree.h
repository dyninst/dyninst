/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ***************************************************************************/

#if !defined( __tree_h )
#define __tree_h  1

namespace MRN {
class BalancedTree;
class KnomialTree;
class GenericTree;

class Tree{
    friend class BalancedTree;
    friend class GenericTree;

 public:
    class Node{
    private:
        std::string _name;
        std::set<Node*> children;
        bool visited;
        
    public:
        Node( std::string n );
        void add_Child( Node * );
        void print_ToFile(FILE *);
        std::string name(){ return _name; } 
        
        unsigned int visit();
    };

 protected:
    Node * root;
    std::map<std::string, Node *> NodesByName;
    bool _contains_cycle, _contains_unreachable_nodes;
    unsigned int _fanout, _num_leaves, _depth,
        _fe_procs_per_node, _be_procs_per_node, _int_procs_per_node;

    bool validate();
    bool contains_Cycle() { return _contains_cycle; }
    bool contains_UnreachableNodes() { return _contains_unreachable_nodes; }
    virtual bool initialize_Tree( std::string &topology_spec,
                                  std::list<std::string> &hosts)=0;

 public:
    Node * get_Node( std::string );

    Tree();
    virtual ~Tree(){}
    bool create_TopologyFile( const char* ifilename );
    bool create_TopologyFile( FILE* ifile );
    void get_TopologyBuffer( char** buf );

    static bool get_HostsFromFile( const char* ifilename, std::list< std::string >& hosts );
    static void get_HostsFromFileStream( FILE* ifile, std::list< std::string >& hosts );
};

class BalancedTree: public Tree{
 private:
    virtual bool initialize_Tree( std::string &topology_spec,
                                  std::list<std::string> &hosts );

 public:
    BalancedTree( std::string &topology_spec, std::list<std::string> &hosts,
                  unsigned int ife_procs_per_node=1,
                  unsigned int ibe_procs_per_node=1,
                  unsigned int iint_procs_per_node=1);
    BalancedTree( std::string &topology_spec, std::string &host_file,
                  unsigned int ife_procs_per_node=1,
                  unsigned int ibe_procs_per_node=1,
                  unsigned int iint_procs_per_node=1 );
};

class KnomialTree: public Tree{
 private:
    virtual bool initialize_Tree( std::string &topology_spec,
                                  std::list<std::string> &hosts );

 public:
    KnomialTree( std::string &topology_spec, std::list<std::string> &hosts );
    KnomialTree( std::string &topology_spec, std::string &host_file );
};

class GenericTree: public Tree{
 private:
    virtual bool initialize_Tree( std::string &topology_spec,
                                  std::list<std::string> &hosts );

 public:
    GenericTree( std::string &topology_spec, std::list<std::string> &hosts );
    GenericTree( std::string &topology_spec, std::string &host_file );
};

} /* namespace MRN */

#endif /* __tree_h */
