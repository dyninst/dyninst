%{
#include <list>

#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/utils.h"

static std::list <MRN::NetworkNode *> hostlist;
static std::list <MRN::NetworkNode *> potential_root;

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
    if(potential_root.size() != 1){
        mrn_printf(1, MCFL, stderr, "graph is not connected\n");
	YYABORT;
    }	   
    NetworkImpl::parsed_graph->set_Root( *potential_root.begin() );
}
;

line: host ARROW hosts SEMI
{
    std::list<NetworkNode *>::iterator iter = hostlist.begin();
    for(; iter != hostlist.end(); iter++){
        NetworkNode * cur_node;
	cur_node = (*iter);
	potential_root.remove(cur_node); //node cannot be a root
	$1->add_Child(cur_node);
    }
    hostlist.clear();
}
| error
{
    fprintf(stderr, "line parse error on line %d\n", linenum-1);
    YYABORT;
}
;

hosts: hosts host
{
    hostlist.push_back($2);
}
| host
{
    hostlist.push_back($1);
}
;


host: HOSTNAME COLON PORT
{
    NetworkNode * cur_node = NetworkImpl::parsed_graph->find_Node($1, $3);
    if(cur_node == NULL){
        cur_node = new NetworkNode($1, $3);
        free($1);
        NetworkImpl::parsed_graph->add_Node(cur_node);
        potential_root.push_back(cur_node);
    }
    $$ = cur_node;
}

| error
{
    fprintf(stderr, "host parse error on line %d\n", linenum);
    YYABORT;
}
;

%%


void yyerror(const char *s)
{
  //fprintf(stderr, "%s\n", s);
}

} // namespace MRN
