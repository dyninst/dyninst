%{
#include <list>

#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/utils.h"

int yylex(void);

namespace MRN
{

#if defined(__cplusplus)
extern "C" {
void yyerror(const char *s);
}
#endif

#define YYDEBUG 1
extern int linenum;
%}

%union {
  unsigned short port;
  char * hostname;
  MRN::NetworkNode * node_ptr;
}

%token <port> PORT 
%token <hostname> HOSTNAME
%token COLON SEMI ARROW
%type <node_ptr> host

%%
config: line config
      | line
          {
	    if(NetworkImpl::potential_root->size() != 1){
	      mrn_printf(1, MCFL, stderr, "graph is not connected\n");
	      YYABORT;
	    }	   
            std::list<NetworkNode *>::iterator iter = 
                NetworkImpl::potential_root->begin();
	    NetworkImpl::parsed_graph->set_Root(*iter);
	  }
;

line: host ARROW hosts SEMI
        {
	  //fprintf(stderr, "%s:%hd's children are:\n",$1->get_HostName().c_str(),
		     //$1->get_Port() );
          std::list<NetworkNode *>::iterator iter = 
            NetworkImpl::hostlist->begin();
          for(; iter != NetworkImpl::hostlist->end(); iter++){
	    NetworkNode * cur_node;
	    cur_node = (*iter);
	    NetworkImpl::potential_root->remove(cur_node); //node cannot be a root
	    //fprintf(stderr, " %s:%hd\n", cur_node->get_HostName().c_str(),
		       //cur_node->get_Port() );
	    $1->add_Child(cur_node);
	  }
	  NetworkImpl::hostlist->clear();
	}
    | error {fprintf(stderr, "line parse error on line %d\n", linenum-1);
             YYABORT; }
;

hosts: hosts host
         {
	   //fprintf(stderr, "Adding %s:%d to hostlist\n",
                   //$2->get_HostName().c_str(), $2->get_Port() );
	   NetworkImpl::hostlist->push_back($2);
	 }
     | host
         {
	   //fprintf(stderr, "Adding %s:%d to hostlist\n",
                   //$1->get_HostName().c_str(), $1->get_Port() );
	   NetworkImpl::hostlist->push_back($1);
	 }
;


host: HOSTNAME COLON PORT
        {
	        // fprintf(stderr, "looking for new node(%s:%d)\n", $1, $3);
          NetworkNode * cur_node = NetworkImpl::parsed_graph->find_Node($1, $3);
          if(cur_node == NULL){
	        // fprintf(stderr, "creating new node(%s:%d)\n", $1, $3);
            cur_node = new NetworkNode($1, $3);
            NetworkImpl::parsed_graph->add_Node(cur_node);
	        NetworkImpl::potential_root->push_back(cur_node);
          }
          else
          {
	        // fprintf(stderr, "found  node(%s:%d)\n", $1, $3);
          }
          $$ = cur_node;
	  //fprintf(stderr, "Reducing %s:%hd\n", cur_node->get_HostName().c_str(),
		     //cur_node->get_Port() );
        }
    | error {fprintf(stderr, "host parse error on line %d\n", linenum);
             YYABORT; }
;

%%


void yyerror(const char *s)
{
  //fprintf(stderr, "%s\n", s);
}

} // namespace MRN
