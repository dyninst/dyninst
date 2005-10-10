/*
 * Copyright (c) 1996-2004 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "pdutil/h/resource.h"
/************************************************************************
 *
 * RTMPI.c: platform independent runtime instrumentation functions 
 * for MPI
 *
 ************************************************************************/

/*
 * maximum number of resource creations to track when checking for
 * a high resource creation rate
 *
 * this value should be large enough that if we stop tracking creations
 * because we've run out of slots, we would likely throttle the 
 * resource creation anyway
 */
#define	DYNINSTmaxResourceCreates			1000

/* 
 * length of the sliding window used to check for high rates of
 * message tag creation
 * units are seconds
 */
#define	DYNINSTtagCreateRateWindowLength	(10)

/* the value used to indicate an undefined tag */
#define	DYNINSTGroupUndefinedTag			(-999)


/*
 * limit on the creation rate of message tags within a group
 * units are number of creations per second
 */
#define	DYNINSTtagCreationRateLimit			(1.0)

/*
 * DYNINSTresourceCreationInfo
 * 
 * Maintains info about the number of resource discovery events 
 * within a sliding window.
 */
typedef struct DYNINSTresourceCreationInfo_
{
	rawTime64		creates[DYNINSTmaxResourceCreates];	/* time of create events */
	unsigned int	nCreatesInWindow;
	double			windowLength;		/* in seconds */
	unsigned int	head;
	unsigned int	tail;
} DYNINSTresourceCreationInfo;

/*
 * DYNINSTresourceCreationInfo_Init
 *
 * Initialize a DYNINSTresourceCreationInfo to be empty.
 */
void
DYNINSTresourceCreationInfo_Init( DYNINSTresourceCreationInfo* ci,
								double windowLength )
{
	ci->windowLength = windowLength;
	ci->nCreatesInWindow = 0;
	ci->head = 0;
	ci->tail = 0;
}

/*
 * DYNINSTresourceCreationInfo_Add
 *
 * Record a resource creation event.  Has the effect of sliding
 * the sliding window forward to the indicated time.
 */
int
DYNINSTresourceCreationInfo_Add( DYNINSTresourceCreationInfo* ci,
									rawTime64 ts )
{
	int ret = 0;

	if( ci->nCreatesInWindow < (DYNINSTmaxResourceCreates - 1) )
	{
		/*
		 * ts is the leading edge of the sliding window
		 * determine new trailing edge of the sliding window
		 *
		 * Note: assumes windowLength is in seconds, and ts in nanoseconds
		 */
		rawTime64 trailingEdge = (rawTime64)(ts - (ci->windowLength * 1000000000));

		/* drop any creation events that have fallen out of the window */
		while( (ci->nCreatesInWindow > 0) && (ci->creates[ci->tail] < trailingEdge))
		{
			ci->tail = (ci->tail + 1) % DYNINSTmaxResourceCreates;
			ci->nCreatesInWindow--;
		}

		/* add the new creation event */
		if( ci->nCreatesInWindow == 0 )
		{
			/* window was empty - reset to beginning of array */
			ci->head = 0;
			ci->tail = 0;
		}
		else
		{
			ci->head = (ci->head + 1) % DYNINSTmaxResourceCreates;
		}
		ci->creates[ci->head] = ts;
		ci->nCreatesInWindow++;

		ret = 1;
	}
	return ret;
}

/* Message groups */
void PARADYNtagGroupInfo_Init( void );
static int PARADYNGroup_CreateUniqueId(int,int);
static void PARADYNrecordTagGroupInfo(int,unsigned);
int PARADYNGroup_FindUniqueId(unsigned gId, char * constraint);

/* for MPI RMA Window support */
struct ParadynWin_st;
void PARADYNWinInfo_Init( void );
static void PARADYNreportNewWindow(const struct ParadynWin_st *);
static unsigned int PARADYNWindow_CreateUniqueId(unsigned int, int);

/*****************************************************************************
 * Support for discovering new communication groups and message tags.
 * and MPI Windows, too
 ****************************************************************************/

#define DYNINSTagsLimit      1000
#define DYNINSTagGroupsLimit 100
#define DYNINSTNewTagsLimit   50 /* don't want to overload the system */

#define DYNINSTWindowsLimit   500

typedef struct ParadynWin_st {
   unsigned int   WinId;        /* winId as used by the program */
   unsigned int   WinUniqueId;     /* our unique identifier for the window */
   /* A Window is identifed by a combination of WinId and WinUniqueId. */
   char * WinName;                   /* name of window given by the user */
   struct ParadynWin_st* Next;       /* next defined window info struct */
} ParadynWinSt;

#define LAM 0
#define MPICH 1

int whichMPI = -1;
char *whichMPIenv = NULL;

