
/*
 * Utility functions for piggybacking information onto data messages.
 *
 */

/* message passing library specific routines */
void DYNINSTpiggyPackInt(int *val);
void DYNINSTpiggyPackDouble(double *val);
void DYNINSTpiggyPackStr(char *val);
void DYNINSTpiggyUnpackInt(int *val);
void DYNINSTpiggyUnpackDouble(double *val);
void DYNINSTpiggyUnpackStr(char **val);

/* generic routines */
void DYNINSTProcessPiggyMessage();
