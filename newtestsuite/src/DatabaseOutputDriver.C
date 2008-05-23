#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <stdio.h>
#include <unistd.h>

#include <mysql.h>

#include "test_lib.h"

#include "DatabaseOutputDriver.h"

extern "C" {
TestOutputDriver *outputDriver_factory() {
  return new DatabaseOutputDriver();
}
}

DatabaseOutputDriver::DatabaseOutputDriver()
  : attributes(NULL), submittedResults(false), result(UNKNOWN)
{
  if (mysql_library_init(-1, NULL, NULL) != 0) {
    fprintf(stderr, "[%s:%u] - Error initializing MySQL library\n", __FILE__, __LINE__);
    // TODO Handle error
  }
}

DatabaseOutputDriver::~DatabaseOutputDriver() {
  if (attributes != NULL) {
    delete attributes;
    attributes = NULL;
  }
  mysql_library_end();
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
    // TODO If dblogFilename isn't set, log to pretestLog
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
  std::stringstream sstream; // Temporary for building strings

  // This is where we'll send all the info we've received so far to the
  // database
  if (submittedResults) {
    return; // Only submit results for a test once
  }

  // Set up the MySQL library and connect to the server
  MYSQL *db;
  db = mysql_init(NULL);
  if (NULL == db) {
    // TODO Handle out of memory error
  }
  // TODO 0. Connect to the database server
  MYSQL *connection;
  connection = mysql_real_connect(db, DB_HOSTNAME, DB_USERNAME, DB_PASSWD,
				  DB_DBNAME, DB_PORT, DB_UXSOCKET,
				  DB_CLIENTFLAG);
  if (NULL == connection) {
    // TODO Handle error connecting to database server
  } else {
  }

  int status;
  std::stringstream queryStream;
  std::string query;
  MYSQL_RES *queryResult;
  std::string testID;
  std::string machineID;
  std::string platformID;
  std::string siteID;

  // 1. Get the IDs for all of our attributes
  std::map<std::string, std::string> attrIDs;
  std::map<std::string, std::string>::iterator iter;
  for (iter = attributes->begin(); iter != attributes->end(); iter++) {
    // Get ID for the attribute with name iter->first
    query = std::string("select id from attributes where name = binary '")
            + iter->first + std::string("'");
    status = mysql_query(db, query.c_str());
    if (status != 0) {
      fprintf(stderr, "[%s:%u] - Error querying database: '%s'\n", __FILE__, __LINE__, mysql_error(db));
    }
    queryResult = mysql_store_result(db);
    if (NULL == queryResult) {
      fprintf(stderr, "[%s:%u] - Error querying database: '%s'\n", __FILE__, __LINE__, mysql_error(db));
    } else {
      if (mysql_num_rows(queryResult) == 0) {
	// If it's not there, insert it into the database; then get the ID
	query = std::string("insert into attributes set name = binary '") + iter->first
      	        + std::string("', isBoolean=0");
	status = mysql_query(db, query.c_str());
	if (status != 0) {
	  fprintf(stderr, "[%s:%u] - Error querying database: '%s'\n", __FILE__, __LINE__, mysql_error(db));
	  // TODO Handle error
	}
	sstream << mysql_insert_id(db);
	attrIDs[iter->first] = std::string(sstream.str());
	sstream.str(std::string()); // Clear buffer
      } else { // Attribute already in database
	if (mysql_num_rows(queryResult) != 1) {
	  fprintf(stderr, "[%s:%u] - WARNING: More than one ID in database for attribute '%s'\n", __FILE__, __LINE__, iter->first.c_str());
	}
	MYSQL_ROW row = mysql_fetch_row(queryResult);
	if (NULL == row[0]) {
	  // TODO Handle error
	} else {
	  attrIDs[iter->first] = std::string(row[0]);
	}
      }
      // By this point, attrIDs should contain the ID for our attribute

      // Clean up
      mysql_free_result(queryResult);
      queryResult = NULL;
    }
  }

  // 2. Find the testID for our test case, or build a new test case description
  std::map<std::string, std::string> descIDs;
  queryStream.str(std::string());
  queryStream << "select t0.testID from testDescription as t0";
  for (unsigned int i = 1; i < attributes->size(); i++) {
    queryStream << ", testDescription as t" << i;
  }
  if (attributes->size() >= 1) {
    iter = attributes->begin();
    queryStream << " where t0.testID=t1.testID"
		<< " and t1.attribID=" << attrIDs[iter->first]
		<< " and t1.value = binary '" << iter->second << "'";
    iter++;
  }
  for (unsigned int i = 2;
       i < attributes->size();
       i++, iter++) {
    queryStream << " and t0.testID=t" << i << ".testID"
		<< " and t" << i << ".attribID = " << attrIDs[iter->first]
		<< " and t" << i << ".value = binary '" << iter->second << "'";
  }
  if (mysql_query(db, queryStream.str().c_str()) != 0) {
    fprintf(stderr, "[%s:%u] - Error submiting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
    // TODO Handle error
  }
  queryResult = mysql_store_result(db);
  if (NULL == queryResult) {
    fprintf(stderr, "[%s:%u] - Error retrieving results: '%s'\n", __FILE__, __LINE__, mysql_error(db));
    // TODO Handle error
  }
  if (mysql_num_rows(queryResult) == 0) {
    // There's a few things I need to do here:
    // 1. Insert a new row into the test table
    std::string testName = (*attributes)["test"];
    queryStream.str(std::string());
    queryStream << "insert into test set name = binary '" << testName << "'";
    if (mysql_query(db, queryStream.str().c_str()) != 0) {
      fprintf(stderr, "[%s:%u] - Error submitting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
      // TODO Handle error
    }
    sstream << mysql_insert_id(db);
    testID = std::string(sstream.str());
    sstream.str(std::string());

    // 2. Insert a new row into testDescription for each (attribute, value)
    //    pair
    for (iter = attributes->begin(); iter != attributes->end(); iter++) {
      queryStream.str(std::string());
      queryStream << "insert into testDescription set testID=" << testID
		  << ", attribID=" << attrIDs[iter->first]
		  << ", value = binary '" << iter->second << "'";
      if (mysql_query(db, queryStream.str().c_str())) {
	fprintf(stderr, "[%s:%u] - Error submitting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
	// TODO Handle error
      }
    }
  } else {
    // We got a test matching the set of (attr, value) pairs
    if (mysql_num_rows(queryResult) > 1) {
      fprintf(stderr, "[%s:%u] - WARNING: Multiple test IDs for attr-value set\n", __FILE__, __LINE__);
    }
    // Get ID from row
    MYSQL_ROW row = mysql_fetch_row(queryResult);
    if (NULL == row[0]) {
      // TODO Handle error
    }
    testID = std::string(row[0]);
  }
  mysql_free_result(queryResult);
  queryResult = NULL;

  // 4. Get machine ID
  // FIXME This needs to be platform-independent
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX (255)
#endif
  char hostname[HOST_NAME_MAX];
  if (gethostname(hostname, HOST_NAME_MAX)) {
    // TODO Handle error
  }
  queryStream.str(std::string());
  queryStream << "select id from machine where name='" << hostname << "'";
  if (mysql_query(db, queryStream.str().c_str())) {
    // TODO Handle error
  }
  queryResult = mysql_store_result(db);
  if (NULL == queryResult) {
    // TODO Handle error
  }
  if (mysql_num_rows(queryResult) == 0) {
    // 4.1. If machine ID is not yet in the database, insert it
  } else {
    if (mysql_num_rows(queryResult) > 1) {
      fprintf(stderr, "[%s:%u] - Warning: multiple entries for machine '%s'\n", __FILE__, __LINE__, hostname);
      // TODO Handle warning?
    }
    MYSQL_ROW row = mysql_fetch_row(queryResult);
    if (NULL == row[0]) {
      // TODO Handle error
    }
    machineID = std::string(row[0]);
  }
  mysql_free_result(queryResult);
  queryResult = NULL;

  // TODO 5. Get platform and site IDs
  // We're going to use the platform ID for the machine's current platform.
  // If it's not in there, we'll leave the plaform field in the results blank
  if (machineID.empty() == false) {
    queryStream.str(std::string());
    queryStream << "select platformID, siteID from machine where "
		<< "id = " << machineID;
    if (mysql_query(db, queryStream.str().c_str()) != 0) {
      fprintf(stderr, "[%s:%u] - Error submitting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
      // TODO Handle error
    }
    queryResult = mysql_store_result(db);
    if (NULL == queryResult) {
      fprintf(stderr, "[%s:%u] - Error retrieving result: '%s'\n", __FILE__, __LINE__, mysql_error(db));
      // TODO Handle error
    } else {
      if (mysql_num_rows(queryResult) == 0) {
	// This shouldn't happen..
	fprintf(stderr, "[%s:%u] - Can't find machine row we just inserted?!\n", __FILE__, __LINE__);
	// TODO Handle weird case where we can't find the row we just inserted
      } else {
	MYSQL_ROW row = mysql_fetch_row(queryResult);
	if (NULL == row[0]) {
	  fprintf(stderr, "[%s:%u] - row contains NULL for platformID\n", __FILE__, __LINE__);
	  // TODO Handle this case
	} else {
	  platformID = std::string(row[0]);
	}
	if (NULL == row[1]) {
	  fprintf(stderr, "[%s:%u] - row contains NULL for siteID\n", __FILE__, __LINE__);
	  // TODO Handle this case
	} else {
	  siteID = std::string(row[1]);
	}
      }
    }
  }
  mysql_free_result(queryResult);
  queryResult = NULL;

  // 7. Get user ID
  std::string userName = std::string("dbtester");
  std::string userEmail = std::string("cooksey@cs.wisc.edu");
  std::string userID;
  query = std::string("select id from user where ")
    + std::string("email = binary '") + userEmail
    + std::string("' and name = binary '") + userName + std::string("'");
  if (mysql_query(db, query.c_str()) != 0) {
    fprintf(stderr, "[%s:%u] - Error submiting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
    // TODO Handle error
  }
  queryResult = mysql_store_result(db);
  if (NULL == queryResult) {
    fprintf(stderr, "[%s:%u] - Error retrieving result: '%s'\n", __FILE__, __LINE__, mysql_error(db));
    // TODO Handle error
  }
  if (mysql_num_rows(queryResult) == 0) {
    // 7.1. If user ID is not yet in the database, insert it
    queryStream.str(std::string()); // Clear the buffer
    queryStream << "insert into user set name = binary '" << userName
		<< "', email = binary '" << userEmail << "'";
    if (mysql_query(db, queryStream.str().c_str()) != 0) {
      fprintf(stderr, "[%s:%u] - Error submitting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
      // TODO Handle error
    }
    sstream << mysql_insert_id(db);
    userID = std::string(sstream.str());
    sstream.str(std::string()); // Clear buffer
  } else {
    if (mysql_num_rows(queryResult) > 1) {
      fprintf(stderr, "[%s:%u] - WARNING: Multiple IDs for user %s <%s>\n", __FILE__, __LINE__, userName.c_str(), userEmail.c_str());
    }
    MYSQL_ROW row = mysql_fetch_row(queryResult);
    if (NULL == row) {
      fprintf(stderr, "[%s:%u] - Error fetching row: '%s'\n", __FILE__, __LINE__, mysql_error(db));
      // TODO Handle error
    }
    if (NULL == row[0]) {
      fprintf(stderr, "[%s:%u] - User ID is null?\n", __FILE__, __LINE__);
      // TODO Handle error
    }
    userID = std::string(row[0]);
  }
  mysql_free_result(queryResult);
  queryResult = NULL;

  // TODO 8. Get date
  // This is filled in automatically as the time when the result is submitted
  // to the database.  If there's a problem submitting the results, then we
  // want to store the current time rather than when the results are submitted.

  // 9. Read result text from database log
  std::stringstream resultStream;
  std::ifstream logFile;
  logFile.open(dblogFilename.c_str());
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
  long strLength = resultStream.str().size();
  char *resultText = new char [(2 * strLength) + 1];
  mysql_real_escape_string(db, resultText, resultStream.str().c_str(),
			   strLength);

  // If result == UNKNOWN, there may be a line in the database log of the form
  // RESULT: <code>
  // with the results of this test.
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
  }

  // 10. Submit result to database
  queryStream.str(std::string()); // Clear buffer
  queryStream << "insert into result set testID=" << testID
	      << ", userID=" << userID
	      << ", machineID=" << machineID;
  if (platformID.empty() == false) {
    queryStream << ", platformID = " << platformID;
  }
  if (siteID.empty() == false) {
    queryStream << ", siteID = " << siteID;
  }
  queryStream << ", resultText = binary '" << resultText << "'"
	      << ", outcome=" << result;
  if (mysql_query(db, queryStream.str().c_str()) != 0) {
    fprintf(stderr, "[%s:%u] - Error submitting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
    // TODO Handle error
  }
  if (mysql_affected_rows(db) == (my_ulonglong) -1) {
    fprintf(stderr, "[%s:%u] - Error inserting test result: '%s'\n", __FILE__, __LINE__, mysql_error(db));
    // TODO Handle error
  } else if (mysql_affected_rows(db) == 0) {
    fprintf(stderr, "[%s:%u] - Test result not inserted into database?\n", __FILE__, __LINE__);
    // TODO Handle error
  }
  delete [] resultText;
  resultText = NULL;

  // TODO 11. Track the first & last run dates for this platform
  if (platformID.empty() == false) {
    queryStream.str(std::string());
    queryStream << "select firstRun from platform where id = "
		<< platformID;
    if (mysql_query(db, queryStream.str().c_str()) != 0) {
      fprintf(stderr, "[%s:%u] - Error submitting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
      // TODO Handle error
    }
    queryResult = mysql_store_result(db);
    if (NULL == queryResult) {
      fprintf(stderr, "[%s:%u] - Error retrieving result: '%s'\n", __FILE__, __LINE__, mysql_error(db));
      // TODO Handle error
    } else {
      if (mysql_num_rows(queryResult) == 0) {
	fprintf(stderr, "[%s:%u] - No such platform matching ID %d\n", __FILE__, __LINE__, platformID.c_str());
	// TODO Handle error
      } else {
	MYSQL_ROW row = mysql_fetch_row(queryResult);
	if (NULL == row) {
	  fprintf(stderr, "[%s:%u] - NULL row?\n", __FILE__, __LINE__);
	  // TODO Handle error
	} else {
	  if (NULL == row[0]) {
	    queryStream.str(std::string());
	    queryStream << "update platform set firstRun = now() where id = "
			<< platformID;
	    if (mysql_query(db, queryStream.str().c_str()) != 0) {
	      fprintf(stderr, "[%s:%u] - Error submitting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
	      // TODO Handle error
	    }
	  }
	}
      }
    }
    mysql_free_result(queryResult);
    queryResult = NULL;

    // Set lastRun
    queryStream.str(std::string());
    queryStream << "update platform set lastRun = now() where id = "
		<< platformID;
    if (mysql_query(db, queryStream.str().c_str()) != 0) {
      fprintf(stderr, "[%s:%u] - Error submitting query: '%s'\n", __FILE__, __LINE__, mysql_error(db));
      // TODO Handle error
    }
  }

  // 12. Mark this test as having been submitted to the database
  mysql_close(db);
  submittedResults = true;

  // Remove the old dblog file
  unlink(dblogFilename.c_str());
}

// This is called if there is an error submitting results to the database.
// It writes the test results to a log file that can be read or later
// resubmitted to the database
void DatabaseOutputDriver::failedResultSubmission(std::string message) {
  std::string filename = std::string("failed_") + dblogFilename;
  std::ofstream out;
  out.open(filename.c_str(), std::ios::app);

  // 1. Write a test label to the file
  out << "BEGIN TEST\n{";
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
  std::ifstream in(dblogFilename.c_str());
  in.seekg(0, std::ifstream::end);
  long size = in.tellg();
  in.seekg(0);
  char *buffer = new char[size];
  in.read(buffer, size);
  out.write(buffer, size);
  in.close();
  delete [] buffer;

  // 3. Write error message to file
  out << "\n" << message << "\n";

  // 4. Write a result code to the file
  char *resultCodes[] = {"UNKNOWN", "PASSED", "FAILED", "SKIPPED", "CRASHED"};
  out << "RESULT: " << resultCodes[result] << "\n\n";
  out.close();

  // Clear the old dblog file
  truncate(dblogFilename.c_str(), 0);
}

void DatabaseOutputDriver::getMutateeArgs(std::vector<std::string> &args) {
  args.clear();
  args.push_back(std::string("-dboutput"));
}