typedef struct {
   int            NumWins;     /* Number of Windows */
   struct ParadynWin_st*  WindowTable[DYNINSTWindowsLimit]; /* Window table */
   /* WindowCounters is an array that maintains the 'next' unique number
      for windows that fall into the same index in WindowTable.  This is
      because the MPI implementation may reuse a WinId if a window is 
      destroyed with MPI_Win_free.  When a window is freed we will remove its 
      entry from WindowTable.  That way when the WinId is reused in a 
      MPI_Win_create call, we won't find it in the WindowTable. We will   
      know it is a new resource.
   */
   int WindowCounters[DYNINSTWindowsLimit];
} DynInstWinArraySt;


typedef struct DynInstTag_st {
   int      TagGroupId;              /* group as used by the program */
   unsigned TGUniqueId;             /* our unique identifier for the group */
   int      NumTags;                   /* number of tags in our TagTable */
   int      TagTable[DYNINSTagsLimit];/* known tags for this group */
   DYNINSTresourceCreationInfo tagCreateInfo; /* record of tag creations */

   struct DynInstTag_st* Next;       /* next defined group info struct */
} DynInstTagSt;



typedef struct DynInstNewTagInfo_
{
	int tagId;                   /* id of new tag */
	unsigned groupId;            /* group tag belongs to */
   int TGUniqueId;              /* our unique identifier for the group */
	int isNewGroup;              /* is this the first time we saw this group? */
} DynInstNewTagInfo;


typedef struct {
   int            TagHierarchy; /* True if hierarchy, false single level */
   int            NumGroups;     /* Number of groups, tag arrays */
   /* Group table, each index pointing to an array of tags */
   DynInstTagSt*  GroupTable[DYNINSTagGroupsLimit]; 
   //for unique representation of communicators - they may be deallocated
   //and the identifier reused in the implmenentation
   int GroupCounters[DYNINSTagGroupsLimit];
   int NumNewTags;   /* number of entries in the NewTags array */
   DynInstNewTagInfo  NewTags[DYNINSTNewTagsLimit];
} DynInstTagGroupSt;


static DynInstTagGroupSt  TagGroupInfo;
static DynInstWinArraySt  WinInfo;

/************************************************************************
 * PARADYNtagGroupInfo_Init
 *
 * Initialize the singleton TagGroupInfo struct.
 *
 ************************************************************************/
void
PARADYNtagGroupInfo_Init( void )
{
   unsigned int dx;

   TagGroupInfo.TagHierarchy = 0; /* FALSE; */
   TagGroupInfo.NumGroups = 0;
   TagGroupInfo.NumNewTags = 0;
   for(dx=0; dx < DYNINSTagGroupsLimit; dx++) {
      TagGroupInfo.GroupTable[dx] = NULL;
      TagGroupInfo.GroupCounters[dx] = 0;
   } 
}

/************************************************************************
 * PARADYNWinInfo_Init
 *
 * Initialize the singleton WinInfo struct for RMA Windows
 ************************************************************************/
void
PARADYNWinInfo_Init( void )
{
   unsigned int dx;
   static int warned = 0;

   WinInfo.NumWins = 0;
   for(dx=0; dx < DYNINSTWindowsLimit; dx++) {
      WinInfo.WindowTable[dx] = NULL;
      WinInfo.WindowCounters[dx] = 0;
   }
   /* determine which MPI implementation we are using - necessary because they
      use different data structures to represent things like communicators and
      windows.
   */
   if(whichMPI == -1){
      whichMPIenv = getenv("PARADYN_MPI");
      //fprintf(stderr,"got it %s\n",whichMPIenv);
      if(whichMPIenv){
         if (!strcmp(whichMPIenv,"LAM"))
            whichMPI = LAM;
         else if(!strcmp(whichMPIenv, "MPICH"))
            whichMPI = MPICH;
         else
            if(!warned){
               warned = 1;
               fprintf(stderr,"ENV variable PARADYN_MPI not set. Unable to \
	            determine MPI implementation.\n");
            }
      }
   }
}

/************************************************************************
 * void DYNINSTreportNewTags(void)
 *
 * Inform the paradyn daemons of new message tags and/or groups.
 *
 ************************************************************************/
