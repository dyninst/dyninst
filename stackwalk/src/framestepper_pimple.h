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

//We have lots of PIMPL (private implementation) classes for FrameSteppers.  This file
//contains a "template" for the common code.  Unfortunately, this isn't a real C++ template,
//as actual template classes wouldn't instanciate in the right place.  Instead we'll fake
//a template with macros.  I'm bad, but this would otherwise be a lot of repeated code.


#if defined(PIMPL_CLASS)
//Can optionally define PIMPL_IMPL_CLASS
//Can optionally define PIMPL_NAME

const char *PIMPL_CLASS::getName() const
{
#if defined(PIMPL_NAME)
   return PIMPL_NAME;
#else
   return "<ERROR>";
#endif
}

#if defined(PIMPL_IMPL_CLASS)

const char *PIMPL_IMPL_CLASS::getName() const
{
#if defined(PIMPL_NAME)
   return PIMPL_NAME;
#else
   return "<ERROR>";
#endif
}

PIMPL_CLASS::PIMPL_CLASS(Walker *w
#if defined(PIMPL_ARG1)
			 , PIMPL_ARG1 arg1
#endif
#if defined(PIMPL_ARG2)
			 , PIMPL_ARG2 arg2
#endif
			 ) :
  FrameStepper(w)
{

#if defined(PIMPL_NAME)
  sw_printf("[%s:%d] - Constructing " PIMPL_NAME " at %p\n",
	    FILE__, __LINE__, (void*)this);
#endif
  impl = new PIMPL_IMPL_CLASS(w, this
#if defined(PIMPL_ARG1)
			      , arg1
#endif
#if defined(PIMPL_ARG2)
			      , arg2
#endif
			      );
}

gcframe_ret_t PIMPL_CLASS::getCallerFrame(const Frame &in, Frame &out)
{
  if (!impl) {
#if defined(PIMPL_NAME)
    sw_printf("[%s:%d] - Error, " PIMPL_NAME " not implemented on this platform\n",
	      FILE__, __LINE__);
    setLastError(err_unsupported, PIMPL_NAME " not supported on this platform");
    return gcf_error;
#else
    return gcf_not_me;
#endif
  }
  return impl->getCallerFrame(in, out);
}

#if defined(OVERLOAD_NEWLIBRARY)
void PIMPL_CLASS::newLibraryNotification(LibAddrPair *la, lib_change_t change)
{
  if (!impl) {
#if defined(PIMPL_NAME)
    sw_printf("[%s:%d] - Error, " PIMPL_NAME " not implemented on this platform\n",
	      FILE__, __LINE__);
    setLastError(err_unsupported, PIMPL_NAME " not supported on this platform");
#endif
    return;
  }
  impl->newLibraryNotification(la, change);
}
#endif

unsigned PIMPL_CLASS::getPriority() const
{
  if (!impl) {
#if defined(PIMPL_NAME)
    sw_printf("[%s:%d] - Error, " PIMPL_NAME " not implemented on this platform\n",
	      FILE__, __LINE__);
    setLastError(err_unsupported, PIMPL_NAME " not supported on this platform");
#endif
    return 0;
  }
  return impl->getPriority();
}

void PIMPL_CLASS::registerStepperGroup(StepperGroup *group)
{
  if (!impl) {
#if defined(PIMPL_NAME)
    sw_printf("[%s:%d] - Error, " PIMPL_NAME " not implemented on this platform\n",
	      FILE__, __LINE__);
    setLastError(err_unsupported, PIMPL_NAME " not supported on this platform");
#endif
    return;
  }
  impl->registerStepperGroup(group);
}

PIMPL_CLASS::~PIMPL_CLASS()
{
#if defined(PIMPL_NAME)
  sw_printf("[%s:%d] - Destructing " PIMPL_NAME " at %p\n", FILE__, __LINE__, (void*)this);
#endif
  if (impl)
    delete impl;
  impl = NULL;
}

#else

PIMPL_CLASS::PIMPL_CLASS(Walker *w
#if defined(PIMPL_ARG1)
			 , PIMPL_ARG1
#endif
#if defined(PIMPL_ARG2)
			 , PIMPL_ARG2
#endif
			 ) :
  FrameStepper(w),
  impl(NULL)
{
}

gcframe_ret_t PIMPL_CLASS::getCallerFrame(const Frame &, Frame &)
{
#if defined(PIMPL_NAME)
  sw_printf("[%s:%d] - Error, " PIMPL_NAME " not implemented on this platform\n",
	    FILE__, __LINE__);
  setLastError(err_unsupported, PIMPL_NAME " not supported on this platform");
  return gcf_error;
#else
  return gcf_not_me;
#endif
}

#if defined(OVERLOAD_NEWLIBRARY)
void PIMPL_CLASS::newLibraryNotification(LibAddrPair *, lib_change_t)
{
#if defined(PIMPL_NAME)
  sw_printf("[%s:%d] - Error, " PIMPL_NAME " not implemented on this platform\n",
	    FILE__, __LINE__);
  setLastError(err_unsupported, PIMPL_NAME " not supported on this platform");
#endif
}
#endif

unsigned PIMPL_CLASS::getPriority() const
{
#if defined(PIMPL_NAME)
  sw_printf("[%s:%d] - Error, " PIMPL_NAME " not implemented on this platform\n",
	    FILE__, __LINE__);
  setLastError(err_unsupported, PIMPL_NAME " not supported on this platform");
#endif
  return 0;
}

void PIMPL_CLASS::registerStepperGroup(StepperGroup *)
{
#if defined(PIMPL_NAME)
  sw_printf("[%s:%d] - Error, " PIMPL_NAME " not implemented on this platform\n",
	    FILE__, __LINE__);
  setLastError(err_unsupported, PIMPL_NAME " not supported on this platform");
#endif
  return;
}

PIMPL_CLASS::~PIMPL_CLASS()
{
}

#endif

#endif

