%{
#include <list>

#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/utils.h"

extern std::list <MC_NetworkNode *>* hostlist;
extern std::list <MC_NetworkNode *>* potential_root;
extern MC_NetworkGraph* parsed_graph;

#if defined(__cplusplus)
extern "C" {
void yyerror(const char *s);
}
#endif
int yylex(void);

#define YYDEBUG 1
extern int linenum;
%}

%union {
  unsigned short port;
  char * hostname;
  MC_NetworkNode * node_ptr;
}

%token <port> PORT 
%token <hostname> HOSTNAME
%token COLON SEMI ARROW
%type <node_ptr> host

%%
config: line config
      | line
          {
	    if(potential_root->size() != 1){
	      mc_printf(MCFL, stderr, "graph is not connected\n");
	      YYABORT;
	    }	   
            std::list<MC_NetworkNode *>::iterator iter=potential_root->begin();
	    parsed_graph->set_Root(*iter);
	    fprintf(stderr, "Graph's Root/FE is %s:%hd\n",
                       (*iter)->get_HostName().c_str(), (*iter)->get_Port() );
	  }

line: host ARROW hosts SEMI
        {
	  //fprintf(stderr, "%s:%hd's children are:\n",$1->get_HostName().c_str(),
		     //$1->get_Port() );
          std::list<MC_NetworkNode *>::iterator iter=hostlist->begin();
          for(; iter != hostlist->end(); iter++){
	    MC_NetworkNode * cur_node;
	    cur_node = (*iter);
	    potential_root->remove(cur_node); //node cannot be a root, remove
	    //fprintf(stderr, " %s:%hd\n", cur_node->get_HostName().c_str(),
		       //cur_node->get_Port() );
	    $1->add_Child(cur_node);
	  }
	  hostlist->clear();
	}
    | error {fprintf(stderr, "line parse error on line %d\n", linenum-1); YYABORT}

hosts: host hosts
         {
	   //fprintf(stderr, "Adding %s:%d to hostlist\n",
                   //$2->get_HostName().c_str(), $2->get_Port() );
	   hostlist->push_back($1);
	 }
     | host
         {
	   //fprintf(stderr, "Adding %s:%d to hostlist\n",
                   //$1->get_HostName().c_str(), $1->get_Port() );
	   hostlist->push_back($1);
	 }


host: HOSTNAME COLON PORT
        {
	        // fprintf(stderr, "looking for new node(%s:%d)\n", $1, $3);
          MC_NetworkNode * cur_node = parsed_graph->find_Node($1, $3);
          if(cur_node == NULL){
	        // fprintf(stderr, "creating new node(%s:%d)\n", $1, $3);
            cur_node = new MC_NetworkNode($1, $3);
            parsed_graph->add_Node(cur_node);
	        potential_root->push_back(cur_node);
          }
          else
          {
	        // fprintf(stderr, "found  node(%s:%d)\n", $1, $3);
          }
          $$ = cur_node;
	  //fprintf(stderr, "Reducing %s:%hd\n", cur_node->get_HostName().c_str(),
		     //cur_node->get_Port() );
        }
    | error {fprintf(stderr, "host parse error on line %d\n", linenum); YYABORT}

%%


void yyerror(const char *s)
{
  //fprintf(stderr, "%s\n", s);
}
