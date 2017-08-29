import os, sys
import re
d = os.listdir(".")
for f in d:
    fdata = ""
    with open(f) as curfile:
        fdata = curfile.read()
    fdata = re.sub(r"/", "/", fdata)
    o = open(f, "w")
    o.write(fdata)
    o.close()

