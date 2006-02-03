#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <unistd.h>

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_function.h"

#include "dyninstCore.h"
#include "config.h"
#include "ipc.h"
#include "log.h"
#include "utils.h"

using namespace std;

/*******************************************************************************
 * Monitor functions
 */
void stateBasedPrint(message *msgData, statusID prevStat, statusID currStat, int *tabDepth, logLevel priority)
{
    switch (currStat) {
    case ID_PASS:
	--(*tabDepth);
	if (prevStat == ID_TEST) {
	    switch (getMsgID(msgData->id_data)) {
	    case ID_INIT_CREATE_PROCESS: dlog(priority, "Mutatee launched as process %d.\n", msgData->int_data); break;
	    case ID_PARSE_MODULE: dlog(priority, "Found %d module(s).\n", msgData->int_data); break;
	    case ID_PARSE_FUNC: dlog(priority, "Found %d function(s).\n", msgData->int_data); break;
	    case ID_INST_GET_FUNCS: dlog(priority, "Retrieved %d function(s).\n", msgData->int_data); break;
	    default: dlog(priority, "done.\n"); break;
	    }
	    return;
	}
	break;

    case ID_WARN:
	--(*tabDepth);
	if (prevStat == ID_TEST) {
	    dlog(priority, "%s\n", msgData->str_data);
	    return;
	}
	break;

    case ID_FAIL:
	--(*tabDepth);
	if (prevStat == ID_TEST) {
	    dlog(priority, "failed.\n");
	}
	break;

    case ID_INFO:
    case ID_TEST:
	if (prevStat == ID_TEST)
	    dlog(priority, "\n");
	break;

    default:
	break;
    }

    if (*tabDepth < 0) dlog(ERR, "%d ", *tabDepth);
    for (int i = 0; i < *tabDepth; ++i) dlog(priority, "    ");

    switch (currStat) {
    case ID_FAIL: dlog(priority, "*** Failure: %s\n", msgData->str_data); break;
    case ID_PASS: dlog(priority, "done.\n"); break;
    case ID_WARN: dlog(priority, "Warning: %s\n", msgData->str_data); break;
    case ID_INFO:
    case ID_TEST:
	switch (getMsgID(msgData->id_data)) {
	case ID_PARSE_FUNC: dlog(priority, "Parsing function data from %s module... ", msgData->str_data); break;
	case ID_PARSE_MODULE_CFG: dlog(priority, "Parsing CFG data from functions in %s module... ", msgData->str_data); break;
	case ID_PARSE_FUNC_CFG: dlog(priority, "Parsing CFG data from %s function... ", msgData->str_data); break;
	case ID_INST_MODULE: dlog(priority, "Instrumenting functions in module %s... ", msgData->str_data); break;
	case ID_INST_FUNC: dlog(priority, "Instrumenting function %s... ", msgData->str_data); break;
	case ID_EXIT_CODE: dlog(priority, "Mutatee exited normally and returned %d\n", msgData->int_data); break;
	case ID_EXIT_SIGNAL: dlog(priority, "Mutatee exited via signal %d\n", msgData->int_data); break;
	case ID_DATA_STRING: dlog(priority, "%s\n", msgData->str_data); --(*tabDepth); break;
	default: dlog(priority, "%s... ", msgStr(getMsgID(msgData->id_data))); break;
	}
	++(*tabDepth);
    }
}

