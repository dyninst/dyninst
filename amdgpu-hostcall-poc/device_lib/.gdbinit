set env ROCR_VISIBLE_DEVICES=1
set breakpoint pending on
set env LD_PRELOAD /home/wuxx1279/KR/preload/preload.so
#b hipLaunchKernel
#r
#c
#d 1
#b empty
#c
#b *empty+0x28
#d 2
#c
#d 3
#b *empty+0x3c
#c
#d 4
#c
#d 3
#c


