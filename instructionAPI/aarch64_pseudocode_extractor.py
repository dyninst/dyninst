import os, sys
import re

ISA_dir = '/p/paradyn/arm/arm-download-1350222/AR100-DA-70000-r0p0-00rel10/AR100-DA-70000-r0p0-00rel10/ISA_xml/ISA_xml_v2_00rel11/'
ISA_ps = '/p/paradyn/development/ssunny/dyninst/dyninst-code/instructionAPI/ISA_ps/'
files_dir = os.listdir(ISA_dir)
ifkeywords = ["else", "elsif"]
lineignorewords = ["Constrain", "CheckSPAlignment"]

for f in files_dir:
    parts = f.split('.')
    if len(parts) != 2 or parts[1] != "xml" or len(parts[0]) < 1:
        continue
    instr = parts[0]

    curfname = ISA_dir + instr + ".xml"
    fdata = ""
    with open(curfname) as curfile:
        fdata = curfile.read().replace('\n', '')
    if fdata.find("aliasto") != -1:
        refiformval = fdata.split("aliasto ")[1].split(" ")[0]
        curfname = ISA_dir + refiformval.split("=")[1].replace('\"', '')

    lines = list(open(curfname))
    idx = -1

    startPs = False
    isDecode = False
    isExecute = False

    blockidxstack = list()
    blockcount = 0
    
    decode = ""
    execute = ""
    for line in lines:
	idx += 1

        if line.find("<ps ") != -1:
            startPs = True
        # elif line.find("section=\"Decode\"") != -1:
        #    isDecode = True
        #    decode += line
        elif line.find("section=\"Execute\"") != -1:
            isExecute = True
            execute += line
	    if (line.find("if") != -1 and line.find("then") != -1) or (line.find("when") != -1):
		blockcount = blockcount + 1
		blockidxstack.append(curidx)
        elif line.find("</ps>") != -1:
            startPs = False
            # if isDecode == True:
            #	isDecode = False
            if isExecute == True:
                isExecute = False
		while blockcount > 0:
		    top = blockidxstack.pop()
		    execute += (" " * top)
		    execute += "end\n"
		    blockcount -= 1
        elif startPs == True:
            # if isDecode == True:
            #	decode += line
            if isExecute == True and any(kw in line for kw in lineignorewords) == False:
		curidx = len(line) - len(line.lstrip())
		
		if blockcount > 0 and (blockidxstack[len(blockidxstack) - 1] >= curidx or line.strip() == "") and any(kw in line for kw in ifkeywords) == False:
		    top = blockidxstack.pop()
		    
		    prevline = lines[idx - 1]
		    if prevline.find("if") == -1 and prevline.find("then") == -1:
			execute += (" " * top)
			execute += "end\n"
		    blockcount -= 1

		if (line.find("if") != -1 and line.find("then") != -1 and line.find("bits") != 0 and line.find("elsif") == -1) or (line.find("when") != -1):
		    blockcount = blockcount + 1
		    blockidxstack.append(curidx)
                
		execute += line
            else:
                print("!!!Found pseudocode section that is not decode or execute!!!")
            # sys.exit(0)
        else:
            continue

    decode = re.sub(r"<[^>]*>|^\s+|\'", "", decode)
    decode = re.sub(r"&amp;", "&", re.sub(r"&gt;", ">", re.sub(r"&lt;", "<", decode)))
    execute = re.sub(r"&amp;", "&", re.sub(r"&gt;", ">", re.sub(r"&lt;", "<", re.sub(r"<[^>]*>|^\s+|\'", "", execute))))
    execute = re.sub(r"Zeros\(\)", "Zeros(64)", execute)

    f = open(ISA_ps + instr, "w")
    if decode != "":
        f.write("##" + instr + "_" + "decode\n")
        f.write(decode)
        f.write("@@\n")
    if execute != "":
        f.write("##" + instr + "_" + "execute\n")
        f.write(execute)
        f.write("@@\n")
    f.close()
