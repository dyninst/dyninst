#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <stdio.h>
#include <time.h>

#ifdef os_windows
#define MAX_USER_NAME	256
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

//#include "test_lib.h"

#include "DatabaseOutputDriver.h"

extern "C" {
TestOutputDriver *outputDriver_factory(void * data) { return new DatabaseOutputDriver(data); }
}

DatabaseOutputDriver::DatabaseOutputDriver(void * data)
  : attributes(NULL), submittedResults(false), result(UNKNOWN)
{
	sqlLogFilename = *((std::string *)data);

	FILE * fp = fopen(sqlLogFilename.c_str(), "r");
	if (fp != NULL) {
		// log file already exists, so don't rewrite header
		wroteLogHeader = true;
		fclose(fp);
	}
}

DatabaseOutputDriver::~DatabaseOutputDriver() {
  if (attributes != NULL) {
    delete attributes;
    attributes = NULL;
  }
}

void
DatabaseOutputDriver::startNewTest(std::map<std::string,
				            std::string> &attrs)
{
  // This needs to set up a log file to store output from all streams
  if (attributes != NULL) {
    delete attributes;
  }
  attributes = new std::map<std::string, std::string>(attrs);

  // I want to log to the file "dblog.<testname>"
  std::stringstream fnstream;
  fnstream << "dblog." << (*attributes)[std::string("test")];
  dblogFilename = fnstream.str();

  // Write anything in pretestLog to the dblog file
  std::ofstream out(dblogFilename.c_str(), std::ios::app);
  out << pretestLog.str();
  out.close();
  pretestLog.str(std::string()); // Clear the buffer

  // We haven't submitted results for this test yet
  submittedResults = false;
  // Don't know what the test's result is
  result = UNKNOWN;
}

void DatabaseOutputDriver::redirectStream(TestOutputStream stream, const char * filename) {
  // This is a no-op for database output
}

void DatabaseOutputDriver::logResult(test_results_t res) {
  // What does this do, exactly?  Store the result code for database check-in
  // I guess..
  // Do I want to submit the results to the database in this method, or use
  // finalizeOutput for that?
  result = res;
}

void DatabaseOutputDriver::logCrash(std::string crashedTest) {
  // New idea: we've already called startNewTest for this test, so
  // dblogFilename is set to recover the old log data for the test.
  // All we need to do is register a crashed result for the test.
  result = CRASHED;
}

void DatabaseOutputDriver::log(TestOutputStream stream, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(stream, fmt, args);
  va_end(args);
}

void DatabaseOutputDriver::vlog(TestOutputStream stream, const char *fmt,
				va_list args)
{
  FILE *dbout = NULL;

  if (dblogFilename.empty()) {
    dbout = tmpfile();
    if (NULL == dbout) {
      fprintf(stderr, "[%s:%u] - Error opening temp log file\n", __FILE__, __LINE__);
    } else { // FIXME Check return values
      int count = vfprintf(dbout, fmt, args);
      fflush(dbout);
      fseek(dbout, 0, SEEK_SET);
      char *buffer = new char[count];
      fread(buffer, sizeof (char), count, dbout);
      pretestLog.write(buffer, count);
      delete [] buffer;
      fclose(dbout);
    }
  } else {
    dbout = fopen(dblogFilename.c_str(), "a");
    if (NULL == dbout) {
      fprintf(stderr, "[%s:%u] - Error opening log file\n", __FILE__, __LINE__);
      return;
    }
    vfprintf(dbout, fmt, args);
    fclose(dbout);
  }
}

