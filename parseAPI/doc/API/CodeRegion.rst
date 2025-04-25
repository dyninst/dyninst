Class CodeRegion
----------------

The CodeRegion interface is an accounting structure used to divide
CodeSources into distinct regions. This interface is mostly of interest
to CodeSource implementors.

void names(Address addr, vector<std::string> & names)

virtual bool findCatchBlock(Address addr, Address & catchStart)

Address low()

Address high()

bool contains(Address addr)

virtual bool wasUserAdded() const
