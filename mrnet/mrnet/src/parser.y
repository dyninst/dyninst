%{
#include "common/h/list.h"
#include "common/src/list.C"
#include "mrnet/src/MC_NetworkGraph.h"
#include "mrnet/src/utils.h"

List <MC_NetworkNode *> hostlist;
List <MC_NetworkNode *> potential_root;
MC_NetworkGraph * graph = new MC_NetworkGraph;

#if defined(__cplusplus)
extern "C" {
void yyerror(const char *s);
}
#endif
int yylex(void);

#define YYDEBUG 1
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
	    if(potential_root.count() != 1){
	      fprintf(stderr, "graph is not connected\n");
	      YYABORT;
	    }	   
            List<MC_NetworkNode *>::iterator iter=potential_root.begin();
	    //graph->set_Root(*iter);
	    fprintf(stderr, "graph's root is %s:%hd\n",
                       (*iter)->get_HostName().c_str(), (*iter)->get_Port() );
	  }

line: host ARROW hosts SEMI
        {
	  fprintf(stderr, "%s:%hd's children are:",$1->get_HostName().c_str(),
		     $1->get_Port() );
          List<MC_NetworkNode *>::iterator iter=hostlist.begin();
          for(; iter != hostlist.end(); iter++){
	    MC_NetworkNode * cur_node;
	    cur_node = (*iter);
	    potential_root.remove(cur_node); //node cannot be a root, remove
	    //$1->add_Child(cur_node);
	    fprintf(stderr, " %s:%hd", cur_node->get_HostName().c_str(),
		       cur_node->get_Port() );
	  }
	  fprintf(stderr, "\n");
	  hostlist.clear();
	}

hosts: host hosts
         {
	   hostlist.push_back($1);
	 }
     | host
         {
	   hostlist.push_back($1);
	 }


host: HOSTNAME COLON PORT
        {
          MC_NetworkNode * cur_node = graph->find_Node($1, $3);
          if(cur_node == NULL){
            cur_node = new MC_NetworkNode($1, $3);
            graph->add_Node(cur_node);
	    potential_root.push_back(cur_node);
          }
          $$ = cur_node;
	  fprintf(stderr, "Reducing %s:%hd\n", cur_node->get_HostName().c_str(),
		     cur_node->get_Port() );
        }

%%

extern int yydebug;
int main(int, char **){
  //yydebug=1;
  yyparse();
  return 0;
}

void yyerror(const char *s)
{
  fprintf(stderr, "%s\n", s);
}