int launch_monitor(FILE *infd)
{
    statusID currStat = ID_FAIL;
    statusID prevStat = ID_FAIL, prevStatLog = ID_FAIL;
    int tabDepth = 0, tabDepthLog = 0;
    logLevel priority = INFO;
    message msgData;

    if (config.time_limit > 0)
	alarm(config.time_limit);

    while (config.state == NORMAL && readMsg(infd, &msgData)) {
	prevStatLog = currStat;
	if (priority <= config.verbose)
	    prevStat = currStat;
	currStat = getStatID(msgData.id_data);
	priority = getPriID(msgData.id_data);

	if (getMsgID(msgData.id_data) == ID_POST_FORK ||
	    getMsgID(msgData.id_data) == ID_INIT_CREATE_PROCESS && currStat == ID_PASS) {
	    config.grandchildren.insert(msgData.int_data);
	}

	if (priority <= config.verbose) {
	    stateBasedPrint(&msgData, prevStat, currStat, &tabDepth, priority);
	    tabDepthLog = tabDepth;
	} else {
	    stateBasedPrint(&msgData, prevStatLog, currStat, &tabDepthLog, LOG_ONLY);
	}

	free(msgData.str_data);
    }

    // Unset alarm
    alarm(0);

    if (config.state == NORMAL) {
	config.state = SIGCHLD_WAIT;
	dlog(INFO, "Waiting for mutator to exit...\n");
	sleep(10);
    }

    // Cleanup process group just in case.
    if (config.state != CHILD_EXITED)
	killProcess(-config.pid);

    return 0;
}

/*******************************************************************************
 * Mutator functions
 */

void printSummary(BPatch_thread *, BPatch_exitType);
void reportNewProcess(BPatch_thread *, BPatch_thread *);
bool insertSummary(BPatch_function *, BPatch_variableExpr *);
BPatch_variableExpr *allocateCounterInMutatee(dynHandle *, BPatch_function *);
bool instrumentFunctionEntry(dynHandle *, BPatch_function *);
bool instrumentFunctionExit(dynHandle *, BPatch_function *);
bool instrumentBasicBlocks(dynHandle *, BPatch_function *);
bool instrumentMemoryReads(dynHandle *, BPatch_function *);
bool instrumentMemoryWrites(dynHandle *, BPatch_function *);

