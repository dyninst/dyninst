%{
extern int ignoring;
#include "parse.h"

#define YYSTYPE union parse_stack

extern int parsing;
extern int yylex();
extern void *yyerror(char*);

%}

%token tIDENT tCOMMA tSTAR tNAME tBASE tUNS tUPCALL tASYNC
%token tLPAREN tRPAREN tSEMI tLBLOCK tRBLOCK 
%token tTYPEDEF tSTRUCT tVERSION tVIRTUAL
%token tCLASS tCOLON tCIGNORE tSIGNORE tLINE tCONST tIGNORE
%token tANGLE_L tANGLE_R tFREE tREF tABSTRACT tFORWARD
%%

completeDefinition: parsableUnitList { return 0;}
| error {
  char str[80];
  sprintf(str, "Sorry, unrecoverable error, exiting\n\n");
  yyerror(str);
  exit(0);
};

parsableUnitList: 
| parsableUnitList parsableUnit { parsing = 0; };

parsableUnit: interface_spec 
            | typeSpec;

interfacePreamble: interfaceName tLBLOCK interfaceBase interfaceVersion {
  interface_spec *is;

  is = new interface_spec($1.cp, $3.u, $4.u);
  delete ($1.cp);
  Options::current_interface = $$.spec = is;
} 

interface_spec: interfacePreamble definitionList tRBLOCK tSEMI { } ;

interfaceName: tIDENT { $$.cp = $1.cp; };

interfaceBase: tBASE tUNS tSEMI { $$.u = $2.u; };

interfaceVersion: tVERSION tUNS tSEMI { $$.u = $2.u; };

forward_spec: tFORWARD tIDENT tSEMI {
  Options::forward_decls += *$2.cp; delete $2.cp;
};

definitionList:
	      | definitionList definition
	      ;

optUpcall:   { $$.fd.call = remote_func::sync_call; $$.fd.is_virtual = false; }
     | tVIRTUAL { $$.fd.call = remote_func::async_call; $$.fd.is_virtual = true;}
     | tASYNC 	 { $$.fd.call = remote_func::async_call; $$.fd.is_virtual = false;}
     | tVIRTUAL tASYNC { $$.fd.call = remote_func::async_call; $$.fd.is_virtual = true;}
     | tUPCALL tASYNC { $$.fd.call = remote_func::async_upcall; $$.fd.is_virtual = false;}
     | tVIRTUAL tUPCALL tASYNC { $$.fd.call = remote_func::async_upcall;
				 $$.fd.is_virtual = true;};

optFree:        { $$.b = false;}
        | tFREE { $$.b = true;}

optRef:         { $$.b = false;}
        | tREF  { $$.b = true;};

definition: optFree optUpcall optConst typeName pointers optRef tIDENT tLPAREN arglist tRPAREN tSEMI {
  arg a($4.cp, $5.u, $3.b, NULL, $6.b);
  if ($5.u && $6.b) {
    char msg[80];
    sprintf(msg, "Cannot use a reference with pointers, goodbye\n");
    yyerror(msg);
    exit(0);
  }
  if (!Options::current_interface->new_remote_func($7.cp, $9.arg_vector,
						   $2.fd.call, $2.fd.is_virtual,
						   a, $1.b))
    abort();
  delete ($7.cp); delete ($4.cp); delete ($9.arg_vector);

} | tCIGNORE {
 Options::current_interface->ignore(false, $1.charp);
} | tSIGNORE {
 Options::current_interface->ignore(true, $1.charp);
};

optIgnore: tIGNORE { $$.cp = new string($1.charp);}
           |        { $$.cp = new string((char*)NULL); };

optAbstract:      { $$.b = false; }
           | tABSTRACT { $$.b = true; };

classOrStruct: optAbstract tCLASS { $$.class_data.b = true; $$.class_data.abs = $1.b;}
             | tSTRUCT { $$.class_data.b = false; $$.class_data.abs = false;};

typeSpec: classOrStruct tIDENT optDerived
                        tLBLOCK fieldDeclList optIgnore tRBLOCK tSEMI {
   // remove ignore tokens -- later
   // is derived? -- later
   if (($1.class_data.b || $3.derived.is_derived) &&
       Options::ml->address_space() == message_layer::AS_one) {
     char str[80];
     sprintf(str, "Not supported for single address spaces, you should not be doing this, goodbye!\n");
     yyerror(str);
     exit(0);
   }
   if (Options::types_defined(*$2.cp)) {
     char str[80];
     
     sprintf(str, "Duplicate type %s, exiting", ($2.cp)->string_of());
     yyerror(str);
     exit(0);
   }

   // FREE ALL MEMORY

   $$.cp = new string(Options::allocate_type(*$2.cp, $1.class_data.b, $1.class_data.abs,
					     $3.derived.is_derived,
					     *($3.derived.name),
					     type_defn::TYPE_COMPLEX, true, false,
					     $5.arg_vector, *$6.cp));
   delete ($2.cp); delete($3.derived.name); delete ($5.arg_vector); delete($6.cp);
   parsing = 0;
 }; 

