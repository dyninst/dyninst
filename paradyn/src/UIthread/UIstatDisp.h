#ifndef _statDisp_h
#define _statDisp_h


class statusDisplayObj {
  char *wname;
public:
  void updateStatusDisplay (int displayCode, 
		       const char *fmt ...);
  statusDisplayObj (int type);
};  


#define PC_STATUSDISPLAY 1
#define NORMAL_STATUSDISPLAY 2
#define URGENT_STATUSDISPLAY 3

#endif
