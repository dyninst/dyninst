/*
 * 
 * $Log: PCglobals.h,v $
 * Revision 1.8  1995/10/05 04:41:40  karavan
 * changes to UI::PC interface calls.
 *
 * Revision 1.7  1995/06/02  20:50:07  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.6  1995/02/16  08:19:09  markc
 * Changed Boolean to bool
 *
 * Revision 1.5  1994/09/05  20:00:57  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.4  1994/06/12  22:40:48  karavan
 * changed printf's to calls to status display service.
 *
 * Revision 1.3  1994/05/18  02:49:27  hollings
 * Changed the time since last change to use the time of the first sample
 * arrivial after the change (rather than the time of the change).
 *
 * Revision 1.2  1994/04/21  04:55:08  karavan
 * added node and edge style numbers for shg display - klk
 *
 * Revision 1.1  1994/02/02  00:38:14  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.5  1993/08/05  19:01:43  hollings
 * added interactiveShgUpdate proto.
 *
 * Revision 1.4  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.3  1992/10/23  20:13:37  hollings
 * Working version right after prelim.
 *
 * Revision 1.2  1992/08/24  15:13:40  hollings
 * list of global variables.
 *
 * Revision 1.1  1992/08/03  20:45:54  hollings
 * Initial revision
 *
 *
 */

// should we print a message when a metric value is printed.
extern bool fetchPrint;

// number of times a metric was sampled.
extern int sampleCount;

extern int traceRead;
extern double bucket_size;

extern bool genHist;
// extern applicationContext *context;


// update the SHG during the search?
extern bool interactiveShgUpdate;

extern int SHGid;
extern dataManagerUser *dataMgr;

// styles for SHG display

#define UNTESTEDNODESTYLE 1
#define INACTIVENODESTYLE 2
#define ACTIVETRUENODESTYLE 3
#define ACTIVEFALSENODESTYLE 4
#define WHEREEDGESTYLE 1
#define WHYEDGESTYLE 2
#define WHENEDGESTYLE 3
