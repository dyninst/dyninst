/*
 * Copyright (c) 1989, 1990 Barton P. Miller, Morgan Clark, Timothy Torzewski
 *     Jeff Hollingsworth, and Bruce Irvin. All rights reserved.
 *
 * This software is furnished under the condition that it may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.  The name of the principals
 * may not be used in any advertising or publicity related to this
 * software without specific, written prior authorization.
 * Any use of this software must include the above copyright notice.
 *
 */

/*
 * command.h - header file of command.c.
 *
 * $Id: command.h,v 1.6 2001/06/12 19:56:12 schendel Exp $
 */

#ifndef COMMAND_H
#define COMMAND_H


#define SA_INIT     0
#define SA_JUMP     1
#define SA_ROTATE   2
#define SA_SMOOTH   3
#define SA_ROUGH    4
#define SA_HIDE     5
#define SA_UNHIDE   6
#define SA_RESIZE   7
#define SA_USEMED   8
#define SA_NOMED    9

extern void plot3drequest( int action );
extern void kill_surface(void);
extern int Graph3DAddNewCurve (char* m_name, char* r_name, char* p_name, char* axis_label,
                               int no_points, int no_curves);
extern void Graph3DSetCurveData();

void done(int status);
int getStartIndex(int ID);
int checkDecimal(double zmax);
void copyResName(char* destination, char* source);
void ReDisplayGraph(void);
void ProcessNewSegments(int printIndex);




#endif