int launch_mutator()
{
    unsigned i, j;
    const char *reason;
    char buf[STRING_MAX];
    BPatch_Vector< BPatch_module * > *appModules;
    BPatch_Vector< BPatch_function * > *appFunctions;
    BPatch_flowGraph *appCFG;

    dynHandle *dh = mutatorInit();
    if (!dh) return(-1);

    /**************************************************************
     * Parsing Phase
     */
    // Is this if redundant?  Will anybody run this without parsing the modules?
    if (config.parse_level >= PARSE_MODULE) {
	sendMsg(config.outfd, ID_PARSE_MODULE, INFO);
	appModules = dh->image->getModules();
	if (!appModules) {
	    sendMsg(config.outfd, ID_PARSE_MODULE, INFO, ID_FAIL,
		    "Failure in BPatch_image::getModules()");
	    return(-1);
	} else
	    sendMsg(config.outfd, ID_PARSE_MODULE, INFO, ID_PASS,
		    appModules->size());
    }

    if (config.parse_level >= PARSE_FUNC) {
	for (i = 0; i < appModules->size(); ++i) {
	    (*appModules)[i]->getName(buf, sizeof(buf));

	    // Check module inclusion/exclusion regular expression rules.
	    reason = config.mod_rules.getReason(buf);
	    if (reason) sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, reason);
	    if (!config.mod_rules.isValid(buf)) continue;

	    sendMsg(config.outfd, ID_PARSE_FUNC, INFO, ID_TEST, buf);
	    appFunctions = (*appModules)[i]->getProcedures();
	    if (!appFunctions) {
		sendMsg(config.outfd, ID_PARSE_FUNC, INFO, ID_FAIL,
			"Failure in BPatch_module::getProcedures()");
		return(-1);
	    } else
		sendMsg(config.outfd, ID_PARSE_FUNC, INFO, ID_PASS,
			appFunctions->size());

	    if (config.parse_level >= PARSE_CFG) {
		sendMsg(config.outfd, ID_PARSE_MODULE_CFG, INFO, ID_TEST,
			(*appModules)[i]->getName(buf, sizeof(buf)));

		int cfg_warn_cnt = 0, cfg_pass_cnt = 0;
		for (j = 0; j < appFunctions->size(); ++j) {
		    (*appFunctions)[j]->getName(buf, sizeof(buf));

		    // Check function inclusion/exclusion regular expression rules.
		    reason = config.func_rules.getReason(buf);
		    if (reason) sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, reason);
		    if (!config.func_rules.isValid(buf)) continue;

		    sendMsg(config.outfd, ID_PARSE_FUNC_CFG, VERB1, ID_TEST, buf);
		    appCFG = (*appFunctions)[j]->getCFG();
		    if (appCFG) {
			sendMsg(config.outfd, ID_PARSE_FUNC_CFG, VERB1, ID_WARN,
				"Failure in BPatch_function::getCFG()");
			++cfg_warn_cnt;
		    } else {
			sendMsg(config.outfd, ID_PARSE_FUNC_CFG, VERB1, ID_PASS);
			++cfg_pass_cnt;
		    }
		}
		if (cfg_warn_cnt)
		    sendMsg(config.outfd, ID_PARSE_MODULE_CFG, INFO, ID_WARN,
			    sprintf_static("%d warning(s), %d passed.", cfg_warn_cnt, cfg_pass_cnt));
		else
		    sendMsg(config.outfd, ID_PARSE_MODULE_CFG, INFO, ID_PASS);
	    }
	}
    }

    /**************************************************************
     * Instrumentation Phase
     */

    if (config.inst_level >= INST_FUNC_ENTRY) {

	if (config.transMode == TRANS_PROCESS)
	    if (!dynStartTransaction(dh)) return(-1);

	for (i = 0; i < appModules->size(); ++i) {
	    int mod_warn_cnt = 0, mod_pass_cnt = 0;
	    (*appModules)[i]->getName(buf, sizeof(buf));

	    // Check module inclusion/exclusion regular expression rules.
	    reason = config.mod_rules.getReason(buf);
	    if (reason) sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, reason);
	    if (!config.mod_rules.isValid(buf)) continue;

	    if (config.transMode == TRANS_MODULE)
		if (!dynStartTransaction(dh)) continue;

	    sendMsg(config.outfd, ID_INST_MODULE, INFO, ID_TEST, buf);

	    sendMsg(config.outfd, ID_INST_GET_FUNCS, VERB1);
	    appFunctions = (*appModules)[i]->getProcedures();
	    if (!appFunctions) {
		sendMsg(config.outfd, ID_INST_GET_FUNCS, VERB1, ID_FAIL,
			"Failure in BPatch_module::getProcedures()");
		return(-1);
	    } else
		sendMsg(config.outfd, ID_INST_GET_FUNCS, VERB1, ID_PASS,
			appFunctions->size());

	    for (j = 0; j < appFunctions->size(); ++j) {
		int func_warn_cnt = 0, func_pass_cnt = 0;
		(*appFunctions)[j]->getName(buf, sizeof(buf));

		// Check function inclusion/exclusion regular expression rules.
		reason = config.func_rules.getReason(buf);
		if (reason) sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, reason);
		if (!config.func_rules.isValid(buf)) continue;

		if (config.transMode == TRANS_FUNCTION)
		    if (!dynStartTransaction(dh)) continue;

		sendMsg(config.outfd, ID_INST_FUNC, VERB1, ID_TEST, buf);

		if (config.inst_level >= INST_FUNC_ENTRY) {
		    if (!instrumentFunctionEntry(dh, (*appFunctions)[j]))
			++func_warn_cnt;
		    else
			++func_pass_cnt;
		}

		if (config.inst_level >= INST_FUNC_EXIT) {
		    if (!instrumentFunctionExit(dh, (*appFunctions)[j]))
			++func_warn_cnt;
		    else
			++func_pass_cnt;
		}

		if (config.inst_level >= INST_BASIC_BLOCK) {
		    if (!instrumentBasicBlocks(dh, (*appFunctions)[j]))
			++func_warn_cnt;
		    else
			++func_pass_cnt;
		}

		if (config.inst_level >= INST_MEMORY_READ) {
		    if (!instrumentMemoryReads(dh, (*appFunctions)[j]))
			++func_warn_cnt;
		    else
			++func_pass_cnt;
		}

		if (config.inst_level >= INST_MEMORY_WRITE) {
		    if (!instrumentMemoryWrites(dh, (*appFunctions)[j]))
                        ++func_warn_cnt;
                    else
                        ++func_pass_cnt;
		}

		if (config.transMode == TRANS_FUNCTION)
		    if (!dynEndTransaction(dh)) {
			func_warn_cnt = func_pass_cnt;
			func_pass_cnt = 0;
		    }

		if (func_warn_cnt) {
		    sendMsg(config.outfd, ID_INST_FUNC, VERB1, ID_WARN,
			    sprintf_static("%d warning(s), %d passed.", func_warn_cnt, func_pass_cnt));
		    ++mod_warn_cnt;
		} else {
		    sendMsg(config.outfd, ID_INST_FUNC, VERB1, ID_PASS);
		    ++mod_pass_cnt;
		}
	    }

	    if (config.transMode == TRANS_MODULE)
		if (!dynEndTransaction(dh)) {
		    mod_warn_cnt = mod_pass_cnt;
		    mod_pass_cnt = 0;
		}

	    if (mod_warn_cnt)
		sendMsg(config.outfd, ID_INST_MODULE, INFO, ID_WARN,
			sprintf_static("%d warning(s), %d passed.", mod_warn_cnt, mod_pass_cnt));
	    else
		sendMsg(config.outfd, ID_INST_MODULE, INFO, ID_PASS);
	}

	if (config.transMode == TRANS_PROCESS)
	    dynEndTransaction(dh);
    }

    sendMsg(config.outfd, ID_RUN_CHILD, INFO);
    if (!dynContinueExecution(dh)) {
        sendMsg(config.outfd, ID_RUN_CHILD, INFO, ID_FAIL,
                "Failure in BPatch_thread::continueExecution()");
        return false;
    }
    sendMsg(config.outfd, ID_RUN_CHILD, INFO, ID_PASS);

    sendMsg(config.outfd, ID_WAIT_TERMINATION, INFO);
    while (!dynIsTerminated(dh)) {
        sendMsg(config.outfd, ID_WAIT_STATUS_CHANGE, DEBUG);
        if (!dh->bpatch->waitForStatusChange())
	    sendMsg(config.outfd, ID_WAIT_STATUS_CHANGE, DEBUG, ID_FAIL);
	else
	    sendMsg(config.outfd, ID_WAIT_STATUS_CHANGE, DEBUG, ID_PASS);
    }
    sendMsg(config.outfd, ID_WAIT_TERMINATION, INFO, ID_PASS);

    BPatch_exitType mutatee_status = dynTerminationStatus(dh);
    switch (mutatee_status) {
    case ExitedNormally:
	sendMsg(config.outfd, ID_EXIT_CODE, INFO, ID_INFO, dynGetExitCode(dh));
	break;

    case ExitedViaSignal:
	sendMsg(config.outfd, ID_EXIT_SIGNAL, INFO, ID_INFO, dynGetExitSignal(dh));
	break;

    case NoExit:
	sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO,
		"Conflicting reports about mutatee status.");
	break;
    }
    return 0;
}