void DYNINSTreportNewTags(void)
{
   int    dx;
   rawTime64 process_time;
   rawTime64 wall_time;

   if(TagGroupInfo.NumNewTags > 0) {
      /* not used by consumer [createProcess() in perfStream.C], so can prob.
       * be set to a dummy value to save a little time.  */
      process_time = DYNINSTgetCPUtime();
      /* this _is_ needed; paradynd keeps the 'creation' time of each resource
       *  (resource.h) */
      wall_time = DYNINSTgetWalltime();
   }
  
   for(dx=0; dx < TagGroupInfo.NumNewTags; dx++) {
      struct _newresource newRes;
    
      if((TagGroupInfo.TagHierarchy) && (TagGroupInfo.NewTags[dx].isNewGroup)) {
         memset(&newRes, '\0', sizeof(newRes));
         sprintf(newRes.name, "SyncObject/Message/%d-%d",
                 TagGroupInfo.NewTags[dx].groupId, TagGroupInfo.NewTags[dx].TGUniqueId);
         strcpy(newRes.abstraction, "BASE");
         newRes.mdlType = RES_TYPE_STRING;
         newRes.btype = MessageGroupResourceType;

         PARADYNgenerateTraceRecord(TR_NEW_RESOURCE,
                                    sizeof(struct _newresource), &newRes,
                                    wall_time, process_time);
      }
    
      memset(&newRes, '\0', sizeof(newRes));
      if(TagGroupInfo.TagHierarchy) {
         sprintf(newRes.name, "SyncObject/Message/%d-%d/%d",
                 TagGroupInfo.NewTags[dx].groupId,TagGroupInfo.NewTags[dx].TGUniqueId, TagGroupInfo.NewTags[dx].tagId);
         newRes.btype = MessageTagResourceType;
      } else {//if there is no Group/Tag hierarchy - when is this true?
         //I believe this was for PVM support, I don't see it used anywhere
         //else
         sprintf(newRes.name, "SyncObject/Message/%d", 
                 TagGroupInfo.NewTags[dx].tagId);
         newRes.btype = MessageGroupResourceType;
      }
      strcpy(newRes.abstraction, "BASE");
      newRes.mdlType = RES_TYPE_INT;

      PARADYNgenerateTraceRecord(TR_NEW_RESOURCE, 
                                 sizeof(struct _newresource), &newRes,
                                 wall_time, process_time);
   }
   TagGroupInfo.NumNewTags = 0;
}


/************************************************************************
 * PARADYNrecordTagGroupInfo
 *
 * Handle a (possibly) new group and/or message tag.  This routine
 * checks whether we've seen the given group and tag before, and
 * if not, reports them to the Paradyn daemon.
 *
 * We use a sort of hash table to keep tags and groups, so this routine
 * involves looking up the table entry to see whether we've seen
 * the tag or group before.
 *
 * There are two issues to consider -
 *
 * 1. We limit the total number of tags and groups so that an application
 *    that uses many of them will not overwhelm Paradyn by reporting
 *    new tags or groups.
 *
 * 2. We throttle the reporting of tags and groups whenever their
 *    creation rate is too high, again to avoid overwhelming the
 *    rest of Paradyn with the creation of new resources.
 *
 ************************************************************************/
static unsigned int rateCheck = 0;

void PARADYNrecordTagGroupInfo(int tagId, unsigned groupId)
{
   DynInstTagSt* tagSt;
   int           dx;
   int           newGroup;
   int		        tagDx = (tagId % DYNINSTagsLimit);
   unsigned		groupDx = (groupId % DYNINSTagGroupsLimit);
   double		recentTagCreateRate;
   rawTime64		ts;
  

   /* 
    * check if we've reached the limit on the number of new tags 
    * to be reported at one time
    * (In the current implementation, this should never happen.  All
    * platforms support shared memory sampling, so we report new tags 
    * and groups at the end of this function whenever we are called.
    */
   if(TagGroupInfo.NumNewTags == DYNINSTNewTagsLimit) return;


   /*
    * Find the info about the group we were given
    */
   tagSt = TagGroupInfo.GroupTable[groupDx];
   while((tagSt != NULL) && (tagSt->TagGroupId != groupId)) {
      tagSt = tagSt->Next;
   }
   if(tagSt == NULL) {
      /* We have not seen this group before, so add a group info struct for it. */
      tagSt = (DynInstTagSt *)malloc(sizeof(DynInstTagSt));
      assert(tagSt != NULL);

      tagSt->TagGroupId = groupId;
      tagSt->TGUniqueId = PARADYNGroup_CreateUniqueId(groupId, groupDx);
      tagSt->NumTags = 0;
      for(dx=0; dx < DYNINSTagsLimit; dx++) {
         tagSt->TagTable[dx] = DYNINSTGroupUndefinedTag;
      }
      DYNINSTresourceCreationInfo_Init( &(tagSt->tagCreateInfo),
                                        DYNINSTtagCreateRateWindowLength );
      tagSt->Next = TagGroupInfo.GroupTable[groupDx];
      TagGroupInfo.GroupTable[groupDx] = tagSt;
      TagGroupInfo.NumGroups++;
      newGroup = 1;
   } else {
      /* We have seen this group before - nothing to do */
      assert(tagSt->TagGroupId == groupId);
      newGroup = 0;
   }


   /*
    * Check if we've reached the limit on the number of tags to track.
    */
   if(tagSt->NumTags == DYNINSTagsLimit) return;


   /*
    * Find the info about the tag we were given
    */
   dx = tagDx;
   while((tagSt->TagTable[dx] != tagId) && (tagSt->TagTable[dx] != DYNINSTGroupUndefinedTag)) {
      dx++;
      if(dx == DYNINSTagsLimit)
      {
         dx = 0;
      }
      assert(dx != tagId);
   }

   if(tagSt->TagTable[dx] == tagId)
   {
      /* We've already seen this tag - nothing to do */
      return;
   }


   /*
    * check if the program is using new tags too quickly...
    */

   /* ...record that a tag was discovered for this group at this time
    * (we chose to use wall time to compute creation rate because the
    * daemon and front end are separate processes from this one, and the
    * issue here is how quickly they are able to handle new resource
    * notifications)...
    */
   ts = DYNINSTgetWalltime();
   if( DYNINSTresourceCreationInfo_Add( &(tagSt->tagCreateInfo), ts ) == 0 )
   {
		/*
		 * we ran out of space in the creation info -
		 * most likely, the application is creating resources too
		 * quickly, so we will throttle
		 *
	 	 * don't report this tag to the daemon now (in fact,
	 	 * don't record that we ever saw this tag so that we will
		 * handle it correctly in case we see the tag in the future
		 * when the program is not coming up with new tags so quickly
		 */
		return;
   }

   /* ...determine the creation rate during this group's sliding window... */
   recentTagCreateRate = (((double)tagSt->tagCreateInfo.nCreatesInWindow) /
                          tagSt->tagCreateInfo.windowLength);

   /* ...check whether the creation rate is too high */
   if( recentTagCreateRate > DYNINSTtagCreationRateLimit )
   {
      /* 
       * the program is using new tags too quickly -
       * don't report this tag to the daemon now (in fact,
       * don't record that we ever saw this tag so that we will
       * handle it correctly in case we see the tag in the future
       * when the program is not coming up with new tags so quickly
       */
      return;
   }


   assert(tagSt->TagTable[dx] == DYNINSTGroupUndefinedTag);

   /* allocate and initialize a new tag info struct */
   tagSt->TagTable[dx] = tagId;
   tagSt->NumTags++;
  
   TagGroupInfo.NewTags[TagGroupInfo.NumNewTags].tagId = tagId;
   TagGroupInfo.NewTags[TagGroupInfo.NumNewTags].groupId = tagSt->TagGroupId;
   TagGroupInfo.NewTags[TagGroupInfo.NumNewTags].TGUniqueId = tagSt->TGUniqueId;
   TagGroupInfo.NewTags[TagGroupInfo.NumNewTags].isNewGroup = newGroup;
   TagGroupInfo.NumNewTags++;

   /* report new tag and/or group to our daemon */
   DYNINSTreportNewTags();
}

