#include "thrtab_entries.h"

namespace pdthr
{

io_entity::io_entity(thread_t owned_by,
						int (*will_block_func)(void*),
						void* the_desc,
						bool is_desc_special)
  : owner(owned_by),
	will_block_fn(will_block_func),
	my_mail(NULL),
	special(is_desc_special),
	desc(the_desc)
{
	// nothing else to do
}


bool
io_entity::is_buffer_ready( void )
{ 
	return ((will_block_fn != NULL) && !(will_block_fn(desc)));
}

} // namespace pdthr

