
typedef enum { parsed_bool, parsed_statement } parse_ret;

extern parse_ret parse_type;

BPatch_variableExpr *findVariable(char *name);


#define YYDEBUG	1
