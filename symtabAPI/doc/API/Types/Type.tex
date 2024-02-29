\subsection{Class Type}

The class \code{Type} represents the types of variables, parameters, return values, and functions. Instances of this class can represent language predefined types (e.g. \code{int}, \code{float}), already defined types in the Object File or binary (e.g., structures compiled into the binary), or newly created types (created using the create factory methods of the corresponding type classes described later in this section) that are added to SymtabAPI by the user. 

As described in Section \ref{subsec:typeInterface}, this class serves as a base class for all the other classes in this interface. An object of this class is returned from type look up operations performed through the Symtab class described in Section \ref{sec:symtabAPI}. The user can then obtain the specific type object from the generic Type class object. The following example shows how to get the specific object from a given \code{Type} object returned as part of a look up operation.

\begin{lstlisting}
//Example shows how to retrieve a structure type object from a given ``Type'' object
using namespace Dyninst;
using namespace SymtabAPI;

//Obj represents a handle to a parsed object file using symtabAPI
//Find a structure type in the object file
Type *structType = obj->findType(``structType1'');

// Get the specific typeStruct object
typeStruct *stType = structType->isStructType();
\end{lstlisting}

\begin{apient}
string &getName()
\end{apient}
\apidesc{
This method returns the name associated with this type.
Each of the types is represented by a symbolic name. This method retrieves the
name for the type. For example, in the example above "structType1"
represents the name for the \code{structType} object.
}

\begin{apient}
bool setName(string zname)
\end{apient}
\apidesc{
This method sets the name of this type to name. Returns \code{true} on success and \code{false} on failure.
}

\begin{apient}
typedef enum{ 
    dataEnum,
    dataPointer,
    dataFunction,
    dataSubrange,
    dataArray,
    dataStructure,
    dataUnion,
    dataCommon,
    dataScalar,
    dataTypedef,
    dataReference,
    dataUnknownType,
    dataNullType,
    dataTypeClass
} dataClass;
\end{apient}

\begin{apient}
dataClass getDataClass()
\end{apient}
\apidesc{
This method returns the data class associated with the type. 
This value should be used to convert this generic type object to a specific type object which offers more functionality by using the corresponding query function described later in this section. For example, if this method returns \code{dataStructure} then the \code{isStructureType()} should be called to dynamically cast the \code{Type} object to the \code{typeStruct} object.
}

\begin{apient}
typeId_t getID()
\end{apient}
\apidesc{
This method returns the ID associated with this type.
Each type is assigned a unique ID within the object file. For example an integer scalar built-in type is assigned an ID -1.
}

\begin{apient}
unsigned getSize()
\end{apient}
\apidesc{
This method returns the total size in bytes occupied by the type.
}

\begin{apient}
typeEnum *getEnumType()
\end{apient}
\apidesc{
If this \code{Type} hobject represents an enum type, then return the object casting the \code{Type} object to \code{typeEnum} otherwise return \code{NULL}.
}

\begin{apient}
typePointer *getPointerType()
\end{apient}
\apidesc{
If this \code{Type} object represents an pointer type, then return the object casting the \code{Type} object to \code{typePointer} otherwise return \code{NULL}.
}

\begin{apient}
typeFunction *getFunctionType()
\end{apient}
\apidesc{
If this \code{Type} object represents an \code{Function} type, then return the object casting the \code{Type} object to \code{typeFunction} otherwise return \code{NULL}.
}

\begin{apient}
typeRange *getSubrangeType()
\end{apient}
\apidesc{
If this \code{Type} object represents a \code{Subrange} type, then return the object casting the \code{Type} object to \code{typeSubrange} otherwise return \code{NULL}.
}

\begin{apient}
typeArray *getArrayType()
\end{apient}
\apidesc{
If this \code{Type} object represents an \code{Array} type, then return the object casting the \code{Type} object to \code{typeArray} otherwise return \code{NULL}.
}

\begin{apient}
typeStruct *getStructType()
\end{apient}
\apidesc{
If this \code{Type} object represents a \code{Structure} type, then return the object casting the \code{Type} object to \code{typeStruct} otherwise return \code{NULL}.
}

