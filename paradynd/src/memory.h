#ifndef _MEMORY_H
#define _MEMORY_H


class memory {
private:
   unsigned global_min;
   unsigned global_max;
   unsigned current_min ;
   unsigned current_max ;
   unsigned blk_size ;
   dictionary_hash <string, unsigned> upper_bounds ;
   dictionary_hash <string, unsigned> lower_bounds ;
   dictionary_hash <string, unsigned> current_upper_bounds ;
   dictionary_hash <string, unsigned> current_lower_bounds ;
public:
//member functions

   memory()
   : upper_bounds(string:: hash), 
     lower_bounds(string::hash),
     current_upper_bounds(string:: hash), 
     current_lower_bounds(string:: hash)
   {
	global_min =
	global_max =
	current_min =
	current_max = 0 ;
   };
   ~memory(){/* TO DO */ };

   //memory boundry
   typedef struct bounds_s {
		unsigned upper ;
		unsigned lower ;
   } bounds;
   //

    void     setBlkSize(unsigned s) {blk_size = s;} ;
    unsigned getBlkSize(void)       {return blk_size;} ;

    void updateGlobalBounds(int va, int mem_size)  
    // va is the virtual address
    {
	if(global_min==0 ||  (unsigned) va < global_min)
		global_min = (unsigned) va ;
	if(global_max==0 || (unsigned) (va + mem_size) > global_max)
		global_max = (unsigned) (va + mem_size) ;

    };

    void setCurrentBounds(string flat, int min, int max)
    {
	current_lower_bounds[flat] = min ;
	current_upper_bounds[flat] = max ;
	// TO DO 
	// current_lower_bounds, upper_bounds need to be
	// cleared each time, new instrumentation request
	// is sent. Add another rpc should suffice.
	if(current_min == (unsigned) 0 || (unsigned) min<current_min) 
		current_min = (unsigned) min ;
	if((unsigned) max>current_max) current_max = (unsigned) max ;
    };

    int getGlobalMax(void) 
    {
	return (int) global_max; 
    };

    int getGlobalMin(void) 
    {
	return (int) global_min; 
    };

    int getCurrentMax(void) 
    {
	return (int) current_max; 
    };

    int getCurrentMin(void) 
    {
	return (int) current_min; 
    };
    bounds getBoundsByUniqueName(string un) 
    {
	bounds b ;
	b.upper = current_upper_bounds[un] ;
	b.lower = current_lower_bounds[un] ;
	return b ;
    } ;
    bounds getVariableBounds(string trailing_res) 
    {
	bounds b ;
	b.upper = upper_bounds[trailing_res] ;
	b.lower = lower_bounds[trailing_res] ;
	return b ;
    };
    void   setVariableBounds(string trailing_res, bounds b) 
    {
	upper_bounds[trailing_res]=b.upper;
	lower_bounds[trailing_res]=b.lower;
    };
};

extern memory *theMemory ;
#endif
