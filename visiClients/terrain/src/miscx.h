#ifndef MISCX_H
#define MISCX_H



void PopUpInit();
void IFeep();
int IGetGeometry(Window win, WindowInfo *Info);
char *ips_malloc(int count);
void XtGetValue(Widget widget, char *field, caddr_t result);
void XtSetValue(Widget widget, char *field, caddr_t result);


#endif
