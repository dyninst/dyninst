rem script for building Paradyn & DynInstAPI on WinNT platform

cmd /c "cd util\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd igen\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd rtinst\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd paradynd\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd dyninstAPI\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd dyninstAPI_RT\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd dyninstAPI\tests\%PLATFORM% && nmake clean && nmake install"