//The following structures are for lam support. The c_contextid field is the
//context id of the communicator or window we are interested in. Another option
//for getting the context id would be to use the LAM specific MPIL_Comm_id
//function. However, that would require statically linking the appropriate
//MPI library into libparadynRT.so.1...

struct LAM_Comm{
   int one;
   int c_contextid;
};
struct LAM_Win{
   struct LAM_Comm * comm;
};


//Record new window information and report it as a new resource
void DYNINSTrecordWindowInfo(unsigned int * WinId)
{
   int           newWindow;
   int WindowId;
   static int warned = 0;
   int WindowDx;
   rawTime64             ts;
   struct ParadynWin_st* WinSt;

   //get context id of window
   if(whichMPI == LAM){
      struct LAM_Win * myWin = *(struct LAM_Win **)WinId;
      WindowId =  myWin->comm->c_contextid;
   }
   else if(whichMPI == MPICH){
      WindowId =(short) *WinId;
   }
   //else
   //fprintf(stderr, "Error: Unable to determine MPI implementation");

   WindowDx = (WindowId % DYNINSTWindowsLimit);

   // check if we've reached the limit on the number of new windows

   if(WinInfo.NumWins == DYNINSTWindowsLimit){
      if(!warned){
         fprintf(stderr,"The limit of the number of RMA windows supported by Paradyn has been reached - The max number of windows is set at %d.\n",DYNINSTWindowsLimit);
         warned = 1;
      }
      return;
   }


   /*
    * Find the info about the window we were given
    */  
   //fprintf(stderr, "in Record: windowDx is %d for WindowId %u\n",
   //WindowDx,WindowId);

   /* search for already existing window, because although right now this function   is only called by MPI_Win_create, in the future, we may call it from any
      functions that use windows, e.g. MPI_Win_fence, MPI_Win_start... */

   WinSt = WinInfo.WindowTable[WindowDx];
   while((WinSt != NULL) && (WinSt->WinId != WindowId)) {
      WinSt = WinSt->Next;
   }
   if(WinSt == NULL) {
      /* We have not seen this Window before, so add a WinInfo struct for it. */
      WinSt = (ParadynWinSt *) malloc(sizeof(ParadynWinSt));
      assert(WinSt != NULL);

      WinSt->WinId = WindowId;
      WinSt->WinUniqueId = PARADYNWindow_CreateUniqueId(WindowId, WindowDx);
      //fprintf(stderr,"WinUniqueId %d for WindowId %d\n",
      // WinSt->WinUniqueId,WindowId);

      //have to wait until we see MPI_Win_set_name on this window to get WinName
      WinSt->WinName = NULL;
      WinSt->Next = WinInfo.WindowTable[WindowDx];
      WinInfo.WindowTable[WindowDx] = WinSt;
      WinInfo.NumWins++;
      newWindow = 1;
   } else {
      /* We have seen this window before - nothing to do */
      assert(WinSt->WinId == WindowId);
      newWindow = 0;
   }

   /* report new window to our daemon */
   if(newWindow)
      PARADYNreportNewWindow(WinSt);
}

