/*
 * Copyright (c) 1996-2003 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: ParadynUI.h,v 1.1 2003/09/05 19:14:20 pcroth Exp $
#ifndef PARADYNUI_H
#define PARADYNUI_H

#include "common/h/Ident.h"
#include "UI.thread.SRVR.h"

class ParadynUI : public UIM
{
private:
    bool stdinIsTTY;
    bool inDeveloperMode;
    perfStreamHandle ps_handle;
    pdstring progName;

    // tunable const callbacks
    static void DeveloperModeCallback( bool newVal );

    // performance stream callbacks
    static void ResourceBatchChangedCallback( perfStreamHandle, batchMode);
    static void ApplicStateChangedCallback( perfStreamHandle, appState);
    static void EnableDataResponseCallback(pdvector<metricInstInfo>*,
                                                u_int,
                                                u_int);
    static void ResourceAddedCallback(perfStreamHandle,
                                        resourceHandle, 
                                        resourceHandle,
                                        const char*,
                                        const char*);
    static void ResourceRetiredCallback(perfStreamHandle,
                                        resourceHandle, 
                                        const char*,
                                        const char*);

protected:
    static const Ident V_id;
    static const Ident V_Uid;
    static const Ident V_Tid;

    perfStreamHandle    GetPerfStreamHandle( void ) const   { return ps_handle;}

    virtual bool IsDoneHandlingEvents( void ) const = NULL;
    virtual void DoPendingWork( void ) = NULL;
    virtual bool HandleEvent( thread_t mtid, tag_t mtag );
    virtual void Panic( pdstring msg );

    // performance stream handlers
    virtual void ResourceBatchChanged( perfStreamHandle,
                                        batchMode mode ) = NULL;
    virtual void ApplicStateChanged( perfStreamHandle h,
                                        appState s ) = NULL;
    virtual void ResourceAdded(perfStreamHandle h,
                        resourceHandle parent, 
                        resourceHandle newResource,
                        const char *name,
                        const char *abs) = NULL;
    virtual void ResourceRetired(perfStreamHandle h,
                                    resourceHandle uniqueID, 
                                    const char* name,
                                    const char *abs) = NULL;

    // igen interface implementation
    virtual void setDeveloperMode( bool newVal );
    virtual void DMready( void );

public:
    ParadynUI( pdstring progName );
    virtual ~ParadynUI( void );

    bool IsInDeveloperMode( void ) const    { return inDeveloperMode; }
    bool IsStdinTTY( void ) const           { return stdinIsTTY; }

    virtual bool Init( void );
    virtual void DoMainLoop( void );

    pdstring GetProgramName( void ) const   { return progName; }
};

#endif // PARADYNUI_H