//
// Structures and functions used with DyninstAPI callbacks.
//
struct summaryElem {
    BPatch_function *func;
    vector< BPatch_variableExpr * > count;
};
static vector< summaryElem * > summary;

void printSummary(BPatch_thread *proc, BPatch_exitType exit_type)
{
    unsigned int i, j;
    char buf[STRING_MAX];
    char *ptr;
    vector<int> value;

    if (exit_type == ExitedViaSignal) {
	sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO,
		"Mutator cannot provide exit summary.  Mutatee terminated by signal.");

    } else if (exit_type == ExitedNormally) {
	if (config.summary) {
	    for (i = 0; i < summary.size(); ++i) {
		value.resize(summary[i]->count.size());

		bool used = false;
		for (j = 0; j < value.size(); ++j) {
		    summary[i]->count[j]->readValue(&value[j]);
		    if (value[j] > 0) used = true;
		}

		if (used) {
		    summary[i]->func->getModule()->getName(buf, sizeof(buf));
		    ptr = sprintf_static("[%s] ", buf);

		    summary[i]->func->getName(buf, sizeof(buf));
		    ptr = strcat_static(ptr, buf);

		    for (j = 0; j < value.size(); ++j)
			ptr = strcat_static(ptr, sprintf_static(" %d", value[j]));

		    sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, ptr);
		}
	    }
	}
    } else if (exit_type == NoExit) {
	sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO,
		"DyninstAPI error.  Exit callback executed before end of mutatee.");

    } else {
	sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO,
		"Monitor error.  Possibly linked with wrong version of DyninstAPI.\n");
    }
}