/************************************************************************
 * void PARADYNreportNewWindow(void)
 *
 * Inform the paradyn daemons of new  RMA Windows
 *
 ************************************************************************/
void PARADYNreportNewWindow(const struct ParadynWin_st * WinSt )
{
   int    dx;
   rawTime64 process_time;
   rawTime64 wall_time;
   struct _newresource newRes;

   process_time = DYNINSTgetCPUtime_hw();
   wall_time = DYNINSTgetWalltime();

   memset(&newRes, '\0', sizeof(newRes));
   sprintf(newRes.name, "SyncObject/Window/%d-%d",
           WinSt->WinId,WinSt->WinUniqueId);
   strcpy(newRes.abstraction, "BASE");
   newRes.mdlType = RES_TYPE_STRING;
   newRes.btype = WindowResourceType;

   PARADYNgenerateTraceRecord(TR_NEW_RESOURCE,
                              sizeof(struct _newresource), &newRes,
                              wall_time, process_time);
}
// report user-defined name for RMA window to the front-end
void DYNINSTnameWindow(unsigned int WindowId, char * name){
   struct ParadynWin_st* WinSt;
   int           WindowDx ;
   rawTime64 process_time;
   rawTime64 wall_time;  static int warned = 0;
   int WinId;
   struct _updtresource Res;

   // get implementation dependent context id for window
   if(whichMPI == LAM){
      struct LAM_Win * lw = (struct LAM_Win *)WindowId;
      WinId = lw->comm->c_contextid;
   } 
   else if(whichMPI == MPICH){
      WinId =(short) WindowId;
   } 
   else{
      //fprintf(stderr, "Unable to determine MPI implementation.\n");
      return;
   }
  
   WindowDx = (WinId % DYNINSTWindowsLimit);

   process_time = DYNINSTgetCPUtime();
   wall_time = DYNINSTgetWalltime();
 
   WinSt = WinInfo.WindowTable[WindowDx];
   //fprintf(stderr, "in Name: windowDx is %d for WindowId %u name is %s\n",
   //WindowDx,WindowId, name);
   while((WinSt != NULL) && (WinSt->WinId != WinId)) {
      WinSt = WinSt->Next;
   }
   if(WinSt == NULL) {
      if(!warned && WinInfo.NumWins >= DYNINSTWindowsLimit){
         fprintf(stderr, "The number of RMA Windows supported by Paradyn has been reached. %u\n",WinId);
         warned = 1;
         return;
      }
      else if(WinInfo.NumWins < DYNINSTWindowsLimit){
         //hmmm this shouldn't happen
         fprintf(stderr, "Attempting to name an RMA Window not already picked up by MPI_Win_create: %u\n",WinId);
         return;
      }
      return;
   }   
   if(!name){
      //I would hope this wouldn't happen either
      fprintf(stderr, "Attempting to name an RMA Window a null value\n");
      return;                     
   }
   WinSt->WinName = (char*)malloc(strlen(name) +1);
   strcpy(WinSt->WinName,name);

   memset(&Res, '\0', sizeof(Res));
   sprintf(Res.name, "SyncObject/Window/%d-%d",
           WinSt->WinId, WinSt->WinUniqueId);
   sprintf(Res.displayname, "SyncObject/Window/%s",
           WinSt->WinName);
   strcpy(Res.abstraction, "BASE");
   Res.mdlType = RES_TYPE_STRING;
   Res.btype = WindowResourceType;  
   Res.retired = 0;

   PARADYNgenerateTraceRecord(TR_UPDATE_RESOURCE,
                              sizeof(struct _updtresource), &Res,
                              wall_time, process_time);
}

