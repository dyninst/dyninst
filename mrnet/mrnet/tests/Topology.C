#include "Topology.h"

namespace MRN {
Topology::Topology( std::string & topology_str )
  :num_nodes(0), num_internalnodes(0), num_leaves(0), _error( false )
{
    const char * cur_ptr;
	int cur_level;

	cur_ptr=topology_str.c_str();
	do{
	    unsigned int num_digits, tmp_num;
	    sscanf( cur_ptr, "%d", &cur_level );

        num_nodes += cur_level;
		levels.push_back( cur_level );

		//find out how many digits are in the number
		tmp_num = cur_level;
		num_digits=0;
		while( tmp_num != 0 ){
		    num_digits += 1;
			tmp_num /= 10;
		}

		//update cur_ptr by "num_digits" spaces
		cur_ptr += num_digits;
		if( *cur_ptr == '\0' ){
            num_leaves = cur_level;
		    break;
		}
		cur_ptr++;
	} while(1);

    num_internalnodes = num_nodes - num_leaves;
}

} /* namespace MRN */
