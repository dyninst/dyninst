/* This marks the end of user code in the text file. */

/* This is to prevent system libraries with symbols compiled into them
 *    from adding extranious material to our inst. environment.
 *
 * $Log: DYNINSTendCode.c,v $
 * Revision 1.1  1994/08/02 18:18:51  hollings
 * added code to save/restore FP state on entry/exit to signal handle
 * (really jcargill, but commited by hollings).
 *
 * changed comparisons on time regression to use 64 bit int compares rather
 * than floats to prevent fp rounding error from causing false alarms.
 *
 *
 */
void DYNINSTendUserCode()
{
}