void reportNewProcess(BPatch_thread *parent, BPatch_thread *child)
{
    sendMsg(config.outfd, ID_POST_FORK, INFO, ID_INFO, child->getPid());
}

//
// Helper mutator functions.
//
bool insertSummary(BPatch_function *func, BPatch_variableExpr *expr)
{
    sendMsg(config.outfd, ID_SUMMARY_INSERT, DEBUG);

    summaryElem *elem;
    vector< summaryElem * >::iterator iter = summary.begin();

    while (iter != summary.end()) {
	elem = *iter;
	if (elem->func == func) break;
	++iter;
    }

    if (iter == summary.end()) {
	elem = new summaryElem;
	if (!elem) {
	    sendMsg(config.outfd, ID_SUMMARY_INSERT, DEBUG, ID_FAIL,
		    "Memory allocation of new summary element failed");
	    return false;
	}
	elem->func = func;
	summary.push_back(elem);
    }

    elem->count.push_back(expr);
    sendMsg(config.outfd, ID_SUMMARY_INSERT, DEBUG, ID_PASS);

    return true;
}

BPatch_variableExpr *allocateCounterInMutatee(dynHandle *dh, BPatch_function *func)
{
    int zero = 0;
    BPatch_variableExpr *countVar;

    sendMsg(config.outfd, ID_ALLOC_COUNTER, VERB3);

    sendMsg(config.outfd, ID_INST_FIND_INT, VERB4);
    BPatch_type *intType = dh->image->findType("int");
    if (!intType) {
	sendMsg(config.outfd, ID_INST_FIND_INT, VERB4, ID_FAIL,
		"BPatch_image::findType(\"int\")");
	goto fail;
    } else
	sendMsg(config.outfd, ID_INST_FIND_INT, VERB4, ID_PASS);

    sendMsg(config.outfd, ID_INST_MALLOC_INT, VERB4);
    countVar = dh->proc->malloc(*intType);
    if (!countVar) {
	sendMsg(config.outfd, ID_INST_MALLOC_INT, VERB4, ID_FAIL,
		"Failure in BPatch_process::malloc()");
	goto fail;

    } else if (!countVar->writeValue(&zero)) {
	sendMsg(config.outfd, ID_INST_MALLOC_INT, VERB4, ID_FAIL,
		"Failure initializing counter in mutatee [BPatch_variableExpr::writeValue()]");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_INST_MALLOC_INT, VERB4, ID_PASS);
    }

    if (!insertSummary(func, countVar))
	goto fail;

    sendMsg(config.outfd, ID_ALLOC_COUNTER, VERB3, ID_PASS);
    return(countVar);

  fail:
    sendMsg(config.outfd, ID_ALLOC_COUNTER, VERB3, ID_FAIL);
    return(NULL);
}

//
// Instrumentation code.
//

