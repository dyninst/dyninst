// barChartTcl.h

#ifndef _BARCHART_TCL_H_
#define _BARCHART_TCL_H_

int resizeCallbackCommand(ClientData, Tcl_Interp *, int argc, char **argv);
int exposeCallbackCommand(ClientData, Tcl_Interp *, int argc, char **argv);
int resourcesAxisHasChangedCommand(ClientData, Tcl_Interp *, int argc, char **argv);
int metricsAxisHasChangedCommand  (ClientData, Tcl_Interp *, int argc, char **argv);
int newScrollPositionCommand   (ClientData, Tcl_Interp *, int argc, char **argv);
int dataFormatHasChangedCommand(ClientData, Tcl_Interp *, int argc, char **argv);
int rethinkIndirectResourcesCommand(ClientData, Tcl_Interp *, int argc, char **argv);
int getMetricColorNameCommand(ClientData, Tcl_Interp *, int, char **);
int long2shortFocusNameCommand(ClientData, Tcl_Interp *interp, int argc, char **argv);

int launchBarChartCommand(ClientData, Tcl_Interp *, int argc, char **argv);

void deleteLaunchBarChartCommand(ClientData);
void deleteDummyProc(ClientData);

int Dg2NewDataCallback(const int lastBucket);
int Dg2PhaseNameCallback(int);
int Dg2InvalidMetricsOrResources(int);
int Dg2Fold(int);
int Dg2AddMetricsCallback(int);

#endif
