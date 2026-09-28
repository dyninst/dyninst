#include <boost/iterator/filter_iterator.hpp>
using boost::make_filter_iterator;
struct target_block
{
  Block* operator()(Edge* e) { return e->trg(); }
};


std::vector<Block*> work;
Intraproc epred; // ignore calls, returns

work.push_back(func->entry()); // assuming `func' is a Function*

// do_stuff is a functor taking a Block* as its argument
while (!work.empty())  {
  Block* b = work.back();
  work.pop_back();

  const Block::edgelist& targets = b->targets();
  // Do stuff for each out edge
  std::for_each(make_filter_iterator(epred, targets.begin(), targets.end()),
                make_filter_iterator(epred, targets.end(), targets.end()),
                do_stuff());
  std::transform(make_filter_iterator(epred, targets.begin(), targets.end()),
                 make_filter_iterator(epred, targets.end(), targets.end()),
                 std::back_inserter(work),
                 target_block());
  Block::edgelist::const_iterator found_interproc =
          std::find_if(targets.begin(), targets.end(), Interproc());
  if (found_interproc != targets.end())  {
    // do something with the interprocedural edge you found
  }
}
