// barChartTcl.h

int resizeCallbackCommand(ClientData cd, Tcl_Interp *interp, int argc, char **argv);
int exposeCallbackCommand(ClientData cd, Tcl_Interp *interp, int argc, char **argv);
int resourcesAxisHasChangedCommand(ClientData cd, Tcl_Interp *interp, int argc, char **argv);
int metricsAxisHasChangedCommand(ClientData cd, Tcl_Interp *interp, int argc, char **argv);
int newScrollPositionCommand(ClientData cd, Tcl_Interp *interp, int argc, char **argv);
int dataFormatHasChangedCommand(ClientData cd, Tcl_Interp *interp, int argc, char **argv);

int launchBarChartCommand(ClientData cd, Tcl_Interp *interp, int argc, char **argv);

void deleteLaunchBarChartCommand(ClientData cd);
void deleteDummyProc(ClientData cd);

int Dg2NewDataCallback(int lastBucket);

