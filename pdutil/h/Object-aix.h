/************************************************************************
 * Object-aix.h: AIX object files.
************************************************************************/





#if !defined(_Object_aix_h_)
#define _Object_aix_h_





/************************************************************************
 * header files.
************************************************************************/

#include <util/h/Dictionary.h>
#include <util/h/Line.h>
#include <util/h/String.h>
#include <util/h/Symbol.h>
#include <util/h/Types.h>
#include <util/h/Vector.h>

extern "C" {
#include <a.out.h>
};

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>





/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject {
public:
             Object (const string, void (*)(const char *) = log_msg);
             Object (const Object &);
    ~Object ();

    Object&   operator= (const Object &);

private:
    void    load_object ();
};




#endif /* !defined(_Object_aix_h_) */
