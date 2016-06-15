old = ""
new = ""
with open("rose/rose-compat.h") as f:
    old = f.read().replace('\n', '')
with open("rose2/rose-compat.h") as f:
    new = f.read().replace('\n', '')

old = old.split("{")[1].split("}")[0]
new = new.split("{")[1].split("}")[0]

oldlist = set()
newlist = set()

oldvals = old.split(',')
for val in oldvals:
    oldlist.add(val.split('=')[0].strip())

newvals = new.split(',')
for val in newvals:
    newlist.add(val.split('=')[0].strip())

printset = oldlist.difference(oldlist.intersection(newlist))
for val in printset:
    print(val.replace("V_", ""))


'''
V_SgAsmPEImportLookupTable
V_SgAsmPEImportHNTEntry
V_SgAsmx86RegisterReferenceExpression
V_SgAsmTypeWord
V_SgAsmType80bitFloat
V_SgAsmFile
V_SgAsmTypeDoubleQuadWord
V_SgAsmByteValueExpression
V_SgAsmTypeSingleFloat
V_SgAsmPowerpcRegisterReferenceExpression
V_SgAsmSingleFloatValueExpression
V_SgUpcLocalsizeof
V_SgAsmDoubleWordValueExpression
V_SgAsmTypeQuadWord
V_SgAsmFunctionDeclaration
V_SgAsmType128bitFloat
V_SgAsmTypeDoubleFloat
V_SgAsmPEImportILTEntry
V_SgAsmDeclaration
V_SgUpcElemsizeof
V_SgAsmTypeDoubleWord
V_SgAsmTypeByte
V_SgAsmArmRegisterReferenceExpression
V_SgAsmPEImportHNTEntryList
V_SgAsmVectorValueExpression
V_SgAsmQuadWordValueExpression
V_SgAsmWordValueExpression
V_SgAsmTypeVector
V_SgAsmx86Instruction
V_SgBinaryFile
V_SgAsmDoubleFloatValueExpression
V_SgUpcBlocksizeof
V_SgAsmFieldDeclaration
V_SgAsmDataStructureDeclaration
V_SgAsmPEImportILTEntryList
'''