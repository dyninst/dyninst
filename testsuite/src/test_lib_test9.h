/* These functions are library functions for test9 and 12 subtests.
 * They are not designed to build on windows, so this portion
 * of libtestSuite are not linked in on windows
 */

int runMutatedBinary(char *path, char* fileName, char* testID);

void changePath(char *path);

int runMutatedBinaryLDLIBRARYPATH(char *path, char* fileName, char* testID);

void sleep_ms(int ms);