optDerived: {$$.derived.is_derived = false; $$.derived.name = new string;}
          | tCOLON tIDENT {$$.derived.is_derived=true; $$.derived.name = $2.cp;};

fieldDeclList:  { $$.arg_vector = new vector<arg*>; }
| fieldDeclList fieldDecl {
  (*$1.arg_vector)+= $2.args;
};

fieldDecl: optConst typeName pointers tIDENT tSEMI {
  $$.args = new arg($2.cp, $3.u, $1.b, $4.cp, false);
  delete($2.cp); delete($4.cp);
};

typeName: tIDENT {

  if (!Options::types_defined(*$1.cp)) {
    char str[80];

    bool in_lib = false;
    bool is_forward = false;

    unsigned fsize = Options::forward_decls.size();
    for (unsigned u=0; u<fsize; u++) 
      if (Options::forward_decls[u] == *$1.cp) {
	is_forward = true; break;
      }

    if (Options::current_interface->are_bundlers_generated()) {
      if (!is_forward) {
	sprintf(str, "unknown type %s, exiting", ($1.cp)->string_of());
	yyerror(str);
	exit(0);
      }
    } else
      in_lib = true;

    if (!is_forward)
      $$.cp = new string(Options::allocate_type(*$1.cp, false, false,
						false, "",
						type_defn::TYPE_COMPLEX, true, in_lib));
    else
      $$.cp = new string(Options::type_prefix() + *$1.cp);

    delete ($1.cp);
  } else
    $$.cp = new string(Options::get_type(*$1.cp));
} | tIDENT tCOLON tCOLON tIDENT {
  string tname = *$1.cp + "::" + *$4.cp;
  if (!Options::types_defined(tname)) {
    char str[80];

    bool in_lib = false;
    if (Options::current_interface->are_bundlers_generated()) {
      sprintf(str, "unknown type %s, exiting", tname.string_of());
      yyerror(str);
      exit(0);
    } else
      in_lib = true;

    $$.cp = new string(Options::allocate_type(tname, false, false, false, "",
					      type_defn::TYPE_COMPLEX, true, in_lib));
  } else
    $$.cp = new string(Options::get_type(tname));
} | tIDENT tANGLE_L typeName pointers tANGLE_R {
  char str[80];
  int stl_index;
  bool in_lib = false;
  if (Options::current_interface->are_bundlers_generated()) {
    bool found = false;
    for (stl_index=0; stl_index<Options::stl_types.size(); stl_index++)
      if (Options::stl_types[stl_index].name == *$1.cp) {
	found = true;
	break;
      }
    if (!found) {
      sprintf(str, "unknown type %s, exiting", ($1.cp)->string_of());
      yyerror(str);
      exit(0);
    }
  }
  Options::el_data el_data;
  el_data.type = *$3.cp; el_data.stars = $4.u;
  el_data.name = el_data.type + Options::make_ptrs($4.u);

  in_lib = false;
  if (Options::current_interface->are_bundlers_generated()) {
    bool element_found = false;
    for (unsigned z=0; z<Options::stl_types[stl_index].elements.size(); z++) {
      if (Options::stl_types[stl_index].elements[z].name == el_data.name)
	element_found = true;
    }
    if (!element_found)
      Options::stl_types[stl_index].elements += el_data;
  } else
    in_lib = true;

  string tname = string(*$1.cp) + "<" + el_data.name + ">";
  Options::stl_seen = true;
  if (!Options::types_defined(tname)) {
    $$.cp = new string(Options::allocate_stl_type(*$1.cp, *$3.cp, $4.u, in_lib));
    delete ($1.cp); delete ($3.cp);
  } else
    $$.cp = new string(tname);
};

optConst:  { $$.b = false; }
        |  tCONST { $$.b = true;};

pointers:		 { $$.u = 0; }
	| tSTAR pointers { $$.u = $2.u + 1; };

// when is it ok to use pointers?
// if thread code is generated, it is always ok
// if xdr/pvm code is generated, pointers can be used for:
//       structs
//       classes
// ...but only 1 level of indirection
//

funcArg: optConst typeName pointers {
            $$.args = new arg($2.cp, $3.u, $1.b, NULL, false);
            delete ($2.cp);
	  } | optConst typeName pointers tIDENT {
	    // this may exit
	    $$.args = new arg($2.cp, $3.u, $1.b, $4.cp, false);
	    delete ($2.cp); delete($4.cp);
	  } | optConst typeName tREF tIDENT {
	    $$.args = new arg($2.cp, 0, $1.b, $4.cp, true);
	    delete ($2.cp); delete ($4.cp);
	  } | optConst typeName tREF {
	    $$.args = new arg($2.cp, 0, $1.b, NULL, true);
	    delete ($2.cp);
	  };

nonEmptyArg: funcArg {    
  $$.arg_vector = new vector<arg*>;
  (*$$.arg_vector)+= $1.args;
}
| nonEmptyArg tCOMMA funcArg {
  (*$1.arg_vector)+= $3.args;
};

arglist:		{ $$.arg_vector = new vector<arg*>; }
| nonEmptyArg	{
  $$.arg_vector = $1.arg_vector;
};