// report user-defined name for communicator to the frontend
void DYNINSTnameGroup(unsigned int gId, char * name){
   struct DynInstTag_st* tagSt;
   int           groupDx ;
   rawTime64 process_time;
   rawTime64 wall_time;
   static int warned = 0;
   int groupId;
   struct _updtresource Res;

   //get implementation dependent context id for communicator
   if(whichMPI == LAM){
      struct LAM_Comm * lc = (struct LAM_Comm *)gId;
      groupId = lc->c_contextid;
   }
   else if(whichMPI == MPICH){
      groupId =(short) gId;
   }
   else{
      //fprintf(stderr, "Unable to determine MPI implementation.\n");
      return;
   }

   wall_time = DYNINSTgetWalltime ();
   process_time = DYNINSTgetCPUtime();

   //find the communicator
   groupDx = (groupId % DYNINSTagGroupsLimit);
   tagSt = TagGroupInfo.GroupTable[groupDx];
   while((tagSt != NULL) && (tagSt->TagGroupId != groupId)) {
      tagSt = tagSt->Next;
   }

   if(tagSt == NULL) {
      if(!warned && TagGroupInfo.NumGroups >= DYNINSTagGroupsLimit){
         fprintf(stderr, "The number of Commicators supported by Paradyn has been reached. %u\n",groupId);
         warned = 1;
         return;
      }
      else if(TagGroupInfo.NumGroups < DYNINSTagGroupsLimit){
         //we get here if they havent' sent any messages on the communicator yet
         //and the communicator wasn't created with MPI_Comm_create
         TagGroupInfo.TagHierarchy = 1;  /* TRUE; */
         PARADYNrecordTagGroupInfo(-1, groupId);
         tagSt = TagGroupInfo.GroupTable[groupDx];
         while((tagSt != NULL) && (tagSt->TagGroupId != groupId)) {
            tagSt = tagSt->Next; 
         }
         assert(tagSt); //we just put it there. it should be there.
      }
   }                              
   if(!name){                     
      //I would hope this wouldn't happen either
      fprintf(stderr, "Attempting to name a Communicator a null value\n");
      return;
   }
   if(!strcmp(name, "")){
      //It seems that LAM names a communicator "" when it deallocates it
      //so this happens on MPI_Comm_free
      return;
   }
   //fprintf(stderr,"name for group %d-%d \n", tagSt->TagGroupId, tagSt->TGUniqueId);
   memset(&Res, '\0', sizeof(Res));
   sprintf(Res.name, "SyncObject/Message/%d-%d",
           tagSt->TagGroupId,tagSt->TGUniqueId );
   sprintf(Res.displayname, "SyncObject/Message/%s", name);
   strcpy(Res.abstraction, "BASE");
   Res.mdlType = RES_TYPE_STRING;
   Res.btype = MessageGroupResourceType;
   Res.retired = 0;
    
   PARADYNgenerateTraceRecord(TR_UPDATE_RESOURCE,
                              sizeof(struct _updtresource), &Res,
                              wall_time, process_time);
} 

// when a communicator is deallocated with MPI_Comm_free report it as retired
// to the front end
void DYNINSTretireGroupTag(unsigned int * gId){
   struct DynInstTag_st* tagSt;
   struct DynInstTag_st* prevtagSt = NULL;
   int           groupDx ;
   rawTime64 process_time;
   rawTime64 wall_time;
   int groupId;
   int i = 0;
   struct _updtresource Res;
   int count = 0;

   // get implementation dependent context id for communicator
   if(whichMPI == LAM){ 
      struct LAM_Comm * lc = (struct LAM_Comm *)*gId;
      groupId = lc->c_contextid;
   }
   else if(whichMPI == MPICH){
      groupId =(short)* gId;
   }
   else{
      //fprintf(stderr, "Unable to determine MPI implementation.\n");
      return;
   }
  
   groupDx = (groupId % DYNINSTagGroupsLimit);
   tagSt = TagGroupInfo.GroupTable[groupDx];
   while((tagSt != NULL) && (tagSt->TagGroupId != groupId)) {
      prevtagSt = tagSt;
      tagSt = tagSt->Next;
   }
  
   process_time = DYNINSTgetCPUtime();
   wall_time = DYNINSTgetWalltime();
  
   if(tagSt == NULL) {     //fprintf(stderr, "Attempting to retire a communicator we haven't seen yet %u\n",groupId);
      /* this could happen if a communicator is freed that we didn't see any
         messages for */
      return;
   }//fprintf(stderr,"found group %u-%d and sending retire request\n",tagSt->TagGroupId, tagSt->TGUniqueId);
  
   /* retire tags first */
  
   for(i = 0 ; i < DYNINSTagsLimit && count < tagSt->NumTags; ++i){
      //fprintf(stderr,"tagSt->NumTags is %d for group %d-%d tag is %d\n",tagSt->NumTags, tagSt->TagGroupId, tagSt->TGUniqueId, tagSt->TagTable[i]);
      if(tagSt->TagTable[i] != DYNINSTGroupUndefinedTag){
         ++count;
         memset(&Res, '\0', sizeof(Res));
         sprintf(Res.name, "SyncObject/Message/%d-%d/%d",
                 tagSt->TagGroupId, tagSt->TGUniqueId, tagSt->TagTable[i]);
         sprintf(Res.displayname, "SyncObject/Message/%d-%d/%s",
                 tagSt->TagGroupId, tagSt->TGUniqueId, "");
         strcpy(Res.abstraction, "BASE");
         Res.mdlType = RES_TYPE_INT;
         Res.btype = MessageTagResourceType;
         Res.retired = 1;

         PARADYNgenerateTraceRecord(TR_UPDATE_RESOURCE,
                                    sizeof(struct _updtresource), &Res,
                                    wall_time, process_time);
      }
   }

   /* retire communicator */
   memset(&Res, '\0', sizeof(Res));
   sprintf(Res.name, "SyncObject/Message/%d-%d",
           tagSt->TagGroupId, tagSt->TGUniqueId);
   sprintf(Res.displayname, "SyncObject/Message/%s", "");
   strcpy(Res.abstraction, "BASE");
   Res.mdlType = RES_TYPE_STRING;
   Res.btype = MessageTagResourceType;
   Res.retired = 1;

   PARADYNgenerateTraceRecord(TR_UPDATE_RESOURCE,
                              sizeof(struct _updtresource), &Res,
                              wall_time, process_time);

   /* remove the group from the GroupTable */

   assert(tagSt);
   if(!prevtagSt)
      TagGroupInfo.GroupTable[groupDx] = tagSt->Next;
   else
      prevtagSt->Next = tagSt->Next;
   free(tagSt);
   tagSt = NULL;
   --TagGroupInfo.NumGroups;
}