\begin{apient}
typeUnion *getUnionType()
\end{apient}
\apidesc{
If this \code{Type} object represents a \code{Union} type, then return the object casting the \code{Type} object to \code{typeUnion} otherwise return \code{NULL}.
}

\begin{apient}
typeScalar *getScalarType()
\end{apient}
\apidesc{
If this \code{Type} object represents a \code{Scalar} type, then return the object casting the \code{Type} object to \code{typeScalar} otherwise return \code{NULL}.
}

\begin{apient}
typeCommon *getCommonType()
\end{apient}
\apidesc{
If this \code{Type} object represents a \code{Common} type, then return the object casting the \code{Type} object to \code{typeCommon} otherwise return \code{NULL}.
}

\begin{apient}
typeTypedef *getTypedefType()
\end{apient}
\apidesc{
If this \code{Type} object represents a \code{TypeDef} type, then return the object casting the \code{Type} object to \code{typeTypedef} otherwise return \code{NULL}.
}

\begin{apient}
typeRef *getRefType()
\end{apient}
\apidesc{
If this \code{Type} object represents a \code{Reference} type, then return the object casting the \code{Type} object to \code{typeRef} otherwise return \code{NULL}.
}

\subsection{Class typeEnum}
This class represents an enumeration type containing a list of constants with values. This class is derived from \code{Type}, so all those member functions are applicable. \code{typeEnum} inherits from the \code{Type} class.

\begin{apient}
static typeEnum *create(string &name,
                        vector<pair<string, int> *> &consts, 
                        Symtab *obj = NULL) 
static typeEnum *create(string &name,
                        vector<string> &constNames,
                        Symtab *obj)
