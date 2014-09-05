#!/usr/bin/python

import os
import sys
import string
import datetime
import subprocess


# Build array of [syscall name, pc, return value] tuples
def processStraceLog(fileName):
    print "Processing " + fileName

    allSyscalls = []
   
    with open(fileName, "r") as fileHandle: 
        for line in fileHandle:
            line = line.rstrip()

            # Skip non-syscall lines
            if (line.find("[") != 0):
                # Skip
                continue

            pc,brace,rest = line.partition("]")
            pc = pc.lstrip("[")
            pc = pc.lstrip()
            rest = rest.lstrip()
            name,paren,args = rest.partition("(")

            args,paren,rest = args.partition(")")
            space,equal,retinfo = rest.partition("=")

            # Strip error code explanations
            retAndErrno,paren,errmsg = retinfo.partition("(")
            retAndErrno = retAndErrno.lstrip().rstrip()

            retvalue=""
            errno=""
            if (paren == ""):
                retvalue = retAndErrno
            else:
                # Separate retvalue from errno
                retvalue,space,errno = retAndErrno.partition(" ")

                # Reformat errmsg
                errmsg = errmsg.rsplit(")")[0]

            curSyscall = []
            curSyscall.append(name)
            curSyscall.append(pc)
            curSyscall.append(retvalue)
            if (errno != ""):
                curSyscall.append(errno)
                curSyscall.append(errmsg)

            allSyscalls.append(curSyscall)

        return allSyscalls

def processToolLog(fileName):
    print "Processing " + fileName

    allSyscalls = []
   
    with open(fileName, "r") as fileHandle: 
        for line in fileHandle:
            line = line.rstrip()

            # Skip non-syscall lines
            if (line.find("[#") != 0):
                continue
        
            number, colon, rest = line.partition(":")
            rest = rest.lstrip()
            name,pc,retvalue = rest.split(",")
            retvalue = retvalue.rstrip("]")

            curSyscall = []
            curSyscall.append(name)
            curSyscall.append(pc)
            curSyscall.append(retvalue)

            allSyscalls.append(curSyscall)

        return allSyscalls

def runTool(curTool, flags, executable):
    # Build argument list
    args = curTool + " " + flags + " " + executable
    args = args.split(" ")

    # Create an output file based on the current date
    outfileName = OUT_DIR + "/log-" + curTool + "-" + DATE + ".log"
    
    with open(outfileName, "w") as outfile:
        print "output in " + outfileName

        # Run curTool 
        print args
        p = subprocess.Popen(args, cwd=os.getcwd(), stdout=outfile, stderr=outfile)
        # Wait for completion
        p.communicate()

        # Force flush
        os.fsync(outfile)

    return outfileName

def runStrace(curTool, flags, executable):
    # Build argument list
    args = curTool + " " + flags
    args = args.split(" ")

    # Create an output file based on the current date
    outfileName = OUT_DIR + "/log-" + curTool + "-" + DATE + ".log"
    
    with open(outfileName, "w") as outfile:
        print "output in " + outfileName

        # Run curTool 
        args.append(outfileName)
        args.append(executable)
        print args
        p = subprocess.Popen(args, cwd=os.getcwd())
        # Wait for completion
        p.communicate()

        # Force flush
        os.fsync(outfile)

    return outfileName

