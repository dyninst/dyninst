#ifndef _BPatch_sourceBlock_h_
#define _BPatch_sourceBlock_h_

#include <iostream.h>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"

/** this class represents the basic blocks in the source
  * code. The source code basic blocks are calculated according to the
  * machine code basic blocks. The constructors can be called by only
  * BPatch_flowGraph class since we do not want to make the user 
  * create source blocks that does not exist and we do not want the user
  * to change the start and end line numbers of the source block
  *
  * @see BPatch_flowGraph
  * @see BPatch_basicBlock
  */

class BPATCH_DLL_EXPORT BPatch_sourceBlock{
	friend class BPatch_flowGraph;
	friend ostream& operator<<(ostream&,BPatch_sourceBlock&);

private:
	const char* sourceFile;
	BPatch_Set<unsigned short>* sourceLines;

public:

	/** method to return source file name 
	  * @param i the number of source file requested */
	const char* getSourceFile();

	/** method to return source lines in the
	  * corresponding source file 
	  * @param i the number of source file requested */
	void getSourceLines(BPatch_Vector<unsigned short>&);

	/** destructor for the sourceBlock class */
	~BPatch_sourceBlock() {}

private:
	/** constructor of the class */
	BPatch_sourceBlock();
	BPatch_sourceBlock(const char*,BPatch_Set<unsigned short>&);
};

#endif /* _BPatch_sourceBlock_h_ */