bool instrumentFunctionEntry(dynHandle *dh, BPatch_function *func)
{
    BPatch_Vector<BPatch_point *> *points;
    BPatchSnippetHandle *handle;
    BPatch_arithExpr incSnippet(BPatch_negate, BPatch_snippet());  // Dummy constructor

    sendMsg(config.outfd, ID_INST_FUNC_ENTRY, VERB2);

    BPatch_variableExpr *countVar = allocateCounterInMutatee(dh, func);
    if (!countVar)
	goto fail;

    // Should we test for errors on this?
    incSnippet = BPatch_arithExpr( BPatch_assign, *countVar,
				   BPatch_arithExpr(BPatch_plus, *countVar,
						    BPatch_constExpr(1)));

    sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3);
    points = func->findPoint(BPatch_entry);
    if (!points) {
	sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_FAIL,
		"Failure in BPatch_function::findPoint(BPatch_entry)");
	goto fail;

    } else if (points->size() == 0) {
	sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_WARN,
		"No instrumentation points found in function");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_PASS);
    }

    sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3);
    handle = dh->proc->insertSnippet(incSnippet, *points);
    if (!handle) {
	sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3, ID_FAIL,
		"Failure in BPatch_process::insertSnippet()");
	goto fail;
    } else
	sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3, ID_PASS);

    sendMsg(config.outfd, ID_INST_FUNC_ENTRY, VERB2, ID_PASS);
    return true;

  fail:
    sendMsg(config.outfd, ID_INST_FUNC_ENTRY, VERB2, ID_WARN,
	    "Failure while instrumenting function entry.");
    return false;
}

bool instrumentFunctionExit(dynHandle *dh, BPatch_function *func)
{
    BPatch_Vector<BPatch_point *> *points;
    BPatchSnippetHandle *handle;
    BPatch_arithExpr incSnippet(BPatch_negate, BPatch_snippet());  // Dummy constructor

    sendMsg(config.outfd, ID_INST_FUNC_EXIT, VERB2);

    BPatch_variableExpr *countVar = allocateCounterInMutatee(dh, func);
    if (!countVar)
	goto fail;

    // Should we test for errors on this?
    incSnippet = BPatch_arithExpr( BPatch_assign, *countVar,
				   BPatch_arithExpr(BPatch_plus, *countVar,
						    BPatch_constExpr(1)));

    sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3);
    points = func->findPoint(BPatch_exit);
    if (!points) {
	sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_FAIL,
		"Failure in BPatch_function::findPoint(BPatch_exit)");
	goto fail;

    } else if (points->size() == 0) {
	sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_WARN,
		"No instrumentation points found in function");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_PASS);
    }

    sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3);
    handle = dh->proc->insertSnippet(incSnippet, *points);
    if (!handle) {
	sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3, ID_FAIL,
		"Failure in BPatch_process::insertSnippet()");
	goto fail;

    } else
	sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3, ID_PASS);

    sendMsg(config.outfd, ID_INST_FUNC_EXIT, VERB2, ID_PASS);
    return true;

  fail:
    sendMsg(config.outfd, ID_INST_FUNC_EXIT, VERB2, ID_WARN,
	    "Failure while instrumenting function exit.");
    return false;
}

