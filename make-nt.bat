rem
rem script for building paradyn on NT platform
rem

cmd /c "cd util\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd igen\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd rtinst\%PLATFORM% && nmake clean && nmake install"
cmd /c "cd paradynd\%PLATFORM% && nmake clean && nmake install"

