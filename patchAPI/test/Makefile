DIRS = findPoint \
insertSnippet \
removeSnippet \
modification \
mod_sharedlib \
proc_sharedlib \
process \
new_cfg \
proc_fork \
proc_integration \

all:
	for dir in $(DIRS); do \
		(cd $$dir; ${MAKE} all); done

clean:
	for dir in $(DIRS); do \
		(cd $$dir; ${MAKE} clean); done
