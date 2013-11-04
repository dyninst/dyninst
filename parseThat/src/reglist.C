/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "reglist.h"

#include "log.h"
#include "utils.h"

reglist::~reglist()
{
    vector< regrule_t * >::iterator i;

    for (i = only_rules.begin(); i != only_rules.end(); ++i) {
	regfree( &((*i)->preg) );
	delete( *i );
    }

    for (i = skip_rules.begin(); i != skip_rules.end(); ++i) {
	regfree( &((*i)->preg) );
	delete( *i );
    }
}

bool reglist::insert(const char *pattern, rule_t ruletype, bool show_reason)
{
    regrule_t *newrule = new regrule_t;

    if (!newrule) {
	dlog(ERR, "Could not allocate memory for regrule_t in reglist::insert().");
	return false;
    }

    int retval = regcomp(&newrule->preg, pattern, REG_NOSUB);
    if (retval) {
	if (!print_regerror(&newrule->preg, retval))
	    dlog(ERR, "    Error in reglist::print_regerror() from reglist::insert()\n");
	return false;
    }
    newrule->pattern = pattern;
    newrule->show_reason = show_reason;

    switch (ruletype) {
    case RULE_ONLY: only_rules.push_back(newrule); break;
    case RULE_SKIP: skip_rules.push_back(newrule); break;
    }

    return true;
}

const char *reglist::getReason(char *s)
{
    vector< regrule_t * >::iterator i;

    if (only_rules.size() > 0) {
	for (i = only_rules.begin(); i != only_rules.end(); ++i) {
	    int retval = regexec(&((*i)->preg), s, 0, NULL, 0);

	    if (retval == 0) {
		if (!(*i)->show_reason) return NULL;
		return sprintf_static("Including '%s' because it matched regular expression '%s'", s, (*i)->pattern);
	    }

	    if (retval == REG_ENOSYS) {
		dlog(ERR, "Error while executing regular expression in reglist::check(): The function is not supported.\n");
		return NULL;
	    }
	}
	return NULL;
    }

    if (skip_rules.size() > 0) {
	for (i = skip_rules.begin(); i != skip_rules.end(); ++i) {
	    int retval = regexec(&((*i)->preg), s, 0, NULL, 0);

	    if (retval == 0 && (*i)->show_reason) {
		if (!(*i)->show_reason) return NULL;
		return sprintf_static("Skipping '%s' because it matched regular expression '%s'", s, (*i)->pattern);
	    }

	    if (retval == REG_ENOSYS) {
		dlog(ERR, "Error while executing regular expression in reglist::check(): The function is not supported.\n");
		return NULL;
	    }
	}
    }
    return NULL;
}

bool reglist::isValid(const char *s)
{
    vector< regrule_t * >::iterator i;

    if (only_rules.size() > 0) {
	for (i = only_rules.begin(); i != only_rules.end(); ++i) {
	    int retval = regexec(&((*i)->preg), s, 0, NULL, 0);

	    if (retval == 0) 
		return true;

	    if (retval == REG_ENOSYS) {
		dlog(ERR, "Error while executing regular expression in reglist::check(): The function is not supported.\n");
		return false;
	    }
	}
	return false;
    }

    if (skip_rules.size() > 0) {
	for (i = skip_rules.begin(); i != skip_rules.end(); ++i) {
	    int retval = regexec(&((*i)->preg), s, 0, NULL, 0);

	    if (retval == 0)
		return false;

	    if (retval == REG_ENOSYS) {
		dlog(ERR, "Error while executing regular expression in reglist::check(): The function is not supported.\n");
		return false;
	    }
	}
    }
    return true;
}

bool reglist::print_regerror(regex_t *preg, int num)
{
    static int len;
    static char *buf = NULL;

    int maxlen = regerror(num, preg, NULL, 0);

    if (maxlen == 0) {
	dlog(ERR, "Error while processing regular expression, but regerror() not implemented on this platform.\n");
	return false;

    } else if (maxlen > len) {
	char *newbuf = (char *)realloc(buf, maxlen);
	if (!newbuf) {
	    dlog(ERR, "Could not allocate %d bytes of memory for string in reglist::print_regerror().\n", maxlen);
	    return false;
	}
	buf = newbuf;
	len = maxlen;
    }

    regerror(num, preg, buf, len);
    dlog(ERR, "Error processing regular expression: %s\n", buf);
    return true;
}
