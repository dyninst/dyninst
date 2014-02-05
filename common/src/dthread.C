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

#include "common/src/dthread.h"
#include <assert.h>

#define duck_template(CLASS, FUN, R) \
class CLASS ## _ ## FUN ## er \
    : public boost::static_visitor<R> \
{ \
public: \
\
    template <typename T> \
    R operator()( T & operand ) const \
    {\
        return operand->FUN();\
    }\
\
}

duck_template(mutex, lock, void);
duck_template(mutex, unlock, void);
duck_template(mutex, try_lock, bool);

#undef duck_template

Mutex::Mutex(bool recursive)
{
	if (recursive)
		mutex = new boost::recursive_mutex();
	else
		mutex = new boost::mutex();
}

Mutex::~Mutex()
{
	void *ptr = boost::get<boost::mutex *>(mutex);
	if (!ptr)
		ptr = boost::get<boost::recursive_mutex *>(mutex);
	
	delete ptr;
}

bool Mutex::lock()
{
	boost::apply_visitor(mutex_locker(), mutex);
	return true;
}

bool Mutex::unlock()
{
	boost::apply_visitor(mutex_unlocker(), mutex);
	return true;
}

bool Mutex::trylock()
{
	return boost::apply_visitor(mutex_try_locker(), mutex);
}

CondVar::CondVar(Mutex *m) :
	cond(),
	created_mutex(false)
{
	if(m == NULL)
	{
		mutex = new Mutex();
		created_mutex = true;
	} else {
		mutex = m;
	}
}

CondVar::~CondVar()
{
	if(created_mutex)
	{
		delete mutex;
	}
}

bool CondVar::unlock()
{
	mutex->unlock();
	return true;
}

bool CondVar::lock()
{
	mutex->lock();
	return true;
}

bool CondVar::trylock()
{
	return mutex->trylock();
}

bool CondVar::signal()
{
	cond.notify_one();
	return true;
}

bool CondVar::broadcast()
{
	cond.notify_all();
	return true;
}


class cond_waiter
    : public boost::static_visitor<>
{
public:
    cond_waiter(boost::condition_variable_any *cv)
    {
        cond = cv;
    }

    template <typename T>
    void operator()( T & operand ) const
    {
	cond->wait(*operand);
    }
private:
    boost::condition_variable_any *cond;
};

bool CondVar::wait()
{
	boost::apply_visitor(cond_waiter(&cond), mutex->mutex);
	return true;
}