bool instrumentBasicBlocks(dynHandle *dh, BPatch_function *func)
{
    BPatch_Set<BPatch_basicBlock*> allBlocks;
    BPatch_variableExpr *countVar;
    BPatch_arithExpr incSnippet(BPatch_negate, BPatch_snippet());  // Dummy constructor
    BPatch_Set<BPatch_opCode> ops;
    BPatch_Set<BPatch_basicBlock*>::iterator iter;
    int bb_warn_cnt = 0, bb_pass_cnt = 0;

    sendMsg(config.outfd, ID_INST_BASIC_BLOCK, VERB2);

    sendMsg(config.outfd, ID_GET_CFG, VERB3);
    BPatch_flowGraph *appCFG = func->getCFG();
    if (!appCFG) {
	sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_FAIL,
		"Failure in BPatch_function::getCFG()");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_PASS);
    }

    sendMsg(config.outfd, ID_INST_GET_BB, VERB3);
    if (appCFG->getAllBasicBlocks(allBlocks)) {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_FAIL,
		"Failure in BPatch_flowGraph::getAllBasicBlocks()");
	goto fail;

    } else if (allBlocks.size() == 0) {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_WARN,
		"No basic blocks found in function");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_PASS);
    }

    countVar = allocateCounterInMutatee(dh, func);
    if (!countVar)
	goto fail;

    // Should we test for errors on this?
    incSnippet = BPatch_arithExpr( BPatch_assign, *countVar,
				   BPatch_arithExpr(BPatch_plus, *countVar,
						    BPatch_constExpr(1)));
    ops.insert(BPatch_opLoad);
    ops.insert(BPatch_opStore);

    sendMsg(config.outfd, ID_INST_BB_LIST, VERB3);
    for (iter = allBlocks.begin(); iter != allBlocks.end(); iter++) {
	sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4);
	BPatch_Vector<BPatch_point*> *points = (*iter)->findPoint(ops);
	if (!points) {
	    sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
		    "Failure in BPatch_basicBlock::findPoint()");
	    ++bb_warn_cnt;
	    continue;

	} else if (points->size() == 0) {
	    sendMsg(config.outfd,  ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
		    "No instrumentation points found in basic block");
	    ++bb_warn_cnt;
	    continue;

	} else {
	    sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_PASS);
	}

	sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4);
	BPatchSnippetHandle *handle = dh->proc->insertSnippet(incSnippet, *(*points)[0]);
	if (!handle) {
	    sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_FAIL,
		    "Failure in BPatch_process::insertSnippet()");
	    ++bb_warn_cnt;
	    continue;

	} else {
	    sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_PASS);
	    ++bb_pass_cnt;
	}
    }
    if (bb_warn_cnt)
	sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_WARN,
		sprintf_static("%d warning(s), %d passed.", bb_warn_cnt, bb_pass_cnt));
    else
	sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_PASS);

    sendMsg(config.outfd, ID_INST_BASIC_BLOCK, VERB2, ID_PASS);
    return true;

  fail:
    sendMsg(config.outfd, ID_INST_BASIC_BLOCK, VERB2, ID_WARN,
	    "Failure while instrumenting basic blocks.");
    return false;
}

bool instrumentMemoryReads(dynHandle *dh, BPatch_function *func)
{
    BPatch_Set<BPatch_basicBlock*> allBlocks;
    BPatch_variableExpr *countVar;
    BPatch_arithExpr incSnippet(BPatch_negate, BPatch_snippet());  // Dummy constructor
    BPatch_Set<BPatch_opCode> ops;
    BPatch_Set<BPatch_basicBlock*>::iterator iter;
    int bb_warn_cnt = 0, bb_pass_cnt = 0;

    sendMsg(config.outfd, ID_INST_MEM_READ, VERB2);

    sendMsg(config.outfd, ID_GET_CFG, VERB3);
    BPatch_flowGraph *appCFG = func->getCFG();
    if (!appCFG) {
	sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_FAIL,
		"Failure in BPatch_function::getCFG()");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_PASS);
    }

    sendMsg(config.outfd, ID_INST_GET_BB, VERB3);
    if (appCFG->getAllBasicBlocks(allBlocks)) {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_FAIL,
		"Failure in BPatch_flowGraph::getAllBasicBlocks()");
	goto fail;

    } else if (allBlocks.size() == 0) {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_WARN,
		"No basic blocks found in function");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_PASS);
    }

    countVar = allocateCounterInMutatee(dh, func);
    if (!countVar)
	goto fail;

    // Should we test for errors on this?
    incSnippet = BPatch_arithExpr( BPatch_assign, *countVar,
				   BPatch_arithExpr(BPatch_plus, *countVar,
						    BPatch_constExpr(1)));
    ops.insert(BPatch_opLoad);

    sendMsg(config.outfd, ID_INST_BB_LIST, VERB3);
    for (iter = allBlocks.begin(); iter != allBlocks.end(); iter++) {
	sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4);
	BPatch_Vector<BPatch_point*> *points = (*iter)->findPoint(ops);
	if (!points) {
	    sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
		    "Failure in BPatch_basicBlock::findPoint()");
	    ++bb_warn_cnt;
	    continue;

	} else if (points->size() == 0) {
	    sendMsg(config.outfd,  ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
		    "No instrumentation points found in basic block");
	    ++bb_warn_cnt;
	    continue;

	} else {
	    sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_PASS);
	}

	sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4);
	BPatchSnippetHandle *handle = dh->proc->insertSnippet(incSnippet, *points);
	if (!handle) {
	    sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_FAIL,
		    "Failure in BPatch_process::insertSnippet()");
	    ++bb_warn_cnt;
	    continue;

	} else {
	    sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_PASS);
	    ++bb_pass_cnt;
	}
    }
    if (bb_warn_cnt)
	sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_WARN,
		sprintf_static("%d warning(s), %d passed.", bb_warn_cnt, bb_pass_cnt));
    else
	sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_PASS);

    sendMsg(config.outfd, ID_INST_MEM_READ, VERB2, ID_PASS);
    return true;

  fail:
    sendMsg(config.outfd, ID_INST_MEM_READ, VERB2, ID_WARN,
	    "Failure while instrumenting memory reads.");
    return false;
}