// when a window is deallocated with MPI_Win_free, report it to the frontend
//as retired
void DYNINSTretireWindow(unsigned int * WindowId){
   struct ParadynWin_st* WinSt;
   struct ParadynWin_st* prevWinSt = NULL;
   int           WindowDx ;
   rawTime64 process_time;
   rawTime64 wall_time;
   int WinId;
   struct _updtresource Res; 

   //get implementation dependent context id for window
   if(whichMPI == LAM){
      struct LAM_Win * lw = (struct LAM_Win *)*WindowId;
      WinId = lw->comm->c_contextid;
   }    
   else if(whichMPI == MPICH){
      WinId =(short) *WindowId;
   }    
   else{
      //fprintf(stderr,"Unable to determine MPI implementation.\n");
      return;
   }    
                                 
   WindowDx = (WinId % DYNINSTWindowsLimit);
    
   process_time = DYNINSTgetCPUtime();
   wall_time = DYNINSTgetWalltime();

   WinSt = WinInfo.WindowTable[WindowDx];
   //fprintf(stderr, "in retire: windowDx is %d for WinId %u\n",WindowDx,WinId);
   while((WinSt != NULL) && (WinSt->WinId != WinId)) {
      prevWinSt = WinSt;
      WinSt = WinSt->Next;
   }
   if(WinSt == NULL) {
      fprintf(stderr, "Attempting to retire a window we haven't seen yet %u\n",WinId);
      return;
   }                              
   //fprintf(stderr,"found window %u-%d and sending retire request\n",WinSt->WinId, WinSt->WinUniqueId);
   memset(&Res, '\0', sizeof(Res));
   sprintf(Res.name, "SyncObject/Window/%u-%d",
           WinSt->WinId, WinSt->WinUniqueId);
   if (WinSt->WinName)
      sprintf(Res.displayname, "SyncObject/Window/%s", WinSt->WinName);
   else
      sprintf(Res.displayname, "SyncObject/Window/%s", "");
   strcpy(Res.abstraction, "BASE");
   Res.mdlType = RES_TYPE_STRING;
   Res.btype = WindowResourceType; 
   Res.retired = 1;

   PARADYNgenerateTraceRecord(TR_UPDATE_RESOURCE, 
                              sizeof(struct _updtresource), &Res,
                              wall_time, process_time);
  
   /* remove the window from the WindowTable */
   assert(WinSt);
   if(!prevWinSt)
      WinInfo.WindowTable[WindowDx] = WinSt->Next;
   else
      prevWinSt->Next = WinSt->Next;
   free(WinSt);
   WinSt = NULL;
   --WinInfo.NumWins; 
}

/************************************************************************
 * 
 * DYNINSTrecordTag
 * DYNINSTrecordTagAndGroup
 * DYNINSTrecordGroup
 *
 * Handle recording of (possibly) new tag and/or group.
 * Message-passing routines such as MPI_Send are instrumented
 * to call these functions.
 *
 ************************************************************************/

void DYNINSTrecordTag(int tagId)
{
   assert(tagId >= 0);
   assert(TagGroupInfo.TagHierarchy == 0); /* FALSE); */
   PARADYNrecordTagGroupInfo(tagId, -1);
}

void DYNINSTrecordTagAndGroup(int tagId, unsigned groupId)
{
   int gId = -1;
   assert(tagId >= 0);
   if(whichMPI == LAM){
      struct LAM_Comm * lc = (struct LAM_Comm*)groupId;
      gId = lc->c_contextid;
   }
   else if(whichMPI == MPICH){
      gId = (short)groupId;
   }
   else{
      gId = (int)groupId;
   }
   TagGroupInfo.TagHierarchy = 1; /* TRUE; */
   PARADYNrecordTagGroupInfo(tagId, gId );
}

void DYNINSTrecordGroup(unsigned groupId)
{
   int gId = -1;
   if(whichMPI == LAM){
      struct LAM_Comm * lc = (struct LAM_Comm*)groupId;
      gId = lc->c_contextid;
   }
   else if(whichMPI == MPICH){
      gId = (short)groupId;
   }
   else{
      gId = (int)groupId;
   }
   TagGroupInfo.TagHierarchy = 1;  /* TRUE; */
   PARADYNrecordTagGroupInfo(-1, gId);
}

