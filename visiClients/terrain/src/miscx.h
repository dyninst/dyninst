/* $Id: miscx.h,v 1.3 2001/06/12 19:56:12 schendel Exp $ */

#ifndef MISCX_H
#define MISCX_H



void PopUpInit(void);
void IFeep(void);
int IGetGeometry(Window win, WindowInfo *Info);
char *ips_malloc(int count);
void XtGetValue(Widget widget, char *field, caddr_t result);
void XtSetValue(Widget widget, char *field, caddr_t result);


#endif