bool instrumentMemoryWrites(dynHandle *dh, BPatch_function *func)
{
    BPatch_Set<BPatch_basicBlock*> allBlocks;
    BPatch_variableExpr *countVar;
    BPatch_arithExpr incSnippet(BPatch_negate, BPatch_snippet());  // Dummy constructor
    BPatch_Set<BPatch_opCode> ops;
    BPatch_Set<BPatch_basicBlock*>::iterator iter;
    int bb_warn_cnt = 0, bb_pass_cnt = 0;

    sendMsg(config.outfd, ID_INST_MEM_WRITE, VERB2);

    sendMsg(config.outfd, ID_GET_CFG, VERB3);
    BPatch_flowGraph *appCFG = func->getCFG();
    if (!appCFG) {
	sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_FAIL,
		"Failure in BPatch_function::getCFG()");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_PASS);
    }

    sendMsg(config.outfd, ID_INST_GET_BB, VERB3);
    if (appCFG->getAllBasicBlocks(allBlocks)) {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_FAIL,
		"Failure in BPatch_flowGraph::getAllBasicBlocks()");
	goto fail;

    } else if (allBlocks.size() == 0) {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_WARN,
		"No basic blocks found in function");
	goto fail;

    } else {
	sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_PASS);
    }

    countVar = allocateCounterInMutatee(dh, func);
    if (!countVar)
	goto fail;

    // Should we test for errors on this?
    incSnippet = BPatch_arithExpr( BPatch_assign, *countVar,
				   BPatch_arithExpr(BPatch_plus, *countVar,
						    BPatch_constExpr(1)));
    ops.insert(BPatch_opStore);

    sendMsg(config.outfd, ID_INST_BB_LIST, VERB3);
    for (iter = allBlocks.begin(); iter != allBlocks.end(); iter++) {
	sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4);
	BPatch_Vector<BPatch_point*> *points = (*iter)->findPoint(ops);
	if (!points) {
	    sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
		    "Failure in BPatch_basicBlock::findPoint()");
	    ++bb_warn_cnt;
	    continue;

	} else if (points->size() == 0) {
	    sendMsg(config.outfd,  ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
		    "No instrumentation points found in basic block");
	    ++bb_warn_cnt;
	    continue;

	} else {
	    sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_PASS);
	}

	sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4);
	BPatchSnippetHandle *handle = dh->proc->insertSnippet(incSnippet, *points);
	if (!handle) {
	    sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_FAIL,
		    "Failure in BPatch_process::insertSnippet()");
	    ++bb_warn_cnt;
	    continue;

	} else {
	    sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_PASS);
	    ++bb_pass_cnt;
	}
    }
    if (bb_warn_cnt)
	sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_WARN,
		sprintf_static("%d warning(s), %d passed.", bb_warn_cnt, bb_pass_cnt));
    else
	sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_PASS);

    sendMsg(config.outfd, ID_INST_MEM_WRITE, VERB2, ID_PASS);
    return true;

  fail:
    sendMsg(config.outfd, ID_INST_MEM_WRITE, VERB2, ID_WARN,
	    "Failure while instrumenting memory writes.");
    return false;
}