def compareOutput(toolAll, straceAll):
    print "Comparing tool and strace logs"

    # [##, name, string result]
    fullResults = []

    for index in range(len(toolAll)):
        result = []
        match = True

        curTool = toolAll[index]
        nameTool = curTool[0]
        pcTool = curTool[1]
        retvalueTool = curTool[2]

        curStrace = straceAll[index+1] # Off-by-one because of initial execve
        nameStrace = curStrace[0]
        pcStrace = curStrace[1]
        retvalueStrace = curStrace[2]
        errnoStrace = ""
        if (len(curStrace) == 5):
            errnoStrace = curStrace[3]
            errmsgStrace = curStrace[4]
      
        if (nameTool != nameStrace):
            result.append("name: " + nameTool + "; strace=" + nameStrace)
            match = False
        if (pcTool != pcStrace): 
            # Note that the PCs may not match due to load addresses
            result.append("pc: " + pcTool + "; strace=" + pcStrace)
        if (retvalueTool != retvalueStrace):
            retvalueToolDec = int(float(retvalueTool))
            
            # Okay, is strace printing hex?
            if (retvalueStrace.find("0x") == 0):
                # Yes! Convert retvalueTool accordingly
                retvalueToolHex = hex(retvalueToolDec)
                if (retvalueToolHex != retvalueStrace):
                    # The hex values don't match
                    # Current executive decision; if everything else has matched, and strace returns a
                    # hex-formatted value (and no error), assume that this system call returns a pointer,
                    # and assume they needn't match
                    if ((match == False) or (errnoStrace != "")):
                        match = False
                        result.append("retvalue: " + retvalueToolHex + "; strace=" + retvalueStrace)
            else:
                # Okay, what about error messages?
                if (errnoStrace != ""):
                    # Do the error messages match?
                    # NOTE: Strace returns the library wrapper error value (usually -1) and reports the error based
                    # on errno; ProcControl is returning the literal kernel return value, which can be negated
                    # to calculate errno. Due to string issues, we'll compare the string error messages rather than
                    # directly comparing errno
                    errmsgTool = os.strerror(abs(retvalueToolDec))
                    
                    if (errmsgTool != errmsgStrace):
                        # They don't match; keep in decimal
                        match = False
                        result.append("retvalue: " + retvalueTool + "; strace=" + retvalueStrace)
                else:
                    # Okay, are these system calls we'd expect to have different return values?
                    safeSet = []
                    safeSet.append("open")
                    safeSet.append("getpid")
                    safeSet.append("clone")
                    if (match == False) or (nameTool not in safeSet):
                        match = False
                        result.append("retvalue: " + retvalueTool + "; strace=" + retvalueStrace)
       
        if (match == False):
            print result
            curSyscall = []
            curSyscall.append(index+1)
            curSyscall.append(nameTool) # If the names don't match, this will appear in the result string
            curSyscall.append(result)
            fullResults.append(curSyscall)

    foundResults = False
    resultsfileName = OUT_DIR + "/results-" + DATE + ".log"
    with open(resultsfileName, "w") as resultsfile:
        # Check that tool and strace recorded same number of system calls
        if (len(toolAll) != (len(straceAll)-1)):
            resultsfile.write("Tool recorded " + str(len(toolAll)) + " system calls; strace recorded " + str(len(straceAll)-1) + " system calls.")
            foundResults = True
        
        # Record any mis-matches we found
        if (len(fullResults) != 0):
            foundResults = True
            for res in fullResults:
                resultsfile.write(str(res) + "\n")

    if (foundResults):
        print "Found inconsistencies -- FAILED"
        print "Detailed results in " + resultsfileName
    else:
        print "Tool and strace logs matched -- SUCCESS"
        print "Detailed results in " + resultsfileName
        os.remove(resultsfileName)

################################################################################
# MAIN SCRIPT #
################################################################################
# Check arguments
arglen = len(sys.argv)
if arglen < 2:
    print "Usage: " + sys.argv[0] + " <filename> [<args>]"
    sys.exit(1)

DATE = datetime.datetime.today().strftime("%Y-%m-%d--%H-%M-%S")
OUT_DIR="results-" + DATE
os.mkdir(OUT_DIR)

executable = sys.argv[1]
if arglen == 3:
    executable = executable + " " + sys.argv[2]

print "Test program \"" + executable + "\""

# Build our sample executable and tool
args = []
args.append("make")
args.append(executable)
args.append("syscall_testing")
p = subprocess.Popen(args, cwd=os.getcwd())
p.communicate()[1]
ret = p.returncode
if (ret != 0):
    print "make failed; stopping"
    sys.exit(1)

# Run strace
straceLog =  runStrace("strace", "-i -o", executable)
straceAll = processStraceLog(straceLog)

# Run mutator tool
os.environ['DEBUG'] = '1'
toolLog = runTool("syscall_testing", "-o " + OUT_DIR + "/tool-stdout.txt -e " + OUT_DIR + "/tool-stderr.txt -b", executable)
toolAll = processToolLog(toolLog)

compareOutput(toolAll, straceAll)