//record new RMA window
void DYNINSTrecordWindow(unsigned int WindowId)
{
   DYNINSTrecordWindowInfo((unsigned int *)WindowId);
}



/************************************************************************
 *
 ************************************************************************/
int PARADYNGroup_CreateUniqueId(int commId, int dx)
{
   // maintain unique identifiers for communicators as their implementation
   //dependent context ids may be reused by the MPI implementation
   return TagGroupInfo.GroupCounters[dx]++;
}

/*
 * RMA windows
 * maintain a unique identifier for each window because the MPI implementation
 * may reuse a window's context id after it is deallocated
 */
unsigned int PARADYNWindow_CreateUniqueId(unsigned int WindowId, int dx){
   return WinInfo.WindowCounters[dx]++;
}

/************************************************************************
 *
 ************************************************************************/
int DYNINSTGroup_CreateLocalId(int tgUniqueId)
{
   return(tgUniqueId);
}


/************************************************************************
 *
 ************************************************************************/
int PARADYNGroup_FindUniqueId(unsigned gId, char * constraint)
{
   int           groupDx;
   int           groupId;
   DynInstTagSt* tagSt;
   
   //get implementation dependent context id for communicator
   if(whichMPI == LAM){
      struct LAM_Comm * lc = (struct LAM_Comm *)gId;
      groupId = lc->c_contextid;
   }
   else if(whichMPI == MPICH){
      groupId = (short)gId;
   }
   //else{
   //fprintf(stderr, "Unable to determine MPI implementation.\n");
   //}
   
   groupDx = groupId % DYNINSTagGroupsLimit;
   
   for(tagSt = TagGroupInfo.GroupTable[groupDx]; tagSt != NULL;
       tagSt = tagSt->Next)
   {
      if(tagSt->TagGroupId == groupId){
         
         // allows two numbers 1024 digits long + a dash and a '\0'
         char temp[2051];
         sprintf(temp,"%d-%d",tagSt->TagGroupId, tagSt->TGUniqueId);
         assert(constraint);
         
         //fprintf(stderr,"constraint is %s temp is %s\n", constraint, temp);
         if (!strcmp(temp, constraint)){
            //match!
            return 1;
         }
         return 0;
      }
   }  
   return(0);
}

//find the indentifier of the RMA window in question
int DYNINSTWindow_FindUniqueId(unsigned int WindowId, char * constraint)
{
   int           WinDx;
   ParadynWinSt* WinSt = NULL;
   int WinId;

   //get the implementation dependent context id of the window
   if(whichMPI == LAM){
      struct LAM_Win * lw = (struct LAM_Win *)WindowId;
      WinId = lw->comm->c_contextid;
   }
   else if(whichMPI == MPICH){
      WinId = (short)WindowId;
   }
   //else{
   //fprintf(stderr, "Unable to determine MPI implementation.\n");
   //}

   WinDx = (WinId % DYNINSTWindowsLimit);

   WinSt = WinInfo.WindowTable[WinDx];
   while((WinSt != NULL) && (WinSt->WinId != WinId)) {
      WinSt = WinSt->Next;
   }
   if(WinSt == NULL) {
      //hmmm this shouldn't happen unless there is a bug somewhere
      // or if paradyn supports attach for MPI programs
      fprintf(stderr, "trying to FIND a window we haven't seen before %u\n",WinId);
      return (-1);
   }

   if(WinSt->WinId == WinId){

      // allows two numbers 1024 digits long + a dash and a '\0'
      char temp[2051];
      sprintf(temp,"%d-%d",WinSt->WinId, WinSt->WinUniqueId);
      //fprintf(stderr,"constraint is %s temp is %s\n", constraint, temp);
      assert(constraint);
      if (!strcmp(temp, constraint)){
         //match
         return 1;
      }
      return 0;
   }

   return(-1);
}

#define KNOWN_MPI_TYPES 34

/*
  This is an approximation to the real MPI_Pack_size routine,
  which may not be present in an MPI application
  Constants are given for MPICH-1.2.0 on x86/Linux
*/
int MPI_Pack_size (int incount, int datatype, int comm, int *size)
{
	const int type_size[KNOWN_MPI_TYPES+1] = {
		0, 1, 1, 1, 2, 2, 4, 4, 4, 4, 
		4, 8,12, 8, 1, 0, 0, 8,12, 8, 
		6, 8,16, 8,16, 4, 4, 8, 4, 8,
		16,32, 8,16};
	static int warned = 0;

	if (!warned) {
		fprintf(stderr, "WARNING: Bogus MPI_Pack_size: relink "
              "the application with \"-u MPI_Pack_size\" "
              "for accurate results\n");
		warned = 1;
	}
	if (datatype <= 0 || datatype > KNOWN_MPI_TYPES) {
		fprintf(stderr, "WARNING: Paradyn_Pack_size: unknown type\n");
		*size = 0;
		return (-1);
	}
	*size = incount * type_size[datatype] + 16;
	
	return 0;
}



