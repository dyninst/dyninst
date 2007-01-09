/* 
 * Copyright (c) 1996-200666666 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 *
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 *
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

 #ifndef Dyn_Symtab_h
 #define Dyn_Symtab_h
 
 #include <sys/types.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <assert.h>
 #include <vector>
 #include <string>
 using namespace std;

#if defined(os_windows)			//On windows it is just hash_map otherwise its in ext/hash_map
	#include <hash_map>
	using namespace stdext;
#else
    #include <ext/hash_map>
	using namespace __gnu_cxx;
#endif
 
 #include "common/h/Types.h"
 #include "common/h/debugOstream.h"
 #include "common/h/headers.h"
 #include "symtabAPI/h/Dyn_Symbol.h"
 #include "symtabAPI/h/Object.h"

 DLLEXPORT string extract_path_tail(const string &path);
 class Dyn_Archive;
 typedef enum { lang_Unknown,
	        lang_Assembly,
	        lang_C,
	        lang_CPlusPlus,
	        lang_GnuCPlusPlus,
	        lang_Fortran,
	        lang_Fortran_with_pretty_debug,
	        lang_CMFortran
	      } supportedLanguages;
 
 typedef enum { Obj_Parsing,
 		Syms_To_Functions,
		Build_Function_Lists,
		No_Such_Function,
		No_Such_Variable,
		No_Such_Module,
		No_Such_Section,
		No_Such_Symbol,
		No_Such_Member,
		Not_A_File,
		Not_An_Archive
	      } SymtabError;

 class Dyn_Symtab;
 class Dyn_ExceptionBlock;
 //class lineDict;
 
 class Dyn_LookupInterface{
  public:
  	DLLEXPORT Dyn_LookupInterface() {}
	DLLEXPORT virtual bool getAllSymbolsByType(vector<Dyn_Symbol *> &ret, Dyn_Symbol::SymbolType sType) = 0;
	DLLEXPORT virtual bool findSymbolByType(vector<Dyn_Symbol *> &ret, const string &name,
				Dyn_Symbol::SymbolType sType, bool isMangled = false,
				bool isRegex = false, bool checkCase = false) = 0;
	DLLEXPORT virtual ~Dyn_LookupInterface(){}
 };
 
 class Dyn_Module : public Dyn_LookupInterface{
  public:
   	DLLEXPORT Dyn_Module(){}
   	DLLEXPORT Dyn_Module(supportedLanguages lang, OFFSET adr, string fullNm,
          	Dyn_Symtab *img);
	DLLEXPORT Dyn_Module(const Dyn_Module &mod):Dyn_LookupInterface(),fullName_(mod.fullName_),
		language_(mod.language_), addr_(mod.addr_),
		exec_(mod.exec_){}
	DLLEXPORT bool operator==(const Dyn_Module &mod) const{
		return ((language_==mod.language_)
			&&(addr_==mod.addr_)
			&&(fullName_==mod.fullName_)
			&&(exec_==mod.exec_));
	}

   	DLLEXPORT const string &fileName() const;
   	DLLEXPORT const string &fullName() const;
	DLLEXPORT bool setName(string newName)
	{
		fullName_ = newName;
		fileName_ = extract_path_tail(fullName_);
		return true;
	}
   	DLLEXPORT supportedLanguages language() const;
   	DLLEXPORT void setLanguage(supportedLanguages lang) {language_ = lang;}
   	DLLEXPORT OFFSET addr() const { return addr_; }
	DLLEXPORT Dyn_Symtab *exec() const;
	
	DLLEXPORT virtual bool getAllSymbolsByType(vector<Dyn_Symbol *> &ret, Dyn_Symbol::SymbolType sType);
	DLLEXPORT virtual bool findSymbolByType(vector<Dyn_Symbol *> &ret, const string &name,
				Dyn_Symbol::SymbolType sType, bool isMangled = false,
				bool isRegex = false, bool checkCase = false);
	
   	DLLEXPORT bool isShared() const;

	//bool parseFileLineInfo();
	//OFFSET& getAddrFromLine(unsigned linenum) const;
	DLLEXPORT ~Dyn_Module();

  private:
   	string fileName_;                   // short file 
   	string fullName_;                   // full path to file 
   	supportedLanguages language_;
   	OFFSET addr_;                      // starting address of module
	Dyn_Symtab *exec_;
	//lineDict *lineInfo;		    // address - line number mapping for the module
 };
 
 class Dyn_Symtab : public Dyn_LookupInterface{
	 
    /***** Public Member Functions *****/
    public:
    DLLEXPORT Dyn_Symtab();

	DLLEXPORT Dyn_Symtab(const Dyn_Symtab& obj);
	
    	DLLEXPORT bool openFile( string &filename, Dyn_Symtab *&obj);
	
	DLLEXPORT bool openFile( char *mem_image, size_t size, Dyn_Symtab *&obj);

	/***** Lookup Functions *****/
	DLLEXPORT virtual bool findSymbolByType(vector<Dyn_Symbol *> &ret, const string &name,
				Dyn_Symbol::SymbolType sType, bool isMangled = false,
				bool isRegex = false, bool checkCase = false);
	
	DLLEXPORT bool findFuncByEntryOffset(const OFFSET &offset, Dyn_Symbol *ret);	

	DLLEXPORT bool findModule(const string &name, Dyn_Module *&ret);
	DLLEXPORT bool findSection(const string &secName, Dyn_Section *&ret);
	DLLEXPORT bool findSectionByEntry(const OFFSET &offset, Dyn_Section *&ret);

	DLLEXPORT bool addSymbol(Dyn_Symbol *newsym);

	DLLEXPORT virtual bool getAllSymbolsByType(vector<Dyn_Symbol *> &ret, Dyn_Symbol::SymbolType sType);
	
	DLLEXPORT bool getAllModules(vector<Dyn_Module *>&ret);
	DLLEXPORT bool getAllSections(vector<Dyn_Section *>&ret);
	
	DLLEXPORT bool findException(OFFSET &addr, Dyn_ExceptionBlock &excp);
	DLLEXPORT bool getAllExceptions(vector<Dyn_ExceptionBlock *> &exceptions);
	DLLEXPORT bool findCatchBlock(Dyn_ExceptionBlock &excp, OFFSET addr, unsigned size = 0);

	DLLEXPORT bool getFuncBindingTable(vector<relocationEntry> &fbt) const;
	
	DLLEXPORT bool isExec() const;
 
	DLLEXPORT bool isCode(const OFFSET &where) const;
	DLLEXPORT bool isData(const OFFSET &where) const;
	DLLEXPORT bool isValidOffset(const OFFSET &where) const;

	DLLEXPORT bool isNativeCompiler() const;
	
	/***** Data Member Access *****/
	DLLEXPORT string file() const {return filename_;}
	DLLEXPORT string name() const {return name_;}

	DLLEXPORT char *  mem_image() const { return linkedFile.mem_image(); }
	
	DLLEXPORT OFFSET codeOffset() const;
	DLLEXPORT OFFSET dataOffset() const;
	DLLEXPORT OFFSET dataLength() const;
	DLLEXPORT OFFSET codeLength() const;
    	DLLEXPORT Word*  code_ptr ()  const;
    	DLLEXPORT Word*  data_ptr ()  const;

	DLLEXPORT unsigned getAddressWidth() const;
	DLLEXPORT OFFSET getLoadAddress() const { return linkedFile.getLoadAddress(); }
	DLLEXPORT OFFSET getEntryAddress() const;
	DLLEXPORT OFFSET getBaseAddress() const;
	DLLEXPORT OFFSET getTOCoffset() const { return linkedFile.getTOCoffset(); }
	
	DLLEXPORT unsigned getNumberofSections() const { return no_of_sections; }
    	DLLEXPORT unsigned getNumberofSymbols() const { return no_of_symbols; }

    #if defined(rs6000_ibm_aix4_1)||defined(rs6000_ibm_aix5_1)		
	void get_stab_info(char *&stabstr, int &nstabs, SYMENT *&stabs, char *&stringpool) const {
		return linkedFile.get_stab_info(stabstr,nstabs,stabs,stringpool);
	}
    
	void get_line_info(int& nlines, char*& lines,unsigned long& fdptr) const {
		return linkedFile.get_line_info(nlines,lines,fdptr);
	}
   #endif
   
	/***** Error Handling *****/
	DLLEXPORT SymtabError getLastSymtabError();
	DLLEXPORT string printError(SymtabError serr);

	DLLEXPORT ~Dyn_Symtab();
	
	void addFunctionName(Dyn_Symbol *func,
                            const string newName,
                            bool isMangled /*=false*/);
	void addVariableName(Dyn_Symbol *var,
                            const string newName,
                            bool isMangled /*=false*/);

	bool delSymbol(Dyn_Symbol *sym);		    
	
  #if defined(rs6000_ibm_aix4_1)||defined(rs6000_ibm_aix5_1) 	
    protected:
    	Dyn_Symtab(string &filename, string &member_name, OFFSET offset, bool &err);
	Dyn_Symtab(char *mem_image, size_t size, string &member_name, OFFSET offset, bool &err);

  #endif	
	 
    /***** Private Member Functions *****/
    private:
	DLLEXPORT Dyn_Symtab(string &filename, bool &err); 
	DLLEXPORT Dyn_Symtab(char *mem_image, size_t image_size, bool &err);
	DLLEXPORT bool extractInfo();

	bool buildDemangledName( const string &mangled, 
                          string &pretty,
                          string &typed,
                          bool nativeCompiler, 
                          supportedLanguages lang );
	bool symbolsToFunctions(vector<Dyn_Symbol *> *raw_funcs);
			       
	supportedLanguages pickLanguage(string &working_module, char *working_options, 
						supportedLanguages working_lang);
	void getModuleLanguageInfo(hash_map<string, supportedLanguages> *mod_langs);
	void setModuleLanguages(hash_map<string, supportedLanguages> *mod_langs);
	Dyn_Module *getOrCreateModule(const string &modName, 
				   const OFFSET modAddr);
	Dyn_Module *newModule(const string &name, const OFFSET addr, supportedLanguages lang);
	bool buildFunctionLists(vector <Dyn_Symbol *> &raw_funcs);
	void enterFunctionInTables(Dyn_Symbol *func, bool wasSymtab);
	bool addSymtabVariables();
	bool isInstructionAligned(const OFFSET &where) const;
	
	bool findFunction(vector <Dyn_Symbol *> &ret, const string &name, 
				bool isMangled=false, bool isRegex = false,
				bool checkCase = false);
	bool findVariable(vector <Dyn_Symbol *> &ret, const string &name,
				bool isMangled=false, bool isRegex = false,
				bool checkCase = false);
	bool findFuncVectorByPretty(const string &name, vector<Dyn_Symbol *> &ret);
	bool findFuncVectorByMangled(const string &name, vector<Dyn_Symbol *> &ret);
	bool findVarVectorByPretty(const string &name, vector<Dyn_Symbol *> &ret);
	bool findVarVectorByMangled(const string &name, vector<Dyn_Symbol *> &ret);

	bool findFuncVectorByMangledRegex(const string &rexp, bool checkCase,
							vector<Dyn_Symbol *>&ret);
	bool findFuncVectorByPrettyRegex(const string &rexp, bool checkCase,
							vector<Dyn_Symbol *>&ret);
	bool findVarVectorByMangledRegex(const string &rexp, bool checkCase,
							vector<Dyn_Symbol *>&ret);
	bool findVarVectorByPrettyRegex(const string &rexp, bool checkCase,
							vector<Dyn_Symbol *>&ret);
	bool getAllFunctions(vector<Dyn_Symbol *> &ret);
	bool getAllVariables(vector<Dyn_Symbol *> &ret);
	bool getAllSymbols(vector<Dyn_Symbol *> &ret);

    /***** Private Data Members *****/
    private:
    	string filename_;
	string member_name_;
	string name_;

        OFFSET codeOffset_;
        unsigned codeLen_;
        OFFSET dataOffset_;
        unsigned dataLen_;

        OFFSET codeValidStart_;
        OFFSET codeValidEnd_;
        OFFSET dataValidStart_;
        OFFSET dataValidEnd_;

        bool is_a_out;
        OFFSET main_call_addr_; // address of call to main()

        bool nativeCompiler;

        // data from the symbol table 
        Object linkedFile; 
	
	// A vector of all Dyn_Symtabs. Used to avoid duplicating
   	// a Dyn_Symtab that already exists.
	static vector<Dyn_Symtab *> allSymtabs;
	
	//sections
	unsigned no_of_sections;
	vector<Dyn_Section *> sections_;
	hash_map <OFFSET, Dyn_Section *> secsByEntryAddr;
	
	//symbols
	unsigned no_of_symbols;
	
	hash_map <OFFSET, Dyn_Symbol *> funcsByEntryAddr;
	// note, a prettyName is not unique, it may map to a function appearing
   	// in several modules.  Also only contains instrumentable functions....
   	hash_map <string, vector<Dyn_Symbol *>*> funcsByPretty;
   	// Hash table holding functions by mangled name.
   	// Should contain same functions as funcsByPretty....
   	hash_map <string, vector<Dyn_Symbol *>*> funcsByMangled;
   	// A way to iterate over all the functions efficiently
   	vector<Dyn_Symbol *> everyUniqueFunction;
   	// We make an initial list of functions based off the symbol table,
   	// and may create more when we actually analyze. Keep track of
   	// those created ones so we can distinguish them if necessary
   	vector<Dyn_Symbol *> createdFunctions;
   	// And the counterpart "ones that are there right away"
   	vector<Dyn_Symbol *> exportedFunctions;

	hash_map <string, Dyn_Module *> modsByFileName;
   	hash_map <string, Dyn_Module *> modsByFullName;
   	
	// Variables indexed by pretty (non-mangled) name
   	hash_map <string, vector <Dyn_Symbol *> *> varsByPretty;
   	hash_map <string, vector <Dyn_Symbol *> *> varsByMangled;
   	hash_map <OFFSET, Dyn_Symbol *> varsByAddr;
   	vector<Dyn_Symbol *> everyUniqueVariable;
   	//vector<Dyn_Symbol *> createdVariables;
   	//vector<Dyn_Symbol *> exportedVariables;
	vector<Dyn_Symbol *> modSyms;
	vector<Dyn_Symbol *> notypeSyms;
	vector<Dyn_Module *> _mods;

	//vector<relocationEntry *> relocation_table_;
	vector<Dyn_ExceptionBlock *> excpBlocks;
  private:
  	friend class Dyn_Archive;
 };

 /* Stores source code to address in text association for modules */
 /*class lineDict {
   public:
 	lineDict() : lineMap() { }
 	~lineDict() {}
   	void setLineAddr (unsigned line, OFFSET addr) { lineMap[line] = addr; }
	inline bool getLineAddr (const unsigned line, OFFSET &addr) 
	{
   		if (lineMap.find(line)==lineMap.end()) 
      			return false;
   		else 
		{
      			addr = lineMap[line];
      			return true;
		}
   	}

   private:
   	hash_map<unsigned, OFFSET> lineMap;
 };*/

 #endif