void DatabaseOutputDriver::finalizeOutput() {
	// This is where we'll send all the info we've received so far to the
	// database
	if (submittedResults) {
		return; // Only submit results for a test once
	}



/*	// Read result text from database log
	std::stringstream resultStream;
	std::ifstream logFile;
	// enforce binary read mode for windows' carriage returns
	logFile.open(dblogFilename.c_str(), std::ifstream::in & std::ifstream::binary);
	long count;

	logFile.seekg(0, std::ifstream::end);
	count = logFile.tellg();
	logFile.seekg(0);
	char *buffer = new char[count];
	if (NULL == buffer) {
		fprintf(stderr, "[%s:%u] - Out of memory!\n", __FILE__, __LINE__);
		// TODO Handle error;
	}
	logFile.read(buffer, count);
	resultStream.write(buffer, count);
	delete [] buffer;
	buffer = NULL;
	logFile.close();

	// If result == UNKNOWN, there may be a line in the database log of the form
	// RESULT: <code>
	// with the results of this test.
	// This would be the case when the mutatee writes to the log file and
	// test_driver has no knowledge of the result
	if (UNKNOWN == result) {
		// Find RESULT: text
		std::string::size_type index =
			resultStream.str().rfind("RESULT: ", resultStream.str().size());
		if (index != std::string::npos) {
			// Found it.  Now I need to extract the result code
			const char *resultString = resultStream.str().substr(index).c_str();
			test_results_t res;
			int count;
			count = sscanf(resultString, "RESULT: %d", &res);
			if (count != 1) {
				// TODO Handle error reading result
			} else {
				result = res;
			}
		}
	}*/

	//write the header if necessary
	if (!wroteLogHeader) {
		// get hostname and username for log header
		// FIXME This needs to be platform-independent
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX (255)
#endif
		char hostname[HOST_NAME_MAX];
		if (gethostname(hostname, HOST_NAME_MAX)) {
			// TODO Handle error
		}
		
		std::string userName;

#ifdef os_windows
		char * szUserName = new char[MAX_USER_NAME + 1];
		LPDWORD lpnSize = (LPDWORD)malloc(sizeof(DWORD));
		*lpnSize = MAX_USER_NAME + 1;
		if (lpnSize == NULL) {
			fprintf(stderr, "[%s:%u] - Out of memory!\n", __FILE__, __LINE__);
			// TODO Handle error;
		}
		memset(szUserName, 0, MAX_USER_NAME + 1);
		if (!GetUserName(szUserName, lpnSize)) {
			fprintf(stderr, "[%s:%u] - Failed to get username: %s\n",
					__FILE__, __LINE__, GetLastError());
			//TODO Handle error
		}
		userName = std::string(szUserName);

		free(lpnSize);
		delete [] szUserName;
#else
		//note: geteuid() is always successful
		//FIXME: use getpwuid_r
		struct passwd * pw = getpwuid(geteuid());
		if (NULL == pw) {
			//TODO unknown user
			userName = "unknown";
		} else {
			userName = pw->pw_name;
		}
#endif

		std::string logHeader = userName + "@" + hostname + "\n\n";
		
		FILE * sqlLog = fopen(sqlLogFilename.c_str(), "wb");
		if (NULL == sqlLog) {
			fprintf(stderr, "[%s:%u] - Error opening log file: %s\n",
					__FILE__, __LINE__, sqlLogFilename.c_str());
			//TODO handle error
		}
		int size = strlen(logHeader.c_str());
		if (fwrite(logHeader.c_str(), sizeof(char), size, sqlLog) != size) {
			fprintf(stderr, "[%s:%u] - Error writing to log file.\n", __FILE__, __LINE__);
			//TODO handle error
		}
		fclose(sqlLog);

		wroteLogHeader = true;
	}
	
	writeSQLLog();
	return;

	//TODO: what about this stuff?
	submittedResults = true;
}

// This is called if there is an error submitting results to the database.
// It writes the test results to a log file that can be read or later
// resubmitted to the database
void DatabaseOutputDriver::writeSQLLog() {
	std::ofstream out;
	out.open(sqlLogFilename.c_str(), std::ios::app);

	// 1. Write a test label to the file
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	char * sqldate = new char[20];
	//TODO handle malloc failure
	sprintf(sqldate, "%4d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900,
			timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
			timeinfo->tm_min, timeinfo->tm_sec);

	out << "BEGIN TEST\n" << sqldate << "\n{";

	delete [] sqldate;

	std::map<std::string, std::string>::iterator iter;
	for (iter = attributes->begin(); iter != attributes->end(); iter++) {
		out << iter->first << ": " << iter->second;
		std::map<std::string, std::string>::iterator testiter = iter;
		if (++testiter != attributes->end()) {
			out << ", ";
		}
	}
	out << "}\n";

	// 2. Copy the contents of the dblog to the file
	// open in 'b' mode has no effect on POSIX, only for windows to avoid ignoring \r
	FILE * fh = fopen(dblogFilename.c_str(), "rb");
	if (NULL == fh) {
		fprintf(stderr, "[%s:%u] - Error opening file: %s\n", __FILE__, __LINE__, dblogFilename.c_str());
		// TODO Handle error
	}
	fseek(fh, 0, SEEK_END);
	long size = ftell(fh);
	fseek(fh, 0, SEEK_SET);
	char *buffer = new char[size + 1];
	fread(buffer, sizeof(char), size, fh);
	fclose(fh);
	buffer[size] = '\0';

	//remove trailing whitespace from buffer
	std::string buf (buffer);
	size_t found = buf.find_last_not_of(" \t\f\v\n\r");
	
	if (found != std::string::npos)
		buf.erase(found + 1);
	else
		buf.clear();			//all whitespace

	out.write(buf.c_str(), buf.size());
	delete [] buffer;

	// 3. Write a result code to the file if one doesn't already exist
	if (buf.rfind("RESULT:") == std::string::npos)
		out << "\nRESULT: " << result;
	out << "\n\n";
	out.close();

	// Clear the old dblog file
	unlink(dblogFilename.c_str());
	dblogFilename.clear();
}

void DatabaseOutputDriver::getMutateeArgs(std::vector<std::string> &args) {
	args.clear();
	args.push_back(std::string("-dboutput"));
}