\end{apient}
\apidesc{
These factory methods create a new enumerated type. There are two variations to this function. \code{consts} supplies the names and Id's of the constants of the enum. The first variant is used when user-defined identifiers are required; the second variant is used when system-defined identifiers will be used. 
The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool addConstant(const string &constname,
                 int value)
\end{apient}
\apidesc{
This method adds a constant to an enum type with name \code{constName} and value \code{value}. 
Returns \code{true} on success and \code{false} on failure.
}

\begin{apient}
std::vector<std::pair<std::string, int> > &getConstants();
\end{apient}
\apidesc{
This method returns the vector containing the enum constants represented by a (name, value) pair of the constant.
}

\begin{apient}
bool setName(const char* name)
\end{apient}
\apidesc{
This method sets the new name of the enum type to \code{name}. 
Returns \code{true} if it succeeds, else returns \code{false}.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
 This method returns \code{true} if the enum type is compatible with the given type \code{type} or else returns \code{false}. 
 }

 \subsection{Class typeFunction}
This class represents a function type, containing a list of parameters and a return type. This class is derived from \code{Type}, so all the member functions of class \code{Type} are applicable. \code{typeFunction} inherits from the \code{Type} class.


\begin{apient}
static typeFunction *create(string &name,
                            Type *retType, 
                            vector<Type *> &paramTypes,
                            Symtab *obj = NULL)
\end{apient}
\apidesc{
This factory method creates a new function type with name \code{name}. \code{retType} represents the return type of the function and \code{paramTypes} is a vector of the types of the parameters in order. 
The the newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if the function type is compatible with the given type \code{type} or else returns \code{false}. 
}

\begin{apient}
bool addParam(Type *type)
\end{apient}
\apidesc{
This method adds a new function parameter with type \code{type} to the function type. 
Returns \code{true} if it succeeds, else returns \code{false}.
}

\begin{apient}
Type *getReturnType() const
\end{apient}
\apidesc{
 This method returns the return type for this function type. Returns \code{NULL} if there is no return type associated with this function type.
}

\begin{apient}
bool setRetType(Type *rtype)
\end{apient}
\apidesc{
This method sets the return type of the function type to \code{rtype}. Returns \code{true} if it succeeds, else returns \code{false}.
}

\begin{apient}
bool setName(string &name)
\end{apient}
\apidesc{
This method sets the new name of the function type to \code{name}. Returns \code{true} if it succeeds, else returns \code{false}.
}

\begin{apient}
vector< Type *> &getParams() const
\end{apient}
\apidesc{
This method returns the vector containing the individual parameters represented by their types in order. Returns \code{NULL} if there are no parameters to the function type.
}

\subsection{Class typeScalar}

This class represents a scalar type. This class is derived from \code{Type}, so all the
member functions of class \code{Type} are applicable. \code{typeScalar} inherits from the Type
class.  

\begin{apient}
static typeScalar *create(string &name, int size, Symtab *obj = NULL)
\end{apient}
\apidesc{
This factory method creates a new scalar type. The \code{name} field is used to specify
the name of the type, and the \code{size} parameter is used to specify the size in
bytes of each instance of the type.  The newly created type is added to the
\code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but
it will be available for further queries.
}

\begin{apient}
bool isSigned()
\end{apient}
\apidesc{
This method returns \code{true} if the scalar type is signed or else returns \code{false}.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if the scalar type is compatible with the given type \code{type} or else returns \code{false}. 
}

\subsection{Class Field}
This class represents a field in a container. For e.g. a field in a structure/union type.


\begin{apient}
typedef enum {  
    visPrivate, 
    visProtected,
    visPublic,
    visUnknown
} visibility_t;
\end{apient}
\apidesc{
A handle for identifying the visibility of a certain \code{Field} in a container type. This can represent private, public, protected or unknown(default) visibility.
}

\begin{apient}
Field(string &name,
      Type *type,
      visibility_t vis = visUnknown)
\end{apient}
\apidesc{
This constructor creates a new field with name \code{name}, type \code{type} and visibility \code{vis}. This newly created \code{Field} can be added to a container type.
}

\begin{apient}
const string &getName()
\end{apient}
\apidesc{
This method returns the name associated with the field in the container.
}

\begin{apient}
Type *getType()
\end{apient}
\apidesc{
 This method returns the type associated with the field in the container.
}

\begin{apient}
int getOffset()
\end{apient}
\apidesc{
This method returns the offset associated with the field in the container.
}

\begin{apient}
visibility_t getVisibility()
\end{apient}
\apidesc{
This method returns the visibility associated with a field in a container.
This returns \code{visPublic} for the variables within a common block.
}

\subsection{Class fieldListType}

This class represents a container type. It is one of the three categories of types as described in Section \ref{subsec:typeInterface}. The structure and the union types fall under this category. This class is derived from \code{Type}, so all the member functions of class \code{Type} are applicable. \code{fieldListType} inherits from the \code{Type} class.


\begin{apient}
vector<Field *> *getComponents()
\end{apient}
\apidesc{
This method returns the list of all fields present in the container. 
This gives information about the name, type and visibility of each of the fields. Returns \code{NULL} of there are no fields.
}

\begin{apient}
void addField(std::string fieldname,
              Type *type,
              int offsetVal = -1, 
              visibility_t vis = visUnknown)
\end{apient}
\apidesc{
This method adds a new field at the end to the container type with field name \code{fieldname}, type \code{type} and type visibility \code{vis}.
}

\begin{apient}
void addField(unsigned num,
              std::string fieldname,
              Type *type,
              int offsetVal = -1, 
              visibility_t vis = visUnknown)         
\end{apient}
\apidesc{
This method adds a field after the field with number \code{num} with field name \code{fieldname}, type \code{type} and type visibility \code{vis}.
}

\begin{apient}
void addField(Field *fld)
\end{apient}
\apidesc{
This method adds a new field \code{fld} to the container type.}

\begin{apient}
void addField(unsigned num,
              Field *fld)
\end{apient}
\apidesc{
This method adds a field \code{fld} after field \code{num} to the container type.
}

\subsubsection{Class typeStruct : public fieldListType}
\mbox{ }\\

This class represents a structure type. The structure type is a special case of
the container type. The fields of the structure represent the fields in this
case. As a subclass of class \code{fieldListType}, all methods in \code{fieldListType} are
applicable.


\begin{apient}
static typeStruct *create(string &name,
                          vector<pair<string, Type *>*> &flds,
                          Symtab *obj = NULL)
\end{apient}
\apidesc{
This factory method creates a new struct type. The name of the structure is specified in the \code{name} parameter. The \code{flds} vector specifies the names and types of the fields of the structure type. 
The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
static typeStruct *create(string &name,
                          vector<Field *> &fields,
                          Symtab *obj = NULL)
\end{apient}
\apidesc{
This factory method creates a new struct type. The name of the structure is specified in the \code{name} parameter. The \code{fields} vector specifies the fields of the type. 
The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if the struct type is compatible with the given type \code{type} or else returns \code{false}. 
}

\subsubsection{Class typeUnion}
\mbox{ }\\

This class represents a union type, a special case of the container type. The fields of the union type represent the fields in this case. As a subclass of class \code{fieldListType}, all methods in \code{fieldListType} are applicable. \code{typeUnion} inherits from the \code{fieldListType} class.


\begin{apient}
static typeUnion *create(string &name,
                         vector<pair<string, Type *>*> &flds,
                         Symtab *obj = NULL)
\end{apient}
\apidesc{
 This factory method creates a new union type. The name of the union is specified in the \code{name} parameter. The \code{flds} vector specifies the names and types of the fields of the union type. 
The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
static typeUnion *create(string &name,
                         vector<Field *> &fields, 
                         Symtab *obj = NULL)
\end{apient}
\apidesc{
This factory method creates a new union type. The name of the structure is specified in the \code{name} parameter. The \code{fields} vector specifies the fields of the type.
The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if the union type is compatible with the given type \code{type} or else returns \code{false}. 
}

\subsubsection{Class typeCommon}
\mbox{ }\\

This class represents a common block type in fortran, a special case of the container type. The variables of the common block represent the fields in this case. As a subclass of class \code{fieldListType}, all methods in \code{fieldListType} are applicable. \code{typeCommon} inherits from the \code{Type} class.


\begin{apient}
vector<CBlocks *> *getCBlocks()
\end{apient}
\apidesc{
This method returns the common block objects for the type. 
The methods of the \code{CBlock} can be used to access information about the members of a common block. The vector returned by this function contains one instance of \code{CBlock} for each unique definition of the common block.
}

\subsubsection{Class CBlock}
\mbox{ }\\

This class represents a common block in Fortran. Multiple functions can share a common block.

\begin{apient}
bool getComponents(vector<Field *> *vars)
\end{apient}
\apidesc{
This method returns the vector containing the individual variables of the common block. 
Returns \code{true} if there is at least one variable, else returns \code{false}.
}

\begin{apient}
bool getFunctions(vector<Symbol *> *funcs)
\end{apient}
\apidesc{
This method returns the functions that can see this common block with the set of variables described in \code{getComponents} method above.
Returns \code{true} if there is at least one function, else returns \code{false}.
}

\subsection{Class derivedType}
This class represents a derived type which is a reference to another type. It is one of the three categories of types as described in Section \ref{subsec:typeInterface}. The pointer, reference and the typedef types fall under this category. This class is derived from \code{Type}, so all the member functions of class \code{Type} are applicable.


\begin{apient}
Type *getConstituentType() const
\end{apient}
\apidesc{
 This method returns the type of the base type to which this type refers to.
}

\subsubsection{Class typePointer}
\mbox{ }\\

This class represents a pointer type, a special case of the derived type. The base type in this case is the type this particular type points to. As a subclass of class \code{derivedType}, all methods in \code{derivedType} are also applicable. 


\begin{apient}
static typePointer *create(string &name,
                           Type *ptr,
                           Symtab *obj = NULL)
static typePointer *create(string &name,
                           Type *ptr,
                           int size,
                           Symtab *obj = NULL)
\end{apient}
\apidesc{
These factory methods create a new type, named \code{name}, which points to objects of type \code{ptr}. The first form creates a pointer whose size is equal to sizeof(void*) on the target platform where the application is running. In the second form, the size of the pointer is the value passed in the \code{size} parameter. 
The newly created type is added to the \code{Symtab} object \code{obj}. If obj is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if the Pointer type is compatible with the given type \code{type} or else returns \code{false}. 
}

\begin{apient}
bool setPtr(Type *ptr)
\end{apient}
\apidesc{
This method sets the pointer type to point to the type in \code{ptr}. Returns \code{true} if it succeeds, else returns \code{false}.
}

\subsubsection{Class typeTypedef}
\mbox{ }\\

This class represents a \code{typedef} type, a special case of the derived type. The base type in this case is the \code{Type}. This particular type is typedefed to. As a subclass of class \code{derivedType}, all methods in \code{derivedType} are also applicable.


\begin{apient}
static typeTypedef *create(string &name,
                           Type *ptr,
                           Symtab *obj = NULL) 
\end{apient}
\apidesc{
This factory method creates a new type called \code{name} and having the type \code{ptr}.
The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if the typedef type is compatible with the given type \code{type} or else returns \code{false}. 
}

\subsubsection{Class typeRef }
\mbox{ }\\

This class represents a reference type, a special case of the derived type. The base type in this case is the \code{Type} this particular type refers to. As a subclass of class \code{derivedType}, all methods in \code{derivedType} are also applicable here. 


\begin{apient}
static typeRef *create(string &name,
                       Type *ptr,
                       Symtab * obj = NULL)
\end{apient}
\apidesc{
This factory method creates a new type, named \code{name}, which is a reference to objects of type \code{ptr}. The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if the ref type is compatible with the given type \code{type} or else returns \code{false}. 
}

\subsection{Class rangedType}
This class represents a range type with a lower and an upper bound. It is one of the three categories of types as described in section \ref{subsec:typeInterface}. The sub-range and the array types fall under this category. This class is derived from \code{Type}, so all the member functions of class \code{Type} are applicable.


\begin{apient}
unsigned long getLow() const
\end{apient}
\apidesc{
This method returns the lower bound of the range. 
This can be the lower bound of the range type or the lowest index for an array type.
}

\begin{apient}
unsigned long getHigh() const
\end{apient}
\apidesc{
This method returns the higher bound of the range. 
This can be the higher bound of the range type or the highest index for an array type.
}

\subsubsection{Class typeSubrange}
\mbox{ }\\

This class represents a sub-range type. As a subclass of class \code{rangedType}, all methods in \code{rangedType} are applicable here. This type is usually used to represent a sub-range of another type. For example, a \code{typeSubrange} can represent a sub-range of the array type or a new integer type can be declared as a sub range of the integer using this type.


\begin{apient}
static typeSubrange *create(string &name,
                            int size,
                            int low,
                            int hi, 
                            symtab *obj = NULL) 
\end{apient}
\apidesc{
This factory method creates a new sub-range type. The name of the type is \code{name}, and the size is \code{size}. The lower bound of the type is represented by \code{low}, and the upper bound is represented by \code{high}. 
The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if this sub range type is compatible with the given type \code{type} or else returns \code{false}. 
}

\subsubsection{Class typeArray}
\mbox{ }\\

This class represents an \code{Array} type. As a subclass of class \code{rangedType}, all methods in \code{rangedType} are applicable. 

\begin{apient}
static typeArray *create(string &name,
                         Type *type,
                         int low,
                         int hi,
                         Symtab *obj = NULL) 
\end{apient}
\apidesc{
This factory method creates a new array type. The name of the type is \code{name}, and the type of each element is \code{type}. The index of the first element of the array is \code{low}, and the last is \code{high}. 
The newly created type is added to the \code{Symtab} object \code{obj}. If \code{obj} is \code{NULL} the type is not added to any object file, but it will be available for further queries.
}

\begin{apient}
bool isCompatible(Type *type)
\end{apient}
\apidesc{
This method returns \code{true} if the array type is compatible with the given type \code{type} or else returns \code{false}. 
}

\begin{apient}
Type *getBaseType() const
\end{apient}
\apidesc{
This method returns the base type of this array type.
}
