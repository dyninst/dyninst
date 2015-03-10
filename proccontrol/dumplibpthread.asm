
/usr/lib64/libpthread-2.20.so:     file format elf64-littleaarch64


Disassembly of section .init:

0000000000004f28 <_init>:
    4f28:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    4f2c:	910003fd 	mov	x29, sp
    4f30:	940001f8 	bl	5710 <__pthread_initialize_minimal>
    4f34:	a8c17bfd 	ldp	x29, x30, [sp],#16
    4f38:	d65f03c0 	ret

Disassembly of section .plt:

0000000000004f40 <memcpy@plt-0x20>:
    4f40:	a9bf7bf0 	stp	x16, x30, [sp,#-16]!
    4f44:	f0000150 	adrp	x16, 2f000 <__FRAME_END__+0x18e30>
    4f48:	f947fe11 	ldr	x17, [x16,#4088]
    4f4c:	913fe210 	add	x16, x16, #0xff8
    4f50:	d61f0220 	br	x17
    4f54:	d503201f 	nop
    4f58:	d503201f 	nop
    4f5c:	d503201f 	nop

0000000000004f60 <memcpy@plt>:
    4f60:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4f64:	f9400211 	ldr	x17, [x16]
    4f68:	91000210 	add	x16, x16, #0x0
    4f6c:	d61f0220 	br	x17

0000000000004f70 <_exit@plt>:
    4f70:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4f74:	f9400611 	ldr	x17, [x16,#8]
    4f78:	91002210 	add	x16, x16, #0x8
    4f7c:	d61f0220 	br	x17

0000000000004f80 <strlen@plt>:
    4f80:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4f84:	f9400a11 	ldr	x17, [x16,#16]
    4f88:	91004210 	add	x16, x16, #0x10
    4f8c:	d61f0220 	br	x17

0000000000004f90 <exit@plt>:
    4f90:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4f94:	f9400e11 	ldr	x17, [x16,#24]
    4f98:	91006210 	add	x16, x16, #0x18
    4f9c:	d61f0220 	br	x17

0000000000004fa0 <_setjmp@plt>:
    4fa0:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4fa4:	f9401211 	ldr	x17, [x16,#32]
    4fa8:	91008210 	add	x16, x16, #0x20
    4fac:	d61f0220 	br	x17

0000000000004fb0 <__getrlimit@plt>:
    4fb0:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4fb4:	f9401611 	ldr	x17, [x16,#40]
    4fb8:	9100a210 	add	x16, x16, #0x28
    4fbc:	d61f0220 	br	x17

0000000000004fc0 <__gettimeofday@plt>:
    4fc0:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4fc4:	f9401a11 	ldr	x17, [x16,#48]
    4fc8:	9100c210 	add	x16, x16, #0x30
    4fcc:	d61f0220 	br	x17

0000000000004fd0 <twalk@plt>:
    4fd0:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4fd4:	f9401e11 	ldr	x17, [x16,#56]
    4fd8:	9100e210 	add	x16, x16, #0x38
    4fdc:	d61f0220 	br	x17

0000000000004fe0 <__libc_dlclose@plt>:
    4fe0:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4fe4:	f9402211 	ldr	x17, [x16,#64]
    4fe8:	91010210 	add	x16, x16, #0x40
    4fec:	d61f0220 	br	x17

0000000000004ff0 <__libc_fatal@plt>:
    4ff0:	90000170 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    4ff4:	f9402611 	ldr	x17, [x16,#72]
    4ff8:	91012210 	add	x16, x16, #0x48
    4ffc:	d61f0220 	br	x17

0000000000005000 <__getpagesize@plt>:
    5000:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5004:	f9402a11 	ldr	x17, [x16,#80]
    5008:	91014210 	add	x16, x16, #0x50
    500c:	d61f0220 	br	x17

0000000000005010 <__cxa_finalize@plt>:
    5010:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5014:	f9402e11 	ldr	x17, [x16,#88]
    5018:	91016210 	add	x16, x16, #0x58
    501c:	d61f0220 	br	x17

0000000000005020 <sprintf@plt>:
    5020:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5024:	f9403211 	ldr	x17, [x16,#96]
    5028:	91018210 	add	x16, x16, #0x60
    502c:	d61f0220 	br	x17

0000000000005030 <getuid@plt>:
    5030:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5034:	f9403611 	ldr	x17, [x16,#104]
    5038:	9101a210 	add	x16, x16, #0x68
    503c:	d61f0220 	br	x17

0000000000005040 <tfind@plt>:
    5040:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5044:	f9403a11 	ldr	x17, [x16,#112]
    5048:	9101c210 	add	x16, x16, #0x70
    504c:	d61f0220 	br	x17

0000000000005050 <fclose@plt>:
    5050:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5054:	f9403e11 	ldr	x17, [x16,#120]
    5058:	9101e210 	add	x16, x16, #0x78
    505c:	d61f0220 	br	x17

0000000000005060 <__libc_system@plt>:
    5060:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5064:	f9404211 	ldr	x17, [x16,#128]
    5068:	91020210 	add	x16, x16, #0x80
    506c:	d61f0220 	br	x17

0000000000005070 <fopen@plt>:
    5070:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5074:	f9404611 	ldr	x17, [x16,#136]
    5078:	91022210 	add	x16, x16, #0x88
    507c:	d61f0220 	br	x17

0000000000005080 <malloc@plt>:
    5080:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5084:	f9404a11 	ldr	x17, [x16,#144]
    5088:	91024210 	add	x16, x16, #0x90
    508c:	d61f0220 	br	x17

0000000000005090 <memset@plt>:
    5090:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5094:	f9404e11 	ldr	x17, [x16,#152]
    5098:	91026210 	add	x16, x16, #0x98
    509c:	d61f0220 	br	x17

00000000000050a0 <__libc_fork@plt>:
    50a0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    50a4:	f9405211 	ldr	x17, [x16,#160]
    50a8:	91028210 	add	x16, x16, #0xa0
    50ac:	d61f0220 	br	x17

00000000000050b0 <__endmntent@plt>:
    50b0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    50b4:	f9405611 	ldr	x17, [x16,#168]
    50b8:	9102a210 	add	x16, x16, #0xa8
    50bc:	d61f0220 	br	x17

00000000000050c0 <tsearch@plt>:
    50c0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    50c4:	f9405a11 	ldr	x17, [x16,#176]
    50c8:	9102c210 	add	x16, x16, #0xb0
    50cc:	d61f0220 	br	x17

00000000000050d0 <sscanf@plt>:
    50d0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    50d4:	f9405e11 	ldr	x17, [x16,#184]
    50d8:	9102e210 	add	x16, x16, #0xb8
    50dc:	d61f0220 	br	x17

00000000000050e0 <_dl_deallocate_tls@plt>:
    50e0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    50e4:	f9406211 	ldr	x17, [x16,#192]
    50e8:	91030210 	add	x16, x16, #0xc0
    50ec:	d61f0220 	br	x17

00000000000050f0 <calloc@plt>:
    50f0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    50f4:	f9406611 	ldr	x17, [x16,#200]
    50f8:	91032210 	add	x16, x16, #0xc8
    50fc:	d61f0220 	br	x17

0000000000005100 <__call_tls_dtors@plt>:
    5100:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5104:	f9406a11 	ldr	x17, [x16,#208]
    5108:	91034210 	add	x16, x16, #0xd0
    510c:	d61f0220 	br	x17

0000000000005110 <realloc@plt>:
    5110:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5114:	f9406e11 	ldr	x17, [x16,#216]
    5118:	91036210 	add	x16, x16, #0xd8
    511c:	d61f0220 	br	x17

0000000000005120 <__libc_thread_freeres@plt>:
    5120:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5124:	f9407211 	ldr	x17, [x16,#224]
    5128:	91038210 	add	x16, x16, #0xe0
    512c:	d61f0220 	br	x17

0000000000005130 <__sched_getparam@plt>:
    5130:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5134:	f9407611 	ldr	x17, [x16,#232]
    5138:	9103a210 	add	x16, x16, #0xe8
    513c:	d61f0220 	br	x17

0000000000005140 <sched_get_priority_max@plt>:
    5140:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5144:	f9407a11 	ldr	x17, [x16,#240]
    5148:	9103c210 	add	x16, x16, #0xf0
    514c:	d61f0220 	br	x17

0000000000005150 <__statfs@plt>:
    5150:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5154:	f9407e11 	ldr	x17, [x16,#248]
    5158:	9103e210 	add	x16, x16, #0xf8
    515c:	d61f0220 	br	x17

0000000000005160 <tdelete@plt>:
    5160:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5164:	f9408211 	ldr	x17, [x16,#256]
    5168:	91040210 	add	x16, x16, #0x100
    516c:	d61f0220 	br	x17

0000000000005170 <abort@plt>:
    5170:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5174:	f9408611 	ldr	x17, [x16,#264]
    5178:	91042210 	add	x16, x16, #0x108
    517c:	d61f0220 	br	x17

0000000000005180 <__sched_get_priority_min@plt>:
    5180:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5184:	f9408a11 	ldr	x17, [x16,#272]
    5188:	91044210 	add	x16, x16, #0x110
    518c:	d61f0220 	br	x17

0000000000005190 <__libc_current_sigrtmax_private@plt>:
    5190:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5194:	f9408e11 	ldr	x17, [x16,#280]
    5198:	91046210 	add	x16, x16, #0x118
    519c:	d61f0220 	br	x17

00000000000051a0 <sched_setparam@plt>:
    51a0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    51a4:	f9409211 	ldr	x17, [x16,#288]
    51a8:	91048210 	add	x16, x16, #0x120
    51ac:	d61f0220 	br	x17

00000000000051b0 <__libc_dlopen_mode@plt>:
    51b0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    51b4:	f9409611 	ldr	x17, [x16,#296]
    51b8:	9104a210 	add	x16, x16, #0x128
    51bc:	d61f0220 	br	x17

00000000000051c0 <strcmp@plt>:
    51c0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    51c4:	f9409a11 	ldr	x17, [x16,#304]
    51c8:	9104c210 	add	x16, x16, #0x130
    51cc:	d61f0220 	br	x17

00000000000051d0 <mmap@plt>:
    51d0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    51d4:	f9409e11 	ldr	x17, [x16,#312]
    51d8:	9104e210 	add	x16, x16, #0x138
    51dc:	d61f0220 	br	x17

00000000000051e0 <_dl_make_stack_executable@plt>:
    51e0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    51e4:	f940a211 	ldr	x17, [x16,#320]
    51e8:	91050210 	add	x16, x16, #0x140
    51ec:	d61f0220 	br	x17

00000000000051f0 <sched_yield@plt>:
    51f0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    51f4:	f940a611 	ldr	x17, [x16,#328]
    51f8:	91052210 	add	x16, x16, #0x148
    51fc:	d61f0220 	br	x17

0000000000005200 <__libc_pthread_init@plt>:
    5200:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5204:	f940aa11 	ldr	x17, [x16,#336]
    5208:	91054210 	add	x16, x16, #0x150
    520c:	d61f0220 	br	x17

0000000000005210 <__ctype_init@plt>:
    5210:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5214:	f940ae11 	ldr	x17, [x16,#344]
    5218:	91056210 	add	x16, x16, #0x158
    521c:	d61f0220 	br	x17

0000000000005220 <free@plt>:
    5220:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5224:	f940b211 	ldr	x17, [x16,#352]
    5228:	91058210 	add	x16, x16, #0x160
    522c:	d61f0220 	br	x17

0000000000005230 <sched_get_priority_min@plt>:
    5230:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5234:	f940b611 	ldr	x17, [x16,#360]
    5238:	9105a210 	add	x16, x16, #0x168
    523c:	d61f0220 	br	x17

0000000000005240 <__setmntent@plt>:
    5240:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5244:	f940ba11 	ldr	x17, [x16,#368]
    5248:	9105c210 	add	x16, x16, #0x170
    524c:	d61f0220 	br	x17

0000000000005250 <__getmntent_r@plt>:
    5250:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5254:	f940be11 	ldr	x17, [x16,#376]
    5258:	9105e210 	add	x16, x16, #0x178
    525c:	d61f0220 	br	x17

0000000000005260 <mempcpy@plt>:
    5260:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5264:	f940c211 	ldr	x17, [x16,#384]
    5268:	91060210 	add	x16, x16, #0x180
    526c:	d61f0220 	br	x17

0000000000005270 <__fxstat64@plt>:
    5270:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5274:	f940c611 	ldr	x17, [x16,#392]
    5278:	91062210 	add	x16, x16, #0x188
    527c:	d61f0220 	br	x17

0000000000005280 <__libc_dlsym@plt>:
    5280:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5284:	f940ca11 	ldr	x17, [x16,#400]
    5288:	91064210 	add	x16, x16, #0x190
    528c:	d61f0220 	br	x17

0000000000005290 <__sched_setscheduler@plt>:
    5290:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5294:	f940ce11 	ldr	x17, [x16,#408]
    5298:	91066210 	add	x16, x16, #0x198
    529c:	d61f0220 	br	x17

00000000000052a0 <__libc_current_sigrtmin_private@plt>:
    52a0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    52a4:	f940d211 	ldr	x17, [x16,#416]
    52a8:	91068210 	add	x16, x16, #0x1a0
    52ac:	d61f0220 	br	x17

00000000000052b0 <munmap@plt>:
    52b0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    52b4:	f940d611 	ldr	x17, [x16,#424]
    52b8:	9106a210 	add	x16, x16, #0x1a8
    52bc:	d61f0220 	br	x17

00000000000052c0 <getrlimit@plt>:
    52c0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    52c4:	f940da11 	ldr	x17, [x16,#432]
    52c8:	9106c210 	add	x16, x16, #0x1b0
    52cc:	d61f0220 	br	x17

00000000000052d0 <__madvise@plt>:
    52d0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    52d4:	f940de11 	ldr	x17, [x16,#440]
    52d8:	9106e210 	add	x16, x16, #0x1b8
    52dc:	d61f0220 	br	x17

00000000000052e0 <__libc_dl_error_tsd@plt>:
    52e0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    52e4:	f940e211 	ldr	x17, [x16,#448]
    52e8:	91070210 	add	x16, x16, #0x1c0
    52ec:	d61f0220 	br	x17

00000000000052f0 <__mktemp@plt>:
    52f0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    52f4:	f940e611 	ldr	x17, [x16,#456]
    52f8:	91072210 	add	x16, x16, #0x1c8
    52fc:	d61f0220 	br	x17

0000000000005300 <__clone@plt>:
    5300:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5304:	f940ea11 	ldr	x17, [x16,#464]
    5308:	91074210 	add	x16, x16, #0x1d0
    530c:	d61f0220 	br	x17

0000000000005310 <_dl_allocate_tls@plt>:
    5310:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5314:	f940ee11 	ldr	x17, [x16,#472]
    5318:	91076210 	add	x16, x16, #0x1d8
    531c:	d61f0220 	br	x17

0000000000005320 <link@plt>:
    5320:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5324:	f940f211 	ldr	x17, [x16,#480]
    5328:	91078210 	add	x16, x16, #0x1e0
    532c:	d61f0220 	br	x17

0000000000005330 <__sched_getscheduler@plt>:
    5330:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5334:	f940f611 	ldr	x17, [x16,#488]
    5338:	9107a210 	add	x16, x16, #0x1e8
    533c:	d61f0220 	br	x17

0000000000005340 <_dl_get_tls_static_info@plt>:
    5340:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5344:	f940fa11 	ldr	x17, [x16,#496]
    5348:	9107c210 	add	x16, x16, #0x1f0
    534c:	d61f0220 	br	x17

0000000000005350 <__sched_get_priority_max@plt>:
    5350:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5354:	f940fe11 	ldr	x17, [x16,#504]
    5358:	9107e210 	add	x16, x16, #0x1f8
    535c:	d61f0220 	br	x17

0000000000005360 <__libc_alloca_cutoff@plt>:
    5360:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5364:	f9410211 	ldr	x17, [x16,#512]
    5368:	91080210 	add	x16, x16, #0x200
    536c:	d61f0220 	br	x17

0000000000005370 <_dl_allocate_tls_init@plt>:
    5370:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5374:	f9410611 	ldr	x17, [x16,#520]
    5378:	91082210 	add	x16, x16, #0x208
    537c:	d61f0220 	br	x17

0000000000005380 <prctl@plt>:
    5380:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5384:	f9410a11 	ldr	x17, [x16,#528]
    5388:	91084210 	add	x16, x16, #0x210
    538c:	d61f0220 	br	x17

0000000000005390 <__libc_allocate_rtsig_private@plt>:
    5390:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5394:	f9410e11 	ldr	x17, [x16,#536]
    5398:	91086210 	add	x16, x16, #0x218
    539c:	d61f0220 	br	x17

00000000000053a0 <__libc_longjmp@plt>:
    53a0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    53a4:	f9411211 	ldr	x17, [x16,#544]
    53a8:	91088210 	add	x16, x16, #0x220
    53ac:	d61f0220 	br	x17

00000000000053b0 <mprotect@plt>:
    53b0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    53b4:	f9411611 	ldr	x17, [x16,#552]
    53b8:	9108a210 	add	x16, x16, #0x228
    53bc:	d61f0220 	br	x17

00000000000053c0 <__getdelim@plt>:
    53c0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    53c4:	f9411a11 	ldr	x17, [x16,#560]
    53c8:	9108c210 	add	x16, x16, #0x230
    53cc:	d61f0220 	br	x17

00000000000053d0 <unlink@plt>:
    53d0:	f0000150 	adrp	x16, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    53d4:	f9411e11 	ldr	x17, [x16,#568]
    53d8:	9108e210 	add	x16, x16, #0x238
    53dc:	d61f0220 	br	x17

00000000000053e0 <h_errno@plt>:
    53e0:	a9bf0fe2 	stp	x2, x3, [sp,#-16]!
    53e4:	d0000142 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    53e8:	d0000143 	adrp	x3, 2f000 <__FRAME_END__+0x18e30>
    53ec:	f947f042 	ldr	x2, [x2,#4064]
    53f0:	913fa063 	add	x3, x3, #0xfe8
    53f4:	d61f0040 	br	x2
    53f8:	d503201f 	nop
    53fc:	d503201f 	nop

Disassembly of section .text:

0000000000005400 <deregister_tm_clones>:
    5400:	f0000141 	adrp	x1, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5404:	f0000140 	adrp	x0, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5408:	910a0021 	add	x1, x1, #0x280
    540c:	910a0000 	add	x0, x0, #0x280
    5410:	91001c21 	add	x1, x1, #0x7
    5414:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    5418:	cb000021 	sub	x1, x1, x0
    541c:	f100383f 	cmp	x1, #0xe
    5420:	910003fd 	mov	x29, sp
    5424:	540000a9 	b.ls	5438 <deregister_tm_clones+0x38>
    5428:	d0000141 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    542c:	f947c021 	ldr	x1, [x1,#3968]
    5430:	b4000041 	cbz	x1, 5438 <deregister_tm_clones+0x38>
    5434:	d63f0020 	blr	x1
    5438:	a8c17bfd 	ldp	x29, x30, [sp],#16
    543c:	d65f03c0 	ret

0000000000005440 <register_tm_clones>:
    5440:	f0000140 	adrp	x0, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5444:	f0000141 	adrp	x1, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5448:	910a0000 	add	x0, x0, #0x280
    544c:	910a0021 	add	x1, x1, #0x280
    5450:	cb000021 	sub	x1, x1, x0
    5454:	9343fc22 	asr	x2, x1, #3
    5458:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    545c:	8b42fc42 	add	x2, x2, x2, lsr #63
    5460:	9341fc41 	asr	x1, x2, #1
    5464:	910003fd 	mov	x29, sp
    5468:	b40000a1 	cbz	x1, 547c <register_tm_clones+0x3c>
    546c:	d0000142 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    5470:	f947e842 	ldr	x2, [x2,#4048]
    5474:	b4000042 	cbz	x2, 547c <register_tm_clones+0x3c>
    5478:	d63f0040 	blr	x2
    547c:	a8c17bfd 	ldp	x29, x30, [sp],#16
    5480:	d65f03c0 	ret

0000000000005484 <__do_global_dtors_aux>:
    5484:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    5488:	910003fd 	mov	x29, sp
    548c:	f9000bf3 	str	x19, [sp,#16]
    5490:	f0000153 	adrp	x19, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5494:	394a0260 	ldrb	w0, [x19,#640]
    5498:	35000140 	cbnz	w0, 54c0 <__do_global_dtors_aux+0x3c>
    549c:	d0000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    54a0:	f947c800 	ldr	x0, [x0,#3984]
    54a4:	b4000080 	cbz	x0, 54b4 <__do_global_dtors_aux+0x30>
    54a8:	d0000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    54ac:	912e0000 	add	x0, x0, #0xb80
    54b0:	97fffed8 	bl	5010 <__cxa_finalize@plt>
    54b4:	97ffffd3 	bl	5400 <deregister_tm_clones>
    54b8:	52800020 	mov	w0, #0x1                   	// #1
    54bc:	390a0260 	strb	w0, [x19,#640]
    54c0:	f9400bf3 	ldr	x19, [sp,#16]
    54c4:	a8c27bfd 	ldp	x29, x30, [sp],#32
    54c8:	d65f03c0 	ret

00000000000054cc <frame_dummy>:
    54cc:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    54d0:	d0000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    54d4:	910003fd 	mov	x29, sp
    54d8:	912de000 	add	x0, x0, #0xb78
    54dc:	f9400001 	ldr	x1, [x0]
    54e0:	b5000061 	cbnz	x1, 54ec <frame_dummy+0x20>
    54e4:	a8c17bfd 	ldp	x29, x30, [sp],#16
    54e8:	17ffffd6 	b	5440 <register_tm_clones>
    54ec:	d0000141 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    54f0:	f947e421 	ldr	x1, [x1,#4040]
    54f4:	b4ffff81 	cbz	x1, 54e4 <frame_dummy+0x18>
    54f8:	d63f0020 	blr	x1
    54fc:	17fffffa 	b	54e4 <frame_dummy+0x18>

0000000000005500 <__nptl_set_robust>:
    5500:	91038000 	add	x0, x0, #0xe0
    5504:	d2800301 	mov	x1, #0x18                  	// #24
    5508:	d2800c68 	mov	x8, #0x63                  	// #99
    550c:	d4000001 	svc	#0x0
    5510:	d65f03c0 	ret

0000000000005514 <sigcancel_handler>:
    5514:	d53bd043 	mrs	x3, tpidr_el0
    5518:	7100801f 	cmp	w0, #0x20
    551c:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    5520:	d11bc062 	sub	x2, x3, #0x6f0
    5524:	910003fd 	mov	x29, sp
    5528:	b940d440 	ldr	w0, [x2,#212]
    552c:	54000060 	b.eq	5538 <sigcancel_handler+0x24>
    5530:	a8c27bfd 	ldp	x29, x30, [sp],#32
    5534:	d65f03c0 	ret
    5538:	93407c00 	sxtw	x0, w0
    553c:	b9401024 	ldr	w4, [x1,#16]
    5540:	ca80fc05 	eor	x5, x0, x0, asr #63
    5544:	cb80fca0 	sub	x0, x5, x0, asr #63
    5548:	6b00009f 	cmp	w4, w0
    554c:	54ffff21 	b.ne	5530 <sigcancel_handler+0x1c>
    5550:	b9400820 	ldr	w0, [x1,#8]
    5554:	3100181f 	cmn	w0, #0x6
    5558:	54fffec1 	b.ne	5530 <sigcancel_handler+0x1c>
    555c:	b9410840 	ldr	w0, [x2,#264]
    5560:	321e0404 	orr	w4, w0, #0xc
    5564:	6b04001f 	cmp	w0, w4
    5568:	54fffe40 	b.eq	5530 <sigcancel_handler+0x1c>
    556c:	3727fe20 	tbnz	w0, #4, 5530 <sigcancel_handler+0x1c>
    5570:	910083a6 	add	x6, x29, #0x20
    5574:	91042045 	add	x5, x2, #0x108
    5578:	2a0003e1 	mov	w1, w0
    557c:	b81fccc0 	str	w0, [x6,#-4]!
    5580:	885ffca7 	ldaxr	w7, [x5]
    5584:	6b0100ff 	cmp	w7, w1
    5588:	54000061 	b.ne	5594 <sigcancel_handler+0x80>
    558c:	88087ca4 	stxr	w8, w4, [x5]
    5590:	35ffff88 	cbnz	w8, 5580 <sigcancel_handler+0x6c>
    5594:	54000040 	b.eq	559c <sigcancel_handler+0x88>
    5598:	b9001fa7 	str	w7, [x29,#28]
    559c:	b9401fa1 	ldr	w1, [x29,#28]
    55a0:	6b01001f 	cmp	w0, w1
    55a4:	54000060 	b.eq	55b0 <sigcancel_handler+0x9c>
    55a8:	2a0103e0 	mov	w0, w1
    55ac:	17ffffed 	b	5560 <sigcancel_handler+0x4c>
    55b0:	92800001 	mov	x1, #0xffffffffffffffff    	// #-1
    55b4:	f9021441 	str	x1, [x2,#1064]
    55b8:	360ffbc0 	tbz	w0, #1, 5530 <sigcancel_handler+0x1c>
    55bc:	d117a063 	sub	x3, x3, #0x5e8
    55c0:	b9400060 	ldr	w0, [x3]
    55c4:	b9001fa0 	str	w0, [x29,#28]
    55c8:	321c0004 	orr	w4, w0, #0x10
    55cc:	885ffca1 	ldaxr	w1, [x5]
    55d0:	6b00003f 	cmp	w1, w0
    55d4:	54000061 	b.ne	55e0 <sigcancel_handler+0xcc>
    55d8:	88077ca4 	stxr	w7, w4, [x5]
    55dc:	35ffff87 	cbnz	w7, 55cc <sigcancel_handler+0xb8>
    55e0:	54000060 	b.eq	55ec <sigcancel_handler+0xd8>
    55e4:	b90000c1 	str	w1, [x6]
    55e8:	17fffff6 	b	55c0 <sigcancel_handler+0xac>
    55ec:	f9408040 	ldr	x0, [x2,#256]
    55f0:	940027c5 	bl	f504 <__pthread_unwind>

00000000000055f4 <sighandler_setxid>:
    55f4:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    55f8:	7100841f 	cmp	w0, #0x21
    55fc:	910003fd 	mov	x29, sp
    5600:	a90153f3 	stp	x19, x20, [sp,#16]
    5604:	d53bd053 	mrs	x19, tpidr_el0
    5608:	d11bc273 	sub	x19, x19, #0x6f0
    560c:	b940d660 	ldr	w0, [x19,#212]
    5610:	54000080 	b.eq	5620 <sighandler_setxid+0x2c>
    5614:	a94153f3 	ldp	x19, x20, [sp,#16]
    5618:	a8c37bfd 	ldp	x29, x30, [sp],#48
    561c:	d65f03c0 	ret
    5620:	93407c00 	sxtw	x0, w0
    5624:	b9401022 	ldr	w2, [x1,#16]
    5628:	ca80fc03 	eor	x3, x0, x0, asr #63
    562c:	cb80fc60 	sub	x0, x3, x0, asr #63
    5630:	6b00005f 	cmp	w2, w0
    5634:	54ffff01 	b.ne	5614 <sighandler_setxid+0x20>
    5638:	b9400820 	ldr	w0, [x1,#8]
    563c:	3100181f 	cmn	w0, #0x6
    5640:	54fffea1 	b.ne	5614 <sighandler_setxid+0x20>
    5644:	f0000174 	adrp	x20, 34000 <__GI___pthread_keys+0x3d78>
    5648:	f9418a83 	ldr	x3, [x20,#784]
    564c:	f9400460 	ldr	x0, [x3,#8]
    5650:	f9400861 	ldr	x1, [x3,#16]
    5654:	f9400c62 	ldr	x2, [x3,#24]
    5658:	b9800068 	ldrsw	x8, [x3]
    565c:	d4000001 	svc	#0x0
    5660:	aa0003e1 	mov	x1, x0
    5664:	f9418a80 	ldr	x0, [x20,#784]
    5668:	3140043f 	cmn	w1, #0x1, lsl #12
    566c:	5a8197e1 	csneg	w1, wzr, w1, ls
    5670:	9400037a 	bl	6458 <__nptl_setxid_error>
    5674:	91042261 	add	x1, x19, #0x108
    5678:	9100b3a5 	add	x5, x29, #0x2c
    567c:	b9410a60 	ldr	w0, [x19,#264]
    5680:	b9002fa0 	str	w0, [x29,#44]
    5684:	12197803 	and	w3, w0, #0xffffffbf
    5688:	2a0003e2 	mov	w2, w0
    568c:	885ffc24 	ldaxr	w4, [x1]
    5690:	6b02009f 	cmp	w4, w2
    5694:	54000061 	b.ne	56a0 <sighandler_setxid+0xac>
    5698:	88067c23 	stxr	w6, w3, [x1]
    569c:	35ffff86 	cbnz	w6, 568c <sighandler_setxid+0x98>
    56a0:	54000040 	b.eq	56a8 <sighandler_setxid+0xb4>
    56a4:	b90000a4 	str	w4, [x5]
    56a8:	b9402fa2 	ldr	w2, [x29,#44]
    56ac:	6b02001f 	cmp	w0, w2
    56b0:	54fffe61 	b.ne	567c <sighandler_setxid+0x88>
    56b4:	52800021 	mov	w1, #0x1                   	// #1
    56b8:	91107260 	add	x0, x19, #0x41c
    56bc:	b9041e61 	str	w1, [x19,#1052]
    56c0:	d2800022 	mov	x2, #0x1                   	// #1
    56c4:	d2801021 	mov	x1, #0x81                  	// #129
    56c8:	d2800003 	mov	x3, #0x0                   	// #0
    56cc:	d2800c48 	mov	x8, #0x62                  	// #98
    56d0:	d4000001 	svc	#0x0
    56d4:	f9418a80 	ldr	x0, [x20,#784]
    56d8:	91008000 	add	x0, x0, #0x20
    56dc:	885ffc04 	ldaxr	w4, [x0]
    56e0:	51000485 	sub	w5, w4, #0x1
    56e4:	88067c05 	stxr	w6, w5, [x0]
    56e8:	35ffffa6 	cbnz	w6, 56dc <sighandler_setxid+0xe8>
    56ec:	b9002fa4 	str	w4, [x29,#44]
    56f0:	b9402fa0 	ldr	w0, [x29,#44]
    56f4:	7100041f 	cmp	w0, #0x1
    56f8:	54fff8e1 	b.ne	5614 <sighandler_setxid+0x20>
    56fc:	f0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    5700:	f9418800 	ldr	x0, [x0,#784]
    5704:	91008000 	add	x0, x0, #0x20
    5708:	d4000001 	svc	#0x0
    570c:	17ffffc2 	b	5614 <sighandler_setxid+0x20>

0000000000005710 <__pthread_initialize_minimal>:
    5710:	a9b17bfd 	stp	x29, x30, [sp,#-240]!
    5714:	d53bd044 	mrs	x4, tpidr_el0
    5718:	d2800c08 	mov	x8, #0x60                  	// #96
    571c:	910003fd 	mov	x29, sp
    5720:	d11bc083 	sub	x3, x4, #0x6f0
    5724:	91034060 	add	x0, x3, #0xd0
    5728:	a90153f3 	stp	x19, x20, [sp,#16]
    572c:	f90013f5 	str	x21, [sp,#32]
    5730:	d4000001 	svc	#0x0
    5734:	91038062 	add	x2, x3, #0xe0
    5738:	91044061 	add	x1, x3, #0x110
    573c:	b900d060 	str	w0, [x3,#208]
    5740:	d2800c68 	mov	x8, #0x63                  	// #99
    5744:	b900d460 	str	w0, [x3,#212]
    5748:	52800020 	mov	w0, #0x1                   	// #1
    574c:	f9018861 	str	x1, [x3,#784]
    5750:	d2800301 	mov	x1, #0x18                  	// #24
    5754:	39104860 	strb	w0, [x3,#1042]
    5758:	aa0203e0 	mov	x0, x2
    575c:	f9006c62 	str	x2, [x3,#216]
    5760:	f9007062 	str	x2, [x3,#224]
    5764:	928003e2 	mov	x2, #0xffffffffffffffe0    	// #-32
    5768:	f9007462 	str	x2, [x3,#232]
    576c:	d4000001 	svc	#0x0
    5770:	d0000147 	adrp	x7, 2f000 <__FRAME_END__+0x18e30>
    5774:	f0000166 	adrp	x6, 34000 <__GI___pthread_keys+0x3d78>
    5778:	910aa0c5 	add	x5, x6, #0x2a8
    577c:	d118c084 	sub	x4, x4, #0x630
    5780:	910183b3 	add	x19, x29, #0x60
    5784:	91030068 	add	x8, x3, #0xc0
    5788:	f947e0e7 	ldr	x7, [x7,#4032]
    578c:	910163b5 	add	x21, x29, #0x58
    5790:	52800094 	mov	w20, #0x4                   	// #4
    5794:	aa1503e1 	mov	x1, x21
    5798:	d2800002 	mov	x2, #0x0                   	// #0
    579c:	52800400 	mov	w0, #0x20                  	// #32
    57a0:	f94000e7 	ldr	x7, [x7]
    57a4:	f9024c67 	str	x7, [x3,#1176]
    57a8:	f90154c5 	str	x5, [x6,#680]
    57ac:	f90004a5 	str	x5, [x5,#8]
    57b0:	f9000085 	str	x5, [x4]
    57b4:	f9000485 	str	x5, [x4,#8]
    57b8:	f94154c4 	ldr	x4, [x6,#680]
    57bc:	f9000488 	str	x8, [x4,#8]
    57c0:	f0000144 	adrp	x4, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    57c4:	d5033bbf 	dmb	ish
    57c8:	f90154c8 	str	x8, [x6,#680]
    57cc:	394a0484 	ldrb	w4, [x4,#641]
    57d0:	a9017e7f 	stp	xzr, xzr, [x19,#16]
    57d4:	a9027e7f 	stp	xzr, xzr, [x19,#32]
    57d8:	b900e3b4 	str	w20, [x29,#224]
    57dc:	72a20014 	movk	w20, #0x1000, lsl #16
    57e0:	39104464 	strb	w4, [x3,#1041]
    57e4:	90000003 	adrp	x3, 5000 <__getpagesize@plt>
    57e8:	91145063 	add	x3, x3, #0x514
    57ec:	f9002fa3 	str	x3, [x29,#88]
    57f0:	a9007e7f 	stp	xzr, xzr, [x19]
    57f4:	a9037e7f 	stp	xzr, xzr, [x19,#48]
    57f8:	a9047e7f 	stp	xzr, xzr, [x19,#64]
    57fc:	a9057e7f 	stp	xzr, xzr, [x19,#80]
    5800:	a9067e7f 	stp	xzr, xzr, [x19,#96]
    5804:	a9077e7f 	stp	xzr, xzr, [x19,#112]
    5808:	94002eb1 	bl	112cc <__libc_sigaction>
    580c:	b900e3b4 	str	w20, [x29,#224]
    5810:	90000003 	adrp	x3, 5000 <__getpagesize@plt>
    5814:	aa1503e1 	mov	x1, x21
    5818:	9117d063 	add	x3, x3, #0x5f4
    581c:	d2800002 	mov	x2, #0x0                   	// #0
    5820:	52800420 	mov	w0, #0x21                  	// #33
    5824:	f9002fa3 	str	x3, [x29,#88]
    5828:	94002ea9 	bl	112cc <__libc_sigaction>
    582c:	f94033a3 	ldr	x3, [x29,#96]
    5830:	d2800020 	mov	x0, #0x1                   	// #1
    5834:	aa1303e1 	mov	x1, x19
    5838:	d2800002 	mov	x2, #0x0                   	// #0
    583c:	b2610464 	orr	x4, x3, #0x180000000
    5840:	d28010e8 	mov	x8, #0x87                  	// #135
    5844:	d2800103 	mov	x3, #0x8                   	// #8
    5848:	f90033a4 	str	x4, [x29,#96]
    584c:	d4000001 	svc	#0x0
    5850:	f0000173 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
    5854:	910103a1 	add	x1, x29, #0x40
    5858:	910c8260 	add	x0, x19, #0x320
    585c:	97fffeb9 	bl	5340 <_dl_get_tls_static_info@plt>
    5860:	f94023a4 	ldr	x4, [x29,#64]
    5864:	f1003c9f 	cmp	x4, #0xf
    5868:	54000e68 	b.hi	5a34 <__pthread_initialize_minimal+0x324>
    586c:	f0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    5870:	d2800202 	mov	x2, #0x10                  	// #16
    5874:	d28001e1 	mov	x1, #0xf                   	// #15
    5878:	f90023a2 	str	x2, [x29,#64]
    587c:	aa0203e4 	mov	x4, x2
    5880:	f9018c01 	str	x1, [x0,#792]
    5884:	f9419262 	ldr	x2, [x19,#800]
    5888:	52800060 	mov	w0, #0x3                   	// #3
    588c:	910123a1 	add	x1, x29, #0x48
    5890:	d1000443 	sub	x3, x2, #0x1
    5894:	8b040063 	add	x3, x3, x4
    5898:	9ac40863 	udiv	x3, x3, x4
    589c:	9b047c62 	mul	x2, x3, x4
    58a0:	f9019262 	str	x2, [x19,#800]
    58a4:	97fffdc3 	bl	4fb0 <__getrlimit@plt>
    58a8:	34000b60 	cbz	w0, 5a14 <__pthread_initialize_minimal+0x304>
    58ac:	d2a00403 	mov	x3, #0x200000              	// #2097152
    58b0:	d0000141 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    58b4:	f9419262 	ldr	x2, [x19,#800]
    58b8:	f0000173 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
    58bc:	b9003fbf 	str	wzr, [x29,#60]
    58c0:	910a2260 	add	x0, x19, #0x288
    58c4:	f947d421 	ldr	x1, [x1,#4008]
    58c8:	f9400c34 	ldr	x20, [x1,#24]
    58cc:	8b020282 	add	x2, x20, x2
    58d0:	d1000684 	sub	x4, x20, #0x1
    58d4:	91200041 	add	x1, x2, #0x800
    58d8:	cb1403e2 	neg	x2, x20
    58dc:	eb03003f 	cmp	x1, x3
    58e0:	9a832021 	csel	x1, x1, x3, cs
    58e4:	8b040021 	add	x1, x1, x4
    58e8:	8a020021 	and	x1, x1, x2
    58ec:	f90027a1 	str	x1, [x29,#72]
    58f0:	52800021 	mov	w1, #0x1                   	// #1
    58f4:	885ffc02 	ldaxr	w2, [x0]
    58f8:	6b1f005f 	cmp	w2, wzr
    58fc:	54000061 	b.ne	5908 <__pthread_initialize_minimal+0x1f8>
    5900:	88037c01 	stxr	w3, w1, [x0]
    5904:	35ffff83 	cbnz	w3, 58f4 <__pthread_initialize_minimal+0x1e4>
    5908:	54000801 	b.ne	5a08 <__pthread_initialize_minimal+0x2f8>
    590c:	f0000161 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    5910:	f94027a2 	ldr	x2, [x29,#72]
    5914:	910cc021 	add	x1, x1, #0x330
    5918:	910a2260 	add	x0, x19, #0x288
    591c:	f9001022 	str	x2, [x1,#32]
    5920:	52800002 	mov	w2, #0x0                   	// #0
    5924:	f9000834 	str	x20, [x1,#16]
    5928:	885f7c01 	ldxr	w1, [x0]
    592c:	8803fc02 	stlxr	w3, w2, [x0]
    5930:	35ffffc3 	cbnz	w3, 5928 <__pthread_initialize_minimal+0x218>
    5934:	7100043f 	cmp	w1, #0x1
    5938:	5400086c 	b.gt	5a44 <__pthread_initialize_minimal+0x334>
    593c:	d0000154 	adrp	x20, 2f000 <__FRAME_END__+0x18e30>
    5940:	97fffe68 	bl	52e0 <__libc_dl_error_tsd@plt>
    5944:	aa0003f3 	mov	x19, x0
    5948:	f947ee95 	ldr	x21, [x20,#4056]
    594c:	f9450ea0 	ldr	x0, [x21,#2584]
    5950:	d63f0000 	blr	x0
    5954:	f9400000 	ldr	x0, [x0]
    5958:	f9000260 	str	x0, [x19]
    595c:	d0000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    5960:	b9498eb3 	ldr	w19, [x21,#2444]
    5964:	f947dc00 	ldr	x0, [x0,#4024]
    5968:	f9050ea0 	str	x0, [x21,#2584]
    596c:	90000020 	adrp	x0, 9000 <__pthread_mutex_lock_full+0x1a0>
    5970:	910f6000 	add	x0, x0, #0x3d8
    5974:	f907c6a0 	str	x0, [x21,#3976]
    5978:	b0000020 	adrp	x0, a000 <pthread_mutex_timedlock+0x3b4>
    597c:	b9098ebf 	str	wzr, [x21,#2444]
    5980:	91296000 	add	x0, x0, #0xa58
    5984:	f907caa0 	str	x0, [x21,#3984]
    5988:	340000d3 	cbz	w19, 59a0 <__pthread_initialize_minimal+0x290>
    598c:	f947ee80 	ldr	x0, [x20,#4056]
    5990:	91262000 	add	x0, x0, #0x988
    5994:	94000e91 	bl	93d8 <__pthread_mutex_lock>
    5998:	71000673 	subs	w19, w19, #0x1
    599c:	54ffff81 	b.ne	598c <__pthread_initialize_minimal+0x27c>
    59a0:	f947ee94 	ldr	x20, [x20,#4056]
    59a4:	b0000003 	adrp	x3, 6000 <do_clone.constprop.4+0x1a4>
    59a8:	91014063 	add	x3, x3, #0x50
    59ac:	f0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    59b0:	b0000001 	adrp	x1, 6000 <do_clone.constprop.4+0x1a4>
    59b4:	d0000142 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    59b8:	f907ce83 	str	x3, [x20,#3992]
    59bc:	b0000003 	adrp	x3, 6000 <do_clone.constprop.4+0x1a4>
    59c0:	91224063 	add	x3, x3, #0x890
    59c4:	f907f683 	str	x3, [x20,#4072]
    59c8:	b0000003 	adrp	x3, 6000 <do_clone.constprop.4+0x1a4>
    59cc:	9106e021 	add	x1, x1, #0x1b8
    59d0:	91285063 	add	x3, x3, #0xa14
    59d4:	912e2042 	add	x2, x2, #0xb88
    59d8:	910e2000 	add	x0, x0, #0x388
    59dc:	f907fa83 	str	x3, [x20,#4080]
    59e0:	97fffe08 	bl	5200 <__libc_pthread_init@plt>
    59e4:	f94013f5 	ldr	x21, [sp,#32]
    59e8:	f0000161 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    59ec:	a94153f3 	ldp	x19, x20, [sp,#16]
    59f0:	a8cf7bfd 	ldp	x29, x30, [sp],#240
    59f4:	f901b820 	str	x0, [x1,#880]
    59f8:	f0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    59fc:	52800021 	mov	w1, #0x1                   	// #1
    5a00:	b9032c01 	str	w1, [x0,#812]
    5a04:	d65f03c0 	ret
    5a08:	b9003fa2 	str	w2, [x29,#60]
    5a0c:	94002757 	bl	f768 <__lll_lock_wait_private>
    5a10:	17ffffbf 	b	590c <__pthread_initialize_minimal+0x1fc>
    5a14:	f94027a3 	ldr	x3, [x29,#72]
    5a18:	b100047f 	cmn	x3, #0x1
    5a1c:	54fff480 	b.eq	58ac <__pthread_initialize_minimal+0x19c>
    5a20:	b24043e0 	mov	x0, #0x1ffff               	// #131071
    5a24:	eb00007f 	cmp	x3, x0
    5a28:	d2a00040 	mov	x0, #0x20000               	// #131072
    5a2c:	9a808063 	csel	x3, x3, x0, hi
    5a30:	17ffffa0 	b	58b0 <__pthread_initialize_minimal+0x1a0>
    5a34:	f0000161 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    5a38:	d1000480 	sub	x0, x4, #0x1
    5a3c:	f9018c20 	str	x0, [x1,#792]
    5a40:	17ffff91 	b	5884 <__pthread_initialize_minimal+0x174>
    5a44:	d2801021 	mov	x1, #0x81                  	// #129
    5a48:	d2800022 	mov	x2, #0x1                   	// #1
    5a4c:	d2800003 	mov	x3, #0x0                   	// #0
    5a50:	d2800c48 	mov	x8, #0x62                  	// #98
    5a54:	d4000001 	svc	#0x0
    5a58:	17ffffb9 	b	593c <__pthread_initialize_minimal+0x22c>

0000000000005a5c <__pthread_get_minstack>:
    5a5c:	d0000142 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    5a60:	f9400803 	ldr	x3, [x0,#16]
    5a64:	f0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    5a68:	f947d442 	ldr	x2, [x2,#4008]
    5a6c:	f9419001 	ldr	x1, [x0,#800]
    5a70:	f9400c40 	ldr	x0, [x2,#24]
    5a74:	8b010000 	add	x0, x0, x1
    5a78:	91408000 	add	x0, x0, #0x20, lsl #12
    5a7c:	8b030000 	add	x0, x0, x3
    5a80:	d65f03c0 	ret

0000000000005a84 <__GI___nptl_create_event>:
    5a84:	d65f03c0 	ret

0000000000005a88 <__GI___nptl_death_event>:
    5a88:	d65f03c0 	ret

0000000000005a8c <__nptl_main>:
    5a8c:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    5a90:	b0000061 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
    5a94:	d2800020 	mov	x0, #0x1                   	// #1
    5a98:	910003fd 	mov	x29, sp
    5a9c:	9121c021 	add	x1, x1, #0x870
    5aa0:	d28023a2 	mov	x2, #0x11d                 	// #285
    5aa4:	d2800808 	mov	x8, #0x40                  	// #64
    5aa8:	d4000001 	svc	#0x0
    5aac:	52800000 	mov	w0, #0x0                   	// #0
    5ab0:	97fffd30 	bl	4f70 <_exit@plt>

0000000000005ab4 <setxid_mark_thread.isra.0>:
    5ab4:	aa0003e4 	mov	x4, x0
    5ab8:	b9441c00 	ldr	w0, [x0,#1052]
    5abc:	d10043ff 	sub	sp, sp, #0x10
    5ac0:	3100041f 	cmn	w0, #0x1
    5ac4:	54000420 	b.eq	5b48 <setxid_mark_thread.isra.0+0x94>
    5ac8:	b9410881 	ldr	w1, [x4,#264]
    5acc:	91042082 	add	x2, x4, #0x108
    5ad0:	b9041c9f 	str	wzr, [x4,#1052]
    5ad4:	321a0023 	orr	w3, w1, #0x40
    5ad8:	2a0103e0 	mov	w0, w1
    5adc:	37200201 	tbnz	w1, #4, 5b1c <setxid_mark_thread.isra.0+0x68>
    5ae0:	b9000fe1 	str	w1, [sp,#12]
    5ae4:	885ffc41 	ldaxr	w1, [x2]
    5ae8:	6b00003f 	cmp	w1, w0
    5aec:	54000061 	b.ne	5af8 <setxid_mark_thread.isra.0+0x44>
    5af0:	88057c43 	stxr	w5, w3, [x2]
    5af4:	35ffff85 	cbnz	w5, 5ae4 <setxid_mark_thread.isra.0+0x30>
    5af8:	54000061 	b.ne	5b04 <setxid_mark_thread.isra.0+0x50>
    5afc:	910043ff 	add	sp, sp, #0x10
    5b00:	d65f03c0 	ret
    5b04:	b9000fe1 	str	w1, [sp,#12]
    5b08:	91042082 	add	x2, x4, #0x108
    5b0c:	b9410881 	ldr	w1, [x4,#264]
    5b10:	321a0023 	orr	w3, w1, #0x40
    5b14:	2a0103e0 	mov	w0, w1
    5b18:	3627fe41 	tbz	w1, #4, 5ae0 <setxid_mark_thread.isra.0+0x2c>
    5b1c:	3737ff01 	tbnz	w1, #6, 5afc <setxid_mark_thread.isra.0+0x48>
    5b20:	52800021 	mov	w1, #0x1                   	// #1
    5b24:	91107080 	add	x0, x4, #0x41c
    5b28:	b9041c81 	str	w1, [x4,#1052]
    5b2c:	d2800022 	mov	x2, #0x1                   	// #1
    5b30:	d2801021 	mov	x1, #0x81                  	// #129
    5b34:	d2800003 	mov	x3, #0x0                   	// #0
    5b38:	d2800c48 	mov	x8, #0x62                  	// #98
    5b3c:	d4000001 	svc	#0x0
    5b40:	910043ff 	add	sp, sp, #0x10
    5b44:	d65f03c0 	ret
    5b48:	b9000fe0 	str	w0, [sp,#12]
    5b4c:	91107085 	add	x5, x4, #0x41c
    5b50:	12800020 	mov	w0, #0xfffffffe            	// #-2
    5b54:	12800002 	mov	w2, #0xffffffff            	// #-1
    5b58:	885ffca1 	ldaxr	w1, [x5]
    5b5c:	6b02003f 	cmp	w1, w2
    5b60:	54000061 	b.ne	5b6c <setxid_mark_thread.isra.0+0xb8>
    5b64:	88037ca0 	stxr	w3, w0, [x5]
    5b68:	35ffff83 	cbnz	w3, 5b58 <setxid_mark_thread.isra.0+0xa4>
    5b6c:	54000161 	b.ne	5b98 <setxid_mark_thread.isra.0+0xe4>
    5b70:	aa0503e0 	mov	x0, x5
    5b74:	d2801001 	mov	x1, #0x80                  	// #128
    5b78:	92800022 	mov	x2, #0xfffffffffffffffe    	// #-2
    5b7c:	d2800003 	mov	x3, #0x0                   	// #0
    5b80:	d2800c48 	mov	x8, #0x62                  	// #98
    5b84:	d4000001 	svc	#0x0
    5b88:	b9441c80 	ldr	w0, [x4,#1052]
    5b8c:	3100081f 	cmn	w0, #0x2
    5b90:	54ffff00 	b.eq	5b70 <setxid_mark_thread.isra.0+0xbc>
    5b94:	17ffffcd 	b	5ac8 <setxid_mark_thread.isra.0+0x14>
    5b98:	b9000fe1 	str	w1, [sp,#12]
    5b9c:	17ffffcb 	b	5ac8 <setxid_mark_thread.isra.0+0x14>

0000000000005ba0 <__free_stacks>:
    5ba0:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    5ba4:	910003fd 	mov	x29, sp
    5ba8:	a9025bf5 	stp	x21, x22, [sp,#32]
    5bac:	f0000155 	adrp	x21, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5bb0:	910962b5 	add	x21, x21, #0x258
    5bb4:	a90363f7 	stp	x23, x24, [sp,#48]
    5bb8:	a90153f3 	stp	x19, x20, [sp,#16]
    5bbc:	f94006a2 	ldr	x2, [x21,#8]
    5bc0:	aa0003f8 	mov	x24, x0
    5bc4:	f0000177 	adrp	x23, 34000 <__GI___pthread_keys+0x3d78>
    5bc8:	eb15005f 	cmp	x2, x21
    5bcc:	f9400453 	ldr	x19, [x2,#8]
    5bd0:	540000e1 	b.ne	5bec <__free_stacks+0x4c>
    5bd4:	14000022 	b	5c5c <__free_stacks+0xbc>
    5bd8:	eb15027f 	cmp	x19, x21
    5bdc:	f9400661 	ldr	x1, [x19,#8]
    5be0:	aa1303e2 	mov	x2, x19
    5be4:	540003c0 	b.eq	5c5c <__free_stacks+0xbc>
    5be8:	aa0103f3 	mov	x19, x1
    5bec:	d1030054 	sub	x20, x2, #0xc0
    5bf0:	b940d281 	ldr	w1, [x20,#208]
    5bf4:	6b1f003f 	cmp	w1, wzr
    5bf8:	54ffff0c 	b.gt	5bd8 <__free_stacks+0x38>
    5bfc:	f9014ae2 	str	x2, [x23,#656]
    5c00:	9118c040 	add	x0, x2, #0x630
    5c04:	910a42f6 	add	x22, x23, #0x290
    5c08:	52800001 	mov	w1, #0x0                   	// #0
    5c0c:	d5033bbf 	dmb	ish
    5c10:	f9400043 	ldr	x3, [x2]
    5c14:	f9400444 	ldr	x4, [x2,#8]
    5c18:	f9000464 	str	x4, [x3,#8]
    5c1c:	f9400442 	ldr	x2, [x2,#8]
    5c20:	f9000043 	str	x3, [x2]
    5c24:	d5033bbf 	dmb	ish
    5c28:	f94006c2 	ldr	x2, [x22,#8]
    5c2c:	f9424e83 	ldr	x3, [x20,#1176]
    5c30:	f9014aff 	str	xzr, [x23,#656]
    5c34:	cb030042 	sub	x2, x2, x3
    5c38:	f90006c2 	str	x2, [x22,#8]
    5c3c:	97fffd29 	bl	50e0 <_dl_deallocate_tls@plt>
    5c40:	f9424a80 	ldr	x0, [x20,#1168]
    5c44:	f9424e81 	ldr	x1, [x20,#1176]
    5c48:	97fffd9a 	bl	52b0 <munmap@plt>
    5c4c:	35000120 	cbnz	w0, 5c70 <__free_stacks+0xd0>
    5c50:	f94006c1 	ldr	x1, [x22,#8]
    5c54:	eb18003f 	cmp	x1, x24
    5c58:	54fffc08 	b.hi	5bd8 <__free_stacks+0x38>
    5c5c:	a94153f3 	ldp	x19, x20, [sp,#16]
    5c60:	a9425bf5 	ldp	x21, x22, [sp,#32]
    5c64:	a94363f7 	ldp	x23, x24, [sp,#48]
    5c68:	a8c47bfd 	ldp	x29, x30, [sp],#64
    5c6c:	d65f03c0 	ret
    5c70:	97fffd40 	bl	5170 <abort@plt>

0000000000005c74 <__deallocate_stack>:
    5c74:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    5c78:	910003fd 	mov	x29, sp
    5c7c:	a90153f3 	stp	x19, x20, [sp,#16]
    5c80:	f0000173 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
    5c84:	aa0003f4 	mov	x20, x0
    5c88:	910a4261 	add	x1, x19, #0x290
    5c8c:	b9004fbf 	str	wzr, [x29,#76]
    5c90:	91004020 	add	x0, x1, #0x10
    5c94:	52800021 	mov	w1, #0x1                   	// #1
    5c98:	a9025bf5 	stp	x21, x22, [sp,#32]
    5c9c:	a90363f7 	stp	x23, x24, [sp,#48]
    5ca0:	885ffc02 	ldaxr	w2, [x0]
    5ca4:	6b1f005f 	cmp	w2, wzr
    5ca8:	54000061 	b.ne	5cb4 <__deallocate_stack+0x40>
    5cac:	88037c01 	stxr	w3, w1, [x0]
    5cb0:	35ffff83 	cbnz	w3, 5ca0 <__deallocate_stack+0x2c>
    5cb4:	54000601 	b.ne	5d74 <__deallocate_stack+0x100>
    5cb8:	91030280 	add	x0, x20, #0xc0
    5cbc:	f9014a60 	str	x0, [x19,#656]
    5cc0:	f0000163 	adrp	x3, 34000 <__GI___pthread_keys+0x3d78>
    5cc4:	910a4262 	add	x2, x19, #0x290
    5cc8:	d5033bbf 	dmb	ish
    5ccc:	f9406281 	ldr	x1, [x20,#192]
    5cd0:	f9406684 	ldr	x4, [x20,#200]
    5cd4:	f9000424 	str	x4, [x1,#8]
    5cd8:	f9406684 	ldr	x4, [x20,#200]
    5cdc:	f9000081 	str	x1, [x4]
    5ce0:	d5033bbf 	dmb	ish
    5ce4:	39504a81 	ldrb	w1, [x20,#1042]
    5ce8:	f9014a7f 	str	xzr, [x19,#656]
    5cec:	350004a1 	cbnz	w1, 5d80 <__deallocate_stack+0x10c>
    5cf0:	f0000141 	adrp	x1, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5cf4:	b2400004 	orr	x4, x0, #0x1
    5cf8:	f9014864 	str	x4, [x3,#656]
    5cfc:	91096036 	add	x22, x1, #0x258
    5d00:	d2a05005 	mov	x5, #0x2800000             	// #41943040
    5d04:	d5033bbf 	dmb	ish
    5d08:	f9412c24 	ldr	x4, [x1,#600]
    5d0c:	f9006284 	str	x4, [x20,#192]
    5d10:	f9006696 	str	x22, [x20,#200]
    5d14:	f9000480 	str	x0, [x4,#8]
    5d18:	d5033bbf 	dmb	ish
    5d1c:	f9012c20 	str	x0, [x1,#600]
    5d20:	d5033bbf 	dmb	ish
    5d24:	f9400441 	ldr	x1, [x2,#8]
    5d28:	f9424e80 	ldr	x0, [x20,#1176]
    5d2c:	f901487f 	str	xzr, [x3,#656]
    5d30:	8b000020 	add	x0, x1, x0
    5d34:	f9000440 	str	x0, [x2,#8]
    5d38:	eb05001f 	cmp	x0, x5
    5d3c:	540003e8 	b.hi	5db8 <__deallocate_stack+0x144>
    5d40:	910a4260 	add	x0, x19, #0x290
    5d44:	52800002 	mov	w2, #0x0                   	// #0
    5d48:	91004000 	add	x0, x0, #0x10
    5d4c:	885f7c01 	ldxr	w1, [x0]
    5d50:	8803fc02 	stlxr	w3, w2, [x0]
    5d54:	35ffffc3 	cbnz	w3, 5d4c <__deallocate_stack+0xd8>
    5d58:	7100043f 	cmp	w1, #0x1
    5d5c:	540001ac 	b.gt	5d90 <__deallocate_stack+0x11c>
    5d60:	a94153f3 	ldp	x19, x20, [sp,#16]
    5d64:	a9425bf5 	ldp	x21, x22, [sp,#32]
    5d68:	a94363f7 	ldp	x23, x24, [sp,#48]
    5d6c:	a8c57bfd 	ldp	x29, x30, [sp],#80
    5d70:	d65f03c0 	ret
    5d74:	b9004fa2 	str	w2, [x29,#76]
    5d78:	9400267c 	bl	f768 <__lll_lock_wait_private>
    5d7c:	17ffffcf 	b	5cb8 <__deallocate_stack+0x44>
    5d80:	911bc280 	add	x0, x20, #0x6f0
    5d84:	52800001 	mov	w1, #0x0                   	// #0
    5d88:	97fffcd6 	bl	50e0 <_dl_deallocate_tls@plt>
    5d8c:	17ffffed 	b	5d40 <__deallocate_stack+0xcc>
    5d90:	d2801021 	mov	x1, #0x81                  	// #129
    5d94:	d2800022 	mov	x2, #0x1                   	// #1
    5d98:	d2800003 	mov	x3, #0x0                   	// #0
    5d9c:	d2800c48 	mov	x8, #0x62                  	// #98
    5da0:	d4000001 	svc	#0x0
    5da4:	a94153f3 	ldp	x19, x20, [sp,#16]
    5da8:	a9425bf5 	ldp	x21, x22, [sp,#32]
    5dac:	a94363f7 	ldp	x23, x24, [sp,#48]
    5db0:	a8c57bfd 	ldp	x29, x30, [sp],#80
    5db4:	d65f03c0 	ret
    5db8:	f94006c2 	ldr	x2, [x22,#8]
    5dbc:	eb16005f 	cmp	x2, x22
    5dc0:	f9400454 	ldr	x20, [x2,#8]
    5dc4:	54fffbe0 	b.eq	5d40 <__deallocate_stack+0xcc>
    5dc8:	aa0503f7 	mov	x23, x5
    5dcc:	14000006 	b	5de4 <__deallocate_stack+0x170>
    5dd0:	eb16029f 	cmp	x20, x22
    5dd4:	f9400680 	ldr	x0, [x20,#8]
    5dd8:	aa1403e2 	mov	x2, x20
    5ddc:	54fffb20 	b.eq	5d40 <__deallocate_stack+0xcc>
    5de0:	aa0003f4 	mov	x20, x0
    5de4:	d1030055 	sub	x21, x2, #0xc0
    5de8:	b940d2a0 	ldr	w0, [x21,#208]
    5dec:	6b1f001f 	cmp	w0, wzr
    5df0:	54ffff0c 	b.gt	5dd0 <__deallocate_stack+0x15c>
    5df4:	f9014a62 	str	x2, [x19,#656]
    5df8:	9118c040 	add	x0, x2, #0x630
    5dfc:	910a4278 	add	x24, x19, #0x290
    5e00:	52800001 	mov	w1, #0x0                   	// #0
    5e04:	d5033bbf 	dmb	ish
    5e08:	f9400043 	ldr	x3, [x2]
    5e0c:	f9400444 	ldr	x4, [x2,#8]
    5e10:	f9000464 	str	x4, [x3,#8]
    5e14:	f9400442 	ldr	x2, [x2,#8]
    5e18:	f9000043 	str	x3, [x2]
    5e1c:	d5033bbf 	dmb	ish
    5e20:	f9400702 	ldr	x2, [x24,#8]
    5e24:	f9424ea3 	ldr	x3, [x21,#1176]
    5e28:	f9014a7f 	str	xzr, [x19,#656]
    5e2c:	cb030042 	sub	x2, x2, x3
    5e30:	f9000702 	str	x2, [x24,#8]
    5e34:	97fffcab 	bl	50e0 <_dl_deallocate_tls@plt>
    5e38:	f9424aa0 	ldr	x0, [x21,#1168]
    5e3c:	f9424ea1 	ldr	x1, [x21,#1176]
    5e40:	97fffd1c 	bl	52b0 <munmap@plt>
    5e44:	350000a0 	cbnz	w0, 5e58 <__deallocate_stack+0x1e4>
    5e48:	f9400700 	ldr	x0, [x24,#8]
    5e4c:	eb17001f 	cmp	x0, x23
    5e50:	54fffc08 	b.hi	5dd0 <__deallocate_stack+0x15c>
    5e54:	17ffffbb 	b	5d40 <__deallocate_stack+0xcc>
    5e58:	97fffcc6 	bl	5170 <abort@plt>

0000000000005e5c <do_clone.constprop.4>:
    5e5c:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    5e60:	911bc005 	add	x5, x0, #0x6f0
    5e64:	910003fd 	mov	x29, sp
    5e68:	a90153f3 	stp	x19, x20, [sp,#16]
    5e6c:	f90013f6 	str	x22, [sp,#32]
    5e70:	f0000154 	adrp	x20, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5e74:	aa0003f3 	mov	x19, x0
    5e78:	aa0103f6 	mov	x22, x1
    5e7c:	91094280 	add	x0, x20, #0x250
    5e80:	aa0203e1 	mov	x1, x2
    5e84:	35000303 	cbnz	w3, 5ee4 <do_clone.constprop.4+0x88>
    5e88:	885ffc02 	ldaxr	w2, [x0]
    5e8c:	11000442 	add	w2, w2, #0x1
    5e90:	88037c02 	stxr	w3, w2, [x0]
    5e94:	35ffffa3 	cbnz	w3, 5e88 <do_clone.constprop.4+0x2c>
    5e98:	91034264 	add	x4, x19, #0xd0
    5e9c:	5281e002 	mov	w2, #0xf00                 	// #3840
    5ea0:	b0000000 	adrp	x0, 6000 <do_clone.constprop.4+0x1a4>
    5ea4:	72a007a2 	movk	w2, #0x3d, lsl #16
    5ea8:	913a3000 	add	x0, x0, #0xe8c
    5eac:	aa1303e3 	mov	x3, x19
    5eb0:	aa0403e6 	mov	x6, x4
    5eb4:	97fffd13 	bl	5300 <__clone@plt>
    5eb8:	3100041f 	cmn	w0, #0x1
    5ebc:	540008c0 	b.eq	5fd4 <do_clone.constprop.4+0x178>
    5ec0:	d53bd041 	mrs	x1, tpidr_el0
    5ec4:	52800022 	mov	w2, #0x1                   	// #1
    5ec8:	d11bc021 	sub	x1, x1, #0x6f0
    5ecc:	52800000 	mov	w0, #0x0                   	// #0
    5ed0:	b9000022 	str	w2, [x1]
    5ed4:	a94153f3 	ldp	x19, x20, [sp,#16]
    5ed8:	f94013f6 	ldr	x22, [sp,#32]
    5edc:	a8c57bfd 	ldp	x29, x30, [sp],#80
    5ee0:	d65f03c0 	ret
    5ee4:	b9004fbf 	str	wzr, [x29,#76]
    5ee8:	91106260 	add	x0, x19, #0x418
    5eec:	52800023 	mov	w3, #0x1                   	// #1
    5ef0:	885ffc02 	ldaxr	w2, [x0]
    5ef4:	6b1f005f 	cmp	w2, wzr
    5ef8:	54000061 	b.ne	5f04 <do_clone.constprop.4+0xa8>
    5efc:	88047c03 	stxr	w4, w3, [x0]
    5f00:	35ffff84 	cbnz	w4, 5ef0 <do_clone.constprop.4+0x94>
    5f04:	540005a1 	b.ne	5fb8 <do_clone.constprop.4+0x15c>
    5f08:	f0000154 	adrp	x20, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    5f0c:	91094280 	add	x0, x20, #0x250
    5f10:	885ffc02 	ldaxr	w2, [x0]
    5f14:	11000442 	add	w2, w2, #0x1
    5f18:	88037c02 	stxr	w3, w2, [x0]
    5f1c:	35ffffa3 	cbnz	w3, 5f10 <do_clone.constprop.4+0xb4>
    5f20:	91034264 	add	x4, x19, #0xd0
    5f24:	5281e002 	mov	w2, #0xf00                 	// #3840
    5f28:	b0000000 	adrp	x0, 6000 <do_clone.constprop.4+0x1a4>
    5f2c:	72a007a2 	movk	w2, #0x3d, lsl #16
    5f30:	913a3000 	add	x0, x0, #0xe8c
    5f34:	aa1303e3 	mov	x3, x19
    5f38:	aa0403e6 	mov	x6, x4
    5f3c:	97fffcf1 	bl	5300 <__clone@plt>
    5f40:	3100041f 	cmn	w0, #0x1
    5f44:	54000480 	b.eq	5fd4 <do_clone.constprop.4+0x178>
    5f48:	f94016c2 	ldr	x2, [x22,#40]
    5f4c:	b40000e2 	cbz	x2, 5f68 <do_clone.constprop.4+0x10c>
    5f50:	b980d260 	ldrsw	x0, [x19,#208]
    5f54:	d2800f48 	mov	x8, #0x7a                  	// #122
    5f58:	f9401ac1 	ldr	x1, [x22,#48]
    5f5c:	d4000001 	svc	#0x0
    5f60:	3140041f 	cmn	w0, #0x1, lsl #12
    5f64:	54000148 	b.hi	5f8c <do_clone.constprop.4+0x130>
    5f68:	b9400ac0 	ldr	w0, [x22,#8]
    5f6c:	360ffaa0 	tbz	w0, #1, 5ec0 <do_clone.constprop.4+0x64>
    5f70:	b980d260 	ldrsw	x0, [x19,#208]
    5f74:	9110c262 	add	x2, x19, #0x430
    5f78:	b9843661 	ldrsw	x1, [x19,#1076]
    5f7c:	d2800ee8 	mov	x8, #0x77                  	// #119
    5f80:	d4000001 	svc	#0x0
    5f84:	3140041f 	cmn	w0, #0x1, lsl #12
    5f88:	54fff9c9 	b.ls	5ec0 <do_clone.constprop.4+0x64>
    5f8c:	2a0003e3 	mov	w3, w0
    5f90:	d53bd040 	mrs	x0, tpidr_el0
    5f94:	d11bc000 	sub	x0, x0, #0x6f0
    5f98:	b980d261 	ldrsw	x1, [x19,#208]
    5f9c:	d2800402 	mov	x2, #0x20                  	// #32
    5fa0:	d2801068 	mov	x8, #0x83                  	// #131
    5fa4:	b980d400 	ldrsw	x0, [x0,#212]
    5fa8:	d4000001 	svc	#0x0
    5fac:	3140047f 	cmn	w3, #0x1, lsl #12
    5fb0:	5a8397e0 	csneg	w0, wzr, w3, ls
    5fb4:	17ffffc8 	b	5ed4 <do_clone.constprop.4+0x78>
    5fb8:	f9001ba1 	str	x1, [x29,#48]
    5fbc:	f9001fa5 	str	x5, [x29,#56]
    5fc0:	b9004fa2 	str	w2, [x29,#76]
    5fc4:	940025e9 	bl	f768 <__lll_lock_wait_private>
    5fc8:	f9401fa5 	ldr	x5, [x29,#56]
    5fcc:	f9401ba1 	ldr	x1, [x29,#48]
    5fd0:	17ffffce 	b	5f08 <do_clone.constprop.4+0xac>
    5fd4:	91094294 	add	x20, x20, #0x250
    5fd8:	885ffe80 	ldaxr	w0, [x20]
    5fdc:	51000400 	sub	w0, w0, #0x1
    5fe0:	88017e80 	stxr	w1, w0, [x20]
    5fe4:	35ffffa1 	cbnz	w1, 5fd8 <do_clone.constprop.4+0x17c>
    5fe8:	91107260 	add	x0, x19, #0x41c
    5fec:	52800002 	mov	w2, #0x0                   	// #0
    5ff0:	885ffc01 	ldaxr	w1, [x0]
    5ff4:	88037c02 	stxr	w3, w2, [x0]
    5ff8:	35ffffc3 	cbnz	w3, 5ff0 <do_clone.constprop.4+0x194>
    5ffc:	3100083f 	cmn	w1, #0x2
    6000:	540001c0 	b.eq	6038 <do_clone.constprop.4+0x1dc>
    6004:	aa1303e0 	mov	x0, x19
    6008:	97ffff1b 	bl	5c74 <__deallocate_stack>
    600c:	f94013f6 	ldr	x22, [sp,#32]
    6010:	b0000141 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    6014:	f947c421 	ldr	x1, [x1,#3976]
    6018:	d53bd040 	mrs	x0, tpidr_el0
    601c:	a94153f3 	ldp	x19, x20, [sp,#16]
    6020:	a8c57bfd 	ldp	x29, x30, [sp],#80
    6024:	b8616800 	ldr	w0, [x0,x1]
    6028:	52800161 	mov	w1, #0xb                   	// #11
    602c:	7100301f 	cmp	w0, #0xc
    6030:	1a811000 	csel	w0, w0, w1, ne
    6034:	d65f03c0 	ret
    6038:	d2801021 	mov	x1, #0x81                  	// #129
    603c:	d2800022 	mov	x2, #0x1                   	// #1
    6040:	d2800003 	mov	x3, #0x0                   	// #0
    6044:	d2800c48 	mov	x8, #0x62                  	// #98
    6048:	d4000001 	svc	#0x0
    604c:	17ffffee 	b	6004 <do_clone.constprop.4+0x1a8>

0000000000006050 <__make_stacks_executable>:
    6050:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    6054:	910003fd 	mov	x29, sp
    6058:	a9025bf5 	stp	x21, x22, [sp,#32]
    605c:	a90153f3 	stp	x19, x20, [sp,#16]
    6060:	a90363f7 	stp	x23, x24, [sp,#48]
    6064:	97fffc5f 	bl	51e0 <_dl_make_stack_executable@plt>
    6068:	2a0003f6 	mov	w22, w0
    606c:	340000e0 	cbz	w0, 6088 <__make_stacks_executable+0x38>
    6070:	2a1603e0 	mov	w0, w22
    6074:	a94153f3 	ldp	x19, x20, [sp,#16]
    6078:	a9425bf5 	ldp	x21, x22, [sp,#32]
    607c:	a94363f7 	ldp	x23, x24, [sp,#48]
    6080:	a8c57bfd 	ldp	x29, x30, [sp],#80
    6084:	d65f03c0 	ret
    6088:	d0000177 	adrp	x23, 34000 <__GI___pthread_keys+0x3d78>
    608c:	b9004fa0 	str	w0, [x29,#76]
    6090:	910a42e0 	add	x0, x23, #0x290
    6094:	52800021 	mov	w1, #0x1                   	// #1
    6098:	91004000 	add	x0, x0, #0x10
    609c:	885ffc02 	ldaxr	w2, [x0]
    60a0:	6b1f005f 	cmp	w2, wzr
    60a4:	54000061 	b.ne	60b0 <__make_stacks_executable+0x60>
    60a8:	88037c01 	stxr	w3, w1, [x0]
    60ac:	35ffff83 	cbnz	w3, 609c <__make_stacks_executable+0x4c>
    60b0:	540004e1 	b.ne	614c <__make_stacks_executable+0xfc>
    60b4:	d0000158 	adrp	x24, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    60b8:	91096314 	add	x20, x24, #0x258
    60bc:	f8410e93 	ldr	x19, [x20,#16]!
    60c0:	eb14027f 	cmp	x19, x20
    60c4:	540004a0 	b.eq	6158 <__make_stacks_executable+0x108>
    60c8:	b0000155 	adrp	x21, 2f000 <__FRAME_END__+0x18e30>
    60cc:	f947c6b5 	ldr	x21, [x21,#3976]
    60d0:	d53bd040 	mrs	x0, tpidr_el0
    60d4:	8b150015 	add	x21, x0, x21
    60d8:	14000004 	b	60e8 <__make_stacks_executable+0x98>
    60dc:	f9400273 	ldr	x19, [x19]
    60e0:	eb14027f 	cmp	x19, x20
    60e4:	540003a0 	b.eq	6158 <__make_stacks_executable+0x108>
    60e8:	f941f261 	ldr	x1, [x19,#992]
    60ec:	528000e2 	mov	w2, #0x7                   	// #7
    60f0:	f941ea60 	ldr	x0, [x19,#976]
    60f4:	f941ee63 	ldr	x3, [x19,#984]
    60f8:	8b010000 	add	x0, x0, x1
    60fc:	cb010061 	sub	x1, x3, x1
    6100:	97fffcac 	bl	53b0 <mprotect@plt>
    6104:	34fffec0 	cbz	w0, 60dc <__make_stacks_executable+0x8c>
    6108:	b94002a1 	ldr	w1, [x21]
    610c:	34fffe81 	cbz	w1, 60dc <__make_stacks_executable+0x8c>
    6110:	2a0103f6 	mov	w22, w1
    6114:	910a42e0 	add	x0, x23, #0x290
    6118:	52800002 	mov	w2, #0x0                   	// #0
    611c:	91004000 	add	x0, x0, #0x10
    6120:	885f7c01 	ldxr	w1, [x0]
    6124:	8803fc02 	stlxr	w3, w2, [x0]
    6128:	35ffffc3 	cbnz	w3, 6120 <__make_stacks_executable+0xd0>
    612c:	7100043f 	cmp	w1, #0x1
    6130:	54fffa0d 	b.le	6070 <__make_stacks_executable+0x20>
    6134:	d2801021 	mov	x1, #0x81                  	// #129
    6138:	d2800022 	mov	x2, #0x1                   	// #1
    613c:	d2800003 	mov	x3, #0x0                   	// #0
    6140:	d2800c48 	mov	x8, #0x62                  	// #98
    6144:	d4000001 	svc	#0x0
    6148:	17ffffca 	b	6070 <__make_stacks_executable+0x20>
    614c:	b9004fa2 	str	w2, [x29,#76]
    6150:	94002586 	bl	f768 <__lll_lock_wait_private>
    6154:	17ffffd8 	b	60b4 <__make_stacks_executable+0x64>
    6158:	f9412f13 	ldr	x19, [x24,#600]
    615c:	91096314 	add	x20, x24, #0x258
    6160:	eb14027f 	cmp	x19, x20
    6164:	54fffd80 	b.eq	6114 <__make_stacks_executable+0xc4>
    6168:	b0000155 	adrp	x21, 2f000 <__FRAME_END__+0x18e30>
    616c:	f947c6b5 	ldr	x21, [x21,#3976]
    6170:	d53bd040 	mrs	x0, tpidr_el0
    6174:	8b150015 	add	x21, x0, x21
    6178:	14000004 	b	6188 <__make_stacks_executable+0x138>
    617c:	f9400273 	ldr	x19, [x19]
    6180:	eb14027f 	cmp	x19, x20
    6184:	54fffc80 	b.eq	6114 <__make_stacks_executable+0xc4>
    6188:	f941f261 	ldr	x1, [x19,#992]
    618c:	528000e2 	mov	w2, #0x7                   	// #7
    6190:	f941ea60 	ldr	x0, [x19,#976]
    6194:	f941ee63 	ldr	x3, [x19,#984]
    6198:	8b010000 	add	x0, x0, x1
    619c:	cb010061 	sub	x1, x3, x1
    61a0:	97fffc84 	bl	53b0 <mprotect@plt>
    61a4:	34fffec0 	cbz	w0, 617c <__make_stacks_executable+0x12c>
    61a8:	b94002a0 	ldr	w0, [x21]
    61ac:	34fffe80 	cbz	w0, 617c <__make_stacks_executable+0x12c>
    61b0:	2a0003f6 	mov	w22, w0
    61b4:	17ffffd8 	b	6114 <__make_stacks_executable+0xc4>

00000000000061b8 <__reclaim_stacks>:
    61b8:	a9b97bfd 	stp	x29, x30, [sp,#-112]!
    61bc:	d53bd040 	mrs	x0, tpidr_el0
    61c0:	910003fd 	mov	x29, sp
    61c4:	a9046bf9 	stp	x25, x26, [sp,#64]
    61c8:	d0000179 	adrp	x25, 34000 <__GI___pthread_keys+0x3d78>
    61cc:	a90363f7 	stp	x23, x24, [sp,#48]
    61d0:	fd0033e8 	str	d8, [sp,#96]
    61d4:	d11bc018 	sub	x24, x0, #0x6f0
    61d8:	9e670008 	fmov	d8, x0
    61dc:	f9414b20 	ldr	x0, [x25,#656]
    61e0:	a90153f3 	stp	x19, x20, [sp,#16]
    61e4:	a9025bf5 	stp	x21, x22, [sp,#32]
    61e8:	a90573fb 	stp	x27, x28, [sp,#80]
    61ec:	b5000e20 	cbnz	x0, 63b0 <__reclaim_stacks+0x1f8>
    61f0:	d000015a 	adrp	x26, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    61f4:	91096340 	add	x0, x26, #0x258
    61f8:	f9400817 	ldr	x23, [x0,#16]
    61fc:	91096340 	add	x0, x26, #0x258
    6200:	9100401c 	add	x28, x0, #0x10
    6204:	eb1c02ff 	cmp	x23, x28
    6208:	54001120 	b.eq	642c <__reclaim_stacks+0x274>
    620c:	52800036 	mov	w22, #0x1                   	// #1
    6210:	aa0003fb 	mov	x27, x0
    6214:	14000004 	b	6224 <__reclaim_stacks+0x6c>
    6218:	f94002f7 	ldr	x23, [x23]
    621c:	eb1c02ff 	cmp	x23, x28
    6220:	54000440 	b.eq	62a8 <__reclaim_stacks+0xf0>
    6224:	d10302f4 	sub	x20, x23, #0xc0
    6228:	eb14031f 	cmp	x24, x20
    622c:	54ffff60 	b.eq	6218 <__reclaim_stacks+0x60>
    6230:	910a4321 	add	x1, x25, #0x290
    6234:	f9424e80 	ldr	x0, [x20,#1176]
    6238:	b940d703 	ldr	w3, [x24,#212]
    623c:	39504282 	ldrb	w2, [x20,#1040]
    6240:	f9400424 	ldr	x4, [x1,#8]
    6244:	b900d29f 	str	wzr, [x20,#208]
    6248:	8b000080 	add	x0, x4, x0
    624c:	b900d683 	str	w3, [x20,#212]
    6250:	f9000420 	str	x0, [x1,#8]
    6254:	34fffe22 	cbz	w2, 6218 <__reclaim_stacks+0x60>
    6258:	910142e0 	add	x0, x23, #0x50
    625c:	52800001 	mov	w1, #0x0                   	// #0
    6260:	d2804002 	mov	x2, #0x200                 	// #512
    6264:	910962f3 	add	x19, x23, #0x258
    6268:	910d42f5 	add	x21, x23, #0x350
    626c:	97fffb89 	bl	5090 <memset@plt>
    6270:	3910429f 	strb	wzr, [x20,#1040]
    6274:	f9400263 	ldr	x3, [x19]
    6278:	52800001 	mov	w1, #0x0                   	// #0
    627c:	d2804002 	mov	x2, #0x200                 	// #512
    6280:	91002273 	add	x19, x19, #0x8
    6284:	aa0303e0 	mov	x0, x3
    6288:	b4000063 	cbz	x3, 6294 <__reclaim_stacks+0xdc>
    628c:	97fffb81 	bl	5090 <memset@plt>
    6290:	39104296 	strb	w22, [x20,#1040]
    6294:	eb15027f 	cmp	x19, x21
    6298:	54fffee1 	b.ne	6274 <__reclaim_stacks+0xbc>
    629c:	f94002f7 	ldr	x23, [x23]
    62a0:	eb1c02ff 	cmp	x23, x28
    62a4:	54fffc01 	b.ne	6224 <__reclaim_stacks+0x6c>
    62a8:	f9400363 	ldr	x3, [x27]
    62ac:	f9400b77 	ldr	x23, [x27,#16]
    62b0:	eb1b007f 	cmp	x3, x27
    62b4:	54000100 	b.eq	62d4 <__reclaim_stacks+0x11c>
    62b8:	aa0303e0 	mov	x0, x3
    62bc:	91096342 	add	x2, x26, #0x258
    62c0:	b940d701 	ldr	w1, [x24,#212]
    62c4:	b9001401 	str	w1, [x0,#20]
    62c8:	f9400000 	ldr	x0, [x0]
    62cc:	eb02001f 	cmp	x0, x2
    62d0:	54ffff81 	b.ne	62c0 <__reclaim_stacks+0x108>
    62d4:	91096340 	add	x0, x26, #0x258
    62d8:	91004001 	add	x1, x0, #0x10
    62dc:	eb0102ff 	cmp	x23, x1
    62e0:	54000100 	b.eq	6300 <__reclaim_stacks+0x148>
    62e4:	f90006e0 	str	x0, [x23,#8]
    62e8:	f9400c01 	ldr	x1, [x0,#24]
    62ec:	f9000023 	str	x3, [x1]
    62f0:	f9412f42 	ldr	x2, [x26,#600]
    62f4:	f9400800 	ldr	x0, [x0,#16]
    62f8:	f9012f40 	str	x0, [x26,#600]
    62fc:	f9000441 	str	x1, [x2,#8]
    6300:	9e660100 	fmov	x0, d8
    6304:	91030302 	add	x2, x24, #0xc0
    6308:	f9014b22 	str	x2, [x25,#656]
    630c:	910a4323 	add	x3, x25, #0x290
    6310:	9109635a 	add	x26, x26, #0x258
    6314:	91006061 	add	x1, x3, #0x18
    6318:	d5033bbf 	dmb	ish
    631c:	d118c01b 	sub	x27, x0, #0x630
    6320:	91004340 	add	x0, x26, #0x10
    6324:	f9400364 	ldr	x4, [x27]
    6328:	f9400765 	ldr	x5, [x27,#8]
    632c:	f9000485 	str	x5, [x4,#8]
    6330:	f9400765 	ldr	x5, [x27,#8]
    6334:	f90000a4 	str	x4, [x5]
    6338:	d5033bbf 	dmb	ish
    633c:	39504b04 	ldrb	w4, [x24,#1042]
    6340:	f9014b3f 	str	xzr, [x25,#656]
    6344:	f9000f40 	str	x0, [x26,#24]
    6348:	f9000b40 	str	x0, [x26,#16]
    634c:	f9000421 	str	x1, [x1,#8]
    6350:	f9000c61 	str	x1, [x3,#24]
    6354:	350005e4 	cbnz	w4, 6410 <__reclaim_stacks+0x258>
    6358:	f9000360 	str	x0, [x27]
    635c:	f9000760 	str	x0, [x27,#8]
    6360:	f9400b40 	ldr	x0, [x26,#16]
    6364:	f9000402 	str	x2, [x0,#8]
    6368:	d5033bbf 	dmb	ish
    636c:	f9000b42 	str	x2, [x26,#16]
    6370:	910a4320 	add	x0, x25, #0x290
    6374:	d0000141 	adrp	x1, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    6378:	f9014b3f 	str	xzr, [x25,#656]
    637c:	52800022 	mov	w2, #0x1                   	// #1
    6380:	a94153f3 	ldp	x19, x20, [sp,#16]
    6384:	b900101f 	str	wzr, [x0,#16]
    6388:	d0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    638c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    6390:	a94363f7 	ldp	x23, x24, [sp,#48]
    6394:	a9446bf9 	ldp	x25, x26, [sp,#64]
    6398:	a94573fb 	ldp	x27, x28, [sp,#80]
    639c:	fd4033e8 	ldr	d8, [sp,#96]
    63a0:	b9025022 	str	w2, [x1,#592]
    63a4:	b902881f 	str	wzr, [x0,#648]
    63a8:	a8c77bfd 	ldp	x29, x30, [sp],#112
    63ac:	d65f03c0 	ret
    63b0:	927ff801 	and	x1, x0, #0xfffffffffffffffe
    63b4:	360001c0 	tbz	w0, #0, 63ec <__reclaim_stacks+0x234>
    63b8:	d000015a 	adrp	x26, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    63bc:	91096342 	add	x2, x26, #0x258
    63c0:	aa0203e0 	mov	x0, x2
    63c4:	f8410c17 	ldr	x23, [x0,#16]!
    63c8:	f94006e3 	ldr	x3, [x23,#8]
    63cc:	eb00007f 	cmp	x3, x0
    63d0:	54000360 	b.eq	643c <__reclaim_stacks+0x284>
    63d4:	f9000037 	str	x23, [x1]
    63d8:	91096342 	add	x2, x26, #0x258
    63dc:	f9000420 	str	x0, [x1,#8]
    63e0:	f9000001 	str	x1, [x0]
    63e4:	f9400857 	ldr	x23, [x2,#16]
    63e8:	17ffff85 	b	61fc <__reclaim_stacks+0x44>
    63ec:	f9400020 	ldr	x0, [x1]
    63f0:	d000015a 	adrp	x26, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    63f4:	f9400423 	ldr	x3, [x1,#8]
    63f8:	91096342 	add	x2, x26, #0x258
    63fc:	f9000403 	str	x3, [x0,#8]
    6400:	f9400421 	ldr	x1, [x1,#8]
    6404:	f9000020 	str	x0, [x1]
    6408:	f9400857 	ldr	x23, [x2,#16]
    640c:	17ffff7c 	b	61fc <__reclaim_stacks+0x44>
    6410:	f9000361 	str	x1, [x27]
    6414:	f9000761 	str	x1, [x27,#8]
    6418:	f9400c60 	ldr	x0, [x3,#24]
    641c:	f9000402 	str	x2, [x0,#8]
    6420:	d5033bbf 	dmb	ish
    6424:	f9000c62 	str	x2, [x3,#24]
    6428:	17ffffd2 	b	6370 <__reclaim_stacks+0x1b8>
    642c:	f9412f43 	ldr	x3, [x26,#600]
    6430:	eb00007f 	cmp	x3, x0
    6434:	54fff421 	b.ne	62b8 <__reclaim_stacks+0x100>
    6438:	17ffffb2 	b	6300 <__reclaim_stacks+0x148>
    643c:	f9412f40 	ldr	x0, [x26,#600]
    6440:	f9400403 	ldr	x3, [x0,#8]
    6444:	eb02007f 	cmp	x3, x2
    6448:	54ffeda0 	b.eq	61fc <__reclaim_stacks+0x44>
    644c:	aa0003f7 	mov	x23, x0
    6450:	aa0203e0 	mov	x0, x2
    6454:	17ffffe0 	b	63d4 <__reclaim_stacks+0x21c>

0000000000006458 <__nptl_setxid_error>:
    6458:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    645c:	910003fd 	mov	x29, sp
    6460:	b9402402 	ldr	w2, [x0,#36]
    6464:	6b01005f 	cmp	w2, w1
    6468:	54000180 	b.eq	6498 <__nptl_setxid_error+0x40>
    646c:	3100045f 	cmn	w2, #0x1
    6470:	540001c1 	b.ne	64a8 <__nptl_setxid_error+0x50>
    6474:	b9001fa2 	str	w2, [x29,#28]
    6478:	91009002 	add	x2, x0, #0x24
    647c:	b9401fa3 	ldr	w3, [x29,#28]
    6480:	885ffc44 	ldaxr	w4, [x2]
    6484:	6b03009f 	cmp	w4, w3
    6488:	54000061 	b.ne	6494 <__nptl_setxid_error+0x3c>
    648c:	88057c41 	stxr	w5, w1, [x2]
    6490:	35ffff85 	cbnz	w5, 6480 <__nptl_setxid_error+0x28>
    6494:	54000061 	b.ne	64a0 <__nptl_setxid_error+0x48>
    6498:	a8c27bfd 	ldp	x29, x30, [sp],#32
    649c:	d65f03c0 	ret
    64a0:	b9001fa4 	str	w4, [x29,#28]
    64a4:	17ffffef 	b	6460 <__nptl_setxid_error+0x8>
    64a8:	97fffb32 	bl	5170 <abort@plt>

00000000000064ac <__nptl_setxid>:
    64ac:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    64b0:	910003fd 	mov	x29, sp
    64b4:	a9025bf5 	stp	x21, x22, [sp,#32]
    64b8:	d0000175 	adrp	x21, 34000 <__GI___pthread_keys+0x3d78>
    64bc:	f90023f9 	str	x25, [sp,#64]
    64c0:	910a42a1 	add	x1, x21, #0x290
    64c4:	b9005fbf 	str	wzr, [x29,#92]
    64c8:	a90153f3 	stp	x19, x20, [sp,#16]
    64cc:	a90363f7 	stp	x23, x24, [sp,#48]
    64d0:	aa0003f4 	mov	x20, x0
    64d4:	91004020 	add	x0, x1, #0x10
    64d8:	52800021 	mov	w1, #0x1                   	// #1
    64dc:	885ffc02 	ldaxr	w2, [x0]
    64e0:	6b1f005f 	cmp	w2, wzr
    64e4:	54000061 	b.ne	64f0 <__nptl_setxid+0x44>
    64e8:	88037c01 	stxr	w3, w1, [x0]
    64ec:	35ffff83 	cbnz	w3, 64dc <__nptl_setxid+0x30>
    64f0:	54000060 	b.eq	64fc <__nptl_setxid+0x50>
    64f4:	b9005fa2 	str	w2, [x29,#92]
    64f8:	9400249c 	bl	f768 <__lll_lock_wait_private>
    64fc:	d0000157 	adrp	x23, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    6500:	12800000 	mov	w0, #0xffffffff            	// #-1
    6504:	910962f9 	add	x25, x23, #0x258
    6508:	b900229f 	str	wzr, [x20,#32]
    650c:	b9002680 	str	w0, [x20,#36]
    6510:	d0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    6514:	d53bd056 	mrs	x22, tpidr_el0
    6518:	f8410f38 	ldr	x24, [x25,#16]!
    651c:	d11bc2d3 	sub	x19, x22, #0x6f0
    6520:	f9018814 	str	x20, [x0,#784]
    6524:	eb19031f 	cmp	x24, x25
    6528:	54000100 	b.eq	6548 <__nptl_setxid+0x9c>
    652c:	d1030300 	sub	x0, x24, #0xc0
    6530:	eb13001f 	cmp	x0, x19
    6534:	54000040 	b.eq	653c <__nptl_setxid+0x90>
    6538:	97fffd5f 	bl	5ab4 <setxid_mark_thread.isra.0>
    653c:	f9400318 	ldr	x24, [x24]
    6540:	eb19031f 	cmp	x24, x25
    6544:	54ffff41 	b.ne	652c <__nptl_setxid+0x80>
    6548:	910a42a0 	add	x0, x21, #0x290
    654c:	91006019 	add	x25, x0, #0x18
    6550:	f9400c18 	ldr	x24, [x0,#24]
    6554:	eb19031f 	cmp	x24, x25
    6558:	54000100 	b.eq	6578 <__nptl_setxid+0xcc>
    655c:	d1030300 	sub	x0, x24, #0xc0
    6560:	eb13001f 	cmp	x0, x19
    6564:	54000040 	b.eq	656c <__nptl_setxid+0xc0>
    6568:	97fffd53 	bl	5ab4 <setxid_mark_thread.isra.0>
    656c:	f9400318 	ldr	x24, [x24]
    6570:	eb19031f 	cmp	x24, x25
    6574:	54ffff41 	b.ne	655c <__nptl_setxid+0xb0>
    6578:	910962e9 	add	x9, x23, #0x258
    657c:	910a42a6 	add	x6, x21, #0x290
    6580:	91004127 	add	x7, x9, #0x10
    6584:	910060c6 	add	x6, x6, #0x18
    6588:	f9400923 	ldr	x3, [x9,#16]
    658c:	52800004 	mov	w4, #0x0                   	// #0
    6590:	eb07007f 	cmp	x3, x7
    6594:	54000240 	b.eq	65dc <__nptl_setxid+0x130>
    6598:	d1030061 	sub	x1, x3, #0xc0
    659c:	eb01027f 	cmp	x19, x1
    65a0:	54000180 	b.eq	65d0 <__nptl_setxid+0x124>
    65a4:	b9410820 	ldr	w0, [x1,#264]
    65a8:	36300100 	tbz	w0, #6, 65c8 <__nptl_setxid+0x11c>
    65ac:	b980d660 	ldrsw	x0, [x19,#212]
    65b0:	d2800422 	mov	x2, #0x21                  	// #33
    65b4:	b980d021 	ldrsw	x1, [x1,#208]
    65b8:	d2801068 	mov	x8, #0x83                  	// #131
    65bc:	d4000001 	svc	#0x0
    65c0:	3140041f 	cmn	w0, #0x1, lsl #12
    65c4:	540010e9 	b.ls	67e0 <__nptl_setxid+0x334>
    65c8:	52800000 	mov	w0, #0x0                   	// #0
    65cc:	0b000084 	add	w4, w4, w0
    65d0:	f9400063 	ldr	x3, [x3]
    65d4:	eb07007f 	cmp	x3, x7
    65d8:	54fffe01 	b.ne	6598 <__nptl_setxid+0xec>
    65dc:	f94000c3 	ldr	x3, [x6]
    65e0:	eb06007f 	cmp	x3, x6
    65e4:	54000240 	b.eq	662c <__nptl_setxid+0x180>
    65e8:	d1030061 	sub	x1, x3, #0xc0
    65ec:	eb01027f 	cmp	x19, x1
    65f0:	54000180 	b.eq	6620 <__nptl_setxid+0x174>
    65f4:	b9410820 	ldr	w0, [x1,#264]
    65f8:	36300100 	tbz	w0, #6, 6618 <__nptl_setxid+0x16c>
    65fc:	b980d660 	ldrsw	x0, [x19,#212]
    6600:	d2800422 	mov	x2, #0x21                  	// #33
    6604:	b980d021 	ldrsw	x1, [x1,#208]
    6608:	d2801068 	mov	x8, #0x83                  	// #131
    660c:	d4000001 	svc	#0x0
    6610:	3140041f 	cmn	w0, #0x1, lsl #12
    6614:	54000f89 	b.ls	6804 <__nptl_setxid+0x358>
    6618:	52800000 	mov	w0, #0x0                   	// #0
    661c:	0b000084 	add	w4, w4, w0
    6620:	f9400063 	ldr	x3, [x3]
    6624:	eb06007f 	cmp	x3, x6
    6628:	54fffe01 	b.ne	65e8 <__nptl_setxid+0x13c>
    662c:	aa1403e5 	mov	x5, x20
    6630:	b8420ca2 	ldr	w2, [x5,#32]!
    6634:	34000122 	cbz	w2, 6658 <__nptl_setxid+0x1ac>
    6638:	aa0503e0 	mov	x0, x5
    663c:	d2801001 	mov	x1, #0x80                  	// #128
    6640:	93407c42 	sxtw	x2, w2
    6644:	d2800003 	mov	x3, #0x0                   	// #0
    6648:	d2800c48 	mov	x8, #0x62                  	// #98
    664c:	d4000001 	svc	#0x0
    6650:	b9402282 	ldr	w2, [x20,#32]
    6654:	35ffff22 	cbnz	w2, 6638 <__nptl_setxid+0x18c>
    6658:	35fff984 	cbnz	w4, 6588 <__nptl_setxid+0xdc>
    665c:	910962f7 	add	x23, x23, #0x258
    6660:	52800026 	mov	w6, #0x1                   	// #1
    6664:	f8410ee4 	ldr	x4, [x23,#16]!
    6668:	eb17009f 	cmp	x4, x23
    666c:	54000320 	b.eq	66d0 <__nptl_setxid+0x224>
    6670:	d1030085 	sub	x5, x4, #0xc0
    6674:	eb05027f 	cmp	x19, x5
    6678:	54000260 	b.eq	66c4 <__nptl_setxid+0x218>
    667c:	b94108a1 	ldr	w1, [x5,#264]
    6680:	36300221 	tbz	w1, #6, 66c4 <__nptl_setxid+0x218>
    6684:	b9005fa1 	str	w1, [x29,#92]
    6688:	12197823 	and	w3, w1, #0xffffffbf
    668c:	91012082 	add	x2, x4, #0x48
    6690:	885ffc40 	ldaxr	w0, [x2]
    6694:	6b01001f 	cmp	w0, w1
    6698:	54000061 	b.ne	66a4 <__nptl_setxid+0x1f8>
    669c:	88077c43 	stxr	w7, w3, [x2]
    66a0:	35ffff87 	cbnz	w7, 6690 <__nptl_setxid+0x1e4>
    66a4:	54000c21 	b.ne	6828 <__nptl_setxid+0x37c>
    66a8:	b9041ca6 	str	w6, [x5,#1052]
    66ac:	910d7080 	add	x0, x4, #0x35c
    66b0:	d2801021 	mov	x1, #0x81                  	// #129
    66b4:	d2800022 	mov	x2, #0x1                   	// #1
    66b8:	d2800003 	mov	x3, #0x0                   	// #0
    66bc:	d2800c48 	mov	x8, #0x62                  	// #98
    66c0:	d4000001 	svc	#0x0
    66c4:	f9400084 	ldr	x4, [x4]
    66c8:	eb17009f 	cmp	x4, x23
    66cc:	54fffd21 	b.ne	6670 <__nptl_setxid+0x1c4>
    66d0:	910a42a0 	add	x0, x21, #0x290
    66d4:	52800027 	mov	w7, #0x1                   	// #1
    66d8:	91006006 	add	x6, x0, #0x18
    66dc:	f9400c04 	ldr	x4, [x0,#24]
    66e0:	eb06009f 	cmp	x4, x6
    66e4:	54000320 	b.eq	6748 <__nptl_setxid+0x29c>
    66e8:	d1030085 	sub	x5, x4, #0xc0
    66ec:	eb05027f 	cmp	x19, x5
    66f0:	54000260 	b.eq	673c <__nptl_setxid+0x290>
    66f4:	b94108a1 	ldr	w1, [x5,#264]
    66f8:	36300221 	tbz	w1, #6, 673c <__nptl_setxid+0x290>
    66fc:	b9005fa1 	str	w1, [x29,#92]
    6700:	12197823 	and	w3, w1, #0xffffffbf
    6704:	91012082 	add	x2, x4, #0x48
    6708:	885ffc40 	ldaxr	w0, [x2]
    670c:	6b01001f 	cmp	w0, w1
    6710:	54000061 	b.ne	671c <__nptl_setxid+0x270>
    6714:	88087c43 	stxr	w8, w3, [x2]
    6718:	35ffff88 	cbnz	w8, 6708 <__nptl_setxid+0x25c>
    671c:	540008a1 	b.ne	6830 <__nptl_setxid+0x384>
    6720:	b9041ca7 	str	w7, [x5,#1052]
    6724:	910d7080 	add	x0, x4, #0x35c
    6728:	d2801021 	mov	x1, #0x81                  	// #129
    672c:	d2800022 	mov	x2, #0x1                   	// #1
    6730:	d2800003 	mov	x3, #0x0                   	// #0
    6734:	d2800c48 	mov	x8, #0x62                  	// #98
    6738:	d4000001 	svc	#0x0
    673c:	f9400084 	ldr	x4, [x4]
    6740:	eb06009f 	cmp	x4, x6
    6744:	54fffd21 	b.ne	66e8 <__nptl_setxid+0x23c>
    6748:	f9400680 	ldr	x0, [x20,#8]
    674c:	f9400a81 	ldr	x1, [x20,#16]
    6750:	f9400e82 	ldr	x2, [x20,#24]
    6754:	b9800288 	ldrsw	x8, [x20]
    6758:	d4000001 	svc	#0x0
    675c:	3140041f 	cmn	w0, #0x1, lsl #12
    6760:	2a0003e5 	mov	w5, w0
    6764:	540006e8 	b.hi	6840 <__nptl_setxid+0x394>
    6768:	52800002 	mov	w2, #0x0                   	// #0
    676c:	b9402681 	ldr	w1, [x20,#36]
    6770:	6b01005f 	cmp	w2, w1
    6774:	54000180 	b.eq	67a4 <__nptl_setxid+0x2f8>
    6778:	3100043f 	cmn	w1, #0x1
    677c:	54000881 	b.ne	688c <__nptl_setxid+0x3e0>
    6780:	b9005fa1 	str	w1, [x29,#92]
    6784:	91009281 	add	x1, x20, #0x24
    6788:	b9405fa3 	ldr	w3, [x29,#92]
    678c:	885ffc24 	ldaxr	w4, [x1]
    6790:	6b03009f 	cmp	w4, w3
    6794:	54000061 	b.ne	67a0 <__nptl_setxid+0x2f4>
    6798:	88007c22 	stxr	w0, w2, [x1]
    679c:	35ffff80 	cbnz	w0, 678c <__nptl_setxid+0x2e0>
    67a0:	540004c1 	b.ne	6838 <__nptl_setxid+0x38c>
    67a4:	910a42b5 	add	x21, x21, #0x290
    67a8:	52800001 	mov	w1, #0x0                   	// #0
    67ac:	910042b5 	add	x21, x21, #0x10
    67b0:	885f7ea0 	ldxr	w0, [x21]
    67b4:	8802fea1 	stlxr	w2, w1, [x21]
    67b8:	35ffffc2 	cbnz	w2, 67b0 <__nptl_setxid+0x304>
    67bc:	7100041f 	cmp	w0, #0x1
    67c0:	540004cc 	b.gt	6858 <__nptl_setxid+0x3ac>
    67c4:	2a0503e0 	mov	w0, w5
    67c8:	f94023f9 	ldr	x25, [sp,#64]
    67cc:	a94153f3 	ldp	x19, x20, [sp,#16]
    67d0:	a9425bf5 	ldp	x21, x22, [sp,#32]
    67d4:	a94363f7 	ldp	x23, x24, [sp,#48]
    67d8:	a8c67bfd 	ldp	x29, x30, [sp],#96
    67dc:	d65f03c0 	ret
    67e0:	91008280 	add	x0, x20, #0x20
    67e4:	885ffc01 	ldaxr	w1, [x0]
    67e8:	11000422 	add	w2, w1, #0x1
    67ec:	88057c02 	stxr	w5, w2, [x0]
    67f0:	35ffffa5 	cbnz	w5, 67e4 <__nptl_setxid+0x338>
    67f4:	b90057a1 	str	w1, [x29,#84]
    67f8:	52800020 	mov	w0, #0x1                   	// #1
    67fc:	b94057a1 	ldr	w1, [x29,#84]
    6800:	17ffff73 	b	65cc <__nptl_setxid+0x120>
    6804:	91008280 	add	x0, x20, #0x20
    6808:	885ffc01 	ldaxr	w1, [x0]
    680c:	11000422 	add	w2, w1, #0x1
    6810:	88057c02 	stxr	w5, w2, [x0]
    6814:	35ffffa5 	cbnz	w5, 6808 <__nptl_setxid+0x35c>
    6818:	b9005ba1 	str	w1, [x29,#88]
    681c:	52800020 	mov	w0, #0x1                   	// #1
    6820:	b9405ba1 	ldr	w1, [x29,#88]
    6824:	17ffff7e 	b	661c <__nptl_setxid+0x170>
    6828:	b9005fa0 	str	w0, [x29,#92]
    682c:	17ffff94 	b	667c <__nptl_setxid+0x1d0>
    6830:	b9005fa0 	str	w0, [x29,#92]
    6834:	17ffffb0 	b	66f4 <__nptl_setxid+0x248>
    6838:	b9005fa4 	str	w4, [x29,#92]
    683c:	17ffffcc 	b	676c <__nptl_setxid+0x2c0>
    6840:	4b0003e2 	neg	w2, w0
    6844:	b0000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    6848:	f947c400 	ldr	x0, [x0,#3976]
    684c:	12800005 	mov	w5, #0xffffffff            	// #-1
    6850:	b8206ac2 	str	w2, [x22,x0]
    6854:	17ffffc6 	b	676c <__nptl_setxid+0x2c0>
    6858:	aa1503e0 	mov	x0, x21
    685c:	d2801021 	mov	x1, #0x81                  	// #129
    6860:	d2800022 	mov	x2, #0x1                   	// #1
    6864:	d2800003 	mov	x3, #0x0                   	// #0
    6868:	d2800c48 	mov	x8, #0x62                  	// #98
    686c:	d4000001 	svc	#0x0
    6870:	2a0503e0 	mov	w0, w5
    6874:	f94023f9 	ldr	x25, [sp,#64]
    6878:	a94153f3 	ldp	x19, x20, [sp,#16]
    687c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    6880:	a94363f7 	ldp	x23, x24, [sp,#48]
    6884:	a8c67bfd 	ldp	x29, x30, [sp],#96
    6888:	d65f03c0 	ret
    688c:	97fffa39 	bl	5170 <abort@plt>

0000000000006890 <__pthread_init_static_tls>:
    6890:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    6894:	910003fd 	mov	x29, sp
    6898:	a9025bf5 	stp	x21, x22, [sp,#32]
    689c:	d0000175 	adrp	x21, 34000 <__GI___pthread_keys+0x3d78>
    68a0:	f9001bf7 	str	x23, [sp,#48]
    68a4:	910a42a1 	add	x1, x21, #0x290
    68a8:	b9004fbf 	str	wzr, [x29,#76]
    68ac:	a90153f3 	stp	x19, x20, [sp,#16]
    68b0:	aa0003f3 	mov	x19, x0
    68b4:	91004020 	add	x0, x1, #0x10
    68b8:	52800021 	mov	w1, #0x1                   	// #1
    68bc:	885ffc02 	ldaxr	w2, [x0]
    68c0:	6b1f005f 	cmp	w2, wzr
    68c4:	54000061 	b.ne	68d0 <__pthread_init_static_tls+0x40>
    68c8:	88037c01 	stxr	w3, w1, [x0]
    68cc:	35ffff83 	cbnz	w3, 68bc <__pthread_init_static_tls+0x2c>
    68d0:	54000881 	b.ne	69e0 <__pthread_init_static_tls+0x150>
    68d4:	d0000156 	adrp	x22, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    68d8:	52800037 	mov	w23, #0x1                   	// #1
    68dc:	910962d6 	add	x22, x22, #0x258
    68e0:	f8410ed4 	ldr	x20, [x22,#16]!
    68e4:	eb16029f 	cmp	x20, x22
    68e8:	540002c0 	b.eq	6940 <__pthread_init_static_tls+0xb0>
    68ec:	f9422262 	ldr	x2, [x19,#1088]
    68f0:	f9431a83 	ldr	x3, [x20,#1584]
    68f4:	f9421e61 	ldr	x1, [x19,#1080]
    68f8:	d37cec42 	lsl	x2, x2, #4
    68fc:	8b020064 	add	x4, x3, x2
    6900:	9118c021 	add	x1, x1, #0x630
    6904:	8b010281 	add	x1, x20, x1
    6908:	f8226861 	str	x1, [x3,x2]
    690c:	39002097 	strb	w23, [x4,#8]
    6910:	aa0103e0 	mov	x0, x1
    6914:	f9420e62 	ldr	x2, [x19,#1048]
    6918:	f9420a61 	ldr	x1, [x19,#1040]
    691c:	97fffa51 	bl	5260 <mempcpy@plt>
    6920:	f9421263 	ldr	x3, [x19,#1056]
    6924:	52800001 	mov	w1, #0x0                   	// #0
    6928:	f9420e62 	ldr	x2, [x19,#1048]
    692c:	cb020062 	sub	x2, x3, x2
    6930:	97fff9d8 	bl	5090 <memset@plt>
    6934:	f9400294 	ldr	x20, [x20]
    6938:	eb16029f 	cmp	x20, x22
    693c:	54fffd81 	b.ne	68ec <__pthread_init_static_tls+0x5c>
    6940:	910a42a0 	add	x0, x21, #0x290
    6944:	52800037 	mov	w23, #0x1                   	// #1
    6948:	91006016 	add	x22, x0, #0x18
    694c:	f9400c14 	ldr	x20, [x0,#24]
    6950:	eb16029f 	cmp	x20, x22
    6954:	540002c0 	b.eq	69ac <__pthread_init_static_tls+0x11c>
    6958:	f9422262 	ldr	x2, [x19,#1088]
    695c:	f9431a83 	ldr	x3, [x20,#1584]
    6960:	f9421e61 	ldr	x1, [x19,#1080]
    6964:	d37cec42 	lsl	x2, x2, #4
    6968:	8b020064 	add	x4, x3, x2
    696c:	9118c021 	add	x1, x1, #0x630
    6970:	8b010281 	add	x1, x20, x1
    6974:	f8226861 	str	x1, [x3,x2]
    6978:	39002097 	strb	w23, [x4,#8]
    697c:	aa0103e0 	mov	x0, x1
    6980:	f9420e62 	ldr	x2, [x19,#1048]
    6984:	f9420a61 	ldr	x1, [x19,#1040]
    6988:	97fffa36 	bl	5260 <mempcpy@plt>
    698c:	f9421263 	ldr	x3, [x19,#1056]
    6990:	52800001 	mov	w1, #0x0                   	// #0
    6994:	f9420e62 	ldr	x2, [x19,#1048]
    6998:	cb020062 	sub	x2, x3, x2
    699c:	97fff9bd 	bl	5090 <memset@plt>
    69a0:	f9400294 	ldr	x20, [x20]
    69a4:	eb16029f 	cmp	x20, x22
    69a8:	54fffd81 	b.ne	6958 <__pthread_init_static_tls+0xc8>
    69ac:	910a42a0 	add	x0, x21, #0x290
    69b0:	52800002 	mov	w2, #0x0                   	// #0
    69b4:	91004000 	add	x0, x0, #0x10
    69b8:	885f7c01 	ldxr	w1, [x0]
    69bc:	8803fc02 	stlxr	w3, w2, [x0]
    69c0:	35ffffc3 	cbnz	w3, 69b8 <__pthread_init_static_tls+0x128>
    69c4:	7100043f 	cmp	w1, #0x1
    69c8:	5400012c 	b.gt	69ec <__pthread_init_static_tls+0x15c>
    69cc:	a94153f3 	ldp	x19, x20, [sp,#16]
    69d0:	a9425bf5 	ldp	x21, x22, [sp,#32]
    69d4:	f9401bf7 	ldr	x23, [sp,#48]
    69d8:	a8c57bfd 	ldp	x29, x30, [sp],#80
    69dc:	d65f03c0 	ret
    69e0:	b9004fa2 	str	w2, [x29,#76]
    69e4:	94002361 	bl	f768 <__lll_lock_wait_private>
    69e8:	17ffffbb 	b	68d4 <__pthread_init_static_tls+0x44>
    69ec:	d2801021 	mov	x1, #0x81                  	// #129
    69f0:	d2800022 	mov	x2, #0x1                   	// #1
    69f4:	d2800003 	mov	x3, #0x0                   	// #0
    69f8:	d2800c48 	mov	x8, #0x62                  	// #98
    69fc:	d4000001 	svc	#0x0
    6a00:	a94153f3 	ldp	x19, x20, [sp,#16]
    6a04:	a9425bf5 	ldp	x21, x22, [sp,#32]
    6a08:	f9401bf7 	ldr	x23, [sp,#48]
    6a0c:	a8c57bfd 	ldp	x29, x30, [sp],#80
    6a10:	d65f03c0 	ret

0000000000006a14 <__wait_lookup_done>:
    6a14:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    6a18:	52800021 	mov	w1, #0x1                   	// #1
    6a1c:	910003fd 	mov	x29, sp
    6a20:	f9000bf3 	str	x19, [sp,#16]
    6a24:	d0000173 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
    6a28:	910a4260 	add	x0, x19, #0x290
    6a2c:	b9002fbf 	str	wzr, [x29,#44]
    6a30:	91004000 	add	x0, x0, #0x10
    6a34:	885ffc02 	ldaxr	w2, [x0]
    6a38:	6b1f005f 	cmp	w2, wzr
    6a3c:	54000061 	b.ne	6a48 <__wait_lookup_done+0x34>
    6a40:	88037c01 	stxr	w3, w1, [x0]
    6a44:	35ffff83 	cbnz	w3, 6a34 <__wait_lookup_done+0x20>
    6a48:	540004c1 	b.ne	6ae0 <__wait_lookup_done+0xcc>
    6a4c:	d0000147 	adrp	x7, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    6a50:	d53bd046 	mrs	x6, tpidr_el0
    6a54:	910960e7 	add	x7, x7, #0x258
    6a58:	d11bc0c6 	sub	x6, x6, #0x6f0
    6a5c:	52800029 	mov	w9, #0x1                   	// #1
    6a60:	5280004a 	mov	w10, #0x2                   	// #2
    6a64:	f8410ce4 	ldr	x4, [x7,#16]!
    6a68:	eb07009f 	cmp	x4, x7
    6a6c:	540000a1 	b.ne	6a80 <__wait_lookup_done+0x6c>
    6a70:	14000023 	b	6afc <__wait_lookup_done+0xe8>
    6a74:	f9400084 	ldr	x4, [x4]
    6a78:	eb07009f 	cmp	x4, x7
    6a7c:	54000400 	b.eq	6afc <__wait_lookup_done+0xe8>
    6a80:	d1030081 	sub	x1, x4, #0xc0
    6a84:	eb0100df 	cmp	x6, x1
    6a88:	54ffff60 	b.eq	6a74 <__wait_lookup_done+0x60>
    6a8c:	b8544082 	ldr	w2, [x4,#-188]
    6a90:	34ffff22 	cbz	w2, 6a74 <__wait_lookup_done+0x60>
    6a94:	b9002fa9 	str	w9, [x29,#44]
    6a98:	91001025 	add	x5, x1, #0x4
    6a9c:	2a0903e0 	mov	w0, w9
    6aa0:	885ffca1 	ldaxr	w1, [x5]
    6aa4:	6b00003f 	cmp	w1, w0
    6aa8:	54000061 	b.ne	6ab4 <__wait_lookup_done+0xa0>
    6aac:	88027caa 	stxr	w2, w10, [x5]
    6ab0:	35ffff82 	cbnz	w2, 6aa0 <__wait_lookup_done+0x8c>
    6ab4:	540001c1 	b.ne	6aec <__wait_lookup_done+0xd8>
    6ab8:	aa0503e0 	mov	x0, x5
    6abc:	d2801001 	mov	x1, #0x80                  	// #128
    6ac0:	d2800042 	mov	x2, #0x2                   	// #2
    6ac4:	d2800003 	mov	x3, #0x0                   	// #0
    6ac8:	d2800c48 	mov	x8, #0x62                  	// #98
    6acc:	d4000001 	svc	#0x0
    6ad0:	b8544080 	ldr	w0, [x4,#-188]
    6ad4:	7100081f 	cmp	w0, #0x2
    6ad8:	54ffff00 	b.eq	6ab8 <__wait_lookup_done+0xa4>
    6adc:	17ffffe6 	b	6a74 <__wait_lookup_done+0x60>
    6ae0:	b9002fa2 	str	w2, [x29,#44]
    6ae4:	94002321 	bl	f768 <__lll_lock_wait_private>
    6ae8:	17ffffd9 	b	6a4c <__wait_lookup_done+0x38>
    6aec:	b9002fa1 	str	w1, [x29,#44]
    6af0:	f9400084 	ldr	x4, [x4]
    6af4:	eb07009f 	cmp	x4, x7
    6af8:	54fffc41 	b.ne	6a80 <__wait_lookup_done+0x6c>
    6afc:	910a4260 	add	x0, x19, #0x290
    6b00:	52800029 	mov	w9, #0x1                   	// #1
    6b04:	91006007 	add	x7, x0, #0x18
    6b08:	5280004a 	mov	w10, #0x2                   	// #2
    6b0c:	f9400c04 	ldr	x4, [x0,#24]
    6b10:	eb07009f 	cmp	x4, x7
    6b14:	540000a1 	b.ne	6b28 <__wait_lookup_done+0x114>
    6b18:	14000020 	b	6b98 <__wait_lookup_done+0x184>
    6b1c:	f9400084 	ldr	x4, [x4]
    6b20:	eb07009f 	cmp	x4, x7
    6b24:	540003a0 	b.eq	6b98 <__wait_lookup_done+0x184>
    6b28:	d1030081 	sub	x1, x4, #0xc0
    6b2c:	eb0100df 	cmp	x6, x1
    6b30:	54ffff60 	b.eq	6b1c <__wait_lookup_done+0x108>
    6b34:	b8544082 	ldr	w2, [x4,#-188]
    6b38:	34ffff22 	cbz	w2, 6b1c <__wait_lookup_done+0x108>
    6b3c:	b9002fa9 	str	w9, [x29,#44]
    6b40:	91001025 	add	x5, x1, #0x4
    6b44:	2a0903e0 	mov	w0, w9
    6b48:	885ffca1 	ldaxr	w1, [x5]
    6b4c:	6b00003f 	cmp	w1, w0
    6b50:	54000061 	b.ne	6b5c <__wait_lookup_done+0x148>
    6b54:	88027caa 	stxr	w2, w10, [x5]
    6b58:	35ffff82 	cbnz	w2, 6b48 <__wait_lookup_done+0x134>
    6b5c:	54000161 	b.ne	6b88 <__wait_lookup_done+0x174>
    6b60:	aa0503e0 	mov	x0, x5
    6b64:	d2801001 	mov	x1, #0x80                  	// #128
    6b68:	d2800042 	mov	x2, #0x2                   	// #2
    6b6c:	d2800003 	mov	x3, #0x0                   	// #0
    6b70:	d2800c48 	mov	x8, #0x62                  	// #98
    6b74:	d4000001 	svc	#0x0
    6b78:	b8544080 	ldr	w0, [x4,#-188]
    6b7c:	7100081f 	cmp	w0, #0x2
    6b80:	54ffff00 	b.eq	6b60 <__wait_lookup_done+0x14c>
    6b84:	17ffffe6 	b	6b1c <__wait_lookup_done+0x108>
    6b88:	b9002fa1 	str	w1, [x29,#44]
    6b8c:	f9400084 	ldr	x4, [x4]
    6b90:	eb07009f 	cmp	x4, x7
    6b94:	54fffca1 	b.ne	6b28 <__wait_lookup_done+0x114>
    6b98:	910a4260 	add	x0, x19, #0x290
    6b9c:	52800002 	mov	w2, #0x0                   	// #0
    6ba0:	91004000 	add	x0, x0, #0x10
    6ba4:	885f7c01 	ldxr	w1, [x0]
    6ba8:	8803fc02 	stlxr	w3, w2, [x0]
    6bac:	35ffffc3 	cbnz	w3, 6ba4 <__wait_lookup_done+0x190>
    6bb0:	7100043f 	cmp	w1, #0x1
    6bb4:	5400008c 	b.gt	6bc4 <__wait_lookup_done+0x1b0>
    6bb8:	f9400bf3 	ldr	x19, [sp,#16]
    6bbc:	a8c37bfd 	ldp	x29, x30, [sp],#48
    6bc0:	d65f03c0 	ret
    6bc4:	d2801021 	mov	x1, #0x81                  	// #129
    6bc8:	d2800022 	mov	x2, #0x1                   	// #1
    6bcc:	d2800003 	mov	x3, #0x0                   	// #0
    6bd0:	d2800c48 	mov	x8, #0x62                  	// #98
    6bd4:	d4000001 	svc	#0x0
    6bd8:	f9400bf3 	ldr	x19, [sp,#16]
    6bdc:	a8c37bfd 	ldp	x29, x30, [sp],#48
    6be0:	d65f03c0 	ret

0000000000006be4 <__find_in_stack_list>:
    6be4:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    6be8:	910003fd 	mov	x29, sp
    6bec:	a90153f3 	stp	x19, x20, [sp,#16]
    6bf0:	d0000174 	adrp	x20, 34000 <__GI___pthread_keys+0x3d78>
    6bf4:	aa0003f3 	mov	x19, x0
    6bf8:	910a4281 	add	x1, x20, #0x290
    6bfc:	b9002fbf 	str	wzr, [x29,#44]
    6c00:	91004020 	add	x0, x1, #0x10
    6c04:	52800021 	mov	w1, #0x1                   	// #1
    6c08:	885ffc02 	ldaxr	w2, [x0]
    6c0c:	6b1f005f 	cmp	w2, wzr
    6c10:	54000061 	b.ne	6c1c <__find_in_stack_list+0x38>
    6c14:	88037c01 	stxr	w3, w1, [x0]
    6c18:	35ffff83 	cbnz	w3, 6c08 <__find_in_stack_list+0x24>
    6c1c:	54000421 	b.ne	6ca0 <__find_in_stack_list+0xbc>
    6c20:	d0000143 	adrp	x3, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    6c24:	91096063 	add	x3, x3, #0x258
    6c28:	f8410c61 	ldr	x1, [x3,#16]!
    6c2c:	eb03003f 	cmp	x1, x3
    6c30:	54000160 	b.eq	6c5c <__find_in_stack_list+0x78>
    6c34:	d1030020 	sub	x0, x1, #0xc0
    6c38:	eb00027f 	cmp	x19, x0
    6c3c:	54000081 	b.ne	6c4c <__find_in_stack_list+0x68>
    6c40:	1400001b 	b	6cac <__find_in_stack_list+0xc8>
    6c44:	eb02027f 	cmp	x19, x2
    6c48:	54000320 	b.eq	6cac <__find_in_stack_list+0xc8>
    6c4c:	f9400021 	ldr	x1, [x1]
    6c50:	eb03003f 	cmp	x1, x3
    6c54:	d1030022 	sub	x2, x1, #0xc0
    6c58:	54ffff61 	b.ne	6c44 <__find_in_stack_list+0x60>
    6c5c:	910a4280 	add	x0, x20, #0x290
    6c60:	91006002 	add	x2, x0, #0x18
    6c64:	f9400c00 	ldr	x0, [x0,#24]
    6c68:	eb02001f 	cmp	x0, x2
    6c6c:	54000160 	b.eq	6c98 <__find_in_stack_list+0xb4>
    6c70:	d1030001 	sub	x1, x0, #0xc0
    6c74:	eb01027f 	cmp	x19, x1
    6c78:	54000081 	b.ne	6c88 <__find_in_stack_list+0xa4>
    6c7c:	1400000d 	b	6cb0 <__find_in_stack_list+0xcc>
    6c80:	eb01027f 	cmp	x19, x1
    6c84:	54000160 	b.eq	6cb0 <__find_in_stack_list+0xcc>
    6c88:	f9400000 	ldr	x0, [x0]
    6c8c:	eb02001f 	cmp	x0, x2
    6c90:	d1030001 	sub	x1, x0, #0xc0
    6c94:	54ffff61 	b.ne	6c80 <__find_in_stack_list+0x9c>
    6c98:	d2800013 	mov	x19, #0x0                   	// #0
    6c9c:	14000005 	b	6cb0 <__find_in_stack_list+0xcc>
    6ca0:	b9002fa2 	str	w2, [x29,#44]
    6ca4:	940022b1 	bl	f768 <__lll_lock_wait_private>
    6ca8:	17ffffde 	b	6c20 <__find_in_stack_list+0x3c>
    6cac:	b4fffd93 	cbz	x19, 6c5c <__find_in_stack_list+0x78>
    6cb0:	910a4280 	add	x0, x20, #0x290
    6cb4:	52800002 	mov	w2, #0x0                   	// #0
    6cb8:	91004000 	add	x0, x0, #0x10
    6cbc:	885f7c01 	ldxr	w1, [x0]
    6cc0:	8803fc02 	stlxr	w3, w2, [x0]
    6cc4:	35ffffc3 	cbnz	w3, 6cbc <__find_in_stack_list+0xd8>
    6cc8:	7100043f 	cmp	w1, #0x1
    6ccc:	540000ac 	b.gt	6ce0 <__find_in_stack_list+0xfc>
    6cd0:	aa1303e0 	mov	x0, x19
    6cd4:	a94153f3 	ldp	x19, x20, [sp,#16]
    6cd8:	a8c37bfd 	ldp	x29, x30, [sp],#48
    6cdc:	d65f03c0 	ret
    6ce0:	d2801021 	mov	x1, #0x81                  	// #129
    6ce4:	d2800022 	mov	x2, #0x1                   	// #1
    6ce8:	d2800003 	mov	x3, #0x0                   	// #0
    6cec:	d2800c48 	mov	x8, #0x62                  	// #98
    6cf0:	d4000001 	svc	#0x0
    6cf4:	aa1303e0 	mov	x0, x19
    6cf8:	a94153f3 	ldp	x19, x20, [sp,#16]
    6cfc:	a8c37bfd 	ldp	x29, x30, [sp],#48
    6d00:	d65f03c0 	ret

0000000000006d04 <__nptl_deallocate_tsd>:
    6d04:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    6d08:	910003fd 	mov	x29, sp
    6d0c:	a90363f7 	stp	x23, x24, [sp,#48]
    6d10:	d53bd058 	mrs	x24, tpidr_el0
    6d14:	a9046bf9 	stp	x25, x26, [sp,#64]
    6d18:	d11bc319 	sub	x25, x24, #0x6f0
    6d1c:	a90153f3 	stp	x19, x20, [sp,#16]
    6d20:	a9025bf5 	stp	x21, x22, [sp,#32]
    6d24:	39504320 	ldrb	w0, [x25,#1040]
    6d28:	a90573fb 	stp	x27, x28, [sp,#80]
    6d2c:	340006e0 	cbz	w0, 6e08 <__nptl_deallocate_tsd+0x104>
    6d30:	d280009a 	mov	x26, #0x4                   	// #4
    6d34:	d10f831c 	sub	x28, x24, #0x3e0
    6d38:	d000015b 	adrp	x27, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    6d3c:	3910433f 	strb	wzr, [x25,#1040]
    6d40:	aa1c03f7 	mov	x23, x28
    6d44:	91124375 	add	x21, x27, #0x490
    6d48:	d2800416 	mov	x22, #0x20                  	// #32
    6d4c:	f94002f3 	ldr	x19, [x23]
    6d50:	b40002d3 	cbz	x19, 6da8 <__nptl_deallocate_tsd+0xa4>
    6d54:	91002273 	add	x19, x19, #0x8
    6d58:	d10802b4 	sub	x20, x21, #0x200
    6d5c:	14000005 	b	6d70 <__nptl_deallocate_tsd+0x6c>
    6d60:	91004294 	add	x20, x20, #0x10
    6d64:	91004273 	add	x19, x19, #0x10
    6d68:	eb15029f 	cmp	x20, x21
    6d6c:	540001e0 	b.eq	6da8 <__nptl_deallocate_tsd+0xa4>
    6d70:	f9400260 	ldr	x0, [x19]
    6d74:	b4ffff60 	cbz	x0, 6d60 <__nptl_deallocate_tsd+0x5c>
    6d78:	f85f8282 	ldr	x2, [x20,#-8]
    6d7c:	f85f8263 	ldr	x3, [x19,#-8]
    6d80:	f900027f 	str	xzr, [x19]
    6d84:	eb02007f 	cmp	x3, x2
    6d88:	54fffec1 	b.ne	6d60 <__nptl_deallocate_tsd+0x5c>
    6d8c:	f9400282 	ldr	x2, [x20]
    6d90:	b4fffe82 	cbz	x2, 6d60 <__nptl_deallocate_tsd+0x5c>
    6d94:	d63f0040 	blr	x2
    6d98:	91004294 	add	x20, x20, #0x10
    6d9c:	eb15029f 	cmp	x20, x21
    6da0:	91004273 	add	x19, x19, #0x10
    6da4:	54fffe61 	b.ne	6d70 <__nptl_deallocate_tsd+0x6c>
    6da8:	910082d6 	add	x22, x22, #0x20
    6dac:	910022f7 	add	x23, x23, #0x8
    6db0:	f11082df 	cmp	x22, #0x420
    6db4:	910802b5 	add	x21, x21, #0x200
    6db8:	54fffca1 	b.ne	6d4c <__nptl_deallocate_tsd+0x48>
    6dbc:	39504320 	ldrb	w0, [x25,#1040]
    6dc0:	340000e0 	cbz	w0, 6ddc <__nptl_deallocate_tsd+0xd8>
    6dc4:	f100075a 	subs	x26, x26, #0x1
    6dc8:	54fffba1 	b.ne	6d3c <__nptl_deallocate_tsd+0x38>
    6dcc:	91044320 	add	x0, x25, #0x110
    6dd0:	2a1a03e1 	mov	w1, w26
    6dd4:	d2804002 	mov	x2, #0x200                 	// #512
    6dd8:	97fff8ae 	bl	5090 <memset@plt>
    6ddc:	d10f6313 	sub	x19, x24, #0x3d8
    6de0:	d10b8314 	sub	x20, x24, #0x2e0
    6de4:	f9400261 	ldr	x1, [x19]
    6de8:	aa0103e0 	mov	x0, x1
    6dec:	b4000061 	cbz	x1, 6df8 <__nptl_deallocate_tsd+0xf4>
    6df0:	97fff90c 	bl	5220 <free@plt>
    6df4:	f900027f 	str	xzr, [x19]
    6df8:	91002273 	add	x19, x19, #0x8
    6dfc:	eb14027f 	cmp	x19, x20
    6e00:	54ffff21 	b.ne	6de4 <__nptl_deallocate_tsd+0xe0>
    6e04:	3910433f 	strb	wzr, [x25,#1040]
    6e08:	a94153f3 	ldp	x19, x20, [sp,#16]
    6e0c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    6e10:	a94363f7 	ldp	x23, x24, [sp,#48]
    6e14:	a9446bf9 	ldp	x25, x26, [sp,#64]
    6e18:	a94573fb 	ldp	x27, x28, [sp,#80]
    6e1c:	a8c67bfd 	ldp	x29, x30, [sp],#96
    6e20:	d65f03c0 	ret

0000000000006e24 <__free_tcb>:
    6e24:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    6e28:	91042002 	add	x2, x0, #0x108
    6e2c:	910003fd 	mov	x29, sp
    6e30:	f9000bf3 	str	x19, [sp,#16]
    6e34:	b9410801 	ldr	w1, [x0,#264]
    6e38:	b9002fa1 	str	w1, [x29,#44]
    6e3c:	321b0024 	orr	w4, w1, #0x20
    6e40:	2a0103e3 	mov	w3, w1
    6e44:	885ffc45 	ldaxr	w5, [x2]
    6e48:	6b0300bf 	cmp	w5, w3
    6e4c:	54000061 	b.ne	6e58 <__free_tcb+0x34>
    6e50:	88067c44 	stxr	w6, w4, [x2]
    6e54:	35ffff86 	cbnz	w6, 6e44 <__free_tcb+0x20>
    6e58:	54fffee1 	b.ne	6e34 <__free_tcb+0x10>
    6e5c:	372800c1 	tbnz	w1, #5, 6e74 <__free_tcb+0x50>
    6e60:	aa0003f3 	mov	x19, x0
    6e64:	f9425800 	ldr	x0, [x0,#1200]
    6e68:	b50000c0 	cbnz	x0, 6e80 <__free_tcb+0x5c>
    6e6c:	aa1303e0 	mov	x0, x19
    6e70:	97fffb81 	bl	5c74 <__deallocate_stack>
    6e74:	f9400bf3 	ldr	x19, [sp,#16]
    6e78:	a8c37bfd 	ldp	x29, x30, [sp],#48
    6e7c:	d65f03c0 	ret
    6e80:	f9025a7f 	str	xzr, [x19,#1200]
    6e84:	97fff8e7 	bl	5220 <free@plt>
    6e88:	17fffff9 	b	6e6c <__free_tcb+0x48>

0000000000006e8c <start_thread>:
    6e8c:	a9ae7bfd 	stp	x29, x30, [sp,#-288]!
    6e90:	9112e002 	add	x2, x0, #0x4b8
    6e94:	b0000141 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    6e98:	f947cc21 	ldr	x1, [x1,#3992]
    6e9c:	910003fd 	mov	x29, sp
    6ea0:	a90153f3 	stp	x19, x20, [sp,#16]
    6ea4:	aa0003f3 	mov	x19, x0
    6ea8:	f90013f5 	str	x21, [sp,#32]
    6eac:	f9001fa0 	str	x0, [x29,#56]
    6eb0:	d53bd040 	mrs	x0, tpidr_el0
    6eb4:	f8216802 	str	x2, [x0,x1]
    6eb8:	97fff8d6 	bl	5210 <__ctype_init@plt>
    6ebc:	91107260 	add	x0, x19, #0x41c
    6ec0:	52800002 	mov	w2, #0x0                   	// #0
    6ec4:	885ffc01 	ldaxr	w1, [x0]
    6ec8:	88037c02 	stxr	w3, w2, [x0]
    6ecc:	35ffffc3 	cbnz	w3, 6ec4 <start_thread+0x38>
    6ed0:	3100083f 	cmn	w1, #0x2
    6ed4:	54000e00 	b.eq	7094 <start_thread+0x208>
    6ed8:	f9401fa2 	ldr	x2, [x29,#56]
    6edc:	d2800301 	mov	x1, #0x18                  	// #24
    6ee0:	d2800c68 	mov	x8, #0x63                  	// #99
    6ee4:	91038040 	add	x0, x2, #0xe0
    6ee8:	d4000001 	svc	#0x0
    6eec:	b9441440 	ldr	w0, [x2,#1044]
    6ef0:	37100880 	tbnz	w0, #2, 7000 <start_thread+0x174>
    6ef4:	910123a0 	add	x0, x29, #0x48
    6ef8:	f90083bf 	str	xzr, [x29,#256]
    6efc:	f90087bf 	str	xzr, [x29,#264]
    6f00:	97fff828 	bl	4fa0 <_setjmp@plt>
    6f04:	2a0003f3 	mov	w19, w0
    6f08:	35000160 	cbnz	w0, 6f34 <start_thread+0xa8>
    6f0c:	f9401fa2 	ldr	x2, [x29,#56]
    6f10:	910123a1 	add	x1, x29, #0x48
    6f14:	39504c40 	ldrb	w0, [x2,#1043]
    6f18:	f9008041 	str	x1, [x2,#256]
    6f1c:	350008e0 	cbnz	w0, 7038 <start_thread+0x1ac>
    6f20:	f9401fb3 	ldr	x19, [x29,#56]
    6f24:	f9421e61 	ldr	x1, [x19,#1080]
    6f28:	f9422260 	ldr	x0, [x19,#1088]
    6f2c:	d63f0020 	blr	x1
    6f30:	f9021660 	str	x0, [x19,#1064]
    6f34:	97fff873 	bl	5100 <__call_tls_dtors@plt>
    6f38:	97ffff73 	bl	6d04 <__nptl_deallocate_tsd>
    6f3c:	97fff879 	bl	5120 <__libc_thread_freeres@plt>
    6f40:	d0000140 	adrp	x0, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    6f44:	91094000 	add	x0, x0, #0x250
    6f48:	885ffc01 	ldaxr	w1, [x0]
    6f4c:	51000422 	sub	w2, w1, #0x1
    6f50:	88037c02 	stxr	w3, w2, [x0]
    6f54:	35ffffa3 	cbnz	w3, 6f48 <start_thread+0xbc>
    6f58:	7100043f 	cmp	w1, #0x1
    6f5c:	54000ae0 	b.eq	70b8 <start_thread+0x22c>
    6f60:	f9401fa0 	ldr	x0, [x29,#56]
    6f64:	39504400 	ldrb	w0, [x0,#1041]
    6f68:	35000c00 	cbnz	w0, 70e8 <start_thread+0x25c>
    6f6c:	f9401fa0 	ldr	x0, [x29,#56]
    6f70:	91042001 	add	x1, x0, #0x108
    6f74:	f9401fa0 	ldr	x0, [x29,#56]
    6f78:	b9410800 	ldr	w0, [x0,#264]
    6f7c:	b90043a0 	str	w0, [x29,#64]
    6f80:	321c0003 	orr	w3, w0, #0x10
    6f84:	885ffc22 	ldaxr	w2, [x1]
    6f88:	6b00005f 	cmp	w2, w0
    6f8c:	54000061 	b.ne	6f98 <start_thread+0x10c>
    6f90:	88047c23 	stxr	w4, w3, [x1]
    6f94:	35ffff84 	cbnz	w4, 6f84 <start_thread+0xf8>
    6f98:	1a9f17e0 	cset	w0, eq
    6f9c:	35000040 	cbnz	w0, 6fa4 <start_thread+0x118>
    6fa0:	b90043a2 	str	w2, [x29,#64]
    6fa4:	34fffe80 	cbz	w0, 6f74 <start_thread+0xe8>
    6fa8:	97fff816 	bl	5000 <__getpagesize@plt>
    6fac:	f9401fa1 	ldr	x1, [x29,#56]
    6fb0:	f9424822 	ldr	x2, [x1,#1168]
    6fb4:	51000401 	sub	w1, w0, #0x1
    6fb8:	cb0203a0 	sub	x0, x29, x2
    6fbc:	93407c21 	sxtw	x1, w1
    6fc0:	8a210001 	bic	x1, x0, x1
    6fc4:	f140803f 	cmp	x1, #0x20, lsl #12
    6fc8:	54000f88 	b.hi	71b8 <start_thread+0x32c>
    6fcc:	f9401fa1 	ldr	x1, [x29,#56]
    6fd0:	f9421020 	ldr	x0, [x1,#1056]
    6fd4:	eb00003f 	cmp	x1, x0
    6fd8:	54000ea0 	b.eq	71ac <start_thread+0x320>
    6fdc:	f9401fa0 	ldr	x0, [x29,#56]
    6fe0:	b9410800 	ldr	w0, [x0,#264]
    6fe4:	373009a0 	tbnz	w0, #6, 7118 <start_thread+0x28c>
    6fe8:	d2800002 	mov	x2, #0x0                   	// #0
    6fec:	d2800ba1 	mov	x1, #0x5d                  	// #93
    6ff0:	aa0203e0 	mov	x0, x2
    6ff4:	aa0103e8 	mov	x8, x1
    6ff8:	d4000001 	svc	#0x0
    6ffc:	17fffffd 	b	6ff0 <start_thread+0x164>
    7000:	910123b3 	add	x19, x29, #0x48
    7004:	52800001 	mov	w1, #0x0                   	// #0
    7008:	aa1303e0 	mov	x0, x19
    700c:	d2801002 	mov	x2, #0x80                  	// #128
    7010:	97fff820 	bl	5090 <memset@plt>
    7014:	d2b00001 	mov	x1, #0x80000000            	// #2147483648
    7018:	d2800020 	mov	x0, #0x1                   	// #1
    701c:	f90027a1 	str	x1, [x29,#72]
    7020:	d2800002 	mov	x2, #0x0                   	// #0
    7024:	aa1303e1 	mov	x1, x19
    7028:	d2800103 	mov	x3, #0x8                   	// #8
    702c:	d28010e8 	mov	x8, #0x87                  	// #135
    7030:	d4000001 	svc	#0x0
    7034:	17ffffb0 	b	6ef4 <start_thread+0x68>
    7038:	94002178 	bl	f618 <__pthread_enable_asynccancel>
    703c:	2a0003f5 	mov	w21, w0
    7040:	f9401fa0 	ldr	x0, [x29,#56]
    7044:	52800021 	mov	w1, #0x1                   	// #1
    7048:	b90043b3 	str	w19, [x29,#64]
    704c:	91106014 	add	x20, x0, #0x418
    7050:	885ffe80 	ldaxr	w0, [x20]
    7054:	6b13001f 	cmp	w0, w19
    7058:	54000061 	b.ne	7064 <start_thread+0x1d8>
    705c:	88027e81 	stxr	w2, w1, [x20]
    7060:	35ffff82 	cbnz	w2, 7050 <start_thread+0x1c4>
    7064:	1a9f17e1 	cset	w1, eq
    7068:	34000221 	cbz	w1, 70ac <start_thread+0x220>
    706c:	340002a1 	cbz	w1, 70c0 <start_thread+0x234>
    7070:	52800001 	mov	w1, #0x0                   	// #0
    7074:	885f7e80 	ldxr	w0, [x20]
    7078:	8802fe81 	stlxr	w2, w1, [x20]
    707c:	35ffffc2 	cbnz	w2, 7074 <start_thread+0x1e8>
    7080:	7100041f 	cmp	w0, #0x1
    7084:	5400024c 	b.gt	70cc <start_thread+0x240>
    7088:	2a1503e0 	mov	w0, w21
    708c:	94002193 	bl	f6d8 <__pthread_disable_asynccancel>
    7090:	17ffffa4 	b	6f20 <start_thread+0x94>
    7094:	d2801021 	mov	x1, #0x81                  	// #129
    7098:	d2800022 	mov	x2, #0x1                   	// #1
    709c:	d2800003 	mov	x3, #0x0                   	// #0
    70a0:	d2800c48 	mov	x8, #0x62                  	// #98
    70a4:	d4000001 	svc	#0x0
    70a8:	17ffff8c 	b	6ed8 <start_thread+0x4c>
    70ac:	b90043a0 	str	w0, [x29,#64]
    70b0:	35fffe01 	cbnz	w1, 7070 <start_thread+0x1e4>
    70b4:	14000003 	b	70c0 <start_thread+0x234>
    70b8:	52800000 	mov	w0, #0x0                   	// #0
    70bc:	97fff7b5 	bl	4f90 <exit@plt>
    70c0:	aa1403e0 	mov	x0, x20
    70c4:	940021a9 	bl	f768 <__lll_lock_wait_private>
    70c8:	17ffffea 	b	7070 <start_thread+0x1e4>
    70cc:	aa1403e0 	mov	x0, x20
    70d0:	d2801021 	mov	x1, #0x81                  	// #129
    70d4:	d2800022 	mov	x2, #0x1                   	// #1
    70d8:	d2800003 	mov	x3, #0x0                   	// #0
    70dc:	d2800c48 	mov	x8, #0x62                  	// #98
    70e0:	d4000001 	svc	#0x0
    70e4:	17ffffe9 	b	7088 <start_thread+0x1fc>
    70e8:	b0000163 	adrp	x3, 34000 <__GI___pthread_keys+0x3d78>
    70ec:	f9401fa0 	ldr	x0, [x29,#56]
    70f0:	910a4063 	add	x3, x3, #0x290
    70f4:	b9444801 	ldr	w1, [x0,#1096]
    70f8:	b9402860 	ldr	w0, [x3,#40]
    70fc:	2a000020 	orr	w0, w1, w0
    7100:	3647f360 	tbz	w0, #8, 6f6c <start_thread+0xe0>
    7104:	f9401fa0 	ldr	x0, [x29,#56]
    7108:	f9423000 	ldr	x0, [x0,#1120]
    710c:	b4000280 	cbz	x0, 715c <start_thread+0x2d0>
    7110:	97fffa5e 	bl	5a88 <__GI___nptl_death_event>
    7114:	17ffff96 	b	6f6c <start_thread+0xe0>
    7118:	f9401fa0 	ldr	x0, [x29,#56]
    711c:	d2801006 	mov	x6, #0x80                  	// #128
    7120:	d2800004 	mov	x4, #0x0                   	// #0
    7124:	d2800c45 	mov	x5, #0x62                  	// #98
    7128:	91107007 	add	x7, x0, #0x41c
    712c:	aa0703e0 	mov	x0, x7
    7130:	aa0603e1 	mov	x1, x6
    7134:	aa0403e2 	mov	x2, x4
    7138:	aa0403e3 	mov	x3, x4
    713c:	aa0503e8 	mov	x8, x5
    7140:	d4000001 	svc	#0x0
    7144:	f9401fa0 	ldr	x0, [x29,#56]
    7148:	b9410800 	ldr	w0, [x0,#264]
    714c:	3737ff00 	tbnz	w0, #6, 712c <start_thread+0x2a0>
    7150:	f9401fa0 	ldr	x0, [x29,#56]
    7154:	b9041c04 	str	w4, [x0,#1052]
    7158:	17ffffa4 	b	6fe8 <start_thread+0x15c>
    715c:	f9401fa4 	ldr	x4, [x29,#56]
    7160:	aa0403e0 	mov	x0, x4
    7164:	f9022c04 	str	x4, [x0,#1112]
    7168:	52800120 	mov	w0, #0x9                   	// #9
    716c:	b9045080 	str	w0, [x4,#1104]
    7170:	aa0303e1 	mov	x1, x3
    7174:	f9401fa2 	ldr	x2, [x29,#56]
    7178:	f8430c20 	ldr	x0, [x1,#48]!
    717c:	f9023040 	str	x0, [x2,#1120]
    7180:	f90023a0 	str	x0, [x29,#64]
    7184:	c85ffc22 	ldaxr	x2, [x1]
    7188:	eb00005f 	cmp	x2, x0
    718c:	54000061 	b.ne	7198 <start_thread+0x30c>
    7190:	c8057c24 	stxr	w5, x4, [x1]
    7194:	35ffff85 	cbnz	w5, 7184 <start_thread+0x2f8>
    7198:	1a9f17e0 	cset	w0, eq
    719c:	35000040 	cbnz	w0, 71a4 <start_thread+0x318>
    71a0:	f90023a2 	str	x2, [x29,#64]
    71a4:	34fffe60 	cbz	w0, 7170 <start_thread+0x2e4>
    71a8:	17ffffda 	b	7110 <start_thread+0x284>
    71ac:	f9401fa0 	ldr	x0, [x29,#56]
    71b0:	97ffff1d 	bl	6e24 <__free_tcb>
    71b4:	17ffff8d 	b	6fe8 <start_thread+0x15c>
    71b8:	aa0203e0 	mov	x0, x2
    71bc:	d1408021 	sub	x1, x1, #0x20, lsl #12
    71c0:	52800082 	mov	w2, #0x4                   	// #4
    71c4:	97fff843 	bl	52d0 <__madvise@plt>
    71c8:	17ffff81 	b	6fcc <start_thread+0x140>

00000000000071cc <pthread_create@@GLIBC_2.17>:
    71cc:	a9b17bfd 	stp	x29, x30, [sp,#-240]!
    71d0:	910003fd 	mov	x29, sp
    71d4:	6d0627e8 	stp	d8, d9, [sp,#96]
    71d8:	a90363f7 	stp	x23, x24, [sp,#48]
    71dc:	6d072fea 	stp	d10, d11, [sp,#112]
    71e0:	a90573fb 	stp	x27, x28, [sp,#80]
    71e4:	6d0837ec 	stp	d12, d13, [sp,#128]
    71e8:	a90153f3 	stp	x19, x20, [sp,#16]
    71ec:	a9025bf5 	stp	x21, x22, [sp,#32]
    71f0:	a9046bf9 	stp	x25, x26, [sp,#64]
    71f4:	aa0103f8 	mov	x24, x1
    71f8:	aa0203fb 	mov	x27, x2
    71fc:	9e67000b 	fmov	d11, x0
    7200:	9e67006c 	fmov	d12, x3
    7204:	1e2703e8 	fmov	s8, wzr
    7208:	b40038a1 	cbz	x1, 791c <pthread_create@@GLIBC_2.17+0x750>
    720c:	97fff77d 	bl	5000 <__getpagesize@plt>
    7210:	f9401314 	ldr	x20, [x24,#32]
    7214:	51000400 	sub	w0, w0, #0x1
    7218:	93407c13 	sxtw	x19, w0
    721c:	b4001d14 	cbz	x20, 75bc <pthread_create@@GLIBC_2.17+0x3f0>
    7220:	b9400b00 	ldr	w0, [x24,#8]
    7224:	37185020 	tbnz	w0, #3, 7c28 <pthread_create@@GLIBC_2.17+0xa5c>
    7228:	9000014a 	adrp	x10, 2f000 <__FRAME_END__+0x18e30>
    722c:	b000016b 	adrp	x11, 34000 <__GI___pthread_keys+0x3d78>
    7230:	b000016c 	adrp	x12, 34000 <__GI___pthread_keys+0x3d78>
    7234:	f9400b19 	ldr	x25, [x24,#16]
    7238:	aa3303e0 	mvn	x0, x19
    723c:	52800095 	mov	w21, #0x4                   	// #4
    7240:	f947ed41 	ldr	x1, [x10,#4056]
    7244:	8b190279 	add	x25, x19, x25
    7248:	f9419162 	ldr	x2, [x11,#800]
    724c:	8a190019 	and	x25, x0, x25
    7250:	f9418d89 	ldr	x9, [x12,#792]
    7254:	528000fa 	mov	w26, #0x7                   	// #7
    7258:	b94fa021 	ldr	w1, [x1,#4000]
    725c:	91200042 	add	x2, x2, #0x800
    7260:	8b130053 	add	x19, x2, x19
    7264:	8a29029c 	bic	x28, x20, x9
    7268:	72000021 	ands	w1, w1, #0x1
    726c:	8b190273 	add	x19, x19, x25
    7270:	8a000273 	and	x19, x19, x0
    7274:	1a9f12b5 	csel	w21, w21, wzr, ne
    7278:	52800060 	mov	w0, #0x3                   	// #3
    727c:	6b1f003f 	cmp	w1, wzr
    7280:	1a801340 	csel	w0, w26, w0, ne
    7284:	eb13039f 	cmp	x28, x19
    7288:	1e27000d 	fmov	s13, w0
    728c:	54001d03 	b.cc	762c <pthread_create@@GLIBC_2.17+0x460>
    7290:	b0000174 	adrp	x20, 34000 <__GI___pthread_keys+0x3d78>
    7294:	b900b3bf 	str	wzr, [x29,#176]
    7298:	910a4280 	add	x0, x20, #0x290
    729c:	52800021 	mov	w1, #0x1                   	// #1
    72a0:	91004000 	add	x0, x0, #0x10
    72a4:	885ffc02 	ldaxr	w2, [x0]
    72a8:	6b1f005f 	cmp	w2, wzr
    72ac:	54000061 	b.ne	72b8 <pthread_create@@GLIBC_2.17+0xec>
    72b0:	88037c01 	stxr	w3, w1, [x0]
    72b4:	35ffff83 	cbnz	w3, 72a4 <pthread_create@@GLIBC_2.17+0xd8>
    72b8:	54001581 	b.ne	7568 <pthread_create@@GLIBC_2.17+0x39c>
    72bc:	b0000156 	adrp	x22, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    72c0:	910962c7 	add	x7, x22, #0x258
    72c4:	f9412ec4 	ldr	x4, [x22,#600]
    72c8:	eb07009f 	cmp	x4, x7
    72cc:	54002460 	b.eq	7758 <pthread_create@@GLIBC_2.17+0x58c>
    72d0:	d2800013 	mov	x19, #0x0                   	// #0
    72d4:	14000004 	b	72e4 <pthread_create@@GLIBC_2.17+0x118>
    72d8:	f9400084 	ldr	x4, [x4]
    72dc:	eb07009f 	cmp	x4, x7
    72e0:	54000200 	b.eq	7320 <pthread_create@@GLIBC_2.17+0x154>
    72e4:	d1030085 	sub	x5, x4, #0xc0
    72e8:	b940d0a6 	ldr	w6, [x5,#208]
    72ec:	6b1f00df 	cmp	w6, wzr
    72f0:	54ffff4c 	b.gt	72d8 <pthread_create@@GLIBC_2.17+0x10c>
    72f4:	f9424ca0 	ldr	x0, [x5,#1176]
    72f8:	eb00039f 	cmp	x28, x0
    72fc:	54fffee8 	b.hi	72d8 <pthread_create@@GLIBC_2.17+0x10c>
    7300:	54004240 	b.eq	7b48 <pthread_create@@GLIBC_2.17+0x97c>
    7304:	b4001993 	cbz	x19, 7634 <pthread_create@@GLIBC_2.17+0x468>
    7308:	f9424e61 	ldr	x1, [x19,#1176]
    730c:	f9400084 	ldr	x4, [x4]
    7310:	eb01001f 	cmp	x0, x1
    7314:	9a852273 	csel	x19, x19, x5, cs
    7318:	eb07009f 	cmp	x4, x7
    731c:	54fffe41 	b.ne	72e4 <pthread_create@@GLIBC_2.17+0x118>
    7320:	b40021d3 	cbz	x19, 7758 <pthread_create@@GLIBC_2.17+0x58c>
    7324:	f9424e60 	ldr	x0, [x19,#1176]
    7328:	eb1c081f 	cmp	x0, x28, lsl #2
    732c:	54002168 	b.hi	7758 <pthread_create@@GLIBC_2.17+0x58c>
    7330:	9103027a 	add	x26, x19, #0xc0
    7334:	12800000 	mov	w0, #0xffffffff            	// #-1
    7338:	f9014a9a 	str	x26, [x20,#656]
    733c:	910962d6 	add	x22, x22, #0x258
    7340:	b9041e60 	str	w0, [x19,#1052]
    7344:	aa1603e2 	mov	x2, x22
    7348:	b2400344 	orr	x4, x26, #0x1
    734c:	910a4281 	add	x1, x20, #0x290
    7350:	d5033bbf 	dmb	ish
    7354:	f9406263 	ldr	x3, [x19,#192]
    7358:	f9406665 	ldr	x5, [x19,#200]
    735c:	91004020 	add	x0, x1, #0x10
    7360:	f9000465 	str	x5, [x3,#8]
    7364:	f9406665 	ldr	x5, [x19,#200]
    7368:	f90000a3 	str	x3, [x5]
    736c:	d5033bbf 	dmb	ish
    7370:	f9014a84 	str	x4, [x20,#656]
    7374:	d5033bbf 	dmb	ish
    7378:	f8410c43 	ldr	x3, [x2,#16]!
    737c:	f9006263 	str	x3, [x19,#192]
    7380:	f9006662 	str	x2, [x19,#200]
    7384:	f900047a 	str	x26, [x3,#8]
    7388:	d5033bbf 	dmb	ish
    738c:	f9000ada 	str	x26, [x22,#16]
    7390:	d5033bbf 	dmb	ish
    7394:	f9400422 	ldr	x2, [x1,#8]
    7398:	f9424e63 	ldr	x3, [x19,#1176]
    739c:	f9014a9f 	str	xzr, [x20,#656]
    73a0:	cb030042 	sub	x2, x2, x3
    73a4:	f9000422 	str	x2, [x1,#8]
    73a8:	52800002 	mov	w2, #0x0                   	// #0
    73ac:	885f7c01 	ldxr	w1, [x0]
    73b0:	8803fc02 	stlxr	w3, w2, [x0]
    73b4:	35ffffc3 	cbnz	w3, 73ac <pthread_create@@GLIBC_2.17+0x1e0>
    73b8:	7100043f 	cmp	w1, #0x1
    73bc:	540043ac 	b.gt	7c30 <pthread_create@@GLIBC_2.17+0xa64>
    73c0:	f9437a77 	ldr	x23, [x19,#1776]
    73c4:	d2800016 	mov	x22, #0x0                   	// #0
    73c8:	f9424e60 	ldr	x0, [x19,#1176]
    73cc:	b9010a7f 	str	wzr, [x19,#264]
    73d0:	910042f5 	add	x21, x23, #0x10
    73d4:	f9007e76 	str	x22, [x19,#248]
    73d8:	f9023276 	str	x22, [x19,#1120]
    73dc:	9e670009 	fmov	d9, x0
    73e0:	f9424a60 	ldr	x0, [x19,#1168]
    73e4:	f85f02e5 	ldr	x5, [x23,#-16]
    73e8:	eb0502df 	cmp	x22, x5
    73ec:	9e67000a 	fmov	d10, x0
    73f0:	54000182 	b.cs	7420 <pthread_create@@GLIBC_2.17+0x254>
    73f4:	394022a4 	ldrb	w4, [x21,#8]
    73f8:	910006d6 	add	x22, x22, #0x1
    73fc:	350000c4 	cbnz	w4, 7414 <pthread_create@@GLIBC_2.17+0x248>
    7400:	f94002a0 	ldr	x0, [x21]
    7404:	b100041f 	cmn	x0, #0x1
    7408:	54000060 	b.eq	7414 <pthread_create@@GLIBC_2.17+0x248>
    740c:	97fff785 	bl	5220 <free@plt>
    7410:	f85f02e5 	ldr	x5, [x23,#-16]
    7414:	eb0502df 	cmp	x22, x5
    7418:	910042b5 	add	x21, x21, #0x10
    741c:	54fffec3 	b.cc	73f4 <pthread_create@@GLIBC_2.17+0x228>
    7420:	910004a2 	add	x2, x5, #0x1
    7424:	aa1703e0 	mov	x0, x23
    7428:	d53bd043 	mrs	x3, tpidr_el0
    742c:	52800001 	mov	w1, #0x0                   	// #0
    7430:	d37cec42 	lsl	x2, x2, #4
    7434:	911bc275 	add	x21, x19, #0x6f0
    7438:	aa0303f7 	mov	x23, x3
    743c:	aa1503f6 	mov	x22, x21
    7440:	97fff714 	bl	5090 <memset@plt>
    7444:	aa1503e0 	mov	x0, x21
    7448:	97fff7ca 	bl	5370 <_dl_allocate_tls_init@plt>
    744c:	f9425261 	ldr	x1, [x19,#1184]
    7450:	eb01033f 	cmp	x25, x1
    7454:	540037e8 	b.hi	7b50 <pthread_create@@GLIBC_2.17+0x984>
    7458:	9e660120 	fmov	x0, d9
    745c:	cb190021 	sub	x1, x1, x25
    7460:	cb1c0009 	sub	x9, x0, x28
    7464:	eb09003f 	cmp	x1, x9
    7468:	54003d28 	b.hi	7c0c <pthread_create@@GLIBC_2.17+0xa40>
    746c:	f9025679 	str	x25, [x19,#1192]
    7470:	d11bc2f5 	sub	x21, x23, #0x6f0
    7474:	b9400b03 	ldr	w3, [x24,#8]
    7478:	911122a4 	add	x4, x21, #0x448
    747c:	b9410ea1 	ldr	w1, [x21,#268]
    7480:	91038262 	add	x2, x19, #0xe0
    7484:	12000065 	and	w5, w3, #0x1
    7488:	12197460 	and	w0, w3, #0xffffff9f
    748c:	6b1f00bf 	cmp	w5, wzr
    7490:	121b0421 	and	w1, w1, #0x60
    7494:	91112265 	add	x5, x19, #0x448
    7498:	2a000021 	orr	w1, w1, w0
    749c:	928003e0 	mov	x0, #0xffffffffffffffe0    	// #-32
    74a0:	b94436a6 	ldr	w6, [x21,#1076]
    74a4:	a9402488 	ldp	x8, x9, [x4]
    74a8:	f9006e62 	str	x2, [x19,#216]
    74ac:	f9007262 	str	x2, [x19,#224]
    74b0:	d11bc262 	sub	x2, x19, #0x6f0
    74b4:	f9007660 	str	x0, [x19,#232]
    74b8:	9a9f1260 	csel	x0, x19, xzr, ne
    74bc:	b9041a7f 	str	wzr, [x19,#1048]
    74c0:	f9021260 	str	x0, [x19,#1056]
    74c4:	f9007a7f 	str	xzr, [x19,#240]
    74c8:	f9021e7b 	str	x27, [x19,#1080]
    74cc:	fd02226c 	str	d12, [x19,#1088]
    74d0:	b9010e61 	str	w1, [x19,#268]
    74d4:	a90024a8 	stp	x8, x9, [x5]
    74d8:	f9422ea0 	ldr	x0, [x21,#1112]
    74dc:	f9022e60 	str	x0, [x19,#1112]
    74e0:	b94432a0 	ldr	w0, [x21,#1072]
    74e4:	b9043260 	str	w0, [x19,#1072]
    74e8:	b9043666 	str	w6, [x19,#1076]
    74ec:	37082923 	tbnz	w3, #1, 7a10 <pthread_create@@GLIBC_2.17+0x844>
    74f0:	9e660161 	fmov	x1, d11
    74f4:	395046a0 	ldrb	w0, [x21,#1041]
    74f8:	f9000033 	str	x19, [x1]
    74fc:	35002e20 	cbnz	w0, 7ac0 <pthread_create@@GLIBC_2.17+0x8f4>
    7500:	f9401703 	ldr	x3, [x24,#40]
    7504:	b4002703 	cbz	x3, 79e4 <pthread_create@@GLIBC_2.17+0x818>
    7508:	b9410aa0 	ldr	w0, [x21,#264]
    750c:	52800021 	mov	w1, #0x1                   	// #1
    7510:	b9041660 	str	w0, [x19,#1044]
    7514:	52800023 	mov	w3, #0x1                   	// #1
    7518:	39104e61 	strb	w1, [x19,#1043]
    751c:	aa1303e0 	mov	x0, x19
    7520:	aa1803e1 	mov	x1, x24
    7524:	97fffa4e 	bl	5e5c <do_clone.constprop.4>
    7528:	2a0003f5 	mov	w21, w0
    752c:	34000300 	cbz	w0, 758c <pthread_create@@GLIBC_2.17+0x3c0>
    7530:	1e260100 	fmov	w0, s8
    7534:	350003e0 	cbnz	w0, 75b0 <pthread_create@@GLIBC_2.17+0x3e4>
    7538:	910003bf 	mov	sp, x29
    753c:	2a1503e0 	mov	w0, w21
    7540:	6d4627e8 	ldp	d8, d9, [sp,#96]
    7544:	a94153f3 	ldp	x19, x20, [sp,#16]
    7548:	6d472fea 	ldp	d10, d11, [sp,#112]
    754c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    7550:	6d4837ec 	ldp	d12, d13, [sp,#128]
    7554:	a94363f7 	ldp	x23, x24, [sp,#48]
    7558:	a9446bf9 	ldp	x25, x26, [sp,#64]
    755c:	a94573fb 	ldp	x27, x28, [sp,#80]
    7560:	a8cf7bfd 	ldp	x29, x30, [sp],#240
    7564:	d65f03c0 	ret
    7568:	f9004fac 	str	x12, [x29,#152]
    756c:	f90053aa 	str	x10, [x29,#160]
    7570:	f90057ab 	str	x11, [x29,#168]
    7574:	b900b3a2 	str	w2, [x29,#176]
    7578:	9400207c 	bl	f768 <__lll_lock_wait_private>
    757c:	f9404fac 	ldr	x12, [x29,#152]
    7580:	f94053aa 	ldr	x10, [x29,#160]
    7584:	f94057ab 	ldr	x11, [x29,#168]
    7588:	17ffff4d 	b	72bc <pthread_create@@GLIBC_2.17+0xf0>
    758c:	91106260 	add	x0, x19, #0x418
    7590:	885f7c01 	ldxr	w1, [x0]
    7594:	8802fc15 	stlxr	w2, w21, [x0]
    7598:	35ffffc2 	cbnz	w2, 7590 <pthread_create@@GLIBC_2.17+0x3c4>
    759c:	7100043f 	cmp	w1, #0x1
    75a0:	5400386c 	b.gt	7cac <pthread_create@@GLIBC_2.17+0xae0>
    75a4:	1e260100 	fmov	w0, s8
    75a8:	52800015 	mov	w21, #0x0                   	// #0
    75ac:	34fffc60 	cbz	w0, 7538 <pthread_create@@GLIBC_2.17+0x36c>
    75b0:	f94073a0 	ldr	x0, [x29,#224]
    75b4:	97fff71b 	bl	5220 <free@plt>
    75b8:	17ffffe0 	b	7538 <pthread_create@@GLIBC_2.17+0x36c>
    75bc:	b900b3b4 	str	w20, [x29,#176]
    75c0:	b0000174 	adrp	x20, 34000 <__GI___pthread_keys+0x3d78>
    75c4:	910a2280 	add	x0, x20, #0x288
    75c8:	52800022 	mov	w2, #0x1                   	// #1
    75cc:	885ffc01 	ldaxr	w1, [x0]
    75d0:	6b1f003f 	cmp	w1, wzr
    75d4:	54000061 	b.ne	75e0 <pthread_create@@GLIBC_2.17+0x414>
    75d8:	88037c02 	stxr	w3, w2, [x0]
    75dc:	35ffff83 	cbnz	w3, 75cc <pthread_create@@GLIBC_2.17+0x400>
    75e0:	540002e1 	b.ne	763c <pthread_create@@GLIBC_2.17+0x470>
    75e4:	b0000161 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    75e8:	910a2280 	add	x0, x20, #0x288
    75ec:	52800002 	mov	w2, #0x0                   	// #0
    75f0:	f941a834 	ldr	x20, [x1,#848]
    75f4:	885f7c01 	ldxr	w1, [x0]
    75f8:	8803fc02 	stlxr	w3, w2, [x0]
    75fc:	35ffffc3 	cbnz	w3, 75f4 <pthread_create@@GLIBC_2.17+0x428>
    7600:	7100043f 	cmp	w1, #0x1
    7604:	5400348c 	b.gt	7c94 <pthread_create@@GLIBC_2.17+0xac8>
    7608:	b9400b00 	ldr	w0, [x24,#8]
    760c:	361fe0e0 	tbz	w0, #3, 7228 <pthread_create@@GLIBC_2.17+0x5c>
    7610:	f9401300 	ldr	x0, [x24,#32]
    7614:	b40001a0 	cbz	x0, 7648 <pthread_create@@GLIBC_2.17+0x47c>
    7618:	b0000161 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    761c:	f9419033 	ldr	x19, [x1,#800]
    7620:	91200261 	add	x1, x19, #0x800
    7624:	eb00003f 	cmp	x1, x0
    7628:	54000149 	b.ls	7650 <pthread_create@@GLIBC_2.17+0x484>
    762c:	528002d5 	mov	w21, #0x16                  	// #22
    7630:	17ffffc0 	b	7530 <pthread_create@@GLIBC_2.17+0x364>
    7634:	aa0503f3 	mov	x19, x5
    7638:	17ffff28 	b	72d8 <pthread_create@@GLIBC_2.17+0x10c>
    763c:	b900b3a1 	str	w1, [x29,#176]
    7640:	9400204a 	bl	f768 <__lll_lock_wait_private>
    7644:	17ffffe8 	b	75e4 <pthread_create@@GLIBC_2.17+0x418>
    7648:	b0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    764c:	f9419013 	ldr	x19, [x0,#800]
    7650:	f9400f00 	ldr	x0, [x24,#24]
    7654:	52800001 	mov	w1, #0x0                   	// #0
    7658:	d280de02 	mov	x2, #0x6f0                 	// #1776
    765c:	52800036 	mov	w22, #0x1                   	// #1
    7660:	cb130013 	sub	x19, x0, x19
    7664:	b0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    7668:	d53bd059 	mrs	x25, tpidr_el0
    766c:	d11bc335 	sub	x21, x25, #0x6f0
    7670:	f9418c00 	ldr	x0, [x0,#792]
    7674:	8a200273 	bic	x19, x19, x0
    7678:	d11bc273 	sub	x19, x19, #0x6f0
    767c:	aa1303e0 	mov	x0, x19
    7680:	97fff684 	bl	5090 <memset@plt>
    7684:	91044261 	add	x1, x19, #0x110
    7688:	f9400f00 	ldr	x0, [x24,#24]
    768c:	f9018a61 	str	x1, [x19,#784]
    7690:	b0000161 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    7694:	b9000276 	str	w22, [x19]
    7698:	cb140000 	sub	x0, x0, x20
    769c:	f9024e74 	str	x20, [x19,#1176]
    76a0:	52800022 	mov	w2, #0x1                   	// #1
    76a4:	f941b821 	ldr	x1, [x1,#880]
    76a8:	f9024a60 	str	x0, [x19,#1168]
    76ac:	911bc260 	add	x0, x19, #0x6f0
    76b0:	39104a62 	strb	w2, [x19,#1042]
    76b4:	b9000036 	str	w22, [x1]
    76b8:	b0000161 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    76bc:	b9032836 	str	w22, [x1,#808]
    76c0:	b940d6a1 	ldr	w1, [x21,#212]
    76c4:	b900d661 	str	w1, [x19,#212]
    76c8:	12800001 	mov	w1, #0xffffffff            	// #-1
    76cc:	b9041e61 	str	w1, [x19,#1052]
    76d0:	97fff710 	bl	5310 <_dl_allocate_tls@plt>
    76d4:	b4002d80 	cbz	x0, 7c84 <pthread_create@@GLIBC_2.17+0xab8>
    76d8:	b0000174 	adrp	x20, 34000 <__GI___pthread_keys+0x3d78>
    76dc:	b900b3bf 	str	wzr, [x29,#176]
    76e0:	910a4280 	add	x0, x20, #0x290
    76e4:	91004000 	add	x0, x0, #0x10
    76e8:	885ffc01 	ldaxr	w1, [x0]
    76ec:	6b1f003f 	cmp	w1, wzr
    76f0:	54000061 	b.ne	76fc <pthread_create@@GLIBC_2.17+0x530>
    76f4:	88027c16 	stxr	w2, w22, [x0]
    76f8:	35ffff82 	cbnz	w2, 76e8 <pthread_create@@GLIBC_2.17+0x51c>
    76fc:	54002b81 	b.ne	7c6c <pthread_create@@GLIBC_2.17+0xaa0>
    7700:	910a4281 	add	x1, x20, #0x290
    7704:	91030262 	add	x2, x19, #0xc0
    7708:	91006020 	add	x0, x1, #0x18
    770c:	f9006660 	str	x0, [x19,#200]
    7710:	91004020 	add	x0, x1, #0x10
    7714:	f9400c23 	ldr	x3, [x1,#24]
    7718:	f9006263 	str	x3, [x19,#192]
    771c:	f9000462 	str	x2, [x3,#8]
    7720:	d5033bbf 	dmb	ish
    7724:	f9000c22 	str	x2, [x1,#24]
    7728:	52800002 	mov	w2, #0x0                   	// #0
    772c:	885f7c01 	ldxr	w1, [x0]
    7730:	8803fc02 	stlxr	w3, w2, [x0]
    7734:	35ffffc3 	cbnz	w3, 772c <pthread_create@@GLIBC_2.17+0x560>
    7738:	7100043f 	cmp	w1, #0x1
    773c:	54ffe9cd 	b.le	7474 <pthread_create@@GLIBC_2.17+0x2a8>
    7740:	d2801021 	mov	x1, #0x81                  	// #129
    7744:	d2800022 	mov	x2, #0x1                   	// #1
    7748:	d2800003 	mov	x3, #0x0                   	// #0
    774c:	d2800c48 	mov	x8, #0x62                  	// #98
    7750:	d4000001 	svc	#0x0
    7754:	17ffff48 	b	7474 <pthread_create@@GLIBC_2.17+0x2a8>
    7758:	910a4280 	add	x0, x20, #0x290
    775c:	52800002 	mov	w2, #0x0                   	// #0
    7760:	91004000 	add	x0, x0, #0x10
    7764:	885f7c01 	ldxr	w1, [x0]
    7768:	8803fc02 	stlxr	w3, w2, [x0]
    776c:	35ffffc3 	cbnz	w3, 7764 <pthread_create@@GLIBC_2.17+0x598>
    7770:	7100043f 	cmp	w1, #0x1
    7774:	54002aec 	b.gt	7cd0 <pthread_create@@GLIBC_2.17+0xb04>
    7778:	1e2601a2 	fmov	w2, s13
    777c:	d2800000 	mov	x0, #0x0                   	// #0
    7780:	52800443 	mov	w3, #0x22                  	// #34
    7784:	aa1c03e1 	mov	x1, x28
    7788:	72a00043 	movk	w3, #0x2, lsl #16
    778c:	12800004 	mov	w4, #0xffffffff            	// #-1
    7790:	aa0003e5 	mov	x5, x0
    7794:	f9004fac 	str	x12, [x29,#152]
    7798:	f90053aa 	str	x10, [x29,#160]
    779c:	f90057ab 	str	x11, [x29,#168]
    77a0:	97fff68c 	bl	51d0 <mmap@plt>
    77a4:	b100041f 	cmn	x0, #0x1
    77a8:	f94057ab 	ldr	x11, [x29,#168]
    77ac:	9e67000a 	fmov	d10, x0
    77b0:	f94053aa 	ldr	x10, [x29,#160]
    77b4:	f9404fac 	ldr	x12, [x29,#152]
    77b8:	54002fe0 	b.eq	7db4 <pthread_create@@GLIBC_2.17+0xbe8>
    77bc:	9e660143 	fmov	x3, d10
    77c0:	f9419160 	ldr	x0, [x11,#800]
    77c4:	f9418d84 	ldr	x4, [x12,#792]
    77c8:	5280003a 	mov	w26, #0x1                   	// #1
    77cc:	cb000380 	sub	x0, x28, x0
    77d0:	d53bd041 	mrs	x1, tpidr_el0
    77d4:	d11bc022 	sub	x2, x1, #0x6f0
    77d8:	f90053aa 	str	x10, [x29,#160]
    77dc:	8b000060 	add	x0, x3, x0
    77e0:	b0000163 	adrp	x3, 34000 <__GI___pthread_keys+0x3d78>
    77e4:	8a240004 	bic	x4, x0, x4
    77e8:	aa0103f7 	mov	x23, x1
    77ec:	d11bc093 	sub	x19, x4, #0x6f0
    77f0:	aa0403e0 	mov	x0, x4
    77f4:	91044265 	add	x5, x19, #0x110
    77f8:	f90057a4 	str	x4, [x29,#168]
    77fc:	b900027a 	str	w26, [x19]
    7800:	fd024a6a 	str	d10, [x19,#1168]
    7804:	f941b863 	ldr	x3, [x3,#880]
    7808:	f9024e7c 	str	x28, [x19,#1176]
    780c:	f9018a65 	str	x5, [x19,#784]
    7810:	b900007a 	str	w26, [x3]
    7814:	12800003 	mov	w3, #0xffffffff            	// #-1
    7818:	b9041e63 	str	w3, [x19,#1052]
    781c:	b940d442 	ldr	w2, [x2,#212]
    7820:	b900d662 	str	w2, [x19,#212]
    7824:	b0000162 	adrp	x2, 34000 <__GI___pthread_keys+0x3d78>
    7828:	b903285a 	str	w26, [x2,#808]
    782c:	97fff6b9 	bl	5310 <_dl_allocate_tls@plt>
    7830:	f94057a4 	ldr	x4, [x29,#168]
    7834:	f94053aa 	ldr	x10, [x29,#160]
    7838:	b4002b00 	cbz	x0, 7d98 <pthread_create@@GLIBC_2.17+0xbcc>
    783c:	910a4280 	add	x0, x20, #0x290
    7840:	b900b3bf 	str	wzr, [x29,#176]
    7844:	91004000 	add	x0, x0, #0x10
    7848:	885ffc02 	ldaxr	w2, [x0]
    784c:	6b1f005f 	cmp	w2, wzr
    7850:	54000061 	b.ne	785c <pthread_create@@GLIBC_2.17+0x690>
    7854:	88017c1a 	stxr	w1, w26, [x0]
    7858:	35ffff81 	cbnz	w1, 7848 <pthread_create@@GLIBC_2.17+0x67c>
    785c:	54001fa1 	b.ne	7c50 <pthread_create@@GLIBC_2.17+0xa84>
    7860:	910962d6 	add	x22, x22, #0x258
    7864:	9103027a 	add	x26, x19, #0xc0
    7868:	aa1603e1 	mov	x1, x22
    786c:	b2400340 	orr	x0, x26, #0x1
    7870:	f9014a80 	str	x0, [x20,#656]
    7874:	910a4280 	add	x0, x20, #0x290
    7878:	91004000 	add	x0, x0, #0x10
    787c:	d5033bbf 	dmb	ish
    7880:	f8410c22 	ldr	x2, [x1,#16]!
    7884:	f9006262 	str	x2, [x19,#192]
    7888:	f9006661 	str	x1, [x19,#200]
    788c:	f900045a 	str	x26, [x2,#8]
    7890:	52800002 	mov	w2, #0x0                   	// #0
    7894:	d5033bbf 	dmb	ish
    7898:	f9000ada 	str	x26, [x22,#16]
    789c:	d5033bbf 	dmb	ish
    78a0:	f9014a9f 	str	xzr, [x20,#656]
    78a4:	885f7c01 	ldxr	w1, [x0]
    78a8:	8803fc02 	stlxr	w3, w2, [x0]
    78ac:	35ffffc3 	cbnz	w3, 78a4 <pthread_create@@GLIBC_2.17+0x6d8>
    78b0:	7100043f 	cmp	w1, #0x1
    78b4:	540025ac 	b.gt	7d68 <pthread_create@@GLIBC_2.17+0xb9c>
    78b8:	f947ed4a 	ldr	x10, [x10,#4056]
    78bc:	aa0403f6 	mov	x22, x4
    78c0:	9e670389 	fmov	d9, x28
    78c4:	b94fa140 	ldr	w0, [x10,#4000]
    78c8:	3607dc20 	tbz	w0, #0, 744c <pthread_create@@GLIBC_2.17+0x280>
    78cc:	35ffdc15 	cbnz	w21, 744c <pthread_create@@GLIBC_2.17+0x280>
    78d0:	f9425261 	ldr	x1, [x19,#1184]
    78d4:	528000e2 	mov	w2, #0x7                   	// #7
    78d8:	f9424a60 	ldr	x0, [x19,#1168]
    78dc:	f9424e63 	ldr	x3, [x19,#1176]
    78e0:	8b010000 	add	x0, x0, x1
    78e4:	cb010061 	sub	x1, x3, x1
    78e8:	97fff6b2 	bl	53b0 <mprotect@plt>
    78ec:	340022e0 	cbz	w0, 7d48 <pthread_create@@GLIBC_2.17+0xb7c>
    78f0:	90000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    78f4:	f947c400 	ldr	x0, [x0,#3976]
    78f8:	b8606af5 	ldr	w21, [x23,x0]
    78fc:	34ffda95 	cbz	w21, 744c <pthread_create@@GLIBC_2.17+0x280>
    7900:	9e660140 	fmov	x0, d10
    7904:	aa1c03e1 	mov	x1, x28
    7908:	97fff66a 	bl	52b0 <munmap@plt>
    790c:	710032bf 	cmp	w21, #0xc
    7910:	52800160 	mov	w0, #0xb                   	// #11
    7914:	1a8012b5 	csel	w21, w21, w0, ne
    7918:	17ffff06 	b	7530 <pthread_create@@GLIBC_2.17+0x364>
    791c:	b0000174 	adrp	x20, 34000 <__GI___pthread_keys+0x3d78>
    7920:	b900bba1 	str	w1, [x29,#184]
    7924:	910a2280 	add	x0, x20, #0x288
    7928:	52800022 	mov	w2, #0x1                   	// #1
    792c:	885ffc01 	ldaxr	w1, [x0]
    7930:	6b1f003f 	cmp	w1, wzr
    7934:	54000061 	b.ne	7940 <pthread_create@@GLIBC_2.17+0x774>
    7938:	88037c02 	stxr	w3, w2, [x0]
    793c:	35ffff83 	cbnz	w3, 792c <pthread_create@@GLIBC_2.17+0x760>
    7940:	54000060 	b.eq	794c <pthread_create@@GLIBC_2.17+0x780>
    7944:	b900bba1 	str	w1, [x29,#184]
    7948:	94001f88 	bl	f768 <__lll_lock_wait_private>
    794c:	b0000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    7950:	9102e3b8 	add	x24, x29, #0xb8
    7954:	910cc000 	add	x0, x0, #0x330
    7958:	a9401404 	ldp	x4, x5, [x0]
    795c:	a9410c02 	ldp	x2, x3, [x0,#16]
    7960:	f9401813 	ldr	x19, [x0,#48]
    7964:	a9420400 	ldp	x0, x1, [x0,#32]
    7968:	f90077b3 	str	x19, [x29,#232]
    796c:	a9001704 	stp	x4, x5, [x24]
    7970:	a9010f02 	stp	x2, x3, [x24,#16]
    7974:	a9020700 	stp	x0, x1, [x24,#32]
    7978:	b4001693 	cbz	x19, 7c48 <pthread_create@@GLIBC_2.17+0xa7c>
    797c:	f140227f 	cmp	x19, #0x8, lsl #12
    7980:	54001b48 	b.hi	7ce8 <pthread_create@@GLIBC_2.17+0xb1c>
    7984:	91007a60 	add	x0, x19, #0x1e
    7988:	910003e1 	mov	x1, sp
    798c:	927cec00 	and	x0, x0, #0xfffffffffffffff0
    7990:	1e2703e8 	fmov	s8, wzr
    7994:	cb20603f 	sub	sp, x1, x0
    7998:	910003e3 	mov	x3, sp
    799c:	f94073a1 	ldr	x1, [x29,#224]
    79a0:	aa0303e0 	mov	x0, x3
    79a4:	aa1303e2 	mov	x2, x19
    79a8:	97fff56e 	bl	4f60 <memcpy@plt>
    79ac:	f90073a0 	str	x0, [x29,#224]
    79b0:	910a2280 	add	x0, x20, #0x288
    79b4:	52800002 	mov	w2, #0x0                   	// #0
    79b8:	885f7c01 	ldxr	w1, [x0]
    79bc:	8803fc02 	stlxr	w3, w2, [x0]
    79c0:	35ffffc3 	cbnz	w3, 79b8 <pthread_create@@GLIBC_2.17+0x7ec>
    79c4:	7100043f 	cmp	w1, #0x1
    79c8:	54ffc22d 	b.le	720c <pthread_create@@GLIBC_2.17+0x40>
    79cc:	d2801021 	mov	x1, #0x81                  	// #129
    79d0:	d2800022 	mov	x2, #0x1                   	// #1
    79d4:	d2800003 	mov	x3, #0x0                   	// #0
    79d8:	d2800c48 	mov	x8, #0x62                  	// #98
    79dc:	d4000001 	svc	#0x0
    79e0:	17fffe0b 	b	720c <pthread_create@@GLIBC_2.17+0x40>
    79e4:	b9400b00 	ldr	w0, [x24,#8]
    79e8:	121f0001 	and	w1, w0, #0x2
    79ec:	370fd8e0 	tbnz	w0, #1, 7508 <pthread_create@@GLIBC_2.17+0x33c>
    79f0:	b9410aa4 	ldr	w4, [x21,#264]
    79f4:	aa1303e0 	mov	x0, x19
    79f8:	39104e61 	strb	w1, [x19,#1043]
    79fc:	aa1803e1 	mov	x1, x24
    7a00:	b9041664 	str	w4, [x19,#1044]
    7a04:	97fff916 	bl	5e5c <do_clone.constprop.4>
    7a08:	2a0003f5 	mov	w21, w0
    7a0c:	17fffec9 	b	7530 <pthread_create@@GLIBC_2.17+0x364>
    7a10:	721b047f 	tst	w3, #0x60
    7a14:	54ffd6e0 	b.eq	74f0 <pthread_create@@GLIBC_2.17+0x324>
    7a18:	37301303 	tbnz	w3, #6, 7c78 <pthread_create@@GLIBC_2.17+0xaac>
    7a1c:	37300121 	tbnz	w1, #6, 7a40 <pthread_create@@GLIBC_2.17+0x874>
    7a20:	d2800000 	mov	x0, #0x0                   	// #0
    7a24:	d2800f08 	mov	x8, #0x78                  	// #120
    7a28:	d4000001 	svc	#0x0
    7a2c:	b9410e61 	ldr	w1, [x19,#268]
    7a30:	b9400b03 	ldr	w3, [x24,#8]
    7a34:	321a0021 	orr	w1, w1, #0x40
    7a38:	b9043660 	str	w0, [x19,#1076]
    7a3c:	b9010e61 	str	w1, [x19,#268]
    7a40:	37281423 	tbnz	w3, #5, 7cc4 <pthread_create@@GLIBC_2.17+0xaf8>
    7a44:	37280101 	tbnz	w1, #5, 7a64 <pthread_create@@GLIBC_2.17+0x898>
    7a48:	d2800000 	mov	x0, #0x0                   	// #0
    7a4c:	9110c261 	add	x1, x19, #0x430
    7a50:	d2800f28 	mov	x8, #0x79                  	// #121
    7a54:	d4000001 	svc	#0x0
    7a58:	b9410e60 	ldr	w0, [x19,#268]
    7a5c:	321b0000 	orr	w0, w0, #0x20
    7a60:	b9010e60 	str	w0, [x19,#268]
    7a64:	b9800700 	ldrsw	x0, [x24,#4]
    7a68:	d2800fc8 	mov	x8, #0x7e                  	// #126
    7a6c:	d4000001 	svc	#0x0
    7a70:	d2800fa8 	mov	x8, #0x7d                  	// #125
    7a74:	aa0003e3 	mov	x3, x0
    7a78:	b9800700 	ldrsw	x0, [x24,#4]
    7a7c:	d4000001 	svc	#0x0
    7a80:	b9443261 	ldr	w1, [x19,#1072]
    7a84:	6b03003f 	cmp	w1, w3
    7a88:	5400006b 	b.lt	7a94 <pthread_create@@GLIBC_2.17+0x8c8>
    7a8c:	6b00003f 	cmp	w1, w0
    7a90:	54ffd30d 	b.le	74f0 <pthread_create@@GLIBC_2.17+0x324>
    7a94:	91107260 	add	x0, x19, #0x41c
    7a98:	52800002 	mov	w2, #0x0                   	// #0
    7a9c:	885ffc01 	ldaxr	w1, [x0]
    7aa0:	88037c02 	stxr	w3, w2, [x0]
    7aa4:	35ffffc3 	cbnz	w3, 7a9c <pthread_create@@GLIBC_2.17+0x8d0>
    7aa8:	3100083f 	cmn	w1, #0x2
    7aac:	54001520 	b.eq	7d50 <pthread_create@@GLIBC_2.17+0xb84>
    7ab0:	aa1303e0 	mov	x0, x19
    7ab4:	528002d5 	mov	w21, #0x16                  	// #22
    7ab8:	97fff86f 	bl	5c74 <__deallocate_stack>
    7abc:	17fffe9d 	b	7530 <pthread_create@@GLIBC_2.17+0x364>
    7ac0:	910a4294 	add	x20, x20, #0x290
    7ac4:	b9444a61 	ldr	w1, [x19,#1096]
    7ac8:	b9402a80 	ldr	w0, [x20,#40]
    7acc:	2a000020 	orr	w0, w1, w0
    7ad0:	363fd180 	tbz	w0, #7, 7500 <pthread_create@@GLIBC_2.17+0x334>
    7ad4:	52800021 	mov	w1, #0x1                   	// #1
    7ad8:	aa1303e0 	mov	x0, x19
    7adc:	39104e61 	strb	w1, [x19,#1043]
    7ae0:	52800023 	mov	w3, #0x1                   	// #1
    7ae4:	aa1803e1 	mov	x1, x24
    7ae8:	97fff8dd 	bl	5e5c <do_clone.constprop.4>
    7aec:	2a0003f5 	mov	w21, w0
    7af0:	35ffd200 	cbnz	w0, 7530 <pthread_create@@GLIBC_2.17+0x364>
    7af4:	52800100 	mov	w0, #0x8                   	// #8
    7af8:	f9022e73 	str	x19, [x19,#1112]
    7afc:	b9045260 	str	w0, [x19,#1104]
    7b00:	aa1403e3 	mov	x3, x20
    7b04:	aa0303e2 	mov	x2, x3
    7b08:	f8430c41 	ldr	x1, [x2,#48]!
    7b0c:	f9023261 	str	x1, [x19,#1120]
    7b10:	f9005ba1 	str	x1, [x29,#176]
    7b14:	c85ffc44 	ldaxr	x4, [x2]
    7b18:	eb01009f 	cmp	x4, x1
    7b1c:	54000061 	b.ne	7b28 <pthread_create@@GLIBC_2.17+0x95c>
    7b20:	c8007c53 	stxr	w0, x19, [x2]
    7b24:	35ffff80 	cbnz	w0, 7b14 <pthread_create@@GLIBC_2.17+0x948>
    7b28:	54fffee1 	b.ne	7b04 <pthread_create@@GLIBC_2.17+0x938>
    7b2c:	97fff7d6 	bl	5a84 <__GI___nptl_create_event>
    7b30:	91106260 	add	x0, x19, #0x418
    7b34:	52800002 	mov	w2, #0x0                   	// #0
    7b38:	885f7c01 	ldxr	w1, [x0]
    7b3c:	8803fc02 	stlxr	w3, w2, [x0]
    7b40:	34ffd2e3 	cbz	w3, 759c <pthread_create@@GLIBC_2.17+0x3d0>
    7b44:	17fffffd 	b	7b38 <pthread_create@@GLIBC_2.17+0x96c>
    7b48:	aa0503f3 	mov	x19, x5
    7b4c:	17fffdf5 	b	7320 <pthread_create@@GLIBC_2.17+0x154>
    7b50:	9e660140 	fmov	x0, d10
    7b54:	aa1903e1 	mov	x1, x25
    7b58:	52800002 	mov	w2, #0x0                   	// #0
    7b5c:	97fff615 	bl	53b0 <mprotect@plt>
    7b60:	34000600 	cbz	w0, 7c20 <pthread_create@@GLIBC_2.17+0xa54>
    7b64:	910a4280 	add	x0, x20, #0x290
    7b68:	b900b3bf 	str	wzr, [x29,#176]
    7b6c:	91004000 	add	x0, x0, #0x10
    7b70:	52800022 	mov	w2, #0x1                   	// #1
    7b74:	885ffc01 	ldaxr	w1, [x0]
    7b78:	6b1f003f 	cmp	w1, wzr
    7b7c:	54000061 	b.ne	7b88 <pthread_create@@GLIBC_2.17+0x9bc>
    7b80:	88037c02 	stxr	w3, w2, [x0]
    7b84:	35ffff83 	cbnz	w3, 7b74 <pthread_create@@GLIBC_2.17+0x9a8>
    7b88:	54000060 	b.eq	7b94 <pthread_create@@GLIBC_2.17+0x9c8>
    7b8c:	b900b3a1 	str	w1, [x29,#176]
    7b90:	94001ef6 	bl	f768 <__lll_lock_wait_private>
    7b94:	f9014a9a 	str	x26, [x20,#656]
    7b98:	910a4280 	add	x0, x20, #0x290
    7b9c:	91004000 	add	x0, x0, #0x10
    7ba0:	d5033bbf 	dmb	ish
    7ba4:	f9406261 	ldr	x1, [x19,#192]
    7ba8:	f9406662 	ldr	x2, [x19,#200]
    7bac:	f9000422 	str	x2, [x1,#8]
    7bb0:	f9406662 	ldr	x2, [x19,#200]
    7bb4:	f9000041 	str	x1, [x2]
    7bb8:	52800002 	mov	w2, #0x0                   	// #0
    7bbc:	d5033bbf 	dmb	ish
    7bc0:	f9014a9f 	str	xzr, [x20,#656]
    7bc4:	885f7c01 	ldxr	w1, [x0]
    7bc8:	8803fc02 	stlxr	w3, w2, [x0]
    7bcc:	35ffffc3 	cbnz	w3, 7bc4 <pthread_create@@GLIBC_2.17+0x9f8>
    7bd0:	7100043f 	cmp	w1, #0x1
    7bd4:	54000d6c 	b.gt	7d80 <pthread_create@@GLIBC_2.17+0xbb4>
    7bd8:	aa1603e0 	mov	x0, x22
    7bdc:	52800001 	mov	w1, #0x0                   	// #0
    7be0:	97fff540 	bl	50e0 <_dl_deallocate_tls@plt>
    7be4:	9e660140 	fmov	x0, d10
    7be8:	9e660121 	fmov	x1, d9
    7bec:	97fff5b1 	bl	52b0 <munmap@plt>
    7bf0:	90000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    7bf4:	f947c400 	ldr	x0, [x0,#3976]
    7bf8:	b8606af5 	ldr	w21, [x23,x0]
    7bfc:	35ffe895 	cbnz	w21, 790c <pthread_create@@GLIBC_2.17+0x740>
    7c00:	d2800000 	mov	x0, #0x0                   	// #0
    7c04:	f9021c1b 	str	x27, [x0,#1080]
    7c08:	d4207d00 	brk	#0x3e8
    7c0c:	9e660140 	fmov	x0, d10
    7c10:	1e2601a2 	fmov	w2, s13
    7c14:	8b190000 	add	x0, x0, x25
    7c18:	97fff5e6 	bl	53b0 <mprotect@plt>
    7c1c:	35fffa40 	cbnz	w0, 7b64 <pthread_create@@GLIBC_2.17+0x998>
    7c20:	f9025279 	str	x25, [x19,#1184]
    7c24:	17fffe12 	b	746c <pthread_create@@GLIBC_2.17+0x2a0>
    7c28:	aa1403e0 	mov	x0, x20
    7c2c:	17fffe7b 	b	7618 <pthread_create@@GLIBC_2.17+0x44c>
    7c30:	d2801021 	mov	x1, #0x81                  	// #129
    7c34:	d2800022 	mov	x2, #0x1                   	// #1
    7c38:	d2800003 	mov	x3, #0x0                   	// #0
    7c3c:	d2800c48 	mov	x8, #0x62                  	// #98
    7c40:	d4000001 	svc	#0x0
    7c44:	17fffddf 	b	73c0 <pthread_create@@GLIBC_2.17+0x1f4>
    7c48:	1e270268 	fmov	s8, w19
    7c4c:	17ffff59 	b	79b0 <pthread_create@@GLIBC_2.17+0x7e4>
    7c50:	f90053aa 	str	x10, [x29,#160]
    7c54:	f90057a4 	str	x4, [x29,#168]
    7c58:	b900b3a2 	str	w2, [x29,#176]
    7c5c:	94001ec3 	bl	f768 <__lll_lock_wait_private>
    7c60:	f94053aa 	ldr	x10, [x29,#160]
    7c64:	f94057a4 	ldr	x4, [x29,#168]
    7c68:	17fffefe 	b	7860 <pthread_create@@GLIBC_2.17+0x694>
    7c6c:	b900b3a1 	str	w1, [x29,#176]
    7c70:	94001ebe 	bl	f768 <__lll_lock_wait_private>
    7c74:	17fffea3 	b	7700 <pthread_create@@GLIBC_2.17+0x534>
    7c78:	b9400700 	ldr	w0, [x24,#4]
    7c7c:	b9043660 	str	w0, [x19,#1076]
    7c80:	17ffff70 	b	7a40 <pthread_create@@GLIBC_2.17+0x874>
    7c84:	90000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    7c88:	f947c400 	ldr	x0, [x0,#3976]
    7c8c:	b8606b35 	ldr	w21, [x25,x0]
    7c90:	17ffffdb 	b	7bfc <pthread_create@@GLIBC_2.17+0xa30>
    7c94:	d2801021 	mov	x1, #0x81                  	// #129
    7c98:	d2800022 	mov	x2, #0x1                   	// #1
    7c9c:	d2800003 	mov	x3, #0x0                   	// #0
    7ca0:	d2800c48 	mov	x8, #0x62                  	// #98
    7ca4:	d4000001 	svc	#0x0
    7ca8:	17fffe58 	b	7608 <pthread_create@@GLIBC_2.17+0x43c>
    7cac:	d2801021 	mov	x1, #0x81                  	// #129
    7cb0:	d2800022 	mov	x2, #0x1                   	// #1
    7cb4:	d2800003 	mov	x3, #0x0                   	// #0
    7cb8:	d2800c48 	mov	x8, #0x62                  	// #98
    7cbc:	d4000001 	svc	#0x0
    7cc0:	17fffe1c 	b	7530 <pthread_create@@GLIBC_2.17+0x364>
    7cc4:	b9400300 	ldr	w0, [x24]
    7cc8:	b9043260 	str	w0, [x19,#1072]
    7ccc:	17ffff66 	b	7a64 <pthread_create@@GLIBC_2.17+0x898>
    7cd0:	d2801021 	mov	x1, #0x81                  	// #129
    7cd4:	d2800022 	mov	x2, #0x1                   	// #1
    7cd8:	d2800003 	mov	x3, #0x0                   	// #0
    7cdc:	d2800c48 	mov	x8, #0x62                  	// #98
    7ce0:	d4000001 	svc	#0x0
    7ce4:	17fffea5 	b	7778 <pthread_create@@GLIBC_2.17+0x5ac>
    7ce8:	aa1303e0 	mov	x0, x19
    7cec:	97fff59d 	bl	5360 <__libc_alloca_cutoff@plt>
    7cf0:	2a0003f5 	mov	w21, w0
    7cf4:	35ffe480 	cbnz	w0, 7984 <pthread_create@@GLIBC_2.17+0x7b8>
    7cf8:	aa1303e0 	mov	x0, x19
    7cfc:	97fff4e1 	bl	5080 <malloc@plt>
    7d00:	aa0003e3 	mov	x3, x0
    7d04:	b4000080 	cbz	x0, 7d14 <pthread_create@@GLIBC_2.17+0xb48>
    7d08:	52800020 	mov	w0, #0x1                   	// #1
    7d0c:	1e270008 	fmov	s8, w0
    7d10:	17ffff23 	b	799c <pthread_create@@GLIBC_2.17+0x7d0>
    7d14:	910a2294 	add	x20, x20, #0x288
    7d18:	885f7e80 	ldxr	w0, [x20]
    7d1c:	8801fe95 	stlxr	w1, w21, [x20]
    7d20:	35ffffc1 	cbnz	w1, 7d18 <pthread_create@@GLIBC_2.17+0xb4c>
    7d24:	7100041f 	cmp	w0, #0x1
    7d28:	52800195 	mov	w21, #0xc                   	// #12
    7d2c:	54ffc06d 	b.le	7538 <pthread_create@@GLIBC_2.17+0x36c>
    7d30:	aa1403e0 	mov	x0, x20
    7d34:	d2801021 	mov	x1, #0x81                  	// #129
    7d38:	d2800022 	mov	x2, #0x1                   	// #1
    7d3c:	d2800c48 	mov	x8, #0x62                  	// #98
    7d40:	d4000001 	svc	#0x0
    7d44:	17fffdfd 	b	7538 <pthread_create@@GLIBC_2.17+0x36c>
    7d48:	9e670389 	fmov	d9, x28
    7d4c:	17fffdc0 	b	744c <pthread_create@@GLIBC_2.17+0x280>
    7d50:	d2801021 	mov	x1, #0x81                  	// #129
    7d54:	d2800022 	mov	x2, #0x1                   	// #1
    7d58:	d2800003 	mov	x3, #0x0                   	// #0
    7d5c:	d2800c48 	mov	x8, #0x62                  	// #98
    7d60:	d4000001 	svc	#0x0
    7d64:	17ffff53 	b	7ab0 <pthread_create@@GLIBC_2.17+0x8e4>
    7d68:	d2801021 	mov	x1, #0x81                  	// #129
    7d6c:	d2800022 	mov	x2, #0x1                   	// #1
    7d70:	d2800003 	mov	x3, #0x0                   	// #0
    7d74:	d2800c48 	mov	x8, #0x62                  	// #98
    7d78:	d4000001 	svc	#0x0
    7d7c:	17fffecf 	b	78b8 <pthread_create@@GLIBC_2.17+0x6ec>
    7d80:	d2801021 	mov	x1, #0x81                  	// #129
    7d84:	d2800022 	mov	x2, #0x1                   	// #1
    7d88:	d2800003 	mov	x3, #0x0                   	// #0
    7d8c:	d2800c48 	mov	x8, #0x62                  	// #98
    7d90:	d4000001 	svc	#0x0
    7d94:	17ffff91 	b	7bd8 <pthread_create@@GLIBC_2.17+0xa0c>
    7d98:	9e660140 	fmov	x0, d10
    7d9c:	aa1c03e1 	mov	x1, x28
    7da0:	97fff544 	bl	52b0 <munmap@plt>
    7da4:	90000140 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    7da8:	f947c400 	ldr	x0, [x0,#3976]
    7dac:	b8606af5 	ldr	w21, [x23,x0]
    7db0:	17ffff93 	b	7bfc <pthread_create@@GLIBC_2.17+0xa30>
    7db4:	d53bd040 	mrs	x0, tpidr_el0
    7db8:	90000141 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    7dbc:	f947c421 	ldr	x1, [x1,#3976]
    7dc0:	b8616815 	ldr	w21, [x0,x1]
    7dc4:	17ffff8e 	b	7bfc <pthread_create@@GLIBC_2.17+0xa30>

0000000000007dc8 <pthread_exit>:
    7dc8:	d53bd041 	mrs	x1, tpidr_el0
    7dcc:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    7dd0:	d11bc022 	sub	x2, x1, #0x6f0
    7dd4:	d117a021 	sub	x1, x1, #0x5e8
    7dd8:	910003fd 	mov	x29, sp
    7ddc:	910073a6 	add	x6, x29, #0x1c
    7de0:	91042043 	add	x3, x2, #0x108
    7de4:	f9021440 	str	x0, [x2,#1064]
    7de8:	b9400020 	ldr	w0, [x1]
    7dec:	b9001fa0 	str	w0, [x29,#28]
    7df0:	321c0004 	orr	w4, w0, #0x10
    7df4:	885ffc65 	ldaxr	w5, [x3]
    7df8:	6b0000bf 	cmp	w5, w0
    7dfc:	54000061 	b.ne	7e08 <pthread_exit+0x40>
    7e00:	88077c64 	stxr	w7, w4, [x3]
    7e04:	35ffff87 	cbnz	w7, 7df4 <pthread_exit+0x2c>
    7e08:	54000060 	b.eq	7e14 <pthread_exit+0x4c>
    7e0c:	b90000c5 	str	w5, [x6]
    7e10:	17fffff6 	b	7de8 <pthread_exit+0x20>
    7e14:	f9408040 	ldr	x0, [x2,#256]
    7e18:	94001dbb 	bl	f504 <__pthread_unwind>

0000000000007e1c <pthread_detach>:
    7e1c:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    7e20:	aa0003e1 	mov	x1, x0
    7e24:	910003fd 	mov	x29, sp
    7e28:	b940d002 	ldr	w2, [x0,#208]
    7e2c:	37f802c2 	tbnz	w2, #31, 7e84 <pthread_detach+0x68>
    7e30:	f9000fbf 	str	xzr, [x29,#24]
    7e34:	91108002 	add	x2, x0, #0x420
    7e38:	c85ffc43 	ldaxr	x3, [x2]
    7e3c:	eb1f007f 	cmp	x3, xzr
    7e40:	54000061 	b.ne	7e4c <pthread_detach+0x30>
    7e44:	c8047c41 	stxr	w4, x1, [x2]
    7e48:	35ffff84 	cbnz	w4, 7e38 <pthread_detach+0x1c>
    7e4c:	540000e1 	b.ne	7e68 <pthread_detach+0x4c>
    7e50:	b9410822 	ldr	w2, [x1,#264]
    7e54:	121c0041 	and	w1, w2, #0x10
    7e58:	36200102 	tbz	w2, #4, 7e78 <pthread_detach+0x5c>
    7e5c:	97fffbf2 	bl	6e24 <__free_tcb>
    7e60:	52800001 	mov	w1, #0x0                   	// #0
    7e64:	14000005 	b	7e78 <pthread_detach+0x5c>
    7e68:	f9421020 	ldr	x0, [x1,#1056]
    7e6c:	eb01001f 	cmp	x0, x1
    7e70:	528002c1 	mov	w1, #0x16                  	// #22
    7e74:	1a8113e1 	csel	w1, wzr, w1, ne
    7e78:	2a0103e0 	mov	w0, w1
    7e7c:	a8c27bfd 	ldp	x29, x30, [sp],#32
    7e80:	d65f03c0 	ret
    7e84:	52800061 	mov	w1, #0x3                   	// #3
    7e88:	2a0103e0 	mov	w0, w1
    7e8c:	a8c27bfd 	ldp	x29, x30, [sp],#32
    7e90:	d65f03c0 	ret

0000000000007e94 <cleanup>:
    7e94:	d10043ff 	sub	sp, sp, #0x10
    7e98:	d53bd041 	mrs	x1, tpidr_el0
    7e9c:	d11bc021 	sub	x1, x1, #0x6f0
    7ea0:	d2800002 	mov	x2, #0x0                   	// #0
    7ea4:	f90007e1 	str	x1, [sp,#8]
    7ea8:	c85ffc03 	ldaxr	x3, [x0]
    7eac:	eb01007f 	cmp	x3, x1
    7eb0:	54000061 	b.ne	7ebc <cleanup+0x28>
    7eb4:	c8047c02 	stxr	w4, x2, [x0]
    7eb8:	35ffff84 	cbnz	w4, 7ea8 <cleanup+0x14>
    7ebc:	910043ff 	add	sp, sp, #0x10
    7ec0:	d65f03c0 	ret

0000000000007ec4 <pthread_join>:
    7ec4:	a9b97bfd 	stp	x29, x30, [sp,#-112]!
    7ec8:	910003fd 	mov	x29, sp
    7ecc:	a90153f3 	stp	x19, x20, [sp,#16]
    7ed0:	aa0003f3 	mov	x19, x0
    7ed4:	b940d000 	ldr	w0, [x0,#208]
    7ed8:	a9025bf5 	stp	x21, x22, [sp,#32]
    7edc:	a90363f7 	stp	x23, x24, [sp,#48]
    7ee0:	37f80a20 	tbnz	w0, #31, 8024 <pthread_join+0x160>
    7ee4:	f9421262 	ldr	x2, [x19,#1056]
    7ee8:	528002c0 	mov	w0, #0x16                  	// #22
    7eec:	eb13005f 	cmp	x2, x19
    7ef0:	54000420 	b.eq	7f74 <pthread_join+0xb0>
    7ef4:	aa0103f7 	mov	x23, x1
    7ef8:	91108276 	add	x22, x19, #0x420
    7efc:	910143b8 	add	x24, x29, #0x50
    7f00:	90000001 	adrp	x1, 7000 <start_thread+0x174>
    7f04:	aa1803e0 	mov	x0, x24
    7f08:	913a5021 	add	x1, x1, #0xe94
    7f0c:	aa1603e2 	mov	x2, x22
    7f10:	d53bd055 	mrs	x21, tpidr_el0
    7f14:	d11bc2b5 	sub	x21, x21, #0x6f0
    7f18:	94001cba 	bl	f200 <_pthread_cleanup_push>
    7f1c:	94001dbf 	bl	f618 <__pthread_enable_asynccancel>
    7f20:	2a0003e5 	mov	w5, w0
    7f24:	eb15027f 	cmp	x19, x21
    7f28:	54000700 	b.eq	8008 <pthread_join+0x144>
    7f2c:	f94212a0 	ldr	x0, [x21,#1056]
    7f30:	eb13001f 	cmp	x0, x19
    7f34:	54000640 	b.eq	7ffc <pthread_join+0x138>
    7f38:	f90027bf 	str	xzr, [x29,#72]
    7f3c:	c85ffec0 	ldaxr	x0, [x22]
    7f40:	eb1f001f 	cmp	x0, xzr
    7f44:	54000061 	b.ne	7f50 <pthread_join+0x8c>
    7f48:	c8017ed5 	stxr	w1, x21, [x22]
    7f4c:	35ffff81 	cbnz	w1, 7f3c <pthread_join+0x78>
    7f50:	540001c0 	b.eq	7f88 <pthread_join+0xc4>
    7f54:	f90027a0 	str	x0, [x29,#72]
    7f58:	528002d4 	mov	w20, #0x16                  	// #22
    7f5c:	2a0503e0 	mov	w0, w5
    7f60:	94001dde 	bl	f6d8 <__pthread_disable_asynccancel>
    7f64:	aa1803e0 	mov	x0, x24
    7f68:	52800001 	mov	w1, #0x0                   	// #0
    7f6c:	94001cad 	bl	f220 <_pthread_cleanup_pop>
    7f70:	2a1403e0 	mov	w0, w20
    7f74:	a94153f3 	ldp	x19, x20, [sp,#16]
    7f78:	a9425bf5 	ldp	x21, x22, [sp,#32]
    7f7c:	a94363f7 	ldp	x23, x24, [sp,#48]
    7f80:	a8c77bfd 	ldp	x29, x30, [sp],#112
    7f84:	d65f03c0 	ret
    7f88:	b940d262 	ldr	w2, [x19,#208]
    7f8c:	34000142 	cbz	w2, 7fb4 <pthread_join+0xf0>
    7f90:	91034264 	add	x4, x19, #0xd0
    7f94:	d2800001 	mov	x1, #0x0                   	// #0
    7f98:	aa0403e0 	mov	x0, x4
    7f9c:	93407c42 	sxtw	x2, w2
    7fa0:	aa0103e3 	mov	x3, x1
    7fa4:	d2800c48 	mov	x8, #0x62                  	// #98
    7fa8:	d4000001 	svc	#0x0
    7fac:	b940d262 	ldr	w2, [x19,#208]
    7fb0:	35ffff22 	cbnz	w2, 7f94 <pthread_join+0xd0>
    7fb4:	2a0503e0 	mov	w0, w5
    7fb8:	94001dc8 	bl	f6d8 <__pthread_disable_asynccancel>
    7fbc:	aa1803e0 	mov	x0, x24
    7fc0:	52800001 	mov	w1, #0x0                   	// #0
    7fc4:	94001c97 	bl	f220 <_pthread_cleanup_pop>
    7fc8:	12800000 	mov	w0, #0xffffffff            	// #-1
    7fcc:	b900d260 	str	w0, [x19,#208]
    7fd0:	b4000077 	cbz	x23, 7fdc <pthread_join+0x118>
    7fd4:	f9421660 	ldr	x0, [x19,#1064]
    7fd8:	f90002e0 	str	x0, [x23]
    7fdc:	aa1303e0 	mov	x0, x19
    7fe0:	97fffb91 	bl	6e24 <__free_tcb>
    7fe4:	52800000 	mov	w0, #0x0                   	// #0
    7fe8:	a94153f3 	ldp	x19, x20, [sp,#16]
    7fec:	a9425bf5 	ldp	x21, x22, [sp,#32]
    7ff0:	a94363f7 	ldp	x23, x24, [sp,#48]
    7ff4:	a8c77bfd 	ldp	x29, x30, [sp],#112
    7ff8:	d65f03c0 	ret
    7ffc:	b9410a60 	ldr	w0, [x19,#264]
    8000:	721e0c1f 	tst	w0, #0x3c
    8004:	54fff9a1 	b.ne	7f38 <pthread_join+0x74>
    8008:	b9410aa1 	ldr	w1, [x21,#264]
    800c:	128008c0 	mov	w0, #0xffffffb9            	// #-71
    8010:	52800474 	mov	w20, #0x23                  	// #35
    8014:	0a000020 	and	w0, w1, w0
    8018:	7100201f 	cmp	w0, #0x8
    801c:	54fffa01 	b.ne	7f5c <pthread_join+0x98>
    8020:	17ffffc6 	b	7f38 <pthread_join+0x74>
    8024:	52800060 	mov	w0, #0x3                   	// #3
    8028:	a94153f3 	ldp	x19, x20, [sp,#16]
    802c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8030:	a94363f7 	ldp	x23, x24, [sp,#48]
    8034:	a8c77bfd 	ldp	x29, x30, [sp],#112
    8038:	d65f03c0 	ret

000000000000803c <pthread_tryjoin_np>:
    803c:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    8040:	910003fd 	mov	x29, sp
    8044:	f9421002 	ldr	x2, [x0,#1056]
    8048:	f9000bf3 	str	x19, [sp,#16]
    804c:	eb00005f 	cmp	x2, x0
    8050:	54000380 	b.eq	80c0 <pthread_tryjoin_np+0x84>
    8054:	d53bd042 	mrs	x2, tpidr_el0
    8058:	d11bc042 	sub	x2, x2, #0x6f0
    805c:	eb02001f 	cmp	x0, x2
    8060:	54000160 	b.eq	808c <pthread_tryjoin_np+0x50>
    8064:	f9421043 	ldr	x3, [x2,#1056]
    8068:	eb00007f 	cmp	x3, x0
    806c:	54000100 	b.eq	808c <pthread_tryjoin_np+0x50>
    8070:	b940d013 	ldr	w19, [x0,#208]
    8074:	34000173 	cbz	w19, 80a0 <pthread_tryjoin_np+0x64>
    8078:	52800213 	mov	w19, #0x10                  	// #16
    807c:	2a1303e0 	mov	w0, w19
    8080:	f9400bf3 	ldr	x19, [sp,#16]
    8084:	a8c37bfd 	ldp	x29, x30, [sp],#48
    8088:	d65f03c0 	ret
    808c:	52800473 	mov	w19, #0x23                  	// #35
    8090:	2a1303e0 	mov	w0, w19
    8094:	f9400bf3 	ldr	x19, [sp,#16]
    8098:	a8c37bfd 	ldp	x29, x30, [sp],#48
    809c:	d65f03c0 	ret
    80a0:	f90017bf 	str	xzr, [x29,#40]
    80a4:	91108003 	add	x3, x0, #0x420
    80a8:	c85ffc64 	ldaxr	x4, [x3]
    80ac:	eb1f009f 	cmp	x4, xzr
    80b0:	54000061 	b.ne	80bc <pthread_tryjoin_np+0x80>
    80b4:	c8057c62 	stxr	w5, x2, [x3]
    80b8:	35ffff85 	cbnz	w5, 80a8 <pthread_tryjoin_np+0x6c>
    80bc:	540000c0 	b.eq	80d4 <pthread_tryjoin_np+0x98>
    80c0:	528002d3 	mov	w19, #0x16                  	// #22
    80c4:	2a1303e0 	mov	w0, w19
    80c8:	f9400bf3 	ldr	x19, [sp,#16]
    80cc:	a8c37bfd 	ldp	x29, x30, [sp],#48
    80d0:	d65f03c0 	ret
    80d4:	b4000061 	cbz	x1, 80e0 <pthread_tryjoin_np+0xa4>
    80d8:	f9421402 	ldr	x2, [x0,#1064]
    80dc:	f9000022 	str	x2, [x1]
    80e0:	97fffb51 	bl	6e24 <__free_tcb>
    80e4:	17ffffe6 	b	807c <pthread_tryjoin_np+0x40>

00000000000080e8 <cleanup>:
    80e8:	f900001f 	str	xzr, [x0]
    80ec:	d65f03c0 	ret

00000000000080f0 <pthread_timedjoin_np>:
    80f0:	a9b97bfd 	stp	x29, x30, [sp,#-112]!
    80f4:	910003fd 	mov	x29, sp
    80f8:	b940d003 	ldr	w3, [x0,#208]
    80fc:	a90153f3 	stp	x19, x20, [sp,#16]
    8100:	a9025bf5 	stp	x21, x22, [sp,#32]
    8104:	f9001bf7 	str	x23, [sp,#48]
    8108:	37f80a43 	tbnz	w3, #31, 8250 <pthread_timedjoin_np+0x160>
    810c:	f9421003 	ldr	x3, [x0,#1056]
    8110:	eb00007f 	cmp	x3, x0
    8114:	54000220 	b.eq	8158 <pthread_timedjoin_np+0x68>
    8118:	d53bd043 	mrs	x3, tpidr_el0
    811c:	d11bc063 	sub	x3, x3, #0x6f0
    8120:	eb03001f 	cmp	x0, x3
    8124:	54000640 	b.eq	81ec <pthread_timedjoin_np+0xfc>
    8128:	f9421065 	ldr	x5, [x3,#1056]
    812c:	52800464 	mov	w4, #0x23                  	// #35
    8130:	eb0000bf 	cmp	x5, x0
    8134:	54000140 	b.eq	815c <pthread_timedjoin_np+0x6c>
    8138:	f9002bbf 	str	xzr, [x29,#80]
    813c:	91108004 	add	x4, x0, #0x420
    8140:	c85ffc85 	ldaxr	x5, [x4]
    8144:	eb1f00bf 	cmp	x5, xzr
    8148:	54000061 	b.ne	8154 <pthread_timedjoin_np+0x64>
    814c:	c8067c83 	stxr	w6, x3, [x4]
    8150:	35ffff86 	cbnz	w6, 8140 <pthread_timedjoin_np+0x50>
    8154:	54000100 	b.eq	8174 <pthread_timedjoin_np+0x84>
    8158:	528002c4 	mov	w4, #0x16                  	// #22
    815c:	2a0403e0 	mov	w0, w4
    8160:	f9401bf7 	ldr	x23, [sp,#48]
    8164:	a94153f3 	ldp	x19, x20, [sp,#16]
    8168:	a9425bf5 	ldp	x21, x22, [sp,#32]
    816c:	a8c77bfd 	ldp	x29, x30, [sp],#112
    8170:	d65f03c0 	ret
    8174:	aa0103f5 	mov	x21, x1
    8178:	910143b6 	add	x22, x29, #0x50
    817c:	90000001 	adrp	x1, 8000 <pthread_join+0x13c>
    8180:	aa0003f3 	mov	x19, x0
    8184:	9103a021 	add	x1, x1, #0xe8
    8188:	aa1603e0 	mov	x0, x22
    818c:	aa0203f4 	mov	x20, x2
    8190:	aa0403e2 	mov	x2, x4
    8194:	94001c1b 	bl	f200 <_pthread_cleanup_push>
    8198:	94001d20 	bl	f618 <__pthread_enable_asynccancel>
    819c:	2a0003f7 	mov	w23, w0
    81a0:	b940d261 	ldr	w1, [x19,#208]
    81a4:	35000321 	cbnz	w1, 8208 <pthread_timedjoin_np+0x118>
    81a8:	f90027a1 	str	x1, [x29,#72]
    81ac:	94001d4b 	bl	f6d8 <__pthread_disable_asynccancel>
    81b0:	f94027a1 	ldr	x1, [x29,#72]
    81b4:	aa1603e0 	mov	x0, x22
    81b8:	94001c1a 	bl	f220 <_pthread_cleanup_pop>
    81bc:	b4000075 	cbz	x21, 81c8 <pthread_timedjoin_np+0xd8>
    81c0:	f9421660 	ldr	x0, [x19,#1064]
    81c4:	f90002a0 	str	x0, [x21]
    81c8:	aa1303e0 	mov	x0, x19
    81cc:	97fffb16 	bl	6e24 <__free_tcb>
    81d0:	f9401bf7 	ldr	x23, [sp,#48]
    81d4:	52800004 	mov	w4, #0x0                   	// #0
    81d8:	2a0403e0 	mov	w0, w4
    81dc:	a94153f3 	ldp	x19, x20, [sp,#16]
    81e0:	a9425bf5 	ldp	x21, x22, [sp,#32]
    81e4:	a8c77bfd 	ldp	x29, x30, [sp],#112
    81e8:	d65f03c0 	ret
    81ec:	52800464 	mov	w4, #0x23                  	// #35
    81f0:	f9401bf7 	ldr	x23, [sp,#48]
    81f4:	2a0403e0 	mov	w0, w4
    81f8:	a94153f3 	ldp	x19, x20, [sp,#16]
    81fc:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8200:	a8c77bfd 	ldp	x29, x30, [sp],#112
    8204:	d65f03c0 	ret
    8208:	aa1403e1 	mov	x1, x20
    820c:	91034260 	add	x0, x19, #0xd0
    8210:	94001dcf 	bl	f94c <__lll_timedwait_tid>
    8214:	2a0003f4 	mov	w20, w0
    8218:	2a1703e0 	mov	w0, w23
    821c:	94001d2f 	bl	f6d8 <__pthread_disable_asynccancel>
    8220:	aa1603e0 	mov	x0, x22
    8224:	52800001 	mov	w1, #0x0                   	// #0
    8228:	94001bfe 	bl	f220 <_pthread_cleanup_pop>
    822c:	34fffc94 	cbz	w20, 81bc <pthread_timedjoin_np+0xcc>
    8230:	f902127f 	str	xzr, [x19,#1056]
    8234:	2a1403e4 	mov	w4, w20
    8238:	2a0403e0 	mov	w0, w4
    823c:	f9401bf7 	ldr	x23, [sp,#48]
    8240:	a94153f3 	ldp	x19, x20, [sp,#16]
    8244:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8248:	a8c77bfd 	ldp	x29, x30, [sp],#112
    824c:	d65f03c0 	ret
    8250:	52800064 	mov	w4, #0x3                   	// #3
    8254:	f9401bf7 	ldr	x23, [sp,#48]
    8258:	2a0403e0 	mov	w0, w4
    825c:	a94153f3 	ldp	x19, x20, [sp,#16]
    8260:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8264:	a8c77bfd 	ldp	x29, x30, [sp],#112
    8268:	d65f03c0 	ret

000000000000826c <pthread_self>:
    826c:	d53bd040 	mrs	x0, tpidr_el0
    8270:	d11bc000 	sub	x0, x0, #0x6f0
    8274:	d65f03c0 	ret

0000000000008278 <pthread_equal>:
    8278:	eb01001f 	cmp	x0, x1
    827c:	1a9f17e0 	cset	w0, eq
    8280:	d65f03c0 	ret

0000000000008284 <pthread_yield>:
    8284:	17fff3db 	b	51f0 <sched_yield@plt>

0000000000008288 <pthread_getconcurrency>:
    8288:	90000160 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    828c:	b9437800 	ldr	w0, [x0,#888]
    8290:	d65f03c0 	ret

0000000000008294 <pthread_setconcurrency>:
    8294:	2a0003e1 	mov	w1, w0
    8298:	528002c0 	mov	w0, #0x16                  	// #22
    829c:	37f80081 	tbnz	w1, #31, 82ac <pthread_setconcurrency+0x18>
    82a0:	90000162 	adrp	x2, 34000 <__GI___pthread_keys+0x3d78>
    82a4:	52800000 	mov	w0, #0x0                   	// #0
    82a8:	b9037841 	str	w1, [x2,#888]
    82ac:	d65f03c0 	ret

00000000000082b0 <pthread_getschedparam>:
    82b0:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    82b4:	910003fd 	mov	x29, sp
    82b8:	b940d003 	ldr	w3, [x0,#208]
    82bc:	a90153f3 	stp	x19, x20, [sp,#16]
    82c0:	a9025bf5 	stp	x21, x22, [sp,#32]
    82c4:	6b1f007f 	cmp	w3, wzr
    82c8:	5400074d 	b.le	83b0 <pthread_getschedparam+0x100>
    82cc:	aa0003f3 	mov	x19, x0
    82d0:	91106014 	add	x20, x0, #0x418
    82d4:	aa0203f6 	mov	x22, x2
    82d8:	aa0103f5 	mov	x21, x1
    82dc:	b9003fbf 	str	wzr, [x29,#60]
    82e0:	52800020 	mov	w0, #0x1                   	// #1
    82e4:	885ffe81 	ldaxr	w1, [x20]
    82e8:	6b1f003f 	cmp	w1, wzr
    82ec:	54000061 	b.ne	82f8 <pthread_getschedparam+0x48>
    82f0:	88027e80 	stxr	w2, w0, [x20]
    82f4:	35ffff82 	cbnz	w2, 82e4 <pthread_getschedparam+0x34>
    82f8:	54000281 	b.ne	8348 <pthread_getschedparam+0x98>
    82fc:	b9410e60 	ldr	w0, [x19,#268]
    8300:	362802e0 	tbz	w0, #5, 835c <pthread_getschedparam+0xac>
    8304:	363003c0 	tbz	w0, #6, 837c <pthread_getschedparam+0xcc>
    8308:	b9443660 	ldr	w0, [x19,#1076]
    830c:	52800004 	mov	w4, #0x0                   	// #0
    8310:	b90002a0 	str	w0, [x21]
    8314:	b9443260 	ldr	w0, [x19,#1072]
    8318:	b90002c0 	str	w0, [x22]
    831c:	52800001 	mov	w1, #0x0                   	// #0
    8320:	885f7e80 	ldxr	w0, [x20]
    8324:	8802fe81 	stlxr	w2, w1, [x20]
    8328:	35ffffc2 	cbnz	w2, 8320 <pthread_getschedparam+0x70>
    832c:	7100041f 	cmp	w0, #0x1
    8330:	540004cc 	b.gt	83c8 <pthread_getschedparam+0x118>
    8334:	2a0403e0 	mov	w0, w4
    8338:	a94153f3 	ldp	x19, x20, [sp,#16]
    833c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8340:	a8c47bfd 	ldp	x29, x30, [sp],#64
    8344:	d65f03c0 	ret
    8348:	aa1403e0 	mov	x0, x20
    834c:	b9003fa1 	str	w1, [x29,#60]
    8350:	94001d06 	bl	f768 <__lll_lock_wait_private>
    8354:	b9410e60 	ldr	w0, [x19,#268]
    8358:	372ffd60 	tbnz	w0, #5, 8304 <pthread_getschedparam+0x54>
    835c:	b940d260 	ldr	w0, [x19,#208]
    8360:	9110c261 	add	x1, x19, #0x430
    8364:	97fff373 	bl	5130 <__sched_getparam@plt>
    8368:	340001c0 	cbz	w0, 83a0 <pthread_getschedparam+0xf0>
    836c:	b9410e60 	ldr	w0, [x19,#268]
    8370:	363003a0 	tbz	w0, #6, 83e4 <pthread_getschedparam+0x134>
    8374:	52800024 	mov	w4, #0x1                   	// #1
    8378:	17ffffe9 	b	831c <pthread_getschedparam+0x6c>
    837c:	b940d260 	ldr	w0, [x19,#208]
    8380:	97fff3ec 	bl	5330 <__sched_getscheduler@plt>
    8384:	3100041f 	cmn	w0, #0x1
    8388:	b9043660 	str	w0, [x19,#1076]
    838c:	54ffff40 	b.eq	8374 <pthread_getschedparam+0xc4>
    8390:	b9410e60 	ldr	w0, [x19,#268]
    8394:	321a0000 	orr	w0, w0, #0x40
    8398:	b9010e60 	str	w0, [x19,#268]
    839c:	17ffffdb 	b	8308 <pthread_getschedparam+0x58>
    83a0:	b9410e60 	ldr	w0, [x19,#268]
    83a4:	321b0000 	orr	w0, w0, #0x20
    83a8:	b9010e60 	str	w0, [x19,#268]
    83ac:	17ffffd6 	b	8304 <pthread_getschedparam+0x54>
    83b0:	52800064 	mov	w4, #0x3                   	// #3
    83b4:	2a0403e0 	mov	w0, w4
    83b8:	a94153f3 	ldp	x19, x20, [sp,#16]
    83bc:	a9425bf5 	ldp	x21, x22, [sp,#32]
    83c0:	a8c47bfd 	ldp	x29, x30, [sp],#64
    83c4:	d65f03c0 	ret
    83c8:	aa1403e0 	mov	x0, x20
    83cc:	d2801021 	mov	x1, #0x81                  	// #129
    83d0:	d2800022 	mov	x2, #0x1                   	// #1
    83d4:	d2800003 	mov	x3, #0x0                   	// #0
    83d8:	d2800c48 	mov	x8, #0x62                  	// #98
    83dc:	d4000001 	svc	#0x0
    83e0:	17ffffd5 	b	8334 <pthread_getschedparam+0x84>
    83e4:	b940d260 	ldr	w0, [x19,#208]
    83e8:	97fff3d2 	bl	5330 <__sched_getscheduler@plt>
    83ec:	3100041f 	cmn	w0, #0x1
    83f0:	b9043660 	str	w0, [x19,#1076]
    83f4:	54fffc00 	b.eq	8374 <pthread_getschedparam+0xc4>
    83f8:	b9410e60 	ldr	w0, [x19,#268]
    83fc:	52800024 	mov	w4, #0x1                   	// #1
    8400:	321a0000 	orr	w0, w0, #0x40
    8404:	b9010e60 	str	w0, [x19,#268]
    8408:	17ffffc5 	b	831c <pthread_getschedparam+0x6c>

000000000000840c <pthread_setschedparam>:
    840c:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    8410:	910003fd 	mov	x29, sp
    8414:	b940d003 	ldr	w3, [x0,#208]
    8418:	a90153f3 	stp	x19, x20, [sp,#16]
    841c:	a9025bf5 	stp	x21, x22, [sp,#32]
    8420:	6b1f007f 	cmp	w3, wzr
    8424:	5400056d 	b.le	84d0 <pthread_setschedparam+0xc4>
    8428:	aa0003f3 	mov	x19, x0
    842c:	91106014 	add	x20, x0, #0x418
    8430:	aa0203f5 	mov	x21, x2
    8434:	2a0103f6 	mov	w22, w1
    8438:	b9003bbf 	str	wzr, [x29,#56]
    843c:	52800020 	mov	w0, #0x1                   	// #1
    8440:	885ffe81 	ldaxr	w1, [x20]
    8444:	6b1f003f 	cmp	w1, wzr
    8448:	54000061 	b.ne	8454 <pthread_setschedparam+0x48>
    844c:	88027e80 	stxr	w2, w0, [x20]
    8450:	35ffff82 	cbnz	w2, 8440 <pthread_setschedparam+0x34>
    8454:	54000361 	b.ne	84c0 <pthread_setschedparam+0xb4>
    8458:	f9425a60 	ldr	x0, [x19,#1200]
    845c:	aa1503e2 	mov	x2, x21
    8460:	b5000440 	cbnz	x0, 84e8 <pthread_setschedparam+0xdc>
    8464:	b940d260 	ldr	w0, [x19,#208]
    8468:	2a1603e1 	mov	w1, w22
    846c:	97fff389 	bl	5290 <__sched_setscheduler@plt>
    8470:	3100041f 	cmn	w0, #0x1
    8474:	54000480 	b.eq	8504 <pthread_setschedparam+0xf8>
    8478:	b9043676 	str	w22, [x19,#1076]
    847c:	52800004 	mov	w4, #0x0                   	// #0
    8480:	b9410e60 	ldr	w0, [x19,#268]
    8484:	b94002a1 	ldr	w1, [x21]
    8488:	321b0400 	orr	w0, w0, #0x60
    848c:	b9043261 	str	w1, [x19,#1072]
    8490:	b9010e60 	str	w0, [x19,#268]
    8494:	52800001 	mov	w1, #0x0                   	// #0
    8498:	885f7e80 	ldxr	w0, [x20]
    849c:	8802fe81 	stlxr	w2, w1, [x20]
    84a0:	35ffffc2 	cbnz	w2, 8498 <pthread_setschedparam+0x8c>
    84a4:	7100041f 	cmp	w0, #0x1
    84a8:	5400038c 	b.gt	8518 <pthread_setschedparam+0x10c>
    84ac:	2a0403e0 	mov	w0, w4
    84b0:	a94153f3 	ldp	x19, x20, [sp,#16]
    84b4:	a9425bf5 	ldp	x21, x22, [sp,#32]
    84b8:	a8c47bfd 	ldp	x29, x30, [sp],#64
    84bc:	d65f03c0 	ret
    84c0:	aa1403e0 	mov	x0, x20
    84c4:	b9003ba1 	str	w1, [x29,#56]
    84c8:	94001ca8 	bl	f768 <__lll_lock_wait_private>
    84cc:	17ffffe3 	b	8458 <pthread_setschedparam+0x4c>
    84d0:	52800064 	mov	w4, #0x3                   	// #3
    84d4:	2a0403e0 	mov	w0, w4
    84d8:	a94153f3 	ldp	x19, x20, [sp,#16]
    84dc:	a9425bf5 	ldp	x21, x22, [sp,#32]
    84e0:	a8c47bfd 	ldp	x29, x30, [sp],#64
    84e4:	d65f03c0 	ret
    84e8:	b9400000 	ldr	w0, [x0]
    84ec:	b94002a1 	ldr	w1, [x21]
    84f0:	6b01001f 	cmp	w0, w1
    84f4:	54fffb8d 	b.le	8464 <pthread_setschedparam+0x58>
    84f8:	910103a2 	add	x2, x29, #0x40
    84fc:	b81f8c40 	str	w0, [x2,#-8]!
    8500:	17ffffd9 	b	8464 <pthread_setschedparam+0x58>
    8504:	d53bd040 	mrs	x0, tpidr_el0
    8508:	f0000121 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    850c:	f947c421 	ldr	x1, [x1,#3976]
    8510:	b8616804 	ldr	w4, [x0,x1]
    8514:	17ffffe0 	b	8494 <pthread_setschedparam+0x88>
    8518:	aa1403e0 	mov	x0, x20
    851c:	d2801021 	mov	x1, #0x81                  	// #129
    8520:	d2800022 	mov	x2, #0x1                   	// #1
    8524:	d2800003 	mov	x3, #0x0                   	// #0
    8528:	d2800c48 	mov	x8, #0x62                  	// #98
    852c:	d4000001 	svc	#0x0
    8530:	17ffffdf 	b	84ac <pthread_setschedparam+0xa0>

0000000000008534 <pthread_setschedprio>:
    8534:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    8538:	910003fd 	mov	x29, sp
    853c:	b940d002 	ldr	w2, [x0,#208]
    8540:	a90153f3 	stp	x19, x20, [sp,#16]
    8544:	f90013f5 	str	x21, [sp,#32]
    8548:	6b1f005f 	cmp	w2, wzr
    854c:	5400052d 	b.le	85f0 <pthread_setschedprio+0xbc>
    8550:	aa0003f3 	mov	x19, x0
    8554:	91106015 	add	x21, x0, #0x418
    8558:	2a0103f4 	mov	w20, w1
    855c:	b9003ba1 	str	w1, [x29,#56]
    8560:	b9003fbf 	str	wzr, [x29,#60]
    8564:	52800020 	mov	w0, #0x1                   	// #1
    8568:	885ffea1 	ldaxr	w1, [x21]
    856c:	6b1f003f 	cmp	w1, wzr
    8570:	54000061 	b.ne	857c <pthread_setschedprio+0x48>
    8574:	88027ea0 	stxr	w2, w0, [x21]
    8578:	35ffff82 	cbnz	w2, 8568 <pthread_setschedprio+0x34>
    857c:	54000321 	b.ne	85e0 <pthread_setschedprio+0xac>
    8580:	f9425a60 	ldr	x0, [x19,#1200]
    8584:	b5000420 	cbnz	x0, 8608 <pthread_setschedprio+0xd4>
    8588:	b940d260 	ldr	w0, [x19,#208]
    858c:	9100e3a1 	add	x1, x29, #0x38
    8590:	97fff304 	bl	51a0 <sched_setparam@plt>
    8594:	3100041f 	cmn	w0, #0x1
    8598:	54000420 	b.eq	861c <pthread_setschedprio+0xe8>
    859c:	b9410e60 	ldr	w0, [x19,#268]
    85a0:	52800004 	mov	w4, #0x0                   	// #0
    85a4:	b9003bb4 	str	w20, [x29,#56]
    85a8:	321b0000 	orr	w0, w0, #0x20
    85ac:	b9043274 	str	w20, [x19,#1072]
    85b0:	b9010e60 	str	w0, [x19,#268]
    85b4:	52800001 	mov	w1, #0x0                   	// #0
    85b8:	885f7ea0 	ldxr	w0, [x21]
    85bc:	8802fea1 	stlxr	w2, w1, [x21]
    85c0:	35ffffc2 	cbnz	w2, 85b8 <pthread_setschedprio+0x84>
    85c4:	7100041f 	cmp	w0, #0x1
    85c8:	5400034c 	b.gt	8630 <pthread_setschedprio+0xfc>
    85cc:	2a0403e0 	mov	w0, w4
    85d0:	f94013f5 	ldr	x21, [sp,#32]
    85d4:	a94153f3 	ldp	x19, x20, [sp,#16]
    85d8:	a8c47bfd 	ldp	x29, x30, [sp],#64
    85dc:	d65f03c0 	ret
    85e0:	aa1503e0 	mov	x0, x21
    85e4:	b9003fa1 	str	w1, [x29,#60]
    85e8:	94001c60 	bl	f768 <__lll_lock_wait_private>
    85ec:	17ffffe5 	b	8580 <pthread_setschedprio+0x4c>
    85f0:	52800064 	mov	w4, #0x3                   	// #3
    85f4:	f94013f5 	ldr	x21, [sp,#32]
    85f8:	2a0403e0 	mov	w0, w4
    85fc:	a94153f3 	ldp	x19, x20, [sp,#16]
    8600:	a8c47bfd 	ldp	x29, x30, [sp],#64
    8604:	d65f03c0 	ret
    8608:	b9400000 	ldr	w0, [x0]
    860c:	6b00029f 	cmp	w20, w0
    8610:	54fffbca 	b.ge	8588 <pthread_setschedprio+0x54>
    8614:	b9003ba0 	str	w0, [x29,#56]
    8618:	17ffffdc 	b	8588 <pthread_setschedprio+0x54>
    861c:	d53bd040 	mrs	x0, tpidr_el0
    8620:	f0000121 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    8624:	f947c421 	ldr	x1, [x1,#3976]
    8628:	b8616804 	ldr	w4, [x0,x1]
    862c:	17ffffe2 	b	85b4 <pthread_setschedprio+0x80>
    8630:	aa1503e0 	mov	x0, x21
    8634:	d2801021 	mov	x1, #0x81                  	// #129
    8638:	d2800022 	mov	x2, #0x1                   	// #1
    863c:	d2800003 	mov	x3, #0x0                   	// #0
    8640:	d2800c48 	mov	x8, #0x62                  	// #98
    8644:	d4000001 	svc	#0x0
    8648:	17ffffe1 	b	85cc <pthread_setschedprio+0x98>

000000000000864c <pthread_attr_init@@GLIBC_2.17>:
    864c:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    8650:	910003fd 	mov	x29, sp
    8654:	f9000bf3 	str	x19, [sp,#16]
    8658:	aa0003f3 	mov	x19, x0
    865c:	a9007c1f 	stp	xzr, xzr, [x0]
    8660:	a9017c1f 	stp	xzr, xzr, [x0,#16]
    8664:	a9027c1f 	stp	xzr, xzr, [x0,#32]
    8668:	a9037c1f 	stp	xzr, xzr, [x0,#48]
    866c:	97fff265 	bl	5000 <__getpagesize@plt>
    8670:	93407c01 	sxtw	x1, w0
    8674:	f9000a61 	str	x1, [x19,#16]
    8678:	52800000 	mov	w0, #0x0                   	// #0
    867c:	f9400bf3 	ldr	x19, [sp,#16]
    8680:	a8c27bfd 	ldp	x29, x30, [sp],#32
    8684:	d65f03c0 	ret

0000000000008688 <pthread_attr_destroy>:
    8688:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    868c:	910003fd 	mov	x29, sp
    8690:	f9401400 	ldr	x0, [x0,#40]
    8694:	97fff2e3 	bl	5220 <free@plt>
    8698:	52800000 	mov	w0, #0x0                   	// #0
    869c:	a8c17bfd 	ldp	x29, x30, [sp],#16
    86a0:	d65f03c0 	ret

00000000000086a4 <pthread_attr_getdetachstate>:
    86a4:	b9400802 	ldr	w2, [x0,#8]
    86a8:	52800000 	mov	w0, #0x0                   	// #0
    86ac:	12000042 	and	w2, w2, #0x1
    86b0:	b9000022 	str	w2, [x1]
    86b4:	d65f03c0 	ret

00000000000086b8 <pthread_attr_setdetachstate>:
    86b8:	7100043f 	cmp	w1, #0x1
    86bc:	540000e0 	b.eq	86d8 <pthread_attr_setdetachstate+0x20>
    86c0:	35000181 	cbnz	w1, 86f0 <pthread_attr_setdetachstate+0x38>
    86c4:	b9400802 	ldr	w2, [x0,#8]
    86c8:	121f7842 	and	w2, w2, #0xfffffffe
    86cc:	b9000802 	str	w2, [x0,#8]
    86d0:	2a0103e0 	mov	w0, w1
    86d4:	d65f03c0 	ret
    86d8:	b9400802 	ldr	w2, [x0,#8]
    86dc:	52800001 	mov	w1, #0x0                   	// #0
    86e0:	32000042 	orr	w2, w2, #0x1
    86e4:	b9000802 	str	w2, [x0,#8]
    86e8:	2a0103e0 	mov	w0, w1
    86ec:	d65f03c0 	ret
    86f0:	528002c1 	mov	w1, #0x16                  	// #22
    86f4:	17fffff7 	b	86d0 <pthread_attr_setdetachstate+0x18>

00000000000086f8 <pthread_attr_getguardsize>:
    86f8:	f9400802 	ldr	x2, [x0,#16]
    86fc:	52800000 	mov	w0, #0x0                   	// #0
    8700:	f9000022 	str	x2, [x1]
    8704:	d65f03c0 	ret

0000000000008708 <pthread_attr_setguardsize>:
    8708:	f9000801 	str	x1, [x0,#16]
    870c:	52800000 	mov	w0, #0x0                   	// #0
    8710:	d65f03c0 	ret

0000000000008714 <pthread_attr_getschedparam>:
    8714:	b9400002 	ldr	w2, [x0]
    8718:	52800000 	mov	w0, #0x0                   	// #0
    871c:	b9000022 	str	w2, [x1]
    8720:	d65f03c0 	ret

0000000000008724 <pthread_attr_setschedparam>:
    8724:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    8728:	910003fd 	mov	x29, sp
    872c:	a90153f3 	stp	x19, x20, [sp,#16]
    8730:	b9400413 	ldr	w19, [x0,#4]
    8734:	a9025bf5 	stp	x21, x22, [sp,#32]
    8738:	aa0003f5 	mov	x21, x0
    873c:	2a1303e0 	mov	w0, w19
    8740:	b9400036 	ldr	w22, [x1]
    8744:	f9001fa1 	str	x1, [x29,#56]
    8748:	97fff28e 	bl	5180 <__sched_get_priority_min@plt>
    874c:	2a0003f4 	mov	w20, w0
    8750:	2a1303e0 	mov	w0, w19
    8754:	97fff2ff 	bl	5350 <__sched_get_priority_max@plt>
    8758:	37f80074 	tbnz	w20, #31, 8764 <pthread_attr_setschedparam+0x40>
    875c:	f9401fa1 	ldr	x1, [x29,#56]
    8760:	36f800c0 	tbz	w0, #31, 8778 <pthread_attr_setschedparam+0x54>
    8764:	528002c0 	mov	w0, #0x16                  	// #22
    8768:	a94153f3 	ldp	x19, x20, [sp,#16]
    876c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8770:	a8c47bfd 	ldp	x29, x30, [sp],#64
    8774:	d65f03c0 	ret
    8778:	6b1402df 	cmp	w22, w20
    877c:	54ffff4b 	b.lt	8764 <pthread_attr_setschedparam+0x40>
    8780:	6b0002df 	cmp	w22, w0
    8784:	54ffff0c 	b.gt	8764 <pthread_attr_setschedparam+0x40>
    8788:	b9400aa2 	ldr	w2, [x21,#8]
    878c:	52800000 	mov	w0, #0x0                   	// #0
    8790:	b9400021 	ldr	w1, [x1]
    8794:	b90002a1 	str	w1, [x21]
    8798:	321b0042 	orr	w2, w2, #0x20
    879c:	b9000aa2 	str	w2, [x21,#8]
    87a0:	a94153f3 	ldp	x19, x20, [sp,#16]
    87a4:	a9425bf5 	ldp	x21, x22, [sp,#32]
    87a8:	a8c47bfd 	ldp	x29, x30, [sp],#64
    87ac:	d65f03c0 	ret

00000000000087b0 <pthread_attr_getschedpolicy>:
    87b0:	b9400402 	ldr	w2, [x0,#4]
    87b4:	52800000 	mov	w0, #0x0                   	// #0
    87b8:	b9000022 	str	w2, [x1]
    87bc:	d65f03c0 	ret

00000000000087c0 <pthread_attr_setschedpolicy>:
    87c0:	7100083f 	cmp	w1, #0x2
    87c4:	528002c2 	mov	w2, #0x16                  	// #22
    87c8:	540000c8 	b.hi	87e0 <pthread_attr_setschedpolicy+0x20>
    87cc:	b9400803 	ldr	w3, [x0,#8]
    87d0:	52800002 	mov	w2, #0x0                   	// #0
    87d4:	b9000401 	str	w1, [x0,#4]
    87d8:	321a0063 	orr	w3, w3, #0x40
    87dc:	b9000803 	str	w3, [x0,#8]
    87e0:	2a0203e0 	mov	w0, w2
    87e4:	d65f03c0 	ret

00000000000087e8 <pthread_attr_getinheritsched>:
    87e8:	b9400802 	ldr	w2, [x0,#8]
    87ec:	52800000 	mov	w0, #0x0                   	// #0
    87f0:	d3410442 	ubfx	x2, x2, #1, #1
    87f4:	b9000022 	str	w2, [x1]
    87f8:	d65f03c0 	ret

00000000000087fc <pthread_attr_setinheritsched>:
    87fc:	7100043f 	cmp	w1, #0x1
    8800:	aa0003e2 	mov	x2, x0
    8804:	528002c0 	mov	w0, #0x16                  	// #22
    8808:	54000049 	b.ls	8810 <pthread_attr_setinheritsched+0x14>
    880c:	d65f03c0 	ret
    8810:	350000c1 	cbnz	w1, 8828 <pthread_attr_setinheritsched+0x2c>
    8814:	b9400843 	ldr	w3, [x2,#8]
    8818:	2a0103e0 	mov	w0, w1
    881c:	121e7863 	and	w3, w3, #0xfffffffd
    8820:	b9000843 	str	w3, [x2,#8]
    8824:	d65f03c0 	ret
    8828:	b9400841 	ldr	w1, [x2,#8]
    882c:	52800000 	mov	w0, #0x0                   	// #0
    8830:	321f0021 	orr	w1, w1, #0x2
    8834:	b9000841 	str	w1, [x2,#8]
    8838:	d65f03c0 	ret

000000000000883c <pthread_attr_getscope>:
    883c:	b9400802 	ldr	w2, [x0,#8]
    8840:	52800000 	mov	w0, #0x0                   	// #0
    8844:	d3420842 	ubfx	x2, x2, #2, #1
    8848:	b9000022 	str	w2, [x1]
    884c:	d65f03c0 	ret

0000000000008850 <pthread_attr_setscope>:
    8850:	340000e1 	cbz	w1, 886c <pthread_attr_setscope+0x1c>
    8854:	7100043f 	cmp	w1, #0x1
    8858:	52800be0 	mov	w0, #0x5f                  	// #95
    885c:	528002c1 	mov	w1, #0x16                  	// #22
    8860:	1a810001 	csel	w1, w0, w1, eq
    8864:	2a0103e0 	mov	w0, w1
    8868:	d65f03c0 	ret
    886c:	b9400802 	ldr	w2, [x0,#8]
    8870:	121d7842 	and	w2, w2, #0xfffffffb
    8874:	b9000802 	str	w2, [x0,#8]
    8878:	2a0103e0 	mov	w0, w1
    887c:	d65f03c0 	ret

0000000000008880 <pthread_attr_getstackaddr>:
    8880:	f9400c02 	ldr	x2, [x0,#24]
    8884:	52800000 	mov	w0, #0x0                   	// #0
    8888:	f9000022 	str	x2, [x1]
    888c:	d65f03c0 	ret

0000000000008890 <pthread_attr_setstackaddr>:
    8890:	aa0003e2 	mov	x2, x0
    8894:	52800000 	mov	w0, #0x0                   	// #0
    8898:	b9400843 	ldr	w3, [x2,#8]
    889c:	f9000c41 	str	x1, [x2,#24]
    88a0:	321d0063 	orr	w3, w3, #0x8
    88a4:	b9000843 	str	w3, [x2,#8]
    88a8:	d65f03c0 	ret

00000000000088ac <pthread_attr_getstacksize>:
    88ac:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    88b0:	910003fd 	mov	x29, sp
    88b4:	f9401004 	ldr	x4, [x0,#32]
    88b8:	a90153f3 	stp	x19, x20, [sp,#16]
    88bc:	aa0103f4 	mov	x20, x1
    88c0:	b40000c4 	cbz	x4, 88d8 <pthread_attr_getstacksize+0x2c>
    88c4:	f9000284 	str	x4, [x20]
    88c8:	52800000 	mov	w0, #0x0                   	// #0
    88cc:	a94153f3 	ldp	x19, x20, [sp,#16]
    88d0:	a8c37bfd 	ldp	x29, x30, [sp],#48
    88d4:	d65f03c0 	ret
    88d8:	90000173 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
    88dc:	b9002fa4 	str	w4, [x29,#44]
    88e0:	910a2260 	add	x0, x19, #0x288
    88e4:	52800021 	mov	w1, #0x1                   	// #1
    88e8:	885ffc02 	ldaxr	w2, [x0]
    88ec:	6b1f005f 	cmp	w2, wzr
    88f0:	54000061 	b.ne	88fc <pthread_attr_getstacksize+0x50>
    88f4:	88037c01 	stxr	w3, w1, [x0]
    88f8:	35ffff83 	cbnz	w3, 88e8 <pthread_attr_getstacksize+0x3c>
    88fc:	54000201 	b.ne	893c <pthread_attr_getstacksize+0x90>
    8900:	90000161 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    8904:	910a2260 	add	x0, x19, #0x288
    8908:	52800002 	mov	w2, #0x0                   	// #0
    890c:	f941a824 	ldr	x4, [x1,#848]
    8910:	885f7c01 	ldxr	w1, [x0]
    8914:	8803fc02 	stlxr	w3, w2, [x0]
    8918:	35ffffc3 	cbnz	w3, 8910 <pthread_attr_getstacksize+0x64>
    891c:	7100043f 	cmp	w1, #0x1
    8920:	54fffd2d 	b.le	88c4 <pthread_attr_getstacksize+0x18>
    8924:	d2801021 	mov	x1, #0x81                  	// #129
    8928:	d2800022 	mov	x2, #0x1                   	// #1
    892c:	d2800003 	mov	x3, #0x0                   	// #0
    8930:	d2800c48 	mov	x8, #0x62                  	// #98
    8934:	d4000001 	svc	#0x0
    8938:	17ffffe3 	b	88c4 <pthread_attr_getstacksize+0x18>
    893c:	b9002fa2 	str	w2, [x29,#44]
    8940:	94001b8a 	bl	f768 <__lll_lock_wait_private>
    8944:	17ffffef 	b	8900 <pthread_attr_getstacksize+0x54>

0000000000008948 <pthread_attr_setstacksize@@GLIBC_2.17>:
    8948:	b24043e2 	mov	x2, #0x1ffff               	// #131071
    894c:	eb02003f 	cmp	x1, x2
    8950:	54000068 	b.hi	895c <pthread_attr_setstacksize@@GLIBC_2.17+0x14>
    8954:	528002c0 	mov	w0, #0x16                  	// #22
    8958:	d65f03c0 	ret
    895c:	f9001001 	str	x1, [x0,#32]
    8960:	52800000 	mov	w0, #0x0                   	// #0
    8964:	d65f03c0 	ret

0000000000008968 <pthread_attr_getstack>:
    8968:	aa0003e3 	mov	x3, x0
    896c:	52800000 	mov	w0, #0x0                   	// #0
    8970:	f9401064 	ldr	x4, [x3,#32]
    8974:	f9400c63 	ldr	x3, [x3,#24]
    8978:	cb040063 	sub	x3, x3, x4
    897c:	f9000023 	str	x3, [x1]
    8980:	f9000044 	str	x4, [x2]
    8984:	d65f03c0 	ret

0000000000008988 <pthread_attr_setstack@@GLIBC_2.17>:
    8988:	b24043e3 	mov	x3, #0x1ffff               	// #131071
    898c:	eb03005f 	cmp	x2, x3
    8990:	54000088 	b.hi	89a0 <pthread_attr_setstack@@GLIBC_2.17+0x18>
    8994:	528002c2 	mov	w2, #0x16                  	// #22
    8998:	2a0203e0 	mov	w0, w2
    899c:	d65f03c0 	ret
    89a0:	b9400803 	ldr	w3, [x0,#8]
    89a4:	8b020021 	add	x1, x1, x2
    89a8:	f9001002 	str	x2, [x0,#32]
    89ac:	52800002 	mov	w2, #0x0                   	// #0
    89b0:	321d0063 	orr	w3, w3, #0x8
    89b4:	f9000c01 	str	x1, [x0,#24]
    89b8:	b9000803 	str	w3, [x0,#8]
    89bc:	2a0203e0 	mov	w0, w2
    89c0:	d65f03c0 	ret

00000000000089c4 <pthread_getattr_np>:
    89c4:	a9b57bfd 	stp	x29, x30, [sp,#-176]!
    89c8:	910003fd 	mov	x29, sp
    89cc:	6d0627e8 	stp	d8, d9, [sp,#96]
    89d0:	a9025bf5 	stp	x21, x22, [sp,#32]
    89d4:	a90363f7 	stp	x23, x24, [sp,#48]
    89d8:	a9046bf9 	stp	x25, x26, [sp,#64]
    89dc:	aa0003f5 	mov	x21, x0
    89e0:	91106019 	add	x25, x0, #0x418
    89e4:	a90153f3 	stp	x19, x20, [sp,#16]
    89e8:	a90573fb 	stp	x27, x28, [sp,#80]
    89ec:	b900a3bf 	str	wzr, [x29,#160]
    89f0:	aa0103f7 	mov	x23, x1
    89f4:	52800020 	mov	w0, #0x1                   	// #1
    89f8:	fd003bea 	str	d10, [sp,#112]
    89fc:	885fff21 	ldaxr	w1, [x25]
    8a00:	6b1f003f 	cmp	w1, wzr
    8a04:	54000061 	b.ne	8a10 <pthread_getattr_np+0x4c>
    8a08:	88027f20 	stxr	w2, w0, [x25]
    8a0c:	35ffff82 	cbnz	w2, 89fc <pthread_getattr_np+0x38>
    8a10:	54000741 	b.ne	8af8 <pthread_getattr_np+0x134>
    8a14:	b94432a0 	ldr	w0, [x21,#1072]
    8a18:	b90002e0 	str	w0, [x23]
    8a1c:	f94212a2 	ldr	x2, [x21,#1056]
    8a20:	b94436a1 	ldr	w1, [x21,#1076]
    8a24:	b9410ea0 	ldr	w0, [x21,#268]
    8a28:	eb15005f 	cmp	x2, x21
    8a2c:	b90006e1 	str	w1, [x23,#4]
    8a30:	b9000ae0 	str	w0, [x23,#8]
    8a34:	54000740 	b.eq	8b1c <pthread_getattr_np+0x158>
    8a38:	f94256a1 	ldr	x1, [x21,#1192]
    8a3c:	f9424ab3 	ldr	x19, [x21,#1168]
    8a40:	f9000ae1 	str	x1, [x23,#16]
    8a44:	b4000793 	cbz	x19, 8b34 <pthread_getattr_np+0x170>
    8a48:	f9424ea1 	ldr	x1, [x21,#1176]
    8a4c:	321d0000 	orr	w0, w0, #0x8
    8a50:	f90012e1 	str	x1, [x23,#32]
    8a54:	b9000ae0 	str	w0, [x23,#8]
    8a58:	8b010261 	add	x1, x19, x1
    8a5c:	f9000ee1 	str	x1, [x23,#24]
    8a60:	d2800016 	mov	x22, #0x0                   	// #0
    8a64:	d2800213 	mov	x19, #0x10                  	// #16
    8a68:	b2404ff8 	mov	x24, #0xfffff               	// #1048575
    8a6c:	14000007 	b	8a88 <pthread_getattr_np+0xc4>
    8a70:	9400227e 	bl	11468 <pthread_getaffinity_np@@GLIBC_2.17>
    8a74:	7100581f 	cmp	w0, #0x16
    8a78:	aa1403f6 	mov	x22, x20
    8a7c:	54000461 	b.ne	8b08 <pthread_getattr_np+0x144>
    8a80:	eb18027f 	cmp	x19, x24
    8a84:	54000428 	b.hi	8b08 <pthread_getattr_np+0x144>
    8a88:	d37ffa73 	lsl	x19, x19, #1
    8a8c:	aa1603e0 	mov	x0, x22
    8a90:	aa1303e1 	mov	x1, x19
    8a94:	97fff19f 	bl	5110 <realloc@plt>
    8a98:	aa0003f4 	mov	x20, x0
    8a9c:	aa1303e1 	mov	x1, x19
    8aa0:	aa1503e0 	mov	x0, x21
    8aa4:	aa1403e2 	mov	x2, x20
    8aa8:	b5fffe54 	cbnz	x20, 8a70 <pthread_getattr_np+0xac>
    8aac:	aa1603e0 	mov	x0, x22
    8ab0:	52800196 	mov	w22, #0xc                   	// #12
    8ab4:	97fff1db 	bl	5220 <free@plt>
    8ab8:	52800001 	mov	w1, #0x0                   	// #0
    8abc:	885f7f20 	ldxr	w0, [x25]
    8ac0:	8802ff21 	stlxr	w2, w1, [x25]
    8ac4:	35ffffc2 	cbnz	w2, 8abc <pthread_getattr_np+0xf8>
    8ac8:	7100041f 	cmp	w0, #0x1
    8acc:	5400096c 	b.gt	8bf8 <pthread_getattr_np+0x234>
    8ad0:	2a1603e0 	mov	w0, w22
    8ad4:	6d4627e8 	ldp	d8, d9, [sp,#96]
    8ad8:	a94153f3 	ldp	x19, x20, [sp,#16]
    8adc:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8ae0:	a94363f7 	ldp	x23, x24, [sp,#48]
    8ae4:	a9446bf9 	ldp	x25, x26, [sp,#64]
    8ae8:	a94573fb 	ldp	x27, x28, [sp,#80]
    8aec:	fd403bea 	ldr	d10, [sp,#112]
    8af0:	a8cb7bfd 	ldp	x29, x30, [sp],#176
    8af4:	d65f03c0 	ret
    8af8:	aa1903e0 	mov	x0, x25
    8afc:	b900a3a1 	str	w1, [x29,#160]
    8b00:	94001b1a 	bl	f768 <__lll_lock_wait_private>
    8b04:	17ffffc4 	b	8a14 <pthread_getattr_np+0x50>
    8b08:	2a0003f6 	mov	w22, w0
    8b0c:	35000f40 	cbnz	w0, 8cf4 <pthread_getattr_np+0x330>
    8b10:	f90016f4 	str	x20, [x23,#40]
    8b14:	f9001af3 	str	x19, [x23,#48]
    8b18:	17ffffe8 	b	8ab8 <pthread_getattr_np+0xf4>
    8b1c:	f94256a1 	ldr	x1, [x21,#1192]
    8b20:	32000000 	orr	w0, w0, #0x1
    8b24:	f9424ab3 	ldr	x19, [x21,#1168]
    8b28:	b9000ae0 	str	w0, [x23,#8]
    8b2c:	f9000ae1 	str	x1, [x23,#16]
    8b30:	b5fff8d3 	cbnz	x19, 8a48 <pthread_getattr_np+0x84>
    8b34:	d0000040 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
    8b38:	d0000041 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
    8b3c:	912e6000 	add	x0, x0, #0xb98
    8b40:	912ea021 	add	x1, x1, #0xba8
    8b44:	97fff14b 	bl	5070 <fopen@plt>
    8b48:	aa0003f4 	mov	x20, x0
    8b4c:	b4000a20 	cbz	x0, 8c90 <pthread_getattr_np+0x2cc>
    8b50:	52800060 	mov	w0, #0x3                   	// #3
    8b54:	910283a1 	add	x1, x29, #0xa0
    8b58:	97fff1da 	bl	52c0 <getrlimit@plt>
    8b5c:	35000740 	cbnz	w0, 8c44 <pthread_getattr_np+0x280>
    8b60:	f0000120 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    8b64:	b9400284 	ldr	w4, [x20]
    8b68:	9e670268 	fmov	d8, x19
    8b6c:	d0000058 	adrp	x24, 12000 <__pthread_current_priority+0xa8>
    8b70:	32110084 	orr	w4, w4, #0x8000
    8b74:	910203bb 	add	x27, x29, #0x80
    8b78:	f947e001 	ldr	x1, [x0,#4032]
    8b7c:	f0000120 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    8b80:	910223ba 	add	x26, x29, #0x88
    8b84:	912ec318 	add	x24, x24, #0xbb0
    8b88:	910243b6 	add	x22, x29, #0x90
    8b8c:	910263bc 	add	x28, x29, #0x98
    8b90:	f947d400 	ldr	x0, [x0,#4008]
    8b94:	f9400021 	ldr	x1, [x1]
    8b98:	b9000284 	str	w4, [x20]
    8b9c:	f9400c00 	ldr	x0, [x0,#24]
    8ba0:	9e67002a 	fmov	d10, x1
    8ba4:	f90043b3 	str	x19, [x29,#128]
    8ba8:	f90047b3 	str	x19, [x29,#136]
    8bac:	9e670009 	fmov	d9, x0
    8bb0:	aa1a03e1 	mov	x1, x26
    8bb4:	aa1403e3 	mov	x3, x20
    8bb8:	aa1b03e0 	mov	x0, x27
    8bbc:	52800142 	mov	w2, #0xa                   	// #10
    8bc0:	121c0093 	and	w19, w4, #0x10
    8bc4:	372005e4 	tbnz	w4, #4, 8c80 <pthread_getattr_np+0x2bc>
    8bc8:	97fff1fe 	bl	53c0 <__getdelim@plt>
    8bcc:	eb1f001f 	cmp	x0, xzr
    8bd0:	aa1803e1 	mov	x1, x24
    8bd4:	aa1603e2 	mov	x2, x22
    8bd8:	aa1c03e3 	mov	x3, x28
    8bdc:	5400052d 	b.le	8c80 <pthread_getattr_np+0x2bc>
    8be0:	f94043a0 	ldr	x0, [x29,#128]
    8be4:	97fff13b 	bl	50d0 <sscanf@plt>
    8be8:	7100081f 	cmp	w0, #0x2
    8bec:	54000140 	b.eq	8c14 <pthread_getattr_np+0x250>
    8bf0:	b9400284 	ldr	w4, [x20]
    8bf4:	17ffffef 	b	8bb0 <pthread_getattr_np+0x1ec>
    8bf8:	aa1903e0 	mov	x0, x25
    8bfc:	d2801021 	mov	x1, #0x81                  	// #129
    8c00:	d2800022 	mov	x2, #0x1                   	// #1
    8c04:	d2800003 	mov	x3, #0x0                   	// #0
    8c08:	d2800c48 	mov	x8, #0x62                  	// #98
    8c0c:	d4000001 	svc	#0x0
    8c10:	17ffffb0 	b	8ad0 <pthread_getattr_np+0x10c>
    8c14:	f0000120 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    8c18:	f9404ba1 	ldr	x1, [x29,#144]
    8c1c:	f947e000 	ldr	x0, [x0,#4032]
    8c20:	f9400000 	ldr	x0, [x0]
    8c24:	eb01001f 	cmp	x0, x1
    8c28:	54000243 	b.cc	8c70 <pthread_getattr_np+0x2ac>
    8c2c:	f9404fa1 	ldr	x1, [x29,#152]
    8c30:	eb01001f 	cmp	x0, x1
    8c34:	54000383 	b.cc	8ca4 <pthread_getattr_np+0x2e0>
    8c38:	9e670028 	fmov	d8, x1
    8c3c:	b9400284 	ldr	w4, [x20]
    8c40:	17ffffdc 	b	8bb0 <pthread_getattr_np+0x1ec>
    8c44:	d53bd040 	mrs	x0, tpidr_el0
    8c48:	f0000121 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    8c4c:	f947c421 	ldr	x1, [x1,#3976]
    8c50:	b8616816 	ldr	w22, [x0,x1]
    8c54:	aa1403e0 	mov	x0, x20
    8c58:	97fff0fe 	bl	5050 <fclose@plt>
    8c5c:	b9400ae0 	ldr	w0, [x23,#8]
    8c60:	321d0000 	orr	w0, w0, #0x8
    8c64:	b9000ae0 	str	w0, [x23,#8]
    8c68:	35fff296 	cbnz	w22, 8ab8 <pthread_getattr_np+0xf4>
    8c6c:	17ffff7d 	b	8a60 <pthread_getattr_np+0x9c>
    8c70:	f9404fa0 	ldr	x0, [x29,#152]
    8c74:	b9400284 	ldr	w4, [x20]
    8c78:	9e670008 	fmov	d8, x0
    8c7c:	17ffffcd 	b	8bb0 <pthread_getattr_np+0x1ec>
    8c80:	52800056 	mov	w22, #0x2                   	// #2
    8c84:	f94043a0 	ldr	x0, [x29,#128]
    8c88:	97fff166 	bl	5220 <free@plt>
    8c8c:	17fffff2 	b	8c54 <pthread_getattr_np+0x290>
    8c90:	d53bd040 	mrs	x0, tpidr_el0
    8c94:	f0000121 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    8c98:	f947c421 	ldr	x1, [x1,#3976]
    8c9c:	b8616816 	ldr	w22, [x0,x1]
    8ca0:	17ffffef 	b	8c5c <pthread_getattr_np+0x298>
    8ca4:	9e660123 	fmov	x3, d9
    8ca8:	9e660140 	fmov	x0, d10
    8cac:	9e660104 	fmov	x4, d8
    8cb0:	cb0303e2 	neg	x2, x3
    8cb4:	8a020000 	and	x0, x0, x2
    8cb8:	8b030000 	add	x0, x0, x3
    8cbc:	f94053a3 	ldr	x3, [x29,#160]
    8cc0:	f9000ee0 	str	x0, [x23,#24]
    8cc4:	8b030003 	add	x3, x0, x3
    8cc8:	cb040000 	sub	x0, x0, x4
    8ccc:	cb010061 	sub	x1, x3, x1
    8cd0:	8a020021 	and	x1, x1, x2
    8cd4:	eb00003f 	cmp	x1, x0
    8cd8:	54000088 	b.hi	8ce8 <pthread_getattr_np+0x324>
    8cdc:	f90012e1 	str	x1, [x23,#32]
    8ce0:	2a1303f6 	mov	w22, w19
    8ce4:	17ffffe8 	b	8c84 <pthread_getattr_np+0x2c0>
    8ce8:	f90012e0 	str	x0, [x23,#32]
    8cec:	2a1303f6 	mov	w22, w19
    8cf0:	17ffffe5 	b	8c84 <pthread_getattr_np+0x2c0>
    8cf4:	aa1403e0 	mov	x0, x20
    8cf8:	97fff14a 	bl	5220 <free@plt>
    8cfc:	71009adf 	cmp	w22, #0x26
    8d00:	54ffedc1 	b.ne	8ab8 <pthread_getattr_np+0xf4>
    8d04:	f90016ff 	str	xzr, [x23,#40]
    8d08:	52800016 	mov	w22, #0x0                   	// #0
    8d0c:	f9001aff 	str	xzr, [x23,#48]
    8d10:	17ffff6a 	b	8ab8 <pthread_getattr_np+0xf4>

0000000000008d14 <__pthread_mutex_init>:
    8d14:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    8d18:	910003fd 	mov	x29, sp
    8d1c:	a90153f3 	stp	x19, x20, [sp,#16]
    8d20:	aa0003f3 	mov	x19, x0
    8d24:	b4000761 	cbz	x1, 8e10 <__pthread_mutex_init+0xfc>
    8d28:	b9400022 	ldr	w2, [x1]
    8d2c:	72040440 	ands	w0, w2, #0x30000000
    8d30:	54000120 	b.eq	8d54 <__pthread_mutex_init+0x40>
    8d34:	52a20003 	mov	w3, #0x10000000            	// #268435456
    8d38:	6b03001f 	cmp	w0, w3
    8d3c:	540000c0 	b.eq	8d54 <__pthread_mutex_init+0x40>
    8d40:	52800be0 	mov	w0, #0x5f                  	// #95
    8d44:	36f00082 	tbz	w2, #30, 8d54 <__pthread_mutex_init+0x40>
    8d48:	a94153f3 	ldp	x19, x20, [sp,#16]
    8d4c:	a8c37bfd 	ldp	x29, x30, [sp],#48
    8d50:	d65f03c0 	ret
    8d54:	a9017e7f 	stp	xzr, xzr, [x19,#16]
    8d58:	a9007e7f 	stp	xzr, xzr, [x19]
    8d5c:	a9027e7f 	stp	xzr, xzr, [x19,#32]
    8d60:	5281ffe0 	mov	w0, #0xfff                 	// #4095
    8d64:	b9400022 	ldr	w2, [x1]
    8d68:	72a1e000 	movk	w0, #0xf00, lsl #16
    8d6c:	0a000040 	and	w0, w2, w0
    8d70:	b9001260 	str	w0, [x19,#16]
    8d74:	b9400022 	ldr	w2, [x1]
    8d78:	36f00082 	tbz	w2, #30, 8d88 <__pthread_mutex_init+0x74>
    8d7c:	321c0000 	orr	w0, w0, #0x10
    8d80:	b9001260 	str	w0, [x19,#16]
    8d84:	b9400022 	ldr	w2, [x1]
    8d88:	12040443 	and	w3, w2, #0x30000000
    8d8c:	52a20004 	mov	w4, #0x10000000            	// #268435456
    8d90:	6b04007f 	cmp	w3, w4
    8d94:	54000360 	b.eq	8e00 <__pthread_mutex_init+0xec>
    8d98:	52a40004 	mov	w4, #0x20000000            	// #536870912
    8d9c:	6b04007f 	cmp	w3, w4
    8da0:	540001e1 	b.ne	8ddc <__pthread_mutex_init+0xc8>
    8da4:	321a0000 	orr	w0, w0, #0x40
    8da8:	b9001260 	str	w0, [x19,#16]
    8dac:	b9400020 	ldr	w0, [x1]
    8db0:	d34c5c00 	ubfx	x0, x0, #12, #12
    8db4:	350000e0 	cbnz	w0, 8dd0 <__pthread_mutex_init+0xbc>
    8db8:	90000154 	adrp	x20, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    8dbc:	b9427e80 	ldr	w0, [x20,#636]
    8dc0:	3100041f 	cmn	w0, #0x1
    8dc4:	540002c0 	b.eq	8e1c <__pthread_mutex_init+0x108>
    8dc8:	6b1f001f 	cmp	w0, wzr
    8dcc:	1a9fa000 	csel	w0, w0, wzr, ge
    8dd0:	530d3000 	lsl	w0, w0, #19
    8dd4:	b9000260 	str	w0, [x19]
    8dd8:	b9400022 	ldr	w2, [x1]
    8ddc:	7202045f 	tst	w2, #0xc0000000
    8de0:	52800000 	mov	w0, #0x0                   	// #0
    8de4:	54fffb20 	b.eq	8d48 <__pthread_mutex_init+0x34>
    8de8:	b9401261 	ldr	w1, [x19,#16]
    8dec:	32190021 	orr	w1, w1, #0x80
    8df0:	b9001261 	str	w1, [x19,#16]
    8df4:	a94153f3 	ldp	x19, x20, [sp,#16]
    8df8:	a8c37bfd 	ldp	x29, x30, [sp],#48
    8dfc:	d65f03c0 	ret
    8e00:	321b0000 	orr	w0, w0, #0x20
    8e04:	b9001260 	str	w0, [x19,#16]
    8e08:	b9400022 	ldr	w2, [x1]
    8e0c:	17fffff4 	b	8ddc <__pthread_mutex_init+0xc8>
    8e10:	d0000041 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
    8e14:	912ee021 	add	x1, x1, #0xbb8
    8e18:	17ffffc4 	b	8d28 <__pthread_mutex_init+0x14>
    8e1c:	f90017a1 	str	x1, [x29,#40]
    8e20:	9400237f 	bl	11c1c <__init_sched_fifo_prio>
    8e24:	b9427e80 	ldr	w0, [x20,#636]
    8e28:	f94017a1 	ldr	x1, [x29,#40]
    8e2c:	17ffffe7 	b	8dc8 <__pthread_mutex_init+0xb4>

0000000000008e30 <__pthread_mutex_destroy>:
    8e30:	b9401001 	ldr	w1, [x0,#16]
    8e34:	372000c1 	tbnz	w1, #4, 8e4c <__pthread_mutex_destroy+0x1c>
    8e38:	b9400c02 	ldr	w2, [x0,#12]
    8e3c:	52800201 	mov	w1, #0x10                  	// #16
    8e40:	34000062 	cbz	w2, 8e4c <__pthread_mutex_destroy+0x1c>
    8e44:	2a0103e0 	mov	w0, w1
    8e48:	d65f03c0 	ret
    8e4c:	52800001 	mov	w1, #0x0                   	// #0
    8e50:	12800002 	mov	w2, #0xffffffff            	// #-1
    8e54:	b9001002 	str	w2, [x0,#16]
    8e58:	2a0103e0 	mov	w0, w1
    8e5c:	d65f03c0 	ret

0000000000008e60 <__pthread_mutex_lock_full>:
    8e60:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    8e64:	910003fd 	mov	x29, sp
    8e68:	a9025bf5 	stp	x21, x22, [sp,#32]
    8e6c:	a90153f3 	stp	x19, x20, [sp,#16]
    8e70:	a90363f7 	stp	x23, x24, [sp,#48]
    8e74:	d53bd056 	mrs	x22, tpidr_el0
    8e78:	b9401001 	ldr	w1, [x0,#16]
    8e7c:	d11bc2d6 	sub	x22, x22, #0x6f0
    8e80:	12001822 	and	w2, w1, #0x7f
    8e84:	51004042 	sub	w2, w2, #0x10
    8e88:	b940d2d5 	ldr	w21, [x22,#208]
    8e8c:	7100cc5f 	cmp	w2, #0x33
    8e90:	54000109 	b.ls	8eb0 <__pthread_mutex_lock_full+0x50>
    8e94:	528002d4 	mov	w20, #0x16                  	// #22
    8e98:	2a1403e0 	mov	w0, w20
    8e9c:	a94153f3 	ldp	x19, x20, [sp,#16]
    8ea0:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8ea4:	a94363f7 	ldp	x23, x24, [sp,#48]
    8ea8:	a8c57bfd 	ldp	x29, x30, [sp],#80
    8eac:	d65f03c0 	ret
    8eb0:	aa0003f3 	mov	x19, x0
    8eb4:	d0000040 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
    8eb8:	912f0000 	add	x0, x0, #0xbc0
    8ebc:	78625800 	ldrh	w0, [x0,w2,uxtw #1]
    8ec0:	10000062 	adr	x2, 8ecc <__pthread_mutex_lock_full+0x6c>
    8ec4:	8b20a840 	add	x0, x2, w0, sxth #2
    8ec8:	d61f0000 	br	x0
    8ecc:	b9400a60 	ldr	w0, [x19,#8]
    8ed0:	b9400263 	ldr	w3, [x19]
    8ed4:	6b15001f 	cmp	w0, w21
    8ed8:	54001da0 	b.eq	928c <__pthread_mutex_lock_full+0x42c>
    8edc:	12800016 	mov	w22, #0xffffffff            	// #-1
    8ee0:	53137c74 	lsr	w20, w3, #19
    8ee4:	9400241d 	bl	11f58 <__pthread_current_priority>
    8ee8:	6b00029f 	cmp	w20, w0
    8eec:	5400204b 	b.lt	92f4 <__pthread_mutex_lock_full+0x494>
    8ef0:	2a1603e0 	mov	w0, w22
    8ef4:	2a1403e1 	mov	w1, w20
    8ef8:	94002358 	bl	11c58 <__pthread_tpp_change_priority>
    8efc:	35001f80 	cbnz	w0, 92ec <__pthread_mutex_lock_full+0x48c>
    8f00:	910143a5 	add	x5, x29, #0x50
    8f04:	530d3284 	lsl	w4, w20, #19
    8f08:	32000089 	orr	w9, w4, #0x1
    8f0c:	2a0403e0 	mov	w0, w4
    8f10:	b81fcca4 	str	w4, [x5,#-4]!
    8f14:	885ffe61 	ldaxr	w1, [x19]
    8f18:	6b00003f 	cmp	w1, w0
    8f1c:	54000061 	b.ne	8f28 <__pthread_mutex_lock_full+0xc8>
    8f20:	88027e69 	stxr	w2, w9, [x19]
    8f24:	35ffff82 	cbnz	w2, 8f14 <__pthread_mutex_lock_full+0xb4>
    8f28:	54000040 	b.eq	8f30 <__pthread_mutex_lock_full+0xd0>
    8f2c:	b9004fa1 	str	w1, [x29,#76]
    8f30:	b9404fa0 	ldr	w0, [x29,#76]
    8f34:	6b00009f 	cmp	w4, w0
    8f38:	540004c0 	b.eq	8fd0 <__pthread_mutex_lock_full+0x170>
    8f3c:	321f0087 	orr	w7, w4, #0x2
    8f40:	93407cea 	sxtw	x10, w7
    8f44:	b9004fa9 	str	w9, [x29,#76]
    8f48:	b94000a0 	ldr	w0, [x5]
    8f4c:	885ffe63 	ldaxr	w3, [x19]
    8f50:	6b00007f 	cmp	w3, w0
    8f54:	54000061 	b.ne	8f60 <__pthread_mutex_lock_full+0x100>
    8f58:	88017e67 	stxr	w1, w7, [x19]
    8f5c:	35ffff81 	cbnz	w1, 8f4c <__pthread_mutex_lock_full+0xec>
    8f60:	54000040 	b.eq	8f68 <__pthread_mutex_lock_full+0x108>
    8f64:	b90000a3 	str	w3, [x5]
    8f68:	b9404fa3 	ldr	w3, [x29,#76]
    8f6c:	120d3060 	and	w0, w3, #0xfff80000
    8f70:	6b04001f 	cmp	w0, w4
    8f74:	54001581 	b.ne	9224 <__pthread_mutex_lock_full+0x3c4>
    8f78:	6b03009f 	cmp	w4, w3
    8f7c:	54000120 	b.eq	8fa0 <__pthread_mutex_lock_full+0x140>
    8f80:	b9401266 	ldr	w6, [x19,#16]
    8f84:	aa1303e0 	mov	x0, x19
    8f88:	aa0a03e2 	mov	x2, x10
    8f8c:	d2800003 	mov	x3, #0x0                   	// #0
    8f90:	2a2603e1 	mvn	w1, w6
    8f94:	d2800c48 	mov	x8, #0x62                  	// #98
    8f98:	92790021 	and	x1, x1, #0x80
    8f9c:	d4000001 	svc	#0x0
    8fa0:	b9004fa4 	str	w4, [x29,#76]
    8fa4:	b94000a0 	ldr	w0, [x5]
    8fa8:	885ffe63 	ldaxr	w3, [x19]
    8fac:	6b00007f 	cmp	w3, w0
    8fb0:	54000061 	b.ne	8fbc <__pthread_mutex_lock_full+0x15c>
    8fb4:	88017e67 	stxr	w1, w7, [x19]
    8fb8:	35ffff81 	cbnz	w1, 8fa8 <__pthread_mutex_lock_full+0x148>
    8fbc:	54000040 	b.eq	8fc4 <__pthread_mutex_lock_full+0x164>
    8fc0:	b90000a3 	str	w3, [x5]
    8fc4:	b9404fa0 	ldr	w0, [x29,#76]
    8fc8:	6b00009f 	cmp	w4, w0
    8fcc:	54fffbc1 	b.ne	8f44 <__pthread_mutex_lock_full+0xe4>
    8fd0:	52800020 	mov	w0, #0x1                   	// #1
    8fd4:	b9000660 	str	w0, [x19,#4]
    8fd8:	b9400e60 	ldr	w0, [x19,#12]
    8fdc:	52800014 	mov	w20, #0x0                   	// #0
    8fe0:	b9000a75 	str	w21, [x19,#8]
    8fe4:	11000400 	add	w0, w0, #0x1
    8fe8:	b9000e60 	str	w0, [x19,#12]
    8fec:	2a1403e0 	mov	w0, w20
    8ff0:	a94153f3 	ldp	x19, x20, [sp,#16]
    8ff4:	a9425bf5 	ldp	x21, x22, [sp,#32]
    8ff8:	a94363f7 	ldp	x23, x24, [sp,#48]
    8ffc:	a8c57bfd 	ldp	x29, x30, [sp],#80
    9000:	d65f03c0 	ret
    9004:	121c0024 	and	w4, w1, #0x10
    9008:	36200081 	tbz	w1, #4, 9018 <__pthread_mutex_lock_full+0x1b8>
    900c:	91008260 	add	x0, x19, #0x20
    9010:	b2400000 	orr	x0, x0, #0x1
    9014:	f9007ac0 	str	x0, [x22,#240]
    9018:	b9400260 	ldr	w0, [x19]
    901c:	12007400 	and	w0, w0, #0x3fffffff
    9020:	6b0002bf 	cmp	w21, w0
    9024:	540014a0 	b.eq	92b8 <__pthread_mutex_lock_full+0x458>
    9028:	b9004fbf 	str	wzr, [x29,#76]
    902c:	885ffe60 	ldaxr	w0, [x19]
    9030:	6b1f001f 	cmp	w0, wzr
    9034:	54000061 	b.ne	9040 <__pthread_mutex_lock_full+0x1e0>
    9038:	88017e75 	stxr	w1, w21, [x19]
    903c:	35ffff81 	cbnz	w1, 902c <__pthread_mutex_lock_full+0x1cc>
    9040:	540011e1 	b.ne	927c <__pthread_mutex_lock_full+0x41c>
    9044:	b9404fa0 	ldr	w0, [x29,#76]
    9048:	34000a40 	cbz	w0, 9190 <__pthread_mutex_lock_full+0x330>
    904c:	d28000c1 	mov	x1, #0x6                   	// #6
    9050:	350000c4 	cbnz	w4, 9068 <__pthread_mutex_lock_full+0x208>
    9054:	b9401261 	ldr	w1, [x19,#16]
    9058:	528010c0 	mov	w0, #0x86                  	// #134
    905c:	12190021 	and	w1, w1, #0x80
    9060:	4a000021 	eor	w1, w1, w0
    9064:	93407c21 	sxtw	x1, w1
    9068:	aa1303e0 	mov	x0, x19
    906c:	d2800022 	mov	x2, #0x1                   	// #1
    9070:	d2800003 	mov	x3, #0x0                   	// #0
    9074:	d2800c48 	mov	x8, #0x62                  	// #98
    9078:	d4000001 	svc	#0x0
    907c:	3140041f 	cmn	w0, #0x1, lsl #12
    9080:	54000089 	b.ls	9090 <__pthread_mutex_lock_full+0x230>
    9084:	121a7800 	and	w0, w0, #0xffffffdf
    9088:	31008c1f 	cmn	w0, #0x23
    908c:	54000f20 	b.eq	9270 <__pthread_mutex_lock_full+0x410>
    9090:	b9400260 	ldr	w0, [x19]
    9094:	36f007e0 	tbz	w0, #30, 9190 <__pthread_mutex_lock_full+0x330>
    9098:	910133a5 	add	x5, x29, #0x4c
    909c:	b9004fa0 	str	w0, [x29,#76]
    90a0:	12017800 	and	w0, w0, #0xbfffffff
    90a4:	b94000a2 	ldr	w2, [x5]
    90a8:	885ffe61 	ldaxr	w1, [x19]
    90ac:	6b02003f 	cmp	w1, w2
    90b0:	54000061 	b.ne	90bc <__pthread_mutex_lock_full+0x25c>
    90b4:	88037e60 	stxr	w3, w0, [x19]
    90b8:	35ffff83 	cbnz	w3, 90a8 <__pthread_mutex_lock_full+0x248>
    90bc:	540012a0 	b.eq	9310 <__pthread_mutex_lock_full+0x4b0>
    90c0:	b90000a1 	str	w1, [x5]
    90c4:	b9400260 	ldr	w0, [x19]
    90c8:	17fffff5 	b	909c <__pthread_mutex_lock_full+0x23c>
    90cc:	91008277 	add	x23, x19, #0x20
    90d0:	f9007ad7 	str	x23, [x22,#240]
    90d4:	321f77f8 	mov	w24, #0x7ffffffe            	// #2147483646
    90d8:	b9400260 	ldr	w0, [x19]
    90dc:	12020014 	and	w20, w0, #0x40000000
    90e0:	35000a74 	cbnz	w20, 922c <__pthread_mutex_lock_full+0x3cc>
    90e4:	12007400 	and	w0, w0, #0x3fffffff
    90e8:	6b0002bf 	cmp	w21, w0
    90ec:	54000380 	b.eq	915c <__pthread_mutex_lock_full+0x2fc>
    90f0:	b9004fbf 	str	wzr, [x29,#76]
    90f4:	885ffe62 	ldaxr	w2, [x19]
    90f8:	6b1f005f 	cmp	w2, wzr
    90fc:	54000061 	b.ne	9108 <__pthread_mutex_lock_full+0x2a8>
    9100:	88007e75 	stxr	w0, w21, [x19]
    9104:	35ffff80 	cbnz	w0, 90f4 <__pthread_mutex_lock_full+0x294>
    9108:	54000680 	b.eq	91d8 <__pthread_mutex_lock_full+0x378>
    910c:	52801001 	mov	w1, #0x80                  	// #128
    9110:	aa1303e0 	mov	x0, x19
    9114:	b9004fa2 	str	w2, [x29,#76]
    9118:	94001a4e 	bl	fa50 <__lll_robust_lock_wait>
    911c:	b9400a61 	ldr	w1, [x19,#8]
    9120:	6b18003f 	cmp	w1, w24
    9124:	54000620 	b.eq	91e8 <__pthread_mutex_lock_full+0x388>
    9128:	12020014 	and	w20, w0, #0x40000000
    912c:	37f7fda0 	tbnz	w0, #30, 90e0 <__pthread_mutex_lock_full+0x280>
    9130:	aa1603e0 	mov	x0, x22
    9134:	52800021 	mov	w1, #0x1                   	// #1
    9138:	b9000661 	str	w1, [x19,#4]
    913c:	f84e0c01 	ldr	x1, [x0,#224]!
    9140:	927ff822 	and	x2, x1, #0xfffffffffffffffe
    9144:	f81f8057 	str	x23, [x2,#-8]
    9148:	f9001261 	str	x1, [x19,#32]
    914c:	f9000e60 	str	x0, [x19,#24]
    9150:	f90072d7 	str	x23, [x22,#224]
    9154:	f9007adf 	str	xzr, [x22,#240]
    9158:	17ffffa0 	b	8fd8 <__pthread_mutex_lock_full+0x178>
    915c:	b9401260 	ldr	w0, [x19,#16]
    9160:	12001800 	and	w0, w0, #0x7f
    9164:	7100481f 	cmp	w0, #0x12
    9168:	54001320 	b.eq	93cc <__pthread_mutex_lock_full+0x56c>
    916c:	7100441f 	cmp	w0, #0x11
    9170:	54fffc01 	b.ne	90f0 <__pthread_mutex_lock_full+0x290>
    9174:	f9007adf 	str	xzr, [x22,#240]
    9178:	b9400660 	ldr	w0, [x19,#4]
    917c:	3100041f 	cmn	w0, #0x1
    9180:	54000980 	b.eq	92b0 <__pthread_mutex_lock_full+0x450>
    9184:	11000400 	add	w0, w0, #0x1
    9188:	b9000660 	str	w0, [x19,#4]
    918c:	17ffff98 	b	8fec <__pthread_mutex_lock_full+0x18c>
    9190:	34fff204 	cbz	w4, 8fd0 <__pthread_mutex_lock_full+0x170>
    9194:	b9400a61 	ldr	w1, [x19,#8]
    9198:	321f77e0 	mov	w0, #0x7ffffffe            	// #2147483646
    919c:	6b00003f 	cmp	w1, w0
    91a0:	54001020 	b.eq	93a4 <__pthread_mutex_lock_full+0x544>
    91a4:	aa1603e0 	mov	x0, x22
    91a8:	52800021 	mov	w1, #0x1                   	// #1
    91ac:	b9000661 	str	w1, [x19,#4]
    91b0:	91008262 	add	x2, x19, #0x20
    91b4:	b2400043 	orr	x3, x2, #0x1
    91b8:	f84e0c01 	ldr	x1, [x0,#224]!
    91bc:	927ff824 	and	x4, x1, #0xfffffffffffffffe
    91c0:	f81f8082 	str	x2, [x4,#-8]
    91c4:	f9001261 	str	x1, [x19,#32]
    91c8:	f9000e60 	str	x0, [x19,#24]
    91cc:	f90072c3 	str	x3, [x22,#224]
    91d0:	f9007adf 	str	xzr, [x22,#240]
    91d4:	17ffff81 	b	8fd8 <__pthread_mutex_lock_full+0x178>
    91d8:	b9400a61 	ldr	w1, [x19,#8]
    91dc:	321f77e0 	mov	w0, #0x7ffffffe            	// #2147483646
    91e0:	6b00003f 	cmp	w1, w0
    91e4:	54fffa61 	b.ne	9130 <__pthread_mutex_lock_full+0x2d0>
    91e8:	b900067f 	str	wzr, [x19,#4]
    91ec:	52800001 	mov	w1, #0x0                   	// #0
    91f0:	885f7e60 	ldxr	w0, [x19]
    91f4:	8802fe61 	stlxr	w2, w1, [x19]
    91f8:	35ffffc2 	cbnz	w2, 91f0 <__pthread_mutex_lock_full+0x390>
    91fc:	7100041f 	cmp	w0, #0x1
    9200:	54000c4c 	b.gt	9388 <__pthread_mutex_lock_full+0x528>
    9204:	f9007adf 	str	xzr, [x22,#240]
    9208:	52801074 	mov	w20, #0x83                  	// #131
    920c:	2a1403e0 	mov	w0, w20
    9210:	a94153f3 	ldp	x19, x20, [sp,#16]
    9214:	a9425bf5 	ldp	x21, x22, [sp,#32]
    9218:	a94363f7 	ldp	x23, x24, [sp,#48]
    921c:	a8c57bfd 	ldp	x29, x30, [sp],#80
    9220:	d65f03c0 	ret
    9224:	2a1403f6 	mov	w22, w20
    9228:	17ffff2e 	b	8ee0 <__pthread_mutex_lock_full+0x80>
    922c:	12010001 	and	w1, w0, #0x80000000
    9230:	b9004fa0 	str	w0, [x29,#76]
    9234:	2a150021 	orr	w1, w1, w21
    9238:	2a0003e2 	mov	w2, w0
    923c:	885ffe63 	ldaxr	w3, [x19]
    9240:	6b02007f 	cmp	w3, w2
    9244:	54000061 	b.ne	9250 <__pthread_mutex_lock_full+0x3f0>
    9248:	88047e61 	stxr	w4, w1, [x19]
    924c:	35ffff84 	cbnz	w4, 923c <__pthread_mutex_lock_full+0x3dc>
    9250:	54000040 	b.eq	9258 <__pthread_mutex_lock_full+0x3f8>
    9254:	b9004fa3 	str	w3, [x29,#76]
    9258:	b9404fa1 	ldr	w1, [x29,#76]
    925c:	6b01001f 	cmp	w0, w1
    9260:	54000780 	b.eq	9350 <__pthread_mutex_lock_full+0x4f0>
    9264:	2a0103e0 	mov	w0, w1
    9268:	12020034 	and	w20, w1, #0x40000000
    926c:	17ffff9d 	b	90e0 <__pthread_mutex_lock_full+0x280>
    9270:	94001dfa 	bl	10a58 <__pause_nocancel>
    9274:	94001df9 	bl	10a58 <__pause_nocancel>
    9278:	17fffffe 	b	9270 <__pthread_mutex_lock_full+0x410>
    927c:	b9004fa0 	str	w0, [x29,#76]
    9280:	b9404fa0 	ldr	w0, [x29,#76]
    9284:	34fff860 	cbz	w0, 9190 <__pthread_mutex_lock_full+0x330>
    9288:	17ffff71 	b	904c <__pthread_mutex_lock_full+0x1ec>
    928c:	12000421 	and	w1, w1, #0x3
    9290:	52800474 	mov	w20, #0x23                  	// #35
    9294:	7100083f 	cmp	w1, #0x2
    9298:	54ffeaa0 	b.eq	8fec <__pthread_mutex_lock_full+0x18c>
    929c:	7100043f 	cmp	w1, #0x1
    92a0:	54ffe1e1 	b.ne	8edc <__pthread_mutex_lock_full+0x7c>
    92a4:	b9400660 	ldr	w0, [x19,#4]
    92a8:	3100041f 	cmn	w0, #0x1
    92ac:	54000181 	b.ne	92dc <__pthread_mutex_lock_full+0x47c>
    92b0:	52800174 	mov	w20, #0xb                   	// #11
    92b4:	17ffff4e 	b	8fec <__pthread_mutex_lock_full+0x18c>
    92b8:	12000421 	and	w1, w1, #0x3
    92bc:	7100083f 	cmp	w1, #0x2
    92c0:	54000860 	b.eq	93cc <__pthread_mutex_lock_full+0x56c>
    92c4:	7100043f 	cmp	w1, #0x1
    92c8:	54ffeb01 	b.ne	9028 <__pthread_mutex_lock_full+0x1c8>
    92cc:	f9007adf 	str	xzr, [x22,#240]
    92d0:	b9400660 	ldr	w0, [x19,#4]
    92d4:	3100041f 	cmn	w0, #0x1
    92d8:	54fffec0 	b.eq	92b0 <__pthread_mutex_lock_full+0x450>
    92dc:	11000400 	add	w0, w0, #0x1
    92e0:	52800014 	mov	w20, #0x0                   	// #0
    92e4:	b9000660 	str	w0, [x19,#4]
    92e8:	17ffff41 	b	8fec <__pthread_mutex_lock_full+0x18c>
    92ec:	2a0003f4 	mov	w20, w0
    92f0:	17ffff3f 	b	8fec <__pthread_mutex_lock_full+0x18c>
    92f4:	310006df 	cmn	w22, #0x1
    92f8:	528002d4 	mov	w20, #0x16                  	// #22
    92fc:	54ffe780 	b.eq	8fec <__pthread_mutex_lock_full+0x18c>
    9300:	2a1603e0 	mov	w0, w22
    9304:	12800001 	mov	w1, #0xffffffff            	// #-1
    9308:	94002254 	bl	11c58 <__pthread_tpp_change_priority>
    930c:	17ffff38 	b	8fec <__pthread_mutex_lock_full+0x18c>
    9310:	aa1603e0 	mov	x0, x22
    9314:	52800021 	mov	w1, #0x1                   	// #1
    9318:	b9000661 	str	w1, [x19,#4]
    931c:	12b00001 	mov	w1, #0x7fffffff            	// #2147483647
    9320:	b9000a61 	str	w1, [x19,#8]
    9324:	91008262 	add	x2, x19, #0x20
    9328:	b2400043 	orr	x3, x2, #0x1
    932c:	52801054 	mov	w20, #0x82                  	// #130
    9330:	f84e0c01 	ldr	x1, [x0,#224]!
    9334:	927ff824 	and	x4, x1, #0xfffffffffffffffe
    9338:	f81f8082 	str	x2, [x4,#-8]
    933c:	f9001261 	str	x1, [x19,#32]
    9340:	f9000e60 	str	x0, [x19,#24]
    9344:	f90072c3 	str	x3, [x22,#224]
    9348:	f9007adf 	str	xzr, [x22,#240]
    934c:	17ffff28 	b	8fec <__pthread_mutex_lock_full+0x18c>
    9350:	aa1603e0 	mov	x0, x22
    9354:	52800021 	mov	w1, #0x1                   	// #1
    9358:	b9000661 	str	w1, [x19,#4]
    935c:	12b00001 	mov	w1, #0x7fffffff            	// #2147483647
    9360:	b9000a61 	str	w1, [x19,#8]
    9364:	52801054 	mov	w20, #0x82                  	// #130
    9368:	f84e0c01 	ldr	x1, [x0,#224]!
    936c:	927ff822 	and	x2, x1, #0xfffffffffffffffe
    9370:	f81f8057 	str	x23, [x2,#-8]
    9374:	f9001261 	str	x1, [x19,#32]
    9378:	f9000e60 	str	x0, [x19,#24]
    937c:	f90072d7 	str	x23, [x22,#224]
    9380:	f9007adf 	str	xzr, [x22,#240]
    9384:	17ffff1a 	b	8fec <__pthread_mutex_lock_full+0x18c>
    9388:	d2800021 	mov	x1, #0x1                   	// #1
    938c:	aa1303e0 	mov	x0, x19
    9390:	aa0103e2 	mov	x2, x1
    9394:	d2800003 	mov	x3, #0x0                   	// #0
    9398:	d2800c48 	mov	x8, #0x62                  	// #98
    939c:	d4000001 	svc	#0x0
    93a0:	17ffff99 	b	9204 <__pthread_mutex_lock_full+0x3a4>
    93a4:	d2800002 	mov	x2, #0x0                   	// #0
    93a8:	b900067f 	str	wzr, [x19,#4]
    93ac:	aa1303e0 	mov	x0, x19
    93b0:	d28000e1 	mov	x1, #0x7                   	// #7
    93b4:	aa0203e3 	mov	x3, x2
    93b8:	d2800c48 	mov	x8, #0x62                  	// #98
    93bc:	d4000001 	svc	#0x0
    93c0:	52801074 	mov	w20, #0x83                  	// #131
    93c4:	f9007ac2 	str	x2, [x22,#240]
    93c8:	17ffff09 	b	8fec <__pthread_mutex_lock_full+0x18c>
    93cc:	f9007adf 	str	xzr, [x22,#240]
    93d0:	52800474 	mov	w20, #0x23                  	// #35
    93d4:	17ffff06 	b	8fec <__pthread_mutex_lock_full+0x18c>

00000000000093d8 <__pthread_mutex_lock>:
    93d8:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    93dc:	52802fe1 	mov	w1, #0x17f                 	// #383
    93e0:	910003fd 	mov	x29, sp
    93e4:	a90153f3 	stp	x19, x20, [sp,#16]
    93e8:	f90013f5 	str	x21, [sp,#32]
    93ec:	aa0003f3 	mov	x19, x0
    93f0:	b9401002 	ldr	w2, [x0,#16]
    93f4:	721e1043 	ands	w3, w2, #0x7c
    93f8:	0a010041 	and	w1, w2, w1
    93fc:	540003c1 	b.ne	9474 <__pthread_mutex_lock+0x9c>
    9400:	35000441 	cbnz	w1, 9488 <__pthread_mutex_lock+0xb0>
    9404:	b9003fa1 	str	w1, [x29,#60]
    9408:	52800020 	mov	w0, #0x1                   	// #1
    940c:	885ffe63 	ldaxr	w3, [x19]
    9410:	6b1f007f 	cmp	w3, wzr
    9414:	54000061 	b.ne	9420 <__pthread_mutex_lock+0x48>
    9418:	88017e60 	stxr	w1, w0, [x19]
    941c:	35ffff81 	cbnz	w1, 940c <__pthread_mutex_lock+0x34>
    9420:	54000120 	b.eq	9444 <__pthread_mutex_lock+0x6c>
    9424:	b9401261 	ldr	w1, [x19,#16]
    9428:	d53bd042 	mrs	x2, tpidr_el0
    942c:	b9003fa3 	str	w3, [x29,#60]
    9430:	12190021 	and	w1, w1, #0x80
    9434:	aa1303e0 	mov	x0, x19
    9438:	d11bc054 	sub	x20, x2, #0x6f0
    943c:	940018e2 	bl	f7c4 <__lll_lock_wait>
    9440:	14000003 	b	944c <__pthread_mutex_lock+0x74>
    9444:	d53bd042 	mrs	x2, tpidr_el0
    9448:	d11bc054 	sub	x20, x2, #0x6f0
    944c:	b9400e61 	ldr	w1, [x19,#12]
    9450:	52800000 	mov	w0, #0x0                   	// #0
    9454:	b940d282 	ldr	w2, [x20,#208]
    9458:	11000421 	add	w1, w1, #0x1
    945c:	b9000a62 	str	w2, [x19,#8]
    9460:	b9000e61 	str	w1, [x19,#12]
    9464:	a94153f3 	ldp	x19, x20, [sp,#16]
    9468:	f94013f5 	ldr	x21, [sp,#32]
    946c:	a8c47bfd 	ldp	x29, x30, [sp],#64
    9470:	d65f03c0 	ret
    9474:	97fffe7b 	bl	8e60 <__pthread_mutex_lock_full>
    9478:	f94013f5 	ldr	x21, [sp,#32]
    947c:	a94153f3 	ldp	x19, x20, [sp,#16]
    9480:	a8c47bfd 	ldp	x29, x30, [sp],#64
    9484:	d65f03c0 	ret
    9488:	12001841 	and	w1, w2, #0x7f
    948c:	7100043f 	cmp	w1, #0x1
    9490:	54000381 	b.ne	9500 <__pthread_mutex_lock+0x128>
    9494:	d53bd042 	mrs	x2, tpidr_el0
    9498:	b9400804 	ldr	w4, [x0,#8]
    949c:	d11bc054 	sub	x20, x2, #0x6f0
    94a0:	b940d282 	ldr	w2, [x20,#208]
    94a4:	6b02009f 	cmp	w4, w2
    94a8:	540001e0 	b.eq	94e4 <__pthread_mutex_lock+0x10c>
    94ac:	b9003fa3 	str	w3, [x29,#60]
    94b0:	885ffe62 	ldaxr	w2, [x19]
    94b4:	6b1f005f 	cmp	w2, wzr
    94b8:	54000061 	b.ne	94c4 <__pthread_mutex_lock+0xec>
    94bc:	88037e61 	stxr	w3, w1, [x19]
    94c0:	35ffff83 	cbnz	w3, 94b0 <__pthread_mutex_lock+0xd8>
    94c4:	540000a0 	b.eq	94d8 <__pthread_mutex_lock+0x100>
    94c8:	b9401261 	ldr	w1, [x19,#16]
    94cc:	b9003fa2 	str	w2, [x29,#60]
    94d0:	12190021 	and	w1, w1, #0x80
    94d4:	940018bc 	bl	f7c4 <__lll_lock_wait>
    94d8:	52800020 	mov	w0, #0x1                   	// #1
    94dc:	b9000660 	str	w0, [x19,#4]
    94e0:	17ffffdb 	b	944c <__pthread_mutex_lock+0x74>
    94e4:	b9400400 	ldr	w0, [x0,#4]
    94e8:	3100041f 	cmn	w0, #0x1
    94ec:	540002e0 	b.eq	9548 <__pthread_mutex_lock+0x170>
    94f0:	11000401 	add	w1, w0, #0x1
    94f4:	2a0303e0 	mov	w0, w3
    94f8:	b9000661 	str	w1, [x19,#4]
    94fc:	17ffffda 	b	9464 <__pthread_mutex_lock+0x8c>
    9500:	71000c3f 	cmp	w1, #0x3
    9504:	54000881 	b.ne	9614 <__pthread_mutex_lock+0x23c>
    9508:	f0000140 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    950c:	b9432c00 	ldr	w0, [x0,#812]
    9510:	35000200 	cbnz	w0, 9550 <__pthread_mutex_lock+0x178>
    9514:	d53bd042 	mrs	x2, tpidr_el0
    9518:	b9003fbf 	str	wzr, [x29,#60]
    951c:	52800020 	mov	w0, #0x1                   	// #1
    9520:	885ffe63 	ldaxr	w3, [x19]
    9524:	6b1f007f 	cmp	w3, wzr
    9528:	54000061 	b.ne	9534 <__pthread_mutex_lock+0x15c>
    952c:	88017e60 	stxr	w1, w0, [x19]
    9530:	35ffff81 	cbnz	w1, 9520 <__pthread_mutex_lock+0x148>
    9534:	54fff8a0 	b.eq	9448 <__pthread_mutex_lock+0x70>
    9538:	b9401261 	ldr	w1, [x19,#16]
    953c:	b9003fa3 	str	w3, [x29,#60]
    9540:	12190021 	and	w1, w1, #0x80
    9544:	17ffffbc 	b	9434 <__pthread_mutex_lock+0x5c>
    9548:	52800160 	mov	w0, #0xb                   	// #11
    954c:	17ffffc6 	b	9464 <__pthread_mutex_lock+0x8c>
    9550:	b9003fa3 	str	w3, [x29,#60]
    9554:	52800022 	mov	w2, #0x1                   	// #1
    9558:	885ffe60 	ldaxr	w0, [x19]
    955c:	6b1f001f 	cmp	w0, wzr
    9560:	54000061 	b.ne	956c <__pthread_mutex_lock+0x194>
    9564:	88017e62 	stxr	w1, w2, [x19]
    9568:	35ffff81 	cbnz	w1, 9558 <__pthread_mutex_lock+0x180>
    956c:	1a9f17f5 	cset	w21, eq
    9570:	35fff6b5 	cbnz	w21, 9444 <__pthread_mutex_lock+0x6c>
    9574:	b9401661 	ldr	w1, [x19,#20]
    9578:	52800c80 	mov	w0, #0x64                  	// #100
    957c:	11001421 	add	w1, w1, #0x5
    9580:	531f7821 	lsl	w1, w1, #1
    9584:	7101903f 	cmp	w1, #0x64
    9588:	1a80d021 	csel	w1, w1, w0, le
    958c:	110006b5 	add	w21, w21, #0x1
    9590:	b9003fbf 	str	wzr, [x29,#60]
    9594:	510006a0 	sub	w0, w21, #0x1
    9598:	6b01001f 	cmp	w0, w1
    959c:	5400022a 	b.ge	95e0 <__pthread_mutex_lock+0x208>
    95a0:	885ffe60 	ldaxr	w0, [x19]
    95a4:	6b1f001f 	cmp	w0, wzr
    95a8:	54000061 	b.ne	95b4 <__pthread_mutex_lock+0x1dc>
    95ac:	88037e62 	stxr	w3, w2, [x19]
    95b0:	35ffff83 	cbnz	w3, 95a0 <__pthread_mutex_lock+0x1c8>
    95b4:	54fffec1 	b.ne	958c <__pthread_mutex_lock+0x1b4>
    95b8:	b9401661 	ldr	w1, [x19,#20]
    95bc:	d53bd042 	mrs	x2, tpidr_el0
    95c0:	d11bc054 	sub	x20, x2, #0x6f0
    95c4:	4b0102b5 	sub	w21, w21, w1
    95c8:	11001ea0 	add	w0, w21, #0x7
    95cc:	6b1f02bf 	cmp	w21, wzr
    95d0:	1a95b015 	csel	w21, w0, w21, lt
    95d4:	0b950c35 	add	w21, w1, w21, asr #3
    95d8:	b9001675 	str	w21, [x19,#20]
    95dc:	17ffff9c 	b	944c <__pthread_mutex_lock+0x74>
    95e0:	52800020 	mov	w0, #0x1                   	// #1
    95e4:	885ffe62 	ldaxr	w2, [x19]
    95e8:	6b1f005f 	cmp	w2, wzr
    95ec:	54000061 	b.ne	95f8 <__pthread_mutex_lock+0x220>
    95f0:	88017e60 	stxr	w1, w0, [x19]
    95f4:	35ffff81 	cbnz	w1, 95e4 <__pthread_mutex_lock+0x20c>
    95f8:	54fffe00 	b.eq	95b8 <__pthread_mutex_lock+0x1e0>
    95fc:	b9401261 	ldr	w1, [x19,#16]
    9600:	aa1303e0 	mov	x0, x19
    9604:	b9003fa2 	str	w2, [x29,#60]
    9608:	12190021 	and	w1, w1, #0x80
    960c:	9400186e 	bl	f7c4 <__lll_lock_wait>
    9610:	17ffffea 	b	95b8 <__pthread_mutex_lock+0x1e0>
    9614:	d53bd042 	mrs	x2, tpidr_el0
    9618:	b9400800 	ldr	w0, [x0,#8]
    961c:	d11bc041 	sub	x1, x2, #0x6f0
    9620:	b940d021 	ldr	w1, [x1,#208]
    9624:	6b00003f 	cmp	w1, w0
    9628:	54fff781 	b.ne	9518 <__pthread_mutex_lock+0x140>
    962c:	52800460 	mov	w0, #0x23                  	// #35
    9630:	17ffff8d 	b	9464 <__pthread_mutex_lock+0x8c>

0000000000009634 <__pthread_mutex_trylock>:
    9634:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    9638:	aa0003e4 	mov	x4, x0
    963c:	d53bd045 	mrs	x5, tpidr_el0
    9640:	910003fd 	mov	x29, sp
    9644:	f9001bf7 	str	x23, [sp,#48]
    9648:	d11bc0a5 	sub	x5, x5, #0x6f0
    964c:	a90153f3 	stp	x19, x20, [sp,#16]
    9650:	a9025bf5 	stp	x21, x22, [sp,#32]
    9654:	b940d0b7 	ldr	w23, [x5,#208]
    9658:	b9401002 	ldr	w2, [x0,#16]
    965c:	52802fe0 	mov	w0, #0x17f                 	// #383
    9660:	0a000040 	and	w0, w2, w0
    9664:	71008c1f 	cmp	w0, #0x23
    9668:	54000328 	b.hi	96cc <__pthread_mutex_trylock+0x98>
    966c:	7100801f 	cmp	w0, #0x20
    9670:	54000542 	b.cs	9718 <__pthread_mutex_trylock+0xe4>
    9674:	71000c1f 	cmp	w0, #0x3
    9678:	54001008 	b.hi	9878 <__pthread_mutex_trylock+0x244>
    967c:	7100081f 	cmp	w0, #0x2
    9680:	54000962 	b.cs	97ac <__pthread_mutex_trylock+0x178>
    9684:	34000940 	cbz	w0, 97ac <__pthread_mutex_trylock+0x178>
    9688:	7100041f 	cmp	w0, #0x1
    968c:	54000821 	b.ne	9790 <__pthread_mutex_trylock+0x15c>
    9690:	b9400880 	ldr	w0, [x4,#8]
    9694:	6b17001f 	cmp	w0, w23
    9698:	540019e1 	b.ne	99d4 <__pthread_mutex_trylock+0x3a0>
    969c:	b9400480 	ldr	w0, [x4,#4]
    96a0:	3100041f 	cmn	w0, #0x1
    96a4:	540021c0 	b.eq	9adc <__pthread_mutex_trylock+0x4a8>
    96a8:	11000400 	add	w0, w0, #0x1
    96ac:	52800013 	mov	w19, #0x0                   	// #0
    96b0:	b9000480 	str	w0, [x4,#4]
    96b4:	2a1303e0 	mov	w0, w19
    96b8:	f9401bf7 	ldr	x23, [sp,#48]
    96bc:	a94153f3 	ldp	x19, x20, [sp,#16]
    96c0:	a9425bf5 	ldp	x21, x22, [sp,#32]
    96c4:	a8c57bfd 	ldp	x29, x30, [sp],#80
    96c8:	d65f03c0 	ret
    96cc:	71010c1f 	cmp	w0, #0x43
    96d0:	540001a9 	b.ls	9704 <__pthread_mutex_trylock+0xd0>
    96d4:	7104001f 	cmp	w0, #0x100
    96d8:	54000581 	b.ne	9788 <__pthread_mutex_trylock+0x154>
    96dc:	b9004fbf 	str	wzr, [x29,#76]
    96e0:	52800020 	mov	w0, #0x1                   	// #1
    96e4:	885ffc81 	ldaxr	w1, [x4]
    96e8:	6b1f003f 	cmp	w1, wzr
    96ec:	54000061 	b.ne	96f8 <__pthread_mutex_trylock+0xc4>
    96f0:	88027c80 	stxr	w2, w0, [x4]
    96f4:	35ffff82 	cbnz	w2, 96e4 <__pthread_mutex_trylock+0xb0>
    96f8:	540003a1 	b.ne	976c <__pthread_mutex_trylock+0x138>
    96fc:	52800013 	mov	w19, #0x0                   	// #0
    9700:	17ffffed 	b	96b4 <__pthread_mutex_trylock+0x80>
    9704:	7101001f 	cmp	w0, #0x40
    9708:	540006e2 	b.cs	97e4 <__pthread_mutex_trylock+0x1b0>
    970c:	5100c000 	sub	w0, w0, #0x30
    9710:	71000c1f 	cmp	w0, #0x3
    9714:	540003e8 	b.hi	9790 <__pthread_mutex_trylock+0x15c>
    9718:	121c0046 	and	w6, w2, #0x10
    971c:	36200082 	tbz	w2, #4, 972c <__pthread_mutex_trylock+0xf8>
    9720:	91008080 	add	x0, x4, #0x20
    9724:	b2400000 	orr	x0, x0, #0x1
    9728:	f90078a0 	str	x0, [x5,#240]
    972c:	b9400080 	ldr	w0, [x4]
    9730:	12007400 	and	w0, w0, #0x3fffffff
    9734:	6b0002ff 	cmp	w23, w0
    9738:	54001c00 	b.eq	9ab8 <__pthread_mutex_trylock+0x484>
    973c:	b9004fbf 	str	wzr, [x29,#76]
    9740:	885ffc80 	ldaxr	w0, [x4]
    9744:	6b1f001f 	cmp	w0, wzr
    9748:	54000061 	b.ne	9754 <__pthread_mutex_trylock+0x120>
    974c:	88017c97 	stxr	w1, w23, [x4]
    9750:	35ffff81 	cbnz	w1, 9740 <__pthread_mutex_trylock+0x10c>
    9754:	54000040 	b.eq	975c <__pthread_mutex_trylock+0x128>
    9758:	b9004fa0 	str	w0, [x29,#76]
    975c:	b9404fa0 	ldr	w0, [x29,#76]
    9760:	34001580 	cbz	w0, 9a10 <__pthread_mutex_trylock+0x3dc>
    9764:	37f01840 	tbnz	w0, #30, 9a6c <__pthread_mutex_trylock+0x438>
    9768:	f90078bf 	str	xzr, [x5,#240]
    976c:	52800213 	mov	w19, #0x10                  	// #16
    9770:	f9401bf7 	ldr	x23, [sp,#48]
    9774:	2a1303e0 	mov	w0, w19
    9778:	a94153f3 	ldp	x19, x20, [sp,#16]
    977c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    9780:	a8c57bfd 	ldp	x29, x30, [sp],#80
    9784:	d65f03c0 	ret
    9788:	7104041f 	cmp	w0, #0x101
    978c:	54fff820 	b.eq	9690 <__pthread_mutex_trylock+0x5c>
    9790:	528002d3 	mov	w19, #0x16                  	// #22
    9794:	f9401bf7 	ldr	x23, [sp,#48]
    9798:	2a1303e0 	mov	w0, w19
    979c:	a94153f3 	ldp	x19, x20, [sp,#16]
    97a0:	a9425bf5 	ldp	x21, x22, [sp,#32]
    97a4:	a8c57bfd 	ldp	x29, x30, [sp],#80
    97a8:	d65f03c0 	ret
    97ac:	b9004fbf 	str	wzr, [x29,#76]
    97b0:	52800020 	mov	w0, #0x1                   	// #1
    97b4:	885ffc81 	ldaxr	w1, [x4]
    97b8:	6b1f003f 	cmp	w1, wzr
    97bc:	54000061 	b.ne	97c8 <__pthread_mutex_trylock+0x194>
    97c0:	88027c80 	stxr	w2, w0, [x4]
    97c4:	35ffff82 	cbnz	w2, 97b4 <__pthread_mutex_trylock+0x180>
    97c8:	54fffd21 	b.ne	976c <__pthread_mutex_trylock+0x138>
    97cc:	b9400c80 	ldr	w0, [x4,#12]
    97d0:	52800013 	mov	w19, #0x0                   	// #0
    97d4:	b9000897 	str	w23, [x4,#8]
    97d8:	11000400 	add	w0, w0, #0x1
    97dc:	b9000c80 	str	w0, [x4,#12]
    97e0:	17ffffb5 	b	96b4 <__pthread_mutex_trylock+0x80>
    97e4:	b9400880 	ldr	w0, [x4,#8]
    97e8:	b9400081 	ldr	w1, [x4]
    97ec:	6b17001f 	cmp	w0, w23
    97f0:	540017a0 	b.eq	9ae4 <__pthread_mutex_trylock+0x4b0>
    97f4:	aa0403f5 	mov	x21, x4
    97f8:	12800016 	mov	w22, #0xffffffff            	// #-1
    97fc:	53137c34 	lsr	w20, w1, #19
    9800:	940021d6 	bl	11f58 <__pthread_current_priority>
    9804:	6b00029f 	cmp	w20, w0
    9808:	2a1403e1 	mov	w1, w20
    980c:	2a1603e0 	mov	w0, w22
    9810:	5400178b 	b.lt	9b00 <__pthread_mutex_trylock+0x4cc>
    9814:	94002111 	bl	11c58 <__pthread_tpp_change_priority>
    9818:	530d3282 	lsl	w2, w20, #19
    981c:	350017e0 	cbnz	w0, 9b18 <__pthread_mutex_trylock+0x4e4>
    9820:	b9004fa2 	str	w2, [x29,#76]
    9824:	32000040 	orr	w0, w2, #0x1
    9828:	2a0203e1 	mov	w1, w2
    982c:	885ffea3 	ldaxr	w3, [x21]
    9830:	6b01007f 	cmp	w3, w1
    9834:	54000061 	b.ne	9840 <__pthread_mutex_trylock+0x20c>
    9838:	88047ea0 	stxr	w4, w0, [x21]
    983c:	35ffff84 	cbnz	w4, 982c <__pthread_mutex_trylock+0x1f8>
    9840:	54000040 	b.eq	9848 <__pthread_mutex_trylock+0x214>
    9844:	b9004fa3 	str	w3, [x29,#76]
    9848:	b9404fa1 	ldr	w1, [x29,#76]
    984c:	2a1403f6 	mov	w22, w20
    9850:	6b01005f 	cmp	w2, w1
    9854:	120d3020 	and	w0, w1, #0xfff80000
    9858:	54001640 	b.eq	9b20 <__pthread_mutex_trylock+0x4ec>
    985c:	6b02001f 	cmp	w0, w2
    9860:	54fffce1 	b.ne	97fc <__pthread_mutex_trylock+0x1c8>
    9864:	2a1403e0 	mov	w0, w20
    9868:	12800001 	mov	w1, #0xffffffff            	// #-1
    986c:	52800213 	mov	w19, #0x10                  	// #16
    9870:	940020fa 	bl	11c58 <__pthread_tpp_change_priority>
    9874:	17ffff90 	b	96b4 <__pthread_mutex_trylock+0x80>
    9878:	51004000 	sub	w0, w0, #0x10
    987c:	71000c1f 	cmp	w0, #0x3
    9880:	54fff888 	b.hi	9790 <__pthread_mutex_trylock+0x15c>
    9884:	91008087 	add	x7, x4, #0x20
    9888:	f90078a7 	str	x7, [x5,#240]
    988c:	321f77e3 	mov	w3, #0x7ffffffe            	// #2147483646
    9890:	b9400081 	ldr	w1, [x4]
    9894:	12020033 	and	w19, w1, #0x40000000
    9898:	350007d3 	cbnz	w19, 9990 <__pthread_mutex_trylock+0x35c>
    989c:	12007421 	and	w1, w1, #0x3fffffff
    98a0:	6b0102ff 	cmp	w23, w1
    98a4:	54000360 	b.eq	9910 <__pthread_mutex_trylock+0x2dc>
    98a8:	b9004fbf 	str	wzr, [x29,#76]
    98ac:	885ffc80 	ldaxr	w0, [x4]
    98b0:	6b1f001f 	cmp	w0, wzr
    98b4:	54000061 	b.ne	98c0 <__pthread_mutex_trylock+0x28c>
    98b8:	88017c97 	stxr	w1, w23, [x4]
    98bc:	35ffff81 	cbnz	w1, 98ac <__pthread_mutex_trylock+0x278>
    98c0:	54000040 	b.eq	98c8 <__pthread_mutex_trylock+0x294>
    98c4:	b9004fa0 	str	w0, [x29,#76]
    98c8:	b9404fa1 	ldr	w1, [x29,#76]
    98cc:	340003c1 	cbz	w1, 9944 <__pthread_mutex_trylock+0x310>
    98d0:	12020033 	and	w19, w1, #0x40000000
    98d4:	36f7f4a1 	tbz	w1, #30, 9768 <__pthread_mutex_trylock+0x134>
    98d8:	b9400880 	ldr	w0, [x4,#8]
    98dc:	6b03001f 	cmp	w0, w3
    98e0:	54fffdc1 	b.ne	9898 <__pthread_mutex_trylock+0x264>
    98e4:	b900049f 	str	wzr, [x4,#4]
    98e8:	6b0102ff 	cmp	w23, w1
    98ec:	540012a0 	b.eq	9b40 <__pthread_mutex_trylock+0x50c>
    98f0:	f90078bf 	str	xzr, [x5,#240]
    98f4:	52801073 	mov	w19, #0x83                  	// #131
    98f8:	2a1303e0 	mov	w0, w19
    98fc:	f9401bf7 	ldr	x23, [sp,#48]
    9900:	a94153f3 	ldp	x19, x20, [sp,#16]
    9904:	a9425bf5 	ldp	x21, x22, [sp,#32]
    9908:	a8c57bfd 	ldp	x29, x30, [sp],#80
    990c:	d65f03c0 	ret
    9910:	b9401080 	ldr	w0, [x4,#16]
    9914:	12001800 	and	w0, w0, #0x7f
    9918:	7100481f 	cmp	w0, #0x12
    991c:	54001480 	b.eq	9bac <__pthread_mutex_trylock+0x578>
    9920:	7100441f 	cmp	w0, #0x11
    9924:	54fffc21 	b.ne	98a8 <__pthread_mutex_trylock+0x274>
    9928:	f90078bf 	str	xzr, [x5,#240]
    992c:	b9400480 	ldr	w0, [x4,#4]
    9930:	3100041f 	cmn	w0, #0x1
    9934:	54000d40 	b.eq	9adc <__pthread_mutex_trylock+0x4a8>
    9938:	11000400 	add	w0, w0, #0x1
    993c:	b9000480 	str	w0, [x4,#4]
    9940:	17ffff5d 	b	96b4 <__pthread_mutex_trylock+0x80>
    9944:	b9400882 	ldr	w2, [x4,#8]
    9948:	321f77e0 	mov	w0, #0x7ffffffe            	// #2147483646
    994c:	6b00005f 	cmp	w2, w0
    9950:	54fffca0 	b.eq	98e4 <__pthread_mutex_trylock+0x2b0>
    9954:	aa0503e0 	mov	x0, x5
    9958:	f84e0c01 	ldr	x1, [x0,#224]!
    995c:	927ff822 	and	x2, x1, #0xfffffffffffffffe
    9960:	f81f8047 	str	x7, [x2,#-8]
    9964:	f9001081 	str	x1, [x4,#32]
    9968:	52800021 	mov	w1, #0x1                   	// #1
    996c:	f9000c80 	str	x0, [x4,#24]
    9970:	f90070a7 	str	x7, [x5,#224]
    9974:	f90078bf 	str	xzr, [x5,#240]
    9978:	b9000897 	str	w23, [x4,#8]
    997c:	b9400c80 	ldr	w0, [x4,#12]
    9980:	b9000481 	str	w1, [x4,#4]
    9984:	0b010000 	add	w0, w0, w1
    9988:	b9000c80 	str	w0, [x4,#12]
    998c:	17ffff4a 	b	96b4 <__pthread_mutex_trylock+0x80>
    9990:	12010020 	and	w0, w1, #0x80000000
    9994:	b9004fa1 	str	w1, [x29,#76]
    9998:	2a170000 	orr	w0, w0, w23
    999c:	2a0103e2 	mov	w2, w1
    99a0:	885ffc86 	ldaxr	w6, [x4]
    99a4:	6b0200df 	cmp	w6, w2
    99a8:	54000061 	b.ne	99b4 <__pthread_mutex_trylock+0x380>
    99ac:	88087c80 	stxr	w8, w0, [x4]
    99b0:	35ffff88 	cbnz	w8, 99a0 <__pthread_mutex_trylock+0x36c>
    99b4:	54000040 	b.eq	99bc <__pthread_mutex_trylock+0x388>
    99b8:	b9004fa6 	str	w6, [x29,#76]
    99bc:	b9404fa0 	ldr	w0, [x29,#76]
    99c0:	6b00003f 	cmp	w1, w0
    99c4:	54000d80 	b.eq	9b74 <__pthread_mutex_trylock+0x540>
    99c8:	2a0003e1 	mov	w1, w0
    99cc:	12020013 	and	w19, w0, #0x40000000
    99d0:	17ffffb2 	b	9898 <__pthread_mutex_trylock+0x264>
    99d4:	b9004fbf 	str	wzr, [x29,#76]
    99d8:	52800020 	mov	w0, #0x1                   	// #1
    99dc:	885ffc81 	ldaxr	w1, [x4]
    99e0:	6b1f003f 	cmp	w1, wzr
    99e4:	54000061 	b.ne	99f0 <__pthread_mutex_trylock+0x3bc>
    99e8:	88027c80 	stxr	w2, w0, [x4]
    99ec:	35ffff82 	cbnz	w2, 99dc <__pthread_mutex_trylock+0x3a8>
    99f0:	54ffebe1 	b.ne	976c <__pthread_mutex_trylock+0x138>
    99f4:	b9400c81 	ldr	w1, [x4,#12]
    99f8:	52800013 	mov	w19, #0x0                   	// #0
    99fc:	b9000897 	str	w23, [x4,#8]
    9a00:	11000421 	add	w1, w1, #0x1
    9a04:	b9000480 	str	w0, [x4,#4]
    9a08:	b9000c81 	str	w1, [x4,#12]
    9a0c:	17ffff2a 	b	96b4 <__pthread_mutex_trylock+0x80>
    9a10:	340001e6 	cbz	w6, 9a4c <__pthread_mutex_trylock+0x418>
    9a14:	b9400881 	ldr	w1, [x4,#8]
    9a18:	321f77e0 	mov	w0, #0x7ffffffe            	// #2147483646
    9a1c:	6b00003f 	cmp	w1, w0
    9a20:	54001020 	b.eq	9c24 <__pthread_mutex_trylock+0x5f0>
    9a24:	aa0503e0 	mov	x0, x5
    9a28:	91008082 	add	x2, x4, #0x20
    9a2c:	b2400043 	orr	x3, x2, #0x1
    9a30:	f84e0c01 	ldr	x1, [x0,#224]!
    9a34:	927ff826 	and	x6, x1, #0xfffffffffffffffe
    9a38:	f81f80c2 	str	x2, [x6,#-8]
    9a3c:	f9001081 	str	x1, [x4,#32]
    9a40:	f9000c80 	str	x0, [x4,#24]
    9a44:	f90070a3 	str	x3, [x5,#224]
    9a48:	f90078bf 	str	xzr, [x5,#240]
    9a4c:	b9400c80 	ldr	w0, [x4,#12]
    9a50:	52800021 	mov	w1, #0x1                   	// #1
    9a54:	b9000897 	str	w23, [x4,#8]
    9a58:	52800013 	mov	w19, #0x0                   	// #0
    9a5c:	11000400 	add	w0, w0, #0x1
    9a60:	b9000481 	str	w1, [x4,#4]
    9a64:	b9000c80 	str	w0, [x4,#12]
    9a68:	17ffff13 	b	96b4 <__pthread_mutex_trylock+0x80>
    9a6c:	d2800101 	mov	x1, #0x8                   	// #8
    9a70:	350000c6 	cbnz	w6, 9a88 <__pthread_mutex_trylock+0x454>
    9a74:	b9401081 	ldr	w1, [x4,#16]
    9a78:	52801100 	mov	w0, #0x88                  	// #136
    9a7c:	12190021 	and	w1, w1, #0x80
    9a80:	4a000021 	eor	w1, w1, w0
    9a84:	93407c21 	sxtw	x1, w1
    9a88:	d2800002 	mov	x2, #0x0                   	// #0
    9a8c:	aa0403e0 	mov	x0, x4
    9a90:	aa0203e3 	mov	x3, x2
    9a94:	d2800c48 	mov	x8, #0x62                  	// #98
    9a98:	d4000001 	svc	#0x0
    9a9c:	31002c1f 	cmn	w0, #0xb
    9aa0:	540008c1 	b.ne	9bb8 <__pthread_mutex_trylock+0x584>
    9aa4:	3140041f 	cmn	w0, #0x1, lsl #12
    9aa8:	54000889 	b.ls	9bb8 <__pthread_mutex_trylock+0x584>
    9aac:	f90078a2 	str	x2, [x5,#240]
    9ab0:	52800213 	mov	w19, #0x10                  	// #16
    9ab4:	17ffff00 	b	96b4 <__pthread_mutex_trylock+0x80>
    9ab8:	12000442 	and	w2, w2, #0x3
    9abc:	7100085f 	cmp	w2, #0x2
    9ac0:	54000760 	b.eq	9bac <__pthread_mutex_trylock+0x578>
    9ac4:	7100045f 	cmp	w2, #0x1
    9ac8:	54ffe3a1 	b.ne	973c <__pthread_mutex_trylock+0x108>
    9acc:	f90078bf 	str	xzr, [x5,#240]
    9ad0:	b9400480 	ldr	w0, [x4,#4]
    9ad4:	3100041f 	cmn	w0, #0x1
    9ad8:	54ffde81 	b.ne	96a8 <__pthread_mutex_trylock+0x74>
    9adc:	52800173 	mov	w19, #0xb                   	// #11
    9ae0:	17fffef5 	b	96b4 <__pthread_mutex_trylock+0x80>
    9ae4:	12000442 	and	w2, w2, #0x3
    9ae8:	52800473 	mov	w19, #0x23                  	// #35
    9aec:	7100085f 	cmp	w2, #0x2
    9af0:	54ffde20 	b.eq	96b4 <__pthread_mutex_trylock+0x80>
    9af4:	7100045f 	cmp	w2, #0x1
    9af8:	54ffe7e1 	b.ne	97f4 <__pthread_mutex_trylock+0x1c0>
    9afc:	17fffee8 	b	969c <__pthread_mutex_trylock+0x68>
    9b00:	310006df 	cmn	w22, #0x1
    9b04:	528002d3 	mov	w19, #0x16                  	// #22
    9b08:	54ffdd60 	b.eq	96b4 <__pthread_mutex_trylock+0x80>
    9b0c:	12800001 	mov	w1, #0xffffffff            	// #-1
    9b10:	94002052 	bl	11c58 <__pthread_tpp_change_priority>
    9b14:	17fffee8 	b	96b4 <__pthread_mutex_trylock+0x80>
    9b18:	2a0003f3 	mov	w19, w0
    9b1c:	17fffee6 	b	96b4 <__pthread_mutex_trylock+0x80>
    9b20:	b9400ea0 	ldr	w0, [x21,#12]
    9b24:	52800021 	mov	w1, #0x1                   	// #1
    9b28:	b9000ab7 	str	w23, [x21,#8]
    9b2c:	52800013 	mov	w19, #0x0                   	// #0
    9b30:	11000400 	add	w0, w0, #0x1
    9b34:	b90006a1 	str	w1, [x21,#4]
    9b38:	b9000ea0 	str	w0, [x21,#12]
    9b3c:	17fffede 	b	96b4 <__pthread_mutex_trylock+0x80>
    9b40:	52800001 	mov	w1, #0x0                   	// #0
    9b44:	885f7c80 	ldxr	w0, [x4]
    9b48:	8802fc81 	stlxr	w2, w1, [x4]
    9b4c:	35ffffc2 	cbnz	w2, 9b44 <__pthread_mutex_trylock+0x510>
    9b50:	7100041f 	cmp	w0, #0x1
    9b54:	54ffeced 	b.le	98f0 <__pthread_mutex_trylock+0x2bc>
    9b58:	d2800021 	mov	x1, #0x1                   	// #1
    9b5c:	aa0403e0 	mov	x0, x4
    9b60:	aa0103e2 	mov	x2, x1
    9b64:	d2800003 	mov	x3, #0x0                   	// #0
    9b68:	d2800c48 	mov	x8, #0x62                  	// #98
    9b6c:	d4000001 	svc	#0x0
    9b70:	17ffff60 	b	98f0 <__pthread_mutex_trylock+0x2bc>
    9b74:	aa0503e0 	mov	x0, x5
    9b78:	52800021 	mov	w1, #0x1                   	// #1
    9b7c:	b9000481 	str	w1, [x4,#4]
    9b80:	12b00001 	mov	w1, #0x7fffffff            	// #2147483647
    9b84:	b9000881 	str	w1, [x4,#8]
    9b88:	52801053 	mov	w19, #0x82                  	// #130
    9b8c:	f84e0c01 	ldr	x1, [x0,#224]!
    9b90:	927ff822 	and	x2, x1, #0xfffffffffffffffe
    9b94:	f81f8047 	str	x7, [x2,#-8]
    9b98:	f9001081 	str	x1, [x4,#32]
    9b9c:	f9000c80 	str	x0, [x4,#24]
    9ba0:	f90070a7 	str	x7, [x5,#224]
    9ba4:	f90078bf 	str	xzr, [x5,#240]
    9ba8:	17fffec3 	b	96b4 <__pthread_mutex_trylock+0x80>
    9bac:	f90078bf 	str	xzr, [x5,#240]
    9bb0:	52800473 	mov	w19, #0x23                  	// #35
    9bb4:	17fffec0 	b	96b4 <__pthread_mutex_trylock+0x80>
    9bb8:	b9400080 	ldr	w0, [x4]
    9bbc:	36f7f2a0 	tbz	w0, #30, 9a10 <__pthread_mutex_trylock+0x3dc>
    9bc0:	b9004fa0 	str	w0, [x29,#76]
    9bc4:	12017801 	and	w1, w0, #0xbfffffff
    9bc8:	885ffc82 	ldaxr	w2, [x4]
    9bcc:	6b00005f 	cmp	w2, w0
    9bd0:	54000061 	b.ne	9bdc <__pthread_mutex_trylock+0x5a8>
    9bd4:	88037c81 	stxr	w3, w1, [x4]
    9bd8:	35ffff83 	cbnz	w3, 9bc8 <__pthread_mutex_trylock+0x594>
    9bdc:	54000060 	b.eq	9be8 <__pthread_mutex_trylock+0x5b4>
    9be0:	b9400080 	ldr	w0, [x4]
    9be4:	17fffff7 	b	9bc0 <__pthread_mutex_trylock+0x58c>
    9be8:	aa0503e0 	mov	x0, x5
    9bec:	52800021 	mov	w1, #0x1                   	// #1
    9bf0:	b9000481 	str	w1, [x4,#4]
    9bf4:	12b00001 	mov	w1, #0x7fffffff            	// #2147483647
    9bf8:	b9000881 	str	w1, [x4,#8]
    9bfc:	91008081 	add	x1, x4, #0x20
    9c00:	52801053 	mov	w19, #0x82                  	// #130
    9c04:	f84e0c02 	ldr	x2, [x0,#224]!
    9c08:	927ff843 	and	x3, x2, #0xfffffffffffffffe
    9c0c:	f81f8061 	str	x1, [x3,#-8]
    9c10:	f9001082 	str	x2, [x4,#32]
    9c14:	f9000c80 	str	x0, [x4,#24]
    9c18:	f90070a1 	str	x1, [x5,#224]
    9c1c:	f90078bf 	str	xzr, [x5,#240]
    9c20:	17fffea5 	b	96b4 <__pthread_mutex_trylock+0x80>
    9c24:	d2800002 	mov	x2, #0x0                   	// #0
    9c28:	b900049f 	str	wzr, [x4,#4]
    9c2c:	aa0403e0 	mov	x0, x4
    9c30:	d28000e1 	mov	x1, #0x7                   	// #7
    9c34:	aa0203e3 	mov	x3, x2
    9c38:	d2800c48 	mov	x8, #0x62                  	// #98
    9c3c:	d4000001 	svc	#0x0
    9c40:	52801073 	mov	w19, #0x83                  	// #131
    9c44:	f90078a2 	str	x2, [x5,#240]
    9c48:	17fffe9b 	b	96b4 <__pthread_mutex_trylock+0x80>

0000000000009c4c <pthread_mutex_timedlock>:
    9c4c:	a9b77bfd 	stp	x29, x30, [sp,#-144]!
    9c50:	52802fe2 	mov	w2, #0x17f                 	// #383
    9c54:	910003fd 	mov	x29, sp
    9c58:	a90153f3 	stp	x19, x20, [sp,#16]
    9c5c:	a9025bf5 	stp	x21, x22, [sp,#32]
    9c60:	a90363f7 	stp	x23, x24, [sp,#48]
    9c64:	a9046bf9 	stp	x25, x26, [sp,#64]
    9c68:	a90573fb 	stp	x27, x28, [sp,#80]
    9c6c:	fd0033e8 	str	d8, [sp,#96]
    9c70:	d53bd054 	mrs	x20, tpidr_el0
    9c74:	b9401003 	ldr	w3, [x0,#16]
    9c78:	d11bc294 	sub	x20, x20, #0x6f0
    9c7c:	aa0003f3 	mov	x19, x0
    9c80:	aa0103f5 	mov	x21, x1
    9c84:	0a020062 	and	w2, w3, w2
    9c88:	71008c5f 	cmp	w2, #0x23
    9c8c:	b940d296 	ldr	w22, [x20,#208]
    9c90:	54000788 	b.hi	9d80 <pthread_mutex_timedlock+0x134>
    9c94:	7100805f 	cmp	w2, #0x20
    9c98:	54000822 	b.cs	9d9c <pthread_mutex_timedlock+0x150>
    9c9c:	7100085f 	cmp	w2, #0x2
    9ca0:	54001560 	b.eq	9f4c <pthread_mutex_timedlock+0x300>
    9ca4:	54000f69 	b.ls	9e90 <pthread_mutex_timedlock+0x244>
    9ca8:	71000c5f 	cmp	w2, #0x3
    9cac:	540015a0 	b.eq	9f60 <pthread_mutex_timedlock+0x314>
    9cb0:	51004042 	sub	w2, w2, #0x10
    9cb4:	71000c5f 	cmp	w2, #0x3
    9cb8:	54002ce8 	b.hi	a254 <pthread_mutex_timedlock+0x608>
    9cbc:	91008018 	add	x24, x0, #0x20
    9cc0:	f9007a98 	str	x24, [x20,#240]
    9cc4:	321f77f7 	mov	w23, #0x7ffffffe            	// #2147483646
    9cc8:	b9400002 	ldr	w2, [x0]
    9ccc:	12020040 	and	w0, w2, #0x40000000
    9cd0:	35000360 	cbnz	w0, 9d3c <pthread_mutex_timedlock+0xf0>
    9cd4:	12007442 	and	w2, w2, #0x3fffffff
    9cd8:	6b0202df 	cmp	w22, w2
    9cdc:	54001a60 	b.eq	a028 <pthread_mutex_timedlock+0x3dc>
    9ce0:	b90083bf 	str	wzr, [x29,#128]
    9ce4:	885ffe63 	ldaxr	w3, [x19]
    9ce8:	6b1f007f 	cmp	w3, wzr
    9cec:	54000061 	b.ne	9cf8 <pthread_mutex_timedlock+0xac>
    9cf0:	88007e76 	stxr	w0, w22, [x19]
    9cf4:	35ffff80 	cbnz	w0, 9ce4 <pthread_mutex_timedlock+0x98>
    9cf8:	54001b20 	b.eq	a05c <pthread_mutex_timedlock+0x410>
    9cfc:	52801002 	mov	w2, #0x80                  	// #128
    9d00:	aa1303e0 	mov	x0, x19
    9d04:	aa1503e1 	mov	x1, x21
    9d08:	b90083a3 	str	w3, [x29,#128]
    9d0c:	9400177b 	bl	faf8 <__lll_robust_timedlock_wait>
    9d10:	2a0003e2 	mov	w2, w0
    9d14:	b9400a60 	ldr	w0, [x19,#8]
    9d18:	6b17001f 	cmp	w0, w23
    9d1c:	54002ae0 	b.eq	a278 <pthread_mutex_timedlock+0x62c>
    9d20:	7100585f 	cmp	w2, #0x16
    9d24:	54003760 	b.eq	a410 <pthread_mutex_timedlock+0x7c4>
    9d28:	7101b85f 	cmp	w2, #0x6e
    9d2c:	54003720 	b.eq	a410 <pthread_mutex_timedlock+0x7c4>
    9d30:	12020040 	and	w0, w2, #0x40000000
    9d34:	36f019e2 	tbz	w2, #30, a070 <pthread_mutex_timedlock+0x424>
    9d38:	34fffce0 	cbz	w0, 9cd4 <pthread_mutex_timedlock+0x88>
    9d3c:	12010040 	and	w0, w2, #0x80000000
    9d40:	b90083a2 	str	w2, [x29,#128]
    9d44:	2a160000 	orr	w0, w0, w22
    9d48:	2a0203e1 	mov	w1, w2
    9d4c:	885ffe63 	ldaxr	w3, [x19]
    9d50:	6b01007f 	cmp	w3, w1
    9d54:	54000061 	b.ne	9d60 <pthread_mutex_timedlock+0x114>
    9d58:	88047e60 	stxr	w4, w0, [x19]
    9d5c:	35ffff84 	cbnz	w4, 9d4c <pthread_mutex_timedlock+0x100>
    9d60:	54000040 	b.eq	9d68 <pthread_mutex_timedlock+0x11c>
    9d64:	b90083a3 	str	w3, [x29,#128]
    9d68:	b94083a0 	ldr	w0, [x29,#128]
    9d6c:	6b00005f 	cmp	w2, w0
    9d70:	54003880 	b.eq	a480 <pthread_mutex_timedlock+0x834>
    9d74:	2a0003e2 	mov	w2, w0
    9d78:	12020000 	and	w0, w0, #0x40000000
    9d7c:	17ffffd5 	b	9cd0 <pthread_mutex_timedlock+0x84>
    9d80:	71010c5f 	cmp	w2, #0x43
    9d84:	54000b28 	b.hi	9ee8 <pthread_mutex_timedlock+0x29c>
    9d88:	7101005f 	cmp	w2, #0x40
    9d8c:	54001882 	b.cs	a09c <pthread_mutex_timedlock+0x450>
    9d90:	5100c042 	sub	w2, w2, #0x30
    9d94:	71000c5f 	cmp	w2, #0x3
    9d98:	540025e8 	b.hi	a254 <pthread_mutex_timedlock+0x608>
    9d9c:	121c0064 	and	w4, w3, #0x10
    9da0:	36200083 	tbz	w3, #4, 9db0 <pthread_mutex_timedlock+0x164>
    9da4:	91008260 	add	x0, x19, #0x20
    9da8:	b2400000 	orr	x0, x0, #0x1
    9dac:	f9007a80 	str	x0, [x20,#240]
    9db0:	b9400260 	ldr	w0, [x19]
    9db4:	12007400 	and	w0, w0, #0x3fffffff
    9db8:	6b0002df 	cmp	w22, w0
    9dbc:	54002d40 	b.eq	a364 <pthread_mutex_timedlock+0x718>
    9dc0:	b90083bf 	str	wzr, [x29,#128]
    9dc4:	885ffe60 	ldaxr	w0, [x19]
    9dc8:	6b1f001f 	cmp	w0, wzr
    9dcc:	54000061 	b.ne	9dd8 <pthread_mutex_timedlock+0x18c>
    9dd0:	88017e76 	stxr	w1, w22, [x19]
    9dd4:	35ffff81 	cbnz	w1, 9dc4 <pthread_mutex_timedlock+0x178>
    9dd8:	54000040 	b.eq	9de0 <pthread_mutex_timedlock+0x194>
    9ddc:	b90083a0 	str	w0, [x29,#128]
    9de0:	b94083a0 	ldr	w0, [x29,#128]
    9de4:	340025e0 	cbz	w0, a2a0 <pthread_mutex_timedlock+0x654>
    9de8:	d28000c1 	mov	x1, #0x6                   	// #6
    9dec:	350000c4 	cbnz	w4, 9e04 <pthread_mutex_timedlock+0x1b8>
    9df0:	b9401261 	ldr	w1, [x19,#16]
    9df4:	528010c0 	mov	w0, #0x86                  	// #134
    9df8:	12190021 	and	w1, w1, #0x80
    9dfc:	4a000021 	eor	w1, w1, w0
    9e00:	93407c21 	sxtw	x1, w1
    9e04:	aa1303e0 	mov	x0, x19
    9e08:	d2800022 	mov	x2, #0x1                   	// #1
    9e0c:	aa1503e3 	mov	x3, x21
    9e10:	d2800c48 	mov	x8, #0x62                  	// #98
    9e14:	d4000001 	svc	#0x0
    9e18:	3140041f 	cmn	w0, #0x1, lsl #12
    9e1c:	aa0003e1 	mov	x1, x0
    9e20:	54002849 	b.ls	a328 <pthread_mutex_timedlock+0x6dc>
    9e24:	3101b81f 	cmn	w0, #0x6e
    9e28:	52800dc0 	mov	w0, #0x6e                  	// #110
    9e2c:	540004e0 	b.eq	9ec8 <pthread_mutex_timedlock+0x27c>
    9e30:	121a7820 	and	w0, w1, #0xffffffdf
    9e34:	31008c1f 	cmn	w0, #0x23
    9e38:	54003401 	b.ne	a4b8 <pthread_mutex_timedlock+0x86c>
    9e3c:	d2800000 	mov	x0, #0x0                   	// #0
    9e40:	910203a1 	add	x1, x29, #0x80
    9e44:	d2800e28 	mov	x8, #0x71                  	// #113
    9e48:	d4000001 	svc	#0x0
    9e4c:	f94002a0 	ldr	x0, [x21]
    9e50:	f94006a1 	ldr	x1, [x21,#8]
    9e54:	f94047a2 	ldr	x2, [x29,#136]
    9e58:	f94043a3 	ldr	x3, [x29,#128]
    9e5c:	eb020021 	subs	x1, x1, x2
    9e60:	cb030000 	sub	x0, x0, x3
    9e64:	f9003ba0 	str	x0, [x29,#112]
    9e68:	540034c4 	b.mi	a500 <pthread_mutex_timedlock+0x8b4>
    9e6c:	f9003fa1 	str	x1, [x29,#120]
    9e70:	b7f800c0 	tbnz	x0, #63, 9e88 <pthread_mutex_timedlock+0x23c>
    9e74:	9101c3b3 	add	x19, x29, #0x70
    9e78:	aa1303e0 	mov	x0, x19
    9e7c:	aa1303e1 	mov	x1, x19
    9e80:	940019bc 	bl	10570 <__nanosleep_nocancel>
    9e84:	35ffffa0 	cbnz	w0, 9e78 <pthread_mutex_timedlock+0x22c>
    9e88:	52800dc0 	mov	w0, #0x6e                  	// #110
    9e8c:	1400000f 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    9e90:	35000b42 	cbnz	w2, 9ff8 <pthread_mutex_timedlock+0x3ac>
    9e94:	b90083bf 	str	wzr, [x29,#128]
    9e98:	52800020 	mov	w0, #0x1                   	// #1
    9e9c:	885ffe63 	ldaxr	w3, [x19]
    9ea0:	6b1f007f 	cmp	w3, wzr
    9ea4:	54000061 	b.ne	9eb0 <pthread_mutex_timedlock+0x264>
    9ea8:	88017e60 	stxr	w1, w0, [x19]
    9eac:	35ffff81 	cbnz	w1, 9e9c <pthread_mutex_timedlock+0x250>
    9eb0:	540003a1 	b.ne	9f24 <pthread_mutex_timedlock+0x2d8>
    9eb4:	b9400e61 	ldr	w1, [x19,#12]
    9eb8:	52800000 	mov	w0, #0x0                   	// #0
    9ebc:	b9000a76 	str	w22, [x19,#8]
    9ec0:	11000421 	add	w1, w1, #0x1
    9ec4:	b9000e61 	str	w1, [x19,#12]
    9ec8:	a94153f3 	ldp	x19, x20, [sp,#16]
    9ecc:	a9425bf5 	ldp	x21, x22, [sp,#32]
    9ed0:	a94363f7 	ldp	x23, x24, [sp,#48]
    9ed4:	a9446bf9 	ldp	x25, x26, [sp,#64]
    9ed8:	a94573fb 	ldp	x27, x28, [sp,#80]
    9edc:	fd4033e8 	ldr	d8, [sp,#96]
    9ee0:	a8c97bfd 	ldp	x29, x30, [sp],#144
    9ee4:	d65f03c0 	ret
    9ee8:	7104005f 	cmp	w2, #0x100
    9eec:	54001b01 	b.ne	a24c <pthread_mutex_timedlock+0x600>
    9ef0:	b90083bf 	str	wzr, [x29,#128]
    9ef4:	52800022 	mov	w2, #0x1                   	// #1
    9ef8:	885ffe63 	ldaxr	w3, [x19]
    9efc:	6b1f007f 	cmp	w3, wzr
    9f00:	54000061 	b.ne	9f0c <pthread_mutex_timedlock+0x2c0>
    9f04:	88047e62 	stxr	w4, w2, [x19]
    9f08:	35ffff84 	cbnz	w4, 9ef8 <pthread_mutex_timedlock+0x2ac>
    9f0c:	540018e0 	b.eq	a228 <pthread_mutex_timedlock+0x5dc>
    9f10:	b9401262 	ldr	w2, [x19,#16]
    9f14:	b90083a3 	str	w3, [x29,#128]
    9f18:	12190042 	and	w2, w2, #0x80
    9f1c:	94001645 	bl	f830 <__lll_timedlock_wait>
    9f20:	17ffffea 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    9f24:	b9401262 	ldr	w2, [x19,#16]
    9f28:	aa1303e0 	mov	x0, x19
    9f2c:	aa1503e1 	mov	x1, x21
    9f30:	b90083a3 	str	w3, [x29,#128]
    9f34:	12190042 	and	w2, w2, #0x80
    9f38:	9400163e 	bl	f830 <__lll_timedlock_wait>
    9f3c:	2a0003e2 	mov	w2, w0
    9f40:	2a0203e0 	mov	w0, w2
    9f44:	35fffc22 	cbnz	w2, 9ec8 <pthread_mutex_timedlock+0x27c>
    9f48:	17ffffdb 	b	9eb4 <pthread_mutex_timedlock+0x268>
    9f4c:	b9400800 	ldr	w0, [x0,#8]
    9f50:	6b0002df 	cmp	w22, w0
    9f54:	54fffa01 	b.ne	9e94 <pthread_mutex_timedlock+0x248>
    9f58:	52800460 	mov	w0, #0x23                  	// #35
    9f5c:	17ffffdb 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    9f60:	f0000140 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    9f64:	b9432c00 	ldr	w0, [x0,#812]
    9f68:	34fff960 	cbz	w0, 9e94 <pthread_mutex_timedlock+0x248>
    9f6c:	b90083bf 	str	wzr, [x29,#128]
    9f70:	52800022 	mov	w2, #0x1                   	// #1
    9f74:	885ffe60 	ldaxr	w0, [x19]
    9f78:	6b1f001f 	cmp	w0, wzr
    9f7c:	54000061 	b.ne	9f88 <pthread_mutex_timedlock+0x33c>
    9f80:	88017e62 	stxr	w1, w2, [x19]
    9f84:	35ffff81 	cbnz	w1, 9f74 <pthread_mutex_timedlock+0x328>
    9f88:	1a9f17f4 	cset	w20, eq
    9f8c:	35fff954 	cbnz	w20, 9eb4 <pthread_mutex_timedlock+0x268>
    9f90:	b9401661 	ldr	w1, [x19,#20]
    9f94:	52800c80 	mov	w0, #0x64                  	// #100
    9f98:	11001421 	add	w1, w1, #0x5
    9f9c:	531f7821 	lsl	w1, w1, #1
    9fa0:	7101903f 	cmp	w1, #0x64
    9fa4:	1a80d021 	csel	w1, w1, w0, le
    9fa8:	11000694 	add	w20, w20, #0x1
    9fac:	b90083bf 	str	wzr, [x29,#128]
    9fb0:	51000680 	sub	w0, w20, #0x1
    9fb4:	6b01001f 	cmp	w0, w1
    9fb8:	5400200a 	b.ge	a3b8 <pthread_mutex_timedlock+0x76c>
    9fbc:	885ffe60 	ldaxr	w0, [x19]
    9fc0:	6b1f001f 	cmp	w0, wzr
    9fc4:	54000061 	b.ne	9fd0 <pthread_mutex_timedlock+0x384>
    9fc8:	88037e62 	stxr	w3, w2, [x19]
    9fcc:	35ffff83 	cbnz	w3, 9fbc <pthread_mutex_timedlock+0x370>
    9fd0:	54fffec1 	b.ne	9fa8 <pthread_mutex_timedlock+0x35c>
    9fd4:	52800002 	mov	w2, #0x0                   	// #0
    9fd8:	b9401661 	ldr	w1, [x19,#20]
    9fdc:	4b010294 	sub	w20, w20, w1
    9fe0:	11001e80 	add	w0, w20, #0x7
    9fe4:	6b1f029f 	cmp	w20, wzr
    9fe8:	1a94b014 	csel	w20, w0, w20, lt
    9fec:	0b940c34 	add	w20, w1, w20, asr #3
    9ff0:	b9001674 	str	w20, [x19,#20]
    9ff4:	17ffffd3 	b	9f40 <pthread_mutex_timedlock+0x2f4>
    9ff8:	7100045f 	cmp	w2, #0x1
    9ffc:	540012c1 	b.ne	a254 <pthread_mutex_timedlock+0x608>
    a000:	b9400a60 	ldr	w0, [x19,#8]
    a004:	6b16001f 	cmp	w0, w22
    a008:	54001701 	b.ne	a2e8 <pthread_mutex_timedlock+0x69c>
    a00c:	b9400660 	ldr	w0, [x19,#4]
    a010:	3100041f 	cmn	w0, #0x1
    a014:	54001ba0 	b.eq	a388 <pthread_mutex_timedlock+0x73c>
    a018:	11000401 	add	w1, w0, #0x1
    a01c:	52800000 	mov	w0, #0x0                   	// #0
    a020:	b9000661 	str	w1, [x19,#4]
    a024:	17ffffa9 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a028:	b9401261 	ldr	w1, [x19,#16]
    a02c:	12001821 	and	w1, w1, #0x7f
    a030:	7100483f 	cmp	w1, #0x12
    a034:	54002200 	b.eq	a474 <pthread_mutex_timedlock+0x828>
    a038:	7100443f 	cmp	w1, #0x11
    a03c:	54ffe521 	b.ne	9ce0 <pthread_mutex_timedlock+0x94>
    a040:	f9007a9f 	str	xzr, [x20,#240]
    a044:	b9400661 	ldr	w1, [x19,#4]
    a048:	3100043f 	cmn	w1, #0x1
    a04c:	540019e0 	b.eq	a388 <pthread_mutex_timedlock+0x73c>
    a050:	11000421 	add	w1, w1, #0x1
    a054:	b9000661 	str	w1, [x19,#4]
    a058:	17ffff9c 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a05c:	b9400a61 	ldr	w1, [x19,#8]
    a060:	321f77e0 	mov	w0, #0x7ffffffe            	// #2147483646
    a064:	6b00003f 	cmp	w1, w0
    a068:	54001080 	b.eq	a278 <pthread_mutex_timedlock+0x62c>
    a06c:	52800002 	mov	w2, #0x0                   	// #0
    a070:	aa1403e0 	mov	x0, x20
    a074:	52800021 	mov	w1, #0x1                   	// #1
    a078:	b9000661 	str	w1, [x19,#4]
    a07c:	f84e0c01 	ldr	x1, [x0,#224]!
    a080:	927ff823 	and	x3, x1, #0xfffffffffffffffe
    a084:	f81f8078 	str	x24, [x3,#-8]
    a088:	f9001261 	str	x1, [x19,#32]
    a08c:	f9000e60 	str	x0, [x19,#24]
    a090:	f9007298 	str	x24, [x20,#224]
    a094:	f9007a9f 	str	xzr, [x20,#240]
    a098:	17ffffaa 	b	9f40 <pthread_mutex_timedlock+0x2f4>
    a09c:	b9400800 	ldr	w0, [x0,#8]
    a0a0:	b9400261 	ldr	w1, [x19]
    a0a4:	6b16001f 	cmp	w0, w22
    a0a8:	54001a60 	b.eq	a3f4 <pthread_mutex_timedlock+0x7a8>
    a0ac:	12800014 	mov	w20, #0xffffffff            	// #-1
    a0b0:	9101c3b8 	add	x24, x29, #0x70
    a0b4:	53137c20 	lsr	w0, w1, #19
    a0b8:	2a0003f7 	mov	w23, w0
    a0bc:	1e270008 	fmov	s8, w0
    a0c0:	94001fa6 	bl	11f58 <__pthread_current_priority>
    a0c4:	6b0002ff 	cmp	w23, w0
    a0c8:	54001fcb 	b.lt	a4c0 <pthread_mutex_timedlock+0x874>
    a0cc:	2a1403e0 	mov	w0, w20
    a0d0:	2a1703e1 	mov	w1, w23
    a0d4:	94001ee1 	bl	11c58 <__pthread_tpp_change_priority>
    a0d8:	35ffef80 	cbnz	w0, 9ec8 <pthread_mutex_timedlock+0x27c>
    a0dc:	910243bc 	add	x28, x29, #0x90
    a0e0:	530d32f4 	lsl	w20, w23, #19
    a0e4:	32000297 	orr	w23, w20, #0x1
    a0e8:	2a1403e0 	mov	w0, w20
    a0ec:	b81f0f94 	str	w20, [x28,#-16]!
    a0f0:	885ffe61 	ldaxr	w1, [x19]
    a0f4:	6b00003f 	cmp	w1, w0
    a0f8:	54000061 	b.ne	a104 <pthread_mutex_timedlock+0x4b8>
    a0fc:	88027e77 	stxr	w2, w23, [x19]
    a100:	35ffff82 	cbnz	w2, a0f0 <pthread_mutex_timedlock+0x4a4>
    a104:	54000040 	b.eq	a10c <pthread_mutex_timedlock+0x4c0>
    a108:	b90083a1 	str	w1, [x29,#128]
    a10c:	b94083a0 	ldr	w0, [x29,#128]
    a110:	6b00029f 	cmp	w20, w0
    a114:	54000780 	b.eq	a204 <pthread_mutex_timedlock+0x5b8>
    a118:	321f029b 	orr	w27, w20, #0x2
    a11c:	910203ba 	add	x26, x29, #0x80
    a120:	93407f79 	sxtw	x25, w27
    a124:	b90083b7 	str	w23, [x29,#128]
    a128:	b9400380 	ldr	w0, [x28]
    a12c:	885ffe61 	ldaxr	w1, [x19]
    a130:	6b00003f 	cmp	w1, w0
    a134:	54000061 	b.ne	a140 <pthread_mutex_timedlock+0x4f4>
    a138:	88027e7b 	stxr	w2, w27, [x19]
    a13c:	35ffff82 	cbnz	w2, a12c <pthread_mutex_timedlock+0x4e0>
    a140:	54000040 	b.eq	a148 <pthread_mutex_timedlock+0x4fc>
    a144:	b9000381 	str	w1, [x28]
    a148:	b94083a1 	ldr	w1, [x29,#128]
    a14c:	120d3020 	and	w0, w1, #0xfff80000
    a150:	6b00029f 	cmp	w20, w0
    a154:	540011e1 	b.ne	a390 <pthread_mutex_timedlock+0x744>
    a158:	6b01029f 	cmp	w20, w1
    a15c:	540003c0 	b.eq	a1d4 <pthread_mutex_timedlock+0x588>
    a160:	f94006a1 	ldr	x1, [x21,#8]
    a164:	d2993fe0 	mov	x0, #0xc9ff                	// #51711
    a168:	f2a77340 	movk	x0, #0x3b9a, lsl #16
    a16c:	eb00003f 	cmp	x1, x0
    a170:	54001148 	b.hi	a398 <pthread_mutex_timedlock+0x74c>
    a174:	d2800001 	mov	x1, #0x0                   	// #0
    a178:	aa1803e0 	mov	x0, x24
    a17c:	97ffeb91 	bl	4fc0 <__gettimeofday@plt>
    a180:	f9403fa7 	ldr	x7, [x29,#120]
    a184:	f94002a6 	ldr	x6, [x21]
    a188:	f94006a0 	ldr	x0, [x21,#8]
    a18c:	cb0714e5 	sub	x5, x7, x7, lsl #5
    a190:	f9403ba1 	ldr	x1, [x29,#112]
    a194:	d37ef4a5 	lsl	x5, x5, #2
    a198:	cb0700a4 	sub	x4, x5, x7
    a19c:	cb0100c6 	sub	x6, x6, x1
    a1a0:	f90043a6 	str	x6, [x29,#128]
    a1a4:	ab040c04 	adds	x4, x0, x4, lsl #3
    a1a8:	54000344 	b.mi	a210 <pthread_mutex_timedlock+0x5c4>
    a1ac:	f90047a4 	str	x4, [x29,#136]
    a1b0:	b7f81006 	tbnz	x6, #63, a3b0 <pthread_mutex_timedlock+0x764>
    a1b4:	b9401261 	ldr	w1, [x19,#16]
    a1b8:	aa1303e0 	mov	x0, x19
    a1bc:	aa1903e2 	mov	x2, x25
    a1c0:	aa1a03e3 	mov	x3, x26
    a1c4:	2a2103e1 	mvn	w1, w1
    a1c8:	d2800c48 	mov	x8, #0x62                  	// #98
    a1cc:	92790021 	and	x1, x1, #0x80
    a1d0:	d4000001 	svc	#0x0
    a1d4:	b90083b4 	str	w20, [x29,#128]
    a1d8:	b9400380 	ldr	w0, [x28]
    a1dc:	885ffe61 	ldaxr	w1, [x19]
    a1e0:	6b00003f 	cmp	w1, w0
    a1e4:	54000061 	b.ne	a1f0 <pthread_mutex_timedlock+0x5a4>
    a1e8:	88027e7b 	stxr	w2, w27, [x19]
    a1ec:	35ffff82 	cbnz	w2, a1dc <pthread_mutex_timedlock+0x590>
    a1f0:	54000040 	b.eq	a1f8 <pthread_mutex_timedlock+0x5ac>
    a1f4:	b9000381 	str	w1, [x28]
    a1f8:	b94083a0 	ldr	w0, [x29,#128]
    a1fc:	6b00029f 	cmp	w20, w0
    a200:	54fff921 	b.ne	a124 <pthread_mutex_timedlock+0x4d8>
    a204:	52800020 	mov	w0, #0x1                   	// #1
    a208:	b9000660 	str	w0, [x19,#4]
    a20c:	17ffff2a 	b	9eb4 <pthread_mutex_timedlock+0x268>
    a210:	d2994000 	mov	x0, #0xca00                	// #51712
    a214:	d10004c6 	sub	x6, x6, #0x1
    a218:	f2a77340 	movk	x0, #0x3b9a, lsl #16
    a21c:	f90043a6 	str	x6, [x29,#128]
    a220:	8b000084 	add	x4, x4, x0
    a224:	17ffffe2 	b	a1ac <pthread_mutex_timedlock+0x560>
    a228:	52800000 	mov	w0, #0x0                   	// #0
    a22c:	a94153f3 	ldp	x19, x20, [sp,#16]
    a230:	a9425bf5 	ldp	x21, x22, [sp,#32]
    a234:	a94363f7 	ldp	x23, x24, [sp,#48]
    a238:	a9446bf9 	ldp	x25, x26, [sp,#64]
    a23c:	a94573fb 	ldp	x27, x28, [sp,#80]
    a240:	fd4033e8 	ldr	d8, [sp,#96]
    a244:	a8c97bfd 	ldp	x29, x30, [sp],#144
    a248:	d65f03c0 	ret
    a24c:	7104045f 	cmp	w2, #0x101
    a250:	54ffed80 	b.eq	a000 <pthread_mutex_timedlock+0x3b4>
    a254:	528002c0 	mov	w0, #0x16                  	// #22
    a258:	a94153f3 	ldp	x19, x20, [sp,#16]
    a25c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    a260:	a94363f7 	ldp	x23, x24, [sp,#48]
    a264:	a9446bf9 	ldp	x25, x26, [sp,#64]
    a268:	a94573fb 	ldp	x27, x28, [sp,#80]
    a26c:	fd4033e8 	ldr	d8, [sp,#96]
    a270:	a8c97bfd 	ldp	x29, x30, [sp],#144
    a274:	d65f03c0 	ret
    a278:	b900067f 	str	wzr, [x19,#4]
    a27c:	52800001 	mov	w1, #0x0                   	// #0
    a280:	885f7e60 	ldxr	w0, [x19]
    a284:	8802fe61 	stlxr	w2, w1, [x19]
    a288:	35ffffc2 	cbnz	w2, a280 <pthread_mutex_timedlock+0x634>
    a28c:	7100041f 	cmp	w0, #0x1
    a290:	54000e4c 	b.gt	a458 <pthread_mutex_timedlock+0x80c>
    a294:	f9007a9f 	str	xzr, [x20,#240]
    a298:	52801060 	mov	w0, #0x83                  	// #131
    a29c:	17ffff0b 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a2a0:	34fffb24 	cbz	w4, a204 <pthread_mutex_timedlock+0x5b8>
    a2a4:	b9400a61 	ldr	w1, [x19,#8]
    a2a8:	321f77e0 	mov	w0, #0x7ffffffe            	// #2147483646
    a2ac:	6b00003f 	cmp	w1, w0
    a2b0:	54001140 	b.eq	a4d8 <pthread_mutex_timedlock+0x88c>
    a2b4:	aa1403e0 	mov	x0, x20
    a2b8:	52800021 	mov	w1, #0x1                   	// #1
    a2bc:	b9000661 	str	w1, [x19,#4]
    a2c0:	91008262 	add	x2, x19, #0x20
    a2c4:	b2400043 	orr	x3, x2, #0x1
    a2c8:	f84e0c01 	ldr	x1, [x0,#224]!
    a2cc:	927ff824 	and	x4, x1, #0xfffffffffffffffe
    a2d0:	f81f8082 	str	x2, [x4,#-8]
    a2d4:	f9001261 	str	x1, [x19,#32]
    a2d8:	f9000e60 	str	x0, [x19,#24]
    a2dc:	f9007283 	str	x3, [x20,#224]
    a2e0:	f9007a9f 	str	xzr, [x20,#240]
    a2e4:	17fffef4 	b	9eb4 <pthread_mutex_timedlock+0x268>
    a2e8:	b90083bf 	str	wzr, [x29,#128]
    a2ec:	52800020 	mov	w0, #0x1                   	// #1
    a2f0:	885ffe63 	ldaxr	w3, [x19]
    a2f4:	6b1f007f 	cmp	w3, wzr
    a2f8:	54000061 	b.ne	a304 <pthread_mutex_timedlock+0x6b8>
    a2fc:	88017e60 	stxr	w1, w0, [x19]
    a300:	35ffff81 	cbnz	w1, a2f0 <pthread_mutex_timedlock+0x6a4>
    a304:	54fff800 	b.eq	a204 <pthread_mutex_timedlock+0x5b8>
    a308:	b9401262 	ldr	w2, [x19,#16]
    a30c:	aa1303e0 	mov	x0, x19
    a310:	aa1503e1 	mov	x1, x21
    a314:	b90083a3 	str	w3, [x29,#128]
    a318:	12190042 	and	w2, w2, #0x80
    a31c:	94001545 	bl	f830 <__lll_timedlock_wait>
    a320:	35ffdd40 	cbnz	w0, 9ec8 <pthread_mutex_timedlock+0x27c>
    a324:	17ffffb8 	b	a204 <pthread_mutex_timedlock+0x5b8>
    a328:	b9400260 	ldr	w0, [x19]
    a32c:	36f7fba0 	tbz	w0, #30, a2a0 <pthread_mutex_timedlock+0x654>
    a330:	910203bc 	add	x28, x29, #0x80
    a334:	b90083a0 	str	w0, [x29,#128]
    a338:	12017800 	and	w0, w0, #0xbfffffff
    a33c:	b9400381 	ldr	w1, [x28]
    a340:	885ffe62 	ldaxr	w2, [x19]
    a344:	6b01005f 	cmp	w2, w1
    a348:	54000061 	b.ne	a354 <pthread_mutex_timedlock+0x708>
    a34c:	88037e60 	stxr	w3, w0, [x19]
    a350:	35ffff83 	cbnz	w3, a340 <pthread_mutex_timedlock+0x6f4>
    a354:	54000620 	b.eq	a418 <pthread_mutex_timedlock+0x7cc>
    a358:	b9000382 	str	w2, [x28]
    a35c:	b9400260 	ldr	w0, [x19]
    a360:	17fffff5 	b	a334 <pthread_mutex_timedlock+0x6e8>
    a364:	12000463 	and	w3, w3, #0x3
    a368:	7100087f 	cmp	w3, #0x2
    a36c:	54000840 	b.eq	a474 <pthread_mutex_timedlock+0x828>
    a370:	7100047f 	cmp	w3, #0x1
    a374:	54ffd261 	b.ne	9dc0 <pthread_mutex_timedlock+0x174>
    a378:	f9007a9f 	str	xzr, [x20,#240]
    a37c:	b9400660 	ldr	w0, [x19,#4]
    a380:	3100041f 	cmn	w0, #0x1
    a384:	54ffe4a1 	b.ne	a018 <pthread_mutex_timedlock+0x3cc>
    a388:	52800160 	mov	w0, #0xb                   	// #11
    a38c:	17fffecf 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a390:	1e260114 	fmov	w20, s8
    a394:	17ffff48 	b	a0b4 <pthread_mutex_timedlock+0x468>
    a398:	528002d3 	mov	w19, #0x16                  	// #22
    a39c:	1e260100 	fmov	w0, s8
    a3a0:	12800001 	mov	w1, #0xffffffff            	// #-1
    a3a4:	94001e2d 	bl	11c58 <__pthread_tpp_change_priority>
    a3a8:	2a1303e0 	mov	w0, w19
    a3ac:	17fffec7 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a3b0:	52800dd3 	mov	w19, #0x6e                  	// #110
    a3b4:	17fffffa 	b	a39c <pthread_mutex_timedlock+0x750>
    a3b8:	52800020 	mov	w0, #0x1                   	// #1
    a3bc:	885ffe63 	ldaxr	w3, [x19]
    a3c0:	6b1f007f 	cmp	w3, wzr
    a3c4:	54000061 	b.ne	a3d0 <pthread_mutex_timedlock+0x784>
    a3c8:	88017e60 	stxr	w1, w0, [x19]
    a3cc:	35ffff81 	cbnz	w1, a3bc <pthread_mutex_timedlock+0x770>
    a3d0:	54ffe020 	b.eq	9fd4 <pthread_mutex_timedlock+0x388>
    a3d4:	b9401262 	ldr	w2, [x19,#16]
    a3d8:	aa1303e0 	mov	x0, x19
    a3dc:	aa1503e1 	mov	x1, x21
    a3e0:	b90083a3 	str	w3, [x29,#128]
    a3e4:	12190042 	and	w2, w2, #0x80
    a3e8:	94001512 	bl	f830 <__lll_timedlock_wait>
    a3ec:	2a0003e2 	mov	w2, w0
    a3f0:	17fffefa 	b	9fd8 <pthread_mutex_timedlock+0x38c>
    a3f4:	12000463 	and	w3, w3, #0x3
    a3f8:	52800460 	mov	w0, #0x23                  	// #35
    a3fc:	7100087f 	cmp	w3, #0x2
    a400:	54ffd640 	b.eq	9ec8 <pthread_mutex_timedlock+0x27c>
    a404:	7100047f 	cmp	w3, #0x1
    a408:	54ffe521 	b.ne	a0ac <pthread_mutex_timedlock+0x460>
    a40c:	17ffff00 	b	a00c <pthread_mutex_timedlock+0x3c0>
    a410:	2a0203e0 	mov	w0, w2
    a414:	17fffead 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a418:	aa1403e1 	mov	x1, x20
    a41c:	52800020 	mov	w0, #0x1                   	// #1
    a420:	b9000660 	str	w0, [x19,#4]
    a424:	12b00000 	mov	w0, #0x7fffffff            	// #2147483647
    a428:	b9000a60 	str	w0, [x19,#8]
    a42c:	91008263 	add	x3, x19, #0x20
    a430:	b2400064 	orr	x4, x3, #0x1
    a434:	52801040 	mov	w0, #0x82                  	// #130
    a438:	f84e0c22 	ldr	x2, [x1,#224]!
    a43c:	927ff845 	and	x5, x2, #0xfffffffffffffffe
    a440:	f81f80a3 	str	x3, [x5,#-8]
    a444:	f9001262 	str	x2, [x19,#32]
    a448:	f9000e61 	str	x1, [x19,#24]
    a44c:	f9007284 	str	x4, [x20,#224]
    a450:	f9007a9f 	str	xzr, [x20,#240]
    a454:	17fffe9d 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a458:	d2800021 	mov	x1, #0x1                   	// #1
    a45c:	aa1303e0 	mov	x0, x19
    a460:	aa0103e2 	mov	x2, x1
    a464:	d2800003 	mov	x3, #0x0                   	// #0
    a468:	d2800c48 	mov	x8, #0x62                  	// #98
    a46c:	d4000001 	svc	#0x0
    a470:	17ffff89 	b	a294 <pthread_mutex_timedlock+0x648>
    a474:	f9007a9f 	str	xzr, [x20,#240]
    a478:	52800460 	mov	w0, #0x23                  	// #35
    a47c:	17fffe93 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a480:	aa1403e1 	mov	x1, x20
    a484:	52800020 	mov	w0, #0x1                   	// #1
    a488:	b9000660 	str	w0, [x19,#4]
    a48c:	12b00000 	mov	w0, #0x7fffffff            	// #2147483647
    a490:	b9000a60 	str	w0, [x19,#8]
    a494:	52801040 	mov	w0, #0x82                  	// #130
    a498:	f84e0c22 	ldr	x2, [x1,#224]!
    a49c:	927ff843 	and	x3, x2, #0xfffffffffffffffe
    a4a0:	f81f8078 	str	x24, [x3,#-8]
    a4a4:	f9001262 	str	x2, [x19,#32]
    a4a8:	f9000e61 	str	x1, [x19,#24]
    a4ac:	f9007298 	str	x24, [x20,#224]
    a4b0:	f9007a9f 	str	xzr, [x20,#240]
    a4b4:	17fffe85 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a4b8:	4b0103e0 	neg	w0, w1
    a4bc:	17fffe83 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a4c0:	3100069f 	cmn	w20, #0x1
    a4c4:	528002c0 	mov	w0, #0x16                  	// #22
    a4c8:	54ffd000 	b.eq	9ec8 <pthread_mutex_timedlock+0x27c>
    a4cc:	1e270288 	fmov	s8, w20
    a4d0:	2a0003f3 	mov	w19, w0
    a4d4:	17ffffb2 	b	a39c <pthread_mutex_timedlock+0x750>
    a4d8:	d2800002 	mov	x2, #0x0                   	// #0
    a4dc:	b900067f 	str	wzr, [x19,#4]
    a4e0:	aa1303e0 	mov	x0, x19
    a4e4:	d28000e1 	mov	x1, #0x7                   	// #7
    a4e8:	aa0203e3 	mov	x3, x2
    a4ec:	d2800c48 	mov	x8, #0x62                  	// #98
    a4f0:	d4000001 	svc	#0x0
    a4f4:	52801060 	mov	w0, #0x83                  	// #131
    a4f8:	f9007a82 	str	x2, [x20,#240]
    a4fc:	17fffe73 	b	9ec8 <pthread_mutex_timedlock+0x27c>
    a500:	d2994002 	mov	x2, #0xca00                	// #51712
    a504:	d1000400 	sub	x0, x0, #0x1
    a508:	f2a77342 	movk	x2, #0x3b9a, lsl #16
    a50c:	f9003ba0 	str	x0, [x29,#112]
    a510:	8b020021 	add	x1, x1, x2
    a514:	17fffe56 	b	9e6c <pthread_mutex_timedlock+0x220>

000000000000a518 <__pthread_mutex_unlock_full>:
    a518:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    a51c:	910003fd 	mov	x29, sp
    a520:	b9401005 	ldr	w5, [x0,#16]
    a524:	120018a2 	and	w2, w5, #0x7f
    a528:	51004042 	sub	w2, w2, #0x10
    a52c:	7100cc5f 	cmp	w2, #0x33
    a530:	540000a9 	b.ls	a544 <__pthread_mutex_unlock_full+0x2c>
    a534:	528002c4 	mov	w4, #0x16                  	// #22
    a538:	2a0403e0 	mov	w0, w4
    a53c:	a8c27bfd 	ldp	x29, x30, [sp],#32
    a540:	d65f03c0 	ret
    a544:	90000043 	adrp	x3, 12000 <__pthread_current_priority+0xa8>
    a548:	9130a063 	add	x3, x3, #0xc28
    a54c:	78625862 	ldrh	w2, [x3,w2,uxtw #1]
    a550:	10000063 	adr	x3, a55c <__pthread_mutex_unlock_full+0x44>
    a554:	8b22a862 	add	x2, x3, w2, sxth #2
    a558:	d61f0040 	br	x2
    a55c:	d53bd045 	mrs	x5, tpidr_el0
    a560:	b9400002 	ldr	w2, [x0]
    a564:	d11bc0a5 	sub	x5, x5, #0x6f0
    a568:	12007444 	and	w4, w2, #0x3fffffff
    a56c:	b940d0a3 	ldr	w3, [x5,#208]
    a570:	6b03009f 	cmp	w4, w3
    a574:	54001761 	b.ne	a860 <__pthread_mutex_unlock_full+0x348>
    a578:	34001742 	cbz	w2, a860 <__pthread_mutex_unlock_full+0x348>
    a57c:	b9400803 	ldr	w3, [x0,#8]
    a580:	12b00002 	mov	w2, #0x7fffffff            	// #2147483647
    a584:	6b02007f 	cmp	w3, w2
    a588:	321f77e2 	mov	w2, #0x7ffffffe            	// #2147483646
    a58c:	1a8213e2 	csel	w2, wzr, w2, ne
    a590:	14000095 	b	a7e4 <__pthread_mutex_unlock_full+0x2cc>
    a594:	d53bd042 	mrs	x2, tpidr_el0
    a598:	b9400803 	ldr	w3, [x0,#8]
    a59c:	d11bc042 	sub	x2, x2, #0x6f0
    a5a0:	52800024 	mov	w4, #0x1                   	// #1
    a5a4:	b940d042 	ldr	w2, [x2,#208]
    a5a8:	6b02007f 	cmp	w3, w2
    a5ac:	54fffc61 	b.ne	a538 <__pthread_mutex_unlock_full+0x20>
    a5b0:	b9400002 	ldr	w2, [x0]
    a5b4:	12004843 	and	w3, w2, #0x7ffff
    a5b8:	34fffc03 	cbz	w3, a538 <__pthread_mutex_unlock_full+0x20>
    a5bc:	b900081f 	str	wzr, [x0,#8]
    a5c0:	35000581 	cbnz	w1, a670 <__pthread_mutex_unlock_full+0x158>
    a5c4:	2a0203e1 	mov	w1, w2
    a5c8:	b9001fa1 	str	w1, [x29,#28]
    a5cc:	120d3024 	and	w4, w1, #0xfff80000
    a5d0:	2a0103e2 	mov	w2, w1
    a5d4:	885f7c03 	ldxr	w3, [x0]
    a5d8:	6b02007f 	cmp	w3, w2
    a5dc:	54000061 	b.ne	a5e8 <__pthread_mutex_unlock_full+0xd0>
    a5e0:	8805fc04 	stlxr	w5, w4, [x0]
    a5e4:	35ffff85 	cbnz	w5, a5d4 <__pthread_mutex_unlock_full+0xbc>
    a5e8:	540004c1 	b.ne	a680 <__pthread_mutex_unlock_full+0x168>
    a5ec:	12004821 	and	w1, w1, #0x7ffff
    a5f0:	7100043f 	cmp	w1, #0x1
    a5f4:	54000149 	b.ls	a61c <__pthread_mutex_unlock_full+0x104>
    a5f8:	b9401001 	ldr	w1, [x0,#16]
    a5fc:	d2800022 	mov	x2, #0x1                   	// #1
    a600:	d2800003 	mov	x3, #0x0                   	// #0
    a604:	d2800c48 	mov	x8, #0x62                  	// #98
    a608:	12190025 	and	w5, w1, #0x80
    a60c:	52801021 	mov	w1, #0x81                  	// #129
    a610:	4a0100a1 	eor	w1, w5, w1
    a614:	93407c21 	sxtw	x1, w1
    a618:	d4000001 	svc	#0x0
    a61c:	13137c80 	asr	w0, w4, #19
    a620:	12800001 	mov	w1, #0xffffffff            	// #-1
    a624:	94001d8d 	bl	11c58 <__pthread_tpp_change_priority>
    a628:	2a0003e4 	mov	w4, w0
    a62c:	2a0403e0 	mov	w0, w4
    a630:	a8c27bfd 	ldp	x29, x30, [sp],#32
    a634:	d65f03c0 	ret
    a638:	d53bd042 	mrs	x2, tpidr_el0
    a63c:	b9400803 	ldr	w3, [x0,#8]
    a640:	d11bc042 	sub	x2, x2, #0x6f0
    a644:	52800024 	mov	w4, #0x1                   	// #1
    a648:	b940d042 	ldr	w2, [x2,#208]
    a64c:	6b02007f 	cmp	w3, w2
    a650:	54fff741 	b.ne	a538 <__pthread_mutex_unlock_full+0x20>
    a654:	b9400402 	ldr	w2, [x0,#4]
    a658:	51000442 	sub	w2, w2, #0x1
    a65c:	b9000402 	str	w2, [x0,#4]
    a660:	35000f82 	cbnz	w2, a850 <__pthread_mutex_unlock_full+0x338>
    a664:	b900081f 	str	wzr, [x0,#8]
    a668:	b9400002 	ldr	w2, [x0]
    a66c:	34fffac1 	cbz	w1, a5c4 <__pthread_mutex_unlock_full+0xac>
    a670:	b9400c01 	ldr	w1, [x0,#12]
    a674:	51000421 	sub	w1, w1, #0x1
    a678:	b9000c01 	str	w1, [x0,#12]
    a67c:	17ffffd2 	b	a5c4 <__pthread_mutex_unlock_full+0xac>
    a680:	b9400001 	ldr	w1, [x0]
    a684:	17ffffd1 	b	a5c8 <__pthread_mutex_unlock_full+0xb0>
    a688:	d53bd043 	mrs	x3, tpidr_el0
    a68c:	b9400004 	ldr	w4, [x0]
    a690:	d11bc063 	sub	x3, x3, #0x6f0
    a694:	12007482 	and	w2, w4, #0x3fffffff
    a698:	b940d066 	ldr	w6, [x3,#208]
    a69c:	6b06005f 	cmp	w2, w6
    a6a0:	b9400802 	ldr	w2, [x0,#8]
    a6a4:	54001060 	b.eq	a8b0 <__pthread_mutex_unlock_full+0x398>
    a6a8:	6b0200df 	cmp	w6, w2
    a6ac:	52800024 	mov	w4, #0x1                   	// #1
    a6b0:	54fff441 	b.ne	a538 <__pthread_mutex_unlock_full+0x20>
    a6b4:	b9400402 	ldr	w2, [x0,#4]
    a6b8:	51000442 	sub	w2, w2, #0x1
    a6bc:	b9000402 	str	w2, [x0,#4]
    a6c0:	340010a2 	cbz	w2, a8d4 <__pthread_mutex_unlock_full+0x3bc>
    a6c4:	14000063 	b	a850 <__pthread_mutex_unlock_full+0x338>
    a6c8:	d53bd043 	mrs	x3, tpidr_el0
    a6cc:	b9400805 	ldr	w5, [x0,#8]
    a6d0:	d11bc063 	sub	x3, x3, #0x6f0
    a6d4:	52800024 	mov	w4, #0x1                   	// #1
    a6d8:	b940d062 	ldr	w2, [x3,#208]
    a6dc:	6b0200bf 	cmp	w5, w2
    a6e0:	54fff2c1 	b.ne	a538 <__pthread_mutex_unlock_full+0x20>
    a6e4:	b9400402 	ldr	w2, [x0,#4]
    a6e8:	51000442 	sub	w2, w2, #0x1
    a6ec:	b9000402 	str	w2, [x0,#4]
    a6f0:	35000b02 	cbnz	w2, a850 <__pthread_mutex_unlock_full+0x338>
    a6f4:	b9400004 	ldr	w4, [x0]
    a6f8:	b9000802 	str	w2, [x0,#8]
    a6fc:	34000081 	cbz	w1, a70c <__pthread_mutex_unlock_full+0x1f4>
    a700:	b9400c01 	ldr	w1, [x0,#12]
    a704:	51000421 	sub	w1, w1, #0x1
    a708:	b9000c01 	str	w1, [x0,#12]
    a70c:	37f80164 	tbnz	w4, #31, a738 <__pthread_mutex_unlock_full+0x220>
    a710:	b940d061 	ldr	w1, [x3,#208]
    a714:	52800004 	mov	w4, #0x0                   	// #0
    a718:	b9001fa1 	str	w1, [x29,#28]
    a71c:	885f7c02 	ldxr	w2, [x0]
    a720:	6b01005f 	cmp	w2, w1
    a724:	54000061 	b.ne	a730 <__pthread_mutex_unlock_full+0x218>
    a728:	8805fc04 	stlxr	w5, w4, [x0]
    a72c:	35ffff85 	cbnz	w5, a71c <__pthread_mutex_unlock_full+0x204>
    a730:	540000e0 	b.eq	a74c <__pthread_mutex_unlock_full+0x234>
    a734:	b9001fa2 	str	w2, [x29,#28]
    a738:	b9401001 	ldr	w1, [x0,#16]
    a73c:	36200b01 	tbz	w1, #4, a89c <__pthread_mutex_unlock_full+0x384>
    a740:	d28000e1 	mov	x1, #0x7                   	// #7
    a744:	d2800c48 	mov	x8, #0x62                  	// #98
    a748:	d4000001 	svc	#0x0
    a74c:	f900787f 	str	xzr, [x3,#240]
    a750:	52800004 	mov	w4, #0x0                   	// #0
    a754:	2a0403e0 	mov	w0, w4
    a758:	a8c27bfd 	ldp	x29, x30, [sp],#32
    a75c:	d65f03c0 	ret
    a760:	d53bd043 	mrs	x3, tpidr_el0
    a764:	b9400004 	ldr	w4, [x0]
    a768:	d11bc063 	sub	x3, x3, #0x6f0
    a76c:	12007486 	and	w6, w4, #0x3fffffff
    a770:	b940d062 	ldr	w2, [x3,#208]
    a774:	6b0200df 	cmp	w6, w2
    a778:	54000741 	b.ne	a860 <__pthread_mutex_unlock_full+0x348>
    a77c:	6b1f009f 	cmp	w4, wzr
    a780:	1a9f17e2 	cset	w2, eq
    a784:	350006e2 	cbnz	w2, a860 <__pthread_mutex_unlock_full+0x348>
    a788:	3627fb85 	tbz	w5, #4, a6f8 <__pthread_mutex_unlock_full+0x1e0>
    a78c:	b9400804 	ldr	w4, [x0,#8]
    a790:	12b00002 	mov	w2, #0x7fffffff            	// #2147483647
    a794:	6b02009f 	cmp	w4, w2
    a798:	321f77e2 	mov	w2, #0x7ffffffe            	// #2147483646
    a79c:	1a8213e2 	csel	w2, wzr, w2, ne
    a7a0:	1400004d 	b	a8d4 <__pthread_mutex_unlock_full+0x3bc>
    a7a4:	d53bd045 	mrs	x5, tpidr_el0
    a7a8:	b9400002 	ldr	w2, [x0]
    a7ac:	d11bc0a5 	sub	x5, x5, #0x6f0
    a7b0:	12007442 	and	w2, w2, #0x3fffffff
    a7b4:	b940d0a3 	ldr	w3, [x5,#208]
    a7b8:	6b03005f 	cmp	w2, w3
    a7bc:	b9400802 	ldr	w2, [x0,#8]
    a7c0:	54000580 	b.eq	a870 <__pthread_mutex_unlock_full+0x358>
    a7c4:	6b02007f 	cmp	w3, w2
    a7c8:	52800024 	mov	w4, #0x1                   	// #1
    a7cc:	54ffeb61 	b.ne	a538 <__pthread_mutex_unlock_full+0x20>
    a7d0:	b9400403 	ldr	w3, [x0,#4]
    a7d4:	52800002 	mov	w2, #0x0                   	// #0
    a7d8:	51000463 	sub	w3, w3, #0x1
    a7dc:	b9000403 	str	w3, [x0,#4]
    a7e0:	35000383 	cbnz	w3, a850 <__pthread_mutex_unlock_full+0x338>
    a7e4:	91008003 	add	x3, x0, #0x20
    a7e8:	f90078a3 	str	x3, [x5,#240]
    a7ec:	f9401004 	ldr	x4, [x0,#32]
    a7f0:	f9400c06 	ldr	x6, [x0,#24]
    a7f4:	927ff883 	and	x3, x4, #0xfffffffffffffffe
    a7f8:	f81f8066 	str	x6, [x3,#-8]
    a7fc:	f9400c03 	ldr	x3, [x0,#24]
    a800:	927ff863 	and	x3, x3, #0xfffffffffffffffe
    a804:	f9000064 	str	x4, [x3]
    a808:	f9000c1f 	str	xzr, [x0,#24]
    a80c:	f900101f 	str	xzr, [x0,#32]
    a810:	b9000802 	str	w2, [x0,#8]
    a814:	34000081 	cbz	w1, a824 <__pthread_mutex_unlock_full+0x30c>
    a818:	b9400c01 	ldr	w1, [x0,#12]
    a81c:	51000421 	sub	w1, w1, #0x1
    a820:	b9000c01 	str	w1, [x0,#12]
    a824:	52800002 	mov	w2, #0x0                   	// #0
    a828:	885f7c01 	ldxr	w1, [x0]
    a82c:	8803fc02 	stlxr	w3, w2, [x0]
    a830:	35ffffc3 	cbnz	w3, a828 <__pthread_mutex_unlock_full+0x310>
    a834:	36f800c1 	tbz	w1, #31, a84c <__pthread_mutex_unlock_full+0x334>
    a838:	d2800021 	mov	x1, #0x1                   	// #1
    a83c:	d2800003 	mov	x3, #0x0                   	// #0
    a840:	aa0103e2 	mov	x2, x1
    a844:	d2800c48 	mov	x8, #0x62                  	// #98
    a848:	d4000001 	svc	#0x0
    a84c:	f90078bf 	str	xzr, [x5,#240]
    a850:	52800004 	mov	w4, #0x0                   	// #0
    a854:	2a0403e0 	mov	w0, w4
    a858:	a8c27bfd 	ldp	x29, x30, [sp],#32
    a85c:	d65f03c0 	ret
    a860:	52800024 	mov	w4, #0x1                   	// #1
    a864:	2a0403e0 	mov	w0, w4
    a868:	a8c27bfd 	ldp	x29, x30, [sp],#32
    a86c:	d65f03c0 	ret
    a870:	12b00004 	mov	w4, #0x7fffffff            	// #2147483647
    a874:	6b04005f 	cmp	w2, w4
    a878:	54fffa61 	b.ne	a7c4 <__pthread_mutex_unlock_full+0x2ac>
    a87c:	b9400402 	ldr	w2, [x0,#4]
    a880:	51000442 	sub	w2, w2, #0x1
    a884:	b9000402 	str	w2, [x0,#4]
    a888:	34000422 	cbz	w2, a90c <__pthread_mutex_unlock_full+0x3f4>
    a88c:	52801064 	mov	w4, #0x83                  	// #131
    a890:	2a0403e0 	mov	w0, w4
    a894:	a8c27bfd 	ldp	x29, x30, [sp],#32
    a898:	d65f03c0 	ret
    a89c:	12190022 	and	w2, w1, #0x80
    a8a0:	528010e1 	mov	w1, #0x87                  	// #135
    a8a4:	4a010041 	eor	w1, w2, w1
    a8a8:	93407c21 	sxtw	x1, w1
    a8ac:	17ffffa6 	b	a744 <__pthread_mutex_unlock_full+0x22c>
    a8b0:	12b00007 	mov	w7, #0x7fffffff            	// #2147483647
    a8b4:	6b07005f 	cmp	w2, w7
    a8b8:	54ffef81 	b.ne	a6a8 <__pthread_mutex_unlock_full+0x190>
    a8bc:	b9400402 	ldr	w2, [x0,#4]
    a8c0:	51000442 	sub	w2, w2, #0x1
    a8c4:	b9000402 	str	w2, [x0,#4]
    a8c8:	35fffe22 	cbnz	w2, a88c <__pthread_mutex_unlock_full+0x374>
    a8cc:	321f77e2 	mov	w2, #0x7ffffffe            	// #2147483646
    a8d0:	3627f145 	tbz	w5, #4, a6f8 <__pthread_mutex_unlock_full+0x1e0>
    a8d4:	91008004 	add	x4, x0, #0x20
    a8d8:	b2400084 	orr	x4, x4, #0x1
    a8dc:	f9007864 	str	x4, [x3,#240]
    a8e0:	f9401005 	ldr	x5, [x0,#32]
    a8e4:	f9400c06 	ldr	x6, [x0,#24]
    a8e8:	927ff8a4 	and	x4, x5, #0xfffffffffffffffe
    a8ec:	f81f8086 	str	x6, [x4,#-8]
    a8f0:	f9400c04 	ldr	x4, [x0,#24]
    a8f4:	927ff884 	and	x4, x4, #0xfffffffffffffffe
    a8f8:	f9000085 	str	x5, [x4]
    a8fc:	f9000c1f 	str	xzr, [x0,#24]
    a900:	b9400004 	ldr	w4, [x0]
    a904:	f900101f 	str	xzr, [x0,#32]
    a908:	17ffff7c 	b	a6f8 <__pthread_mutex_unlock_full+0x1e0>
    a90c:	321f77e2 	mov	w2, #0x7ffffffe            	// #2147483646
    a910:	17ffffb5 	b	a7e4 <__pthread_mutex_unlock_full+0x2cc>

000000000000a914 <__pthread_mutex_unlock_usercnt>:
    a914:	b9401003 	ldr	w3, [x0,#16]
    a918:	52802fe4 	mov	w4, #0x17f                 	// #383
    a91c:	aa0003e2 	mov	x2, x0
    a920:	2a0103e6 	mov	w6, w1
    a924:	721e1065 	ands	w5, w3, #0x7c
    a928:	0a040064 	and	w4, w3, w4
    a92c:	54000201 	b.ne	a96c <__pthread_mutex_unlock_usercnt+0x58>
    a930:	35000204 	cbnz	w4, a970 <__pthread_mutex_unlock_usercnt+0x5c>
    a934:	b900085f 	str	wzr, [x2,#8]
    a938:	35000126 	cbnz	w6, a95c <__pthread_mutex_unlock_usercnt+0x48>
    a93c:	52800001 	mov	w1, #0x0                   	// #0
    a940:	885f7c40 	ldxr	w0, [x2]
    a944:	8803fc41 	stlxr	w3, w1, [x2]
    a948:	35ffffc3 	cbnz	w3, a940 <__pthread_mutex_unlock_usercnt+0x2c>
    a94c:	7100041f 	cmp	w0, #0x1
    a950:	5400032c 	b.gt	a9b4 <__pthread_mutex_unlock_usercnt+0xa0>
    a954:	2a0503e0 	mov	w0, w5
    a958:	d65f03c0 	ret
    a95c:	b9400c40 	ldr	w0, [x2,#12]
    a960:	51000400 	sub	w0, w0, #0x1
    a964:	b9000c40 	str	w0, [x2,#12]
    a968:	17fffff5 	b	a93c <__pthread_mutex_unlock_usercnt+0x28>
    a96c:	17fffeeb 	b	a518 <__pthread_mutex_unlock_full>
    a970:	7104009f 	cmp	w4, #0x100
    a974:	54000361 	b.ne	a9e0 <__pthread_mutex_unlock_usercnt+0xcc>
    a978:	885f7c41 	ldxr	w1, [x2]
    a97c:	8803fc45 	stlxr	w3, w5, [x2]
    a980:	35ffffc3 	cbnz	w3, a978 <__pthread_mutex_unlock_usercnt+0x64>
    a984:	7100043f 	cmp	w1, #0x1
    a988:	54fffe6d 	b.le	a954 <__pthread_mutex_unlock_usercnt+0x40>
    a98c:	b9401041 	ldr	w1, [x2,#16]
    a990:	d2800003 	mov	x3, #0x0                   	// #0
    a994:	d2800022 	mov	x2, #0x1                   	// #1
    a998:	d2800c48 	mov	x8, #0x62                  	// #98
    a99c:	12190024 	and	w4, w1, #0x80
    a9a0:	52801021 	mov	w1, #0x81                  	// #129
    a9a4:	4a010081 	eor	w1, w4, w1
    a9a8:	93407c21 	sxtw	x1, w1
    a9ac:	d4000001 	svc	#0x0
    a9b0:	17ffffe9 	b	a954 <__pthread_mutex_unlock_usercnt+0x40>
    a9b4:	b9401041 	ldr	w1, [x2,#16]
    a9b8:	aa0203e0 	mov	x0, x2
    a9bc:	d2800003 	mov	x3, #0x0                   	// #0
    a9c0:	d2800022 	mov	x2, #0x1                   	// #1
    a9c4:	12190024 	and	w4, w1, #0x80
    a9c8:	52801021 	mov	w1, #0x81                  	// #129
    a9cc:	4a010081 	eor	w1, w4, w1
    a9d0:	d2800c48 	mov	x8, #0x62                  	// #98
    a9d4:	93407c21 	sxtw	x1, w1
    a9d8:	d4000001 	svc	#0x0
    a9dc:	17ffffde 	b	a954 <__pthread_mutex_unlock_usercnt+0x40>
    a9e0:	12001863 	and	w3, w3, #0x7f
    a9e4:	7100047f 	cmp	w3, #0x1
    a9e8:	540001c1 	b.ne	aa20 <__pthread_mutex_unlock_usercnt+0x10c>
    a9ec:	b9400801 	ldr	w1, [x0,#8]
    a9f0:	d53bd040 	mrs	x0, tpidr_el0
    a9f4:	d11bc000 	sub	x0, x0, #0x6f0
    a9f8:	b940d000 	ldr	w0, [x0,#208]
    a9fc:	6b00003f 	cmp	w1, w0
    aa00:	54000060 	b.eq	aa0c <__pthread_mutex_unlock_usercnt+0xf8>
    aa04:	2a0303e5 	mov	w5, w3
    aa08:	17ffffd3 	b	a954 <__pthread_mutex_unlock_usercnt+0x40>
    aa0c:	b9400440 	ldr	w0, [x2,#4]
    aa10:	51000400 	sub	w0, w0, #0x1
    aa14:	b9000440 	str	w0, [x2,#4]
    aa18:	34fff8e0 	cbz	w0, a934 <__pthread_mutex_unlock_usercnt+0x20>
    aa1c:	17ffffce 	b	a954 <__pthread_mutex_unlock_usercnt+0x40>
    aa20:	71000c7f 	cmp	w3, #0x3
    aa24:	54fff880 	b.eq	a934 <__pthread_mutex_unlock_usercnt+0x20>
    aa28:	b9400801 	ldr	w1, [x0,#8]
    aa2c:	d53bd040 	mrs	x0, tpidr_el0
    aa30:	d11bc000 	sub	x0, x0, #0x6f0
    aa34:	b940d000 	ldr	w0, [x0,#208]
    aa38:	6b00003f 	cmp	w1, w0
    aa3c:	54000060 	b.eq	aa48 <__pthread_mutex_unlock_usercnt+0x134>
    aa40:	52800025 	mov	w5, #0x1                   	// #1
    aa44:	17ffffc4 	b	a954 <__pthread_mutex_unlock_usercnt+0x40>
    aa48:	b9400040 	ldr	w0, [x2]
    aa4c:	35fff740 	cbnz	w0, a934 <__pthread_mutex_unlock_usercnt+0x20>
    aa50:	52800025 	mov	w5, #0x1                   	// #1
    aa54:	17ffffc0 	b	a954 <__pthread_mutex_unlock_usercnt+0x40>

000000000000aa58 <__pthread_mutex_unlock>:
    aa58:	b9401001 	ldr	w1, [x0,#16]
    aa5c:	52802fe3 	mov	w3, #0x17f                 	// #383
    aa60:	aa0003e2 	mov	x2, x0
    aa64:	721e1024 	ands	w4, w1, #0x7c
    aa68:	0a030023 	and	w3, w1, w3
    aa6c:	540001c1 	b.ne	aaa4 <__pthread_mutex_unlock+0x4c>
    aa70:	350001e3 	cbnz	w3, aaac <__pthread_mutex_unlock+0x54>
    aa74:	b9400c41 	ldr	w1, [x2,#12]
    aa78:	b900085f 	str	wzr, [x2,#8]
    aa7c:	51000421 	sub	w1, w1, #0x1
    aa80:	b9000c41 	str	w1, [x2,#12]
    aa84:	52800001 	mov	w1, #0x0                   	// #0
    aa88:	885f7c40 	ldxr	w0, [x2]
    aa8c:	8803fc41 	stlxr	w3, w1, [x2]
    aa90:	35ffffc3 	cbnz	w3, aa88 <__pthread_mutex_unlock+0x30>
    aa94:	7100041f 	cmp	w0, #0x1
    aa98:	540002cc 	b.gt	aaf0 <__pthread_mutex_unlock+0x98>
    aa9c:	2a0403e0 	mov	w0, w4
    aaa0:	d65f03c0 	ret
    aaa4:	52800021 	mov	w1, #0x1                   	// #1
    aaa8:	17fffe9c 	b	a518 <__pthread_mutex_unlock_full>
    aaac:	7104007f 	cmp	w3, #0x100
    aab0:	54000361 	b.ne	ab1c <__pthread_mutex_unlock+0xc4>
    aab4:	885f7c41 	ldxr	w1, [x2]
    aab8:	8803fc44 	stlxr	w3, w4, [x2]
    aabc:	35ffffc3 	cbnz	w3, aab4 <__pthread_mutex_unlock+0x5c>
    aac0:	7100043f 	cmp	w1, #0x1
    aac4:	54fffecd 	b.le	aa9c <__pthread_mutex_unlock+0x44>
    aac8:	b9401041 	ldr	w1, [x2,#16]
    aacc:	d2800003 	mov	x3, #0x0                   	// #0
    aad0:	d2800022 	mov	x2, #0x1                   	// #1
    aad4:	d2800c48 	mov	x8, #0x62                  	// #98
    aad8:	12190025 	and	w5, w1, #0x80
    aadc:	52801021 	mov	w1, #0x81                  	// #129
    aae0:	4a0100a1 	eor	w1, w5, w1
    aae4:	93407c21 	sxtw	x1, w1
    aae8:	d4000001 	svc	#0x0
    aaec:	17ffffec 	b	aa9c <__pthread_mutex_unlock+0x44>
    aaf0:	b9401041 	ldr	w1, [x2,#16]
    aaf4:	aa0203e0 	mov	x0, x2
    aaf8:	d2800003 	mov	x3, #0x0                   	// #0
    aafc:	d2800022 	mov	x2, #0x1                   	// #1
    ab00:	12190025 	and	w5, w1, #0x80
    ab04:	52801021 	mov	w1, #0x81                  	// #129
    ab08:	4a0100a1 	eor	w1, w5, w1
    ab0c:	d2800c48 	mov	x8, #0x62                  	// #98
    ab10:	93407c21 	sxtw	x1, w1
    ab14:	d4000001 	svc	#0x0
    ab18:	17ffffe1 	b	aa9c <__pthread_mutex_unlock+0x44>
    ab1c:	12001821 	and	w1, w1, #0x7f
    ab20:	7100043f 	cmp	w1, #0x1
    ab24:	540001c1 	b.ne	ab5c <__pthread_mutex_unlock+0x104>
    ab28:	b9400803 	ldr	w3, [x0,#8]
    ab2c:	d53bd040 	mrs	x0, tpidr_el0
    ab30:	d11bc000 	sub	x0, x0, #0x6f0
    ab34:	b940d000 	ldr	w0, [x0,#208]
    ab38:	6b00007f 	cmp	w3, w0
    ab3c:	54000060 	b.eq	ab48 <__pthread_mutex_unlock+0xf0>
    ab40:	2a0103e4 	mov	w4, w1
    ab44:	17ffffd6 	b	aa9c <__pthread_mutex_unlock+0x44>
    ab48:	b9400440 	ldr	w0, [x2,#4]
    ab4c:	51000400 	sub	w0, w0, #0x1
    ab50:	b9000440 	str	w0, [x2,#4]
    ab54:	34fff900 	cbz	w0, aa74 <__pthread_mutex_unlock+0x1c>
    ab58:	17ffffd1 	b	aa9c <__pthread_mutex_unlock+0x44>
    ab5c:	71000c3f 	cmp	w1, #0x3
    ab60:	54fff8a0 	b.eq	aa74 <__pthread_mutex_unlock+0x1c>
    ab64:	b9400801 	ldr	w1, [x0,#8]
    ab68:	d53bd040 	mrs	x0, tpidr_el0
    ab6c:	d11bc000 	sub	x0, x0, #0x6f0
    ab70:	b940d000 	ldr	w0, [x0,#208]
    ab74:	6b00003f 	cmp	w1, w0
    ab78:	54000060 	b.eq	ab84 <__pthread_mutex_unlock+0x12c>
    ab7c:	52800024 	mov	w4, #0x1                   	// #1
    ab80:	17ffffc7 	b	aa9c <__pthread_mutex_unlock+0x44>
    ab84:	b9400040 	ldr	w0, [x2]
    ab88:	35fff760 	cbnz	w0, aa74 <__pthread_mutex_unlock+0x1c>
    ab8c:	52800024 	mov	w4, #0x1                   	// #1
    ab90:	17ffffc3 	b	aa9c <__pthread_mutex_unlock+0x44>

000000000000ab94 <__pthread_mutex_cond_lock_full>:
    ab94:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    ab98:	910003fd 	mov	x29, sp
    ab9c:	a9025bf5 	stp	x21, x22, [sp,#32]
    aba0:	a90153f3 	stp	x19, x20, [sp,#16]
    aba4:	a90363f7 	stp	x23, x24, [sp,#48]
    aba8:	f90023f9 	str	x25, [sp,#64]
    abac:	d53bd056 	mrs	x22, tpidr_el0
    abb0:	d11bc2d6 	sub	x22, x22, #0x6f0
    abb4:	b9401001 	ldr	w1, [x0,#16]
    abb8:	12001822 	and	w2, w1, #0x7f
    abbc:	b940d2d5 	ldr	w21, [x22,#208]
    abc0:	51004042 	sub	w2, w2, #0x10
    abc4:	7100cc5f 	cmp	w2, #0x33
    abc8:	54000129 	b.ls	abec <__pthread_mutex_cond_lock_full+0x58>
    abcc:	528002d4 	mov	w20, #0x16                  	// #22
    abd0:	f94023f9 	ldr	x25, [sp,#64]
    abd4:	2a1403e0 	mov	w0, w20
    abd8:	a94153f3 	ldp	x19, x20, [sp,#16]
    abdc:	a9425bf5 	ldp	x21, x22, [sp,#32]
    abe0:	a94363f7 	ldp	x23, x24, [sp,#48]
    abe4:	a8c67bfd 	ldp	x29, x30, [sp],#96
    abe8:	d65f03c0 	ret
    abec:	aa0003f3 	mov	x19, x0
    abf0:	90000040 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
    abf4:	91324000 	add	x0, x0, #0xc90
    abf8:	38624800 	ldrb	w0, [x0,w2,uxtw]
    abfc:	10000062 	adr	x2, ac08 <__pthread_mutex_cond_lock_full+0x74>
    ac00:	8b208840 	add	x0, x2, w0, sxtb #2
    ac04:	d61f0000 	br	x0
    ac08:	b9400a60 	ldr	w0, [x19,#8]
    ac0c:	b9400263 	ldr	w3, [x19]
    ac10:	6b15001f 	cmp	w0, w21
    ac14:	54001da0 	b.eq	afc8 <__pthread_mutex_cond_lock_full+0x434>
    ac18:	12800016 	mov	w22, #0xffffffff            	// #-1
    ac1c:	53137c74 	lsr	w20, w3, #19
    ac20:	94001cce 	bl	11f58 <__pthread_current_priority>
    ac24:	6b00029f 	cmp	w20, w0
    ac28:	54001eab 	b.lt	affc <__pthread_mutex_cond_lock_full+0x468>
    ac2c:	2a1603e0 	mov	w0, w22
    ac30:	2a1403e1 	mov	w1, w20
    ac34:	94001c09 	bl	11c58 <__pthread_tpp_change_priority>
    ac38:	35001de0 	cbnz	w0, aff4 <__pthread_mutex_cond_lock_full+0x460>
    ac3c:	910183a5 	add	x5, x29, #0x60
    ac40:	530d3284 	lsl	w4, w20, #19
    ac44:	321f0087 	orr	w7, w4, #0x2
    ac48:	2a0403e0 	mov	w0, w4
    ac4c:	b81fcca4 	str	w4, [x5,#-4]!
    ac50:	885ffe61 	ldaxr	w1, [x19]
    ac54:	6b00003f 	cmp	w1, w0
    ac58:	54000061 	b.ne	ac64 <__pthread_mutex_cond_lock_full+0xd0>
    ac5c:	88027e67 	stxr	w2, w7, [x19]
    ac60:	35ffff82 	cbnz	w2, ac50 <__pthread_mutex_cond_lock_full+0xbc>
    ac64:	54000040 	b.eq	ac6c <__pthread_mutex_cond_lock_full+0xd8>
    ac68:	b9005fa1 	str	w1, [x29,#92]
    ac6c:	b9405fa0 	ldr	w0, [x29,#92]
    ac70:	6b00009f 	cmp	w4, w0
    ac74:	540004c0 	b.eq	ad0c <__pthread_mutex_cond_lock_full+0x178>
    ac78:	32000089 	orr	w9, w4, #0x1
    ac7c:	93407cea 	sxtw	x10, w7
    ac80:	b9005fa9 	str	w9, [x29,#92]
    ac84:	b94000a0 	ldr	w0, [x5]
    ac88:	885ffe63 	ldaxr	w3, [x19]
    ac8c:	6b00007f 	cmp	w3, w0
    ac90:	54000061 	b.ne	ac9c <__pthread_mutex_cond_lock_full+0x108>
    ac94:	88017e67 	stxr	w1, w7, [x19]
    ac98:	35ffff81 	cbnz	w1, ac88 <__pthread_mutex_cond_lock_full+0xf4>
    ac9c:	54000040 	b.eq	aca4 <__pthread_mutex_cond_lock_full+0x110>
    aca0:	b90000a3 	str	w3, [x5]
    aca4:	b9405fa3 	ldr	w3, [x29,#92]
    aca8:	120d3060 	and	w0, w3, #0xfff80000
    acac:	6b04001f 	cmp	w0, w4
    acb0:	54001541 	b.ne	af58 <__pthread_mutex_cond_lock_full+0x3c4>
    acb4:	6b03009f 	cmp	w4, w3
    acb8:	54000120 	b.eq	acdc <__pthread_mutex_cond_lock_full+0x148>
    acbc:	b9401266 	ldr	w6, [x19,#16]
    acc0:	aa1303e0 	mov	x0, x19
    acc4:	aa0a03e2 	mov	x2, x10
    acc8:	d2800003 	mov	x3, #0x0                   	// #0
    accc:	2a2603e1 	mvn	w1, w6
    acd0:	d2800c48 	mov	x8, #0x62                  	// #98
    acd4:	92790021 	and	x1, x1, #0x80
    acd8:	d4000001 	svc	#0x0
    acdc:	b9005fa4 	str	w4, [x29,#92]
    ace0:	b94000a0 	ldr	w0, [x5]
    ace4:	885ffe63 	ldaxr	w3, [x19]
    ace8:	6b00007f 	cmp	w3, w0
    acec:	54000061 	b.ne	acf8 <__pthread_mutex_cond_lock_full+0x164>
    acf0:	88017e67 	stxr	w1, w7, [x19]
    acf4:	35ffff81 	cbnz	w1, ace4 <__pthread_mutex_cond_lock_full+0x150>
    acf8:	54000040 	b.eq	ad00 <__pthread_mutex_cond_lock_full+0x16c>
    acfc:	b90000a3 	str	w3, [x5]
    ad00:	b9405fa0 	ldr	w0, [x29,#92]
    ad04:	6b00009f 	cmp	w4, w0
    ad08:	54fffbc1 	b.ne	ac80 <__pthread_mutex_cond_lock_full+0xec>
    ad0c:	52800020 	mov	w0, #0x1                   	// #1
    ad10:	b9000660 	str	w0, [x19,#4]
    ad14:	b9000a75 	str	w21, [x19,#8]
    ad18:	52800014 	mov	w20, #0x0                   	// #0
    ad1c:	2a1403e0 	mov	w0, w20
    ad20:	f94023f9 	ldr	x25, [sp,#64]
    ad24:	a94153f3 	ldp	x19, x20, [sp,#16]
    ad28:	a9425bf5 	ldp	x21, x22, [sp,#32]
    ad2c:	a94363f7 	ldp	x23, x24, [sp,#48]
    ad30:	a8c67bfd 	ldp	x29, x30, [sp],#96
    ad34:	d65f03c0 	ret
    ad38:	121c0024 	and	w4, w1, #0x10
    ad3c:	37201301 	tbnz	w1, #4, af9c <__pthread_mutex_cond_lock_full+0x408>
    ad40:	b9400260 	ldr	w0, [x19]
    ad44:	12007400 	and	w0, w0, #0x3fffffff
    ad48:	6b0002bf 	cmp	w21, w0
    ad4c:	54001660 	b.eq	b018 <__pthread_mutex_cond_lock_full+0x484>
    ad50:	b9005fbf 	str	wzr, [x29,#92]
    ad54:	320102a1 	orr	w1, w21, #0x80000000
    ad58:	885ffe60 	ldaxr	w0, [x19]
    ad5c:	6b1f001f 	cmp	w0, wzr
    ad60:	54000061 	b.ne	ad6c <__pthread_mutex_cond_lock_full+0x1d8>
    ad64:	88027e61 	stxr	w2, w1, [x19]
    ad68:	35ffff82 	cbnz	w2, ad58 <__pthread_mutex_cond_lock_full+0x1c4>
    ad6c:	54001261 	b.ne	afb8 <__pthread_mutex_cond_lock_full+0x424>
    ad70:	b9405fa0 	ldr	w0, [x29,#92]
    ad74:	34000a60 	cbz	w0, aec0 <__pthread_mutex_cond_lock_full+0x32c>
    ad78:	d28000c1 	mov	x1, #0x6                   	// #6
    ad7c:	350000c4 	cbnz	w4, ad94 <__pthread_mutex_cond_lock_full+0x200>
    ad80:	b9401261 	ldr	w1, [x19,#16]
    ad84:	528010c0 	mov	w0, #0x86                  	// #134
    ad88:	12190021 	and	w1, w1, #0x80
    ad8c:	4a000021 	eor	w1, w1, w0
    ad90:	93407c21 	sxtw	x1, w1
    ad94:	aa1303e0 	mov	x0, x19
    ad98:	d2800022 	mov	x2, #0x1                   	// #1
    ad9c:	d2800003 	mov	x3, #0x0                   	// #0
    ada0:	d2800c48 	mov	x8, #0x62                  	// #98
    ada4:	d4000001 	svc	#0x0
    ada8:	3140041f 	cmn	w0, #0x1, lsl #12
    adac:	54000089 	b.ls	adbc <__pthread_mutex_cond_lock_full+0x228>
    adb0:	121a7800 	and	w0, w0, #0xffffffdf
    adb4:	31008c1f 	cmn	w0, #0x23
    adb8:	54000fa0 	b.eq	afac <__pthread_mutex_cond_lock_full+0x418>
    adbc:	b9400260 	ldr	w0, [x19]
    adc0:	36f00800 	tbz	w0, #30, aec0 <__pthread_mutex_cond_lock_full+0x32c>
    adc4:	910173a5 	add	x5, x29, #0x5c
    adc8:	b9005fa0 	str	w0, [x29,#92]
    adcc:	12017800 	and	w0, w0, #0xbfffffff
    add0:	b94000a2 	ldr	w2, [x5]
    add4:	885ffe61 	ldaxr	w1, [x19]
    add8:	6b02003f 	cmp	w1, w2
    addc:	54000061 	b.ne	ade8 <__pthread_mutex_cond_lock_full+0x254>
    ade0:	88037e60 	stxr	w3, w0, [x19]
    ade4:	35ffff83 	cbnz	w3, add4 <__pthread_mutex_cond_lock_full+0x240>
    ade8:	54001320 	b.eq	b04c <__pthread_mutex_cond_lock_full+0x4b8>
    adec:	b90000a1 	str	w1, [x5]
    adf0:	b9400260 	ldr	w0, [x19]
    adf4:	17fffff5 	b	adc8 <__pthread_mutex_cond_lock_full+0x234>
    adf8:	91008277 	add	x23, x19, #0x20
    adfc:	f9007ad7 	str	x23, [x22,#240]
    ae00:	320102b8 	orr	w24, w21, #0x80000000
    ae04:	321f77f9 	mov	w25, #0x7ffffffe            	// #2147483646
    ae08:	b9400260 	ldr	w0, [x19]
    ae0c:	12020014 	and	w20, w0, #0x40000000
    ae10:	35000a94 	cbnz	w20, af60 <__pthread_mutex_cond_lock_full+0x3cc>
    ae14:	12007400 	and	w0, w0, #0x3fffffff
    ae18:	6b0002bf 	cmp	w21, w0
    ae1c:	54000380 	b.eq	ae8c <__pthread_mutex_cond_lock_full+0x2f8>
    ae20:	b9005fbf 	str	wzr, [x29,#92]
    ae24:	885ffe62 	ldaxr	w2, [x19]
    ae28:	6b1f005f 	cmp	w2, wzr
    ae2c:	54000061 	b.ne	ae38 <__pthread_mutex_cond_lock_full+0x2a4>
    ae30:	88007e78 	stxr	w0, w24, [x19]
    ae34:	35ffff80 	cbnz	w0, ae24 <__pthread_mutex_cond_lock_full+0x290>
    ae38:	54000680 	b.eq	af08 <__pthread_mutex_cond_lock_full+0x374>
    ae3c:	52801001 	mov	w1, #0x80                  	// #128
    ae40:	aa1303e0 	mov	x0, x19
    ae44:	b9005fa2 	str	w2, [x29,#92]
    ae48:	94001302 	bl	fa50 <__lll_robust_lock_wait>
    ae4c:	b9400a61 	ldr	w1, [x19,#8]
    ae50:	6b19003f 	cmp	w1, w25
    ae54:	54000620 	b.eq	af18 <__pthread_mutex_cond_lock_full+0x384>
    ae58:	12020014 	and	w20, w0, #0x40000000
    ae5c:	37f7fda0 	tbnz	w0, #30, ae10 <__pthread_mutex_cond_lock_full+0x27c>
    ae60:	aa1603e0 	mov	x0, x22
    ae64:	52800021 	mov	w1, #0x1                   	// #1
    ae68:	b9000661 	str	w1, [x19,#4]
    ae6c:	f84e0c01 	ldr	x1, [x0,#224]!
    ae70:	927ff822 	and	x2, x1, #0xfffffffffffffffe
    ae74:	f81f8057 	str	x23, [x2,#-8]
    ae78:	f9001261 	str	x1, [x19,#32]
    ae7c:	f9000e60 	str	x0, [x19,#24]
    ae80:	f90072d7 	str	x23, [x22,#224]
    ae84:	f9007adf 	str	xzr, [x22,#240]
    ae88:	17ffffa3 	b	ad14 <__pthread_mutex_cond_lock_full+0x180>
    ae8c:	b9401260 	ldr	w0, [x19,#16]
    ae90:	12001800 	and	w0, w0, #0x7f
    ae94:	7100481f 	cmp	w0, #0x12
    ae98:	54001440 	b.eq	b120 <__pthread_mutex_cond_lock_full+0x58c>
    ae9c:	7100441f 	cmp	w0, #0x11
    aea0:	54fffc01 	b.ne	ae20 <__pthread_mutex_cond_lock_full+0x28c>
    aea4:	f9007adf 	str	xzr, [x22,#240]
    aea8:	b9400660 	ldr	w0, [x19,#4]
    aeac:	3100041f 	cmn	w0, #0x1
    aeb0:	540009e0 	b.eq	afec <__pthread_mutex_cond_lock_full+0x458>
    aeb4:	11000400 	add	w0, w0, #0x1
    aeb8:	b9000660 	str	w0, [x19,#4]
    aebc:	17ffff98 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>
    aec0:	34fff264 	cbz	w4, ad0c <__pthread_mutex_cond_lock_full+0x178>
    aec4:	b9400a61 	ldr	w1, [x19,#8]
    aec8:	321f77e0 	mov	w0, #0x7ffffffe            	// #2147483646
    aecc:	6b00003f 	cmp	w1, w0
    aed0:	54001140 	b.eq	b0f8 <__pthread_mutex_cond_lock_full+0x564>
    aed4:	aa1603e0 	mov	x0, x22
    aed8:	52800021 	mov	w1, #0x1                   	// #1
    aedc:	b9000661 	str	w1, [x19,#4]
    aee0:	91008262 	add	x2, x19, #0x20
    aee4:	b2400043 	orr	x3, x2, #0x1
    aee8:	f84e0c01 	ldr	x1, [x0,#224]!
    aeec:	927ff824 	and	x4, x1, #0xfffffffffffffffe
    aef0:	f81f8082 	str	x2, [x4,#-8]
    aef4:	f9001261 	str	x1, [x19,#32]
    aef8:	f9000e60 	str	x0, [x19,#24]
    aefc:	f90072c3 	str	x3, [x22,#224]
    af00:	f9007adf 	str	xzr, [x22,#240]
    af04:	17ffff84 	b	ad14 <__pthread_mutex_cond_lock_full+0x180>
    af08:	b9400a61 	ldr	w1, [x19,#8]
    af0c:	321f77e0 	mov	w0, #0x7ffffffe            	// #2147483646
    af10:	6b00003f 	cmp	w1, w0
    af14:	54fffa61 	b.ne	ae60 <__pthread_mutex_cond_lock_full+0x2cc>
    af18:	b900067f 	str	wzr, [x19,#4]
    af1c:	52800001 	mov	w1, #0x0                   	// #0
    af20:	885f7e60 	ldxr	w0, [x19]
    af24:	8802fe61 	stlxr	w2, w1, [x19]
    af28:	35ffffc2 	cbnz	w2, af20 <__pthread_mutex_cond_lock_full+0x38c>
    af2c:	7100041f 	cmp	w0, #0x1
    af30:	54000d6c 	b.gt	b0dc <__pthread_mutex_cond_lock_full+0x548>
    af34:	f9007adf 	str	xzr, [x22,#240]
    af38:	52801074 	mov	w20, #0x83                  	// #131
    af3c:	2a1403e0 	mov	w0, w20
    af40:	f94023f9 	ldr	x25, [sp,#64]
    af44:	a94153f3 	ldp	x19, x20, [sp,#16]
    af48:	a9425bf5 	ldp	x21, x22, [sp,#32]
    af4c:	a94363f7 	ldp	x23, x24, [sp,#48]
    af50:	a8c67bfd 	ldp	x29, x30, [sp],#96
    af54:	d65f03c0 	ret
    af58:	2a1403f6 	mov	w22, w20
    af5c:	17ffff30 	b	ac1c <__pthread_mutex_cond_lock_full+0x88>
    af60:	b9005fa0 	str	w0, [x29,#92]
    af64:	2a0003e1 	mov	w1, w0
    af68:	885ffe62 	ldaxr	w2, [x19]
    af6c:	6b01005f 	cmp	w2, w1
    af70:	54000061 	b.ne	af7c <__pthread_mutex_cond_lock_full+0x3e8>
    af74:	88037e78 	stxr	w3, w24, [x19]
    af78:	35ffff83 	cbnz	w3, af68 <__pthread_mutex_cond_lock_full+0x3d4>
    af7c:	54000040 	b.eq	af84 <__pthread_mutex_cond_lock_full+0x3f0>
    af80:	b9005fa2 	str	w2, [x29,#92]
    af84:	b9405fa1 	ldr	w1, [x29,#92]
    af88:	6b01001f 	cmp	w0, w1
    af8c:	54000860 	b.eq	b098 <__pthread_mutex_cond_lock_full+0x504>
    af90:	2a0103e0 	mov	w0, w1
    af94:	12020034 	and	w20, w1, #0x40000000
    af98:	17ffff9e 	b	ae10 <__pthread_mutex_cond_lock_full+0x27c>
    af9c:	91008260 	add	x0, x19, #0x20
    afa0:	b2400000 	orr	x0, x0, #0x1
    afa4:	f9007ac0 	str	x0, [x22,#240]
    afa8:	17ffff66 	b	ad40 <__pthread_mutex_cond_lock_full+0x1ac>
    afac:	940016ab 	bl	10a58 <__pause_nocancel>
    afb0:	940016aa 	bl	10a58 <__pause_nocancel>
    afb4:	17fffffe 	b	afac <__pthread_mutex_cond_lock_full+0x418>
    afb8:	b9005fa0 	str	w0, [x29,#92]
    afbc:	b9405fa0 	ldr	w0, [x29,#92]
    afc0:	34fff800 	cbz	w0, aec0 <__pthread_mutex_cond_lock_full+0x32c>
    afc4:	17ffff6d 	b	ad78 <__pthread_mutex_cond_lock_full+0x1e4>
    afc8:	12000421 	and	w1, w1, #0x3
    afcc:	52800474 	mov	w20, #0x23                  	// #35
    afd0:	7100083f 	cmp	w1, #0x2
    afd4:	54ffea40 	b.eq	ad1c <__pthread_mutex_cond_lock_full+0x188>
    afd8:	7100043f 	cmp	w1, #0x1
    afdc:	54ffe1e1 	b.ne	ac18 <__pthread_mutex_cond_lock_full+0x84>
    afe0:	b9400660 	ldr	w0, [x19,#4]
    afe4:	3100041f 	cmn	w0, #0x1
    afe8:	540002a1 	b.ne	b03c <__pthread_mutex_cond_lock_full+0x4a8>
    afec:	52800174 	mov	w20, #0xb                   	// #11
    aff0:	17ffff4b 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>
    aff4:	2a0003f4 	mov	w20, w0
    aff8:	17ffff49 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>
    affc:	310006df 	cmn	w22, #0x1
    b000:	528002d4 	mov	w20, #0x16                  	// #22
    b004:	54ffe8c0 	b.eq	ad1c <__pthread_mutex_cond_lock_full+0x188>
    b008:	2a1603e0 	mov	w0, w22
    b00c:	12800001 	mov	w1, #0xffffffff            	// #-1
    b010:	94001b12 	bl	11c58 <__pthread_tpp_change_priority>
    b014:	17ffff42 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>
    b018:	12000421 	and	w1, w1, #0x3
    b01c:	7100083f 	cmp	w1, #0x2
    b020:	54000800 	b.eq	b120 <__pthread_mutex_cond_lock_full+0x58c>
    b024:	7100043f 	cmp	w1, #0x1
    b028:	54ffe941 	b.ne	ad50 <__pthread_mutex_cond_lock_full+0x1bc>
    b02c:	f9007adf 	str	xzr, [x22,#240]
    b030:	b9400660 	ldr	w0, [x19,#4]
    b034:	3100041f 	cmn	w0, #0x1
    b038:	54fffda0 	b.eq	afec <__pthread_mutex_cond_lock_full+0x458>
    b03c:	11000400 	add	w0, w0, #0x1
    b040:	52800014 	mov	w20, #0x0                   	// #0
    b044:	b9000660 	str	w0, [x19,#4]
    b048:	17ffff35 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>
    b04c:	aa1603e0 	mov	x0, x22
    b050:	52800021 	mov	w1, #0x1                   	// #1
    b054:	b9000661 	str	w1, [x19,#4]
    b058:	12b00001 	mov	w1, #0x7fffffff            	// #2147483647
    b05c:	b9000a61 	str	w1, [x19,#8]
    b060:	91008262 	add	x2, x19, #0x20
    b064:	b2400043 	orr	x3, x2, #0x1
    b068:	52801054 	mov	w20, #0x82                  	// #130
    b06c:	f84e0c01 	ldr	x1, [x0,#224]!
    b070:	927ff824 	and	x4, x1, #0xfffffffffffffffe
    b074:	f81f8082 	str	x2, [x4,#-8]
    b078:	f9000e60 	str	x0, [x19,#24]
    b07c:	f9001261 	str	x1, [x19,#32]
    b080:	f90072c3 	str	x3, [x22,#224]
    b084:	f9007adf 	str	xzr, [x22,#240]
    b088:	b9400e60 	ldr	w0, [x19,#12]
    b08c:	51000400 	sub	w0, w0, #0x1
    b090:	b9000e60 	str	w0, [x19,#12]
    b094:	17ffff22 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>
    b098:	aa1603e0 	mov	x0, x22
    b09c:	52800021 	mov	w1, #0x1                   	// #1
    b0a0:	b9000661 	str	w1, [x19,#4]
    b0a4:	12b00001 	mov	w1, #0x7fffffff            	// #2147483647
    b0a8:	b9000a61 	str	w1, [x19,#8]
    b0ac:	52801054 	mov	w20, #0x82                  	// #130
    b0b0:	f84e0c01 	ldr	x1, [x0,#224]!
    b0b4:	927ff822 	and	x2, x1, #0xfffffffffffffffe
    b0b8:	f81f8057 	str	x23, [x2,#-8]
    b0bc:	f9000e60 	str	x0, [x19,#24]
    b0c0:	f9001261 	str	x1, [x19,#32]
    b0c4:	f90072d7 	str	x23, [x22,#224]
    b0c8:	f9007adf 	str	xzr, [x22,#240]
    b0cc:	b9400e60 	ldr	w0, [x19,#12]
    b0d0:	51000400 	sub	w0, w0, #0x1
    b0d4:	b9000e60 	str	w0, [x19,#12]
    b0d8:	17ffff11 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>
    b0dc:	d2800021 	mov	x1, #0x1                   	// #1
    b0e0:	aa1303e0 	mov	x0, x19
    b0e4:	aa0103e2 	mov	x2, x1
    b0e8:	d2800003 	mov	x3, #0x0                   	// #0
    b0ec:	d2800c48 	mov	x8, #0x62                  	// #98
    b0f0:	d4000001 	svc	#0x0
    b0f4:	17ffff90 	b	af34 <__pthread_mutex_cond_lock_full+0x3a0>
    b0f8:	d2800002 	mov	x2, #0x0                   	// #0
    b0fc:	b900067f 	str	wzr, [x19,#4]
    b100:	aa1303e0 	mov	x0, x19
    b104:	d28000e1 	mov	x1, #0x7                   	// #7
    b108:	aa0203e3 	mov	x3, x2
    b10c:	d2800c48 	mov	x8, #0x62                  	// #98
    b110:	d4000001 	svc	#0x0
    b114:	52801074 	mov	w20, #0x83                  	// #131
    b118:	f9007ac2 	str	x2, [x22,#240]
    b11c:	17ffff00 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>
    b120:	f9007adf 	str	xzr, [x22,#240]
    b124:	52800474 	mov	w20, #0x23                  	// #35
    b128:	17fffefd 	b	ad1c <__pthread_mutex_cond_lock_full+0x188>

000000000000b12c <__pthread_mutex_cond_lock>:
    b12c:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    b130:	52802fe2 	mov	w2, #0x17f                 	// #383
    b134:	910003fd 	mov	x29, sp
    b138:	a90153f3 	stp	x19, x20, [sp,#16]
    b13c:	f90013f5 	str	x21, [sp,#32]
    b140:	aa0003f3 	mov	x19, x0
    b144:	b9401001 	ldr	w1, [x0,#16]
    b148:	721e1023 	ands	w3, w1, #0x7c
    b14c:	0a020022 	and	w2, w1, w2
    b150:	540004e1 	b.ne	b1ec <__pthread_mutex_cond_lock+0xc0>
    b154:	350001e2 	cbnz	w2, b190 <__pthread_mutex_cond_lock+0x64>
    b158:	d53bd054 	mrs	x20, tpidr_el0
    b15c:	52800041 	mov	w1, #0x2                   	// #2
    b160:	885ffe60 	ldaxr	w0, [x19]
    b164:	88027e61 	stxr	w2, w1, [x19]
    b168:	35ffffc2 	cbnz	w2, b160 <__pthread_mutex_cond_lock+0x34>
    b16c:	35000340 	cbnz	w0, b1d4 <__pthread_mutex_cond_lock+0xa8>
    b170:	d11bc294 	sub	x20, x20, #0x6f0
    b174:	b940d281 	ldr	w1, [x20,#208]
    b178:	52800000 	mov	w0, #0x0                   	// #0
    b17c:	b9000a61 	str	w1, [x19,#8]
    b180:	a94153f3 	ldp	x19, x20, [sp,#16]
    b184:	f94013f5 	ldr	x21, [sp,#32]
    b188:	a8c47bfd 	ldp	x29, x30, [sp],#64
    b18c:	d65f03c0 	ret
    b190:	12001821 	and	w1, w1, #0x7f
    b194:	7100043f 	cmp	w1, #0x1
    b198:	54000421 	b.ne	b21c <__pthread_mutex_cond_lock+0xf0>
    b19c:	d53bd054 	mrs	x20, tpidr_el0
    b1a0:	b9400802 	ldr	w2, [x0,#8]
    b1a4:	d11bc294 	sub	x20, x20, #0x6f0
    b1a8:	b940d281 	ldr	w1, [x20,#208]
    b1ac:	6b01005f 	cmp	w2, w1
    b1b0:	54000280 	b.eq	b200 <__pthread_mutex_cond_lock+0xd4>
    b1b4:	52800042 	mov	w2, #0x2                   	// #2
    b1b8:	885ffe61 	ldaxr	w1, [x19]
    b1bc:	88037e62 	stxr	w3, w2, [x19]
    b1c0:	35ffffc3 	cbnz	w3, b1b8 <__pthread_mutex_cond_lock+0x8c>
    b1c4:	350004c1 	cbnz	w1, b25c <__pthread_mutex_cond_lock+0x130>
    b1c8:	52800020 	mov	w0, #0x1                   	// #1
    b1cc:	b9000660 	str	w0, [x19,#4]
    b1d0:	17ffffe9 	b	b174 <__pthread_mutex_cond_lock+0x48>
    b1d4:	b9401261 	ldr	w1, [x19,#16]
    b1d8:	aa1303e0 	mov	x0, x19
    b1dc:	d11bc294 	sub	x20, x20, #0x6f0
    b1e0:	12190021 	and	w1, w1, #0x80
    b1e4:	94001178 	bl	f7c4 <__lll_lock_wait>
    b1e8:	17ffffe3 	b	b174 <__pthread_mutex_cond_lock+0x48>
    b1ec:	97fffe6a 	bl	ab94 <__pthread_mutex_cond_lock_full>
    b1f0:	f94013f5 	ldr	x21, [sp,#32]
    b1f4:	a94153f3 	ldp	x19, x20, [sp,#16]
    b1f8:	a8c47bfd 	ldp	x29, x30, [sp],#64
    b1fc:	d65f03c0 	ret
    b200:	b9400400 	ldr	w0, [x0,#4]
    b204:	3100041f 	cmn	w0, #0x1
    b208:	54000320 	b.eq	b26c <__pthread_mutex_cond_lock+0x140>
    b20c:	11000401 	add	w1, w0, #0x1
    b210:	2a0303e0 	mov	w0, w3
    b214:	b9000661 	str	w1, [x19,#4]
    b218:	17ffffda 	b	b180 <__pthread_mutex_cond_lock+0x54>
    b21c:	71000c3f 	cmp	w1, #0x3
    b220:	540007a1 	b.ne	b314 <__pthread_mutex_cond_lock+0x1e8>
    b224:	b0000140 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    b228:	b9432c00 	ldr	w0, [x0,#812]
    b22c:	34fff960 	cbz	w0, b158 <__pthread_mutex_cond_lock+0x2c>
    b230:	b9003fa3 	str	w3, [x29,#60]
    b234:	52800042 	mov	w2, #0x2                   	// #2
    b238:	885ffe60 	ldaxr	w0, [x19]
    b23c:	6b1f001f 	cmp	w0, wzr
    b240:	54000061 	b.ne	b24c <__pthread_mutex_cond_lock+0x120>
    b244:	88017e62 	stxr	w1, w2, [x19]
    b248:	35ffff81 	cbnz	w1, b238 <__pthread_mutex_cond_lock+0x10c>
    b24c:	1a9f17f5 	cset	w21, eq
    b250:	34000135 	cbz	w21, b274 <__pthread_mutex_cond_lock+0x148>
    b254:	d53bd054 	mrs	x20, tpidr_el0
    b258:	17ffffc6 	b	b170 <__pthread_mutex_cond_lock+0x44>
    b25c:	b9401261 	ldr	w1, [x19,#16]
    b260:	12190021 	and	w1, w1, #0x80
    b264:	94001158 	bl	f7c4 <__lll_lock_wait>
    b268:	17ffffd8 	b	b1c8 <__pthread_mutex_cond_lock+0x9c>
    b26c:	52800160 	mov	w0, #0xb                   	// #11
    b270:	17ffffc4 	b	b180 <__pthread_mutex_cond_lock+0x54>
    b274:	b9401661 	ldr	w1, [x19,#20]
    b278:	b9003fa0 	str	w0, [x29,#60]
    b27c:	52800c80 	mov	w0, #0x64                  	// #100
    b280:	11001421 	add	w1, w1, #0x5
    b284:	531f7821 	lsl	w1, w1, #1
    b288:	7101903f 	cmp	w1, #0x64
    b28c:	1a80d021 	csel	w1, w1, w0, le
    b290:	110006b5 	add	w21, w21, #0x1
    b294:	510006a0 	sub	w0, w21, #0x1
    b298:	6b01001f 	cmp	w0, w1
    b29c:	5400028a 	b.ge	b2ec <__pthread_mutex_cond_lock+0x1c0>
    b2a0:	b9003fbf 	str	wzr, [x29,#60]
    b2a4:	885ffe60 	ldaxr	w0, [x19]
    b2a8:	6b1f001f 	cmp	w0, wzr
    b2ac:	54000061 	b.ne	b2b8 <__pthread_mutex_cond_lock+0x18c>
    b2b0:	88037e62 	stxr	w3, w2, [x19]
    b2b4:	35ffff83 	cbnz	w3, b2a4 <__pthread_mutex_cond_lock+0x178>
    b2b8:	54000161 	b.ne	b2e4 <__pthread_mutex_cond_lock+0x1b8>
    b2bc:	b9401661 	ldr	w1, [x19,#20]
    b2c0:	d53bd054 	mrs	x20, tpidr_el0
    b2c4:	d11bc294 	sub	x20, x20, #0x6f0
    b2c8:	4b0102b5 	sub	w21, w21, w1
    b2cc:	11001ea0 	add	w0, w21, #0x7
    b2d0:	6b1f02bf 	cmp	w21, wzr
    b2d4:	1a95b015 	csel	w21, w0, w21, lt
    b2d8:	0b950c35 	add	w21, w1, w21, asr #3
    b2dc:	b9001675 	str	w21, [x19,#20]
    b2e0:	17ffffa5 	b	b174 <__pthread_mutex_cond_lock+0x48>
    b2e4:	b9003fa0 	str	w0, [x29,#60]
    b2e8:	17ffffea 	b	b290 <__pthread_mutex_cond_lock+0x164>
    b2ec:	52800041 	mov	w1, #0x2                   	// #2
    b2f0:	885ffe60 	ldaxr	w0, [x19]
    b2f4:	88027e61 	stxr	w2, w1, [x19]
    b2f8:	35ffffc2 	cbnz	w2, b2f0 <__pthread_mutex_cond_lock+0x1c4>
    b2fc:	34fffe00 	cbz	w0, b2bc <__pthread_mutex_cond_lock+0x190>
    b300:	b9401261 	ldr	w1, [x19,#16]
    b304:	aa1303e0 	mov	x0, x19
    b308:	12190021 	and	w1, w1, #0x80
    b30c:	9400112e 	bl	f7c4 <__lll_lock_wait>
    b310:	17ffffeb 	b	b2bc <__pthread_mutex_cond_lock+0x190>
    b314:	d53bd054 	mrs	x20, tpidr_el0
    b318:	b9400800 	ldr	w0, [x0,#8]
    b31c:	d11bc281 	sub	x1, x20, #0x6f0
    b320:	b940d021 	ldr	w1, [x1,#208]
    b324:	6b00003f 	cmp	w1, w0
    b328:	54fff1a1 	b.ne	b15c <__pthread_mutex_cond_lock+0x30>
    b32c:	52800460 	mov	w0, #0x23                  	// #35
    b330:	17ffff94 	b	b180 <__pthread_mutex_cond_lock+0x54>

000000000000b334 <__pthread_mutex_cond_lock_adjust>:
    b334:	d53bd041 	mrs	x1, tpidr_el0
    b338:	b9401002 	ldr	w2, [x0,#16]
    b33c:	d11bc021 	sub	x1, x1, #0x6f0
    b340:	7100845f 	cmp	w2, #0x21
    b344:	b940d021 	ldr	w1, [x1,#208]
    b348:	b9000801 	str	w1, [x0,#8]
    b34c:	54000040 	b.eq	b354 <__pthread_mutex_cond_lock_adjust+0x20>
    b350:	d65f03c0 	ret
    b354:	b9400401 	ldr	w1, [x0,#4]
    b358:	11000421 	add	w1, w1, #0x1
    b35c:	b9000401 	str	w1, [x0,#4]
    b360:	d65f03c0 	ret

000000000000b364 <__pthread_mutexattr_init>:
    b364:	f900001f 	str	xzr, [x0]
    b368:	52800000 	mov	w0, #0x0                   	// #0
    b36c:	d65f03c0 	ret

000000000000b370 <__pthread_mutexattr_destroy>:
    b370:	52800000 	mov	w0, #0x0                   	// #0
    b374:	d65f03c0 	ret

000000000000b378 <pthread_mutexattr_getpshared>:
    b378:	b9400002 	ldr	w2, [x0]
    b37c:	52800000 	mov	w0, #0x0                   	// #0
    b380:	531f7c42 	lsr	w2, w2, #31
    b384:	b9000022 	str	w2, [x1]
    b388:	d65f03c0 	ret

000000000000b38c <pthread_mutexattr_setpshared>:
    b38c:	350000c1 	cbnz	w1, b3a4 <pthread_mutexattr_setpshared+0x18>
    b390:	b9400002 	ldr	w2, [x0]
    b394:	12007842 	and	w2, w2, #0x7fffffff
    b398:	b9000002 	str	w2, [x0]
    b39c:	2a0103e0 	mov	w0, w1
    b3a0:	d65f03c0 	ret
    b3a4:	7100043f 	cmp	w1, #0x1
    b3a8:	540000e1 	b.ne	b3c4 <pthread_mutexattr_setpshared+0x38>
    b3ac:	b9400002 	ldr	w2, [x0]
    b3b0:	52800001 	mov	w1, #0x0                   	// #0
    b3b4:	32010042 	orr	w2, w2, #0x80000000
    b3b8:	b9000002 	str	w2, [x0]
    b3bc:	2a0103e0 	mov	w0, w1
    b3c0:	d65f03c0 	ret
    b3c4:	528002c1 	mov	w1, #0x16                  	// #22
    b3c8:	17fffff5 	b	b39c <pthread_mutexattr_setpshared+0x10>

000000000000b3cc <pthread_mutexattr_gettype>:
    b3cc:	b9400003 	ldr	w3, [x0]
    b3d0:	5281ffe2 	mov	w2, #0xfff                 	// #4095
    b3d4:	72a1e002 	movk	w2, #0xf00, lsl #16
    b3d8:	52800000 	mov	w0, #0x0                   	// #0
    b3dc:	0a020062 	and	w2, w3, w2
    b3e0:	b9000022 	str	w2, [x1]
    b3e4:	d65f03c0 	ret

000000000000b3e8 <__pthread_mutexattr_settype>:
    b3e8:	71000c3f 	cmp	w1, #0x3
    b3ec:	aa0003e3 	mov	x3, x0
    b3f0:	528002c0 	mov	w0, #0x16                  	// #22
    b3f4:	54000049 	b.ls	b3fc <__pthread_mutexattr_settype+0x14>
    b3f8:	d65f03c0 	ret
    b3fc:	b9400060 	ldr	w0, [x3]
    b400:	529e0002 	mov	w2, #0xf000                	// #61440
    b404:	6b1f003f 	cmp	w1, wzr
    b408:	52804004 	mov	w4, #0x200                 	// #512
    b40c:	72be1fe2 	movk	w2, #0xf0ff, lsl #16
    b410:	1a841021 	csel	w1, w1, w4, ne
    b414:	0a020002 	and	w2, w0, w2
    b418:	52800000 	mov	w0, #0x0                   	// #0
    b41c:	2a020021 	orr	w1, w1, w2
    b420:	b9000061 	str	w1, [x3]
    b424:	d65f03c0 	ret

000000000000b428 <__pthread_rwlock_init>:
    b428:	aa0003e2 	mov	x2, x0
    b42c:	b4000201 	cbz	x1, b46c <__pthread_rwlock_init+0x44>
    b430:	a9017c5f 	stp	xzr, xzr, [x2,#16]
    b434:	f900185f 	str	xzr, [x2,#48]
    b438:	52800000 	mov	w0, #0x0                   	// #0
    b43c:	a9007c5f 	stp	xzr, xzr, [x2]
    b440:	a9027c5f 	stp	xzr, xzr, [x2,#32]
    b444:	b9400023 	ldr	w3, [x1]
    b448:	7100087f 	cmp	w3, #0x2
    b44c:	1a9f17e3 	cset	w3, eq
    b450:	b9003043 	str	w3, [x2,#48]
    b454:	52801003 	mov	w3, #0x80                  	// #128
    b458:	b9400421 	ldr	w1, [x1,#4]
    b45c:	6b00003f 	cmp	w1, w0
    b460:	1a830021 	csel	w1, w1, w3, eq
    b464:	b9001c41 	str	w1, [x2,#28]
    b468:	d65f03c0 	ret
    b46c:	f0000021 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
    b470:	912ee021 	add	x1, x1, #0xbb8
    b474:	17ffffef 	b	b430 <__pthread_rwlock_init+0x8>

000000000000b478 <__pthread_rwlock_destroy>:
    b478:	52800000 	mov	w0, #0x0                   	// #0
    b47c:	d65f03c0 	ret

000000000000b480 <__pthread_rwlock_rdlock_slow>:
    b480:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    b484:	910003fd 	mov	x29, sp
    b488:	a90153f3 	stp	x19, x20, [sp,#16]
    b48c:	a9025bf5 	stp	x21, x22, [sp,#32]
    b490:	f9001bf7 	str	x23, [sp,#48]
    b494:	d53bd054 	mrs	x20, tpidr_el0
    b498:	aa0003f3 	mov	x19, x0
    b49c:	d11bc294 	sub	x20, x20, #0x6f0
    b4a0:	b9401801 	ldr	w1, [x0,#24]
    b4a4:	52800016 	mov	w22, #0x0                   	// #0
    b4a8:	52801037 	mov	w23, #0x81                  	// #129
    b4ac:	52800035 	mov	w21, #0x1                   	// #1
    b4b0:	b940d280 	ldr	w0, [x20,#208]
    b4b4:	6b01001f 	cmp	w0, w1
    b4b8:	54000700 	b.eq	b598 <__pthread_rwlock_rdlock_slow+0x118>
    b4bc:	b9401261 	ldr	w1, [x19,#16]
    b4c0:	11000420 	add	w0, w1, #0x1
    b4c4:	b9001260 	str	w0, [x19,#16]
    b4c8:	340006c0 	cbz	w0, b5a0 <__pthread_rwlock_rdlock_slow+0x120>
    b4cc:	b9400a64 	ldr	w4, [x19,#8]
    b4d0:	885f7e60 	ldxr	w0, [x19]
    b4d4:	8801fe76 	stlxr	w1, w22, [x19]
    b4d8:	35ffffc1 	cbnz	w1, b4d0 <__pthread_rwlock_rdlock_slow+0x50>
    b4dc:	7100041f 	cmp	w0, #0x1
    b4e0:	5400066c 	b.gt	b5ac <__pthread_rwlock_rdlock_slow+0x12c>
    b4e4:	b9401e61 	ldr	w1, [x19,#28]
    b4e8:	93407c82 	sxtw	x2, w4
    b4ec:	91002260 	add	x0, x19, #0x8
    b4f0:	d2800003 	mov	x3, #0x0                   	// #0
    b4f4:	52190021 	eor	w1, w1, #0x80
    b4f8:	d2800c48 	mov	x8, #0x62                  	// #98
    b4fc:	93407c21 	sxtw	x1, w1
    b500:	d4000001 	svc	#0x0
    b504:	b9004fa3 	str	w3, [x29,#76]
    b508:	885ffe62 	ldaxr	w2, [x19]
    b50c:	6b1f005f 	cmp	w2, wzr
    b510:	54000061 	b.ne	b51c <__pthread_rwlock_rdlock_slow+0x9c>
    b514:	88007e75 	stxr	w0, w21, [x19]
    b518:	35ffff80 	cbnz	w0, b508 <__pthread_rwlock_rdlock_slow+0x88>
    b51c:	aa1303e0 	mov	x0, x19
    b520:	54000080 	b.eq	b530 <__pthread_rwlock_rdlock_slow+0xb0>
    b524:	b9401e61 	ldr	w1, [x19,#28]
    b528:	b9004fa2 	str	w2, [x29,#76]
    b52c:	940010a6 	bl	f7c4 <__lll_lock_wait>
    b530:	b9401260 	ldr	w0, [x19,#16]
    b534:	b9401a61 	ldr	w1, [x19,#24]
    b538:	51000400 	sub	w0, w0, #0x1
    b53c:	b9001260 	str	w0, [x19,#16]
    b540:	35fffb81 	cbnz	w1, b4b0 <__pthread_rwlock_rdlock_slow+0x30>
    b544:	b9401660 	ldr	w0, [x19,#20]
    b548:	34000060 	cbz	w0, b554 <__pthread_rwlock_rdlock_slow+0xd4>
    b54c:	b9403260 	ldr	w0, [x19,#48]
    b550:	35fffb00 	cbnz	w0, b4b0 <__pthread_rwlock_rdlock_slow+0x30>
    b554:	b9400661 	ldr	w1, [x19,#4]
    b558:	52800004 	mov	w4, #0x0                   	// #0
    b55c:	11000420 	add	w0, w1, #0x1
    b560:	b9000660 	str	w0, [x19,#4]
    b564:	34000540 	cbz	w0, b60c <__pthread_rwlock_rdlock_slow+0x18c>
    b568:	52800001 	mov	w1, #0x0                   	// #0
    b56c:	885f7e60 	ldxr	w0, [x19]
    b570:	8802fe61 	stlxr	w2, w1, [x19]
    b574:	35ffffc2 	cbnz	w2, b56c <__pthread_rwlock_rdlock_slow+0xec>
    b578:	7100041f 	cmp	w0, #0x1
    b57c:	540002ac 	b.gt	b5d0 <__pthread_rwlock_rdlock_slow+0x150>
    b580:	2a0403e0 	mov	w0, w4
    b584:	f9401bf7 	ldr	x23, [sp,#48]
    b588:	a94153f3 	ldp	x19, x20, [sp,#16]
    b58c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    b590:	a8c57bfd 	ldp	x29, x30, [sp],#80
    b594:	d65f03c0 	ret
    b598:	52800464 	mov	w4, #0x23                  	// #35
    b59c:	17fffff3 	b	b568 <__pthread_rwlock_rdlock_slow+0xe8>
    b5a0:	b9001261 	str	w1, [x19,#16]
    b5a4:	52800164 	mov	w4, #0xb                   	// #11
    b5a8:	17fffff0 	b	b568 <__pthread_rwlock_rdlock_slow+0xe8>
    b5ac:	b9401e61 	ldr	w1, [x19,#28]
    b5b0:	aa1303e0 	mov	x0, x19
    b5b4:	d2800022 	mov	x2, #0x1                   	// #1
    b5b8:	d2800003 	mov	x3, #0x0                   	// #0
    b5bc:	4a170021 	eor	w1, w1, w23
    b5c0:	d2800c48 	mov	x8, #0x62                  	// #98
    b5c4:	93407c21 	sxtw	x1, w1
    b5c8:	d4000001 	svc	#0x0
    b5cc:	17ffffc6 	b	b4e4 <__pthread_rwlock_rdlock_slow+0x64>
    b5d0:	b9401e63 	ldr	w3, [x19,#28]
    b5d4:	52801021 	mov	w1, #0x81                  	// #129
    b5d8:	aa1303e0 	mov	x0, x19
    b5dc:	d2800022 	mov	x2, #0x1                   	// #1
    b5e0:	4a010061 	eor	w1, w3, w1
    b5e4:	d2800c48 	mov	x8, #0x62                  	// #98
    b5e8:	d2800003 	mov	x3, #0x0                   	// #0
    b5ec:	93407c21 	sxtw	x1, w1
    b5f0:	d4000001 	svc	#0x0
    b5f4:	2a0403e0 	mov	w0, w4
    b5f8:	f9401bf7 	ldr	x23, [sp,#48]
    b5fc:	a94153f3 	ldp	x19, x20, [sp,#16]
    b600:	a9425bf5 	ldp	x21, x22, [sp,#32]
    b604:	a8c57bfd 	ldp	x29, x30, [sp],#80
    b608:	d65f03c0 	ret
    b60c:	b9000661 	str	w1, [x19,#4]
    b610:	52800164 	mov	w4, #0xb                   	// #11
    b614:	17ffffd5 	b	b568 <__pthread_rwlock_rdlock_slow+0xe8>

000000000000b618 <__pthread_rwlock_rdlock>:
    b618:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    b61c:	52800021 	mov	w1, #0x1                   	// #1
    b620:	910003fd 	mov	x29, sp
    b624:	f9000bf3 	str	x19, [sp,#16]
    b628:	aa0003f3 	mov	x19, x0
    b62c:	b9002fbf 	str	wzr, [x29,#44]
    b630:	885ffe62 	ldaxr	w2, [x19]
    b634:	6b1f005f 	cmp	w2, wzr
    b638:	54000061 	b.ne	b644 <__pthread_rwlock_rdlock+0x2c>
    b63c:	88037e61 	stxr	w3, w1, [x19]
    b640:	35ffff83 	cbnz	w3, b630 <__pthread_rwlock_rdlock+0x18>
    b644:	54000281 	b.ne	b694 <__pthread_rwlock_rdlock+0x7c>
    b648:	b9401a60 	ldr	w0, [x19,#24]
    b64c:	350002e0 	cbnz	w0, b6a8 <__pthread_rwlock_rdlock+0x90>
    b650:	b9401660 	ldr	w0, [x19,#20]
    b654:	35000380 	cbnz	w0, b6c4 <__pthread_rwlock_rdlock+0xac>
    b658:	b9400661 	ldr	w1, [x19,#4]
    b65c:	52800004 	mov	w4, #0x0                   	// #0
    b660:	11000420 	add	w0, w1, #0x1
    b664:	b9000660 	str	w0, [x19,#4]
    b668:	340003c0 	cbz	w0, b6e0 <__pthread_rwlock_rdlock+0xc8>
    b66c:	52800001 	mov	w1, #0x0                   	// #0
    b670:	885f7e60 	ldxr	w0, [x19]
    b674:	8802fe61 	stlxr	w2, w1, [x19]
    b678:	35ffffc2 	cbnz	w2, b670 <__pthread_rwlock_rdlock+0x58>
    b67c:	7100041f 	cmp	w0, #0x1
    b680:	5400036c 	b.gt	b6ec <__pthread_rwlock_rdlock+0xd4>
    b684:	2a0403e0 	mov	w0, w4
    b688:	f9400bf3 	ldr	x19, [sp,#16]
    b68c:	a8c37bfd 	ldp	x29, x30, [sp],#48
    b690:	d65f03c0 	ret
    b694:	b9401e61 	ldr	w1, [x19,#28]
    b698:	b9002fa2 	str	w2, [x29,#44]
    b69c:	9400104a 	bl	f7c4 <__lll_lock_wait>
    b6a0:	b9401a60 	ldr	w0, [x19,#24]
    b6a4:	34fffd60 	cbz	w0, b650 <__pthread_rwlock_rdlock+0x38>
    b6a8:	aa1303e0 	mov	x0, x19
    b6ac:	97ffff75 	bl	b480 <__pthread_rwlock_rdlock_slow>
    b6b0:	2a0003e4 	mov	w4, w0
    b6b4:	2a0403e0 	mov	w0, w4
    b6b8:	f9400bf3 	ldr	x19, [sp,#16]
    b6bc:	a8c37bfd 	ldp	x29, x30, [sp],#48
    b6c0:	d65f03c0 	ret
    b6c4:	b9403260 	ldr	w0, [x19,#48]
    b6c8:	35ffff00 	cbnz	w0, b6a8 <__pthread_rwlock_rdlock+0x90>
    b6cc:	b9400661 	ldr	w1, [x19,#4]
    b6d0:	52800004 	mov	w4, #0x0                   	// #0
    b6d4:	11000420 	add	w0, w1, #0x1
    b6d8:	b9000660 	str	w0, [x19,#4]
    b6dc:	35fffc80 	cbnz	w0, b66c <__pthread_rwlock_rdlock+0x54>
    b6e0:	b9000661 	str	w1, [x19,#4]
    b6e4:	52800164 	mov	w4, #0xb                   	// #11
    b6e8:	17ffffe1 	b	b66c <__pthread_rwlock_rdlock+0x54>
    b6ec:	b9401e63 	ldr	w3, [x19,#28]
    b6f0:	52801021 	mov	w1, #0x81                  	// #129
    b6f4:	aa1303e0 	mov	x0, x19
    b6f8:	d2800022 	mov	x2, #0x1                   	// #1
    b6fc:	4a010061 	eor	w1, w3, w1
    b700:	d2800c48 	mov	x8, #0x62                  	// #98
    b704:	d2800003 	mov	x3, #0x0                   	// #0
    b708:	93407c21 	sxtw	x1, w1
    b70c:	d4000001 	svc	#0x0
    b710:	17ffffdd 	b	b684 <__pthread_rwlock_rdlock+0x6c>

000000000000b714 <pthread_rwlock_timedrdlock>:
    b714:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    b718:	910003fd 	mov	x29, sp
    b71c:	a90153f3 	stp	x19, x20, [sp,#16]
    b720:	a9025bf5 	stp	x21, x22, [sp,#32]
    b724:	aa0103f4 	mov	x20, x1
    b728:	b9005fbf 	str	wzr, [x29,#92]
    b72c:	a90363f7 	stp	x23, x24, [sp,#48]
    b730:	aa0003f3 	mov	x19, x0
    b734:	52800021 	mov	w1, #0x1                   	// #1
    b738:	885ffe62 	ldaxr	w2, [x19]
    b73c:	6b1f005f 	cmp	w2, wzr
    b740:	54000061 	b.ne	b74c <pthread_rwlock_timedrdlock+0x38>
    b744:	88037e61 	stxr	w3, w1, [x19]
    b748:	35ffff83 	cbnz	w3, b738 <pthread_rwlock_timedrdlock+0x24>
    b74c:	54000721 	b.ne	b830 <pthread_rwlock_timedrdlock+0x11c>
    b750:	d53bd055 	mrs	x21, tpidr_el0
    b754:	52800017 	mov	w23, #0x0                   	// #0
    b758:	d11bc2b5 	sub	x21, x21, #0x6f0
    b75c:	52801038 	mov	w24, #0x81                  	// #129
    b760:	52803136 	mov	w22, #0x189                 	// #393
    b764:	b9401a60 	ldr	w0, [x19,#24]
    b768:	350000a0 	cbnz	w0, b77c <pthread_rwlock_timedrdlock+0x68>
    b76c:	b9401661 	ldr	w1, [x19,#20]
    b770:	34000f21 	cbz	w1, b954 <pthread_rwlock_timedrdlock+0x240>
    b774:	b9403261 	ldr	w1, [x19,#48]
    b778:	34000ee1 	cbz	w1, b954 <pthread_rwlock_timedrdlock+0x240>
    b77c:	b940d2a1 	ldr	w1, [x21,#208]
    b780:	6b01001f 	cmp	w0, w1
    b784:	54000b00 	b.eq	b8e4 <pthread_rwlock_timedrdlock+0x1d0>
    b788:	f9400681 	ldr	x1, [x20,#8]
    b78c:	d2993fe0 	mov	x0, #0xc9ff                	// #51711
    b790:	f2a77340 	movk	x0, #0x3b9a, lsl #16
    b794:	eb00003f 	cmp	x1, x0
    b798:	54000aa8 	b.hi	b8ec <pthread_rwlock_timedrdlock+0x1d8>
    b79c:	f9400280 	ldr	x0, [x20]
    b7a0:	b7f80760 	tbnz	x0, #63, b88c <pthread_rwlock_timedrdlock+0x178>
    b7a4:	b9401261 	ldr	w1, [x19,#16]
    b7a8:	11000420 	add	w0, w1, #0x1
    b7ac:	b9001260 	str	w0, [x19,#16]
    b7b0:	34000e00 	cbz	w0, b970 <pthread_rwlock_timedrdlock+0x25c>
    b7b4:	b9400a65 	ldr	w5, [x19,#8]
    b7b8:	885f7e60 	ldxr	w0, [x19]
    b7bc:	8801fe77 	stlxr	w1, w23, [x19]
    b7c0:	35ffffc1 	cbnz	w1, b7b8 <pthread_rwlock_timedrdlock+0xa4>
    b7c4:	7100041f 	cmp	w0, #0x1
    b7c8:	5400096c 	b.gt	b8f4 <pthread_rwlock_timedrdlock+0x1e0>
    b7cc:	b9401e64 	ldr	w4, [x19,#28]
    b7d0:	93407ca2 	sxtw	x2, w5
    b7d4:	91002260 	add	x0, x19, #0x8
    b7d8:	aa1403e3 	mov	x3, x20
    b7dc:	4a160081 	eor	w1, w4, w22
    b7e0:	b2407fe5 	mov	x5, #0xffffffff            	// #4294967295
    b7e4:	d2800004 	mov	x4, #0x0                   	// #0
    b7e8:	d2800c48 	mov	x8, #0x62                  	// #98
    b7ec:	93407c21 	sxtw	x1, w1
    b7f0:	d4000001 	svc	#0x0
    b7f4:	b9005fa4 	str	w4, [x29,#92]
    b7f8:	b140041f 	cmn	x0, #0x1, lsl #12
    b7fc:	54000228 	b.hi	b840 <pthread_rwlock_timedrdlock+0x12c>
    b800:	2a0403e0 	mov	w0, w4
    b804:	52800021 	mov	w1, #0x1                   	// #1
    b808:	885ffe62 	ldaxr	w2, [x19]
    b80c:	6b00005f 	cmp	w2, w0
    b810:	54000061 	b.ne	b81c <pthread_rwlock_timedrdlock+0x108>
    b814:	88037e61 	stxr	w3, w1, [x19]
    b818:	35ffff83 	cbnz	w3, b808 <pthread_rwlock_timedrdlock+0xf4>
    b81c:	54000601 	b.ne	b8dc <pthread_rwlock_timedrdlock+0x1c8>
    b820:	b9401260 	ldr	w0, [x19,#16]
    b824:	51000400 	sub	w0, w0, #0x1
    b828:	b9001260 	str	w0, [x19,#16]
    b82c:	17ffffce 	b	b764 <pthread_rwlock_timedrdlock+0x50>
    b830:	b9401e61 	ldr	w1, [x19,#28]
    b834:	b9005fa2 	str	w2, [x29,#92]
    b838:	94000fe3 	bl	f7c4 <__lll_lock_wait>
    b83c:	17ffffc5 	b	b750 <pthread_rwlock_timedrdlock+0x3c>
    b840:	52800021 	mov	w1, #0x1                   	// #1
    b844:	885ffe62 	ldaxr	w2, [x19]
    b848:	6b1f005f 	cmp	w2, wzr
    b84c:	54000061 	b.ne	b858 <pthread_rwlock_timedrdlock+0x144>
    b850:	88037e61 	stxr	w3, w1, [x19]
    b854:	35ffff83 	cbnz	w3, b844 <pthread_rwlock_timedrdlock+0x130>
    b858:	54000340 	b.eq	b8c0 <pthread_rwlock_timedrdlock+0x1ac>
    b85c:	b9005fa2 	str	w2, [x29,#92]
    b860:	aa0003e4 	mov	x4, x0
    b864:	b9401e61 	ldr	w1, [x19,#28]
    b868:	aa1303e0 	mov	x0, x19
    b86c:	f90027a4 	str	x4, [x29,#72]
    b870:	94000fd5 	bl	f7c4 <__lll_lock_wait>
    b874:	b9401260 	ldr	w0, [x19,#16]
    b878:	f94027a4 	ldr	x4, [x29,#72]
    b87c:	51000400 	sub	w0, w0, #0x1
    b880:	b9001260 	str	w0, [x19,#16]
    b884:	b101b89f 	cmn	x4, #0x6e
    b888:	54fff6e1 	b.ne	b764 <pthread_rwlock_timedrdlock+0x50>
    b88c:	52800dc4 	mov	w4, #0x6e                  	// #110
    b890:	52800001 	mov	w1, #0x0                   	// #0
    b894:	885f7e60 	ldxr	w0, [x19]
    b898:	8802fe61 	stlxr	w2, w1, [x19]
    b89c:	35ffffc2 	cbnz	w2, b894 <pthread_rwlock_timedrdlock+0x180>
    b8a0:	7100041f 	cmp	w0, #0x1
    b8a4:	540003ac 	b.gt	b918 <pthread_rwlock_timedrdlock+0x204>
    b8a8:	2a0403e0 	mov	w0, w4
    b8ac:	a94153f3 	ldp	x19, x20, [sp,#16]
    b8b0:	a9425bf5 	ldp	x21, x22, [sp,#32]
    b8b4:	a94363f7 	ldp	x23, x24, [sp,#48]
    b8b8:	a8c67bfd 	ldp	x29, x30, [sp],#96
    b8bc:	d65f03c0 	ret
    b8c0:	aa0003e4 	mov	x4, x0
    b8c4:	b9401260 	ldr	w0, [x19,#16]
    b8c8:	b101b89f 	cmn	x4, #0x6e
    b8cc:	51000400 	sub	w0, w0, #0x1
    b8d0:	b9001260 	str	w0, [x19,#16]
    b8d4:	54fff481 	b.ne	b764 <pthread_rwlock_timedrdlock+0x50>
    b8d8:	17ffffed 	b	b88c <pthread_rwlock_timedrdlock+0x178>
    b8dc:	b9005fa2 	str	w2, [x29,#92]
    b8e0:	17ffffe1 	b	b864 <pthread_rwlock_timedrdlock+0x150>
    b8e4:	52800464 	mov	w4, #0x23                  	// #35
    b8e8:	17ffffea 	b	b890 <pthread_rwlock_timedrdlock+0x17c>
    b8ec:	528002c4 	mov	w4, #0x16                  	// #22
    b8f0:	17ffffe8 	b	b890 <pthread_rwlock_timedrdlock+0x17c>
    b8f4:	b9401e61 	ldr	w1, [x19,#28]
    b8f8:	aa1303e0 	mov	x0, x19
    b8fc:	d2800022 	mov	x2, #0x1                   	// #1
    b900:	d2800003 	mov	x3, #0x0                   	// #0
    b904:	4a180021 	eor	w1, w1, w24
    b908:	d2800c48 	mov	x8, #0x62                  	// #98
    b90c:	93407c21 	sxtw	x1, w1
    b910:	d4000001 	svc	#0x0
    b914:	17ffffae 	b	b7cc <pthread_rwlock_timedrdlock+0xb8>
    b918:	b9401e63 	ldr	w3, [x19,#28]
    b91c:	52801021 	mov	w1, #0x81                  	// #129
    b920:	aa1303e0 	mov	x0, x19
    b924:	d2800022 	mov	x2, #0x1                   	// #1
    b928:	4a010061 	eor	w1, w3, w1
    b92c:	d2800c48 	mov	x8, #0x62                  	// #98
    b930:	d2800003 	mov	x3, #0x0                   	// #0
    b934:	93407c21 	sxtw	x1, w1
    b938:	d4000001 	svc	#0x0
    b93c:	2a0403e0 	mov	w0, w4
    b940:	a94153f3 	ldp	x19, x20, [sp,#16]
    b944:	a9425bf5 	ldp	x21, x22, [sp,#32]
    b948:	a94363f7 	ldp	x23, x24, [sp,#48]
    b94c:	a8c67bfd 	ldp	x29, x30, [sp],#96
    b950:	d65f03c0 	ret
    b954:	b9400660 	ldr	w0, [x19,#4]
    b958:	52800164 	mov	w4, #0xb                   	// #11
    b95c:	31000400 	adds	w0, w0, #0x1
    b960:	54fff980 	b.eq	b890 <pthread_rwlock_timedrdlock+0x17c>
    b964:	b9000660 	str	w0, [x19,#4]
    b968:	52800004 	mov	w4, #0x0                   	// #0
    b96c:	17ffffc9 	b	b890 <pthread_rwlock_timedrdlock+0x17c>
    b970:	b9001261 	str	w1, [x19,#16]
    b974:	52800164 	mov	w4, #0xb                   	// #11
    b978:	17ffffc6 	b	b890 <pthread_rwlock_timedrdlock+0x17c>

000000000000b97c <__pthread_rwlock_wrlock_slow>:
    b97c:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    b980:	910003fd 	mov	x29, sp
    b984:	a90153f3 	stp	x19, x20, [sp,#16]
    b988:	a9025bf5 	stp	x21, x22, [sp,#32]
    b98c:	f9001bf7 	str	x23, [sp,#48]
    b990:	d53bd054 	mrs	x20, tpidr_el0
    b994:	aa0003f3 	mov	x19, x0
    b998:	d11bc294 	sub	x20, x20, #0x6f0
    b99c:	b9401801 	ldr	w1, [x0,#24]
    b9a0:	52800016 	mov	w22, #0x0                   	// #0
    b9a4:	52801037 	mov	w23, #0x81                  	// #129
    b9a8:	52800035 	mov	w21, #0x1                   	// #1
    b9ac:	b940d280 	ldr	w0, [x20,#208]
    b9b0:	6b01001f 	cmp	w0, w1
    b9b4:	54000680 	b.eq	ba84 <__pthread_rwlock_wrlock_slow+0x108>
    b9b8:	b9401661 	ldr	w1, [x19,#20]
    b9bc:	11000420 	add	w0, w1, #0x1
    b9c0:	b9001660 	str	w0, [x19,#20]
    b9c4:	34000760 	cbz	w0, bab0 <__pthread_rwlock_wrlock_slow+0x134>
    b9c8:	b9400e64 	ldr	w4, [x19,#12]
    b9cc:	885f7e60 	ldxr	w0, [x19]
    b9d0:	8801fe76 	stlxr	w1, w22, [x19]
    b9d4:	35ffffc1 	cbnz	w1, b9cc <__pthread_rwlock_wrlock_slow+0x50>
    b9d8:	7100041f 	cmp	w0, #0x1
    b9dc:	5400058c 	b.gt	ba8c <__pthread_rwlock_wrlock_slow+0x110>
    b9e0:	b9401e61 	ldr	w1, [x19,#28]
    b9e4:	93407c82 	sxtw	x2, w4
    b9e8:	91003260 	add	x0, x19, #0xc
    b9ec:	d2800003 	mov	x3, #0x0                   	// #0
    b9f0:	52190021 	eor	w1, w1, #0x80
    b9f4:	d2800c48 	mov	x8, #0x62                  	// #98
    b9f8:	93407c21 	sxtw	x1, w1
    b9fc:	d4000001 	svc	#0x0
    ba00:	b9004fa3 	str	w3, [x29,#76]
    ba04:	885ffe62 	ldaxr	w2, [x19]
    ba08:	6b1f005f 	cmp	w2, wzr
    ba0c:	54000061 	b.ne	ba18 <__pthread_rwlock_wrlock_slow+0x9c>
    ba10:	88007e75 	stxr	w0, w21, [x19]
    ba14:	35ffff80 	cbnz	w0, ba04 <__pthread_rwlock_wrlock_slow+0x88>
    ba18:	aa1303e0 	mov	x0, x19
    ba1c:	54000080 	b.eq	ba2c <__pthread_rwlock_wrlock_slow+0xb0>
    ba20:	b9401e61 	ldr	w1, [x19,#28]
    ba24:	b9004fa2 	str	w2, [x29,#76]
    ba28:	94000f67 	bl	f7c4 <__lll_lock_wait>
    ba2c:	b9401660 	ldr	w0, [x19,#20]
    ba30:	b9401a61 	ldr	w1, [x19,#24]
    ba34:	51000400 	sub	w0, w0, #0x1
    ba38:	b9001660 	str	w0, [x19,#20]
    ba3c:	35fffb81 	cbnz	w1, b9ac <__pthread_rwlock_wrlock_slow+0x30>
    ba40:	b9400660 	ldr	w0, [x19,#4]
    ba44:	35fffb40 	cbnz	w0, b9ac <__pthread_rwlock_wrlock_slow+0x30>
    ba48:	b940d281 	ldr	w1, [x20,#208]
    ba4c:	2a0003e4 	mov	w4, w0
    ba50:	b9001a61 	str	w1, [x19,#24]
    ba54:	52800001 	mov	w1, #0x0                   	// #0
    ba58:	885f7e60 	ldxr	w0, [x19]
    ba5c:	8802fe61 	stlxr	w2, w1, [x19]
    ba60:	35ffffc2 	cbnz	w2, ba58 <__pthread_rwlock_wrlock_slow+0xdc>
    ba64:	7100041f 	cmp	w0, #0x1
    ba68:	540002ac 	b.gt	babc <__pthread_rwlock_wrlock_slow+0x140>
    ba6c:	2a0403e0 	mov	w0, w4
    ba70:	f9401bf7 	ldr	x23, [sp,#48]
    ba74:	a94153f3 	ldp	x19, x20, [sp,#16]
    ba78:	a9425bf5 	ldp	x21, x22, [sp,#32]
    ba7c:	a8c57bfd 	ldp	x29, x30, [sp],#80
    ba80:	d65f03c0 	ret
    ba84:	52800464 	mov	w4, #0x23                  	// #35
    ba88:	17fffff3 	b	ba54 <__pthread_rwlock_wrlock_slow+0xd8>
    ba8c:	b9401e61 	ldr	w1, [x19,#28]
    ba90:	aa1303e0 	mov	x0, x19
    ba94:	d2800022 	mov	x2, #0x1                   	// #1
    ba98:	d2800003 	mov	x3, #0x0                   	// #0
    ba9c:	4a170021 	eor	w1, w1, w23
    baa0:	d2800c48 	mov	x8, #0x62                  	// #98
    baa4:	93407c21 	sxtw	x1, w1
    baa8:	d4000001 	svc	#0x0
    baac:	17ffffcd 	b	b9e0 <__pthread_rwlock_wrlock_slow+0x64>
    bab0:	b9001661 	str	w1, [x19,#20]
    bab4:	52800164 	mov	w4, #0xb                   	// #11
    bab8:	17ffffe7 	b	ba54 <__pthread_rwlock_wrlock_slow+0xd8>
    babc:	b9401e63 	ldr	w3, [x19,#28]
    bac0:	52801021 	mov	w1, #0x81                  	// #129
    bac4:	aa1303e0 	mov	x0, x19
    bac8:	d2800022 	mov	x2, #0x1                   	// #1
    bacc:	4a010061 	eor	w1, w3, w1
    bad0:	d2800c48 	mov	x8, #0x62                  	// #98
    bad4:	d2800003 	mov	x3, #0x0                   	// #0
    bad8:	93407c21 	sxtw	x1, w1
    badc:	d4000001 	svc	#0x0
    bae0:	2a0403e0 	mov	w0, w4
    bae4:	f9401bf7 	ldr	x23, [sp,#48]
    bae8:	a94153f3 	ldp	x19, x20, [sp,#16]
    baec:	a9425bf5 	ldp	x21, x22, [sp,#32]
    baf0:	a8c57bfd 	ldp	x29, x30, [sp],#80
    baf4:	d65f03c0 	ret

000000000000baf8 <__pthread_rwlock_wrlock>:
    baf8:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    bafc:	52800021 	mov	w1, #0x1                   	// #1
    bb00:	910003fd 	mov	x29, sp
    bb04:	f9000bf3 	str	x19, [sp,#16]
    bb08:	aa0003f3 	mov	x19, x0
    bb0c:	b9002fbf 	str	wzr, [x29,#44]
    bb10:	885ffe62 	ldaxr	w2, [x19]
    bb14:	6b1f005f 	cmp	w2, wzr
    bb18:	54000061 	b.ne	bb24 <__pthread_rwlock_wrlock+0x2c>
    bb1c:	88037e61 	stxr	w3, w1, [x19]
    bb20:	35ffff83 	cbnz	w3, bb10 <__pthread_rwlock_wrlock+0x18>
    bb24:	54000261 	b.ne	bb70 <__pthread_rwlock_wrlock+0x78>
    bb28:	b9401a61 	ldr	w1, [x19,#24]
    bb2c:	b9400660 	ldr	w0, [x19,#4]
    bb30:	2a000020 	orr	w0, w1, w0
    bb34:	350002c0 	cbnz	w0, bb8c <__pthread_rwlock_wrlock+0x94>
    bb38:	d53bd041 	mrs	x1, tpidr_el0
    bb3c:	d11bc021 	sub	x1, x1, #0x6f0
    bb40:	b940d021 	ldr	w1, [x1,#208]
    bb44:	b9001a61 	str	w1, [x19,#24]
    bb48:	885f7e61 	ldxr	w1, [x19]
    bb4c:	8802fe60 	stlxr	w2, w0, [x19]
    bb50:	35ffffc2 	cbnz	w2, bb48 <__pthread_rwlock_wrlock+0x50>
    bb54:	7100043f 	cmp	w1, #0x1
    bb58:	2a0003e4 	mov	w4, w0
    bb5c:	5400026c 	b.gt	bba8 <__pthread_rwlock_wrlock+0xb0>
    bb60:	2a0403e0 	mov	w0, w4
    bb64:	f9400bf3 	ldr	x19, [sp,#16]
    bb68:	a8c37bfd 	ldp	x29, x30, [sp],#48
    bb6c:	d65f03c0 	ret
    bb70:	b9401e61 	ldr	w1, [x19,#28]
    bb74:	b9002fa2 	str	w2, [x29,#44]
    bb78:	94000f13 	bl	f7c4 <__lll_lock_wait>
    bb7c:	b9401a61 	ldr	w1, [x19,#24]
    bb80:	b9400660 	ldr	w0, [x19,#4]
    bb84:	2a000020 	orr	w0, w1, w0
    bb88:	34fffd80 	cbz	w0, bb38 <__pthread_rwlock_wrlock+0x40>
    bb8c:	aa1303e0 	mov	x0, x19
    bb90:	97ffff7b 	bl	b97c <__pthread_rwlock_wrlock_slow>
    bb94:	2a0003e4 	mov	w4, w0
    bb98:	2a0403e0 	mov	w0, w4
    bb9c:	f9400bf3 	ldr	x19, [sp,#16]
    bba0:	a8c37bfd 	ldp	x29, x30, [sp],#48
    bba4:	d65f03c0 	ret
    bba8:	b9401e63 	ldr	w3, [x19,#28]
    bbac:	52801021 	mov	w1, #0x81                  	// #129
    bbb0:	aa1303e0 	mov	x0, x19
    bbb4:	d2800022 	mov	x2, #0x1                   	// #1
    bbb8:	4a010061 	eor	w1, w3, w1
    bbbc:	d2800c48 	mov	x8, #0x62                  	// #98
    bbc0:	d2800003 	mov	x3, #0x0                   	// #0
    bbc4:	93407c21 	sxtw	x1, w1
    bbc8:	d4000001 	svc	#0x0
    bbcc:	17ffffe5 	b	bb60 <__pthread_rwlock_wrlock+0x68>

000000000000bbd0 <pthread_rwlock_timedwrlock>:
    bbd0:	a9b97bfd 	stp	x29, x30, [sp,#-112]!
    bbd4:	910003fd 	mov	x29, sp
    bbd8:	a90153f3 	stp	x19, x20, [sp,#16]
    bbdc:	a9025bf5 	stp	x21, x22, [sp,#32]
    bbe0:	aa0103f4 	mov	x20, x1
    bbe4:	f90023f9 	str	x25, [sp,#64]
    bbe8:	a90363f7 	stp	x23, x24, [sp,#48]
    bbec:	b9006fbf 	str	wzr, [x29,#108]
    bbf0:	aa0003f3 	mov	x19, x0
    bbf4:	52800021 	mov	w1, #0x1                   	// #1
    bbf8:	885ffe62 	ldaxr	w2, [x19]
    bbfc:	6b1f005f 	cmp	w2, wzr
    bc00:	54000061 	b.ne	bc0c <pthread_rwlock_timedwrlock+0x3c>
    bc04:	88037e61 	stxr	w3, w1, [x19]
    bc08:	35ffff83 	cbnz	w3, bbf8 <pthread_rwlock_timedwrlock+0x28>
    bc0c:	540006e1 	b.ne	bce8 <pthread_rwlock_timedwrlock+0x118>
    bc10:	d53bd055 	mrs	x21, tpidr_el0
    bc14:	52800018 	mov	w24, #0x0                   	// #0
    bc18:	d11bc2b5 	sub	x21, x21, #0x6f0
    bc1c:	52801039 	mov	w25, #0x81                  	// #129
    bc20:	52803137 	mov	w23, #0x189                 	// #393
    bc24:	52800036 	mov	w22, #0x1                   	// #1
    bc28:	b9401a62 	ldr	w2, [x19,#24]
    bc2c:	35000062 	cbnz	w2, bc38 <pthread_rwlock_timedwrlock+0x68>
    bc30:	b9400660 	ldr	w0, [x19,#4]
    bc34:	34000f40 	cbz	w0, be1c <pthread_rwlock_timedwrlock+0x24c>
    bc38:	b940d2a0 	ldr	w0, [x21,#208]
    bc3c:	6b00005f 	cmp	w2, w0
    bc40:	54000ae0 	b.eq	bd9c <pthread_rwlock_timedwrlock+0x1cc>
    bc44:	f9400681 	ldr	x1, [x20,#8]
    bc48:	d2993fe0 	mov	x0, #0xc9ff                	// #51711
    bc4c:	f2a77340 	movk	x0, #0x3b9a, lsl #16
    bc50:	eb00003f 	cmp	x1, x0
    bc54:	54000a88 	b.hi	bda4 <pthread_rwlock_timedwrlock+0x1d4>
    bc58:	f9400280 	ldr	x0, [x20]
    bc5c:	b7f80720 	tbnz	x0, #63, bd40 <pthread_rwlock_timedwrlock+0x170>
    bc60:	b9401661 	ldr	w1, [x19,#20]
    bc64:	11000420 	add	w0, w1, #0x1
    bc68:	b9001660 	str	w0, [x19,#20]
    bc6c:	34000d20 	cbz	w0, be10 <pthread_rwlock_timedwrlock+0x240>
    bc70:	b9400e65 	ldr	w5, [x19,#12]
    bc74:	885f7e60 	ldxr	w0, [x19]
    bc78:	8801fe78 	stlxr	w1, w24, [x19]
    bc7c:	35ffffc1 	cbnz	w1, bc74 <pthread_rwlock_timedwrlock+0xa4>
    bc80:	7100041f 	cmp	w0, #0x1
    bc84:	5400094c 	b.gt	bdac <pthread_rwlock_timedwrlock+0x1dc>
    bc88:	b9401e64 	ldr	w4, [x19,#28]
    bc8c:	93407ca2 	sxtw	x2, w5
    bc90:	91003260 	add	x0, x19, #0xc
    bc94:	aa1403e3 	mov	x3, x20
    bc98:	4a170081 	eor	w1, w4, w23
    bc9c:	b2407fe5 	mov	x5, #0xffffffff            	// #4294967295
    bca0:	d2800004 	mov	x4, #0x0                   	// #0
    bca4:	d2800c48 	mov	x8, #0x62                  	// #98
    bca8:	93407c21 	sxtw	x1, w1
    bcac:	d4000001 	svc	#0x0
    bcb0:	b9006fa4 	str	w4, [x29,#108]
    bcb4:	b140041f 	cmn	x0, #0x1, lsl #12
    bcb8:	54000208 	b.hi	bcf8 <pthread_rwlock_timedwrlock+0x128>
    bcbc:	2a0403e0 	mov	w0, w4
    bcc0:	885ffe61 	ldaxr	w1, [x19]
    bcc4:	6b00003f 	cmp	w1, w0
    bcc8:	54000061 	b.ne	bcd4 <pthread_rwlock_timedwrlock+0x104>
    bccc:	88027e76 	stxr	w2, w22, [x19]
    bcd0:	35ffff82 	cbnz	w2, bcc0 <pthread_rwlock_timedwrlock+0xf0>
    bcd4:	54000601 	b.ne	bd94 <pthread_rwlock_timedwrlock+0x1c4>
    bcd8:	b9401660 	ldr	w0, [x19,#20]
    bcdc:	51000400 	sub	w0, w0, #0x1
    bce0:	b9001660 	str	w0, [x19,#20]
    bce4:	17ffffd1 	b	bc28 <pthread_rwlock_timedwrlock+0x58>
    bce8:	b9401e61 	ldr	w1, [x19,#28]
    bcec:	b9006fa2 	str	w2, [x29,#108]
    bcf0:	94000eb5 	bl	f7c4 <__lll_lock_wait>
    bcf4:	17ffffc7 	b	bc10 <pthread_rwlock_timedwrlock+0x40>
    bcf8:	885ffe61 	ldaxr	w1, [x19]
    bcfc:	6b1f003f 	cmp	w1, wzr
    bd00:	54000061 	b.ne	bd0c <pthread_rwlock_timedwrlock+0x13c>
    bd04:	88027e76 	stxr	w2, w22, [x19]
    bd08:	35ffff82 	cbnz	w2, bcf8 <pthread_rwlock_timedwrlock+0x128>
    bd0c:	54000360 	b.eq	bd78 <pthread_rwlock_timedwrlock+0x1a8>
    bd10:	b9006fa1 	str	w1, [x29,#108]
    bd14:	aa0003e4 	mov	x4, x0
    bd18:	b9401e61 	ldr	w1, [x19,#28]
    bd1c:	aa1303e0 	mov	x0, x19
    bd20:	f9002fa4 	str	x4, [x29,#88]
    bd24:	94000ea8 	bl	f7c4 <__lll_lock_wait>
    bd28:	b9401660 	ldr	w0, [x19,#20]
    bd2c:	f9402fa4 	ldr	x4, [x29,#88]
    bd30:	51000400 	sub	w0, w0, #0x1
    bd34:	b9001660 	str	w0, [x19,#20]
    bd38:	b101b89f 	cmn	x4, #0x6e
    bd3c:	54fff761 	b.ne	bc28 <pthread_rwlock_timedwrlock+0x58>
    bd40:	52800dc4 	mov	w4, #0x6e                  	// #110
    bd44:	52800001 	mov	w1, #0x0                   	// #0
    bd48:	885f7e60 	ldxr	w0, [x19]
    bd4c:	8802fe61 	stlxr	w2, w1, [x19]
    bd50:	35ffffc2 	cbnz	w2, bd48 <pthread_rwlock_timedwrlock+0x178>
    bd54:	7100041f 	cmp	w0, #0x1
    bd58:	540003cc 	b.gt	bdd0 <pthread_rwlock_timedwrlock+0x200>
    bd5c:	2a0403e0 	mov	w0, w4
    bd60:	f94023f9 	ldr	x25, [sp,#64]
    bd64:	a94153f3 	ldp	x19, x20, [sp,#16]
    bd68:	a9425bf5 	ldp	x21, x22, [sp,#32]
    bd6c:	a94363f7 	ldp	x23, x24, [sp,#48]
    bd70:	a8c77bfd 	ldp	x29, x30, [sp],#112
    bd74:	d65f03c0 	ret
    bd78:	aa0003e4 	mov	x4, x0
    bd7c:	b9401660 	ldr	w0, [x19,#20]
    bd80:	b101b89f 	cmn	x4, #0x6e
    bd84:	51000400 	sub	w0, w0, #0x1
    bd88:	b9001660 	str	w0, [x19,#20]
    bd8c:	54fff4e1 	b.ne	bc28 <pthread_rwlock_timedwrlock+0x58>
    bd90:	17ffffec 	b	bd40 <pthread_rwlock_timedwrlock+0x170>
    bd94:	b9006fa1 	str	w1, [x29,#108]
    bd98:	17ffffe0 	b	bd18 <pthread_rwlock_timedwrlock+0x148>
    bd9c:	52800464 	mov	w4, #0x23                  	// #35
    bda0:	17ffffe9 	b	bd44 <pthread_rwlock_timedwrlock+0x174>
    bda4:	528002c4 	mov	w4, #0x16                  	// #22
    bda8:	17ffffe7 	b	bd44 <pthread_rwlock_timedwrlock+0x174>
    bdac:	b9401e61 	ldr	w1, [x19,#28]
    bdb0:	aa1303e0 	mov	x0, x19
    bdb4:	d2800022 	mov	x2, #0x1                   	// #1
    bdb8:	d2800003 	mov	x3, #0x0                   	// #0
    bdbc:	4a190021 	eor	w1, w1, w25
    bdc0:	d2800c48 	mov	x8, #0x62                  	// #98
    bdc4:	93407c21 	sxtw	x1, w1
    bdc8:	d4000001 	svc	#0x0
    bdcc:	17ffffaf 	b	bc88 <pthread_rwlock_timedwrlock+0xb8>
    bdd0:	b9401e63 	ldr	w3, [x19,#28]
    bdd4:	52801021 	mov	w1, #0x81                  	// #129
    bdd8:	aa1303e0 	mov	x0, x19
    bddc:	d2800022 	mov	x2, #0x1                   	// #1
    bde0:	4a010061 	eor	w1, w3, w1
    bde4:	d2800c48 	mov	x8, #0x62                  	// #98
    bde8:	d2800003 	mov	x3, #0x0                   	// #0
    bdec:	93407c21 	sxtw	x1, w1
    bdf0:	d4000001 	svc	#0x0
    bdf4:	2a0403e0 	mov	w0, w4
    bdf8:	f94023f9 	ldr	x25, [sp,#64]
    bdfc:	a94153f3 	ldp	x19, x20, [sp,#16]
    be00:	a9425bf5 	ldp	x21, x22, [sp,#32]
    be04:	a94363f7 	ldp	x23, x24, [sp,#48]
    be08:	a8c77bfd 	ldp	x29, x30, [sp],#112
    be0c:	d65f03c0 	ret
    be10:	b9001661 	str	w1, [x19,#20]
    be14:	52800164 	mov	w4, #0xb                   	// #11
    be18:	17ffffcb 	b	bd44 <pthread_rwlock_timedwrlock+0x174>
    be1c:	b940d2a1 	ldr	w1, [x21,#208]
    be20:	2a0003e4 	mov	w4, w0
    be24:	b9001a61 	str	w1, [x19,#24]
    be28:	17ffffc7 	b	bd44 <pthread_rwlock_timedwrlock+0x174>

000000000000be2c <__pthread_rwlock_tryrdlock>:
    be2c:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    be30:	52800021 	mov	w1, #0x1                   	// #1
    be34:	910003fd 	mov	x29, sp
    be38:	f9000bf3 	str	x19, [sp,#16]
    be3c:	aa0003f3 	mov	x19, x0
    be40:	b9002fbf 	str	wzr, [x29,#44]
    be44:	885ffe62 	ldaxr	w2, [x19]
    be48:	6b1f005f 	cmp	w2, wzr
    be4c:	54000061 	b.ne	be58 <__pthread_rwlock_tryrdlock+0x2c>
    be50:	88037e61 	stxr	w3, w1, [x19]
    be54:	35ffff83 	cbnz	w3, be44 <__pthread_rwlock_tryrdlock+0x18>
    be58:	540001c1 	b.ne	be90 <__pthread_rwlock_tryrdlock+0x64>
    be5c:	b9401a60 	ldr	w0, [x19,#24]
    be60:	52800204 	mov	w4, #0x10                  	// #16
    be64:	34000220 	cbz	w0, bea8 <__pthread_rwlock_tryrdlock+0x7c>
    be68:	52800001 	mov	w1, #0x0                   	// #0
    be6c:	885f7e60 	ldxr	w0, [x19]
    be70:	8802fe61 	stlxr	w2, w1, [x19]
    be74:	35ffffc2 	cbnz	w2, be6c <__pthread_rwlock_tryrdlock+0x40>
    be78:	7100041f 	cmp	w0, #0x1
    be7c:	5400030c 	b.gt	bedc <__pthread_rwlock_tryrdlock+0xb0>
    be80:	2a0403e0 	mov	w0, w4
    be84:	f9400bf3 	ldr	x19, [sp,#16]
    be88:	a8c37bfd 	ldp	x29, x30, [sp],#48
    be8c:	d65f03c0 	ret
    be90:	b9401e61 	ldr	w1, [x19,#28]
    be94:	b9002fa2 	str	w2, [x29,#44]
    be98:	94000e4b 	bl	f7c4 <__lll_lock_wait>
    be9c:	b9401a60 	ldr	w0, [x19,#24]
    bea0:	52800204 	mov	w4, #0x10                  	// #16
    bea4:	35fffe20 	cbnz	w0, be68 <__pthread_rwlock_tryrdlock+0x3c>
    bea8:	b9401660 	ldr	w0, [x19,#20]
    beac:	35000120 	cbnz	w0, bed0 <__pthread_rwlock_tryrdlock+0xa4>
    beb0:	b9400661 	ldr	w1, [x19,#4]
    beb4:	52800004 	mov	w4, #0x0                   	// #0
    beb8:	11000420 	add	w0, w1, #0x1
    bebc:	b9000660 	str	w0, [x19,#4]
    bec0:	35fffd40 	cbnz	w0, be68 <__pthread_rwlock_tryrdlock+0x3c>
    bec4:	b9000661 	str	w1, [x19,#4]
    bec8:	52800164 	mov	w4, #0xb                   	// #11
    becc:	17ffffe7 	b	be68 <__pthread_rwlock_tryrdlock+0x3c>
    bed0:	b9403260 	ldr	w0, [x19,#48]
    bed4:	35fffca0 	cbnz	w0, be68 <__pthread_rwlock_tryrdlock+0x3c>
    bed8:	17fffff6 	b	beb0 <__pthread_rwlock_tryrdlock+0x84>
    bedc:	b9401e63 	ldr	w3, [x19,#28]
    bee0:	52801021 	mov	w1, #0x81                  	// #129
    bee4:	aa1303e0 	mov	x0, x19
    bee8:	d2800022 	mov	x2, #0x1                   	// #1
    beec:	4a010061 	eor	w1, w3, w1
    bef0:	d2800c48 	mov	x8, #0x62                  	// #98
    bef4:	d2800003 	mov	x3, #0x0                   	// #0
    bef8:	93407c21 	sxtw	x1, w1
    befc:	d4000001 	svc	#0x0
    bf00:	2a0403e0 	mov	w0, w4
    bf04:	f9400bf3 	ldr	x19, [sp,#16]
    bf08:	a8c37bfd 	ldp	x29, x30, [sp],#48
    bf0c:	d65f03c0 	ret

000000000000bf10 <__pthread_rwlock_trywrlock>:
    bf10:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    bf14:	52800021 	mov	w1, #0x1                   	// #1
    bf18:	910003fd 	mov	x29, sp
    bf1c:	f9000bf3 	str	x19, [sp,#16]
    bf20:	aa0003f3 	mov	x19, x0
    bf24:	b9002fbf 	str	wzr, [x29,#44]
    bf28:	885ffe62 	ldaxr	w2, [x19]
    bf2c:	6b1f005f 	cmp	w2, wzr
    bf30:	54000061 	b.ne	bf3c <__pthread_rwlock_trywrlock+0x2c>
    bf34:	88037e61 	stxr	w3, w1, [x19]
    bf38:	35ffff83 	cbnz	w3, bf28 <__pthread_rwlock_trywrlock+0x18>
    bf3c:	540001c1 	b.ne	bf74 <__pthread_rwlock_trywrlock+0x64>
    bf40:	b9401a60 	ldr	w0, [x19,#24]
    bf44:	52800204 	mov	w4, #0x10                  	// #16
    bf48:	34000220 	cbz	w0, bf8c <__pthread_rwlock_trywrlock+0x7c>
    bf4c:	52800001 	mov	w1, #0x0                   	// #0
    bf50:	885f7e60 	ldxr	w0, [x19]
    bf54:	8802fe61 	stlxr	w2, w1, [x19]
    bf58:	35ffffc2 	cbnz	w2, bf50 <__pthread_rwlock_trywrlock+0x40>
    bf5c:	7100041f 	cmp	w0, #0x1
    bf60:	5400026c 	b.gt	bfac <__pthread_rwlock_trywrlock+0x9c>
    bf64:	2a0403e0 	mov	w0, w4
    bf68:	f9400bf3 	ldr	x19, [sp,#16]
    bf6c:	a8c37bfd 	ldp	x29, x30, [sp],#48
    bf70:	d65f03c0 	ret
    bf74:	b9401e61 	ldr	w1, [x19,#28]
    bf78:	b9002fa2 	str	w2, [x29,#44]
    bf7c:	94000e12 	bl	f7c4 <__lll_lock_wait>
    bf80:	b9401a60 	ldr	w0, [x19,#24]
    bf84:	52800204 	mov	w4, #0x10                  	// #16
    bf88:	35fffe20 	cbnz	w0, bf4c <__pthread_rwlock_trywrlock+0x3c>
    bf8c:	b9400660 	ldr	w0, [x19,#4]
    bf90:	35fffde0 	cbnz	w0, bf4c <__pthread_rwlock_trywrlock+0x3c>
    bf94:	2a0003e4 	mov	w4, w0
    bf98:	d53bd040 	mrs	x0, tpidr_el0
    bf9c:	d11bc000 	sub	x0, x0, #0x6f0
    bfa0:	b940d000 	ldr	w0, [x0,#208]
    bfa4:	b9001a60 	str	w0, [x19,#24]
    bfa8:	17ffffe9 	b	bf4c <__pthread_rwlock_trywrlock+0x3c>
    bfac:	b9401e63 	ldr	w3, [x19,#28]
    bfb0:	52801021 	mov	w1, #0x81                  	// #129
    bfb4:	aa1303e0 	mov	x0, x19
    bfb8:	d2800022 	mov	x2, #0x1                   	// #1
    bfbc:	4a010061 	eor	w1, w3, w1
    bfc0:	d2800c48 	mov	x8, #0x62                  	// #98
    bfc4:	d2800003 	mov	x3, #0x0                   	// #0
    bfc8:	93407c21 	sxtw	x1, w1
    bfcc:	d4000001 	svc	#0x0
    bfd0:	2a0403e0 	mov	w0, w4
    bfd4:	f9400bf3 	ldr	x19, [sp,#16]
    bfd8:	a8c37bfd 	ldp	x29, x30, [sp],#48
    bfdc:	d65f03c0 	ret

000000000000bfe0 <__pthread_rwlock_unlock>:
    bfe0:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    bfe4:	52800021 	mov	w1, #0x1                   	// #1
    bfe8:	910003fd 	mov	x29, sp
    bfec:	f9000bf3 	str	x19, [sp,#16]
    bff0:	aa0003f3 	mov	x19, x0
    bff4:	b9002fbf 	str	wzr, [x29,#44]
    bff8:	885ffe62 	ldaxr	w2, [x19]
    bffc:	6b1f005f 	cmp	w2, wzr
    c000:	54000061 	b.ne	c00c <__pthread_rwlock_unlock+0x2c>
    c004:	88037e61 	stxr	w3, w1, [x19]
    c008:	35ffff83 	cbnz	w3, bff8 <__pthread_rwlock_unlock+0x18>
    c00c:	54000281 	b.ne	c05c <__pthread_rwlock_unlock+0x7c>
    c010:	b9401a60 	ldr	w0, [x19,#24]
    c014:	340002e0 	cbz	w0, c070 <__pthread_rwlock_unlock+0x90>
    c018:	b9400660 	ldr	w0, [x19,#4]
    c01c:	b9001a7f 	str	wzr, [x19,#24]
    c020:	350000a0 	cbnz	w0, c034 <__pthread_rwlock_unlock+0x54>
    c024:	b9401661 	ldr	w1, [x19,#20]
    c028:	350002c1 	cbnz	w1, c080 <__pthread_rwlock_unlock+0xa0>
    c02c:	b9401261 	ldr	w1, [x19,#16]
    c030:	35000521 	cbnz	w1, c0d4 <__pthread_rwlock_unlock+0xf4>
    c034:	52800001 	mov	w1, #0x0                   	// #0
    c038:	885f7e60 	ldxr	w0, [x19]
    c03c:	8802fe61 	stlxr	w2, w1, [x19]
    c040:	35ffffc2 	cbnz	w2, c038 <__pthread_rwlock_unlock+0x58>
    c044:	7100041f 	cmp	w0, #0x1
    c048:	540005ec 	b.gt	c104 <__pthread_rwlock_unlock+0x124>
    c04c:	52800000 	mov	w0, #0x0                   	// #0
    c050:	f9400bf3 	ldr	x19, [sp,#16]
    c054:	a8c37bfd 	ldp	x29, x30, [sp],#48
    c058:	d65f03c0 	ret
    c05c:	b9401e61 	ldr	w1, [x19,#28]
    c060:	b9002fa2 	str	w2, [x29,#44]
    c064:	94000dd8 	bl	f7c4 <__lll_lock_wait>
    c068:	b9401a60 	ldr	w0, [x19,#24]
    c06c:	35fffd60 	cbnz	w0, c018 <__pthread_rwlock_unlock+0x38>
    c070:	b9400660 	ldr	w0, [x19,#4]
    c074:	51000400 	sub	w0, w0, #0x1
    c078:	b9000660 	str	w0, [x19,#4]
    c07c:	17ffffe9 	b	c020 <__pthread_rwlock_unlock+0x40>
    c080:	b9400e61 	ldr	w1, [x19,#12]
    c084:	11000421 	add	w1, w1, #0x1
    c088:	b9000e61 	str	w1, [x19,#12]
    c08c:	885f7e61 	ldxr	w1, [x19]
    c090:	8802fe60 	stlxr	w2, w0, [x19]
    c094:	35ffffc2 	cbnz	w2, c08c <__pthread_rwlock_unlock+0xac>
    c098:	7100043f 	cmp	w1, #0x1
    c09c:	540003ac 	b.gt	c110 <__pthread_rwlock_unlock+0x130>
    c0a0:	b9401e63 	ldr	w3, [x19,#28]
    c0a4:	91003260 	add	x0, x19, #0xc
    c0a8:	d2800022 	mov	x2, #0x1                   	// #1
    c0ac:	52801021 	mov	w1, #0x81                  	// #129
    c0b0:	d2800c48 	mov	x8, #0x62                  	// #98
    c0b4:	4a010061 	eor	w1, w3, w1
    c0b8:	d2800003 	mov	x3, #0x0                   	// #0
    c0bc:	93407c21 	sxtw	x1, w1
    c0c0:	d4000001 	svc	#0x0
    c0c4:	52800000 	mov	w0, #0x0                   	// #0
    c0c8:	f9400bf3 	ldr	x19, [sp,#16]
    c0cc:	a8c37bfd 	ldp	x29, x30, [sp],#48
    c0d0:	d65f03c0 	ret
    c0d4:	b9400a61 	ldr	w1, [x19,#8]
    c0d8:	11000421 	add	w1, w1, #0x1
    c0dc:	b9000a61 	str	w1, [x19,#8]
    c0e0:	885f7e61 	ldxr	w1, [x19]
    c0e4:	8802fe60 	stlxr	w2, w0, [x19]
    c0e8:	35ffffc2 	cbnz	w2, c0e0 <__pthread_rwlock_unlock+0x100>
    c0ec:	7100043f 	cmp	w1, #0x1
    c0f0:	5400024c 	b.gt	c138 <__pthread_rwlock_unlock+0x158>
    c0f4:	b9401e63 	ldr	w3, [x19,#28]
    c0f8:	91002260 	add	x0, x19, #0x8
    c0fc:	b2407be2 	mov	x2, #0x7fffffff            	// #2147483647
    c100:	17ffffeb 	b	c0ac <__pthread_rwlock_unlock+0xcc>
    c104:	b9401e63 	ldr	w3, [x19,#28]
    c108:	aa1303e0 	mov	x0, x19
    c10c:	17ffffe7 	b	c0a8 <__pthread_rwlock_unlock+0xc8>
    c110:	b9401e63 	ldr	w3, [x19,#28]
    c114:	52801021 	mov	w1, #0x81                  	// #129
    c118:	aa1303e0 	mov	x0, x19
    c11c:	d2800022 	mov	x2, #0x1                   	// #1
    c120:	4a010061 	eor	w1, w3, w1
    c124:	d2800c48 	mov	x8, #0x62                  	// #98
    c128:	d2800003 	mov	x3, #0x0                   	// #0
    c12c:	93407c21 	sxtw	x1, w1
    c130:	d4000001 	svc	#0x0
    c134:	17ffffdb 	b	c0a0 <__pthread_rwlock_unlock+0xc0>
    c138:	b9401e63 	ldr	w3, [x19,#28]
    c13c:	52801021 	mov	w1, #0x81                  	// #129
    c140:	aa1303e0 	mov	x0, x19
    c144:	d2800022 	mov	x2, #0x1                   	// #1
    c148:	4a010061 	eor	w1, w3, w1
    c14c:	d2800c48 	mov	x8, #0x62                  	// #98
    c150:	d2800003 	mov	x3, #0x0                   	// #0
    c154:	93407c21 	sxtw	x1, w1
    c158:	d4000001 	svc	#0x0
    c15c:	17ffffe6 	b	c0f4 <__pthread_rwlock_unlock+0x114>

000000000000c160 <pthread_rwlockattr_init>:
    c160:	aa0003e1 	mov	x1, x0
    c164:	52800000 	mov	w0, #0x0                   	// #0
    c168:	b9000020 	str	w0, [x1]
    c16c:	b9000420 	str	w0, [x1,#4]
    c170:	d65f03c0 	ret

000000000000c174 <pthread_rwlockattr_destroy>:
    c174:	52800000 	mov	w0, #0x0                   	// #0
    c178:	d65f03c0 	ret

000000000000c17c <pthread_rwlockattr_getpshared>:
    c17c:	b9400402 	ldr	w2, [x0,#4]
    c180:	52800000 	mov	w0, #0x0                   	// #0
    c184:	b9000022 	str	w2, [x1]
    c188:	d65f03c0 	ret

000000000000c18c <pthread_rwlockattr_setpshared>:
    c18c:	7100043f 	cmp	w1, #0x1
    c190:	54000088 	b.hi	c1a0 <pthread_rwlockattr_setpshared+0x14>
    c194:	b9000401 	str	w1, [x0,#4]
    c198:	52800000 	mov	w0, #0x0                   	// #0
    c19c:	d65f03c0 	ret
    c1a0:	528002c0 	mov	w0, #0x16                  	// #22
    c1a4:	d65f03c0 	ret

000000000000c1a8 <pthread_rwlockattr_getkind_np>:
    c1a8:	b9400002 	ldr	w2, [x0]
    c1ac:	52800000 	mov	w0, #0x0                   	// #0
    c1b0:	b9000022 	str	w2, [x1]
    c1b4:	d65f03c0 	ret

000000000000c1b8 <pthread_rwlockattr_setkind_np>:
    c1b8:	7100083f 	cmp	w1, #0x2
    c1bc:	54000088 	b.hi	c1cc <pthread_rwlockattr_setkind_np+0x14>
    c1c0:	b9000001 	str	w1, [x0]
    c1c4:	52800000 	mov	w0, #0x0                   	// #0
    c1c8:	d65f03c0 	ret
    c1cc:	528002c0 	mov	w0, #0x16                  	// #22
    c1d0:	d65f03c0 	ret

000000000000c1d4 <pthread_cond_init@@GLIBC_2.17>:
    c1d4:	b900001f 	str	wzr, [x0]
    c1d8:	b900041f 	str	wzr, [x0,#4]
    c1dc:	b40001e1 	cbz	x1, c218 <pthread_cond_init@@GLIBC_2.17+0x44>
    c1e0:	b9400022 	ldr	w2, [x1]
    c1e4:	f900041f 	str	xzr, [x0,#8]
    c1e8:	f900081f 	str	xzr, [x0,#16]
    c1ec:	d3410442 	ubfx	x2, x2, #1, #1
    c1f0:	f9000c1f 	str	xzr, [x0,#24]
    c1f4:	b9002802 	str	w2, [x0,#40]
    c1f8:	b9400021 	ldr	w1, [x1]
    c1fc:	b9002c1f 	str	wzr, [x0,#44]
    c200:	12000021 	and	w1, w1, #0x1
    c204:	6b1f003f 	cmp	w1, wzr
    c208:	da9f03e2 	csetm	x2, ne
    c20c:	f9001002 	str	x2, [x0,#32]
    c210:	52800000 	mov	w0, #0x0                   	// #0
    c214:	d65f03c0 	ret
    c218:	b9002801 	str	w1, [x0,#40]
    c21c:	aa0103e2 	mov	x2, x1
    c220:	f9000401 	str	x1, [x0,#8]
    c224:	f9000801 	str	x1, [x0,#16]
    c228:	f9000c01 	str	x1, [x0,#24]
    c22c:	f9001002 	str	x2, [x0,#32]
    c230:	b9002c1f 	str	wzr, [x0,#44]
    c234:	52800000 	mov	w0, #0x0                   	// #0
    c238:	d65f03c0 	ret

000000000000c23c <pthread_cond_destroy@@GLIBC_2.17>:
    c23c:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    c240:	910003fd 	mov	x29, sp
    c244:	a90153f3 	stp	x19, x20, [sp,#16]
    c248:	a9025bf5 	stp	x21, x22, [sp,#32]
    c24c:	a90363f7 	stp	x23, x24, [sp,#48]
    c250:	a9046bf9 	stp	x25, x26, [sp,#64]
    c254:	aa0003f3 	mov	x19, x0
    c258:	b9005fbf 	str	wzr, [x29,#92]
    c25c:	f9401000 	ldr	x0, [x0,#32]
    c260:	b100041f 	cmn	x0, #0x1
    c264:	52800020 	mov	w0, #0x1                   	// #1
    c268:	54000d00 	b.eq	c408 <pthread_cond_destroy@@GLIBC_2.17+0x1cc>
    c26c:	885ffe61 	ldaxr	w1, [x19]
    c270:	6b1f003f 	cmp	w1, wzr
    c274:	54000061 	b.ne	c280 <pthread_cond_destroy@@GLIBC_2.17+0x44>
    c278:	88027e60 	stxr	w2, w0, [x19]
    c27c:	35ffff82 	cbnz	w2, c26c <pthread_cond_destroy@@GLIBC_2.17+0x30>
    c280:	1a9f17e0 	cset	w0, eq
    c284:	52800014 	mov	w20, #0x0                   	// #0
    c288:	34000700 	cbz	w0, c368 <pthread_cond_destroy@@GLIBC_2.17+0x12c>
    c28c:	f9400661 	ldr	x1, [x19,#8]
    c290:	f9400a60 	ldr	x0, [x19,#16]
    c294:	eb00003f 	cmp	x1, x0
    c298:	54000988 	b.hi	c3c8 <pthread_cond_destroy@@GLIBC_2.17+0x18c>
    c29c:	b9402a64 	ldr	w4, [x19,#40]
    c2a0:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    c2a4:	f9000660 	str	x0, [x19,#8]
    c2a8:	7100049f 	cmp	w4, #0x1
    c2ac:	540006a9 	b.ls	c380 <pthread_cond_destroy@@GLIBC_2.17+0x144>
    c2b0:	f9401260 	ldr	x0, [x19,#32]
    c2b4:	d1000401 	sub	x1, x0, #0x1
    c2b8:	b1000c3f 	cmn	x1, #0x3
    c2bc:	54000729 	b.ls	c3a0 <pthread_cond_destroy@@GLIBC_2.17+0x164>
    c2c0:	5280103a 	mov	w26, #0x81                  	// #129
    c2c4:	52190295 	eor	w21, w20, #0x80
    c2c8:	4a1a029a 	eor	w26, w20, w26
    c2cc:	9100a278 	add	x24, x19, #0x28
    c2d0:	93407eb5 	sxtw	x21, w21
    c2d4:	910173b9 	add	x25, x29, #0x5c
    c2d8:	52800017 	mov	w23, #0x0                   	// #0
    c2dc:	93407f5a 	sxtw	x26, w26
    c2e0:	52800036 	mov	w22, #0x1                   	// #1
    c2e4:	14000015 	b	c338 <pthread_cond_destroy@@GLIBC_2.17+0xfc>
    c2e8:	2a0403e2 	mov	w2, w4
    c2ec:	aa1803e0 	mov	x0, x24
    c2f0:	aa1503e1 	mov	x1, x21
    c2f4:	d2800003 	mov	x3, #0x0                   	// #0
    c2f8:	d2800c48 	mov	x8, #0x62                  	// #98
    c2fc:	d4000001 	svc	#0x0
    c300:	b9005fa3 	str	w3, [x29,#92]
    c304:	885ffe62 	ldaxr	w2, [x19]
    c308:	6b1f005f 	cmp	w2, wzr
    c30c:	54000061 	b.ne	c318 <pthread_cond_destroy@@GLIBC_2.17+0xdc>
    c310:	88007e76 	stxr	w0, w22, [x19]
    c314:	35ffff80 	cbnz	w0, c304 <pthread_cond_destroy@@GLIBC_2.17+0xc8>
    c318:	aa1303e0 	mov	x0, x19
    c31c:	2a1403e1 	mov	w1, w20
    c320:	54000060 	b.eq	c32c <pthread_cond_destroy@@GLIBC_2.17+0xf0>
    c324:	b9000322 	str	w2, [x25]
    c328:	94000d27 	bl	f7c4 <__lll_lock_wait>
    c32c:	b9402a64 	ldr	w4, [x19,#40]
    c330:	7100049f 	cmp	w4, #0x1
    c334:	54000269 	b.ls	c380 <pthread_cond_destroy@@GLIBC_2.17+0x144>
    c338:	885f7e62 	ldxr	w2, [x19]
    c33c:	8800fe77 	stlxr	w0, w23, [x19]
    c340:	35ffffc0 	cbnz	w0, c338 <pthread_cond_destroy@@GLIBC_2.17+0xfc>
    c344:	7100045f 	cmp	w2, #0x1
    c348:	54fffd0d 	b.le	c2e8 <pthread_cond_destroy@@GLIBC_2.17+0xac>
    c34c:	aa1303e0 	mov	x0, x19
    c350:	aa1a03e1 	mov	x1, x26
    c354:	d2800022 	mov	x2, #0x1                   	// #1
    c358:	d2800003 	mov	x3, #0x0                   	// #0
    c35c:	d2800c48 	mov	x8, #0x62                  	// #98
    c360:	d4000001 	svc	#0x0
    c364:	17ffffe1 	b	c2e8 <pthread_cond_destroy@@GLIBC_2.17+0xac>
    c368:	b9005fa1 	str	w1, [x29,#92]
    c36c:	2a0003f4 	mov	w20, w0
    c370:	aa1303e0 	mov	x0, x19
    c374:	2a1403e1 	mov	w1, w20
    c378:	94000d13 	bl	f7c4 <__lll_lock_wait>
    c37c:	17ffffc4 	b	c28c <pthread_cond_destroy@@GLIBC_2.17+0x50>
    c380:	52800004 	mov	w4, #0x0                   	// #0
    c384:	2a0403e0 	mov	w0, w4
    c388:	a94153f3 	ldp	x19, x20, [sp,#16]
    c38c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    c390:	a94363f7 	ldp	x23, x24, [sp,#48]
    c394:	a9446bf9 	ldp	x25, x26, [sp,#64]
    c398:	a8c67bfd 	ldp	x29, x30, [sp],#96
    c39c:	d65f03c0 	ret
    c3a0:	b9401001 	ldr	w1, [x0,#16]
    c3a4:	b2407be2 	mov	x2, #0x7fffffff            	// #2147483647
    c3a8:	d2800003 	mov	x3, #0x0                   	// #0
    c3ac:	d2800c48 	mov	x8, #0x62                  	// #98
    c3b0:	12190025 	and	w5, w1, #0x80
    c3b4:	52801021 	mov	w1, #0x81                  	// #129
    c3b8:	4a0100a1 	eor	w1, w5, w1
    c3bc:	93407c21 	sxtw	x1, w1
    c3c0:	d4000001 	svc	#0x0
    c3c4:	17ffffbf 	b	c2c0 <pthread_cond_destroy@@GLIBC_2.17+0x84>
    c3c8:	52800001 	mov	w1, #0x0                   	// #0
    c3cc:	885f7e60 	ldxr	w0, [x19]
    c3d0:	8802fe61 	stlxr	w2, w1, [x19]
    c3d4:	35ffffc2 	cbnz	w2, c3cc <pthread_cond_destroy@@GLIBC_2.17+0x190>
    c3d8:	7100041f 	cmp	w0, #0x1
    c3dc:	52800204 	mov	w4, #0x10                  	// #16
    c3e0:	54fffd2d 	b.le	c384 <pthread_cond_destroy@@GLIBC_2.17+0x148>
    c3e4:	52801021 	mov	w1, #0x81                  	// #129
    c3e8:	aa1303e0 	mov	x0, x19
    c3ec:	4a010281 	eor	w1, w20, w1
    c3f0:	d2800022 	mov	x2, #0x1                   	// #1
    c3f4:	d2800003 	mov	x3, #0x0                   	// #0
    c3f8:	d2800c48 	mov	x8, #0x62                  	// #98
    c3fc:	93407c21 	sxtw	x1, w1
    c400:	d4000001 	svc	#0x0
    c404:	17ffffe0 	b	c384 <pthread_cond_destroy@@GLIBC_2.17+0x148>
    c408:	885ffe61 	ldaxr	w1, [x19]
    c40c:	6b1f003f 	cmp	w1, wzr
    c410:	54000061 	b.ne	c41c <pthread_cond_destroy@@GLIBC_2.17+0x1e0>
    c414:	88027e60 	stxr	w2, w0, [x19]
    c418:	35ffff82 	cbnz	w2, c408 <pthread_cond_destroy@@GLIBC_2.17+0x1cc>
    c41c:	54000080 	b.eq	c42c <pthread_cond_destroy@@GLIBC_2.17+0x1f0>
    c420:	b9005fa1 	str	w1, [x29,#92]
    c424:	52801014 	mov	w20, #0x80                  	// #128
    c428:	17ffffd2 	b	c370 <pthread_cond_destroy@@GLIBC_2.17+0x134>
    c42c:	52801014 	mov	w20, #0x80                  	// #128
    c430:	17ffff97 	b	c28c <pthread_cond_destroy@@GLIBC_2.17+0x50>

000000000000c434 <__condvar_cleanup>:
    c434:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    c438:	910003fd 	mov	x29, sp
    c43c:	a90153f3 	stp	x19, x20, [sp,#16]
    c440:	aa0003f3 	mov	x19, x0
    c444:	f9400400 	ldr	x0, [x0,#8]
    c448:	b9002fbf 	str	wzr, [x29,#44]
    c44c:	f9401001 	ldr	x1, [x0,#32]
    c450:	b100043f 	cmn	x1, #0x1
    c454:	52800021 	mov	w1, #0x1                   	// #1
    c458:	54000c40 	b.eq	c5e0 <__condvar_cleanup+0x1ac>
    c45c:	885ffc03 	ldaxr	w3, [x0]
    c460:	6b1f007f 	cmp	w3, wzr
    c464:	54000061 	b.ne	c470 <__condvar_cleanup+0x3c>
    c468:	88027c01 	stxr	w2, w1, [x0]
    c46c:	35ffff82 	cbnz	w2, c45c <__condvar_cleanup+0x28>
    c470:	1a9f17e2 	cset	w2, eq
    c474:	d2801034 	mov	x20, #0x81                  	// #129
    c478:	340004c2 	cbz	w2, c510 <__condvar_cleanup+0xdc>
    c47c:	f9400660 	ldr	x0, [x19,#8]
    c480:	b9401a62 	ldr	w2, [x19,#24]
    c484:	b9402c01 	ldr	w1, [x0,#44]
    c488:	6b01005f 	cmp	w2, w1
    c48c:	540008a0 	b.eq	c5a0 <__condvar_cleanup+0x16c>
    c490:	b9402802 	ldr	w2, [x0,#40]
    c494:	51000842 	sub	w2, w2, #0x2
    c498:	b9002802 	str	w2, [x0,#40]
    c49c:	f9400660 	ldr	x0, [x19,#8]
    c4a0:	f9400401 	ldr	x1, [x0,#8]
    c4a4:	b100043f 	cmn	x1, #0x1
    c4a8:	540004e0 	b.eq	c544 <__condvar_cleanup+0x110>
    c4ac:	52800002 	mov	w2, #0x0                   	// #0
    c4b0:	885f7c01 	ldxr	w1, [x0]
    c4b4:	8803fc02 	stlxr	w3, w2, [x0]
    c4b8:	35ffffc3 	cbnz	w3, c4b0 <__condvar_cleanup+0x7c>
    c4bc:	7100043f 	cmp	w1, #0x1
    c4c0:	5400060c 	b.gt	c580 <__condvar_cleanup+0x14c>
    c4c4:	f9400660 	ldr	x0, [x19,#8]
    c4c8:	aa1403e1 	mov	x1, x20
    c4cc:	b2407be2 	mov	x2, #0x7fffffff            	// #2147483647
    c4d0:	d2800003 	mov	x3, #0x0                   	// #0
    c4d4:	91001000 	add	x0, x0, #0x4
    c4d8:	d2800c48 	mov	x8, #0x62                  	// #98
    c4dc:	d4000001 	svc	#0x0
    c4e0:	f9400a60 	ldr	x0, [x19,#16]
    c4e4:	d1000401 	sub	x1, x0, #0x1
    c4e8:	b1000c3f 	cmn	x1, #0x3
    c4ec:	540000a8 	b.hi	c500 <__condvar_cleanup+0xcc>
    c4f0:	b9401001 	ldr	w1, [x0,#16]
    c4f4:	121c0421 	and	w1, w1, #0x30
    c4f8:	7100803f 	cmp	w1, #0x20
    c4fc:	54000120 	b.eq	c520 <__condvar_cleanup+0xec>
    c500:	97fffb0b 	bl	b12c <__pthread_mutex_cond_lock>
    c504:	a94153f3 	ldp	x19, x20, [sp,#16]
    c508:	a8c37bfd 	ldp	x29, x30, [sp],#48
    c50c:	d65f03c0 	ret
    c510:	2a0203e1 	mov	w1, w2
    c514:	b9002fa3 	str	w3, [x29,#44]
    c518:	94000cab 	bl	f7c4 <__lll_lock_wait>
    c51c:	17ffffd8 	b	c47c <__condvar_cleanup+0x48>
    c520:	d53bd042 	mrs	x2, tpidr_el0
    c524:	b9400001 	ldr	w1, [x0]
    c528:	d11bc042 	sub	x2, x2, #0x6f0
    c52c:	12007421 	and	w1, w1, #0x3fffffff
    c530:	b940d042 	ldr	w2, [x2,#208]
    c534:	6b02003f 	cmp	w1, w2
    c538:	54fffe41 	b.ne	c500 <__condvar_cleanup+0xcc>
    c53c:	97fffb7e 	bl	b334 <__pthread_mutex_cond_lock_adjust>
    c540:	17fffff1 	b	c504 <__condvar_cleanup+0xd0>
    c544:	b9402801 	ldr	w1, [x0,#40]
    c548:	7100043f 	cmp	w1, #0x1
    c54c:	54fffb08 	b.hi	c4ac <__condvar_cleanup+0x78>
    c550:	9100a000 	add	x0, x0, #0x28
    c554:	aa1403e1 	mov	x1, x20
    c558:	d2800022 	mov	x2, #0x1                   	// #1
    c55c:	d2800003 	mov	x3, #0x0                   	// #0
    c560:	d2800c48 	mov	x8, #0x62                  	// #98
    c564:	d4000001 	svc	#0x0
    c568:	f9400660 	ldr	x0, [x19,#8]
    c56c:	885f7c01 	ldxr	w1, [x0]
    c570:	8804fc03 	stlxr	w4, w3, [x0]
    c574:	35ffffc4 	cbnz	w4, c56c <__condvar_cleanup+0x138>
    c578:	7100043f 	cmp	w1, #0x1
    c57c:	54fffb2d 	b.le	c4e0 <__condvar_cleanup+0xac>
    c580:	2a0203e4 	mov	w4, w2
    c584:	aa1403e1 	mov	x1, x20
    c588:	d2800022 	mov	x2, #0x1                   	// #1
    c58c:	d2800003 	mov	x3, #0x0                   	// #0
    c590:	d2800c48 	mov	x8, #0x62                  	// #98
    c594:	d4000001 	svc	#0x0
    c598:	35fffa44 	cbnz	w4, c4e0 <__condvar_cleanup+0xac>
    c59c:	17ffffca 	b	c4c4 <__condvar_cleanup+0x90>
    c5a0:	f9400801 	ldr	x1, [x0,#16]
    c5a4:	f9400402 	ldr	x2, [x0,#8]
    c5a8:	eb02003f 	cmp	x1, x2
    c5ac:	54000102 	b.cs	c5cc <__condvar_cleanup+0x198>
    c5b0:	91000421 	add	x1, x1, #0x1
    c5b4:	f9000801 	str	x1, [x0,#16]
    c5b8:	f9400661 	ldr	x1, [x19,#8]
    c5bc:	b9400420 	ldr	w0, [x1,#4]
    c5c0:	11000400 	add	w0, w0, #0x1
    c5c4:	b9000420 	str	w0, [x1,#4]
    c5c8:	f9400660 	ldr	x0, [x19,#8]
    c5cc:	f9400c01 	ldr	x1, [x0,#24]
    c5d0:	91000421 	add	x1, x1, #0x1
    c5d4:	f9000c01 	str	x1, [x0,#24]
    c5d8:	f9400660 	ldr	x0, [x19,#8]
    c5dc:	17ffffad 	b	c490 <__condvar_cleanup+0x5c>
    c5e0:	885ffc02 	ldaxr	w2, [x0]
    c5e4:	6b1f005f 	cmp	w2, wzr
    c5e8:	54000061 	b.ne	c5f4 <__condvar_cleanup+0x1c0>
    c5ec:	88037c01 	stxr	w3, w1, [x0]
    c5f0:	35ffff83 	cbnz	w3, c5e0 <__condvar_cleanup+0x1ac>
    c5f4:	540000c0 	b.eq	c60c <__condvar_cleanup+0x1d8>
    c5f8:	52801001 	mov	w1, #0x80                  	// #128
    c5fc:	b9002fa2 	str	w2, [x29,#44]
    c600:	d2800034 	mov	x20, #0x1                   	// #1
    c604:	94000c70 	bl	f7c4 <__lll_lock_wait>
    c608:	17ffff9d 	b	c47c <__condvar_cleanup+0x48>
    c60c:	d2800034 	mov	x20, #0x1                   	// #1
    c610:	17ffff9b 	b	c47c <__condvar_cleanup+0x48>

000000000000c614 <pthread_cond_wait@@GLIBC_2.17>:
    c614:	a9b47bfd 	stp	x29, x30, [sp,#-192]!
    c618:	910003fd 	mov	x29, sp
    c61c:	6d0627e8 	stp	d8, d9, [sp,#96]
    c620:	a90153f3 	stp	x19, x20, [sp,#16]
    c624:	a90363f7 	stp	x23, x24, [sp,#48]
    c628:	a9025bf5 	stp	x21, x22, [sp,#32]
    c62c:	a9046bf9 	stp	x25, x26, [sp,#64]
    c630:	a90573fb 	stp	x27, x28, [sp,#80]
    c634:	aa0003f3 	mov	x19, x0
    c638:	b900a3bf 	str	wzr, [x29,#160]
    c63c:	f9401000 	ldr	x0, [x0,#32]
    c640:	aa0103f7 	mov	x23, x1
    c644:	b100041f 	cmn	x0, #0x1
    c648:	52800020 	mov	w0, #0x1                   	// #1
    c64c:	54001520 	b.eq	c8f0 <pthread_cond_wait@@GLIBC_2.17+0x2dc>
    c650:	885ffe61 	ldaxr	w1, [x19]
    c654:	6b1f003f 	cmp	w1, wzr
    c658:	54000061 	b.ne	c664 <pthread_cond_wait@@GLIBC_2.17+0x50>
    c65c:	88027e60 	stxr	w2, w0, [x19]
    c660:	35ffff82 	cbnz	w2, c650 <pthread_cond_wait@@GLIBC_2.17+0x3c>
    c664:	1a9f17e0 	cset	w0, eq
    c668:	52800018 	mov	w24, #0x0                   	// #0
    c66c:	34000f00 	cbz	w0, c84c <pthread_cond_wait@@GLIBC_2.17+0x238>
    c670:	aa1703e0 	mov	x0, x23
    c674:	52800001 	mov	w1, #0x0                   	// #0
    c678:	97fff8a7 	bl	a914 <__pthread_mutex_unlock_usercnt>
    c67c:	35001560 	cbnz	w0, c928 <pthread_cond_wait@@GLIBC_2.17+0x314>
    c680:	f9400662 	ldr	x2, [x19,#8]
    c684:	b9400661 	ldr	w1, [x19,#4]
    c688:	b9402a60 	ldr	w0, [x19,#40]
    c68c:	91000442 	add	x2, x2, #0x1
    c690:	f9401263 	ldr	x3, [x19,#32]
    c694:	11000421 	add	w1, w1, #0x1
    c698:	11000800 	add	w0, w0, #0x2
    c69c:	f9000662 	str	x2, [x19,#8]
    c6a0:	b9000661 	str	w1, [x19,#4]
    c6a4:	b100047f 	cmn	x3, #0x1
    c6a8:	b9002a60 	str	w0, [x19,#40]
    c6ac:	54000040 	b.eq	c6b4 <pthread_cond_wait@@GLIBC_2.17+0xa0>
    c6b0:	f9001277 	str	x23, [x19,#32]
    c6b4:	910203a0 	add	x0, x29, #0x80
    c6b8:	910283b4 	add	x20, x29, #0xa0
    c6bc:	90000001 	adrp	x1, c000 <__pthread_rwlock_unlock+0x20>
    c6c0:	aa1403e2 	mov	x2, x20
    c6c4:	9e670009 	fmov	d9, x0
    c6c8:	9110d021 	add	x1, x1, #0x434
    c6cc:	f90057b3 	str	x19, [x29,#168]
    c6d0:	5280117c 	mov	w28, #0x8b                  	// #139
    c6d4:	f9005bb7 	str	x23, [x29,#176]
    c6d8:	4a1c031c 	eor	w28, w24, w28
    c6dc:	d10006f9 	sub	x25, x23, #0x1
    c6e0:	52800015 	mov	w21, #0x0                   	// #0
    c6e4:	94000ac7 	bl	f200 <_pthread_cleanup_push>
    c6e8:	f9400a7a 	ldr	x26, [x19,#16]
    c6ec:	52801020 	mov	w0, #0x81                  	// #129
    c6f0:	b9402e61 	ldr	w1, [x19,#44]
    c6f4:	4a000300 	eor	w0, w24, w0
    c6f8:	9101f3bb 	add	x27, x29, #0x7c
    c6fc:	b900bba1 	str	w1, [x29,#184]
    c700:	93407f9c 	sxtw	x28, w28
    c704:	93407c00 	sxtw	x0, w0
    c708:	9e670008 	fmov	d8, x0
    c70c:	b9400676 	ldr	w22, [x19,#4]
    c710:	52800001 	mov	w1, #0x0                   	// #0
    c714:	885f7e60 	ldxr	w0, [x19]
    c718:	8802fe61 	stlxr	w2, w1, [x19]
    c71c:	35ffffc2 	cbnz	w2, c714 <pthread_cond_wait@@GLIBC_2.17+0x100>
    c720:	7100041f 	cmp	w0, #0x1
    c724:	54000c0c 	b.gt	c8a4 <pthread_cond_wait@@GLIBC_2.17+0x290>
    c728:	94000bbc 	bl	f618 <__pthread_enable_asynccancel>
    c72c:	b9000280 	str	w0, [x20]
    c730:	350009b5 	cbnz	w21, c864 <pthread_cond_wait@@GLIBC_2.17+0x250>
    c734:	b1000f3f 	cmn	x25, #0x3
    c738:	540000a8 	b.hi	c74c <pthread_cond_wait@@GLIBC_2.17+0x138>
    c73c:	b94012e0 	ldr	w0, [x23,#16]
    c740:	121c0400 	and	w0, w0, #0x30
    c744:	7100801f 	cmp	w0, #0x20
    c748:	540009a0 	b.eq	c87c <pthread_cond_wait@@GLIBC_2.17+0x268>
    c74c:	52190305 	eor	w5, w24, #0x80
    c750:	91001260 	add	x0, x19, #0x4
    c754:	2a1603e2 	mov	w2, w22
    c758:	d2800003 	mov	x3, #0x0                   	// #0
    c75c:	93407ca1 	sxtw	x1, w5
    c760:	d2800c48 	mov	x8, #0x62                  	// #98
    c764:	d4000001 	svc	#0x0
    c768:	2a0303f5 	mov	w21, w3
    c76c:	b9400280 	ldr	w0, [x20]
    c770:	94000bda 	bl	f6d8 <__pthread_disable_asynccancel>
    c774:	b9007fbf 	str	wzr, [x29,#124]
    c778:	52800022 	mov	w2, #0x1                   	// #1
    c77c:	885ffe63 	ldaxr	w3, [x19]
    c780:	6b1f007f 	cmp	w3, wzr
    c784:	54000061 	b.ne	c790 <pthread_cond_wait@@GLIBC_2.17+0x17c>
    c788:	88007e62 	stxr	w0, w2, [x19]
    c78c:	35ffff80 	cbnz	w0, c77c <pthread_cond_wait@@GLIBC_2.17+0x168>
    c790:	540000a0 	b.eq	c7a4 <pthread_cond_wait@@GLIBC_2.17+0x190>
    c794:	aa1303e0 	mov	x0, x19
    c798:	2a1803e1 	mov	w1, w24
    c79c:	b9000363 	str	w3, [x27]
    c7a0:	94000c09 	bl	f7c4 <__lll_lock_wait>
    c7a4:	b9401a83 	ldr	w3, [x20,#24]
    c7a8:	b9402e62 	ldr	w2, [x19,#44]
    c7ac:	6b02007f 	cmp	w3, w2
    c7b0:	54000121 	b.ne	c7d4 <pthread_cond_wait@@GLIBC_2.17+0x1c0>
    c7b4:	f9400a62 	ldr	x2, [x19,#16]
    c7b8:	eb1a005f 	cmp	x2, x26
    c7bc:	54fffa80 	b.eq	c70c <pthread_cond_wait@@GLIBC_2.17+0xf8>
    c7c0:	f9400e60 	ldr	x0, [x19,#24]
    c7c4:	eb02001f 	cmp	x0, x2
    c7c8:	54fffa20 	b.eq	c70c <pthread_cond_wait@@GLIBC_2.17+0xf8>
    c7cc:	91000400 	add	x0, x0, #0x1
    c7d0:	f9000e60 	str	x0, [x19,#24]
    c7d4:	b9402a60 	ldr	w0, [x19,#40]
    c7d8:	51000800 	sub	w0, w0, #0x2
    c7dc:	b9002a60 	str	w0, [x19,#40]
    c7e0:	7100041f 	cmp	w0, #0x1
    c7e4:	54000088 	b.hi	c7f4 <pthread_cond_wait@@GLIBC_2.17+0x1e0>
    c7e8:	f9400660 	ldr	x0, [x19,#8]
    c7ec:	b100041f 	cmn	x0, #0x1
    c7f0:	540006e0 	b.eq	c8cc <pthread_cond_wait@@GLIBC_2.17+0x2b8>
    c7f4:	52800001 	mov	w1, #0x0                   	// #0
    c7f8:	885f7e60 	ldxr	w0, [x19]
    c7fc:	8802fe61 	stlxr	w2, w1, [x19]
    c800:	35ffffc2 	cbnz	w2, c7f8 <pthread_cond_wait@@GLIBC_2.17+0x1e4>
    c804:	7100041f 	cmp	w0, #0x1
    c808:	54000b0c 	b.gt	c968 <pthread_cond_wait@@GLIBC_2.17+0x354>
    c80c:	9e660120 	fmov	x0, d9
    c810:	52800001 	mov	w1, #0x0                   	// #0
    c814:	94000a83 	bl	f220 <_pthread_cleanup_pop>
    c818:	aa1703e0 	mov	x0, x23
    c81c:	35000535 	cbnz	w21, c8c0 <pthread_cond_wait@@GLIBC_2.17+0x2ac>
    c820:	97fffa43 	bl	b12c <__pthread_mutex_cond_lock>
    c824:	2a0003e4 	mov	w4, w0
    c828:	2a0403e0 	mov	w0, w4
    c82c:	6d4627e8 	ldp	d8, d9, [sp,#96]
    c830:	a94153f3 	ldp	x19, x20, [sp,#16]
    c834:	a9425bf5 	ldp	x21, x22, [sp,#32]
    c838:	a94363f7 	ldp	x23, x24, [sp,#48]
    c83c:	a9446bf9 	ldp	x25, x26, [sp,#64]
    c840:	a94573fb 	ldp	x27, x28, [sp,#80]
    c844:	a8cc7bfd 	ldp	x29, x30, [sp],#192
    c848:	d65f03c0 	ret
    c84c:	b900a3a1 	str	w1, [x29,#160]
    c850:	2a0003f8 	mov	w24, w0
    c854:	aa1303e0 	mov	x0, x19
    c858:	2a1803e1 	mov	w1, w24
    c85c:	94000bda 	bl	f7c4 <__lll_lock_wait>
    c860:	17ffff84 	b	c670 <pthread_cond_wait@@GLIBC_2.17+0x5c>
    c864:	aa1703e0 	mov	x0, x23
    c868:	97fffab3 	bl	b334 <__pthread_mutex_cond_lock_adjust>
    c86c:	aa1703e0 	mov	x0, x23
    c870:	52800001 	mov	w1, #0x0                   	// #0
    c874:	97fff828 	bl	a914 <__pthread_mutex_unlock_usercnt>
    c878:	17ffffaf 	b	c734 <pthread_cond_wait@@GLIBC_2.17+0x120>
    c87c:	91001260 	add	x0, x19, #0x4
    c880:	aa1c03e1 	mov	x1, x28
    c884:	2a1603e2 	mov	w2, w22
    c888:	d2800003 	mov	x3, #0x0                   	// #0
    c88c:	aa1703e4 	mov	x4, x23
    c890:	d2800c48 	mov	x8, #0x62                  	// #98
    c894:	d4000001 	svc	#0x0
    c898:	b140041f 	cmn	x0, #0x1, lsl #12
    c89c:	1a9f87f5 	cset	w21, ls
    c8a0:	17ffffb3 	b	c76c <pthread_cond_wait@@GLIBC_2.17+0x158>
    c8a4:	aa1303e0 	mov	x0, x19
    c8a8:	d2800022 	mov	x2, #0x1                   	// #1
    c8ac:	9e660101 	fmov	x1, d8
    c8b0:	d2800003 	mov	x3, #0x0                   	// #0
    c8b4:	d2800c48 	mov	x8, #0x62                  	// #98
    c8b8:	d4000001 	svc	#0x0
    c8bc:	17ffff9b 	b	c728 <pthread_cond_wait@@GLIBC_2.17+0x114>
    c8c0:	97fffa9d 	bl	b334 <__pthread_mutex_cond_lock_adjust>
    c8c4:	52800004 	mov	w4, #0x0                   	// #0
    c8c8:	17ffffd8 	b	c828 <pthread_cond_wait@@GLIBC_2.17+0x214>
    c8cc:	52801021 	mov	w1, #0x81                  	// #129
    c8d0:	9100a260 	add	x0, x19, #0x28
    c8d4:	4a010301 	eor	w1, w24, w1
    c8d8:	d2800022 	mov	x2, #0x1                   	// #1
    c8dc:	d2800003 	mov	x3, #0x0                   	// #0
    c8e0:	d2800c48 	mov	x8, #0x62                  	// #98
    c8e4:	93407c21 	sxtw	x1, w1
    c8e8:	d4000001 	svc	#0x0
    c8ec:	17ffffc2 	b	c7f4 <pthread_cond_wait@@GLIBC_2.17+0x1e0>
    c8f0:	885ffe61 	ldaxr	w1, [x19]
    c8f4:	6b1f003f 	cmp	w1, wzr
    c8f8:	54000061 	b.ne	c904 <pthread_cond_wait@@GLIBC_2.17+0x2f0>
    c8fc:	88027e60 	stxr	w2, w0, [x19]
    c900:	35ffff82 	cbnz	w2, c8f0 <pthread_cond_wait@@GLIBC_2.17+0x2dc>
    c904:	54000080 	b.eq	c914 <pthread_cond_wait@@GLIBC_2.17+0x300>
    c908:	b900a3a1 	str	w1, [x29,#160]
    c90c:	52801018 	mov	w24, #0x80                  	// #128
    c910:	17ffffd1 	b	c854 <pthread_cond_wait@@GLIBC_2.17+0x240>
    c914:	aa1703e0 	mov	x0, x23
    c918:	52800001 	mov	w1, #0x0                   	// #0
    c91c:	52801018 	mov	w24, #0x80                  	// #128
    c920:	97fff7fd 	bl	a914 <__pthread_mutex_unlock_usercnt>
    c924:	34ffeae0 	cbz	w0, c680 <pthread_cond_wait@@GLIBC_2.17+0x6c>
    c928:	52800002 	mov	w2, #0x0                   	// #0
    c92c:	885f7e61 	ldxr	w1, [x19]
    c930:	8803fe62 	stlxr	w3, w2, [x19]
    c934:	35ffffc3 	cbnz	w3, c92c <pthread_cond_wait@@GLIBC_2.17+0x318>
    c938:	7100043f 	cmp	w1, #0x1
    c93c:	2a0003e4 	mov	w4, w0
    c940:	54fff74d 	b.le	c828 <pthread_cond_wait@@GLIBC_2.17+0x214>
    c944:	52801021 	mov	w1, #0x81                  	// #129
    c948:	aa1303e0 	mov	x0, x19
    c94c:	4a010301 	eor	w1, w24, w1
    c950:	d2800022 	mov	x2, #0x1                   	// #1
    c954:	d2800003 	mov	x3, #0x0                   	// #0
    c958:	d2800c48 	mov	x8, #0x62                  	// #98
    c95c:	93407c21 	sxtw	x1, w1
    c960:	d4000001 	svc	#0x0
    c964:	17ffffb1 	b	c828 <pthread_cond_wait@@GLIBC_2.17+0x214>
    c968:	52801021 	mov	w1, #0x81                  	// #129
    c96c:	aa1303e0 	mov	x0, x19
    c970:	4a010301 	eor	w1, w24, w1
    c974:	d2800022 	mov	x2, #0x1                   	// #1
    c978:	d2800003 	mov	x3, #0x0                   	// #0
    c97c:	d2800c48 	mov	x8, #0x62                  	// #98
    c980:	93407c21 	sxtw	x1, w1
    c984:	d4000001 	svc	#0x0
    c988:	17ffffa1 	b	c80c <pthread_cond_wait@@GLIBC_2.17+0x1f8>

000000000000c98c <pthread_cond_timedwait@@GLIBC_2.17>:
    c98c:	a9b37bfd 	stp	x29, x30, [sp,#-208]!
    c990:	d2993fe3 	mov	x3, #0xc9ff                	// #51711
    c994:	528002c4 	mov	w4, #0x16                  	// #22
    c998:	910003fd 	mov	x29, sp
    c99c:	6d0627e8 	stp	d8, d9, [sp,#96]
    c9a0:	f9400445 	ldr	x5, [x2,#8]
    c9a4:	f2a77343 	movk	x3, #0x3b9a, lsl #16
    c9a8:	a90153f3 	stp	x19, x20, [sp,#16]
    c9ac:	a9025bf5 	stp	x21, x22, [sp,#32]
    c9b0:	a90363f7 	stp	x23, x24, [sp,#48]
    c9b4:	a9046bf9 	stp	x25, x26, [sp,#64]
    c9b8:	a90573fb 	stp	x27, x28, [sp,#80]
    c9bc:	eb0300bf 	cmp	x5, x3
    c9c0:	fd003bea 	str	d10, [sp,#112]
    c9c4:	54000169 	b.ls	c9f0 <pthread_cond_timedwait@@GLIBC_2.17+0x64>
    c9c8:	2a0403e0 	mov	w0, w4
    c9cc:	6d4627e8 	ldp	d8, d9, [sp,#96]
    c9d0:	a94153f3 	ldp	x19, x20, [sp,#16]
    c9d4:	a9425bf5 	ldp	x21, x22, [sp,#32]
    c9d8:	a94363f7 	ldp	x23, x24, [sp,#48]
    c9dc:	a9446bf9 	ldp	x25, x26, [sp,#64]
    c9e0:	a94573fb 	ldp	x27, x28, [sp,#80]
    c9e4:	fd403bea 	ldr	d10, [sp,#112]
    c9e8:	a8cd7bfd 	ldp	x29, x30, [sp],#208
    c9ec:	d65f03c0 	ret
    c9f0:	aa0003f3 	mov	x19, x0
    c9f4:	f9401000 	ldr	x0, [x0,#32]
    c9f8:	b900b3bf 	str	wzr, [x29,#176]
    c9fc:	aa0103f5 	mov	x21, x1
    ca00:	b100041f 	cmn	x0, #0x1
    ca04:	aa0203f6 	mov	x22, x2
    ca08:	52800020 	mov	w0, #0x1                   	// #1
    ca0c:	540019a0 	b.eq	cd40 <pthread_cond_timedwait@@GLIBC_2.17+0x3b4>
    ca10:	885ffe61 	ldaxr	w1, [x19]
    ca14:	6b1f003f 	cmp	w1, wzr
    ca18:	54000061 	b.ne	ca24 <pthread_cond_timedwait@@GLIBC_2.17+0x98>
    ca1c:	88027e60 	stxr	w2, w0, [x19]
    ca20:	35ffff82 	cbnz	w2, ca10 <pthread_cond_timedwait@@GLIBC_2.17+0x84>
    ca24:	1a9f17e0 	cset	w0, eq
    ca28:	52800017 	mov	w23, #0x0                   	// #0
    ca2c:	34001360 	cbz	w0, cc98 <pthread_cond_timedwait@@GLIBC_2.17+0x30c>
    ca30:	aa1503e0 	mov	x0, x21
    ca34:	52800001 	mov	w1, #0x0                   	// #0
    ca38:	97fff7b7 	bl	a914 <__pthread_mutex_unlock_usercnt>
    ca3c:	35001500 	cbnz	w0, ccdc <pthread_cond_timedwait@@GLIBC_2.17+0x350>
    ca40:	f9400663 	ldr	x3, [x19,#8]
    ca44:	b9400662 	ldr	w2, [x19,#4]
    ca48:	b9402a61 	ldr	w1, [x19,#40]
    ca4c:	91000463 	add	x3, x3, #0x1
    ca50:	11000442 	add	w2, w2, #0x1
    ca54:	f9000663 	str	x3, [x19,#8]
    ca58:	b9000662 	str	w2, [x19,#4]
    ca5c:	11000821 	add	w1, w1, #0x2
    ca60:	b9002a61 	str	w1, [x19,#40]
    ca64:	f94002c4 	ldr	x4, [x22]
    ca68:	b7f81a44 	tbnz	x4, #63, cdb0 <pthread_cond_timedwait@@GLIBC_2.17+0x424>
    ca6c:	f9401260 	ldr	x0, [x19,#32]
    ca70:	b100041f 	cmn	x0, #0x1
    ca74:	54000040 	b.eq	ca7c <pthread_cond_timedwait@@GLIBC_2.17+0xf0>
    ca78:	f9001275 	str	x21, [x19,#32]
    ca7c:	910243a0 	add	x0, x29, #0x90
    ca80:	9102c3b4 	add	x20, x29, #0xb0
    ca84:	90000001 	adrp	x1, c000 <__pthread_rwlock_unlock+0x20>
    ca88:	aa1403e2 	mov	x2, x20
    ca8c:	9e670009 	fmov	d9, x0
    ca90:	9110d021 	add	x1, x1, #0x434
    ca94:	f9005fb3 	str	x19, [x29,#184]
    ca98:	5280001b 	mov	w27, #0x0                   	// #0
    ca9c:	f90063b5 	str	x21, [x29,#192]
    caa0:	d10006b9 	sub	x25, x21, #0x1
    caa4:	2a1b03f8 	mov	w24, w27
    caa8:	940009d6 	bl	f200 <_pthread_cleanup_push>
    caac:	f9400a7a 	ldr	x26, [x19,#16]
    cab0:	52801020 	mov	w0, #0x81                  	// #129
    cab4:	b9402e61 	ldr	w1, [x19,#44]
    cab8:	4a0002e0 	eor	w0, w23, w0
    cabc:	b900cba1 	str	w1, [x29,#200]
    cac0:	910233a1 	add	x1, x29, #0x8c
    cac4:	93407c00 	sxtw	x0, w0
    cac8:	9e670028 	fmov	d8, x1
    cacc:	9e67000a 	fmov	d10, x0
    cad0:	b940067c 	ldr	w28, [x19,#4]
    cad4:	885f7e60 	ldxr	w0, [x19]
    cad8:	8801fe78 	stlxr	w1, w24, [x19]
    cadc:	35ffffc1 	cbnz	w1, cad4 <pthread_cond_timedwait@@GLIBC_2.17+0x148>
    cae0:	7100041f 	cmp	w0, #0x1
    cae4:	54000e6c 	b.gt	ccb0 <pthread_cond_timedwait@@GLIBC_2.17+0x324>
    cae8:	94000acc 	bl	f618 <__pthread_enable_asynccancel>
    caec:	b9000280 	str	w0, [x20]
    caf0:	35000a5b 	cbnz	w27, cc38 <pthread_cond_timedwait@@GLIBC_2.17+0x2ac>
    caf4:	b1000f3f 	cmn	x25, #0x3
    caf8:	540000a8 	b.hi	cb0c <pthread_cond_timedwait@@GLIBC_2.17+0x180>
    cafc:	b94012a0 	ldr	w0, [x21,#16]
    cb00:	121c0400 	and	w0, w0, #0x30
    cb04:	7100801f 	cmp	w0, #0x20
    cb08:	54000a40 	b.eq	cc50 <pthread_cond_timedwait@@GLIBC_2.17+0x2c4>
    cb0c:	b9402a65 	ldr	w5, [x19,#40]
    cb10:	52801121 	mov	w1, #0x89                  	// #137
    cb14:	52803126 	mov	w6, #0x189                 	// #393
    cb18:	91001260 	add	x0, x19, #0x4
    cb1c:	120000a5 	and	w5, w5, #0x1
    cb20:	2a1c03e2 	mov	w2, w28
    cb24:	6b1f00bf 	cmp	w5, wzr
    cb28:	aa1603e3 	mov	x3, x22
    cb2c:	1a861026 	csel	w6, w1, w6, ne
    cb30:	d2800004 	mov	x4, #0x0                   	// #0
    cb34:	b2407fe5 	mov	x5, #0xffffffff            	// #4294967295
    cb38:	d2800c48 	mov	x8, #0x62                  	// #98
    cb3c:	4a1700c1 	eor	w1, w6, w23
    cb40:	d4000001 	svc	#0x0
    cb44:	b13ffc1f 	cmn	x0, #0xfff
    cb48:	2a0403fb 	mov	w27, w4
    cb4c:	1a9f201c 	csel	w28, w0, wzr, cs
    cb50:	b9400280 	ldr	w0, [x20]
    cb54:	94000ae1 	bl	f6d8 <__pthread_disable_asynccancel>
    cb58:	b9008fbf 	str	wzr, [x29,#140]
    cb5c:	52800020 	mov	w0, #0x1                   	// #1
    cb60:	885ffe62 	ldaxr	w2, [x19]
    cb64:	6b1f005f 	cmp	w2, wzr
    cb68:	54000061 	b.ne	cb74 <pthread_cond_timedwait@@GLIBC_2.17+0x1e8>
    cb6c:	88017e60 	stxr	w1, w0, [x19]
    cb70:	35ffff81 	cbnz	w1, cb60 <pthread_cond_timedwait@@GLIBC_2.17+0x1d4>
    cb74:	540000c0 	b.eq	cb8c <pthread_cond_timedwait@@GLIBC_2.17+0x200>
    cb78:	9e660103 	fmov	x3, d8
    cb7c:	aa1303e0 	mov	x0, x19
    cb80:	2a1703e1 	mov	w1, w23
    cb84:	b9000062 	str	w2, [x3]
    cb88:	94000b0f 	bl	f7c4 <__lll_lock_wait>
    cb8c:	b9401a81 	ldr	w1, [x20,#24]
    cb90:	b9402e60 	ldr	w0, [x19,#44]
    cb94:	6b00003f 	cmp	w1, w0
    cb98:	54000ea1 	b.ne	cd6c <pthread_cond_timedwait@@GLIBC_2.17+0x3e0>
    cb9c:	f9400a65 	ldr	x5, [x19,#16]
    cba0:	eb1a00bf 	cmp	x5, x26
    cba4:	54000080 	b.eq	cbb4 <pthread_cond_timedwait@@GLIBC_2.17+0x228>
    cba8:	f9400e60 	ldr	x0, [x19,#24]
    cbac:	eb05001f 	cmp	x0, x5
    cbb0:	54000e61 	b.ne	cd7c <pthread_cond_timedwait@@GLIBC_2.17+0x3f0>
    cbb4:	3101bb9f 	cmn	w28, #0x6e
    cbb8:	54fff8c1 	b.ne	cad0 <pthread_cond_timedwait@@GLIBC_2.17+0x144>
    cbbc:	b9400662 	ldr	w2, [x19,#4]
    cbc0:	b9402a61 	ldr	w1, [x19,#40]
    cbc4:	f9400663 	ldr	x3, [x19,#8]
    cbc8:	f9400e60 	ldr	x0, [x19,#24]
    cbcc:	910004a5 	add	x5, x5, #0x1
    cbd0:	11000442 	add	w2, w2, #0x1
    cbd4:	f9000a65 	str	x5, [x19,#16]
    cbd8:	b9000662 	str	w2, [x19,#4]
    cbdc:	52800dd4 	mov	w20, #0x6e                  	// #110
    cbe0:	91000400 	add	x0, x0, #0x1
    cbe4:	f9000e60 	str	x0, [x19,#24]
    cbe8:	51000821 	sub	w1, w1, #0x2
    cbec:	b9002a61 	str	w1, [x19,#40]
    cbf0:	7100043f 	cmp	w1, #0x1
    cbf4:	54000068 	b.hi	cc00 <pthread_cond_timedwait@@GLIBC_2.17+0x274>
    cbf8:	b100047f 	cmn	x3, #0x1
    cbfc:	54000900 	b.eq	cd1c <pthread_cond_timedwait@@GLIBC_2.17+0x390>
    cc00:	52800001 	mov	w1, #0x0                   	// #0
    cc04:	885f7e60 	ldxr	w0, [x19]
    cc08:	8802fe61 	stlxr	w2, w1, [x19]
    cc0c:	35ffffc2 	cbnz	w2, cc04 <pthread_cond_timedwait@@GLIBC_2.17+0x278>
    cc10:	7100041f 	cmp	w0, #0x1
    cc14:	54000bcc 	b.gt	cd8c <pthread_cond_timedwait@@GLIBC_2.17+0x400>
    cc18:	9e660120 	fmov	x0, d9
    cc1c:	52800001 	mov	w1, #0x0                   	// #0
    cc20:	94000980 	bl	f220 <_pthread_cleanup_pop>
    cc24:	aa1503e0 	mov	x0, x21
    cc28:	3400053b 	cbz	w27, cccc <pthread_cond_timedwait@@GLIBC_2.17+0x340>
    cc2c:	97fff9c2 	bl	b334 <__pthread_mutex_cond_lock_adjust>
    cc30:	2a1403e4 	mov	w4, w20
    cc34:	17ffff65 	b	c9c8 <pthread_cond_timedwait@@GLIBC_2.17+0x3c>
    cc38:	aa1503e0 	mov	x0, x21
    cc3c:	97fff9be 	bl	b334 <__pthread_mutex_cond_lock_adjust>
    cc40:	aa1503e0 	mov	x0, x21
    cc44:	52800001 	mov	w1, #0x0                   	// #0
    cc48:	97fff733 	bl	a914 <__pthread_mutex_unlock_usercnt>
    cc4c:	17ffffaa 	b	caf4 <pthread_cond_timedwait@@GLIBC_2.17+0x168>
    cc50:	b9402a61 	ldr	w1, [x19,#40]
    cc54:	52803164 	mov	w4, #0x18b                 	// #395
    cc58:	91001260 	add	x0, x19, #0x4
    cc5c:	2a1c03e2 	mov	w2, w28
    cc60:	12000021 	and	w1, w1, #0x1
    cc64:	aa1603e3 	mov	x3, x22
    cc68:	6b1f003f 	cmp	w1, wzr
    cc6c:	52801161 	mov	w1, #0x8b                  	// #139
    cc70:	1a841021 	csel	w1, w1, w4, ne
    cc74:	d2800c48 	mov	x8, #0x62                  	// #98
    cc78:	aa1503e4 	mov	x4, x21
    cc7c:	4a170021 	eor	w1, w1, w23
    cc80:	d4000001 	svc	#0x0
    cc84:	b13ffc1f 	cmn	x0, #0xfff
    cc88:	1a9f201c 	csel	w28, w0, wzr, cs
    cc8c:	b13ffc1f 	cmn	x0, #0xfff
    cc90:	1a9f27fb 	cset	w27, cc
    cc94:	17ffffaf 	b	cb50 <pthread_cond_timedwait@@GLIBC_2.17+0x1c4>
    cc98:	b900b3a1 	str	w1, [x29,#176]
    cc9c:	2a0003f7 	mov	w23, w0
    cca0:	aa1303e0 	mov	x0, x19
    cca4:	2a1703e1 	mov	w1, w23
    cca8:	94000ac7 	bl	f7c4 <__lll_lock_wait>
    ccac:	17ffff61 	b	ca30 <pthread_cond_timedwait@@GLIBC_2.17+0xa4>
    ccb0:	aa1303e0 	mov	x0, x19
    ccb4:	d2800022 	mov	x2, #0x1                   	// #1
    ccb8:	9e660141 	fmov	x1, d10
    ccbc:	d2800003 	mov	x3, #0x0                   	// #0
    ccc0:	d2800c48 	mov	x8, #0x62                  	// #98
    ccc4:	d4000001 	svc	#0x0
    ccc8:	17ffff88 	b	cae8 <pthread_cond_timedwait@@GLIBC_2.17+0x15c>
    cccc:	97fff918 	bl	b12c <__pthread_mutex_cond_lock>
    ccd0:	6b1f001f 	cmp	w0, wzr
    ccd4:	1a941004 	csel	w4, w0, w20, ne
    ccd8:	17ffff3c 	b	c9c8 <pthread_cond_timedwait@@GLIBC_2.17+0x3c>
    ccdc:	52800002 	mov	w2, #0x0                   	// #0
    cce0:	885f7e61 	ldxr	w1, [x19]
    cce4:	8803fe62 	stlxr	w3, w2, [x19]
    cce8:	35ffffc3 	cbnz	w3, cce0 <pthread_cond_timedwait@@GLIBC_2.17+0x354>
    ccec:	7100043f 	cmp	w1, #0x1
    ccf0:	2a0003e4 	mov	w4, w0
    ccf4:	54ffe6ad 	b.le	c9c8 <pthread_cond_timedwait@@GLIBC_2.17+0x3c>
    ccf8:	52801021 	mov	w1, #0x81                  	// #129
    ccfc:	aa1303e0 	mov	x0, x19
    cd00:	4a0102e1 	eor	w1, w23, w1
    cd04:	d2800022 	mov	x2, #0x1                   	// #1
    cd08:	d2800003 	mov	x3, #0x0                   	// #0
    cd0c:	d2800c48 	mov	x8, #0x62                  	// #98
    cd10:	93407c21 	sxtw	x1, w1
    cd14:	d4000001 	svc	#0x0
    cd18:	17ffff2c 	b	c9c8 <pthread_cond_timedwait@@GLIBC_2.17+0x3c>
    cd1c:	52801021 	mov	w1, #0x81                  	// #129
    cd20:	9100a260 	add	x0, x19, #0x28
    cd24:	4a0102e1 	eor	w1, w23, w1
    cd28:	d2800022 	mov	x2, #0x1                   	// #1
    cd2c:	d2800003 	mov	x3, #0x0                   	// #0
    cd30:	d2800c48 	mov	x8, #0x62                  	// #98
    cd34:	93407c21 	sxtw	x1, w1
    cd38:	d4000001 	svc	#0x0
    cd3c:	17ffffb1 	b	cc00 <pthread_cond_timedwait@@GLIBC_2.17+0x274>
    cd40:	885ffe61 	ldaxr	w1, [x19]
    cd44:	6b1f003f 	cmp	w1, wzr
    cd48:	54000061 	b.ne	cd54 <pthread_cond_timedwait@@GLIBC_2.17+0x3c8>
    cd4c:	88027e60 	stxr	w2, w0, [x19]
    cd50:	35ffff82 	cbnz	w2, cd40 <pthread_cond_timedwait@@GLIBC_2.17+0x3b4>
    cd54:	54000080 	b.eq	cd64 <pthread_cond_timedwait@@GLIBC_2.17+0x3d8>
    cd58:	b900b3a1 	str	w1, [x29,#176]
    cd5c:	52801017 	mov	w23, #0x80                  	// #128
    cd60:	17ffffd0 	b	cca0 <pthread_cond_timedwait@@GLIBC_2.17+0x314>
    cd64:	52801017 	mov	w23, #0x80                  	// #128
    cd68:	17ffff32 	b	ca30 <pthread_cond_timedwait@@GLIBC_2.17+0xa4>
    cd6c:	b9402a61 	ldr	w1, [x19,#40]
    cd70:	52800014 	mov	w20, #0x0                   	// #0
    cd74:	f9400663 	ldr	x3, [x19,#8]
    cd78:	17ffff9c 	b	cbe8 <pthread_cond_timedwait@@GLIBC_2.17+0x25c>
    cd7c:	b9402a61 	ldr	w1, [x19,#40]
    cd80:	52800014 	mov	w20, #0x0                   	// #0
    cd84:	f9400663 	ldr	x3, [x19,#8]
    cd88:	17ffff96 	b	cbe0 <pthread_cond_timedwait@@GLIBC_2.17+0x254>
    cd8c:	52801021 	mov	w1, #0x81                  	// #129
    cd90:	aa1303e0 	mov	x0, x19
    cd94:	4a0102e1 	eor	w1, w23, w1
    cd98:	d2800022 	mov	x2, #0x1                   	// #1
    cd9c:	d2800003 	mov	x3, #0x0                   	// #0
    cda0:	d2800c48 	mov	x8, #0x62                  	// #98
    cda4:	93407c21 	sxtw	x1, w1
    cda8:	d4000001 	svc	#0x0
    cdac:	17ffff9b 	b	cc18 <pthread_cond_timedwait@@GLIBC_2.17+0x28c>
    cdb0:	2a0003fb 	mov	w27, w0
    cdb4:	910243a0 	add	x0, x29, #0x90
    cdb8:	f9400a65 	ldr	x5, [x19,#16]
    cdbc:	9e670009 	fmov	d9, x0
    cdc0:	17ffff82 	b	cbc8 <pthread_cond_timedwait@@GLIBC_2.17+0x23c>

000000000000cdc4 <pthread_cond_signal@@GLIBC_2.17>:
    cdc4:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    cdc8:	910003fd 	mov	x29, sp
    cdcc:	a90153f3 	stp	x19, x20, [sp,#16]
    cdd0:	aa0003f3 	mov	x19, x0
    cdd4:	f9401000 	ldr	x0, [x0,#32]
    cdd8:	b9002fbf 	str	wzr, [x29,#44]
    cddc:	b100041f 	cmn	x0, #0x1
    cde0:	52800020 	mov	w0, #0x1                   	// #1
    cde4:	54000ba0 	b.eq	cf58 <pthread_cond_signal@@GLIBC_2.17+0x194>
    cde8:	885ffe61 	ldaxr	w1, [x19]
    cdec:	6b1f003f 	cmp	w1, wzr
    cdf0:	54000061 	b.ne	cdfc <pthread_cond_signal@@GLIBC_2.17+0x38>
    cdf4:	88027e60 	stxr	w2, w0, [x19]
    cdf8:	35ffff82 	cbnz	w2, cde8 <pthread_cond_signal@@GLIBC_2.17+0x24>
    cdfc:	1a9f17e0 	cset	w0, eq
    ce00:	52800014 	mov	w20, #0x0                   	// #0
    ce04:	34000400 	cbz	w0, ce84 <pthread_cond_signal@@GLIBC_2.17+0xc0>
    ce08:	f9400a60 	ldr	x0, [x19,#16]
    ce0c:	f9400661 	ldr	x1, [x19,#8]
    ce10:	eb00003f 	cmp	x1, x0
    ce14:	54000849 	b.ls	cf1c <pthread_cond_signal@@GLIBC_2.17+0x158>
    ce18:	f9401264 	ldr	x4, [x19,#32]
    ce1c:	91000400 	add	x0, x0, #0x1
    ce20:	b9400665 	ldr	w5, [x19,#4]
    ce24:	d1000481 	sub	x1, x4, #0x1
    ce28:	f9000a60 	str	x0, [x19,#16]
    ce2c:	110004a5 	add	w5, w5, #0x1
    ce30:	b1000c3f 	cmn	x1, #0x3
    ce34:	b9000665 	str	w5, [x19,#4]
    ce38:	54000329 	b.ls	ce9c <pthread_cond_signal@@GLIBC_2.17+0xd8>
    ce3c:	528010a1 	mov	w1, #0x85                  	// #133
    ce40:	d2800022 	mov	x2, #0x1                   	// #1
    ce44:	4a010281 	eor	w1, w20, w1
    ce48:	91001266 	add	x6, x19, #0x4
    ce4c:	aa0203e5 	mov	x5, x2
    ce50:	aa0603e0 	mov	x0, x6
    ce54:	93407c21 	sxtw	x1, w1
    ce58:	aa0203e3 	mov	x3, x2
    ce5c:	aa1303e4 	mov	x4, x19
    ce60:	f2a08005 	movk	x5, #0x400, lsl #16
    ce64:	d2800c48 	mov	x8, #0x62                  	// #98
    ce68:	d4000001 	svc	#0x0
    ce6c:	b140041f 	cmn	x0, #0x1, lsl #12
    ce70:	540004a8 	b.hi	cf04 <pthread_cond_signal@@GLIBC_2.17+0x140>
    ce74:	52800000 	mov	w0, #0x0                   	// #0
    ce78:	a94153f3 	ldp	x19, x20, [sp,#16]
    ce7c:	a8c37bfd 	ldp	x29, x30, [sp],#48
    ce80:	d65f03c0 	ret
    ce84:	b9002fa1 	str	w1, [x29,#44]
    ce88:	2a0003f4 	mov	w20, w0
    ce8c:	aa1303e0 	mov	x0, x19
    ce90:	2a1403e1 	mov	w1, w20
    ce94:	94000a4c 	bl	f7c4 <__lll_lock_wait>
    ce98:	17ffffdc 	b	ce08 <pthread_cond_signal@@GLIBC_2.17+0x44>
    ce9c:	b9401080 	ldr	w0, [x4,#16]
    cea0:	121c0400 	and	w0, w0, #0x30
    cea4:	7100801f 	cmp	w0, #0x20
    cea8:	54fffca1 	b.ne	ce3c <pthread_cond_signal@@GLIBC_2.17+0x78>
    ceac:	52801181 	mov	w1, #0x8c                  	// #140
    ceb0:	91001260 	add	x0, x19, #0x4
    ceb4:	4a010281 	eor	w1, w20, w1
    ceb8:	d2800022 	mov	x2, #0x1                   	// #1
    cebc:	d2800003 	mov	x3, #0x0                   	// #0
    cec0:	d2800c48 	mov	x8, #0x62                  	// #98
    cec4:	93407c21 	sxtw	x1, w1
    cec8:	d4000001 	svc	#0x0
    cecc:	b140041f 	cmn	x0, #0x1, lsl #12
    ced0:	2a0303e1 	mov	w1, w3
    ced4:	54fffb48 	b.hi	ce3c <pthread_cond_signal@@GLIBC_2.17+0x78>
    ced8:	885f7e60 	ldxr	w0, [x19]
    cedc:	8804fe61 	stlxr	w4, w1, [x19]
    cee0:	35ffffc4 	cbnz	w4, ced8 <pthread_cond_signal@@GLIBC_2.17+0x114>
    cee4:	7100041f 	cmp	w0, #0x1
    cee8:	54fffc6d 	b.le	ce74 <pthread_cond_signal@@GLIBC_2.17+0xb0>
    ceec:	52801021 	mov	w1, #0x81                  	// #129
    cef0:	aa1303e0 	mov	x0, x19
    cef4:	4a010281 	eor	w1, w20, w1
    cef8:	93407c21 	sxtw	x1, w1
    cefc:	d4000001 	svc	#0x0
    cf00:	17ffffdd 	b	ce74 <pthread_cond_signal@@GLIBC_2.17+0xb0>
    cf04:	52801021 	mov	w1, #0x81                  	// #129
    cf08:	aa0603e0 	mov	x0, x6
    cf0c:	4a010281 	eor	w1, w20, w1
    cf10:	d2800003 	mov	x3, #0x0                   	// #0
    cf14:	93407c21 	sxtw	x1, w1
    cf18:	d4000001 	svc	#0x0
    cf1c:	52800001 	mov	w1, #0x0                   	// #0
    cf20:	885f7e60 	ldxr	w0, [x19]
    cf24:	8802fe61 	stlxr	w2, w1, [x19]
    cf28:	35ffffc2 	cbnz	w2, cf20 <pthread_cond_signal@@GLIBC_2.17+0x15c>
    cf2c:	7100041f 	cmp	w0, #0x1
    cf30:	54fffa2d 	b.le	ce74 <pthread_cond_signal@@GLIBC_2.17+0xb0>
    cf34:	52801021 	mov	w1, #0x81                  	// #129
    cf38:	aa1303e0 	mov	x0, x19
    cf3c:	4a010281 	eor	w1, w20, w1
    cf40:	d2800022 	mov	x2, #0x1                   	// #1
    cf44:	d2800003 	mov	x3, #0x0                   	// #0
    cf48:	d2800c48 	mov	x8, #0x62                  	// #98
    cf4c:	93407c21 	sxtw	x1, w1
    cf50:	d4000001 	svc	#0x0
    cf54:	17ffffc8 	b	ce74 <pthread_cond_signal@@GLIBC_2.17+0xb0>
    cf58:	885ffe61 	ldaxr	w1, [x19]
    cf5c:	6b1f003f 	cmp	w1, wzr
    cf60:	54000061 	b.ne	cf6c <pthread_cond_signal@@GLIBC_2.17+0x1a8>
    cf64:	88027e60 	stxr	w2, w0, [x19]
    cf68:	35ffff82 	cbnz	w2, cf58 <pthread_cond_signal@@GLIBC_2.17+0x194>
    cf6c:	54000080 	b.eq	cf7c <pthread_cond_signal@@GLIBC_2.17+0x1b8>
    cf70:	b9002fa1 	str	w1, [x29,#44]
    cf74:	52801014 	mov	w20, #0x80                  	// #128
    cf78:	17ffffc5 	b	ce8c <pthread_cond_signal@@GLIBC_2.17+0xc8>
    cf7c:	52801014 	mov	w20, #0x80                  	// #128
    cf80:	17ffffa2 	b	ce08 <pthread_cond_signal@@GLIBC_2.17+0x44>

000000000000cf84 <pthread_cond_broadcast@@GLIBC_2.17>:
    cf84:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    cf88:	910003fd 	mov	x29, sp
    cf8c:	a90153f3 	stp	x19, x20, [sp,#16]
    cf90:	aa0003f3 	mov	x19, x0
    cf94:	f9401000 	ldr	x0, [x0,#32]
    cf98:	b9002fbf 	str	wzr, [x29,#44]
    cf9c:	b100041f 	cmn	x0, #0x1
    cfa0:	52800020 	mov	w0, #0x1                   	// #1
    cfa4:	540009a0 	b.eq	d0d8 <pthread_cond_broadcast@@GLIBC_2.17+0x154>
    cfa8:	885ffe61 	ldaxr	w1, [x19]
    cfac:	6b1f003f 	cmp	w1, wzr
    cfb0:	54000061 	b.ne	cfbc <pthread_cond_broadcast@@GLIBC_2.17+0x38>
    cfb4:	88027e60 	stxr	w2, w0, [x19]
    cfb8:	35ffff82 	cbnz	w2, cfa8 <pthread_cond_broadcast@@GLIBC_2.17+0x24>
    cfbc:	1a9f17e0 	cset	w0, eq
    cfc0:	52800014 	mov	w20, #0x0                   	// #0
    cfc4:	340005e0 	cbz	w0, d080 <pthread_cond_broadcast@@GLIBC_2.17+0xfc>
    cfc8:	f9400660 	ldr	x0, [x19,#8]
    cfcc:	f9400a61 	ldr	x1, [x19,#16]
    cfd0:	eb01001f 	cmp	x0, x1
    cfd4:	54000629 	b.ls	d098 <pthread_cond_broadcast@@GLIBC_2.17+0x114>
    cfd8:	b9402e61 	ldr	w1, [x19,#44]
    cfdc:	531f7805 	lsl	w5, w0, #1
    cfe0:	f9000a60 	str	x0, [x19,#16]
    cfe4:	11000421 	add	w1, w1, #0x1
    cfe8:	f9000e60 	str	x0, [x19,#24]
    cfec:	b9002e61 	str	w1, [x19,#44]
    cff0:	52800001 	mov	w1, #0x0                   	// #0
    cff4:	b9000665 	str	w5, [x19,#4]
    cff8:	885f7e60 	ldxr	w0, [x19]
    cffc:	8802fe61 	stlxr	w2, w1, [x19]
    d000:	35ffffc2 	cbnz	w2, cff8 <pthread_cond_broadcast@@GLIBC_2.17+0x74>
    d004:	7100041f 	cmp	w0, #0x1
    d008:	540007ec 	b.gt	d104 <pthread_cond_broadcast@@GLIBC_2.17+0x180>
    d00c:	f9401264 	ldr	x4, [x19,#32]
    d010:	b100049f 	cmn	x4, #0x1
    d014:	540001e0 	b.eq	d050 <pthread_cond_broadcast@@GLIBC_2.17+0xcc>
    d018:	b9401080 	ldr	w0, [x4,#16]
    d01c:	373801a0 	tbnz	w0, #7, d050 <pthread_cond_broadcast@@GLIBC_2.17+0xcc>
    d020:	d1000481 	sub	x1, x4, #0x1
    d024:	b1000c3f 	cmn	x1, #0x3
    d028:	540004c9 	b.ls	d0c0 <pthread_cond_broadcast@@GLIBC_2.17+0x13c>
    d02c:	91001260 	add	x0, x19, #0x4
    d030:	d2801081 	mov	x1, #0x84                  	// #132
    d034:	d2800022 	mov	x2, #0x1                   	// #1
    d038:	b2407be3 	mov	x3, #0x7fffffff            	// #2147483647
    d03c:	93407ca5 	sxtw	x5, w5
    d040:	d2800c48 	mov	x8, #0x62                  	// #98
    d044:	d4000001 	svc	#0x0
    d048:	b140041f 	cmn	x0, #0x1, lsl #12
    d04c:	54000329 	b.ls	d0b0 <pthread_cond_broadcast@@GLIBC_2.17+0x12c>
    d050:	52801021 	mov	w1, #0x81                  	// #129
    d054:	91001260 	add	x0, x19, #0x4
    d058:	4a010281 	eor	w1, w20, w1
    d05c:	b2407be2 	mov	x2, #0x7fffffff            	// #2147483647
    d060:	d2800003 	mov	x3, #0x0                   	// #0
    d064:	d2800c48 	mov	x8, #0x62                  	// #98
    d068:	93407c21 	sxtw	x1, w1
    d06c:	d4000001 	svc	#0x0
    d070:	52800000 	mov	w0, #0x0                   	// #0
    d074:	a94153f3 	ldp	x19, x20, [sp,#16]
    d078:	a8c37bfd 	ldp	x29, x30, [sp],#48
    d07c:	d65f03c0 	ret
    d080:	b9002fa1 	str	w1, [x29,#44]
    d084:	2a0003f4 	mov	w20, w0
    d088:	aa1303e0 	mov	x0, x19
    d08c:	2a1403e1 	mov	w1, w20
    d090:	940009cd 	bl	f7c4 <__lll_lock_wait>
    d094:	17ffffcd 	b	cfc8 <pthread_cond_broadcast@@GLIBC_2.17+0x44>
    d098:	52800001 	mov	w1, #0x0                   	// #0
    d09c:	885f7e60 	ldxr	w0, [x19]
    d0a0:	8802fe61 	stlxr	w2, w1, [x19]
    d0a4:	35ffffc2 	cbnz	w2, d09c <pthread_cond_broadcast@@GLIBC_2.17+0x118>
    d0a8:	7100041f 	cmp	w0, #0x1
    d0ac:	540003ec 	b.gt	d128 <pthread_cond_broadcast@@GLIBC_2.17+0x1a4>
    d0b0:	52800000 	mov	w0, #0x0                   	// #0
    d0b4:	a94153f3 	ldp	x19, x20, [sp,#16]
    d0b8:	a8c37bfd 	ldp	x29, x30, [sp],#48
    d0bc:	d65f03c0 	ret
    d0c0:	121c0400 	and	w0, w0, #0x30
    d0c4:	7100801f 	cmp	w0, #0x20
    d0c8:	54fffb21 	b.ne	d02c <pthread_cond_broadcast@@GLIBC_2.17+0xa8>
    d0cc:	91001260 	add	x0, x19, #0x4
    d0d0:	d2801181 	mov	x1, #0x8c                  	// #140
    d0d4:	17ffffd8 	b	d034 <pthread_cond_broadcast@@GLIBC_2.17+0xb0>
    d0d8:	885ffe61 	ldaxr	w1, [x19]
    d0dc:	6b1f003f 	cmp	w1, wzr
    d0e0:	54000061 	b.ne	d0ec <pthread_cond_broadcast@@GLIBC_2.17+0x168>
    d0e4:	88027e60 	stxr	w2, w0, [x19]
    d0e8:	35ffff82 	cbnz	w2, d0d8 <pthread_cond_broadcast@@GLIBC_2.17+0x154>
    d0ec:	54000080 	b.eq	d0fc <pthread_cond_broadcast@@GLIBC_2.17+0x178>
    d0f0:	b9002fa1 	str	w1, [x29,#44]
    d0f4:	52801014 	mov	w20, #0x80                  	// #128
    d0f8:	17ffffe4 	b	d088 <pthread_cond_broadcast@@GLIBC_2.17+0x104>
    d0fc:	52801014 	mov	w20, #0x80                  	// #128
    d100:	17ffffb2 	b	cfc8 <pthread_cond_broadcast@@GLIBC_2.17+0x44>
    d104:	52801021 	mov	w1, #0x81                  	// #129
    d108:	aa1303e0 	mov	x0, x19
    d10c:	4a010281 	eor	w1, w20, w1
    d110:	d2800022 	mov	x2, #0x1                   	// #1
    d114:	d2800003 	mov	x3, #0x0                   	// #0
    d118:	d2800c48 	mov	x8, #0x62                  	// #98
    d11c:	93407c21 	sxtw	x1, w1
    d120:	d4000001 	svc	#0x0
    d124:	17ffffba 	b	d00c <pthread_cond_broadcast@@GLIBC_2.17+0x88>
    d128:	52801021 	mov	w1, #0x81                  	// #129
    d12c:	aa1303e0 	mov	x0, x19
    d130:	4a010281 	eor	w1, w20, w1
    d134:	d2800022 	mov	x2, #0x1                   	// #1
    d138:	d2800003 	mov	x3, #0x0                   	// #0
    d13c:	d2800c48 	mov	x8, #0x62                  	// #98
    d140:	93407c21 	sxtw	x1, w1
    d144:	d4000001 	svc	#0x0
    d148:	17ffffda 	b	d0b0 <pthread_cond_broadcast@@GLIBC_2.17+0x12c>

000000000000d14c <pthread_condattr_init>:
    d14c:	f900001f 	str	xzr, [x0]
    d150:	52800000 	mov	w0, #0x0                   	// #0
    d154:	d65f03c0 	ret

000000000000d158 <pthread_condattr_destroy>:
    d158:	52800000 	mov	w0, #0x0                   	// #0
    d15c:	d65f03c0 	ret

000000000000d160 <pthread_condattr_getpshared>:
    d160:	b9400002 	ldr	w2, [x0]
    d164:	52800000 	mov	w0, #0x0                   	// #0
    d168:	12000042 	and	w2, w2, #0x1
    d16c:	b9000022 	str	w2, [x1]
    d170:	d65f03c0 	ret

000000000000d174 <pthread_condattr_setpshared>:
    d174:	7100043f 	cmp	w1, #0x1
    d178:	54000108 	b.hi	d198 <pthread_condattr_setpshared+0x24>
    d17c:	b9400002 	ldr	w2, [x0]
    d180:	52800003 	mov	w3, #0x0                   	// #0
    d184:	121f7842 	and	w2, w2, #0xfffffffe
    d188:	2a020021 	orr	w1, w1, w2
    d18c:	b9000001 	str	w1, [x0]
    d190:	2a0303e0 	mov	w0, w3
    d194:	d65f03c0 	ret
    d198:	528002c3 	mov	w3, #0x16                  	// #22
    d19c:	2a0303e0 	mov	w0, w3
    d1a0:	d65f03c0 	ret

000000000000d1a4 <pthread_condattr_getclock>:
    d1a4:	b9400002 	ldr	w2, [x0]
    d1a8:	52800000 	mov	w0, #0x0                   	// #0
    d1ac:	d3410442 	ubfx	x2, x2, #1, #1
    d1b0:	b9000022 	str	w2, [x1]
    d1b4:	d65f03c0 	ret

000000000000d1b8 <pthread_condattr_setclock>:
    d1b8:	7100043f 	cmp	w1, #0x1
    d1bc:	aa0003e2 	mov	x2, x0
    d1c0:	528002c0 	mov	w0, #0x16                  	// #22
    d1c4:	54000049 	b.ls	d1cc <pthread_condattr_setclock+0x14>
    d1c8:	d65f03c0 	ret
    d1cc:	b9400043 	ldr	w3, [x2]
    d1d0:	52800000 	mov	w0, #0x0                   	// #0
    d1d4:	121e7863 	and	w3, w3, #0xfffffffd
    d1d8:	2a010461 	orr	w1, w3, w1, lsl #1
    d1dc:	b9000041 	str	w1, [x2]
    d1e0:	d65f03c0 	ret

000000000000d1e4 <pthread_spin_init>:
    d1e4:	aa0003e1 	mov	x1, x0
    d1e8:	52800000 	mov	w0, #0x0                   	// #0
    d1ec:	b9000020 	str	w0, [x1]
    d1f0:	d65f03c0 	ret

000000000000d1f4 <pthread_spin_destroy>:
    d1f4:	52800000 	mov	w0, #0x0                   	// #0
    d1f8:	d65f03c0 	ret

000000000000d1fc <pthread_spin_lock>:
    d1fc:	d10043ff 	sub	sp, sp, #0x10
    d200:	52800024 	mov	w4, #0x1                   	// #1
    d204:	885ffc01 	ldaxr	w1, [x0]
    d208:	88027c04 	stxr	w2, w4, [x0]
    d20c:	35ffffc2 	cbnz	w2, d204 <pthread_spin_lock+0x8>
    d210:	b90007e1 	str	w1, [sp,#4]
    d214:	b94007e1 	ldr	w1, [sp,#4]
    d218:	340002e1 	cbz	w1, d274 <pthread_spin_lock+0x78>
    d21c:	910033e3 	add	x3, sp, #0xc
    d220:	b9400001 	ldr	w1, [x0]
    d224:	340000e1 	cbz	w1, d240 <pthread_spin_lock+0x44>
    d228:	52807d01 	mov	w1, #0x3e8                 	// #1000
    d22c:	14000002 	b	d234 <pthread_spin_lock+0x38>
    d230:	34000081 	cbz	w1, d240 <pthread_spin_lock+0x44>
    d234:	b9400002 	ldr	w2, [x0]
    d238:	51000421 	sub	w1, w1, #0x1
    d23c:	35ffffa2 	cbnz	w2, d230 <pthread_spin_lock+0x34>
    d240:	b9000fff 	str	wzr, [sp,#12]
    d244:	b9400061 	ldr	w1, [x3]
    d248:	885ffc02 	ldaxr	w2, [x0]
    d24c:	6b01005f 	cmp	w2, w1
    d250:	54000061 	b.ne	d25c <pthread_spin_lock+0x60>
    d254:	88057c04 	stxr	w5, w4, [x0]
    d258:	35ffff85 	cbnz	w5, d248 <pthread_spin_lock+0x4c>
    d25c:	54000040 	b.eq	d264 <pthread_spin_lock+0x68>
    d260:	b9000062 	str	w2, [x3]
    d264:	b9400fe1 	ldr	w1, [sp,#12]
    d268:	b9000be1 	str	w1, [sp,#8]
    d26c:	b9400be1 	ldr	w1, [sp,#8]
    d270:	35fffd81 	cbnz	w1, d220 <pthread_spin_lock+0x24>
    d274:	52800000 	mov	w0, #0x0                   	// #0
    d278:	910043ff 	add	sp, sp, #0x10
    d27c:	d65f03c0 	ret

000000000000d280 <pthread_spin_trylock>:
    d280:	d10043ff 	sub	sp, sp, #0x10
    d284:	52800022 	mov	w2, #0x1                   	// #1
    d288:	885ffc01 	ldaxr	w1, [x0]
    d28c:	88037c02 	stxr	w3, w2, [x0]
    d290:	35ffffc3 	cbnz	w3, d288 <pthread_spin_trylock+0x8>
    d294:	b9000fe1 	str	w1, [sp,#12]
    d298:	b9400fe0 	ldr	w0, [sp,#12]
    d29c:	910043ff 	add	sp, sp, #0x10
    d2a0:	6b1f001f 	cmp	w0, wzr
    d2a4:	52800200 	mov	w0, #0x10                  	// #16
    d2a8:	1a9f1000 	csel	w0, w0, wzr, ne
    d2ac:	d65f03c0 	ret

000000000000d2b0 <pthread_spin_unlock>:
    d2b0:	aa0003e1 	mov	x1, x0
    d2b4:	d5033bbf 	dmb	ish
    d2b8:	52800000 	mov	w0, #0x0                   	// #0
    d2bc:	b9000020 	str	w0, [x1]
    d2c0:	d65f03c0 	ret

000000000000d2c4 <pthread_barrier_init>:
    d2c4:	340002c2 	cbz	w2, d31c <pthread_barrier_init+0x58>
    d2c8:	b40002e1 	cbz	x1, d324 <pthread_barrier_init+0x60>
    d2cc:	b9400021 	ldr	w1, [x1]
    d2d0:	34000161 	cbz	w1, d2fc <pthread_barrier_init+0x38>
    d2d4:	7100043f 	cmp	w1, #0x1
    d2d8:	54000221 	b.ne	d31c <pthread_barrier_init+0x58>
    d2dc:	52800003 	mov	w3, #0x0                   	// #0
    d2e0:	b900041f 	str	wzr, [x0,#4]
    d2e4:	b9000802 	str	w2, [x0,#8]
    d2e8:	b9000c02 	str	w2, [x0,#12]
    d2ec:	b9000003 	str	w3, [x0]
    d2f0:	b9001003 	str	w3, [x0,#16]
    d2f4:	52800000 	mov	w0, #0x0                   	// #0
    d2f8:	d65f03c0 	ret
    d2fc:	b9000401 	str	w1, [x0,#4]
    d300:	52801003 	mov	w3, #0x80                  	// #128
    d304:	b9000802 	str	w2, [x0,#8]
    d308:	b9000c02 	str	w2, [x0,#12]
    d30c:	b9000001 	str	w1, [x0]
    d310:	b9001003 	str	w3, [x0,#16]
    d314:	52800000 	mov	w0, #0x0                   	// #0
    d318:	d65f03c0 	ret
    d31c:	528002c0 	mov	w0, #0x16                  	// #22
    d320:	d65f03c0 	ret
    d324:	b0000021 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
    d328:	912ee021 	add	x1, x1, #0xbb8
    d32c:	17ffffe8 	b	d2cc <pthread_barrier_init+0x8>

000000000000d330 <pthread_barrier_destroy>:
    d330:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    d334:	910003fd 	mov	x29, sp
    d338:	a90153f3 	stp	x19, x20, [sp,#16]
    d33c:	aa0003f3 	mov	x19, x0
    d340:	91001014 	add	x20, x0, #0x4
    d344:	b9002fbf 	str	wzr, [x29,#44]
    d348:	52800020 	mov	w0, #0x1                   	// #1
    d34c:	885ffe82 	ldaxr	w2, [x20]
    d350:	6b1f005f 	cmp	w2, wzr
    d354:	54000061 	b.ne	d360 <pthread_barrier_destroy+0x30>
    d358:	88017e80 	stxr	w1, w0, [x20]
    d35c:	35ffff81 	cbnz	w1, d34c <pthread_barrier_destroy+0x1c>
    d360:	54000141 	b.ne	d388 <pthread_barrier_destroy+0x58>
    d364:	b9400a61 	ldr	w1, [x19,#8]
    d368:	52800004 	mov	w4, #0x0                   	// #0
    d36c:	b9400e60 	ldr	w0, [x19,#12]
    d370:	6b00003f 	cmp	w1, w0
    d374:	540001e1 	b.ne	d3b0 <pthread_barrier_destroy+0x80>
    d378:	2a0403e0 	mov	w0, w4
    d37c:	a94153f3 	ldp	x19, x20, [sp,#16]
    d380:	a8c37bfd 	ldp	x29, x30, [sp],#48
    d384:	d65f03c0 	ret
    d388:	b9401261 	ldr	w1, [x19,#16]
    d38c:	aa1403e0 	mov	x0, x20
    d390:	b9002fa2 	str	w2, [x29,#44]
    d394:	52190021 	eor	w1, w1, #0x80
    d398:	9400090b 	bl	f7c4 <__lll_lock_wait>
    d39c:	b9400a61 	ldr	w1, [x19,#8]
    d3a0:	52800004 	mov	w4, #0x0                   	// #0
    d3a4:	b9400e60 	ldr	w0, [x19,#12]
    d3a8:	6b00003f 	cmp	w1, w0
    d3ac:	54fffe60 	b.eq	d378 <pthread_barrier_destroy+0x48>
    d3b0:	885f7e80 	ldxr	w0, [x20]
    d3b4:	8801fe84 	stlxr	w1, w4, [x20]
    d3b8:	35ffffc1 	cbnz	w1, d3b0 <pthread_barrier_destroy+0x80>
    d3bc:	7100041f 	cmp	w0, #0x1
    d3c0:	52800204 	mov	w4, #0x10                  	// #16
    d3c4:	54fffdad 	b.le	d378 <pthread_barrier_destroy+0x48>
    d3c8:	b9401261 	ldr	w1, [x19,#16]
    d3cc:	aa1403e0 	mov	x0, x20
    d3d0:	d2800022 	mov	x2, #0x1                   	// #1
    d3d4:	d2800003 	mov	x3, #0x0                   	// #0
    d3d8:	52000021 	eor	w1, w1, #0x1
    d3dc:	d2800c48 	mov	x8, #0x62                  	// #98
    d3e0:	93407c21 	sxtw	x1, w1
    d3e4:	d4000001 	svc	#0x0
    d3e8:	17ffffe4 	b	d378 <pthread_barrier_destroy+0x48>

000000000000d3ec <pthread_barrier_wait>:
    d3ec:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    d3f0:	910003fd 	mov	x29, sp
    d3f4:	a90153f3 	stp	x19, x20, [sp,#16]
    d3f8:	aa0003f3 	mov	x19, x0
    d3fc:	91001014 	add	x20, x0, #0x4
    d400:	b9002fbf 	str	wzr, [x29,#44]
    d404:	52800020 	mov	w0, #0x1                   	// #1
    d408:	885ffe82 	ldaxr	w2, [x20]
    d40c:	6b1f005f 	cmp	w2, wzr
    d410:	54000061 	b.ne	d41c <pthread_barrier_wait+0x30>
    d414:	88017e80 	stxr	w1, w0, [x20]
    d418:	35ffff81 	cbnz	w1, d408 <pthread_barrier_wait+0x1c>
    d41c:	54000481 	b.ne	d4ac <pthread_barrier_wait+0xc0>
    d420:	b9400a60 	ldr	w0, [x19,#8]
    d424:	51000400 	sub	w0, w0, #0x1
    d428:	b9000a60 	str	w0, [x19,#8]
    d42c:	340007c0 	cbz	w0, d524 <pthread_barrier_wait+0x138>
    d430:	b9400264 	ldr	w4, [x19]
    d434:	52800001 	mov	w1, #0x0                   	// #0
    d438:	885f7e80 	ldxr	w0, [x20]
    d43c:	8802fe81 	stlxr	w2, w1, [x20]
    d440:	35ffffc2 	cbnz	w2, d438 <pthread_barrier_wait+0x4c>
    d444:	7100041f 	cmp	w0, #0x1
    d448:	540003ec 	b.gt	d4c4 <pthread_barrier_wait+0xd8>
    d44c:	2a0403e5 	mov	w5, w4
    d450:	aa1303e0 	mov	x0, x19
    d454:	b9801261 	ldrsw	x1, [x19,#16]
    d458:	aa0503e2 	mov	x2, x5
    d45c:	d2800003 	mov	x3, #0x0                   	// #0
    d460:	d2800c48 	mov	x8, #0x62                  	// #98
    d464:	d4000001 	svc	#0x0
    d468:	b9400261 	ldr	w1, [x19]
    d46c:	6b04003f 	cmp	w1, w4
    d470:	54ffff00 	b.eq	d450 <pthread_barrier_wait+0x64>
    d474:	2a0303e4 	mov	w4, w3
    d478:	b9400e60 	ldr	w0, [x19,#12]
    d47c:	91002262 	add	x2, x19, #0x8
    d480:	885ffc41 	ldaxr	w1, [x2]
    d484:	11000423 	add	w3, w1, #0x1
    d488:	88057c43 	stxr	w5, w3, [x2]
    d48c:	35ffffa5 	cbnz	w5, d480 <pthread_barrier_wait+0x94>
    d490:	11000421 	add	w1, w1, #0x1
    d494:	6b00003f 	cmp	w1, w0
    d498:	54000280 	b.eq	d4e8 <pthread_barrier_wait+0xfc>
    d49c:	2a0403e0 	mov	w0, w4
    d4a0:	a94153f3 	ldp	x19, x20, [sp,#16]
    d4a4:	a8c37bfd 	ldp	x29, x30, [sp],#48
    d4a8:	d65f03c0 	ret
    d4ac:	b9401261 	ldr	w1, [x19,#16]
    d4b0:	aa1403e0 	mov	x0, x20
    d4b4:	b9002fa2 	str	w2, [x29,#44]
    d4b8:	52190021 	eor	w1, w1, #0x80
    d4bc:	940008c2 	bl	f7c4 <__lll_lock_wait>
    d4c0:	17ffffd8 	b	d420 <pthread_barrier_wait+0x34>
    d4c4:	b9401261 	ldr	w1, [x19,#16]
    d4c8:	aa1403e0 	mov	x0, x20
    d4cc:	d2800022 	mov	x2, #0x1                   	// #1
    d4d0:	d2800003 	mov	x3, #0x0                   	// #0
    d4d4:	52000021 	eor	w1, w1, #0x1
    d4d8:	d2800c48 	mov	x8, #0x62                  	// #98
    d4dc:	93407c21 	sxtw	x1, w1
    d4e0:	d4000001 	svc	#0x0
    d4e4:	17ffffda 	b	d44c <pthread_barrier_wait+0x60>
    d4e8:	52800001 	mov	w1, #0x0                   	// #0
    d4ec:	885f7e80 	ldxr	w0, [x20]
    d4f0:	8802fe81 	stlxr	w2, w1, [x20]
    d4f4:	35ffffc2 	cbnz	w2, d4ec <pthread_barrier_wait+0x100>
    d4f8:	7100041f 	cmp	w0, #0x1
    d4fc:	54fffd0d 	b.le	d49c <pthread_barrier_wait+0xb0>
    d500:	b9401261 	ldr	w1, [x19,#16]
    d504:	aa1403e0 	mov	x0, x20
    d508:	d2800022 	mov	x2, #0x1                   	// #1
    d50c:	d2800003 	mov	x3, #0x0                   	// #0
    d510:	52000021 	eor	w1, w1, #0x1
    d514:	d2800c48 	mov	x8, #0x62                  	// #98
    d518:	93407c21 	sxtw	x1, w1
    d51c:	d4000001 	svc	#0x0
    d520:	17ffffdf 	b	d49c <pthread_barrier_wait+0xb0>
    d524:	b9401261 	ldr	w1, [x19,#16]
    d528:	aa1303e0 	mov	x0, x19
    d52c:	b9400264 	ldr	w4, [x19]
    d530:	b2407be2 	mov	x2, #0x7fffffff            	// #2147483647
    d534:	52000021 	eor	w1, w1, #0x1
    d538:	d2800003 	mov	x3, #0x0                   	// #0
    d53c:	11000484 	add	w4, w4, #0x1
    d540:	d2800c48 	mov	x8, #0x62                  	// #98
    d544:	b9000264 	str	w4, [x19]
    d548:	93407c21 	sxtw	x1, w1
    d54c:	d4000001 	svc	#0x0
    d550:	12800004 	mov	w4, #0xffffffff            	// #-1
    d554:	17ffffc9 	b	d478 <pthread_barrier_wait+0x8c>

000000000000d558 <pthread_barrierattr_init>:
    d558:	b900001f 	str	wzr, [x0]
    d55c:	52800000 	mov	w0, #0x0                   	// #0
    d560:	d65f03c0 	ret

000000000000d564 <pthread_barrierattr_destroy>:
    d564:	52800000 	mov	w0, #0x0                   	// #0
    d568:	d65f03c0 	ret

000000000000d56c <pthread_barrierattr_getpshared>:
    d56c:	b9400002 	ldr	w2, [x0]
    d570:	52800000 	mov	w0, #0x0                   	// #0
    d574:	b9000022 	str	w2, [x1]
    d578:	d65f03c0 	ret

000000000000d57c <pthread_barrierattr_setpshared>:
    d57c:	7100043f 	cmp	w1, #0x1
    d580:	54000088 	b.hi	d590 <pthread_barrierattr_setpshared+0x14>
    d584:	b9000001 	str	w1, [x0]
    d588:	52800000 	mov	w0, #0x0                   	// #0
    d58c:	d65f03c0 	ret
    d590:	528002c0 	mov	w0, #0x16                  	// #22
    d594:	d65f03c0 	ret

000000000000d598 <__pthread_key_create>:
    d598:	f0000108 	adrp	x8, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    d59c:	d10043ff 	sub	sp, sp, #0x10
    d5a0:	910a2103 	add	x3, x8, #0x288
    d5a4:	d2800004 	mov	x4, #0x0                   	// #0
    d5a8:	14000005 	b	d5bc <__pthread_key_create+0x24>
    d5ac:	91000484 	add	x4, x4, #0x1
    d5b0:	91004063 	add	x3, x3, #0x10
    d5b4:	f110009f 	cmp	x4, #0x400
    d5b8:	54000280 	b.eq	d608 <__pthread_key_create+0x70>
    d5bc:	f9400062 	ldr	x2, [x3]
    d5c0:	91000845 	add	x5, x2, #0x2
    d5c4:	92400046 	and	x6, x2, #0x1
    d5c8:	eb05005f 	cmp	x2, x5
    d5cc:	3707ff02 	tbnz	w2, #0, d5ac <__pthread_key_create+0x14>
    d5d0:	54fffee2 	b.cs	d5ac <__pthread_key_create+0x14>
    d5d4:	f90007e2 	str	x2, [sp,#8]
    d5d8:	91000445 	add	x5, x2, #0x1
    d5dc:	c85ffc67 	ldaxr	x7, [x3]
    d5e0:	eb0200ff 	cmp	x7, x2
    d5e4:	54000061 	b.ne	d5f0 <__pthread_key_create+0x58>
    d5e8:	c8097c65 	stxr	w9, x5, [x3]
    d5ec:	35ffff89 	cbnz	w9, d5dc <__pthread_key_create+0x44>
    d5f0:	54fffde1 	b.ne	d5ac <__pthread_key_create+0x14>
    d5f4:	910a2108 	add	x8, x8, #0x288
    d5f8:	8b041108 	add	x8, x8, x4, lsl #4
    d5fc:	f9000501 	str	x1, [x8,#8]
    d600:	b9000004 	str	w4, [x0]
    d604:	14000002 	b	d60c <__pthread_key_create+0x74>
    d608:	52800166 	mov	w6, #0xb                   	// #11
    d60c:	2a0603e0 	mov	w0, w6
    d610:	910043ff 	add	sp, sp, #0x10
    d614:	d65f03c0 	ret

000000000000d618 <pthread_key_delete>:
    d618:	710ffc1f 	cmp	w0, #0x3ff
    d61c:	d10043ff 	sub	sp, sp, #0x10
    d620:	54000208 	b.hi	d660 <pthread_key_delete+0x48>
    d624:	f0000101 	adrp	x1, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    d628:	d37c7c00 	ubfiz	x0, x0, #4, #32
    d62c:	910a2021 	add	x1, x1, #0x288
    d630:	8b000022 	add	x2, x1, x0
    d634:	f8606820 	ldr	x0, [x1,x0]
    d638:	36000140 	tbz	w0, #0, d660 <pthread_key_delete+0x48>
    d63c:	92407c01 	and	x1, x0, #0xffffffff
    d640:	11000400 	add	w0, w0, #0x1
    d644:	f90007e1 	str	x1, [sp,#8]
    d648:	c85ffc43 	ldaxr	x3, [x2]
    d64c:	eb01007f 	cmp	x3, x1
    d650:	54000061 	b.ne	d65c <pthread_key_delete+0x44>
    d654:	c8047c40 	stxr	w4, x0, [x2]
    d658:	35ffff84 	cbnz	w4, d648 <pthread_key_delete+0x30>
    d65c:	54000080 	b.eq	d66c <pthread_key_delete+0x54>
    d660:	528002c0 	mov	w0, #0x16                  	// #22
    d664:	910043ff 	add	sp, sp, #0x10
    d668:	d65f03c0 	ret
    d66c:	52800000 	mov	w0, #0x0                   	// #0
    d670:	17fffffd 	b	d664 <pthread_key_delete+0x4c>

000000000000d674 <__pthread_getspecific>:
    d674:	71007c1f 	cmp	w0, #0x1f
    d678:	540002a8 	b.hi	d6cc <__pthread_getspecific+0x58>
    d67c:	2a0003e1 	mov	w1, w0
    d680:	d53bd042 	mrs	x2, tpidr_el0
    d684:	8b011041 	add	x1, x2, x1, lsl #4
    d688:	d1178021 	sub	x1, x1, #0x5e0
    d68c:	f9400423 	ldr	x3, [x1,#8]
    d690:	b40001a3 	cbz	x3, d6c4 <__pthread_getspecific+0x50>
    d694:	d37c7c02 	ubfiz	x2, x0, #4, #32
    d698:	aa0303e0 	mov	x0, x3
    d69c:	f0000103 	adrp	x3, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    d6a0:	f9400024 	ldr	x4, [x1]
    d6a4:	910a2063 	add	x3, x3, #0x288
    d6a8:	f8626862 	ldr	x2, [x3,x2]
    d6ac:	eb02009f 	cmp	x4, x2
    d6b0:	54000041 	b.ne	d6b8 <__pthread_getspecific+0x44>
    d6b4:	d65f03c0 	ret
    d6b8:	f900043f 	str	xzr, [x1,#8]
    d6bc:	d2800000 	mov	x0, #0x0                   	// #0
    d6c0:	d65f03c0 	ret
    d6c4:	aa0303e0 	mov	x0, x3
    d6c8:	d65f03c0 	ret
    d6cc:	710ffc1f 	cmp	w0, #0x3ff
    d6d0:	54000168 	b.hi	d6fc <__pthread_getspecific+0x88>
    d6d4:	d3457c01 	ubfx	x1, x0, #5, #27
    d6d8:	d53bd043 	mrs	x3, tpidr_el0
    d6dc:	d11bc063 	sub	x3, x3, #0x6f0
    d6e0:	91018821 	add	x1, x1, #0x62
    d6e4:	12001002 	and	w2, w0, #0x1f
    d6e8:	f8617861 	ldr	x1, [x3,x1,lsl #3]
    d6ec:	b40000c1 	cbz	x1, d704 <__pthread_getspecific+0x90>
    d6f0:	d37c1042 	ubfiz	x2, x2, #4, #5
    d6f4:	8b020021 	add	x1, x1, x2
    d6f8:	17ffffe5 	b	d68c <__pthread_getspecific+0x18>
    d6fc:	d2800000 	mov	x0, #0x0                   	// #0
    d700:	d65f03c0 	ret
    d704:	aa0103e0 	mov	x0, x1
    d708:	d65f03c0 	ret

000000000000d70c <__pthread_setspecific>:
    d70c:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    d710:	71007c1f 	cmp	w0, #0x1f
    d714:	910003fd 	mov	x29, sp
    d718:	a90153f3 	stp	x19, x20, [sp,#16]
    d71c:	a9025bf5 	stp	x21, x22, [sp,#32]
    d720:	f9001bf7 	str	x23, [sp,#48]
    d724:	aa0103f6 	mov	x22, x1
    d728:	d53bd054 	mrs	x20, tpidr_el0
    d72c:	540003c8 	b.hi	d7a4 <__pthread_setspecific+0x98>
    d730:	2a0003e2 	mov	w2, w0
    d734:	f0000103 	adrp	x3, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    d738:	d37cec40 	lsl	x0, x2, #4
    d73c:	910a2063 	add	x3, x3, #0x288
    d740:	528002c4 	mov	w4, #0x16                  	// #22
    d744:	f8606863 	ldr	x3, [x3,x0]
    d748:	2a0303f5 	mov	w21, w3
    d74c:	370000e3 	tbnz	w3, #0, d768 <__pthread_setspecific+0x5c>
    d750:	2a0403e0 	mov	w0, w4
    d754:	f9401bf7 	ldr	x23, [sp,#48]
    d758:	a94153f3 	ldp	x19, x20, [sp,#16]
    d75c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    d760:	a8c47bfd 	ldp	x29, x30, [sp],#64
    d764:	d65f03c0 	ret
    d768:	91004440 	add	x0, x2, #0x11
    d76c:	d11bc294 	sub	x20, x20, #0x6f0
    d770:	8b001280 	add	x0, x20, x0, lsl #4
    d774:	b4000061 	cbz	x1, d780 <__pthread_setspecific+0x74>
    d778:	52800021 	mov	w1, #0x1                   	// #1
    d77c:	39104281 	strb	w1, [x20,#1040]
    d780:	f9000416 	str	x22, [x0,#8]
    d784:	f9000015 	str	x21, [x0]
    d788:	52800004 	mov	w4, #0x0                   	// #0
    d78c:	f9401bf7 	ldr	x23, [sp,#48]
    d790:	2a0403e0 	mov	w0, w4
    d794:	a94153f3 	ldp	x19, x20, [sp,#16]
    d798:	a9425bf5 	ldp	x21, x22, [sp,#32]
    d79c:	a8c47bfd 	ldp	x29, x30, [sp],#64
    d7a0:	d65f03c0 	ret
    d7a4:	710ffc1f 	cmp	w0, #0x3ff
    d7a8:	528002c4 	mov	w4, #0x16                  	// #22
    d7ac:	54fffd28 	b.hi	d750 <__pthread_setspecific+0x44>
    d7b0:	f0000102 	adrp	x2, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
    d7b4:	d37c7c01 	ubfiz	x1, x0, #4, #32
    d7b8:	910a2042 	add	x2, x2, #0x288
    d7bc:	f8616841 	ldr	x1, [x2,x1]
    d7c0:	2a0103f5 	mov	w21, w1
    d7c4:	3607fc61 	tbz	w1, #0, d750 <__pthread_setspecific+0x44>
    d7c8:	d3457c17 	ubfx	x23, x0, #5, #27
    d7cc:	12001013 	and	w19, w0, #0x1f
    d7d0:	8b170e97 	add	x23, x20, x23, lsl #3
    d7d4:	d10f82f7 	sub	x23, x23, #0x3e0
    d7d8:	f94002e0 	ldr	x0, [x23]
    d7dc:	b40000a0 	cbz	x0, d7f0 <__pthread_setspecific+0xe4>
    d7e0:	d37c1273 	ubfiz	x19, x19, #4, #5
    d7e4:	d11bc294 	sub	x20, x20, #0x6f0
    d7e8:	8b130000 	add	x0, x0, x19
    d7ec:	17ffffe3 	b	d778 <__pthread_setspecific+0x6c>
    d7f0:	b4fffcd6 	cbz	x22, d788 <__pthread_setspecific+0x7c>
    d7f4:	d2800400 	mov	x0, #0x20                  	// #32
    d7f8:	d2800201 	mov	x1, #0x10                  	// #16
    d7fc:	97ffde3d 	bl	50f0 <calloc@plt>
    d800:	b4000060 	cbz	x0, d80c <__pthread_setspecific+0x100>
    d804:	f90002e0 	str	x0, [x23]
    d808:	17fffff6 	b	d7e0 <__pthread_setspecific+0xd4>
    d80c:	52800184 	mov	w4, #0xc                   	// #12
    d810:	17ffffd0 	b	d750 <__pthread_setspecific+0x44>

000000000000d814 <pthread_sigmask>:
    d814:	a9b67bfd 	stp	x29, x30, [sp,#-160]!
    d818:	aa0103e3 	mov	x3, x1
    d81c:	910003fd 	mov	x29, sp
    d820:	a90153f3 	stp	x19, x20, [sp,#16]
    d824:	2a0003f4 	mov	w20, w0
    d828:	aa0203f3 	mov	x19, x2
    d82c:	b4000081 	cbz	x1, d83c <pthread_sigmask+0x28>
    d830:	f9400020 	ldr	x0, [x1]
    d834:	f261041f 	tst	x0, #0x180000000
    d838:	54000181 	b.ne	d868 <pthread_sigmask+0x54>
    d83c:	aa0303e1 	mov	x1, x3
    d840:	93407e80 	sxtw	x0, w20
    d844:	aa1303e2 	mov	x2, x19
    d848:	d2800103 	mov	x3, #0x8                   	// #8
    d84c:	d28010e8 	mov	x8, #0x87                  	// #135
    d850:	d4000001 	svc	#0x0
    d854:	3140041f 	cmn	w0, #0x1, lsl #12
    d858:	5a8097e0 	csneg	w0, wzr, w0, ls
    d85c:	a94153f3 	ldp	x19, x20, [sp,#16]
    d860:	a8ca7bfd 	ldp	x29, x30, [sp],#160
    d864:	d65f03c0 	ret
    d868:	910083a3 	add	x3, x29, #0x20
    d86c:	d2801002 	mov	x2, #0x80                  	// #128
    d870:	aa0303e0 	mov	x0, x3
    d874:	97ffddbb 	bl	4f60 <memcpy@plt>
    d878:	aa0003e1 	mov	x1, x0
    d87c:	f94013a0 	ldr	x0, [x29,#32]
    d880:	925ff400 	and	x0, x0, #0xfffffffe7fffffff
    d884:	f90013a0 	str	x0, [x29,#32]
    d888:	17ffffee 	b	d840 <pthread_sigmask+0x2c>

000000000000d88c <pthread_kill>:
    d88c:	b940d003 	ldr	w3, [x0,#208]
    d890:	93407c22 	sxtw	x2, w1
    d894:	2a0303e1 	mov	w1, w3
    d898:	6b1f003f 	cmp	w1, wzr
    d89c:	540001cd 	b.le	d8d4 <pthread_kill+0x48>
    d8a0:	51008044 	sub	w4, w2, #0x20
    d8a4:	528002c0 	mov	w0, #0x16                  	// #22
    d8a8:	7100049f 	cmp	w4, #0x1
    d8ac:	54000129 	b.ls	d8d0 <pthread_kill+0x44>
    d8b0:	d53bd040 	mrs	x0, tpidr_el0
    d8b4:	93407c21 	sxtw	x1, w1
    d8b8:	d11bc000 	sub	x0, x0, #0x6f0
    d8bc:	d2801068 	mov	x8, #0x83                  	// #131
    d8c0:	b980d400 	ldrsw	x0, [x0,#212]
    d8c4:	d4000001 	svc	#0x0
    d8c8:	3140041f 	cmn	w0, #0x1, lsl #12
    d8cc:	5a8097e0 	csneg	w0, wzr, w0, ls
    d8d0:	d65f03c0 	ret
    d8d4:	52800060 	mov	w0, #0x3                   	// #3
    d8d8:	d65f03c0 	ret

000000000000d8dc <pthread_sigqueue>:
    d8dc:	a9b57bfd 	stp	x29, x30, [sp,#-176]!
    d8e0:	910003fd 	mov	x29, sp
    d8e4:	a90153f3 	stp	x19, x20, [sp,#16]
    d8e8:	a9025bf5 	stp	x21, x22, [sp,#32]
    d8ec:	b940d013 	ldr	w19, [x0,#208]
    d8f0:	6b1f027f 	cmp	w19, wzr
    d8f4:	540004ad 	b.le	d988 <pthread_sigqueue+0xac>
    d8f8:	51008023 	sub	w3, w1, #0x20
    d8fc:	528002c0 	mov	w0, #0x16                  	// #22
    d900:	7100047f 	cmp	w3, #0x1
    d904:	540003a9 	b.ls	d978 <pthread_sigqueue+0x9c>
    d908:	d53bd056 	mrs	x22, tpidr_el0
    d90c:	2a0103f4 	mov	w20, w1
    d910:	d11bc2d6 	sub	x22, x22, #0x6f0
    d914:	aa0203f5 	mov	x21, x2
    d918:	a9037fbf 	stp	xzr, xzr, [x29,#48]
    d91c:	b90033a1 	str	w1, [x29,#48]
    d920:	12800001 	mov	w1, #0xffffffff            	// #-1
    d924:	b940d6c0 	ldr	w0, [x22,#212]
    d928:	a9047fbf 	stp	xzr, xzr, [x29,#64]
    d92c:	b9003ba1 	str	w1, [x29,#56]
    d930:	a9057fbf 	stp	xzr, xzr, [x29,#80]
    d934:	a9067fbf 	stp	xzr, xzr, [x29,#96]
    d938:	a9077fbf 	stp	xzr, xzr, [x29,#112]
    d93c:	a9087fbf 	stp	xzr, xzr, [x29,#128]
    d940:	a9097fbf 	stp	xzr, xzr, [x29,#144]
    d944:	a90a7fbf 	stp	xzr, xzr, [x29,#160]
    d948:	b90043a0 	str	w0, [x29,#64]
    d94c:	97ffddb9 	bl	5030 <getuid@plt>
    d950:	93407e61 	sxtw	x1, w19
    d954:	b90047a0 	str	w0, [x29,#68]
    d958:	f90027b5 	str	x21, [x29,#72]
    d95c:	93407e82 	sxtw	x2, w20
    d960:	b980d6c0 	ldrsw	x0, [x22,#212]
    d964:	9100c3a3 	add	x3, x29, #0x30
    d968:	d2801e08 	mov	x8, #0xf0                  	// #240
    d96c:	d4000001 	svc	#0x0
    d970:	3140041f 	cmn	w0, #0x1, lsl #12
    d974:	5a8097e0 	csneg	w0, wzr, w0, ls
    d978:	a94153f3 	ldp	x19, x20, [sp,#16]
    d97c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    d980:	a8cb7bfd 	ldp	x29, x30, [sp],#176
    d984:	d65f03c0 	ret
    d988:	52800060 	mov	w0, #0x3                   	// #3
    d98c:	a94153f3 	ldp	x19, x20, [sp,#16]
    d990:	a9425bf5 	ldp	x21, x22, [sp,#32]
    d994:	a8cb7bfd 	ldp	x29, x30, [sp],#176
    d998:	d65f03c0 	ret

000000000000d99c <pthread_cancel>:
    d99c:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    d9a0:	910003fd 	mov	x29, sp
    d9a4:	b940d001 	ldr	w1, [x0,#208]
    d9a8:	f9000bf3 	str	x19, [sp,#16]
    d9ac:	6b1f003f 	cmp	w1, wzr
    d9b0:	5400078d 	b.le	daa0 <pthread_cancel+0x104>
    d9b4:	aa0003f3 	mov	x19, x0
    d9b8:	94000fb5 	bl	1188c <pthread_cancel_init>
    d9bc:	d53bd047 	mrs	x7, tpidr_el0
    d9c0:	f0000126 	adrp	x6, 34000 <__GI___pthread_keys+0x3d78>
    d9c4:	12800885 	mov	w5, #0xffffffbb            	// #-69
    d9c8:	d11bc0e7 	sub	x7, x7, #0x6f0
    d9cc:	52800023 	mov	w3, #0x1                   	// #1
    d9d0:	f0000128 	adrp	x8, 34000 <__GI___pthread_keys+0x3d78>
    d9d4:	910ca0c6 	add	x6, x6, #0x328
    d9d8:	b9410a61 	ldr	w1, [x19,#264]
    d9dc:	91042260 	add	x0, x19, #0x108
    d9e0:	321e0422 	orr	w2, w1, #0xc
    d9e4:	6b02003f 	cmp	w1, w2
    d9e8:	0a050044 	and	w4, w2, w5
    d9ec:	540001e0 	b.eq	da28 <pthread_cancel+0x8c>
    d9f0:	7100289f 	cmp	w4, #0xa
    d9f4:	54000260 	b.eq	da40 <pthread_cancel+0xa4>
    d9f8:	b90000e3 	str	w3, [x7]
    d9fc:	f941b904 	ldr	x4, [x8,#880]
    da00:	b9000083 	str	w3, [x4]
    da04:	b9002fa1 	str	w1, [x29,#44]
    da08:	b90000c3 	str	w3, [x6]
    da0c:	b9402fa1 	ldr	w1, [x29,#44]
    da10:	885ffc04 	ldaxr	w4, [x0]
    da14:	6b01009f 	cmp	w4, w1
    da18:	54000061 	b.ne	da24 <pthread_cancel+0x88>
    da1c:	88097c02 	stxr	w9, w2, [x0]
    da20:	35ffff89 	cbnz	w9, da10 <pthread_cancel+0x74>
    da24:	540000a1 	b.ne	da38 <pthread_cancel+0x9c>
    da28:	52800000 	mov	w0, #0x0                   	// #0
    da2c:	f9400bf3 	ldr	x19, [sp,#16]
    da30:	a8c37bfd 	ldp	x29, x30, [sp],#48
    da34:	d65f03c0 	ret
    da38:	b9002fa4 	str	w4, [x29,#44]
    da3c:	17ffffe7 	b	d9d8 <pthread_cancel+0x3c>
    da40:	b9002ba1 	str	w1, [x29,#40]
    da44:	91042260 	add	x0, x19, #0x108
    da48:	321e0021 	orr	w1, w1, #0x4
    da4c:	b9402ba2 	ldr	w2, [x29,#40]
    da50:	885ffc04 	ldaxr	w4, [x0]
    da54:	6b02009f 	cmp	w4, w2
    da58:	54000061 	b.ne	da64 <pthread_cancel+0xc8>
    da5c:	88097c01 	stxr	w9, w1, [x0]
    da60:	35ffff89 	cbnz	w9, da50 <pthread_cancel+0xb4>
    da64:	540001a1 	b.ne	da98 <pthread_cancel+0xfc>
    da68:	d53bd040 	mrs	x0, tpidr_el0
    da6c:	b940d261 	ldr	w1, [x19,#208]
    da70:	d11bc000 	sub	x0, x0, #0x6f0
    da74:	d2800402 	mov	x2, #0x20                  	// #32
    da78:	d2801068 	mov	x8, #0x83                  	// #131
    da7c:	93407c21 	sxtw	x1, w1
    da80:	b980d400 	ldrsw	x0, [x0,#212]
    da84:	d4000001 	svc	#0x0
    da88:	3140041f 	cmn	w0, #0x1, lsl #12
    da8c:	54fffce9 	b.ls	da28 <pthread_cancel+0x8c>
    da90:	4b0003e0 	neg	w0, w0
    da94:	17ffffe6 	b	da2c <pthread_cancel+0x90>
    da98:	b9002ba4 	str	w4, [x29,#40]
    da9c:	17ffffcf 	b	d9d8 <pthread_cancel+0x3c>
    daa0:	52800060 	mov	w0, #0x3                   	// #3
    daa4:	f9400bf3 	ldr	x19, [sp,#16]
    daa8:	a8c37bfd 	ldp	x29, x30, [sp],#48
    daac:	d65f03c0 	ret

000000000000dab0 <pthread_testcancel>:
    dab0:	d53bd042 	mrs	x2, tpidr_el0
    dab4:	128008c0 	mov	w0, #0xffffffb9            	// #-71
    dab8:	d11bc041 	sub	x1, x2, #0x6f0
    dabc:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    dac0:	910003fd 	mov	x29, sp
    dac4:	b9410823 	ldr	w3, [x1,#264]
    dac8:	0a000060 	and	w0, w3, w0
    dacc:	7100201f 	cmp	w0, #0x8
    dad0:	54000060 	b.eq	dadc <pthread_testcancel+0x2c>
    dad4:	a8c27bfd 	ldp	x29, x30, [sp],#32
    dad8:	d65f03c0 	ret
    dadc:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    dae0:	91042023 	add	x3, x1, #0x108
    dae4:	f9021420 	str	x0, [x1,#1064]
    dae8:	d117a042 	sub	x2, x2, #0x5e8
    daec:	910073a4 	add	x4, x29, #0x1c
    daf0:	b9400040 	ldr	w0, [x2]
    daf4:	b9001fa0 	str	w0, [x29,#28]
    daf8:	321c0006 	orr	w6, w0, #0x10
    dafc:	885ffc65 	ldaxr	w5, [x3]
    db00:	6b0000bf 	cmp	w5, w0
    db04:	54000061 	b.ne	db10 <pthread_testcancel+0x60>
    db08:	88077c66 	stxr	w7, w6, [x3]
    db0c:	35ffff87 	cbnz	w7, dafc <pthread_testcancel+0x4c>
    db10:	54000060 	b.eq	db1c <pthread_testcancel+0x6c>
    db14:	b9000085 	str	w5, [x4]
    db18:	17fffff6 	b	daf0 <pthread_testcancel+0x40>
    db1c:	f9408020 	ldr	x0, [x1,#256]
    db20:	94000679 	bl	f504 <__pthread_unwind>

000000000000db24 <pthread_setcancelstate>:
    db24:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    db28:	7100041f 	cmp	w0, #0x1
    db2c:	528002c0 	mov	w0, #0x16                  	// #22
    db30:	910003fd 	mov	x29, sp
    db34:	54000069 	b.ls	db40 <pthread_setcancelstate+0x1c>
    db38:	a8c27bfd 	ldp	x29, x30, [sp],#32
    db3c:	d65f03c0 	ret
    db40:	d53bd045 	mrs	x5, tpidr_el0
    db44:	d11bc0a4 	sub	x4, x5, #0x6f0
    db48:	b9410880 	ldr	w0, [x4,#264]
    db4c:	540007e0 	b.eq	dc48 <pthread_setcancelstate+0x124>
    db50:	b40004a1 	cbz	x1, dbe4 <pthread_setcancelstate+0xc0>
    db54:	121f7802 	and	w2, w0, #0xfffffffe
    db58:	12000003 	and	w3, w0, #0x1
    db5c:	6b00005f 	cmp	w2, w0
    db60:	b9000023 	str	w3, [x1]
    db64:	54000260 	b.eq	dbb0 <pthread_setcancelstate+0x8c>
    db68:	b9001ba0 	str	w0, [x29,#24]
    db6c:	91042083 	add	x3, x4, #0x108
    db70:	b9401ba6 	ldr	w6, [x29,#24]
    db74:	885ffc67 	ldaxr	w7, [x3]
    db78:	6b0600ff 	cmp	w7, w6
    db7c:	54000061 	b.ne	db88 <pthread_setcancelstate+0x64>
    db80:	88087c62 	stxr	w8, w2, [x3]
    db84:	35ffff88 	cbnz	w8, db74 <pthread_setcancelstate+0x50>
    db88:	540001a1 	b.ne	dbbc <pthread_setcancelstate+0x98>
    db8c:	b9401ba6 	ldr	w6, [x29,#24]
    db90:	b90017a6 	str	w6, [x29,#20]
    db94:	b94017a6 	ldr	w6, [x29,#20]
    db98:	6b06001f 	cmp	w0, w6
    db9c:	54000ac1 	b.ne	dcf4 <pthread_setcancelstate+0x1d0>
    dba0:	12800880 	mov	w0, #0xffffffbb            	// #-69
    dba4:	0a000042 	and	w2, w2, w0
    dba8:	7100285f 	cmp	w2, #0xa
    dbac:	54000340 	b.eq	dc14 <pthread_setcancelstate+0xf0>
    dbb0:	52800000 	mov	w0, #0x0                   	// #0
    dbb4:	a8c27bfd 	ldp	x29, x30, [sp],#32
    dbb8:	d65f03c0 	ret
    dbbc:	b9001ba7 	str	w7, [x29,#24]
    dbc0:	17fffff3 	b	db8c <pthread_setcancelstate+0x68>
    dbc4:	54000040 	b.eq	dbcc <pthread_setcancelstate+0xa8>
    dbc8:	b9001ba6 	str	w6, [x29,#24]
    dbcc:	b9401ba1 	ldr	w1, [x29,#24]
    dbd0:	b90017a1 	str	w1, [x29,#20]
    dbd4:	b94017a1 	ldr	w1, [x29,#20]
    dbd8:	6b01001f 	cmp	w0, w1
    dbdc:	54fffe20 	b.eq	dba0 <pthread_setcancelstate+0x7c>
    dbe0:	2a0103e0 	mov	w0, w1
    dbe4:	121f7802 	and	w2, w0, #0xfffffffe
    dbe8:	6b02001f 	cmp	w0, w2
    dbec:	54fffe20 	b.eq	dbb0 <pthread_setcancelstate+0x8c>
    dbf0:	b9001ba0 	str	w0, [x29,#24]
    dbf4:	91042083 	add	x3, x4, #0x108
    dbf8:	b9401ba1 	ldr	w1, [x29,#24]
    dbfc:	885ffc66 	ldaxr	w6, [x3]
    dc00:	6b0100df 	cmp	w6, w1
    dc04:	54fffe01 	b.ne	dbc4 <pthread_setcancelstate+0xa0>
    dc08:	88077c62 	stxr	w7, w2, [x3]
    dc0c:	34fffdc7 	cbz	w7, dbc4 <pthread_setcancelstate+0xa0>
    dc10:	17fffffb 	b	dbfc <pthread_setcancelstate+0xd8>
    dc14:	d117a0a5 	sub	x5, x5, #0x5e8
    dc18:	910073a1 	add	x1, x29, #0x1c
    dc1c:	b94000a0 	ldr	w0, [x5]
    dc20:	b9001fa0 	str	w0, [x29,#28]
    dc24:	321c0006 	orr	w6, w0, #0x10
    dc28:	885ffc62 	ldaxr	w2, [x3]
    dc2c:	6b00005f 	cmp	w2, w0
    dc30:	54000061 	b.ne	dc3c <pthread_setcancelstate+0x118>
    dc34:	88077c66 	stxr	w7, w6, [x3]
    dc38:	35ffff87 	cbnz	w7, dc28 <pthread_setcancelstate+0x104>
    dc3c:	54000600 	b.eq	dcfc <pthread_setcancelstate+0x1d8>
    dc40:	b9000022 	str	w2, [x1]
    dc44:	17fffff6 	b	dc1c <pthread_setcancelstate+0xf8>
    dc48:	b40003e1 	cbz	x1, dcc4 <pthread_setcancelstate+0x1a0>
    dc4c:	32000002 	orr	w2, w0, #0x1
    dc50:	12000003 	and	w3, w0, #0x1
    dc54:	6b02001f 	cmp	w0, w2
    dc58:	b9000023 	str	w3, [x1]
    dc5c:	54fffaa0 	b.eq	dbb0 <pthread_setcancelstate+0x8c>
    dc60:	b9001ba0 	str	w0, [x29,#24]
    dc64:	91042083 	add	x3, x4, #0x108
    dc68:	b9401ba6 	ldr	w6, [x29,#24]
    dc6c:	885ffc67 	ldaxr	w7, [x3]
    dc70:	6b0600ff 	cmp	w7, w6
    dc74:	54000061 	b.ne	dc80 <pthread_setcancelstate+0x15c>
    dc78:	88087c62 	stxr	w8, w2, [x3]
    dc7c:	35ffff88 	cbnz	w8, dc6c <pthread_setcancelstate+0x148>
    dc80:	54000040 	b.eq	dc88 <pthread_setcancelstate+0x164>
    dc84:	b9001ba7 	str	w7, [x29,#24]
    dc88:	b9401ba6 	ldr	w6, [x29,#24]
    dc8c:	b90017a6 	str	w6, [x29,#20]
    dc90:	b94017a6 	ldr	w6, [x29,#20]
    dc94:	6b06001f 	cmp	w0, w6
    dc98:	54fff840 	b.eq	dba0 <pthread_setcancelstate+0x7c>
    dc9c:	2a0603e0 	mov	w0, w6
    dca0:	17ffffeb 	b	dc4c <pthread_setcancelstate+0x128>
    dca4:	54000040 	b.eq	dcac <pthread_setcancelstate+0x188>
    dca8:	b9001ba6 	str	w6, [x29,#24]
    dcac:	b9401ba1 	ldr	w1, [x29,#24]
    dcb0:	b90017a1 	str	w1, [x29,#20]
    dcb4:	b94017a1 	ldr	w1, [x29,#20]
    dcb8:	6b01001f 	cmp	w0, w1
    dcbc:	54fff720 	b.eq	dba0 <pthread_setcancelstate+0x7c>
    dcc0:	2a0103e0 	mov	w0, w1
    dcc4:	32000002 	orr	w2, w0, #0x1
    dcc8:	6b02001f 	cmp	w0, w2
    dccc:	54fff720 	b.eq	dbb0 <pthread_setcancelstate+0x8c>
    dcd0:	b9001ba0 	str	w0, [x29,#24]
    dcd4:	91042083 	add	x3, x4, #0x108
    dcd8:	b9401ba1 	ldr	w1, [x29,#24]
    dcdc:	885ffc66 	ldaxr	w6, [x3]
    dce0:	6b0100df 	cmp	w6, w1
    dce4:	54fffe01 	b.ne	dca4 <pthread_setcancelstate+0x180>
    dce8:	88077c62 	stxr	w7, w2, [x3]
    dcec:	34fffdc7 	cbz	w7, dca4 <pthread_setcancelstate+0x180>
    dcf0:	17fffffb 	b	dcdc <pthread_setcancelstate+0x1b8>
    dcf4:	2a0603e0 	mov	w0, w6
    dcf8:	17ffff97 	b	db54 <pthread_setcancelstate+0x30>
    dcfc:	f9408080 	ldr	x0, [x4,#256]
    dd00:	94000601 	bl	f504 <__pthread_unwind>

000000000000dd04 <pthread_setcanceltype>:
    dd04:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    dd08:	7100041f 	cmp	w0, #0x1
    dd0c:	528002c0 	mov	w0, #0x16                  	// #22
    dd10:	910003fd 	mov	x29, sp
    dd14:	54000069 	b.ls	dd20 <pthread_setcanceltype+0x1c>
    dd18:	a8c27bfd 	ldp	x29, x30, [sp],#32
    dd1c:	d65f03c0 	ret
    dd20:	d53bd045 	mrs	x5, tpidr_el0
    dd24:	d11bc0a4 	sub	x4, x5, #0x6f0
    dd28:	b9410880 	ldr	w0, [x4,#264]
    dd2c:	54000820 	b.eq	de30 <pthread_setcanceltype+0x12c>
    dd30:	b40004a1 	cbz	x1, ddc4 <pthread_setcanceltype+0xc0>
    dd34:	121e7802 	and	w2, w0, #0xfffffffd
    dd38:	d3410403 	ubfx	x3, x0, #1, #1
    dd3c:	6b00005f 	cmp	w2, w0
    dd40:	b9000023 	str	w3, [x1]
    dd44:	54000260 	b.eq	dd90 <pthread_setcanceltype+0x8c>
    dd48:	b9001ba0 	str	w0, [x29,#24]
    dd4c:	91042083 	add	x3, x4, #0x108
    dd50:	b9401ba6 	ldr	w6, [x29,#24]
    dd54:	885ffc67 	ldaxr	w7, [x3]
    dd58:	6b0600ff 	cmp	w7, w6
    dd5c:	54000061 	b.ne	dd68 <pthread_setcanceltype+0x64>
    dd60:	88087c62 	stxr	w8, w2, [x3]
    dd64:	35ffff88 	cbnz	w8, dd54 <pthread_setcanceltype+0x50>
    dd68:	540001a1 	b.ne	dd9c <pthread_setcanceltype+0x98>
    dd6c:	b9401ba6 	ldr	w6, [x29,#24]
    dd70:	b90017a6 	str	w6, [x29,#20]
    dd74:	b94017a6 	ldr	w6, [x29,#20]
    dd78:	6b06001f 	cmp	w0, w6
    dd7c:	54000b01 	b.ne	dedc <pthread_setcanceltype+0x1d8>
    dd80:	12800880 	mov	w0, #0xffffffbb            	// #-69
    dd84:	0a000042 	and	w2, w2, w0
    dd88:	7100285f 	cmp	w2, #0xa
    dd8c:	54000340 	b.eq	ddf4 <pthread_setcanceltype+0xf0>
    dd90:	52800000 	mov	w0, #0x0                   	// #0
    dd94:	a8c27bfd 	ldp	x29, x30, [sp],#32
    dd98:	d65f03c0 	ret
    dd9c:	b9001ba7 	str	w7, [x29,#24]
    dda0:	17fffff3 	b	dd6c <pthread_setcanceltype+0x68>
    dda4:	54000040 	b.eq	ddac <pthread_setcanceltype+0xa8>
    dda8:	b9001ba6 	str	w6, [x29,#24]
    ddac:	b9401ba1 	ldr	w1, [x29,#24]
    ddb0:	b90017a1 	str	w1, [x29,#20]
    ddb4:	b94017a1 	ldr	w1, [x29,#20]
    ddb8:	6b01001f 	cmp	w0, w1
    ddbc:	54fffe20 	b.eq	dd80 <pthread_setcanceltype+0x7c>
    ddc0:	2a0103e0 	mov	w0, w1
    ddc4:	121e7802 	and	w2, w0, #0xfffffffd
    ddc8:	6b02001f 	cmp	w0, w2
    ddcc:	54fffe20 	b.eq	dd90 <pthread_setcanceltype+0x8c>
    ddd0:	b9001ba0 	str	w0, [x29,#24]
    ddd4:	91042083 	add	x3, x4, #0x108
    ddd8:	b9401ba1 	ldr	w1, [x29,#24]
    dddc:	885ffc66 	ldaxr	w6, [x3]
    dde0:	6b0100df 	cmp	w6, w1
    dde4:	54fffe01 	b.ne	dda4 <pthread_setcanceltype+0xa0>
    dde8:	88077c62 	stxr	w7, w2, [x3]
    ddec:	34fffdc7 	cbz	w7, dda4 <pthread_setcanceltype+0xa0>
    ddf0:	17fffffb 	b	dddc <pthread_setcanceltype+0xd8>
    ddf4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    ddf8:	d117a0a5 	sub	x5, x5, #0x5e8
    ddfc:	f9021480 	str	x0, [x4,#1064]
    de00:	910073a1 	add	x1, x29, #0x1c
    de04:	b94000a0 	ldr	w0, [x5]
    de08:	b9001fa0 	str	w0, [x29,#28]
    de0c:	321c0006 	orr	w6, w0, #0x10
    de10:	885ffc62 	ldaxr	w2, [x3]
    de14:	6b00005f 	cmp	w2, w0
    de18:	54000061 	b.ne	de24 <pthread_setcanceltype+0x120>
    de1c:	88077c66 	stxr	w7, w6, [x3]
    de20:	35ffff87 	cbnz	w7, de10 <pthread_setcanceltype+0x10c>
    de24:	54000600 	b.eq	dee4 <pthread_setcanceltype+0x1e0>
    de28:	b9000022 	str	w2, [x1]
    de2c:	17fffff6 	b	de04 <pthread_setcanceltype+0x100>
    de30:	b40003e1 	cbz	x1, deac <pthread_setcanceltype+0x1a8>
    de34:	321f0002 	orr	w2, w0, #0x2
    de38:	d3410403 	ubfx	x3, x0, #1, #1
    de3c:	6b02001f 	cmp	w0, w2
    de40:	b9000023 	str	w3, [x1]
    de44:	54fffa60 	b.eq	dd90 <pthread_setcanceltype+0x8c>
    de48:	b9001ba0 	str	w0, [x29,#24]
    de4c:	91042083 	add	x3, x4, #0x108
    de50:	b9401ba6 	ldr	w6, [x29,#24]
    de54:	885ffc67 	ldaxr	w7, [x3]
    de58:	6b0600ff 	cmp	w7, w6
    de5c:	54000061 	b.ne	de68 <pthread_setcanceltype+0x164>
    de60:	88087c62 	stxr	w8, w2, [x3]
    de64:	35ffff88 	cbnz	w8, de54 <pthread_setcanceltype+0x150>
    de68:	54000040 	b.eq	de70 <pthread_setcanceltype+0x16c>
    de6c:	b9001ba7 	str	w7, [x29,#24]
    de70:	b9401ba6 	ldr	w6, [x29,#24]
    de74:	b90017a6 	str	w6, [x29,#20]
    de78:	b94017a6 	ldr	w6, [x29,#20]
    de7c:	6b06001f 	cmp	w0, w6
    de80:	54fff800 	b.eq	dd80 <pthread_setcanceltype+0x7c>
    de84:	2a0603e0 	mov	w0, w6
    de88:	17ffffeb 	b	de34 <pthread_setcanceltype+0x130>
    de8c:	54000040 	b.eq	de94 <pthread_setcanceltype+0x190>
    de90:	b9001ba6 	str	w6, [x29,#24]
    de94:	b9401ba1 	ldr	w1, [x29,#24]
    de98:	b90017a1 	str	w1, [x29,#20]
    de9c:	b94017a1 	ldr	w1, [x29,#20]
    dea0:	6b01001f 	cmp	w0, w1
    dea4:	54fff6e0 	b.eq	dd80 <pthread_setcanceltype+0x7c>
    dea8:	2a0103e0 	mov	w0, w1
    deac:	321f0002 	orr	w2, w0, #0x2
    deb0:	6b02001f 	cmp	w0, w2
    deb4:	54fff6e0 	b.eq	dd90 <pthread_setcanceltype+0x8c>
    deb8:	b9001ba0 	str	w0, [x29,#24]
    debc:	91042083 	add	x3, x4, #0x108
    dec0:	b9401ba1 	ldr	w1, [x29,#24]
    dec4:	885ffc66 	ldaxr	w6, [x3]
    dec8:	6b0100df 	cmp	w6, w1
    decc:	54fffe01 	b.ne	de8c <pthread_setcanceltype+0x188>
    ded0:	88077c62 	stxr	w7, w2, [x3]
    ded4:	34fffdc7 	cbz	w7, de8c <pthread_setcanceltype+0x188>
    ded8:	17fffffb 	b	dec4 <pthread_setcanceltype+0x1c0>
    dedc:	2a0603e0 	mov	w0, w6
    dee0:	17ffff95 	b	dd34 <pthread_setcanceltype+0x30>
    dee4:	f9408080 	ldr	x0, [x4,#256]
    dee8:	94000587 	bl	f504 <__pthread_unwind>

000000000000deec <clear_once_control>:
    deec:	b900001f 	str	wzr, [x0]
    def0:	d2801021 	mov	x1, #0x81                  	// #129
    def4:	b2407be2 	mov	x2, #0x7fffffff            	// #2147483647
    def8:	d2800003 	mov	x3, #0x0                   	// #0
    defc:	d2800c48 	mov	x8, #0x62                  	// #98
    df00:	d4000001 	svc	#0x0
    df04:	d65f03c0 	ret

000000000000df08 <__pthread_once>:
    df08:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    df0c:	aa0003e4 	mov	x4, x0
    df10:	aa0103e5 	mov	x5, x1
    df14:	910003fd 	mov	x29, sp
    df18:	a90153f3 	stp	x19, x20, [sp,#16]
    df1c:	f90013f5 	str	x21, [sp,#32]
    df20:	f0000126 	adrp	x6, 34000 <__GI___pthread_keys+0x3d78>
    df24:	b9400083 	ldr	w3, [x4]
    df28:	d5033bbf 	dmb	ish
    df2c:	360800c3 	tbz	w3, #1, df44 <__pthread_once+0x3c>
    df30:	52800000 	mov	w0, #0x0                   	// #0
    df34:	f94013f5 	ldr	x21, [sp,#32]
    df38:	a94153f3 	ldp	x19, x20, [sp,#16]
    df3c:	a8c57bfd 	ldp	x29, x30, [sp],#80
    df40:	d65f03c0 	ret
    df44:	910143b5 	add	x21, x29, #0x50
    df48:	f941c4c2 	ldr	x2, [x6,#904]
    df4c:	2a0303e0 	mov	w0, w3
    df50:	32000042 	orr	w2, w2, #0x1
    df54:	b81e0ea3 	str	w3, [x21,#-32]!
    df58:	885ffc81 	ldaxr	w1, [x4]
    df5c:	6b00003f 	cmp	w1, w0
    df60:	54000061 	b.ne	df6c <__pthread_once+0x64>
    df64:	88077c82 	stxr	w7, w2, [x4]
    df68:	35ffff87 	cbnz	w7, df58 <__pthread_once+0x50>
    df6c:	54000040 	b.eq	df74 <__pthread_once+0x6c>
    df70:	b90033a1 	str	w1, [x29,#48]
    df74:	b94033a0 	ldr	w0, [x29,#48]
    df78:	6b00007f 	cmp	w3, w0
    df7c:	54000401 	b.ne	dffc <__pthread_once+0xf4>
    df80:	36000063 	tbz	w3, #0, df8c <__pthread_once+0x84>
    df84:	6b03005f 	cmp	w2, w3
    df88:	540002c0 	b.eq	dfe0 <__pthread_once+0xd8>
    df8c:	90000001 	adrp	x1, d000 <pthread_cond_broadcast@@GLIBC_2.17+0x7c>
    df90:	aa0403e2 	mov	x2, x4
    df94:	aa0503f4 	mov	x20, x5
    df98:	aa1503e0 	mov	x0, x21
    df9c:	913bb021 	add	x1, x1, #0xeec
    dfa0:	aa0403f3 	mov	x19, x4
    dfa4:	94000497 	bl	f200 <_pthread_cleanup_push>
    dfa8:	d63f0280 	blr	x20
    dfac:	aa1503e0 	mov	x0, x21
    dfb0:	52800001 	mov	w1, #0x0                   	// #0
    dfb4:	9400049b 	bl	f220 <_pthread_cleanup_pop>
    dfb8:	d5033bbf 	dmb	ish
    dfbc:	52800041 	mov	w1, #0x2                   	// #2
    dfc0:	aa1303e0 	mov	x0, x19
    dfc4:	b9000261 	str	w1, [x19]
    dfc8:	b2407be2 	mov	x2, #0x7fffffff            	// #2147483647
    dfcc:	d2801021 	mov	x1, #0x81                  	// #129
    dfd0:	d2800003 	mov	x3, #0x0                   	// #0
    dfd4:	d2800c48 	mov	x8, #0x62                  	// #98
    dfd8:	d4000001 	svc	#0x0
    dfdc:	17ffffd5 	b	df30 <__pthread_once+0x28>
    dfe0:	aa0403e0 	mov	x0, x4
    dfe4:	d2801001 	mov	x1, #0x80                  	// #128
    dfe8:	93407c42 	sxtw	x2, w2
    dfec:	d2800003 	mov	x3, #0x0                   	// #0
    dff0:	d2800c48 	mov	x8, #0x62                  	// #98
    dff4:	d4000001 	svc	#0x0
    dff8:	17ffffcb 	b	df24 <__pthread_once+0x1c>
    dffc:	2a0003e3 	mov	w3, w0
    e000:	17ffffcb 	b	df2c <__pthread_once+0x24>

000000000000e004 <pthread_getcpuclockid>:
    e004:	b940d000 	ldr	w0, [x0,#208]
    e008:	6b1f001f 	cmp	w0, wzr
    e00c:	540000ed 	b.le	e028 <pthread_getcpuclockid+0x24>
    e010:	2a2003e2 	mvn	w2, w0
    e014:	52800000 	mov	w0, #0x0                   	// #0
    e018:	531d7042 	lsl	w2, w2, #3
    e01c:	321f0442 	orr	w2, w2, #0x6
    e020:	b9000022 	str	w2, [x1]
    e024:	d65f03c0 	ret
    e028:	52800060 	mov	w0, #0x3                   	// #3
    e02c:	d65f03c0 	ret

000000000000e030 <sem_init@@GLIBC_2.17>:
    e030:	37f80142 	tbnz	w2, #31, e058 <sem_init@@GLIBC_2.17+0x28>
    e034:	6b1f003f 	cmp	w1, wzr
    e038:	52801001 	mov	w1, #0x80                  	// #128
    e03c:	1a8113e1 	csel	w1, wzr, w1, ne
    e040:	b9000401 	str	w1, [x0,#4]
    e044:	52800001 	mov	w1, #0x0                   	// #0
    e048:	b9000002 	str	w2, [x0]
    e04c:	f900041f 	str	xzr, [x0,#8]
    e050:	2a0103e0 	mov	w0, w1
    e054:	d65f03c0 	ret
    e058:	d53bd040 	mrs	x0, tpidr_el0
    e05c:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    e060:	f947c442 	ldr	x2, [x2,#3976]
    e064:	12800001 	mov	w1, #0xffffffff            	// #-1
    e068:	528002c3 	mov	w3, #0x16                  	// #22
    e06c:	b8226803 	str	w3, [x0,x2]
    e070:	2a0103e0 	mov	w0, w1
    e074:	d65f03c0 	ret

000000000000e078 <sem_destroy@@GLIBC_2.17>:
    e078:	52800000 	mov	w0, #0x0                   	// #0
    e07c:	d65f03c0 	ret

000000000000e080 <__where_is_shmfs>:
    e080:	d10a83ff 	sub	sp, sp, #0x2a0
    e084:	90000020 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
    e088:	91348000 	add	x0, x0, #0xd20
    e08c:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
    e090:	910003fd 	mov	x29, sp
    e094:	a9025bf5 	stp	x21, x22, [sp,#32]
    e098:	9101e3b5 	add	x21, x29, #0x78
    e09c:	f90023f9 	str	x25, [sp,#64]
    e0a0:	aa1503e1 	mov	x1, x21
    e0a4:	a90153f3 	stp	x19, x20, [sp,#16]
    e0a8:	a90363f7 	stp	x23, x24, [sp,#48]
    e0ac:	97ffdc29 	bl	5150 <__statfs@plt>
    e0b0:	35000140 	cbnz	w0, e0d8 <__where_is_shmfs+0x58>
    e0b4:	f94002a1 	ldr	x1, [x21]
    e0b8:	d28b1ec0 	mov	x0, #0x58f6                	// #22774
    e0bc:	f2b0b080 	movk	x0, #0x8584, lsl #16
    e0c0:	eb00003f 	cmp	x1, x0
    e0c4:	54000b80 	b.eq	e234 <__where_is_shmfs+0x1b4>
    e0c8:	d2833280 	mov	x0, #0x1994                	// #6548
    e0cc:	f2a02040 	movk	x0, #0x102, lsl #16
    e0d0:	eb00003f 	cmp	x1, x0
    e0d4:	54000b00 	b.eq	e234 <__where_is_shmfs+0x1b4>
    e0d8:	90000033 	adrp	x19, 12000 <__pthread_current_priority+0xa8>
    e0dc:	90000020 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
    e0e0:	91336273 	add	x19, x19, #0xcd8
    e0e4:	91332000 	add	x0, x0, #0xcc8
    e0e8:	aa1303e1 	mov	x1, x19
    e0ec:	97ffdc55 	bl	5240 <__setmntent@plt>
    e0f0:	aa0003f6 	mov	x22, x0
    e0f4:	b4000c00 	cbz	x0, e274 <__where_is_shmfs+0x1f4>
    e0f8:	90000037 	adrp	x23, 12000 <__pthread_current_priority+0xa8>
    e0fc:	910143b9 	add	x25, x29, #0x50
    e100:	9103c3b8 	add	x24, x29, #0xf0
    e104:	9133c2f7 	add	x23, x23, #0xcf0
    e108:	aa1903e1 	mov	x1, x25
    e10c:	aa1803e2 	mov	x2, x24
    e110:	52804003 	mov	w3, #0x200                 	// #512
    e114:	aa1603e0 	mov	x0, x22
    e118:	97ffdc4e 	bl	5250 <__getmntent_r@plt>
    e11c:	aa0003f3 	mov	x19, x0
    e120:	aa1703e1 	mov	x1, x23
    e124:	b4000760 	cbz	x0, e210 <__where_is_shmfs+0x190>
    e128:	f9400a74 	ldr	x20, [x19,#16]
    e12c:	aa1403e0 	mov	x0, x20
    e130:	97ffdc24 	bl	51c0 <strcmp@plt>
    e134:	34000180 	cbz	w0, e164 <__where_is_shmfs+0xe4>
    e138:	39400283 	ldrb	w3, [x20]
    e13c:	7101cc7f 	cmp	w3, #0x73
    e140:	54fffe41 	b.ne	e108 <__where_is_shmfs+0x88>
    e144:	39400680 	ldrb	w0, [x20,#1]
    e148:	7101a01f 	cmp	w0, #0x68
    e14c:	54fffde1 	b.ne	e108 <__where_is_shmfs+0x88>
    e150:	39400a80 	ldrb	w0, [x20,#2]
    e154:	7101b41f 	cmp	w0, #0x6d
    e158:	54fffd81 	b.ne	e108 <__where_is_shmfs+0x88>
    e15c:	39400e80 	ldrb	w0, [x20,#3]
    e160:	35fffd40 	cbnz	w0, e108 <__where_is_shmfs+0x88>
    e164:	f9400660 	ldr	x0, [x19,#8]
    e168:	aa1503e1 	mov	x1, x21
    e16c:	97ffdbf9 	bl	5150 <__statfs@plt>
    e170:	d28b1ec3 	mov	x3, #0x58f6                	// #22774
    e174:	f2b0b083 	movk	x3, #0x8584, lsl #16
    e178:	35fffc80 	cbnz	w0, e108 <__where_is_shmfs+0x88>
    e17c:	f94002a1 	ldr	x1, [x21]
    e180:	d2833280 	mov	x0, #0x1994                	// #6548
    e184:	f2a02040 	movk	x0, #0x102, lsl #16
    e188:	eb03003f 	cmp	x1, x3
    e18c:	54000060 	b.eq	e198 <__where_is_shmfs+0x118>
    e190:	eb00003f 	cmp	x1, x0
    e194:	54fffba1 	b.ne	e108 <__where_is_shmfs+0x88>
    e198:	f9400674 	ldr	x20, [x19,#8]
    e19c:	aa1403e0 	mov	x0, x20
    e1a0:	97ffdb78 	bl	4f80 <strlen@plt>
    e1a4:	aa0003f3 	mov	x19, x0
    e1a8:	b4fffb00 	cbz	x0, e108 <__where_is_shmfs+0x88>
    e1ac:	d0000135 	adrp	x21, 34000 <__GI___pthread_keys+0x3d78>
    e1b0:	91001800 	add	x0, x0, #0x6
    e1b4:	97ffdbb3 	bl	5080 <malloc@plt>
    e1b8:	aa0003f7 	mov	x23, x0
    e1bc:	f901caa0 	str	x0, [x21,#912]
    e1c0:	b4000280 	cbz	x0, e210 <__where_is_shmfs+0x190>
    e1c4:	aa1403e1 	mov	x1, x20
    e1c8:	aa1303e2 	mov	x2, x19
    e1cc:	97ffdc25 	bl	5260 <mempcpy@plt>
    e1d0:	385ff001 	ldrb	w1, [x0,#-1]
    e1d4:	7100bc3f 	cmp	w1, #0x2f
    e1d8:	540004a0 	b.eq	e26c <__where_is_shmfs+0x1ec>
    e1dc:	528005e1 	mov	w1, #0x2f                  	// #47
    e1e0:	91000403 	add	x3, x0, #0x1
    e1e4:	39000001 	strb	w1, [x0]
    e1e8:	90000022 	adrp	x2, 12000 <__pthread_current_priority+0xa8>
    e1ec:	aa0303e1 	mov	x1, x3
    e1f0:	9133e042 	add	x2, x2, #0xcf8
    e1f4:	910e42b5 	add	x21, x21, #0x390
    e1f8:	b9400040 	ldr	w0, [x2]
    e1fc:	b9000060 	str	w0, [x3]
    e200:	39401040 	ldrb	w0, [x2,#4]
    e204:	38004c20 	strb	w0, [x1,#4]!
    e208:	cb170021 	sub	x1, x1, x23
    e20c:	f90006a1 	str	x1, [x21,#8]
    e210:	aa1603e0 	mov	x0, x22
    e214:	97ffdba7 	bl	50b0 <__endmntent@plt>
    e218:	a94153f3 	ldp	x19, x20, [sp,#16]
    e21c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    e220:	a94363f7 	ldp	x23, x24, [sp,#48]
    e224:	f94023f9 	ldr	x25, [sp,#64]
    e228:	a8c57bfd 	ldp	x29, x30, [sp],#80
    e22c:	910a83ff 	add	sp, sp, #0x2a0
    e230:	d65f03c0 	ret
    e234:	d0000121 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    e238:	90000020 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
    e23c:	910e4022 	add	x2, x1, #0x390
    e240:	91344000 	add	x0, x0, #0xd10
    e244:	a94153f3 	ldp	x19, x20, [sp,#16]
    e248:	f901c820 	str	x0, [x1,#912]
    e24c:	d28001a0 	mov	x0, #0xd                   	// #13
    e250:	f9000440 	str	x0, [x2,#8]
    e254:	a9425bf5 	ldp	x21, x22, [sp,#32]
    e258:	a94363f7 	ldp	x23, x24, [sp,#48]
    e25c:	f94023f9 	ldr	x25, [sp,#64]
    e260:	a8c57bfd 	ldp	x29, x30, [sp],#80
    e264:	910a83ff 	add	sp, sp, #0x2a0
    e268:	d65f03c0 	ret
    e26c:	aa0003e3 	mov	x3, x0
    e270:	17ffffde 	b	e1e8 <__where_is_shmfs+0x168>
    e274:	90000020 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
    e278:	aa1303e1 	mov	x1, x19
    e27c:	91338000 	add	x0, x0, #0xce0
    e280:	97ffdbf0 	bl	5240 <__setmntent@plt>
    e284:	aa0003f6 	mov	x22, x0
    e288:	b5fff380 	cbnz	x0, e0f8 <__where_is_shmfs+0x78>
    e28c:	17ffffe3 	b	e218 <__where_is_shmfs+0x198>

000000000000e290 <check_add_mapping>:
    e290:	a9b17bfd 	stp	x29, x30, [sp,#-240]!
    e294:	910003fd 	mov	x29, sp
    e298:	a90363f7 	stp	x23, x24, [sp,#48]
    e29c:	9101c3b7 	add	x23, x29, #0x70
    e2a0:	2a0203f8 	mov	w24, w2
    e2a4:	a9025bf5 	stp	x21, x22, [sp,#32]
    e2a8:	a9046bf9 	stp	x25, x26, [sp,#64]
    e2ac:	a90153f3 	stp	x19, x20, [sp,#16]
    e2b0:	a90573fb 	stp	x27, x28, [sp,#80]
    e2b4:	aa0003f9 	mov	x25, x0
    e2b8:	aa0103f6 	mov	x22, x1
    e2bc:	52800000 	mov	w0, #0x0                   	// #0
    e2c0:	2a0203e1 	mov	w1, w2
    e2c4:	aa1703e2 	mov	x2, x23
    e2c8:	aa0303f5 	mov	x21, x3
    e2cc:	97ffdbe9 	bl	5270 <__fxstat64@plt>
    e2d0:	35000860 	cbnz	w0, e3dc <check_add_mapping+0x14c>
    e2d4:	d0000133 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
    e2d8:	b9006fa0 	str	w0, [x29,#108]
    e2dc:	910b3263 	add	x3, x19, #0x2cc
    e2e0:	52800020 	mov	w0, #0x1                   	// #1
    e2e4:	885ffc61 	ldaxr	w1, [x3]
    e2e8:	6b1f003f 	cmp	w1, wzr
    e2ec:	54000061 	b.ne	e2f8 <check_add_mapping+0x68>
    e2f0:	88027c60 	stxr	w2, w0, [x3]
    e2f4:	35ffff82 	cbnz	w2, e2e4 <check_add_mapping+0x54>
    e2f8:	540006a1 	b.ne	e3cc <check_add_mapping+0x13c>
    e2fc:	9100fac5 	add	x5, x22, #0x3e
    e300:	910003e0 	mov	x0, sp
    e304:	927ceca5 	and	x5, x5, #0xfffffffffffffff0
    e308:	aa1903e1 	mov	x1, x25
    e30c:	cb25601f 	sub	sp, x0, x5
    e310:	aa1603e2 	mov	x2, x22
    e314:	910083e0 	add	x0, sp, #0x20
    e318:	d000013b 	adrp	x27, 34000 <__GI___pthread_keys+0x3d78>
    e31c:	9000001a 	adrp	x26, e000 <__pthread_once+0xf8>
    e320:	910082dc 	add	x28, x22, #0x20
    e324:	97ffdb0f 	bl	4f60 <memcpy@plt>
    e328:	f9403ba3 	ldr	x3, [x29,#112]
    e32c:	910003e0 	mov	x0, sp
    e330:	f90003e3 	str	x3, [sp]
    e334:	910e8361 	add	x1, x27, #0x3a0
    e338:	f9403fa3 	ldr	x3, [x29,#120]
    e33c:	91139342 	add	x2, x26, #0x4e4
    e340:	f90007e3 	str	x3, [sp,#8]
    e344:	97ffdb3f 	bl	5040 <tfind@plt>
    e348:	b4000680 	cbz	x0, e418 <check_add_mapping+0x188>
    e34c:	f9400000 	ldr	x0, [x0]
    e350:	d10006a1 	sub	x1, x21, #0x1
    e354:	b1000c3f 	cmn	x1, #0x3
    e358:	1a9f87e4 	cset	w4, ls
    e35c:	f9400c05 	ldr	x5, [x0,#24]
    e360:	b9401002 	ldr	w2, [x0,#16]
    e364:	eb0502bf 	cmp	x21, x5
    e368:	1a9f07e1 	cset	w1, ne
    e36c:	11000442 	add	w2, w2, #0x1
    e370:	0a010084 	and	w4, w4, w1
    e374:	b9001002 	str	w2, [x0,#16]
    e378:	910b3261 	add	x1, x19, #0x2cc
    e37c:	52800002 	mov	w2, #0x0                   	// #0
    e380:	885f7c20 	ldxr	w0, [x1]
    e384:	8803fc22 	stlxr	w3, w2, [x1]
    e388:	35ffffc3 	cbnz	w3, e380 <check_add_mapping+0xf0>
    e38c:	7100041f 	cmp	w0, #0x1
    e390:	5400036c 	b.gt	e3fc <check_add_mapping+0x16c>
    e394:	340000a4 	cbz	w4, e3a8 <check_add_mapping+0x118>
    e398:	aa1503e0 	mov	x0, x21
    e39c:	d2800401 	mov	x1, #0x20                  	// #32
    e3a0:	d2801ae8 	mov	x8, #0xd7                  	// #215
    e3a4:	d4000001 	svc	#0x0
    e3a8:	910003bf 	mov	sp, x29
    e3ac:	aa0503e0 	mov	x0, x5
    e3b0:	a94153f3 	ldp	x19, x20, [sp,#16]
    e3b4:	a9425bf5 	ldp	x21, x22, [sp,#32]
    e3b8:	a94363f7 	ldp	x23, x24, [sp,#48]
    e3bc:	a9446bf9 	ldp	x25, x26, [sp,#64]
    e3c0:	a94573fb 	ldp	x27, x28, [sp,#80]
    e3c4:	a8cf7bfd 	ldp	x29, x30, [sp],#240
    e3c8:	d65f03c0 	ret
    e3cc:	aa0303e0 	mov	x0, x3
    e3d0:	b9006fa1 	str	w1, [x29,#108]
    e3d4:	940004e5 	bl	f768 <__lll_lock_wait_private>
    e3d8:	17ffffc9 	b	e2fc <check_add_mapping+0x6c>
    e3dc:	d10006a0 	sub	x0, x21, #0x1
    e3e0:	d2800005 	mov	x5, #0x0                   	// #0
    e3e4:	b1000c1f 	cmn	x0, #0x3
    e3e8:	1a9f87e4 	cset	w4, ls
    e3ec:	eb0502bf 	cmp	x21, x5
    e3f0:	1a9f07e1 	cset	w1, ne
    e3f4:	0a010084 	and	w4, w4, w1
    e3f8:	17ffffe7 	b	e394 <check_add_mapping+0x104>
    e3fc:	aa0103e0 	mov	x0, x1
    e400:	d2800022 	mov	x2, #0x1                   	// #1
    e404:	d2801021 	mov	x1, #0x81                  	// #129
    e408:	d2800003 	mov	x3, #0x0                   	// #0
    e40c:	d2800c48 	mov	x8, #0x62                  	// #98
    e410:	d4000001 	svc	#0x0
    e414:	17ffffe0 	b	e394 <check_add_mapping+0x104>
    e418:	aa1c03e0 	mov	x0, x28
    e41c:	97ffdb19 	bl	5080 <malloc@plt>
    e420:	aa0003f4 	mov	x20, x0
    e424:	b40002a0 	cbz	x0, e478 <check_add_mapping+0x1e8>
    e428:	b40004d5 	cbz	x21, e4c0 <check_add_mapping+0x230>
    e42c:	3dc002e0 	ldr	q0, [x23]
    e430:	52800023 	mov	w3, #0x1                   	// #1
    e434:	91008280 	add	x0, x20, #0x20
    e438:	aa1903e1 	mov	x1, x25
    e43c:	aa1603e2 	mov	x2, x22
    e440:	f9000e95 	str	x21, [x20,#24]
    e444:	b9001283 	str	w3, [x20,#16]
    e448:	3d800280 	str	q0, [x20]
    e44c:	97ffdac5 	bl	4f60 <memcpy@plt>
    e450:	b10006bf 	cmn	x21, #0x1
    e454:	54000220 	b.eq	e498 <check_add_mapping+0x208>
    e458:	aa1403e0 	mov	x0, x20
    e45c:	910e8361 	add	x1, x27, #0x3a0
    e460:	91139342 	add	x2, x26, #0x4e4
    e464:	97ffdb17 	bl	50c0 <tsearch@plt>
    e468:	b4000180 	cbz	x0, e498 <check_add_mapping+0x208>
    e46c:	aa1503e5 	mov	x5, x21
    e470:	52800004 	mov	w4, #0x0                   	// #0
    e474:	17ffffc1 	b	e378 <check_add_mapping+0xe8>
    e478:	d10006a0 	sub	x0, x21, #0x1
    e47c:	aa1403e5 	mov	x5, x20
    e480:	b1000c1f 	cmn	x0, #0x3
    e484:	1a9f87e4 	cset	w4, ls
    e488:	eb1f02bf 	cmp	x21, xzr
    e48c:	1a9f07e1 	cset	w1, ne
    e490:	0a010084 	and	w4, w4, w1
    e494:	17ffffb9 	b	e378 <check_add_mapping+0xe8>
    e498:	aa1403e0 	mov	x0, x20
    e49c:	97ffdb61 	bl	5220 <free@plt>
    e4a0:	d10006a0 	sub	x0, x21, #0x1
    e4a4:	d2800005 	mov	x5, #0x0                   	// #0
    e4a8:	b1000c1f 	cmn	x0, #0x3
    e4ac:	1a9f87e4 	cset	w4, ls
    e4b0:	eb0502bf 	cmp	x21, x5
    e4b4:	1a9f07e1 	cset	w1, ne
    e4b8:	0a010084 	and	w4, w4, w1
    e4bc:	17ffffaf 	b	e378 <check_add_mapping+0xe8>
    e4c0:	aa1503e0 	mov	x0, x21
    e4c4:	aa1503e5 	mov	x5, x21
    e4c8:	d2800401 	mov	x1, #0x20                  	// #32
    e4cc:	52800062 	mov	w2, #0x3                   	// #3
    e4d0:	52800023 	mov	w3, #0x1                   	// #1
    e4d4:	2a1803e4 	mov	w4, w24
    e4d8:	97ffdb3e 	bl	51d0 <mmap@plt>
    e4dc:	aa0003f5 	mov	x21, x0
    e4e0:	17ffffd3 	b	e42c <check_add_mapping+0x19c>

000000000000e4e4 <__sem_search>:
    e4e4:	f9400403 	ldr	x3, [x0,#8]
    e4e8:	f9400422 	ldr	x2, [x1,#8]
    e4ec:	eb02007f 	cmp	x3, x2
    e4f0:	54000080 	b.eq	e500 <__sem_search+0x1c>
    e4f4:	12800000 	mov	w0, #0xffffffff            	// #-1
    e4f8:	1a9f3400 	csinc	w0, w0, wzr, cc
    e4fc:	d65f03c0 	ret
    e500:	f9400003 	ldr	x3, [x0]
    e504:	f9400022 	ldr	x2, [x1]
    e508:	eb02007f 	cmp	x3, x2
    e50c:	54ffff41 	b.ne	e4f4 <__sem_search+0x10>
    e510:	91008000 	add	x0, x0, #0x20
    e514:	91008021 	add	x1, x1, #0x20
    e518:	17ffdb2a 	b	51c0 <strcmp@plt>

000000000000e51c <sem_open>:
    e51c:	a9a87bfd 	stp	x29, x30, [sp,#-384]!
    e520:	910003fd 	mov	x29, sp
    e524:	6d0627e8 	stp	d8, d9, [sp,#96]
    e528:	a90153f3 	stp	x19, x20, [sp,#16]
    e52c:	a9046bf9 	stp	x25, x26, [sp,#64]
    e530:	a9025bf5 	stp	x21, x22, [sp,#32]
    e534:	a90363f7 	stp	x23, x24, [sp,#48]
    e538:	a90573fb 	stp	x27, x28, [sp,#80]
    e53c:	fd003bea 	str	d10, [sp,#112]
    e540:	aa0003f3 	mov	x19, x0
    e544:	b90083a1 	str	w1, [x29,#128]
    e548:	d0000120 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    e54c:	f900aba2 	str	x2, [x29,#336]
    e550:	910403a2 	add	x2, x29, #0x100
    e554:	f900afa3 	str	x3, [x29,#344]
    e558:	d000013a 	adrp	x26, 34000 <__GI___pthread_keys+0x3d78>
    e55c:	f900b3a4 	str	x4, [x29,#352]
    e560:	910b3000 	add	x0, x0, #0x2cc
    e564:	f900b7a5 	str	x5, [x29,#360]
    e568:	90000001 	adrp	x1, e000 <__pthread_once+0xf8>
    e56c:	f900bba6 	str	x6, [x29,#368]
    e570:	91001000 	add	x0, x0, #0x4
    e574:	f900bfa7 	str	x7, [x29,#376]
    e578:	91020021 	add	x1, x1, #0x80
    e57c:	3d8037a0 	str	q0, [x29,#208]
    e580:	3d803ba1 	str	q1, [x29,#224]
    e584:	3d803fa2 	str	q2, [x29,#240]
    e588:	3d800043 	str	q3, [x2]
    e58c:	910443a2 	add	x2, x29, #0x110
    e590:	3d800044 	str	q4, [x2]
    e594:	910483a2 	add	x2, x29, #0x120
    e598:	3d800045 	str	q5, [x2]
    e59c:	9104c3a2 	add	x2, x29, #0x130
    e5a0:	3d800046 	str	q6, [x2]
    e5a4:	910503a2 	add	x2, x29, #0x140
    e5a8:	3d800047 	str	q7, [x2]
    e5ac:	97fffe57 	bl	df08 <__pthread_once>
    e5b0:	f941cb54 	ldr	x20, [x26,#912]
    e5b4:	b4001934 	cbz	x20, e8d8 <sem_open+0x3bc>
    e5b8:	39400262 	ldrb	w2, [x19]
    e5bc:	7100bc5f 	cmp	w2, #0x2f
    e5c0:	54000081 	b.ne	e5d0 <sem_open+0xb4>
    e5c4:	38401e62 	ldrb	w2, [x19,#1]!
    e5c8:	7100bc5f 	cmp	w2, #0x2f
    e5cc:	54ffffc0 	b.eq	e5c4 <sem_open+0xa8>
    e5d0:	34000f62 	cbz	w2, e7bc <sem_open+0x2a0>
    e5d4:	aa1303e0 	mov	x0, x19
    e5d8:	97ffda6a 	bl	4f80 <strlen@plt>
    e5dc:	91000400 	add	x0, x0, #0x1
    e5e0:	910e4342 	add	x2, x26, #0x390
    e5e4:	aa1403e1 	mov	x1, x20
    e5e8:	9e67000a 	fmov	d10, x0
    e5ec:	f9400442 	ldr	x2, [x2,#8]
    e5f0:	8b020003 	add	x3, x0, x2
    e5f4:	910003e0 	mov	x0, sp
    e5f8:	91007863 	add	x3, x3, #0x1e
    e5fc:	927cec63 	and	x3, x3, #0xfffffffffffffff0
    e600:	cb23601f 	sub	sp, x0, x3
    e604:	910003e0 	mov	x0, sp
    e608:	9e670009 	fmov	d9, x0
    e60c:	97ffdb15 	bl	5260 <mempcpy@plt>
    e610:	9e660142 	fmov	x2, d10
    e614:	aa1303e1 	mov	x1, x19
    e618:	97ffda52 	bl	4f60 <memcpy@plt>
    e61c:	b94083a0 	ldr	w0, [x29,#128]
    e620:	121a0400 	and	w0, w0, #0xc0
    e624:	7103001f 	cmp	w0, #0xc0
    e628:	54000540 	b.eq	e6d0 <sem_open+0x1b4>
    e62c:	b94083a0 	ldr	w0, [x29,#128]
    e630:	12900861 	mov	w1, #0xffff7fbc            	// #-32836
    e634:	52900042 	mov	w2, #0x8002                	// #32770
    e638:	0a010001 	and	w1, w0, w1
    e63c:	9e660120 	fmov	x0, d9
    e640:	2a020021 	orr	w1, w1, w2
    e644:	940007e7 	bl	105e0 <__open>
    e648:	3100041f 	cmn	w0, #0x1
    e64c:	2a0003f5 	mov	w21, w0
    e650:	540002e0 	b.eq	e6ac <sem_open+0x190>
    e654:	9e660141 	fmov	x1, d10
    e658:	aa1303e0 	mov	x0, x19
    e65c:	2a1503e2 	mov	w2, w21
    e660:	d2800003 	mov	x3, #0x0                   	// #0
    e664:	97ffff0b 	bl	e290 <check_add_mapping>
    e668:	aa0003f4 	mov	x20, x0
    e66c:	b100069f 	cmn	x20, #0x1
    e670:	93407ea0 	sxtw	x0, w21
    e674:	9a9f1294 	csel	x20, x20, xzr, ne
    e678:	d2800728 	mov	x8, #0x39                  	// #57
    e67c:	d4000001 	svc	#0x0
    e680:	aa1403e0 	mov	x0, x20
    e684:	910003bf 	mov	sp, x29
    e688:	6d4627e8 	ldp	d8, d9, [sp,#96]
    e68c:	fd403bea 	ldr	d10, [sp,#112]
    e690:	a94153f3 	ldp	x19, x20, [sp,#16]
    e694:	a9425bf5 	ldp	x21, x22, [sp,#32]
    e698:	a94363f7 	ldp	x23, x24, [sp,#48]
    e69c:	a9446bf9 	ldp	x25, x26, [sp,#64]
    e6a0:	a94573fb 	ldp	x27, x28, [sp,#80]
    e6a4:	a8d87bfd 	ldp	x29, x30, [sp],#384
    e6a8:	d65f03c0 	ret
    e6ac:	f94043a1 	ldr	x1, [x29,#128]
    e6b0:	d2800000 	mov	x0, #0x0                   	// #0
    e6b4:	3637fe81 	tbz	w1, #6, e684 <sem_open+0x168>
    e6b8:	d53bd041 	mrs	x1, tpidr_el0
    e6bc:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    e6c0:	f947c442 	ldr	x2, [x2,#3976]
    e6c4:	b8626821 	ldr	w1, [x1,x2]
    e6c8:	7100083f 	cmp	w1, #0x2
    e6cc:	54fffdc1 	b.ne	e684 <sem_open+0x168>
    e6d0:	910603a0 	add	x0, x29, #0x180
    e6d4:	f9004ba0 	str	x0, [x29,#144]
    e6d8:	f9004fa0 	str	x0, [x29,#152]
    e6dc:	910543a0 	add	x0, x29, #0x150
    e6e0:	b9415ba2 	ldr	w2, [x29,#344]
    e6e4:	f90053a0 	str	x0, [x29,#160]
    e6e8:	12800fe0 	mov	w0, #0xffffff80            	// #-128
    e6ec:	b900afa0 	str	w0, [x29,#172]
    e6f0:	128004e0 	mov	w0, #0xffffffd8            	// #-40
    e6f4:	b900aba0 	str	w0, [x29,#168]
    e6f8:	b94153bc 	ldr	w28, [x29,#336]
    e6fc:	37f80e02 	tbnz	w2, #31, e8bc <sem_open+0x3a0>
    e700:	910e4340 	add	x0, x26, #0x390
    e704:	b900b3a2 	str	w2, [x29,#176]
    e708:	910003e3 	mov	x3, sp
    e70c:	f941cb41 	ldr	x1, [x26,#912]
    e710:	b900b7bf 	str	wzr, [x29,#180]
    e714:	52800655 	mov	w21, #0x32                  	// #50
    e718:	f9400402 	ldr	x2, [x0,#8]
    e71c:	f9005fbf 	str	xzr, [x29,#184]
    e720:	91009440 	add	x0, x2, #0x25
    e724:	b0000119 	adrp	x25, 2f000 <__FRAME_END__+0x18e30>
    e728:	f947c739 	ldr	x25, [x25,#3976]
    e72c:	927cec00 	and	x0, x0, #0xfffffffffffffff0
    e730:	cb20607f 	sub	sp, x3, x0
    e734:	910003e0 	mov	x0, sp
    e738:	910003f7 	mov	x23, sp
    e73c:	a90c7fbf 	stp	xzr, xzr, [x29,#192]
    e740:	97ffdac8 	bl	5260 <mempcpy@plt>
    e744:	aa0003f4 	mov	x20, x0
    e748:	90000020 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
    e74c:	d53bd041 	mrs	x1, tpidr_el0
    e750:	91340000 	add	x0, x0, #0xd00
    e754:	8b190039 	add	x25, x1, x25
    e758:	b9400001 	ldr	w1, [x0]
    e75c:	7940081b 	ldrh	w27, [x0,#4]
    e760:	39401818 	ldrb	w24, [x0,#6]
    e764:	1e270028 	fmov	s8, w1
    e768:	1400000b 	b	e794 <sem_open+0x278>
    e76c:	aa1703e0 	mov	x0, x23
    e770:	9400079c 	bl	105e0 <__open>
    e774:	3100041f 	cmn	w0, #0x1
    e778:	2a0003f6 	mov	w22, w0
    e77c:	540002e1 	b.ne	e7d8 <sem_open+0x2bc>
    e780:	b9400323 	ldr	w3, [x25]
    e784:	7100447f 	cmp	w3, #0x11
    e788:	54000161 	b.ne	e7b4 <sem_open+0x298>
    e78c:	710006b5 	subs	w21, w21, #0x1
    e790:	54000860 	b.eq	e89c <sem_open+0x380>
    e794:	bd000288 	str	s8, [x20]
    e798:	aa1703e0 	mov	x0, x23
    e79c:	79000a9b 	strh	w27, [x20,#4]
    e7a0:	39001a98 	strb	w24, [x20,#6]
    e7a4:	97ffdad3 	bl	52f0 <__mktemp@plt>
    e7a8:	52801841 	mov	w1, #0xc2                  	// #194
    e7ac:	2a1c03e2 	mov	w2, w28
    e7b0:	b5fffde0 	cbnz	x0, e76c <sem_open+0x250>
    e7b4:	d2800000 	mov	x0, #0x0                   	// #0
    e7b8:	17ffffb3 	b	e684 <sem_open+0x168>
    e7bc:	d53bd041 	mrs	x1, tpidr_el0
    e7c0:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    e7c4:	f947c442 	ldr	x2, [x2,#3976]
    e7c8:	528002c3 	mov	w3, #0x16                  	// #22
    e7cc:	d2800000 	mov	x0, #0x0                   	// #0
    e7d0:	b8226823 	str	w3, [x1,x2]
    e7d4:	17ffffac 	b	e684 <sem_open+0x168>
    e7d8:	b0000114 	adrp	x20, 2f000 <__FRAME_END__+0x18e30>
    e7dc:	f947c694 	ldr	x20, [x20,#3976]
    e7e0:	2a0003f5 	mov	w21, w0
    e7e4:	d53bd040 	mrs	x0, tpidr_el0
    e7e8:	9102c3bc 	add	x28, x29, #0xb0
    e7ec:	8b140014 	add	x20, x0, x20
    e7f0:	14000004 	b	e800 <sem_open+0x2e4>
    e7f4:	b9400280 	ldr	w0, [x20]
    e7f8:	7100101f 	cmp	w0, #0x4
    e7fc:	54000581 	b.ne	e8ac <sem_open+0x390>
    e800:	2a1603e0 	mov	w0, w22
    e804:	aa1c03e1 	mov	x1, x28
    e808:	d2800402 	mov	x2, #0x20                  	// #32
    e80c:	94000505 	bl	fc20 <__write>
    e810:	b100041f 	cmn	x0, #0x1
    e814:	aa0003f9 	mov	x25, x0
    e818:	54fffee0 	b.eq	e7f4 <sem_open+0x2d8>
    e81c:	f100801f 	cmp	x0, #0x20
    e820:	54000461 	b.ne	e8ac <sem_open+0x390>
    e824:	d2800000 	mov	x0, #0x0                   	// #0
    e828:	aa1903e1 	mov	x1, x25
    e82c:	52800062 	mov	w2, #0x3                   	// #3
    e830:	52800023 	mov	w3, #0x1                   	// #1
    e834:	2a1603e4 	mov	w4, w22
    e838:	aa0003e5 	mov	x5, x0
    e83c:	97ffda65 	bl	51d0 <mmap@plt>
    e840:	b100041f 	cmn	x0, #0x1
    e844:	aa0003f4 	mov	x20, x0
    e848:	54000340 	b.eq	e8b0 <sem_open+0x394>
    e84c:	9e660121 	fmov	x1, d9
    e850:	aa1703e0 	mov	x0, x23
    e854:	97ffdab3 	bl	5320 <link@plt>
    e858:	340004e0 	cbz	w0, e8f4 <sem_open+0x3d8>
    e85c:	aa1403e0 	mov	x0, x20
    e860:	aa1903e1 	mov	x1, x25
    e864:	97ffda93 	bl	52b0 <munmap@plt>
    e868:	f94043a0 	ldr	x0, [x29,#128]
    e86c:	37380200 	tbnz	w0, #7, e8ac <sem_open+0x390>
    e870:	d53bd040 	mrs	x0, tpidr_el0
    e874:	b0000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    e878:	f947c421 	ldr	x1, [x1,#3976]
    e87c:	b8616800 	ldr	w0, [x0,x1]
    e880:	7100441f 	cmp	w0, #0x11
    e884:	54000141 	b.ne	e8ac <sem_open+0x390>
    e888:	aa1703e0 	mov	x0, x23
    e88c:	97ffdad1 	bl	53d0 <unlink@plt>
    e890:	2a1603e0 	mov	w0, w22
    e894:	9400051b 	bl	fd00 <__close>
    e898:	17ffff65 	b	e62c <sem_open+0x110>
    e89c:	52800160 	mov	w0, #0xb                   	// #11
    e8a0:	b9000320 	str	w0, [x25]
    e8a4:	d2800000 	mov	x0, #0x0                   	// #0
    e8a8:	17ffff77 	b	e684 <sem_open+0x168>
    e8ac:	d2800014 	mov	x20, #0x0                   	// #0
    e8b0:	aa1703e0 	mov	x0, x23
    e8b4:	97ffdac7 	bl	53d0 <unlink@plt>
    e8b8:	17ffff6d 	b	e66c <sem_open+0x150>
    e8bc:	d53bd040 	mrs	x0, tpidr_el0
    e8c0:	b0000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    e8c4:	f947c421 	ldr	x1, [x1,#3976]
    e8c8:	528002c2 	mov	w2, #0x16                  	// #22
    e8cc:	b8216802 	str	w2, [x0,x1]
    e8d0:	d2800000 	mov	x0, #0x0                   	// #0
    e8d4:	17ffff6c 	b	e684 <sem_open+0x168>
    e8d8:	d53bd041 	mrs	x1, tpidr_el0
    e8dc:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    e8e0:	f947c442 	ldr	x2, [x2,#3976]
    e8e4:	528004c3 	mov	w3, #0x26                  	// #38
    e8e8:	aa1403e0 	mov	x0, x20
    e8ec:	b8226823 	str	w3, [x1,x2]
    e8f0:	17ffff65 	b	e684 <sem_open+0x168>
    e8f4:	9e660141 	fmov	x1, d10
    e8f8:	aa1403e3 	mov	x3, x20
    e8fc:	aa1303e0 	mov	x0, x19
    e900:	2a1603e2 	mov	w2, w22
    e904:	97fffe63 	bl	e290 <check_add_mapping>
    e908:	aa0003f4 	mov	x20, x0
    e90c:	17ffffe9 	b	e8b0 <sem_open+0x394>

000000000000e910 <walker>:
    e910:	d0000121 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
    e914:	f9400000 	ldr	x0, [x0]
    e918:	910b6023 	add	x3, x1, #0x2d8
    e91c:	f9400c02 	ldr	x2, [x0,#24]
    e920:	f9416c21 	ldr	x1, [x1,#728]
    e924:	eb01005f 	cmp	x2, x1
    e928:	54000040 	b.eq	e930 <walker+0x20>
    e92c:	d65f03c0 	ret
    e930:	f9000460 	str	x0, [x3,#8]
    e934:	d65f03c0 	ret

000000000000e938 <sem_close>:
    e938:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    e93c:	910003fd 	mov	x29, sp
    e940:	a90153f3 	stp	x19, x20, [sp,#16]
    e944:	d0000133 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
    e948:	a9025bf5 	stp	x21, x22, [sp,#32]
    e94c:	b9003fbf 	str	wzr, [x29,#60]
    e950:	aa0003f5 	mov	x21, x0
    e954:	910b3261 	add	x1, x19, #0x2cc
    e958:	52800020 	mov	w0, #0x1                   	// #1
    e95c:	885ffc22 	ldaxr	w2, [x1]
    e960:	6b1f005f 	cmp	w2, wzr
    e964:	54000061 	b.ne	e970 <sem_close+0x38>
    e968:	88037c20 	stxr	w3, w0, [x1]
    e96c:	35ffff83 	cbnz	w3, e95c <sem_close+0x24>
    e970:	540003c1 	b.ne	e9e8 <sem_close+0xb0>
    e974:	d0000123 	adrp	x3, 34000 <__GI___pthread_keys+0x3d78>
    e978:	d0000122 	adrp	x2, 34000 <__GI___pthread_keys+0x3d78>
    e97c:	910b6054 	add	x20, x2, #0x2d8
    e980:	90000001 	adrp	x1, e000 <__pthread_once+0xf8>
    e984:	91244021 	add	x1, x1, #0x910
    e988:	910e8076 	add	x22, x3, #0x3a0
    e98c:	f941d060 	ldr	x0, [x3,#928]
    e990:	f900069f 	str	xzr, [x20,#8]
    e994:	f9016c55 	str	x21, [x2,#728]
    e998:	97ffd98e 	bl	4fd0 <twalk@plt>
    e99c:	f9400680 	ldr	x0, [x20,#8]
    e9a0:	b40005a0 	cbz	x0, ea54 <sem_close+0x11c>
    e9a4:	b9401002 	ldr	w2, [x0,#16]
    e9a8:	52800015 	mov	w21, #0x0                   	// #0
    e9ac:	51000442 	sub	w2, w2, #0x1
    e9b0:	b9001002 	str	w2, [x0,#16]
    e9b4:	34000222 	cbz	w2, e9f8 <sem_close+0xc0>
    e9b8:	910b3261 	add	x1, x19, #0x2cc
    e9bc:	52800002 	mov	w2, #0x0                   	// #0
    e9c0:	885f7c20 	ldxr	w0, [x1]
    e9c4:	8803fc22 	stlxr	w3, w2, [x1]
    e9c8:	35ffffc3 	cbnz	w3, e9c0 <sem_close+0x88>
    e9cc:	7100041f 	cmp	w0, #0x1
    e9d0:	540002cc 	b.gt	ea28 <sem_close+0xf0>
    e9d4:	2a1503e0 	mov	w0, w21
    e9d8:	a94153f3 	ldp	x19, x20, [sp,#16]
    e9dc:	a9425bf5 	ldp	x21, x22, [sp,#32]
    e9e0:	a8c47bfd 	ldp	x29, x30, [sp],#64
    e9e4:	d65f03c0 	ret
    e9e8:	aa0103e0 	mov	x0, x1
    e9ec:	b9003fa2 	str	w2, [x29,#60]
    e9f0:	9400035e 	bl	f768 <__lll_lock_wait_private>
    e9f4:	17ffffe0 	b	e974 <sem_close+0x3c>
    e9f8:	90000002 	adrp	x2, e000 <__pthread_once+0xf8>
    e9fc:	aa1603e1 	mov	x1, x22
    ea00:	91139042 	add	x2, x2, #0x4e4
    ea04:	97ffd9d7 	bl	5160 <tdelete@plt>
    ea08:	f9400680 	ldr	x0, [x20,#8]
    ea0c:	d2800401 	mov	x1, #0x20                  	// #32
    ea10:	f9400c00 	ldr	x0, [x0,#24]
    ea14:	97ffda27 	bl	52b0 <munmap@plt>
    ea18:	2a0003f5 	mov	w21, w0
    ea1c:	f9400680 	ldr	x0, [x20,#8]
    ea20:	97ffda00 	bl	5220 <free@plt>
    ea24:	17ffffe5 	b	e9b8 <sem_close+0x80>
    ea28:	aa0103e0 	mov	x0, x1
    ea2c:	d2800022 	mov	x2, #0x1                   	// #1
    ea30:	d2801021 	mov	x1, #0x81                  	// #129
    ea34:	d2800003 	mov	x3, #0x0                   	// #0
    ea38:	d2800c48 	mov	x8, #0x62                  	// #98
    ea3c:	d4000001 	svc	#0x0
    ea40:	2a1503e0 	mov	w0, w21
    ea44:	a94153f3 	ldp	x19, x20, [sp,#16]
    ea48:	a9425bf5 	ldp	x21, x22, [sp,#32]
    ea4c:	a8c47bfd 	ldp	x29, x30, [sp],#64
    ea50:	d65f03c0 	ret
    ea54:	d53bd040 	mrs	x0, tpidr_el0
    ea58:	b0000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    ea5c:	f947c421 	ldr	x1, [x1,#3976]
    ea60:	528002c2 	mov	w2, #0x16                  	// #22
    ea64:	12800015 	mov	w21, #0xffffffff            	// #-1
    ea68:	b8216802 	str	w2, [x0,x1]
    ea6c:	17ffffd3 	b	e9b8 <sem_close+0x80>

000000000000ea70 <sem_unlink>:
    ea70:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    ea74:	90000001 	adrp	x1, e000 <__pthread_once+0xf8>
    ea78:	910003fd 	mov	x29, sp
    ea7c:	a90153f3 	stp	x19, x20, [sp,#16]
    ea80:	aa0003f3 	mov	x19, x0
    ea84:	d0000120 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
    ea88:	91020021 	add	x1, x1, #0x80
    ea8c:	f90013f5 	str	x21, [sp,#32]
    ea90:	d0000134 	adrp	x20, 34000 <__GI___pthread_keys+0x3d78>
    ea94:	910b4000 	add	x0, x0, #0x2d0
    ea98:	97fffd1c 	bl	df08 <__pthread_once>
    ea9c:	f941ca81 	ldr	x1, [x20,#912]
    eaa0:	b4000621 	cbz	x1, eb64 <sem_unlink+0xf4>
    eaa4:	39400260 	ldrb	w0, [x19]
    eaa8:	7100bc1f 	cmp	w0, #0x2f
    eaac:	54000081 	b.ne	eabc <sem_unlink+0x4c>
    eab0:	38401e60 	ldrb	w0, [x19,#1]!
    eab4:	7100bc1f 	cmp	w0, #0x2f
    eab8:	54ffffc0 	b.eq	eab0 <sem_unlink+0x40>
    eabc:	34000460 	cbz	w0, eb48 <sem_unlink+0xd8>
    eac0:	aa1303e0 	mov	x0, x19
    eac4:	910e4294 	add	x20, x20, #0x390
    eac8:	f9001fa1 	str	x1, [x29,#56]
    eacc:	97ffd92d 	bl	4f80 <strlen@plt>
    ead0:	aa0003f5 	mov	x21, x0
    ead4:	f9400682 	ldr	x2, [x20,#8]
    ead8:	f9401fa1 	ldr	x1, [x29,#56]
    eadc:	8b000043 	add	x3, x2, x0
    eae0:	910003e0 	mov	x0, sp
    eae4:	91007c63 	add	x3, x3, #0x1f
    eae8:	927cec63 	and	x3, x3, #0xfffffffffffffff0
    eaec:	cb23601f 	sub	sp, x0, x3
    eaf0:	910003e0 	mov	x0, sp
    eaf4:	97ffd9db 	bl	5260 <mempcpy@plt>
    eaf8:	aa1303e1 	mov	x1, x19
    eafc:	910006a2 	add	x2, x21, #0x1
    eb00:	97ffd918 	bl	4f60 <memcpy@plt>
    eb04:	910003e0 	mov	x0, sp
    eb08:	97ffda32 	bl	53d0 <unlink@plt>
    eb0c:	37f800c0 	tbnz	w0, #31, eb24 <sem_unlink+0xb4>
    eb10:	910003bf 	mov	sp, x29
    eb14:	a94153f3 	ldp	x19, x20, [sp,#16]
    eb18:	f94013f5 	ldr	x21, [sp,#32]
    eb1c:	a8c47bfd 	ldp	x29, x30, [sp],#64
    eb20:	d65f03c0 	ret
    eb24:	d53bd042 	mrs	x2, tpidr_el0
    eb28:	b0000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    eb2c:	f947c421 	ldr	x1, [x1,#3976]
    eb30:	b8616843 	ldr	w3, [x2,x1]
    eb34:	7100047f 	cmp	w3, #0x1
    eb38:	54fffec1 	b.ne	eb10 <sem_unlink+0xa0>
    eb3c:	528001a3 	mov	w3, #0xd                   	// #13
    eb40:	b8216843 	str	w3, [x2,x1]
    eb44:	17fffff3 	b	eb10 <sem_unlink+0xa0>
    eb48:	d53bd041 	mrs	x1, tpidr_el0
    eb4c:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    eb50:	f947c442 	ldr	x2, [x2,#3976]
    eb54:	52800043 	mov	w3, #0x2                   	// #2
    eb58:	12800000 	mov	w0, #0xffffffff            	// #-1
    eb5c:	b8226823 	str	w3, [x1,x2]
    eb60:	17ffffec 	b	eb10 <sem_unlink+0xa0>
    eb64:	d53bd041 	mrs	x1, tpidr_el0
    eb68:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    eb6c:	f947c442 	ldr	x2, [x2,#3976]
    eb70:	528004c3 	mov	w3, #0x26                  	// #38
    eb74:	12800000 	mov	w0, #0xffffffff            	// #-1
    eb78:	b8226823 	str	w3, [x1,x2]
    eb7c:	17ffffe5 	b	eb10 <sem_unlink+0xa0>

000000000000eb80 <sem_getvalue@@GLIBC_2.17>:
    eb80:	b9400002 	ldr	w2, [x0]
    eb84:	52800000 	mov	w0, #0x0                   	// #0
    eb88:	b9000022 	str	w2, [x1]
    eb8c:	d65f03c0 	ret

000000000000eb90 <__sem_wait_cleanup>:
    eb90:	91002000 	add	x0, x0, #0x8
    eb94:	c85ffc01 	ldaxr	x1, [x0]
    eb98:	d1000421 	sub	x1, x1, #0x1
    eb9c:	c8027c01 	stxr	w2, x1, [x0]
    eba0:	35ffffa2 	cbnz	w2, eb94 <__sem_wait_cleanup+0x4>
    eba4:	d65f03c0 	ret

000000000000eba8 <do_futex_wait>:
    eba8:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    ebac:	910003fd 	mov	x29, sp
    ebb0:	f9000bf3 	str	x19, [sp,#16]
    ebb4:	aa0003f3 	mov	x19, x0
    ebb8:	94000298 	bl	f618 <__pthread_enable_asynccancel>
    ebbc:	2a0003e4 	mov	w4, w0
    ebc0:	d2800002 	mov	x2, #0x0                   	// #0
    ebc4:	aa1303e0 	mov	x0, x19
    ebc8:	b9800661 	ldrsw	x1, [x19,#4]
    ebcc:	aa0203e3 	mov	x3, x2
    ebd0:	d2800c48 	mov	x8, #0x62                  	// #98
    ebd4:	d4000001 	svc	#0x0
    ebd8:	aa0003e1 	mov	x1, x0
    ebdc:	2a0403e0 	mov	w0, w4
    ebe0:	b13ffc3f 	cmn	x1, #0xfff
    ebe4:	1a813053 	csel	w19, w2, w1, cc
    ebe8:	940002bc 	bl	f6d8 <__pthread_disable_asynccancel>
    ebec:	2a1303e0 	mov	w0, w19
    ebf0:	f9400bf3 	ldr	x19, [sp,#16]
    ebf4:	a8c27bfd 	ldp	x29, x30, [sp],#32
    ebf8:	d65f03c0 	ret

000000000000ebfc <sem_wait@@GLIBC_2.17>:
    ebfc:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    ec00:	910003fd 	mov	x29, sp
    ec04:	b9400001 	ldr	w1, [x0]
    ec08:	a90153f3 	stp	x19, x20, [sp,#16]
    ec0c:	f90013f5 	str	x21, [sp,#32]
    ec10:	910103b4 	add	x20, x29, #0x40
    ec14:	51000423 	sub	w3, w1, #0x1
    ec18:	2a0103e2 	mov	w2, w1
    ec1c:	34000241 	cbz	w1, ec64 <sem_wait@@GLIBC_2.17+0x68>
    ec20:	b90043a1 	str	w1, [x29,#64]
    ec24:	885ffc01 	ldaxr	w1, [x0]
    ec28:	6b02003f 	cmp	w1, w2
    ec2c:	54000061 	b.ne	ec38 <sem_wait@@GLIBC_2.17+0x3c>
    ec30:	88047c03 	stxr	w4, w3, [x0]
    ec34:	35ffff84 	cbnz	w4, ec24 <sem_wait@@GLIBC_2.17+0x28>
    ec38:	540000c1 	b.ne	ec50 <sem_wait@@GLIBC_2.17+0x54>
    ec3c:	52800000 	mov	w0, #0x0                   	// #0
    ec40:	f94013f5 	ldr	x21, [sp,#32]
    ec44:	a94153f3 	ldp	x19, x20, [sp,#16]
    ec48:	a8c67bfd 	ldp	x29, x30, [sp],#96
    ec4c:	d65f03c0 	ret
    ec50:	b9000281 	str	w1, [x20]
    ec54:	b9400001 	ldr	w1, [x0]
    ec58:	51000423 	sub	w3, w1, #0x1
    ec5c:	2a0103e2 	mov	w2, w1
    ec60:	35fffe01 	cbnz	w1, ec20 <sem_wait@@GLIBC_2.17+0x24>
    ec64:	aa0003f3 	mov	x19, x0
    ec68:	91002015 	add	x21, x0, #0x8
    ec6c:	c85ffea0 	ldaxr	x0, [x21]
    ec70:	91000400 	add	x0, x0, #0x1
    ec74:	c8017ea0 	stxr	w1, x0, [x21]
    ec78:	35ffffa1 	cbnz	w1, ec6c <sem_wait@@GLIBC_2.17+0x70>
    ec7c:	90000001 	adrp	x1, e000 <__pthread_once+0xf8>
    ec80:	aa1403e0 	mov	x0, x20
    ec84:	912e4021 	add	x1, x1, #0xb90
    ec88:	aa1303e2 	mov	x2, x19
    ec8c:	9400015d 	bl	f200 <_pthread_cleanup_push>
    ec90:	aa1303e0 	mov	x0, x19
    ec94:	97ffffc5 	bl	eba8 <do_futex_wait>
    ec98:	31002c1f 	cmn	w0, #0xb
    ec9c:	54000040 	b.eq	eca4 <sem_wait@@GLIBC_2.17+0xa8>
    eca0:	35000360 	cbnz	w0, ed0c <sem_wait@@GLIBC_2.17+0x110>
    eca4:	b9400261 	ldr	w1, [x19]
    eca8:	51000423 	sub	w3, w1, #0x1
    ecac:	2a0103e2 	mov	w2, w1
    ecb0:	34ffff01 	cbz	w1, ec90 <sem_wait@@GLIBC_2.17+0x94>
    ecb4:	b9003fa1 	str	w1, [x29,#60]
    ecb8:	885ffe61 	ldaxr	w1, [x19]
    ecbc:	6b02003f 	cmp	w1, w2
    ecc0:	54000061 	b.ne	eccc <sem_wait@@GLIBC_2.17+0xd0>
    ecc4:	88007e63 	stxr	w0, w3, [x19]
    ecc8:	35ffff80 	cbnz	w0, ecb8 <sem_wait@@GLIBC_2.17+0xbc>
    eccc:	54000060 	b.eq	ecd8 <sem_wait@@GLIBC_2.17+0xdc>
    ecd0:	b9003fa1 	str	w1, [x29,#60]
    ecd4:	17fffff4 	b	eca4 <sem_wait@@GLIBC_2.17+0xa8>
    ecd8:	52800013 	mov	w19, #0x0                   	// #0
    ecdc:	aa1403e0 	mov	x0, x20
    ece0:	52800001 	mov	w1, #0x0                   	// #0
    ece4:	9400014f 	bl	f220 <_pthread_cleanup_pop>
    ece8:	c85ffea0 	ldaxr	x0, [x21]
    ecec:	d1000400 	sub	x0, x0, #0x1
    ecf0:	c8017ea0 	stxr	w1, x0, [x21]
    ecf4:	35ffffa1 	cbnz	w1, ece8 <sem_wait@@GLIBC_2.17+0xec>
    ecf8:	2a1303e0 	mov	w0, w19
    ecfc:	f94013f5 	ldr	x21, [sp,#32]
    ed00:	a94153f3 	ldp	x19, x20, [sp,#16]
    ed04:	a8c67bfd 	ldp	x29, x30, [sp],#96
    ed08:	d65f03c0 	ret
    ed0c:	4b0003e1 	neg	w1, w0
    ed10:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    ed14:	f947c442 	ldr	x2, [x2,#3976]
    ed18:	d53bd040 	mrs	x0, tpidr_el0
    ed1c:	12800013 	mov	w19, #0xffffffff            	// #-1
    ed20:	b8226801 	str	w1, [x0,x2]
    ed24:	17ffffee 	b	ecdc <sem_wait@@GLIBC_2.17+0xe0>

000000000000ed28 <sem_trywait@@GLIBC_2.17>:
    ed28:	b9400001 	ldr	w1, [x0]
    ed2c:	d10043ff 	sub	sp, sp, #0x10
    ed30:	6b1f003f 	cmp	w1, wzr
    ed34:	540001ed 	b.le	ed70 <sem_trywait@@GLIBC_2.17+0x48>
    ed38:	b9000fe1 	str	w1, [sp,#12]
    ed3c:	51000422 	sub	w2, w1, #0x1
    ed40:	885ffc03 	ldaxr	w3, [x0]
    ed44:	6b01007f 	cmp	w3, w1
    ed48:	54000061 	b.ne	ed54 <sem_trywait@@GLIBC_2.17+0x2c>
    ed4c:	88047c02 	stxr	w4, w2, [x0]
    ed50:	35ffff84 	cbnz	w4, ed40 <sem_trywait@@GLIBC_2.17+0x18>
    ed54:	54000081 	b.ne	ed64 <sem_trywait@@GLIBC_2.17+0x3c>
    ed58:	52800000 	mov	w0, #0x0                   	// #0
    ed5c:	910043ff 	add	sp, sp, #0x10
    ed60:	d65f03c0 	ret
    ed64:	b9400001 	ldr	w1, [x0]
    ed68:	6b1f003f 	cmp	w1, wzr
    ed6c:	54fffe6c 	b.gt	ed38 <sem_trywait@@GLIBC_2.17+0x10>
    ed70:	d53bd041 	mrs	x1, tpidr_el0
    ed74:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    ed78:	f947c442 	ldr	x2, [x2,#3976]
    ed7c:	12800000 	mov	w0, #0xffffffff            	// #-1
    ed80:	52800163 	mov	w3, #0xb                   	// #11
    ed84:	910043ff 	add	sp, sp, #0x10
    ed88:	b8226823 	str	w3, [x1,x2]
    ed8c:	d65f03c0 	ret

000000000000ed90 <do_futex_timed_wait>:
    ed90:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
    ed94:	910003fd 	mov	x29, sp
    ed98:	f9000bf3 	str	x19, [sp,#16]
    ed9c:	aa0003f3 	mov	x19, x0
    eda0:	f90017a1 	str	x1, [x29,#40]
    eda4:	9400021d 	bl	f618 <__pthread_enable_asynccancel>
    eda8:	b9800661 	ldrsw	x1, [x19,#4]
    edac:	2a0003e4 	mov	w4, w0
    edb0:	d2800002 	mov	x2, #0x0                   	// #0
    edb4:	aa1303e0 	mov	x0, x19
    edb8:	f94017a3 	ldr	x3, [x29,#40]
    edbc:	d2800c48 	mov	x8, #0x62                  	// #98
    edc0:	d4000001 	svc	#0x0
    edc4:	aa0003e1 	mov	x1, x0
    edc8:	2a0403e0 	mov	w0, w4
    edcc:	b13ffc3f 	cmn	x1, #0xfff
    edd0:	1a813053 	csel	w19, w2, w1, cc
    edd4:	94000241 	bl	f6d8 <__pthread_disable_asynccancel>
    edd8:	2a1303e0 	mov	w0, w19
    eddc:	f9400bf3 	ldr	x19, [sp,#16]
    ede0:	a8c37bfd 	ldp	x29, x30, [sp],#48
    ede4:	d65f03c0 	ret

000000000000ede8 <sem_timedwait>:
    ede8:	a9b67bfd 	stp	x29, x30, [sp,#-160]!
    edec:	910003fd 	mov	x29, sp
    edf0:	b9400002 	ldr	w2, [x0]
    edf4:	a90153f3 	stp	x19, x20, [sp,#16]
    edf8:	a9025bf5 	stp	x21, x22, [sp,#32]
    edfc:	a90363f7 	stp	x23, x24, [sp,#48]
    ee00:	f90023f9 	str	x25, [sp,#64]
    ee04:	51000444 	sub	w4, w2, #0x1
    ee08:	2a0203e3 	mov	w3, w2
    ee0c:	34000282 	cbz	w2, ee5c <sem_timedwait+0x74>
    ee10:	b90083a2 	str	w2, [x29,#128]
    ee14:	885ffc02 	ldaxr	w2, [x0]
    ee18:	6b03005f 	cmp	w2, w3
    ee1c:	54000061 	b.ne	ee28 <sem_timedwait+0x40>
    ee20:	88057c04 	stxr	w5, w4, [x0]
    ee24:	35ffff85 	cbnz	w5, ee14 <sem_timedwait+0x2c>
    ee28:	54000101 	b.ne	ee48 <sem_timedwait+0x60>
    ee2c:	52800000 	mov	w0, #0x0                   	// #0
    ee30:	a94153f3 	ldp	x19, x20, [sp,#16]
    ee34:	a9425bf5 	ldp	x21, x22, [sp,#32]
    ee38:	a94363f7 	ldp	x23, x24, [sp,#48]
    ee3c:	f94023f9 	ldr	x25, [sp,#64]
    ee40:	a8ca7bfd 	ldp	x29, x30, [sp],#160
    ee44:	d65f03c0 	ret
    ee48:	b90083a2 	str	w2, [x29,#128]
    ee4c:	b9400002 	ldr	w2, [x0]
    ee50:	51000444 	sub	w4, w2, #0x1
    ee54:	2a0203e3 	mov	w3, w2
    ee58:	35fffdc2 	cbnz	w2, ee10 <sem_timedwait+0x28>
    ee5c:	f9400423 	ldr	x3, [x1,#8]
    ee60:	d2993fe2 	mov	x2, #0xc9ff                	// #51711
    ee64:	f2a77342 	movk	x2, #0x3b9a, lsl #16
    ee68:	eb02007f 	cmp	x3, x2
    ee6c:	54000aa8 	b.hi	efc0 <sem_timedwait+0x1d8>
    ee70:	aa0103f4 	mov	x20, x1
    ee74:	aa0003f3 	mov	x19, x0
    ee78:	91002017 	add	x23, x0, #0x8
    ee7c:	c85ffee0 	ldaxr	x0, [x23]
    ee80:	91000400 	add	x0, x0, #0x1
    ee84:	c8017ee0 	stxr	w1, x0, [x23]
    ee88:	35ffffa1 	cbnz	w1, ee7c <sem_timedwait+0x94>
    ee8c:	910203b8 	add	x24, x29, #0x80
    ee90:	90000001 	adrp	x1, e000 <__pthread_once+0xf8>
    ee94:	aa1803e0 	mov	x0, x24
    ee98:	912e4021 	add	x1, x1, #0xb90
    ee9c:	aa1303e2 	mov	x2, x19
    eea0:	910183b6 	add	x22, x29, #0x60
    eea4:	12807cf5 	mov	w21, #0xfffffc18            	// #-1000
    eea8:	9101c3b9 	add	x25, x29, #0x70
    eeac:	940000d5 	bl	f200 <_pthread_cleanup_push>
    eeb0:	aa1603e0 	mov	x0, x22
    eeb4:	d2800001 	mov	x1, #0x0                   	// #0
    eeb8:	97ffd842 	bl	4fc0 <__gettimeofday@plt>
    eebc:	f9400682 	ldr	x2, [x20,#8]
    eec0:	f94037a3 	ldr	x3, [x29,#104]
    eec4:	f9400284 	ldr	x4, [x20]
    eec8:	1b030aa3 	madd	w3, w21, w3, w2
    eecc:	f94033a2 	ldr	x2, [x29,#96]
    eed0:	4b020084 	sub	w4, w4, w2
    eed4:	37f80323 	tbnz	w3, #31, ef38 <sem_timedwait+0x150>
    eed8:	37f803a4 	tbnz	w4, #31, ef4c <sem_timedwait+0x164>
    eedc:	93407c84 	sxtw	x4, w4
    eee0:	93407c63 	sxtw	x3, w3
    eee4:	aa1303e0 	mov	x0, x19
    eee8:	aa1903e1 	mov	x1, x25
    eeec:	f9003ba4 	str	x4, [x29,#112]
    eef0:	f9003fa3 	str	x3, [x29,#120]
    eef4:	97ffffa7 	bl	ed90 <do_futex_timed_wait>
    eef8:	31002c1f 	cmn	w0, #0xb
    eefc:	54000040 	b.eq	ef04 <sem_timedwait+0x11c>
    ef00:	35000520 	cbnz	w0, efa4 <sem_timedwait+0x1bc>
    ef04:	b9400263 	ldr	w3, [x19]
    ef08:	51000460 	sub	w0, w3, #0x1
    ef0c:	2a0303e2 	mov	w2, w3
    ef10:	34fffd03 	cbz	w3, eeb0 <sem_timedwait+0xc8>
    ef14:	b9005fa3 	str	w3, [x29,#92]
    ef18:	885ffe61 	ldaxr	w1, [x19]
    ef1c:	6b02003f 	cmp	w1, w2
    ef20:	54000061 	b.ne	ef2c <sem_timedwait+0x144>
    ef24:	88037e60 	stxr	w3, w0, [x19]
    ef28:	35ffff83 	cbnz	w3, ef18 <sem_timedwait+0x130>
    ef2c:	540001e0 	b.eq	ef68 <sem_timedwait+0x180>
    ef30:	b9005fa1 	str	w1, [x29,#92]
    ef34:	17fffff4 	b	ef04 <sem_timedwait+0x11c>
    ef38:	52994000 	mov	w0, #0xca00                	// #51712
    ef3c:	51000484 	sub	w4, w4, #0x1
    ef40:	72a77340 	movk	w0, #0x3b9a, lsl #16
    ef44:	0b000063 	add	w3, w3, w0
    ef48:	36fffca4 	tbz	w4, #31, eedc <sem_timedwait+0xf4>
    ef4c:	d53bd040 	mrs	x0, tpidr_el0
    ef50:	b0000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    ef54:	f947c421 	ldr	x1, [x1,#3976]
    ef58:	52800dc2 	mov	w2, #0x6e                  	// #110
    ef5c:	12800013 	mov	w19, #0xffffffff            	// #-1
    ef60:	b8216802 	str	w2, [x0,x1]
    ef64:	14000002 	b	ef6c <sem_timedwait+0x184>
    ef68:	52800013 	mov	w19, #0x0                   	// #0
    ef6c:	aa1803e0 	mov	x0, x24
    ef70:	52800001 	mov	w1, #0x0                   	// #0
    ef74:	940000ab 	bl	f220 <_pthread_cleanup_pop>
    ef78:	c85ffee0 	ldaxr	x0, [x23]
    ef7c:	d1000400 	sub	x0, x0, #0x1
    ef80:	c8017ee0 	stxr	w1, x0, [x23]
    ef84:	35ffffa1 	cbnz	w1, ef78 <sem_timedwait+0x190>
    ef88:	2a1303e0 	mov	w0, w19
    ef8c:	f94023f9 	ldr	x25, [sp,#64]
    ef90:	a94153f3 	ldp	x19, x20, [sp,#16]
    ef94:	a9425bf5 	ldp	x21, x22, [sp,#32]
    ef98:	a94363f7 	ldp	x23, x24, [sp,#48]
    ef9c:	a8ca7bfd 	ldp	x29, x30, [sp],#160
    efa0:	d65f03c0 	ret
    efa4:	4b0003e1 	neg	w1, w0
    efa8:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    efac:	f947c442 	ldr	x2, [x2,#3976]
    efb0:	d53bd040 	mrs	x0, tpidr_el0
    efb4:	12800013 	mov	w19, #0xffffffff            	// #-1
    efb8:	b8226801 	str	w1, [x0,x2]
    efbc:	17ffffec 	b	ef6c <sem_timedwait+0x184>
    efc0:	d53bd041 	mrs	x1, tpidr_el0
    efc4:	b0000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    efc8:	f947c442 	ldr	x2, [x2,#3976]
    efcc:	528002c3 	mov	w3, #0x16                  	// #22
    efd0:	12800000 	mov	w0, #0xffffffff            	// #-1
    efd4:	b8226823 	str	w3, [x1,x2]
    efd8:	17ffff96 	b	ee30 <sem_timedwait+0x48>

000000000000efdc <sem_post@@GLIBC_2.17>:
    efdc:	d10043ff 	sub	sp, sp, #0x10
    efe0:	12b00004 	mov	w4, #0x7fffffff            	// #2147483647
    efe4:	b9400001 	ldr	w1, [x0]
    efe8:	6b04003f 	cmp	w1, w4
    efec:	11000423 	add	w3, w1, #0x1
    eff0:	2a0103e2 	mov	w2, w1
    eff4:	540002c0 	b.eq	f04c <sem_post@@GLIBC_2.17+0x70>
    eff8:	b9000fe1 	str	w1, [sp,#12]
    effc:	885f7c01 	ldxr	w1, [x0]
    f000:	6b02003f 	cmp	w1, w2
    f004:	54000061 	b.ne	f010 <sem_post@@GLIBC_2.17+0x34>
    f008:	8805fc03 	stlxr	w5, w3, [x0]
    f00c:	35ffff85 	cbnz	w5, effc <sem_post@@GLIBC_2.17+0x20>
    f010:	54fffea1 	b.ne	efe4 <sem_post@@GLIBC_2.17+0x8>
    f014:	d5033bbf 	dmb	ish
    f018:	f9400401 	ldr	x1, [x0,#8]
    f01c:	b4000141 	cbz	x1, f044 <sem_post@@GLIBC_2.17+0x68>
    f020:	b9400401 	ldr	w1, [x0,#4]
    f024:	d2800022 	mov	x2, #0x1                   	// #1
    f028:	d2800003 	mov	x3, #0x0                   	// #0
    f02c:	d2800c48 	mov	x8, #0x62                  	// #98
    f030:	52000021 	eor	w1, w1, #0x1
    f034:	93407c21 	sxtw	x1, w1
    f038:	d4000001 	svc	#0x0
    f03c:	b140041f 	cmn	x0, #0x1, lsl #12
    f040:	54000168 	b.hi	f06c <sem_post@@GLIBC_2.17+0x90>
    f044:	52800000 	mov	w0, #0x0                   	// #0
    f048:	14000007 	b	f064 <sem_post@@GLIBC_2.17+0x88>
    f04c:	d53bd041 	mrs	x1, tpidr_el0
    f050:	90000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    f054:	f947c442 	ldr	x2, [x2,#3976]
    f058:	52800963 	mov	w3, #0x4b                  	// #75
    f05c:	12800000 	mov	w0, #0xffffffff            	// #-1
    f060:	b8226823 	str	w3, [x1,x2]
    f064:	910043ff 	add	sp, sp, #0x10
    f068:	d65f03c0 	ret
    f06c:	d53bd042 	mrs	x2, tpidr_el0
    f070:	90000103 	adrp	x3, 2f000 <__FRAME_END__+0x18e30>
    f074:	f947c463 	ldr	x3, [x3,#3976]
    f078:	4b0003e1 	neg	w1, w0
    f07c:	12800000 	mov	w0, #0xffffffff            	// #-1
    f080:	b8236841 	str	w1, [x2,x3]
    f084:	17fffff8 	b	f064 <sem_post@@GLIBC_2.17+0x88>

000000000000f088 <__pthread_register_cancel>:
    f088:	d53bd041 	mrs	x1, tpidr_el0
    f08c:	d11bc021 	sub	x1, x1, #0x6f0
    f090:	f9408022 	ldr	x2, [x1,#256]
    f094:	f9005c02 	str	x2, [x0,#184]
    f098:	f9407c22 	ldr	x2, [x1,#248]
    f09c:	f9006002 	str	x2, [x0,#192]
    f0a0:	f9008020 	str	x0, [x1,#256]
    f0a4:	d65f03c0 	ret

000000000000f0a8 <__pthread_unregister_cancel>:
    f0a8:	f9405c01 	ldr	x1, [x0,#184]
    f0ac:	d53bd040 	mrs	x0, tpidr_el0
    f0b0:	d11bc000 	sub	x0, x0, #0x6f0
    f0b4:	f9008001 	str	x1, [x0,#256]
    f0b8:	d65f03c0 	ret

000000000000f0bc <__pthread_register_cancel_defer>:
    f0bc:	d53bd041 	mrs	x1, tpidr_el0
    f0c0:	d10043ff 	sub	sp, sp, #0x10
    f0c4:	d11bc021 	sub	x1, x1, #0x6f0
    f0c8:	f9408022 	ldr	x2, [x1,#256]
    f0cc:	f9005c02 	str	x2, [x0,#184]
    f0d0:	f9407c22 	ldr	x2, [x1,#248]
    f0d4:	f9006002 	str	x2, [x0,#192]
    f0d8:	b9410822 	ldr	w2, [x1,#264]
    f0dc:	370800c2 	tbnz	w2, #1, f0f4 <__pthread_register_cancel_defer+0x38>
    f0e0:	d3410442 	ubfx	x2, x2, #1, #1
    f0e4:	b900c802 	str	w2, [x0,#200]
    f0e8:	f9008020 	str	x0, [x1,#256]
    f0ec:	910043ff 	add	sp, sp, #0x10
    f0f0:	d65f03c0 	ret
    f0f4:	91042023 	add	x3, x1, #0x108
    f0f8:	2a0203e4 	mov	w4, w2
    f0fc:	910033e7 	add	x7, sp, #0xc
    f100:	b9000fe2 	str	w2, [sp,#12]
    f104:	121e7845 	and	w5, w2, #0xfffffffd
    f108:	885ffc66 	ldaxr	w6, [x3]
    f10c:	6b0200df 	cmp	w6, w2
    f110:	54000061 	b.ne	f11c <__pthread_register_cancel_defer+0x60>
    f114:	88087c65 	stxr	w8, w5, [x3]
    f118:	35ffff88 	cbnz	w8, f108 <__pthread_register_cancel_defer+0x4c>
    f11c:	54000040 	b.eq	f124 <__pthread_register_cancel_defer+0x68>
    f120:	b90000e6 	str	w6, [x7]
    f124:	b9400fe2 	ldr	w2, [sp,#12]
    f128:	6b02009f 	cmp	w4, w2
    f12c:	54fffda0 	b.eq	f0e0 <__pthread_register_cancel_defer+0x24>
    f130:	2a0203e4 	mov	w4, w2
    f134:	17fffff3 	b	f100 <__pthread_register_cancel_defer+0x44>

000000000000f138 <__pthread_unregister_cancel_restore>:
    f138:	d53bd042 	mrs	x2, tpidr_el0
    f13c:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    f140:	d11bc041 	sub	x1, x2, #0x6f0
    f144:	910003fd 	mov	x29, sp
    f148:	f9405c03 	ldr	x3, [x0,#184]
    f14c:	f9008023 	str	x3, [x1,#256]
    f150:	b940c800 	ldr	w0, [x0,#200]
    f154:	340002e0 	cbz	w0, f1b0 <__pthread_unregister_cancel_restore+0x78>
    f158:	b9410820 	ldr	w0, [x1,#264]
    f15c:	370802a0 	tbnz	w0, #1, f1b0 <__pthread_unregister_cancel_restore+0x78>
    f160:	91042024 	add	x4, x1, #0x108
    f164:	910073a7 	add	x7, x29, #0x1c
    f168:	b9001fa0 	str	w0, [x29,#28]
    f16c:	321f0003 	orr	w3, w0, #0x2
    f170:	2a0003e5 	mov	w5, w0
    f174:	885ffc86 	ldaxr	w6, [x4]
    f178:	6b0500df 	cmp	w6, w5
    f17c:	54000061 	b.ne	f188 <__pthread_unregister_cancel_restore+0x50>
    f180:	88087c83 	stxr	w8, w3, [x4]
    f184:	35ffff88 	cbnz	w8, f174 <__pthread_unregister_cancel_restore+0x3c>
    f188:	54000040 	b.eq	f190 <__pthread_unregister_cancel_restore+0x58>
    f18c:	b90000e6 	str	w6, [x7]
    f190:	b9401fa3 	ldr	w3, [x29,#28]
    f194:	6b03001f 	cmp	w0, w3
    f198:	540002c1 	b.ne	f1f0 <__pthread_unregister_cancel_restore+0xb8>
    f19c:	b9410823 	ldr	w3, [x1,#264]
    f1a0:	128008c0 	mov	w0, #0xffffffb9            	// #-71
    f1a4:	0a000060 	and	w0, w3, w0
    f1a8:	7100201f 	cmp	w0, #0x8
    f1ac:	54000060 	b.eq	f1b8 <__pthread_unregister_cancel_restore+0x80>
    f1b0:	a8c27bfd 	ldp	x29, x30, [sp],#32
    f1b4:	d65f03c0 	ret
    f1b8:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    f1bc:	d117a042 	sub	x2, x2, #0x5e8
    f1c0:	f9021420 	str	x0, [x1,#1064]
    f1c4:	b9400040 	ldr	w0, [x2]
    f1c8:	b9001fa0 	str	w0, [x29,#28]
    f1cc:	321c0005 	orr	w5, w0, #0x10
    f1d0:	885ffc83 	ldaxr	w3, [x4]
    f1d4:	6b00007f 	cmp	w3, w0
    f1d8:	54000061 	b.ne	f1e4 <__pthread_unregister_cancel_restore+0xac>
    f1dc:	88067c85 	stxr	w6, w5, [x4]
    f1e0:	35ffff86 	cbnz	w6, f1d0 <__pthread_unregister_cancel_restore+0x98>
    f1e4:	540000a0 	b.eq	f1f8 <__pthread_unregister_cancel_restore+0xc0>
    f1e8:	b90000e3 	str	w3, [x7]
    f1ec:	17fffff6 	b	f1c4 <__pthread_unregister_cancel_restore+0x8c>
    f1f0:	2a0303e0 	mov	w0, w3
    f1f4:	17ffffdd 	b	f168 <__pthread_unregister_cancel_restore+0x30>
    f1f8:	f9408020 	ldr	x0, [x1,#256]
    f1fc:	940000c2 	bl	f504 <__pthread_unwind>

000000000000f200 <_pthread_cleanup_push>:
    f200:	d53bd043 	mrs	x3, tpidr_el0
    f204:	f9000001 	str	x1, [x0]
    f208:	d11bc063 	sub	x3, x3, #0x6f0
    f20c:	f9407c61 	ldr	x1, [x3,#248]
    f210:	f9000c01 	str	x1, [x0,#24]
    f214:	f9000402 	str	x2, [x0,#8]
    f218:	f9007c60 	str	x0, [x3,#248]
    f21c:	d65f03c0 	ret

000000000000f220 <_pthread_cleanup_pop>:
    f220:	d53bd042 	mrs	x2, tpidr_el0
    f224:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    f228:	d11bc042 	sub	x2, x2, #0x6f0
    f22c:	910003fd 	mov	x29, sp
    f230:	f9400c03 	ldr	x3, [x0,#24]
    f234:	f9007c43 	str	x3, [x2,#248]
    f238:	34000081 	cbz	w1, f248 <_pthread_cleanup_pop+0x28>
    f23c:	f9400001 	ldr	x1, [x0]
    f240:	f9400400 	ldr	x0, [x0,#8]
    f244:	d63f0020 	blr	x1
    f248:	a8c17bfd 	ldp	x29, x30, [sp],#16
    f24c:	d65f03c0 	ret

000000000000f250 <_pthread_cleanup_push_defer>:
    f250:	d53bd043 	mrs	x3, tpidr_el0
    f254:	f9000001 	str	x1, [x0]
    f258:	d11bc063 	sub	x3, x3, #0x6f0
    f25c:	d10043ff 	sub	sp, sp, #0x10
    f260:	f9407c64 	ldr	x4, [x3,#248]
    f264:	b9410861 	ldr	w1, [x3,#264]
    f268:	f9000402 	str	x2, [x0,#8]
    f26c:	f9000c04 	str	x4, [x0,#24]
    f270:	370800c1 	tbnz	w1, #1, f288 <_pthread_cleanup_push_defer+0x38>
    f274:	d3410421 	ubfx	x1, x1, #1, #1
    f278:	b9001001 	str	w1, [x0,#16]
    f27c:	f9007c60 	str	x0, [x3,#248]
    f280:	910043ff 	add	sp, sp, #0x10
    f284:	d65f03c0 	ret
    f288:	91042062 	add	x2, x3, #0x108
    f28c:	2a0103e4 	mov	w4, w1
    f290:	910033e7 	add	x7, sp, #0xc
    f294:	b9000fe1 	str	w1, [sp,#12]
    f298:	121e7825 	and	w5, w1, #0xfffffffd
    f29c:	885ffc46 	ldaxr	w6, [x2]
    f2a0:	6b0100df 	cmp	w6, w1
    f2a4:	54000061 	b.ne	f2b0 <_pthread_cleanup_push_defer+0x60>
    f2a8:	88087c45 	stxr	w8, w5, [x2]
    f2ac:	35ffff88 	cbnz	w8, f29c <_pthread_cleanup_push_defer+0x4c>
    f2b0:	54000040 	b.eq	f2b8 <_pthread_cleanup_push_defer+0x68>
    f2b4:	b90000e6 	str	w6, [x7]
    f2b8:	b9400fe1 	ldr	w1, [sp,#12]
    f2bc:	6b01009f 	cmp	w4, w1
    f2c0:	54fffda0 	b.eq	f274 <_pthread_cleanup_push_defer+0x24>
    f2c4:	2a0103e4 	mov	w4, w1
    f2c8:	17fffff3 	b	f294 <_pthread_cleanup_push_defer+0x44>

000000000000f2cc <_pthread_cleanup_pop_restore>:
    f2cc:	d53bd043 	mrs	x3, tpidr_el0
    f2d0:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    f2d4:	d11bc062 	sub	x2, x3, #0x6f0
    f2d8:	910003fd 	mov	x29, sp
    f2dc:	f9400c05 	ldr	x5, [x0,#24]
    f2e0:	b9401004 	ldr	w4, [x0,#16]
    f2e4:	f9007c45 	str	x5, [x2,#248]
    f2e8:	350000e4 	cbnz	w4, f304 <_pthread_cleanup_pop_restore+0x38>
    f2ec:	34000081 	cbz	w1, f2fc <_pthread_cleanup_pop_restore+0x30>
    f2f0:	f9400001 	ldr	x1, [x0]
    f2f4:	f9400400 	ldr	x0, [x0,#8]
    f2f8:	d63f0020 	blr	x1
    f2fc:	a8c27bfd 	ldp	x29, x30, [sp],#32
    f300:	d65f03c0 	ret
    f304:	b9410844 	ldr	w4, [x2,#264]
    f308:	370fff24 	tbnz	w4, #1, f2ec <_pthread_cleanup_pop_restore+0x20>
    f30c:	91042046 	add	x6, x2, #0x108
    f310:	910073a9 	add	x9, x29, #0x1c
    f314:	b9001fa4 	str	w4, [x29,#28]
    f318:	321f0085 	orr	w5, w4, #0x2
    f31c:	2a0403e7 	mov	w7, w4
    f320:	885ffcc8 	ldaxr	w8, [x6]
    f324:	6b07011f 	cmp	w8, w7
    f328:	54000061 	b.ne	f334 <_pthread_cleanup_pop_restore+0x68>
    f32c:	880a7cc5 	stxr	w10, w5, [x6]
    f330:	35ffff8a 	cbnz	w10, f320 <_pthread_cleanup_pop_restore+0x54>
    f334:	540002e1 	b.ne	f390 <_pthread_cleanup_pop_restore+0xc4>
    f338:	b9401fa5 	ldr	w5, [x29,#28]
    f33c:	6b05009f 	cmp	w4, w5
    f340:	540002c1 	b.ne	f398 <_pthread_cleanup_pop_restore+0xcc>
    f344:	b9410845 	ldr	w5, [x2,#264]
    f348:	128008c4 	mov	w4, #0xffffffb9            	// #-71
    f34c:	0a0400a4 	and	w4, w5, w4
    f350:	7100209f 	cmp	w4, #0x8
    f354:	54fffcc1 	b.ne	f2ec <_pthread_cleanup_pop_restore+0x20>
    f358:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    f35c:	d117a063 	sub	x3, x3, #0x5e8
    f360:	f9021440 	str	x0, [x2,#1064]
    f364:	b9400060 	ldr	w0, [x3]
    f368:	b9001fa0 	str	w0, [x29,#28]
    f36c:	321c0004 	orr	w4, w0, #0x10
    f370:	885ffcc1 	ldaxr	w1, [x6]
    f374:	6b00003f 	cmp	w1, w0
    f378:	54000061 	b.ne	f384 <_pthread_cleanup_pop_restore+0xb8>
    f37c:	88057cc4 	stxr	w5, w4, [x6]
    f380:	35ffff85 	cbnz	w5, f370 <_pthread_cleanup_pop_restore+0xa4>
    f384:	540000e0 	b.eq	f3a0 <_pthread_cleanup_pop_restore+0xd4>
    f388:	b9000121 	str	w1, [x9]
    f38c:	17fffff6 	b	f364 <_pthread_cleanup_pop_restore+0x98>
    f390:	b9000128 	str	w8, [x9]
    f394:	17ffffe9 	b	f338 <_pthread_cleanup_pop_restore+0x6c>
    f398:	2a0503e4 	mov	w4, w5
    f39c:	17ffffde 	b	f314 <_pthread_cleanup_pop_restore+0x48>
    f3a0:	f9408040 	ldr	x0, [x2,#256]
    f3a4:	94000058 	bl	f504 <__pthread_unwind>

000000000000f3a8 <unwind_cleanup>:
    f3a8:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    f3ac:	f0000001 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
    f3b0:	d2800040 	mov	x0, #0x2                   	// #2
    f3b4:	910003fd 	mov	x29, sp
    f3b8:	9134c021 	add	x1, x1, #0xd30
    f3bc:	d28003c2 	mov	x2, #0x1e                  	// #30
    f3c0:	d2800808 	mov	x8, #0x40                  	// #64
    f3c4:	d4000001 	svc	#0x0
    f3c8:	97ffd76a 	bl	5170 <abort@plt>

000000000000f3cc <unwind_stop>:
    f3cc:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    f3d0:	910003fd 	mov	x29, sp
    f3d4:	a90153f3 	stp	x19, x20, [sp,#16]
    f3d8:	d53bd054 	mrs	x20, tpidr_el0
    f3dc:	f90023f9 	str	x25, [sp,#64]
    f3e0:	d11bc294 	sub	x20, x20, #0x6f0
    f3e4:	a9025bf5 	stp	x21, x22, [sp,#32]
    f3e8:	a90363f7 	stp	x23, x24, [sp,#48]
    f3ec:	f9424a95 	ldr	x21, [x20,#1168]
    f3f0:	aa0503f7 	mov	x23, x5
    f3f4:	f9424e82 	ldr	x2, [x20,#1176]
    f3f8:	f9407e93 	ldr	x19, [x20,#248]
    f3fc:	8b0202b5 	add	x21, x21, x2
    f400:	362000a1 	tbz	w1, #4, f414 <unwind_stop+0x48>
    f404:	b5000333 	cbnz	x19, f468 <unwind_stop+0x9c>
    f408:	aa1703e0 	mov	x0, x23
    f40c:	52800021 	mov	w1, #0x1                   	// #1
    f410:	97ffd7e4 	bl	53a0 <__libc_longjmp@plt>
    f414:	aa0403e0 	mov	x0, x4
    f418:	f9002fa4 	str	x4, [x29,#88]
    f41c:	940009a2 	bl	11aa4 <_Unwind_GetCFA>
    f420:	cb150000 	sub	x0, x0, x21
    f424:	90000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    f428:	f94036e3 	ldr	x3, [x23,#104]
    f42c:	f9402fa4 	ldr	x4, [x29,#88]
    f430:	f947d042 	ldr	x2, [x2,#4000]
    f434:	f9400041 	ldr	x1, [x2]
    f438:	ca010061 	eor	x1, x3, x1
    f43c:	cb150021 	sub	x1, x1, x21
    f440:	eb01001f 	cmp	x0, x1
    f444:	54fffe02 	b.cs	f404 <unwind_stop+0x38>
    f448:	b5000313 	cbnz	x19, f4a8 <unwind_stop+0xdc>
    f44c:	52800000 	mov	w0, #0x0                   	// #0
    f450:	f94023f9 	ldr	x25, [sp,#64]
    f454:	a94153f3 	ldp	x19, x20, [sp,#16]
    f458:	a9425bf5 	ldp	x21, x22, [sp,#32]
    f45c:	a94363f7 	ldp	x23, x24, [sp,#48]
    f460:	a8c67bfd 	ldp	x29, x30, [sp],#96
    f464:	d65f03c0 	ret
    f468:	aa0403e0 	mov	x0, x4
    f46c:	f94062f5 	ldr	x21, [x23,#192]
    f470:	9400098d 	bl	11aa4 <_Unwind_GetCFA>
    f474:	eb15027f 	cmp	x19, x21
    f478:	54fffc80 	b.eq	f408 <unwind_stop+0x3c>
    f47c:	f9400261 	ldr	x1, [x19]
    f480:	f9400660 	ldr	x0, [x19,#8]
    f484:	f9400e76 	ldr	x22, [x19,#24]
    f488:	d63f0020 	blr	x1
    f48c:	aa1603f3 	mov	x19, x22
    f490:	eb1502df 	cmp	x22, x21
    f494:	54ffff41 	b.ne	f47c <unwind_stop+0xb0>
    f498:	52800020 	mov	w0, #0x1                   	// #1
    f49c:	f9007e96 	str	x22, [x20,#248]
    f4a0:	34fffd60 	cbz	w0, f44c <unwind_stop+0x80>
    f4a4:	17ffffd9 	b	f408 <unwind_stop+0x3c>
    f4a8:	aa0403e0 	mov	x0, x4
    f4ac:	f94062f8 	ldr	x24, [x23,#192]
    f4b0:	9400097d 	bl	11aa4 <_Unwind_GetCFA>
    f4b4:	eb18027f 	cmp	x19, x24
    f4b8:	54fffca0 	b.eq	f44c <unwind_stop+0x80>
    f4bc:	cb150019 	sub	x25, x0, x21
    f4c0:	cb150260 	sub	x0, x19, x21
    f4c4:	eb00033f 	cmp	x25, x0
    f4c8:	54fffc23 	b.cc	f44c <unwind_stop+0x80>
    f4cc:	f9400261 	ldr	x1, [x19]
    f4d0:	f9400660 	ldr	x0, [x19,#8]
    f4d4:	f9400e76 	ldr	x22, [x19,#24]
    f4d8:	d63f0020 	blr	x1
    f4dc:	eb16031f 	cmp	x24, x22
    f4e0:	540000e0 	b.eq	f4fc <unwind_stop+0x130>
    f4e4:	cb1502c1 	sub	x1, x22, x21
    f4e8:	aa1603f3 	mov	x19, x22
    f4ec:	eb19003f 	cmp	x1, x25
    f4f0:	54fffee9 	b.ls	f4cc <unwind_stop+0x100>
    f4f4:	f9007e96 	str	x22, [x20,#248]
    f4f8:	17ffffd5 	b	f44c <unwind_stop+0x80>
    f4fc:	52800000 	mov	w0, #0x0                   	// #0
    f500:	17ffffe7 	b	f49c <unwind_stop+0xd0>

000000000000f504 <__pthread_unwind>:
    f504:	d53bd043 	mrs	x3, tpidr_el0
    f508:	90000001 	adrp	x1, f000 <sem_post@@GLIBC_2.17+0x24>
    f50c:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    f510:	d11bc063 	sub	x3, x3, #0x6f0
    f514:	90000004 	adrp	x4, f000 <sem_post@@GLIBC_2.17+0x24>
    f518:	910003fd 	mov	x29, sp
    f51c:	aa0003e2 	mov	x2, x0
    f520:	910ea084 	add	x4, x4, #0x3a8
    f524:	910f3021 	add	x1, x1, #0x3cc
    f528:	f902387f 	str	xzr, [x3,#1136]
    f52c:	9111c060 	add	x0, x3, #0x470
    f530:	f9023c64 	str	x4, [x3,#1144]
    f534:	94000943 	bl	11a40 <_Unwind_ForcedUnwind>
    f538:	97ffd70e 	bl	5170 <abort@plt>

000000000000f53c <__pthread_unwind_next>:
    f53c:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    f540:	910003fd 	mov	x29, sp
    f544:	f9405c00 	ldr	x0, [x0,#184]
    f548:	97ffffef 	bl	f504 <__pthread_unwind>

000000000000f54c <longjmp>:
    f54c:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
    f550:	910003fd 	mov	x29, sp
    f554:	97ffd793 	bl	53a0 <__libc_longjmp@plt>

000000000000f558 <__GI___pthread_cleanup_upto>:
    f558:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
    f55c:	910003fd 	mov	x29, sp
    f560:	a90363f7 	stp	x23, x24, [sp,#48]
    f564:	d53bd058 	mrs	x24, tpidr_el0
    f568:	d11bc318 	sub	x24, x24, #0x6f0
    f56c:	a90153f3 	stp	x19, x20, [sp,#16]
    f570:	a9025bf5 	stp	x21, x22, [sp,#32]
    f574:	f9424b14 	ldr	x20, [x24,#1168]
    f578:	f9424f02 	ldr	x2, [x24,#1176]
    f57c:	f9407f13 	ldr	x19, [x24,#248]
    f580:	8b020294 	add	x20, x20, x2
    f584:	cb140037 	sub	x23, x1, x20
    f588:	b40003b3 	cbz	x19, f5fc <__GI___pthread_cleanup_upto+0xa4>
    f58c:	90000116 	adrp	x22, 2f000 <__FRAME_END__+0x18e30>
    f590:	f9403403 	ldr	x3, [x0,#104]
    f594:	cb140262 	sub	x2, x19, x20
    f598:	f947d2c1 	ldr	x1, [x22,#4000]
    f59c:	f9400021 	ldr	x1, [x1]
    f5a0:	ca010061 	eor	x1, x3, x1
    f5a4:	cb140021 	sub	x1, x1, x20
    f5a8:	eb01005f 	cmp	x2, x1
    f5ac:	540002a2 	b.cs	f600 <__GI___pthread_cleanup_upto+0xa8>
    f5b0:	eb0202ff 	cmp	x23, x2
    f5b4:	aa0003f5 	mov	x21, x0
    f5b8:	54000163 	b.cc	f5e4 <__GI___pthread_cleanup_upto+0x8c>
    f5bc:	14000010 	b	f5fc <__GI___pthread_cleanup_upto+0xa4>
    f5c0:	f947d2c2 	ldr	x2, [x22,#4000]
    f5c4:	f94036a4 	ldr	x4, [x21,#104]
    f5c8:	f9400042 	ldr	x2, [x2]
    f5cc:	ca020082 	eor	x2, x4, x2
    f5d0:	cb140042 	sub	x2, x2, x20
    f5d4:	eb02007f 	cmp	x3, x2
    f5d8:	54000142 	b.cs	f600 <__GI___pthread_cleanup_upto+0xa8>
    f5dc:	eb0302ff 	cmp	x23, x3
    f5e0:	540000e2 	b.cs	f5fc <__GI___pthread_cleanup_upto+0xa4>
    f5e4:	f9400262 	ldr	x2, [x19]
    f5e8:	f9400660 	ldr	x0, [x19,#8]
    f5ec:	d63f0040 	blr	x2
    f5f0:	f9400e73 	ldr	x19, [x19,#24]
    f5f4:	cb140263 	sub	x3, x19, x20
    f5f8:	b5fffe53 	cbnz	x19, f5c0 <__GI___pthread_cleanup_upto+0x68>
    f5fc:	d2800013 	mov	x19, #0x0                   	// #0
    f600:	f9007f13 	str	x19, [x24,#248]
    f604:	a9425bf5 	ldp	x21, x22, [sp,#32]
    f608:	a94153f3 	ldp	x19, x20, [sp,#16]
    f60c:	a94363f7 	ldp	x23, x24, [sp,#48]
    f610:	a8c47bfd 	ldp	x29, x30, [sp],#64
    f614:	d65f03c0 	ret

000000000000f618 <__pthread_enable_asynccancel>:
    f618:	d53bd046 	mrs	x6, tpidr_el0
    f61c:	d11bc0c5 	sub	x5, x6, #0x6f0
    f620:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
    f624:	910003fd 	mov	x29, sp
    f628:	b94108a1 	ldr	w1, [x5,#264]
    f62c:	321f0022 	orr	w2, w1, #0x2
    f630:	6b01005f 	cmp	w2, w1
    f634:	540002e0 	b.eq	f690 <__pthread_enable_asynccancel+0x78>
    f638:	910083a4 	add	x4, x29, #0x20
    f63c:	910420a3 	add	x3, x5, #0x108
    f640:	2a0103e0 	mov	w0, w1
    f644:	b81fcc81 	str	w1, [x4,#-4]!
    f648:	885ffc67 	ldaxr	w7, [x3]
    f64c:	6b0000ff 	cmp	w7, w0
    f650:	54000061 	b.ne	f65c <__pthread_enable_asynccancel+0x44>
    f654:	88087c62 	stxr	w8, w2, [x3]
    f658:	35ffff88 	cbnz	w8, f648 <__pthread_enable_asynccancel+0x30>
    f65c:	54000040 	b.eq	f664 <__pthread_enable_asynccancel+0x4c>
    f660:	b9001fa7 	str	w7, [x29,#28]
    f664:	b9401fa0 	ldr	w0, [x29,#28]
    f668:	6b00003f 	cmp	w1, w0
    f66c:	540000e1 	b.ne	f688 <__pthread_enable_asynccancel+0x70>
    f670:	12800881 	mov	w1, #0xffffffbb            	// #-69
    f674:	0a010042 	and	w2, w2, w1
    f678:	7100285f 	cmp	w2, #0xa
    f67c:	540000e0 	b.eq	f698 <__pthread_enable_asynccancel+0x80>
    f680:	a8c27bfd 	ldp	x29, x30, [sp],#32
    f684:	d65f03c0 	ret
    f688:	2a0003e1 	mov	w1, w0
    f68c:	17ffffe8 	b	f62c <__pthread_enable_asynccancel+0x14>
    f690:	2a0103e0 	mov	w0, w1
    f694:	17fffffb 	b	f680 <__pthread_enable_asynccancel+0x68>
    f698:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    f69c:	d117a0c6 	sub	x6, x6, #0x5e8
    f6a0:	f90214a0 	str	x0, [x5,#1064]
    f6a4:	b94000c0 	ldr	w0, [x6]
    f6a8:	b9001fa0 	str	w0, [x29,#28]
    f6ac:	321c0002 	orr	w2, w0, #0x10
    f6b0:	885ffc61 	ldaxr	w1, [x3]
    f6b4:	6b00003f 	cmp	w1, w0
    f6b8:	54000061 	b.ne	f6c4 <__pthread_enable_asynccancel+0xac>
    f6bc:	88077c62 	stxr	w7, w2, [x3]
    f6c0:	35ffff87 	cbnz	w7, f6b0 <__pthread_enable_asynccancel+0x98>
    f6c4:	54000060 	b.eq	f6d0 <__pthread_enable_asynccancel+0xb8>
    f6c8:	b9000081 	str	w1, [x4]
    f6cc:	17fffff6 	b	f6a4 <__pthread_enable_asynccancel+0x8c>
    f6d0:	f94080a0 	ldr	x0, [x5,#256]
    f6d4:	97ffff8c 	bl	f504 <__pthread_unwind>

000000000000f6d8 <__pthread_disable_asynccancel>:
    f6d8:	d10043ff 	sub	sp, sp, #0x10
    f6dc:	370802a0 	tbnz	w0, #1, f730 <__pthread_disable_asynccancel+0x58>
    f6e0:	d53bd044 	mrs	x4, tpidr_el0
    f6e4:	910033e6 	add	x6, sp, #0xc
    f6e8:	d11bc084 	sub	x4, x4, #0x6f0
    f6ec:	91042085 	add	x5, x4, #0x108
    f6f0:	b9410881 	ldr	w1, [x4,#264]
    f6f4:	b9000fe1 	str	w1, [sp,#12]
    f6f8:	121e7822 	and	w2, w1, #0xfffffffd
    f6fc:	2a0103e0 	mov	w0, w1
    f700:	885ffca3 	ldaxr	w3, [x5]
    f704:	6b00007f 	cmp	w3, w0
    f708:	54000061 	b.ne	f714 <__pthread_disable_asynccancel+0x3c>
    f70c:	88077ca2 	stxr	w7, w2, [x5]
    f710:	35ffff87 	cbnz	w7, f700 <__pthread_disable_asynccancel+0x28>
    f714:	54000121 	b.ne	f738 <__pthread_disable_asynccancel+0x60>
    f718:	b9400fe0 	ldr	w0, [sp,#12]
    f71c:	6b00003f 	cmp	w1, w0
    f720:	54000201 	b.ne	f760 <__pthread_disable_asynccancel+0x88>
    f724:	121e0440 	and	w0, w2, #0xc
    f728:	7100101f 	cmp	w0, #0x4
    f72c:	540000a0 	b.eq	f740 <__pthread_disable_asynccancel+0x68>
    f730:	910043ff 	add	sp, sp, #0x10
    f734:	d65f03c0 	ret
    f738:	b90000c3 	str	w3, [x6]
    f73c:	17fffff7 	b	f718 <__pthread_disable_asynccancel+0x40>
    f740:	aa0503e0 	mov	x0, x5
    f744:	d2801001 	mov	x1, #0x80                  	// #128
    f748:	93407c42 	sxtw	x2, w2
    f74c:	d2800003 	mov	x3, #0x0                   	// #0
    f750:	d2800c48 	mov	x8, #0x62                  	// #98
    f754:	d4000001 	svc	#0x0
    f758:	b9410882 	ldr	w2, [x4,#264]
    f75c:	17fffff2 	b	f724 <__pthread_disable_asynccancel+0x4c>
    f760:	2a0003e1 	mov	w1, w0
    f764:	17ffffe4 	b	f6f4 <__pthread_disable_asynccancel+0x1c>

000000000000f768 <__lll_lock_wait_private>:
    f768:	b9400001 	ldr	w1, [x0]
    f76c:	aa0003e4 	mov	x4, x0
    f770:	7100083f 	cmp	w1, #0x2
    f774:	540001c0 	b.eq	f7ac <__lll_lock_wait_private+0x44>
    f778:	52800045 	mov	w5, #0x2                   	// #2
    f77c:	14000007 	b	f798 <__lll_lock_wait_private+0x30>
    f780:	aa0403e0 	mov	x0, x4
    f784:	d2801001 	mov	x1, #0x80                  	// #128
    f788:	d2800042 	mov	x2, #0x2                   	// #2
    f78c:	d2800003 	mov	x3, #0x0                   	// #0
    f790:	d2800c48 	mov	x8, #0x62                  	// #98
    f794:	d4000001 	svc	#0x0
    f798:	885ffc80 	ldaxr	w0, [x4]
    f79c:	88017c85 	stxr	w1, w5, [x4]
    f7a0:	35ffffc1 	cbnz	w1, f798 <__lll_lock_wait_private+0x30>
    f7a4:	35fffee0 	cbnz	w0, f780 <__lll_lock_wait_private+0x18>
    f7a8:	d65f03c0 	ret
    f7ac:	d2801001 	mov	x1, #0x80                  	// #128
    f7b0:	d2800042 	mov	x2, #0x2                   	// #2
    f7b4:	d2800003 	mov	x3, #0x0                   	// #0
    f7b8:	d2800c48 	mov	x8, #0x62                  	// #98
    f7bc:	d4000001 	svc	#0x0
    f7c0:	17ffffee 	b	f778 <__lll_lock_wait_private+0x10>

000000000000f7c4 <__lll_lock_wait>:
    f7c4:	2a0103e5 	mov	w5, w1
    f7c8:	b9400001 	ldr	w1, [x0]
    f7cc:	aa0003e4 	mov	x4, x0
    f7d0:	7100083f 	cmp	w1, #0x2
    f7d4:	54000200 	b.eq	f814 <__lll_lock_wait+0x50>
    f7d8:	521900a5 	eor	w5, w5, #0x80
    f7dc:	52800046 	mov	w6, #0x2                   	// #2
    f7e0:	93407ca5 	sxtw	x5, w5
    f7e4:	14000007 	b	f800 <__lll_lock_wait+0x3c>
    f7e8:	aa0403e0 	mov	x0, x4
    f7ec:	aa0503e1 	mov	x1, x5
    f7f0:	d2800042 	mov	x2, #0x2                   	// #2
    f7f4:	d2800003 	mov	x3, #0x0                   	// #0
    f7f8:	d2800c48 	mov	x8, #0x62                  	// #98
    f7fc:	d4000001 	svc	#0x0
    f800:	885ffc80 	ldaxr	w0, [x4]
    f804:	88017c86 	stxr	w1, w6, [x4]
    f808:	35ffffc1 	cbnz	w1, f800 <__lll_lock_wait+0x3c>
    f80c:	35fffee0 	cbnz	w0, f7e8 <__lll_lock_wait+0x24>
    f810:	d65f03c0 	ret
    f814:	521900a1 	eor	w1, w5, #0x80
    f818:	d2800042 	mov	x2, #0x2                   	// #2
    f81c:	d2800003 	mov	x3, #0x0                   	// #0
    f820:	d2800c48 	mov	x8, #0x62                  	// #98
    f824:	93407c21 	sxtw	x1, w1
    f828:	d4000001 	svc	#0x0
    f82c:	17ffffeb 	b	f7d8 <__lll_lock_wait+0x14>

000000000000f830 <__lll_timedlock_wait>:
    f830:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    f834:	d2993fe3 	mov	x3, #0xc9ff                	// #51711
    f838:	910003fd 	mov	x29, sp
    f83c:	f9400424 	ldr	x4, [x1,#8]
    f840:	f2a77343 	movk	x3, #0x3b9a, lsl #16
    f844:	a90153f3 	stp	x19, x20, [sp,#16]
    f848:	a9025bf5 	stp	x21, x22, [sp,#32]
    f84c:	a90363f7 	stp	x23, x24, [sp,#48]
    f850:	eb03009f 	cmp	x4, x3
    f854:	aa0003f3 	mov	x19, x0
    f858:	528002c0 	mov	w0, #0x16                  	// #22
    f85c:	540000c9 	b.ls	f874 <__lll_timedlock_wait+0x44>
    f860:	a94153f3 	ldp	x19, x20, [sp,#16]
    f864:	a9425bf5 	ldp	x21, x22, [sp,#32]
    f868:	a94363f7 	ldp	x23, x24, [sp,#48]
    f86c:	a8c67bfd 	ldp	x29, x30, [sp],#96
    f870:	d65f03c0 	ret
    f874:	52190055 	eor	w21, w2, #0x80
    f878:	aa0103f4 	mov	x20, x1
    f87c:	52800057 	mov	w23, #0x2                   	// #2
    f880:	910103b6 	add	x22, x29, #0x40
    f884:	910143b8 	add	x24, x29, #0x50
    f888:	93407eb5 	sxtw	x21, w21
    f88c:	14000009 	b	f8b0 <__lll_timedlock_wait+0x80>
    f890:	f9002fa2 	str	x2, [x29,#88]
    f894:	aa1303e0 	mov	x0, x19
    f898:	aa1503e1 	mov	x1, x21
    f89c:	b7f80405 	tbnz	x5, #63, f91c <__lll_timedlock_wait+0xec>
    f8a0:	d2800042 	mov	x2, #0x2                   	// #2
    f8a4:	aa1803e3 	mov	x3, x24
    f8a8:	d2800c48 	mov	x8, #0x62                  	// #98
    f8ac:	d4000001 	svc	#0x0
    f8b0:	885ffe62 	ldaxr	w2, [x19]
    f8b4:	88007e77 	stxr	w0, w23, [x19]
    f8b8:	35ffffc0 	cbnz	w0, f8b0 <__lll_timedlock_wait+0x80>
    f8bc:	aa1603e0 	mov	x0, x22
    f8c0:	d2800001 	mov	x1, #0x0                   	// #0
    f8c4:	34000382 	cbz	w2, f934 <__lll_timedlock_wait+0x104>
    f8c8:	97ffd5be 	bl	4fc0 <__gettimeofday@plt>
    f8cc:	f94027a6 	ldr	x6, [x29,#72]
    f8d0:	f94023a2 	ldr	x2, [x29,#64]
    f8d4:	f9400285 	ldr	x5, [x20]
    f8d8:	cb0614c4 	sub	x4, x6, x6, lsl #5
    f8dc:	f9400687 	ldr	x7, [x20,#8]
    f8e0:	d37ef484 	lsl	x4, x4, #2
    f8e4:	cb0200a5 	sub	x5, x5, x2
    f8e8:	cb060082 	sub	x2, x4, x6
    f8ec:	f9002ba5 	str	x5, [x29,#80]
    f8f0:	ab020ce2 	adds	x2, x7, x2, lsl #3
    f8f4:	54fffce5 	b.pl	f890 <__lll_timedlock_wait+0x60>
    f8f8:	d2994001 	mov	x1, #0xca00                	// #51712
    f8fc:	d10004a5 	sub	x5, x5, #0x1
    f900:	f2a77341 	movk	x1, #0x3b9a, lsl #16
    f904:	f9002ba5 	str	x5, [x29,#80]
    f908:	8b010042 	add	x2, x2, x1
    f90c:	f9002fa2 	str	x2, [x29,#88]
    f910:	aa1303e0 	mov	x0, x19
    f914:	aa1503e1 	mov	x1, x21
    f918:	b6fffc45 	tbz	x5, #63, f8a0 <__lll_timedlock_wait+0x70>
    f91c:	52800dc0 	mov	w0, #0x6e                  	// #110
    f920:	a94153f3 	ldp	x19, x20, [sp,#16]
    f924:	a9425bf5 	ldp	x21, x22, [sp,#32]
    f928:	a94363f7 	ldp	x23, x24, [sp,#48]
    f92c:	a8c67bfd 	ldp	x29, x30, [sp],#96
    f930:	d65f03c0 	ret
    f934:	2a0203e0 	mov	w0, w2
    f938:	a94153f3 	ldp	x19, x20, [sp,#16]
    f93c:	a9425bf5 	ldp	x21, x22, [sp,#32]
    f940:	a94363f7 	ldp	x23, x24, [sp,#48]
    f944:	a8c67bfd 	ldp	x29, x30, [sp],#96
    f948:	d65f03c0 	ret

000000000000f94c <__lll_timedwait_tid>:
    f94c:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
    f950:	d2993fe2 	mov	x2, #0xc9ff                	// #51711
    f954:	910003fd 	mov	x29, sp
    f958:	f9400423 	ldr	x3, [x1,#8]
    f95c:	f2a77342 	movk	x2, #0x3b9a, lsl #16
    f960:	a9025bf5 	stp	x21, x22, [sp,#32]
    f964:	a90153f3 	stp	x19, x20, [sp,#16]
    f968:	f9001bf7 	str	x23, [sp,#48]
    f96c:	eb02007f 	cmp	x3, x2
    f970:	aa0003f5 	mov	x21, x0
    f974:	528002c0 	mov	w0, #0x16                  	// #22
    f978:	540000c9 	b.ls	f990 <__lll_timedwait_tid+0x44>
    f97c:	a94153f3 	ldp	x19, x20, [sp,#16]
    f980:	a9425bf5 	ldp	x21, x22, [sp,#32]
    f984:	f9401bf7 	ldr	x23, [sp,#48]
    f988:	a8c67bfd 	ldp	x29, x30, [sp],#96
    f98c:	d65f03c0 	ret
    f990:	b94002b3 	ldr	w19, [x21]
    f994:	aa0103f4 	mov	x20, x1
    f998:	910103b7 	add	x23, x29, #0x40
    f99c:	910143b6 	add	x22, x29, #0x50
    f9a0:	35000093 	cbnz	w19, f9b0 <__lll_timedwait_tid+0x64>
    f9a4:	14000029 	b	fa48 <__lll_timedwait_tid+0xfc>
    f9a8:	b94002b3 	ldr	w19, [x21]
    f9ac:	340004f3 	cbz	w19, fa48 <__lll_timedwait_tid+0xfc>
    f9b0:	d2800001 	mov	x1, #0x0                   	// #0
    f9b4:	aa1703e0 	mov	x0, x23
    f9b8:	97ffd582 	bl	4fc0 <__gettimeofday@plt>
    f9bc:	f94027a7 	ldr	x7, [x29,#72]
    f9c0:	f9400286 	ldr	x6, [x20]
    f9c4:	f9400682 	ldr	x2, [x20,#8]
    f9c8:	cb0714e5 	sub	x5, x7, x7, lsl #5
    f9cc:	f94023a1 	ldr	x1, [x29,#64]
    f9d0:	d37ef4a5 	lsl	x5, x5, #2
    f9d4:	cb0700a4 	sub	x4, x5, x7
    f9d8:	cb0100c6 	sub	x6, x6, x1
    f9dc:	f9002ba6 	str	x6, [x29,#80]
    f9e0:	ab040c44 	adds	x4, x2, x4, lsl #3
    f9e4:	54000264 	b.mi	fa30 <__lll_timedwait_tid+0xe4>
    f9e8:	f9002fa4 	str	x4, [x29,#88]
    f9ec:	aa1503e0 	mov	x0, x21
    f9f0:	d2800001 	mov	x1, #0x0                   	// #0
    f9f4:	93407e62 	sxtw	x2, w19
    f9f8:	aa1603e3 	mov	x3, x22
    f9fc:	d2800c48 	mov	x8, #0x62                  	// #98
    fa00:	b7f800c6 	tbnz	x6, #63, fa18 <__lll_timedwait_tid+0xcc>
    fa04:	d4000001 	svc	#0x0
    fa08:	b140041f 	cmn	x0, #0x1, lsl #12
    fa0c:	54fffce9 	b.ls	f9a8 <__lll_timedwait_tid+0x5c>
    fa10:	b101b81f 	cmn	x0, #0x6e
    fa14:	54fffca1 	b.ne	f9a8 <__lll_timedwait_tid+0x5c>
    fa18:	52800dc0 	mov	w0, #0x6e                  	// #110
    fa1c:	f9401bf7 	ldr	x23, [sp,#48]
    fa20:	a94153f3 	ldp	x19, x20, [sp,#16]
    fa24:	a9425bf5 	ldp	x21, x22, [sp,#32]
    fa28:	a8c67bfd 	ldp	x29, x30, [sp],#96
    fa2c:	d65f03c0 	ret
    fa30:	d2994001 	mov	x1, #0xca00                	// #51712
    fa34:	d10004c6 	sub	x6, x6, #0x1
    fa38:	f2a77341 	movk	x1, #0x3b9a, lsl #16
    fa3c:	f9002ba6 	str	x6, [x29,#80]
    fa40:	8b010084 	add	x4, x4, x1
    fa44:	17ffffe9 	b	f9e8 <__lll_timedwait_tid+0x9c>
    fa48:	52800000 	mov	w0, #0x0                   	// #0
    fa4c:	17ffffcc 	b	f97c <__lll_timedwait_tid+0x30>

000000000000fa50 <__lll_robust_lock_wait>:
    fa50:	d53bd042 	mrs	x2, tpidr_el0
    fa54:	aa0003e4 	mov	x4, x0
    fa58:	d11bc042 	sub	x2, x2, #0x6f0
    fa5c:	b9400000 	ldr	w0, [x0]
    fa60:	d10043ff 	sub	sp, sp, #0x10
    fa64:	2a0103e6 	mov	w6, w1
    fa68:	b940d047 	ldr	w7, [x2,#208]
    fa6c:	34000260 	cbz	w0, fab8 <__lll_robust_lock_wait+0x68>
    fa70:	37f00380 	tbnz	w0, #30, fae0 <__lll_robust_lock_wait+0x90>
    fa74:	32010002 	orr	w2, w0, #0x80000000
    fa78:	6b02001f 	cmp	w0, w2
    fa7c:	54000100 	b.eq	fa9c <__lll_robust_lock_wait+0x4c>
    fa80:	b9000fe0 	str	w0, [sp,#12]
    fa84:	885ffc81 	ldaxr	w1, [x4]
    fa88:	6b00003f 	cmp	w1, w0
    fa8c:	54000061 	b.ne	fa98 <__lll_robust_lock_wait+0x48>
    fa90:	88037c82 	stxr	w3, w2, [x4]
    fa94:	35ffff83 	cbnz	w3, fa84 <__lll_robust_lock_wait+0x34>
    fa98:	54000101 	b.ne	fab8 <__lll_robust_lock_wait+0x68>
    fa9c:	521900c5 	eor	w5, w6, #0x80
    faa0:	aa0403e0 	mov	x0, x4
    faa4:	93407c42 	sxtw	x2, w2
    faa8:	d2800003 	mov	x3, #0x0                   	// #0
    faac:	93407ca1 	sxtw	x1, w5
    fab0:	d2800c48 	mov	x8, #0x62                  	// #98
    fab4:	d4000001 	svc	#0x0
    fab8:	b9000fff 	str	wzr, [sp,#12]
    fabc:	320100e0 	orr	w0, w7, #0x80000000
    fac0:	885ffc81 	ldaxr	w1, [x4]
    fac4:	6b1f003f 	cmp	w1, wzr
    fac8:	54000061 	b.ne	fad4 <__lll_robust_lock_wait+0x84>
    facc:	88027c80 	stxr	w2, w0, [x4]
    fad0:	35ffff82 	cbnz	w2, fac0 <__lll_robust_lock_wait+0x70>
    fad4:	540000a1 	b.ne	fae8 <__lll_robust_lock_wait+0x98>
    fad8:	b9400fe0 	ldr	w0, [sp,#12]
    fadc:	35fffca0 	cbnz	w0, fa70 <__lll_robust_lock_wait+0x20>
    fae0:	910043ff 	add	sp, sp, #0x10
    fae4:	d65f03c0 	ret
    fae8:	b9000fe1 	str	w1, [sp,#12]
    faec:	b9400fe0 	ldr	w0, [sp,#12]
    faf0:	35fffc00 	cbnz	w0, fa70 <__lll_robust_lock_wait+0x20>
    faf4:	17fffffb 	b	fae0 <__lll_robust_lock_wait+0x90>

000000000000faf8 <__lll_robust_timedlock_wait>:
    faf8:	aa0103e6 	mov	x6, x1
    fafc:	d2993fe5 	mov	x5, #0xc9ff                	// #51711
    fb00:	f9400421 	ldr	x1, [x1,#8]
    fb04:	f2a77345 	movk	x5, #0x3b9a, lsl #16
    fb08:	aa0003e7 	mov	x7, x0
    fb0c:	d10043ff 	sub	sp, sp, #0x10
    fb10:	eb05003f 	cmp	x1, x5
    fb14:	2a0203e9 	mov	w9, w2
    fb18:	528002c0 	mov	w0, #0x16                  	// #22
    fb1c:	54000069 	b.ls	fb28 <__lll_robust_timedlock_wait+0x30>
    fb20:	910043ff 	add	sp, sp, #0x10
    fb24:	d65f03c0 	ret
    fb28:	d53bd040 	mrs	x0, tpidr_el0
    fb2c:	b94000e4 	ldr	w4, [x7]
    fb30:	d11bc000 	sub	x0, x0, #0x6f0
    fb34:	b940d00a 	ldr	w10, [x0,#208]
    fb38:	340001c4 	cbz	w4, fb70 <__lll_robust_timedlock_wait+0x78>
    fb3c:	f94000c0 	ldr	x0, [x6]
    fb40:	b7f804a0 	tbnz	x0, #63, fbd4 <__lll_robust_timedlock_wait+0xdc>
    fb44:	37f002a4 	tbnz	w4, #30, fb98 <__lll_robust_timedlock_wait+0xa0>
    fb48:	32010082 	orr	w2, w4, #0x80000000
    fb4c:	6b02009f 	cmp	w4, w2
    fb50:	54000280 	b.eq	fba0 <__lll_robust_timedlock_wait+0xa8>
    fb54:	b9000fe4 	str	w4, [sp,#12]
    fb58:	885ffce0 	ldaxr	w0, [x7]
    fb5c:	6b04001f 	cmp	w0, w4
    fb60:	54000061 	b.ne	fb6c <__lll_robust_timedlock_wait+0x74>
    fb64:	88017ce2 	stxr	w1, w2, [x7]
    fb68:	35ffff81 	cbnz	w1, fb58 <__lll_robust_timedlock_wait+0x60>
    fb6c:	540001a0 	b.eq	fba0 <__lll_robust_timedlock_wait+0xa8>
    fb70:	b9000fff 	str	wzr, [sp,#12]
    fb74:	32010140 	orr	w0, w10, #0x80000000
    fb78:	885ffce1 	ldaxr	w1, [x7]
    fb7c:	6b1f003f 	cmp	w1, wzr
    fb80:	54000061 	b.ne	fb8c <__lll_robust_timedlock_wait+0x94>
    fb84:	88027ce0 	stxr	w2, w0, [x7]
    fb88:	35ffff82 	cbnz	w2, fb78 <__lll_robust_timedlock_wait+0x80>
    fb8c:	54000201 	b.ne	fbcc <__lll_robust_timedlock_wait+0xd4>
    fb90:	b9400fe4 	ldr	w4, [sp,#12]
    fb94:	35fffd84 	cbnz	w4, fb44 <__lll_robust_timedlock_wait+0x4c>
    fb98:	2a0403e0 	mov	w0, w4
    fb9c:	17ffffe1 	b	fb20 <__lll_robust_timedlock_wait+0x28>
    fba0:	52803121 	mov	w1, #0x189                 	// #393
    fba4:	aa0703e0 	mov	x0, x7
    fba8:	4a010121 	eor	w1, w9, w1
    fbac:	93407c42 	sxtw	x2, w2
    fbb0:	aa0603e3 	mov	x3, x6
    fbb4:	d2800004 	mov	x4, #0x0                   	// #0
    fbb8:	93407c21 	sxtw	x1, w1
    fbbc:	b2407fe5 	mov	x5, #0xffffffff            	// #4294967295
    fbc0:	d2800c48 	mov	x8, #0x62                  	// #98
    fbc4:	d4000001 	svc	#0x0
    fbc8:	17ffffea 	b	fb70 <__lll_robust_timedlock_wait+0x78>
    fbcc:	b9000fe1 	str	w1, [sp,#12]
    fbd0:	17fffff0 	b	fb90 <__lll_robust_timedlock_wait+0x98>
    fbd4:	52800dc0 	mov	w0, #0x6e                  	// #110
    fbd8:	17ffffd2 	b	fb20 <__lll_robust_timedlock_wait+0x28>

000000000000fbdc <__fork>:
    fbdc:	17ffd531 	b	50a0 <__libc_fork@plt>

000000000000fbe0 <vfork_resolve>:
    fbe0:	90000100 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
    fbe4:	f947d800 	ldr	x0, [x0,#4016]
    fbe8:	d65f03c0 	ret
    fbec:	00000000 	.inst	0x00000000 ; undefined

000000000000fbf0 <__write_nocancel>:
    fbf0:	d2800808 	mov	x8, #0x40                  	// #64
    fbf4:	d4000001 	svc	#0x0
    fbf8:	b13ffc1f 	cmn	x0, #0xfff
    fbfc:	54000042 	b.cs	fc04 <__write_nocancel+0x14>
    fc00:	d65f03c0 	ret
    fc04:	90000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    fc08:	4b0003e2 	neg	w2, w0
    fc0c:	f947c421 	ldr	x1, [x1,#3976]
    fc10:	d53bd043 	mrs	x3, tpidr_el0
    fc14:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    fc18:	b8236822 	str	w2, [x1,x3]
    fc1c:	d65f03c0 	ret

000000000000fc20 <__write>:
    fc20:	b0000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
    fc24:	b9432a10 	ldr	w16, [x16,#808]
    fc28:	34fffe50 	cbz	w16, fbf0 <__write_nocancel>
    fc2c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
    fc30:	a9010be1 	stp	x1, x2, [sp,#16]
    fc34:	97fffe79 	bl	f618 <__pthread_enable_asynccancel>
    fc38:	aa0003f0 	mov	x16, x0
    fc3c:	f94007e0 	ldr	x0, [sp,#8]
    fc40:	a9410be1 	ldp	x1, x2, [sp,#16]
    fc44:	d2800808 	mov	x8, #0x40                  	// #64
    fc48:	d4000001 	svc	#0x0
    fc4c:	f90007e0 	str	x0, [sp,#8]
    fc50:	aa1003e0 	mov	x0, x16
    fc54:	97fffea1 	bl	f6d8 <__pthread_disable_asynccancel>
    fc58:	a8c403fe 	ldp	x30, x0, [sp],#64
    fc5c:	17ffffe7 	b	fbf8 <__write_nocancel+0x8>

000000000000fc60 <__read_nocancel>:
    fc60:	d28007e8 	mov	x8, #0x3f                  	// #63
    fc64:	d4000001 	svc	#0x0
    fc68:	b13ffc1f 	cmn	x0, #0xfff
    fc6c:	54000042 	b.cs	fc74 <__read_nocancel+0x14>
    fc70:	d65f03c0 	ret
    fc74:	90000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    fc78:	4b0003e2 	neg	w2, w0
    fc7c:	f947c421 	ldr	x1, [x1,#3976]
    fc80:	d53bd043 	mrs	x3, tpidr_el0
    fc84:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    fc88:	b8236822 	str	w2, [x1,x3]
    fc8c:	d65f03c0 	ret

000000000000fc90 <__read>:
    fc90:	b0000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
    fc94:	b9432a10 	ldr	w16, [x16,#808]
    fc98:	34fffe50 	cbz	w16, fc60 <__read_nocancel>
    fc9c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
    fca0:	a9010be1 	stp	x1, x2, [sp,#16]
    fca4:	97fffe5d 	bl	f618 <__pthread_enable_asynccancel>
    fca8:	aa0003f0 	mov	x16, x0
    fcac:	f94007e0 	ldr	x0, [sp,#8]
    fcb0:	a9410be1 	ldp	x1, x2, [sp,#16]
    fcb4:	d28007e8 	mov	x8, #0x3f                  	// #63
    fcb8:	d4000001 	svc	#0x0
    fcbc:	f90007e0 	str	x0, [sp,#8]
    fcc0:	aa1003e0 	mov	x0, x16
    fcc4:	97fffe85 	bl	f6d8 <__pthread_disable_asynccancel>
    fcc8:	a8c403fe 	ldp	x30, x0, [sp],#64
    fccc:	17ffffe7 	b	fc68 <__read_nocancel+0x8>

000000000000fcd0 <__close_nocancel>:
    fcd0:	d2800728 	mov	x8, #0x39                  	// #57
    fcd4:	d4000001 	svc	#0x0
    fcd8:	b13ffc1f 	cmn	x0, #0xfff
    fcdc:	54000042 	b.cs	fce4 <__close_nocancel+0x14>
    fce0:	d65f03c0 	ret
    fce4:	90000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    fce8:	4b0003e2 	neg	w2, w0
    fcec:	f947c421 	ldr	x1, [x1,#3976]
    fcf0:	d53bd043 	mrs	x3, tpidr_el0
    fcf4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    fcf8:	b8236822 	str	w2, [x1,x3]
    fcfc:	d65f03c0 	ret

000000000000fd00 <__close>:
    fd00:	b0000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
    fd04:	b9432a10 	ldr	w16, [x16,#808]
    fd08:	34fffe50 	cbz	w16, fcd0 <__close_nocancel>
    fd0c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
    fd10:	97fffe42 	bl	f618 <__pthread_enable_asynccancel>
    fd14:	aa0003f0 	mov	x16, x0
    fd18:	f94007e0 	ldr	x0, [sp,#8]
    fd1c:	d2800728 	mov	x8, #0x39                  	// #57
    fd20:	d4000001 	svc	#0x0
    fd24:	f90007e0 	str	x0, [sp,#8]
    fd28:	aa1003e0 	mov	x0, x16
    fd2c:	97fffe6b 	bl	f6d8 <__pthread_disable_asynccancel>
    fd30:	a8c403fe 	ldp	x30, x0, [sp],#64
    fd34:	17ffffe9 	b	fcd8 <__close_nocancel+0x8>
    fd38:	d503201f 	nop
    fd3c:	d503201f 	nop

000000000000fd40 <__fcntl_nocancel>:
    fd40:	d10383ff 	sub	sp, sp, #0xe0
    fd44:	7100243f 	cmp	w1, #0x9
    fd48:	93407c00 	sxtw	x0, w0
    fd4c:	3d800fe0 	str	q0, [sp,#48]
    fd50:	f9005fe3 	str	x3, [sp,#184]
    fd54:	910383e3 	add	x3, sp, #0xe0
    fd58:	f9000be3 	str	x3, [sp,#16]
    fd5c:	f9000fe3 	str	x3, [sp,#24]
    fd60:	9102c3e3 	add	x3, sp, #0xb0
    fd64:	f90013e3 	str	x3, [sp,#32]
    fd68:	128005e3 	mov	w3, #0xffffffd0            	// #-48
    fd6c:	f9005be2 	str	x2, [sp,#176]
    fd70:	b9002be3 	str	w3, [sp,#40]
    fd74:	12800fe3 	mov	w3, #0xffffff80            	// #-128
    fd78:	f90063e4 	str	x4, [sp,#192]
    fd7c:	f90067e5 	str	x5, [sp,#200]
    fd80:	f9006be6 	str	x6, [sp,#208]
    fd84:	f9006fe7 	str	x7, [sp,#216]
    fd88:	b9002fe3 	str	w3, [sp,#44]
    fd8c:	3d8013e1 	str	q1, [sp,#64]
    fd90:	3d8017e2 	str	q2, [sp,#80]
    fd94:	3d801be3 	str	q3, [sp,#96]
    fd98:	3d801fe4 	str	q4, [sp,#112]
    fd9c:	3d8023e5 	str	q5, [sp,#128]
    fda0:	3d8027e6 	str	q6, [sp,#144]
    fda4:	3d802be7 	str	q7, [sp,#160]
    fda8:	54000100 	b.eq	fdc8 <__fcntl_nocancel+0x88>
    fdac:	93407c21 	sxtw	x1, w1
    fdb0:	d2800328 	mov	x8, #0x19                  	// #25
    fdb4:	d4000001 	svc	#0x0
    fdb8:	b140041f 	cmn	x0, #0x1, lsl #12
    fdbc:	540001e8 	b.hi	fdf8 <__fcntl_nocancel+0xb8>
    fdc0:	910383ff 	add	sp, sp, #0xe0
    fdc4:	d65f03c0 	ret
    fdc8:	d2800201 	mov	x1, #0x10                  	// #16
    fdcc:	910023e2 	add	x2, sp, #0x8
    fdd0:	d2800328 	mov	x8, #0x19                  	// #25
    fdd4:	d4000001 	svc	#0x0
    fdd8:	3140041f 	cmn	w0, #0x1, lsl #12
    fddc:	540001c8 	b.hi	fe14 <__fcntl_nocancel+0xd4>
    fde0:	b9400be0 	ldr	w0, [sp,#8]
    fde4:	7100081f 	cmp	w0, #0x2
    fde8:	b9400fe0 	ldr	w0, [sp,#12]
    fdec:	54fffea1 	b.ne	fdc0 <__fcntl_nocancel+0x80>
    fdf0:	4b0003e0 	neg	w0, w0
    fdf4:	17fffff3 	b	fdc0 <__fcntl_nocancel+0x80>
    fdf8:	d53bd041 	mrs	x1, tpidr_el0
    fdfc:	90000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    fe00:	f947c442 	ldr	x2, [x2,#3976]
    fe04:	4b0003e3 	neg	w3, w0
    fe08:	12800000 	mov	w0, #0xffffffff            	// #-1
    fe0c:	b8226823 	str	w3, [x1,x2]
    fe10:	17ffffec 	b	fdc0 <__fcntl_nocancel+0x80>
    fe14:	d53bd042 	mrs	x2, tpidr_el0
    fe18:	90000103 	adrp	x3, 2f000 <__FRAME_END__+0x18e30>
    fe1c:	f947c463 	ldr	x3, [x3,#3976]
    fe20:	4b0003e1 	neg	w1, w0
    fe24:	12800000 	mov	w0, #0xffffffff            	// #-1
    fe28:	b8236841 	str	w1, [x2,x3]
    fe2c:	17ffffe5 	b	fdc0 <__fcntl_nocancel+0x80>

000000000000fe30 <__fcntl>:
    fe30:	a9af7bfd 	stp	x29, x30, [sp,#-272]!
    fe34:	71001c3f 	cmp	w1, #0x7
    fe38:	910003fd 	mov	x29, sp
    fe3c:	f9000bf3 	str	x19, [sp,#16]
    fe40:	3d801ba0 	str	q0, [x29,#96]
    fe44:	f90077a3 	str	x3, [x29,#232]
    fe48:	910443a3 	add	x3, x29, #0x110
    fe4c:	f90023a3 	str	x3, [x29,#64]
    fe50:	f90027a3 	str	x3, [x29,#72]
    fe54:	910383a3 	add	x3, x29, #0xe0
    fe58:	f9002ba3 	str	x3, [x29,#80]
    fe5c:	128005e3 	mov	w3, #0xffffffd0            	// #-48
    fe60:	f90073a2 	str	x2, [x29,#224]
    fe64:	b9005ba3 	str	w3, [x29,#88]
    fe68:	12800fe3 	mov	w3, #0xffffff80            	// #-128
    fe6c:	f9007ba4 	str	x4, [x29,#240]
    fe70:	f9007fa5 	str	x5, [x29,#248]
    fe74:	f90083a6 	str	x6, [x29,#256]
    fe78:	f90087a7 	str	x7, [x29,#264]
    fe7c:	b9005fa3 	str	w3, [x29,#92]
    fe80:	3d801fa1 	str	q1, [x29,#112]
    fe84:	3d8023a2 	str	q2, [x29,#128]
    fe88:	3d8027a3 	str	q3, [x29,#144]
    fe8c:	3d802ba4 	str	q4, [x29,#160]
    fe90:	3d802fa5 	str	q5, [x29,#176]
    fe94:	3d8033a6 	str	q6, [x29,#192]
    fe98:	3d8037a7 	str	q7, [x29,#208]
    fe9c:	54000081 	b.ne	feac <__fcntl+0x7c>
    fea0:	b0000123 	adrp	x3, 34000 <__GI___pthread_keys+0x3d78>
    fea4:	b9432863 	ldr	w3, [x3,#808]
    fea8:	350004c3 	cbnz	w3, ff40 <__fcntl+0x110>
    feac:	7100243f 	cmp	w1, #0x9
    feb0:	93407c00 	sxtw	x0, w0
    feb4:	54000160 	b.eq	fee0 <__fcntl+0xb0>
    feb8:	93407c21 	sxtw	x1, w1
    febc:	d2800328 	mov	x8, #0x19                  	// #25
    fec0:	d4000001 	svc	#0x0
    fec4:	b140041f 	cmn	x0, #0x1, lsl #12
    fec8:	540002e8 	b.hi	ff24 <__fcntl+0xf4>
    fecc:	2a0003f3 	mov	w19, w0
    fed0:	2a1303e0 	mov	w0, w19
    fed4:	f9400bf3 	ldr	x19, [sp,#16]
    fed8:	a8d17bfd 	ldp	x29, x30, [sp],#272
    fedc:	d65f03c0 	ret
    fee0:	d2800201 	mov	x1, #0x10                  	// #16
    fee4:	9100e3a2 	add	x2, x29, #0x38
    fee8:	d2800328 	mov	x8, #0x19                  	// #25
    feec:	d4000001 	svc	#0x0
    fef0:	3140041f 	cmn	w0, #0x1, lsl #12
    fef4:	54000188 	b.hi	ff24 <__fcntl+0xf4>
    fef8:	b9403ba0 	ldr	w0, [x29,#56]
    fefc:	7100081f 	cmp	w0, #0x2
    ff00:	540000c0 	b.eq	ff18 <__fcntl+0xe8>
    ff04:	b9403fb3 	ldr	w19, [x29,#60]
    ff08:	2a1303e0 	mov	w0, w19
    ff0c:	f9400bf3 	ldr	x19, [sp,#16]
    ff10:	a8d17bfd 	ldp	x29, x30, [sp],#272
    ff14:	d65f03c0 	ret
    ff18:	b9403fa0 	ldr	w0, [x29,#60]
    ff1c:	4b0003f3 	neg	w19, w0
    ff20:	17ffffec 	b	fed0 <__fcntl+0xa0>
    ff24:	d53bd041 	mrs	x1, tpidr_el0
    ff28:	90000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    ff2c:	f947c442 	ldr	x2, [x2,#3976]
    ff30:	4b0003e0 	neg	w0, w0
    ff34:	12800013 	mov	w19, #0xffffffff            	// #-1
    ff38:	b8226820 	str	w0, [x1,x2]
    ff3c:	17ffffe5 	b	fed0 <__fcntl+0xa0>
    ff40:	2a0003f3 	mov	w19, w0
    ff44:	f90017a2 	str	x2, [x29,#40]
    ff48:	97fffdb4 	bl	f618 <__pthread_enable_asynccancel>
    ff4c:	2a0003e3 	mov	w3, w0
    ff50:	d28000e1 	mov	x1, #0x7                   	// #7
    ff54:	93407e60 	sxtw	x0, w19
    ff58:	f94017a2 	ldr	x2, [x29,#40]
    ff5c:	d2800328 	mov	x8, #0x19                  	// #25
    ff60:	d4000001 	svc	#0x0
    ff64:	b140041f 	cmn	x0, #0x1, lsl #12
    ff68:	540000a8 	b.hi	ff7c <__fcntl+0x14c>
    ff6c:	2a0003f3 	mov	w19, w0
    ff70:	2a0303e0 	mov	w0, w3
    ff74:	97fffdd9 	bl	f6d8 <__pthread_disable_asynccancel>
    ff78:	17ffffd6 	b	fed0 <__fcntl+0xa0>
    ff7c:	d53bd041 	mrs	x1, tpidr_el0
    ff80:	90000102 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
    ff84:	f947c442 	ldr	x2, [x2,#3976]
    ff88:	4b0003e0 	neg	w0, w0
    ff8c:	12800013 	mov	w19, #0xffffffff            	// #-1
    ff90:	b8226820 	str	w0, [x1,x2]
    ff94:	17fffff7 	b	ff70 <__fcntl+0x140>
	...

000000000000ffa0 <__accept_nocancel>:
    ffa0:	d2801948 	mov	x8, #0xca                  	// #202
    ffa4:	d4000001 	svc	#0x0
    ffa8:	b13ffc1f 	cmn	x0, #0xfff
    ffac:	54000042 	b.cs	ffb4 <__accept_nocancel+0x14>
    ffb0:	d65f03c0 	ret
    ffb4:	90000101 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
    ffb8:	4b0003e2 	neg	w2, w0
    ffbc:	f947c421 	ldr	x1, [x1,#3976]
    ffc0:	d53bd043 	mrs	x3, tpidr_el0
    ffc4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
    ffc8:	b8236822 	str	w2, [x1,x3]
    ffcc:	d65f03c0 	ret

000000000000ffd0 <accept>:
    ffd0:	b0000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
    ffd4:	b9432a10 	ldr	w16, [x16,#808]
    ffd8:	34fffe50 	cbz	w16, ffa0 <__accept_nocancel>
    ffdc:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
    ffe0:	a9010be1 	stp	x1, x2, [sp,#16]
    ffe4:	97fffd8d 	bl	f618 <__pthread_enable_asynccancel>
    ffe8:	aa0003f0 	mov	x16, x0
    ffec:	f94007e0 	ldr	x0, [sp,#8]
    fff0:	a9410be1 	ldp	x1, x2, [sp,#16]
    fff4:	d2801948 	mov	x8, #0xca                  	// #202
    fff8:	d4000001 	svc	#0x0
    fffc:	f90007e0 	str	x0, [sp,#8]
   10000:	aa1003e0 	mov	x0, x16
   10004:	97fffdb5 	bl	f6d8 <__pthread_disable_asynccancel>
   10008:	a8c403fe 	ldp	x30, x0, [sp],#64
   1000c:	17ffffe7 	b	ffa8 <__accept_nocancel+0x8>

0000000000010010 <__connect_nocancel>:
   10010:	d2801968 	mov	x8, #0xcb                  	// #203
   10014:	d4000001 	svc	#0x0
   10018:	b13ffc1f 	cmn	x0, #0xfff
   1001c:	54000042 	b.cs	10024 <__connect_nocancel+0x14>
   10020:	d65f03c0 	ret
   10024:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10028:	4b0003e2 	neg	w2, w0
   1002c:	f947c421 	ldr	x1, [x1,#3976]
   10030:	d53bd043 	mrs	x3, tpidr_el0
   10034:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10038:	b8236822 	str	w2, [x1,x3]
   1003c:	d65f03c0 	ret

0000000000010040 <__connect>:
   10040:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10044:	b9432a10 	ldr	w16, [x16,#808]
   10048:	34fffe50 	cbz	w16, 10010 <__connect_nocancel>
   1004c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10050:	a9010be1 	stp	x1, x2, [sp,#16]
   10054:	97fffd71 	bl	f618 <__pthread_enable_asynccancel>
   10058:	aa0003f0 	mov	x16, x0
   1005c:	f94007e0 	ldr	x0, [sp,#8]
   10060:	a9410be1 	ldp	x1, x2, [sp,#16]
   10064:	d2801968 	mov	x8, #0xcb                  	// #203
   10068:	d4000001 	svc	#0x0
   1006c:	f90007e0 	str	x0, [sp,#8]
   10070:	aa1003e0 	mov	x0, x16
   10074:	97fffd99 	bl	f6d8 <__pthread_disable_asynccancel>
   10078:	a8c403fe 	ldp	x30, x0, [sp],#64
   1007c:	17ffffe7 	b	10018 <__connect_nocancel+0x8>

0000000000010080 <recv>:
   10080:	90000124 	adrp	x4, 34000 <__GI___pthread_keys+0x3d78>
   10084:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   10088:	910003fd 	mov	x29, sp
   1008c:	b9432884 	ldr	w4, [x4,#808]
   10090:	a90153f3 	stp	x19, x20, [sp,#16]
   10094:	350001c4 	cbnz	w4, 100cc <recv+0x4c>
   10098:	d2800004 	mov	x4, #0x0                   	// #0
   1009c:	93407c00 	sxtw	x0, w0
   100a0:	93407c63 	sxtw	x3, w3
   100a4:	aa0403e5 	mov	x5, x4
   100a8:	d28019e8 	mov	x8, #0xcf                  	// #207
   100ac:	d4000001 	svc	#0x0
   100b0:	b140041f 	cmn	x0, #0x1, lsl #12
   100b4:	540003a8 	b.hi	10128 <recv+0xa8>
   100b8:	aa0003f3 	mov	x19, x0
   100bc:	aa1303e0 	mov	x0, x19
   100c0:	a94153f3 	ldp	x19, x20, [sp,#16]
   100c4:	a8c37bfd 	ldp	x29, x30, [sp],#48
   100c8:	d65f03c0 	ret
   100cc:	2a0303f3 	mov	w19, w3
   100d0:	f90013a2 	str	x2, [x29,#32]
   100d4:	f90017a1 	str	x1, [x29,#40]
   100d8:	2a0003f4 	mov	w20, w0
   100dc:	97fffd4f 	bl	f618 <__pthread_enable_asynccancel>
   100e0:	2a0003e6 	mov	w6, w0
   100e4:	d2800004 	mov	x4, #0x0                   	// #0
   100e8:	93407e80 	sxtw	x0, w20
   100ec:	f94017a1 	ldr	x1, [x29,#40]
   100f0:	93407e63 	sxtw	x3, w19
   100f4:	f94013a2 	ldr	x2, [x29,#32]
   100f8:	aa0403e5 	mov	x5, x4
   100fc:	d28019e8 	mov	x8, #0xcf                  	// #207
   10100:	d4000001 	svc	#0x0
   10104:	b140041f 	cmn	x0, #0x1, lsl #12
   10108:	540001e8 	b.hi	10144 <recv+0xc4>
   1010c:	aa0003f3 	mov	x19, x0
   10110:	2a0603e0 	mov	w0, w6
   10114:	97fffd71 	bl	f6d8 <__pthread_disable_asynccancel>
   10118:	aa1303e0 	mov	x0, x19
   1011c:	a94153f3 	ldp	x19, x20, [sp,#16]
   10120:	a8c37bfd 	ldp	x29, x30, [sp],#48
   10124:	d65f03c0 	ret
   10128:	d53bd041 	mrs	x1, tpidr_el0
   1012c:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10130:	f947c442 	ldr	x2, [x2,#3976]
   10134:	4b0003e0 	neg	w0, w0
   10138:	92800013 	mov	x19, #0xffffffffffffffff    	// #-1
   1013c:	b8226820 	str	w0, [x1,x2]
   10140:	17ffffdf 	b	100bc <recv+0x3c>
   10144:	d53bd041 	mrs	x1, tpidr_el0
   10148:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   1014c:	f947c442 	ldr	x2, [x2,#3976]
   10150:	4b0003e0 	neg	w0, w0
   10154:	92800013 	mov	x19, #0xffffffffffffffff    	// #-1
   10158:	b8226820 	str	w0, [x1,x2]
   1015c:	17ffffed 	b	10110 <recv+0x90>

0000000000010160 <__recvfrom_nocancel>:
   10160:	d28019e8 	mov	x8, #0xcf                  	// #207
   10164:	d4000001 	svc	#0x0
   10168:	b13ffc1f 	cmn	x0, #0xfff
   1016c:	54000042 	b.cs	10174 <__recvfrom_nocancel+0x14>
   10170:	d65f03c0 	ret
   10174:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10178:	4b0003e2 	neg	w2, w0
   1017c:	f947c421 	ldr	x1, [x1,#3976]
   10180:	d53bd043 	mrs	x3, tpidr_el0
   10184:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10188:	b8236822 	str	w2, [x1,x3]
   1018c:	d65f03c0 	ret

0000000000010190 <recvfrom>:
   10190:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10194:	b9432a10 	ldr	w16, [x16,#808]
   10198:	34fffe50 	cbz	w16, 10160 <__recvfrom_nocancel>
   1019c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   101a0:	a9010be1 	stp	x1, x2, [sp,#16]
   101a4:	a90213e3 	stp	x3, x4, [sp,#32]
   101a8:	f9001be5 	str	x5, [sp,#48]
   101ac:	97fffd1b 	bl	f618 <__pthread_enable_asynccancel>
   101b0:	aa0003f0 	mov	x16, x0
   101b4:	a94087e0 	ldp	x0, x1, [sp,#8]
   101b8:	a9418fe2 	ldp	x2, x3, [sp,#24]
   101bc:	a94297e4 	ldp	x4, x5, [sp,#40]
   101c0:	d28019e8 	mov	x8, #0xcf                  	// #207
   101c4:	d4000001 	svc	#0x0
   101c8:	f90007e0 	str	x0, [sp,#8]
   101cc:	aa1003e0 	mov	x0, x16
   101d0:	97fffd42 	bl	f6d8 <__pthread_disable_asynccancel>
   101d4:	a8c403fe 	ldp	x30, x0, [sp],#64
   101d8:	17ffffe4 	b	10168 <__recvfrom_nocancel+0x8>
   101dc:	d503201f 	nop

00000000000101e0 <__recvmsg_nocancel>:
   101e0:	d2801a88 	mov	x8, #0xd4                  	// #212
   101e4:	d4000001 	svc	#0x0
   101e8:	b13ffc1f 	cmn	x0, #0xfff
   101ec:	54000042 	b.cs	101f4 <__recvmsg_nocancel+0x14>
   101f0:	d65f03c0 	ret
   101f4:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   101f8:	4b0003e2 	neg	w2, w0
   101fc:	f947c421 	ldr	x1, [x1,#3976]
   10200:	d53bd043 	mrs	x3, tpidr_el0
   10204:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10208:	b8236822 	str	w2, [x1,x3]
   1020c:	d65f03c0 	ret

0000000000010210 <recvmsg>:
   10210:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10214:	b9432a10 	ldr	w16, [x16,#808]
   10218:	34fffe50 	cbz	w16, 101e0 <__recvmsg_nocancel>
   1021c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10220:	a9010be1 	stp	x1, x2, [sp,#16]
   10224:	97fffcfd 	bl	f618 <__pthread_enable_asynccancel>
   10228:	aa0003f0 	mov	x16, x0
   1022c:	f94007e0 	ldr	x0, [sp,#8]
   10230:	a9410be1 	ldp	x1, x2, [sp,#16]
   10234:	d2801a88 	mov	x8, #0xd4                  	// #212
   10238:	d4000001 	svc	#0x0
   1023c:	f90007e0 	str	x0, [sp,#8]
   10240:	aa1003e0 	mov	x0, x16
   10244:	97fffd25 	bl	f6d8 <__pthread_disable_asynccancel>
   10248:	a8c403fe 	ldp	x30, x0, [sp],#64
   1024c:	17ffffe7 	b	101e8 <__recvmsg_nocancel+0x8>

0000000000010250 <__send>:
   10250:	90000124 	adrp	x4, 34000 <__GI___pthread_keys+0x3d78>
   10254:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   10258:	910003fd 	mov	x29, sp
   1025c:	b9432884 	ldr	w4, [x4,#808]
   10260:	a90153f3 	stp	x19, x20, [sp,#16]
   10264:	350001c4 	cbnz	w4, 1029c <__send+0x4c>
   10268:	d2800004 	mov	x4, #0x0                   	// #0
   1026c:	93407c00 	sxtw	x0, w0
   10270:	93407c63 	sxtw	x3, w3
   10274:	aa0403e5 	mov	x5, x4
   10278:	d28019c8 	mov	x8, #0xce                  	// #206
   1027c:	d4000001 	svc	#0x0
   10280:	b140041f 	cmn	x0, #0x1, lsl #12
   10284:	540003a8 	b.hi	102f8 <__send+0xa8>
   10288:	aa0003f3 	mov	x19, x0
   1028c:	aa1303e0 	mov	x0, x19
   10290:	a94153f3 	ldp	x19, x20, [sp,#16]
   10294:	a8c37bfd 	ldp	x29, x30, [sp],#48
   10298:	d65f03c0 	ret
   1029c:	2a0303f3 	mov	w19, w3
   102a0:	f90013a2 	str	x2, [x29,#32]
   102a4:	f90017a1 	str	x1, [x29,#40]
   102a8:	2a0003f4 	mov	w20, w0
   102ac:	97fffcdb 	bl	f618 <__pthread_enable_asynccancel>
   102b0:	2a0003e6 	mov	w6, w0
   102b4:	d2800004 	mov	x4, #0x0                   	// #0
   102b8:	93407e80 	sxtw	x0, w20
   102bc:	f94017a1 	ldr	x1, [x29,#40]
   102c0:	93407e63 	sxtw	x3, w19
   102c4:	f94013a2 	ldr	x2, [x29,#32]
   102c8:	aa0403e5 	mov	x5, x4
   102cc:	d28019c8 	mov	x8, #0xce                  	// #206
   102d0:	d4000001 	svc	#0x0
   102d4:	b140041f 	cmn	x0, #0x1, lsl #12
   102d8:	540001e8 	b.hi	10314 <__send+0xc4>
   102dc:	aa0003f3 	mov	x19, x0
   102e0:	2a0603e0 	mov	w0, w6
   102e4:	97fffcfd 	bl	f6d8 <__pthread_disable_asynccancel>
   102e8:	aa1303e0 	mov	x0, x19
   102ec:	a94153f3 	ldp	x19, x20, [sp,#16]
   102f0:	a8c37bfd 	ldp	x29, x30, [sp],#48
   102f4:	d65f03c0 	ret
   102f8:	d53bd041 	mrs	x1, tpidr_el0
   102fc:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10300:	f947c442 	ldr	x2, [x2,#3976]
   10304:	4b0003e0 	neg	w0, w0
   10308:	92800013 	mov	x19, #0xffffffffffffffff    	// #-1
   1030c:	b8226820 	str	w0, [x1,x2]
   10310:	17ffffdf 	b	1028c <__send+0x3c>
   10314:	d53bd041 	mrs	x1, tpidr_el0
   10318:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   1031c:	f947c442 	ldr	x2, [x2,#3976]
   10320:	4b0003e0 	neg	w0, w0
   10324:	92800013 	mov	x19, #0xffffffffffffffff    	// #-1
   10328:	b8226820 	str	w0, [x1,x2]
   1032c:	17ffffed 	b	102e0 <__send+0x90>

0000000000010330 <__sendmsg_nocancel>:
   10330:	d2801a68 	mov	x8, #0xd3                  	// #211
   10334:	d4000001 	svc	#0x0
   10338:	b13ffc1f 	cmn	x0, #0xfff
   1033c:	54000042 	b.cs	10344 <__sendmsg_nocancel+0x14>
   10340:	d65f03c0 	ret
   10344:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10348:	4b0003e2 	neg	w2, w0
   1034c:	f947c421 	ldr	x1, [x1,#3976]
   10350:	d53bd043 	mrs	x3, tpidr_el0
   10354:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10358:	b8236822 	str	w2, [x1,x3]
   1035c:	d65f03c0 	ret

0000000000010360 <sendmsg>:
   10360:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10364:	b9432a10 	ldr	w16, [x16,#808]
   10368:	34fffe50 	cbz	w16, 10330 <__sendmsg_nocancel>
   1036c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10370:	a9010be1 	stp	x1, x2, [sp,#16]
   10374:	97fffca9 	bl	f618 <__pthread_enable_asynccancel>
   10378:	aa0003f0 	mov	x16, x0
   1037c:	f94007e0 	ldr	x0, [sp,#8]
   10380:	a9410be1 	ldp	x1, x2, [sp,#16]
   10384:	d2801a68 	mov	x8, #0xd3                  	// #211
   10388:	d4000001 	svc	#0x0
   1038c:	f90007e0 	str	x0, [sp,#8]
   10390:	aa1003e0 	mov	x0, x16
   10394:	97fffcd1 	bl	f6d8 <__pthread_disable_asynccancel>
   10398:	a8c403fe 	ldp	x30, x0, [sp],#64
   1039c:	17ffffe7 	b	10338 <__sendmsg_nocancel+0x8>

00000000000103a0 <__sendto_nocancel>:
   103a0:	d28019c8 	mov	x8, #0xce                  	// #206
   103a4:	d4000001 	svc	#0x0
   103a8:	b13ffc1f 	cmn	x0, #0xfff
   103ac:	54000042 	b.cs	103b4 <__sendto_nocancel+0x14>
   103b0:	d65f03c0 	ret
   103b4:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   103b8:	4b0003e2 	neg	w2, w0
   103bc:	f947c421 	ldr	x1, [x1,#3976]
   103c0:	d53bd043 	mrs	x3, tpidr_el0
   103c4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   103c8:	b8236822 	str	w2, [x1,x3]
   103cc:	d65f03c0 	ret

00000000000103d0 <sendto>:
   103d0:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   103d4:	b9432a10 	ldr	w16, [x16,#808]
   103d8:	34fffe50 	cbz	w16, 103a0 <__sendto_nocancel>
   103dc:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   103e0:	a9010be1 	stp	x1, x2, [sp,#16]
   103e4:	a90213e3 	stp	x3, x4, [sp,#32]
   103e8:	f9001be5 	str	x5, [sp,#48]
   103ec:	97fffc8b 	bl	f618 <__pthread_enable_asynccancel>
   103f0:	aa0003f0 	mov	x16, x0
   103f4:	a94087e0 	ldp	x0, x1, [sp,#8]
   103f8:	a9418fe2 	ldp	x2, x3, [sp,#24]
   103fc:	a94297e4 	ldp	x4, x5, [sp,#40]
   10400:	d28019c8 	mov	x8, #0xce                  	// #206
   10404:	d4000001 	svc	#0x0
   10408:	f90007e0 	str	x0, [sp,#8]
   1040c:	aa1003e0 	mov	x0, x16
   10410:	97fffcb2 	bl	f6d8 <__pthread_disable_asynccancel>
   10414:	a8c403fe 	ldp	x30, x0, [sp],#64
   10418:	17ffffe4 	b	103a8 <__sendto_nocancel+0x8>
   1041c:	d503201f 	nop

0000000000010420 <__fsync_nocancel>:
   10420:	d2800a48 	mov	x8, #0x52                  	// #82
   10424:	d4000001 	svc	#0x0
   10428:	b13ffc1f 	cmn	x0, #0xfff
   1042c:	54000042 	b.cs	10434 <__fsync_nocancel+0x14>
   10430:	d65f03c0 	ret
   10434:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10438:	4b0003e2 	neg	w2, w0
   1043c:	f947c421 	ldr	x1, [x1,#3976]
   10440:	d53bd043 	mrs	x3, tpidr_el0
   10444:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10448:	b8236822 	str	w2, [x1,x3]
   1044c:	d65f03c0 	ret

0000000000010450 <fsync>:
   10450:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10454:	b9432a10 	ldr	w16, [x16,#808]
   10458:	34fffe50 	cbz	w16, 10420 <__fsync_nocancel>
   1045c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10460:	97fffc6e 	bl	f618 <__pthread_enable_asynccancel>
   10464:	aa0003f0 	mov	x16, x0
   10468:	f94007e0 	ldr	x0, [sp,#8]
   1046c:	d2800a48 	mov	x8, #0x52                  	// #82
   10470:	d4000001 	svc	#0x0
   10474:	f90007e0 	str	x0, [sp,#8]
   10478:	aa1003e0 	mov	x0, x16
   1047c:	97fffc97 	bl	f6d8 <__pthread_disable_asynccancel>
   10480:	a8c403fe 	ldp	x30, x0, [sp],#64
   10484:	17ffffe9 	b	10428 <__fsync_nocancel+0x8>
   10488:	d503201f 	nop
   1048c:	d503201f 	nop

0000000000010490 <__lseek_nocancel>:
   10490:	d28007c8 	mov	x8, #0x3e                  	// #62
   10494:	d4000001 	svc	#0x0
   10498:	b13ffc1f 	cmn	x0, #0xfff
   1049c:	54000042 	b.cs	104a4 <__lseek_nocancel+0x14>
   104a0:	d65f03c0 	ret
   104a4:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   104a8:	4b0003e2 	neg	w2, w0
   104ac:	f947c421 	ldr	x1, [x1,#3976]
   104b0:	d53bd043 	mrs	x3, tpidr_el0
   104b4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   104b8:	b8236822 	str	w2, [x1,x3]
   104bc:	d65f03c0 	ret

00000000000104c0 <__lseek>:
   104c0:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   104c4:	b9432a10 	ldr	w16, [x16,#808]
   104c8:	34fffe50 	cbz	w16, 10490 <__lseek_nocancel>
   104cc:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   104d0:	a9010be1 	stp	x1, x2, [sp,#16]
   104d4:	97fffc51 	bl	f618 <__pthread_enable_asynccancel>
   104d8:	aa0003f0 	mov	x16, x0
   104dc:	f94007e0 	ldr	x0, [sp,#8]
   104e0:	a9410be1 	ldp	x1, x2, [sp,#16]
   104e4:	d28007c8 	mov	x8, #0x3e                  	// #62
   104e8:	d4000001 	svc	#0x0
   104ec:	f90007e0 	str	x0, [sp,#8]
   104f0:	aa1003e0 	mov	x0, x16
   104f4:	97fffc79 	bl	f6d8 <__pthread_disable_asynccancel>
   104f8:	a8c403fe 	ldp	x30, x0, [sp],#64
   104fc:	17ffffe7 	b	10498 <__lseek_nocancel+0x8>

0000000000010500 <__msync_nocancel>:
   10500:	d2801c68 	mov	x8, #0xe3                  	// #227
   10504:	d4000001 	svc	#0x0
   10508:	b13ffc1f 	cmn	x0, #0xfff
   1050c:	54000042 	b.cs	10514 <__msync_nocancel+0x14>
   10510:	d65f03c0 	ret
   10514:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10518:	4b0003e2 	neg	w2, w0
   1051c:	f947c421 	ldr	x1, [x1,#3976]
   10520:	d53bd043 	mrs	x3, tpidr_el0
   10524:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10528:	b8236822 	str	w2, [x1,x3]
   1052c:	d65f03c0 	ret

0000000000010530 <msync>:
   10530:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10534:	b9432a10 	ldr	w16, [x16,#808]
   10538:	34fffe50 	cbz	w16, 10500 <__msync_nocancel>
   1053c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10540:	a9010be1 	stp	x1, x2, [sp,#16]
   10544:	97fffc35 	bl	f618 <__pthread_enable_asynccancel>
   10548:	aa0003f0 	mov	x16, x0
   1054c:	f94007e0 	ldr	x0, [sp,#8]
   10550:	a9410be1 	ldp	x1, x2, [sp,#16]
   10554:	d2801c68 	mov	x8, #0xe3                  	// #227
   10558:	d4000001 	svc	#0x0
   1055c:	f90007e0 	str	x0, [sp,#8]
   10560:	aa1003e0 	mov	x0, x16
   10564:	97fffc5d 	bl	f6d8 <__pthread_disable_asynccancel>
   10568:	a8c403fe 	ldp	x30, x0, [sp],#64
   1056c:	17ffffe7 	b	10508 <__msync_nocancel+0x8>

0000000000010570 <__nanosleep_nocancel>:
   10570:	d2800ca8 	mov	x8, #0x65                  	// #101
   10574:	d4000001 	svc	#0x0
   10578:	b13ffc1f 	cmn	x0, #0xfff
   1057c:	54000042 	b.cs	10584 <__nanosleep_nocancel+0x14>
   10580:	d65f03c0 	ret
   10584:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10588:	4b0003e2 	neg	w2, w0
   1058c:	f947c421 	ldr	x1, [x1,#3976]
   10590:	d53bd043 	mrs	x3, tpidr_el0
   10594:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10598:	b8236822 	str	w2, [x1,x3]
   1059c:	d65f03c0 	ret

00000000000105a0 <__nanosleep>:
   105a0:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   105a4:	b9432a10 	ldr	w16, [x16,#808]
   105a8:	34fffe50 	cbz	w16, 10570 <__nanosleep_nocancel>
   105ac:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   105b0:	f9000be1 	str	x1, [sp,#16]
   105b4:	97fffc19 	bl	f618 <__pthread_enable_asynccancel>
   105b8:	aa0003f0 	mov	x16, x0
   105bc:	a94087e0 	ldp	x0, x1, [sp,#8]
   105c0:	d2800ca8 	mov	x8, #0x65                  	// #101
   105c4:	d4000001 	svc	#0x0
   105c8:	f90007e0 	str	x0, [sp,#8]
   105cc:	aa1003e0 	mov	x0, x16
   105d0:	97fffc42 	bl	f6d8 <__pthread_disable_asynccancel>
   105d4:	a8c403fe 	ldp	x30, x0, [sp],#64
   105d8:	17ffffe8 	b	10578 <__nanosleep_nocancel+0x8>
   105dc:	d503201f 	nop

00000000000105e0 <__open>:
   105e0:	a9b07bfd 	stp	x29, x30, [sp,#-256]!
   105e4:	910003fd 	mov	x29, sp
   105e8:	f9000bf3 	str	x19, [sp,#16]
   105ec:	2a0103f3 	mov	w19, w1
   105f0:	3d8017a0 	str	q0, [x29,#80]
   105f4:	aa0003e1 	mov	x1, x0
   105f8:	f9006fa3 	str	x3, [x29,#216]
   105fc:	d2800003 	mov	x3, #0x0                   	// #0
   10600:	f9006ba2 	str	x2, [x29,#208]
   10604:	f90073a4 	str	x4, [x29,#224]
   10608:	f90077a5 	str	x5, [x29,#232]
   1060c:	f9007ba6 	str	x6, [x29,#240]
   10610:	f9007fa7 	str	x7, [x29,#248]
   10614:	3d801ba1 	str	q1, [x29,#96]
   10618:	3d801fa2 	str	q2, [x29,#112]
   1061c:	3d8023a3 	str	q3, [x29,#128]
   10620:	3d8027a4 	str	q4, [x29,#144]
   10624:	3d802ba5 	str	q5, [x29,#160]
   10628:	3d802fa6 	str	q6, [x29,#176]
   1062c:	3d8033a7 	str	q7, [x29,#192]
   10630:	373001f3 	tbnz	w19, #6, 1066c <__open+0x8c>
   10634:	90000120 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
   10638:	b9432800 	ldr	w0, [x0,#808]
   1063c:	35000320 	cbnz	w0, 106a0 <__open+0xc0>
   10640:	92800c60 	mov	x0, #0xffffffffffffff9c    	// #-100
   10644:	93407e62 	sxtw	x2, w19
   10648:	d2800708 	mov	x8, #0x38                  	// #56
   1064c:	d4000001 	svc	#0x0
   10650:	b140041f 	cmn	x0, #0x1, lsl #12
   10654:	540004c8 	b.hi	106ec <__open+0x10c>
   10658:	2a0003f3 	mov	w19, w0
   1065c:	2a1303e0 	mov	w0, w19
   10660:	f9400bf3 	ldr	x19, [sp,#16]
   10664:	a8d07bfd 	ldp	x29, x30, [sp],#256
   10668:	d65f03c0 	ret
   1066c:	910343a0 	add	x0, x29, #0xd0
   10670:	f90023a0 	str	x0, [x29,#64]
   10674:	128005e0 	mov	w0, #0xffffffd0            	// #-48
   10678:	b9004ba0 	str	w0, [x29,#72]
   1067c:	12800fe0 	mov	w0, #0xffffff80            	// #-128
   10680:	b9004fa0 	str	w0, [x29,#76]
   10684:	90000120 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
   10688:	910403a2 	add	x2, x29, #0x100
   1068c:	f9001ba2 	str	x2, [x29,#48]
   10690:	f9001fa2 	str	x2, [x29,#56]
   10694:	b9432800 	ldr	w0, [x0,#808]
   10698:	b980d3a3 	ldrsw	x3, [x29,#208]
   1069c:	34fffd20 	cbz	w0, 10640 <__open+0x60>
   106a0:	f90013a1 	str	x1, [x29,#32]
   106a4:	f90017a3 	str	x3, [x29,#40]
   106a8:	97fffbdc 	bl	f618 <__pthread_enable_asynccancel>
   106ac:	f94013a1 	ldr	x1, [x29,#32]
   106b0:	2a0003e4 	mov	w4, w0
   106b4:	93407e62 	sxtw	x2, w19
   106b8:	92800c60 	mov	x0, #0xffffffffffffff9c    	// #-100
   106bc:	f94017a3 	ldr	x3, [x29,#40]
   106c0:	d2800708 	mov	x8, #0x38                  	// #56
   106c4:	d4000001 	svc	#0x0
   106c8:	b140041f 	cmn	x0, #0x1, lsl #12
   106cc:	540001e8 	b.hi	10708 <__open+0x128>
   106d0:	2a0003f3 	mov	w19, w0
   106d4:	2a0403e0 	mov	w0, w4
   106d8:	97fffc00 	bl	f6d8 <__pthread_disable_asynccancel>
   106dc:	2a1303e0 	mov	w0, w19
   106e0:	f9400bf3 	ldr	x19, [sp,#16]
   106e4:	a8d07bfd 	ldp	x29, x30, [sp],#256
   106e8:	d65f03c0 	ret
   106ec:	d53bd041 	mrs	x1, tpidr_el0
   106f0:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   106f4:	f947c442 	ldr	x2, [x2,#3976]
   106f8:	4b0003e0 	neg	w0, w0
   106fc:	12800013 	mov	w19, #0xffffffff            	// #-1
   10700:	b8226820 	str	w0, [x1,x2]
   10704:	17ffffd6 	b	1065c <__open+0x7c>
   10708:	d53bd041 	mrs	x1, tpidr_el0
   1070c:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10710:	f947c442 	ldr	x2, [x2,#3976]
   10714:	4b0003e0 	neg	w0, w0
   10718:	12800013 	mov	w19, #0xffffffff            	// #-1
   1071c:	b8226820 	str	w0, [x1,x2]
   10720:	17ffffed 	b	106d4 <__open+0xf4>

0000000000010724 <__open_nocancel>:
   10724:	d10343ff 	sub	sp, sp, #0xd0
   10728:	3d800be0 	str	q0, [sp,#32]
   1072c:	f90053e2 	str	x2, [sp,#160]
   10730:	93407c22 	sxtw	x2, w1
   10734:	f90057e3 	str	x3, [sp,#168]
   10738:	aa0003e1 	mov	x1, x0
   1073c:	f9005be4 	str	x4, [sp,#176]
   10740:	d2800003 	mov	x3, #0x0                   	// #0
   10744:	f9005fe5 	str	x5, [sp,#184]
   10748:	f90063e6 	str	x6, [sp,#192]
   1074c:	f90067e7 	str	x7, [sp,#200]
   10750:	3d800fe1 	str	q1, [sp,#48]
   10754:	3d8013e2 	str	q2, [sp,#64]
   10758:	3d8017e3 	str	q3, [sp,#80]
   1075c:	3d801be4 	str	q4, [sp,#96]
   10760:	3d801fe5 	str	q5, [sp,#112]
   10764:	3d8023e6 	str	q6, [sp,#128]
   10768:	3d8027e7 	str	q7, [sp,#144]
   1076c:	37300102 	tbnz	w2, #6, 1078c <__open_nocancel+0x68>
   10770:	92800c60 	mov	x0, #0xffffffffffffff9c    	// #-100
   10774:	d2800708 	mov	x8, #0x38                  	// #56
   10778:	d4000001 	svc	#0x0
   1077c:	b140041f 	cmn	x0, #0x1, lsl #12
   10780:	54000248 	b.hi	107c8 <__open_nocancel+0xa4>
   10784:	910343ff 	add	sp, sp, #0xd0
   10788:	d65f03c0 	ret
   1078c:	910283e0 	add	x0, sp, #0xa0
   10790:	f9000be0 	str	x0, [sp,#16]
   10794:	128005e0 	mov	w0, #0xffffffd0            	// #-48
   10798:	b9001be0 	str	w0, [sp,#24]
   1079c:	12800fe0 	mov	w0, #0xffffff80            	// #-128
   107a0:	910343e4 	add	x4, sp, #0xd0
   107a4:	b9001fe0 	str	w0, [sp,#28]
   107a8:	d2800708 	mov	x8, #0x38                  	// #56
   107ac:	b980a3e3 	ldrsw	x3, [sp,#160]
   107b0:	92800c60 	mov	x0, #0xffffffffffffff9c    	// #-100
   107b4:	f90003e4 	str	x4, [sp]
   107b8:	f90007e4 	str	x4, [sp,#8]
   107bc:	d4000001 	svc	#0x0
   107c0:	b140041f 	cmn	x0, #0x1, lsl #12
   107c4:	54fffe09 	b.ls	10784 <__open_nocancel+0x60>
   107c8:	d53bd041 	mrs	x1, tpidr_el0
   107cc:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   107d0:	f947c442 	ldr	x2, [x2,#3976]
   107d4:	4b0003e3 	neg	w3, w0
   107d8:	910343ff 	add	sp, sp, #0xd0
   107dc:	12800000 	mov	w0, #0xffffffff            	// #-1
   107e0:	b8226823 	str	w3, [x1,x2]
   107e4:	d65f03c0 	ret

00000000000107e8 <__open64>:
   107e8:	a9b07bfd 	stp	x29, x30, [sp,#-256]!
   107ec:	910003fd 	mov	x29, sp
   107f0:	f9000bf3 	str	x19, [sp,#16]
   107f4:	2a0103f3 	mov	w19, w1
   107f8:	3d8017a0 	str	q0, [x29,#80]
   107fc:	aa0003e1 	mov	x1, x0
   10800:	f9006fa3 	str	x3, [x29,#216]
   10804:	d2800003 	mov	x3, #0x0                   	// #0
   10808:	f9006ba2 	str	x2, [x29,#208]
   1080c:	f90073a4 	str	x4, [x29,#224]
   10810:	f90077a5 	str	x5, [x29,#232]
   10814:	f9007ba6 	str	x6, [x29,#240]
   10818:	f9007fa7 	str	x7, [x29,#248]
   1081c:	3d801ba1 	str	q1, [x29,#96]
   10820:	3d801fa2 	str	q2, [x29,#112]
   10824:	3d8023a3 	str	q3, [x29,#128]
   10828:	3d8027a4 	str	q4, [x29,#144]
   1082c:	3d802ba5 	str	q5, [x29,#160]
   10830:	3d802fa6 	str	q6, [x29,#176]
   10834:	3d8033a7 	str	q7, [x29,#192]
   10838:	373001f3 	tbnz	w19, #6, 10874 <__open64+0x8c>
   1083c:	90000120 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
   10840:	b9432800 	ldr	w0, [x0,#808]
   10844:	35000320 	cbnz	w0, 108a8 <__open64+0xc0>
   10848:	92800c60 	mov	x0, #0xffffffffffffff9c    	// #-100
   1084c:	93407e62 	sxtw	x2, w19
   10850:	d2800708 	mov	x8, #0x38                  	// #56
   10854:	d4000001 	svc	#0x0
   10858:	b140041f 	cmn	x0, #0x1, lsl #12
   1085c:	540004c8 	b.hi	108f4 <__open64+0x10c>
   10860:	2a0003f3 	mov	w19, w0
   10864:	2a1303e0 	mov	w0, w19
   10868:	f9400bf3 	ldr	x19, [sp,#16]
   1086c:	a8d07bfd 	ldp	x29, x30, [sp],#256
   10870:	d65f03c0 	ret
   10874:	910343a0 	add	x0, x29, #0xd0
   10878:	f90023a0 	str	x0, [x29,#64]
   1087c:	128005e0 	mov	w0, #0xffffffd0            	// #-48
   10880:	b9004ba0 	str	w0, [x29,#72]
   10884:	12800fe0 	mov	w0, #0xffffff80            	// #-128
   10888:	b9004fa0 	str	w0, [x29,#76]
   1088c:	90000120 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
   10890:	910403a2 	add	x2, x29, #0x100
   10894:	f9001ba2 	str	x2, [x29,#48]
   10898:	f9001fa2 	str	x2, [x29,#56]
   1089c:	b9432800 	ldr	w0, [x0,#808]
   108a0:	b980d3a3 	ldrsw	x3, [x29,#208]
   108a4:	34fffd20 	cbz	w0, 10848 <__open64+0x60>
   108a8:	f90013a1 	str	x1, [x29,#32]
   108ac:	f90017a3 	str	x3, [x29,#40]
   108b0:	97fffb5a 	bl	f618 <__pthread_enable_asynccancel>
   108b4:	f94013a1 	ldr	x1, [x29,#32]
   108b8:	2a0003e4 	mov	w4, w0
   108bc:	93407e62 	sxtw	x2, w19
   108c0:	92800c60 	mov	x0, #0xffffffffffffff9c    	// #-100
   108c4:	f94017a3 	ldr	x3, [x29,#40]
   108c8:	d2800708 	mov	x8, #0x38                  	// #56
   108cc:	d4000001 	svc	#0x0
   108d0:	b140041f 	cmn	x0, #0x1, lsl #12
   108d4:	540001e8 	b.hi	10910 <__open64+0x128>
   108d8:	2a0003f3 	mov	w19, w0
   108dc:	2a0403e0 	mov	w0, w4
   108e0:	97fffb7e 	bl	f6d8 <__pthread_disable_asynccancel>
   108e4:	2a1303e0 	mov	w0, w19
   108e8:	f9400bf3 	ldr	x19, [sp,#16]
   108ec:	a8d07bfd 	ldp	x29, x30, [sp],#256
   108f0:	d65f03c0 	ret
   108f4:	d53bd041 	mrs	x1, tpidr_el0
   108f8:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   108fc:	f947c442 	ldr	x2, [x2,#3976]
   10900:	4b0003e0 	neg	w0, w0
   10904:	12800013 	mov	w19, #0xffffffff            	// #-1
   10908:	b8226820 	str	w0, [x1,x2]
   1090c:	17ffffd6 	b	10864 <__open64+0x7c>
   10910:	d53bd041 	mrs	x1, tpidr_el0
   10914:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10918:	f947c442 	ldr	x2, [x2,#3976]
   1091c:	4b0003e0 	neg	w0, w0
   10920:	12800013 	mov	w19, #0xffffffff            	// #-1
   10924:	b8226820 	str	w0, [x1,x2]
   10928:	17ffffed 	b	108dc <__open64+0xf4>

000000000001092c <pause>:
   1092c:	90000120 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
   10930:	a9b67bfd 	stp	x29, x30, [sp,#-160]!
   10934:	910003fd 	mov	x29, sp
   10938:	b9432800 	ldr	w0, [x0,#808]
   1093c:	f9000bf3 	str	x19, [sp,#16]
   10940:	350003c0 	cbnz	w0, 109b8 <pause+0x8c>
   10944:	910083a4 	add	x4, x29, #0x20
   10948:	d2800000 	mov	x0, #0x0                   	// #0
   1094c:	aa0003e1 	mov	x1, x0
   10950:	aa0403e2 	mov	x2, x4
   10954:	d2800103 	mov	x3, #0x8                   	// #8
   10958:	d28010e8 	mov	x8, #0x87                  	// #135
   1095c:	d4000001 	svc	#0x0
   10960:	b140041f 	cmn	x0, #0x1, lsl #12
   10964:	540001c8 	b.hi	1099c <pause+0x70>
   10968:	2a0003e1 	mov	w1, w0
   1096c:	35000100 	cbnz	w0, 1098c <pause+0x60>
   10970:	aa0403e0 	mov	x0, x4
   10974:	aa0303e1 	mov	x1, x3
   10978:	d28010a8 	mov	x8, #0x85                  	// #133
   1097c:	d4000001 	svc	#0x0
   10980:	b140041f 	cmn	x0, #0x1, lsl #12
   10984:	540000c8 	b.hi	1099c <pause+0x70>
   10988:	2a0003e1 	mov	w1, w0
   1098c:	2a0103e0 	mov	w0, w1
   10990:	f9400bf3 	ldr	x19, [sp,#16]
   10994:	a8ca7bfd 	ldp	x29, x30, [sp],#160
   10998:	d65f03c0 	ret
   1099c:	d53bd042 	mrs	x2, tpidr_el0
   109a0:	f00000e3 	adrp	x3, 2f000 <__FRAME_END__+0x18e30>
   109a4:	f947c463 	ldr	x3, [x3,#3976]
   109a8:	4b0003e0 	neg	w0, w0
   109ac:	12800001 	mov	w1, #0xffffffff            	// #-1
   109b0:	b8236840 	str	w0, [x2,x3]
   109b4:	17fffff6 	b	1098c <pause+0x60>
   109b8:	97fffb18 	bl	f618 <__pthread_enable_asynccancel>
   109bc:	2a0003e5 	mov	w5, w0
   109c0:	910083a4 	add	x4, x29, #0x20
   109c4:	d2800000 	mov	x0, #0x0                   	// #0
   109c8:	aa0003e1 	mov	x1, x0
   109cc:	aa0403e2 	mov	x2, x4
   109d0:	d2800103 	mov	x3, #0x8                   	// #8
   109d4:	d28010e8 	mov	x8, #0x87                  	// #135
   109d8:	d4000001 	svc	#0x0
   109dc:	b140041f 	cmn	x0, #0x1, lsl #12
   109e0:	54000208 	b.hi	10a20 <pause+0xf4>
   109e4:	2a0003f3 	mov	w19, w0
   109e8:	35000100 	cbnz	w0, 10a08 <pause+0xdc>
   109ec:	aa0403e0 	mov	x0, x4
   109f0:	aa0303e1 	mov	x1, x3
   109f4:	d28010a8 	mov	x8, #0x85                  	// #133
   109f8:	d4000001 	svc	#0x0
   109fc:	b140041f 	cmn	x0, #0x1, lsl #12
   10a00:	540001e8 	b.hi	10a3c <pause+0x110>
   10a04:	2a0003f3 	mov	w19, w0
   10a08:	2a0503e0 	mov	w0, w5
   10a0c:	97fffb33 	bl	f6d8 <__pthread_disable_asynccancel>
   10a10:	2a1303e0 	mov	w0, w19
   10a14:	f9400bf3 	ldr	x19, [sp,#16]
   10a18:	a8ca7bfd 	ldp	x29, x30, [sp],#160
   10a1c:	d65f03c0 	ret
   10a20:	4b0003e1 	neg	w1, w0
   10a24:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10a28:	f947c442 	ldr	x2, [x2,#3976]
   10a2c:	d53bd040 	mrs	x0, tpidr_el0
   10a30:	12800013 	mov	w19, #0xffffffff            	// #-1
   10a34:	b8226801 	str	w1, [x0,x2]
   10a38:	17fffff4 	b	10a08 <pause+0xdc>
   10a3c:	d53bd041 	mrs	x1, tpidr_el0
   10a40:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10a44:	f947c442 	ldr	x2, [x2,#3976]
   10a48:	4b0003e0 	neg	w0, w0
   10a4c:	12800013 	mov	w19, #0xffffffff            	// #-1
   10a50:	b8226820 	str	w0, [x1,x2]
   10a54:	17ffffed 	b	10a08 <pause+0xdc>

0000000000010a58 <__pause_nocancel>:
   10a58:	d10203ff 	sub	sp, sp, #0x80
   10a5c:	d2800000 	mov	x0, #0x0                   	// #0
   10a60:	aa0003e1 	mov	x1, x0
   10a64:	910003e2 	mov	x2, sp
   10a68:	d2800103 	mov	x3, #0x8                   	// #8
   10a6c:	d28010e8 	mov	x8, #0x87                  	// #135
   10a70:	d4000001 	svc	#0x0
   10a74:	b140041f 	cmn	x0, #0x1, lsl #12
   10a78:	540001a8 	b.hi	10aac <__pause_nocancel+0x54>
   10a7c:	2a0003e1 	mov	w1, w0
   10a80:	35000100 	cbnz	w0, 10aa0 <__pause_nocancel+0x48>
   10a84:	910003e0 	mov	x0, sp
   10a88:	aa0303e1 	mov	x1, x3
   10a8c:	d28010a8 	mov	x8, #0x85                  	// #133
   10a90:	d4000001 	svc	#0x0
   10a94:	b140041f 	cmn	x0, #0x1, lsl #12
   10a98:	540000a8 	b.hi	10aac <__pause_nocancel+0x54>
   10a9c:	2a0003e1 	mov	w1, w0
   10aa0:	2a0103e0 	mov	w0, w1
   10aa4:	910203ff 	add	sp, sp, #0x80
   10aa8:	d65f03c0 	ret
   10aac:	d53bd042 	mrs	x2, tpidr_el0
   10ab0:	f00000e3 	adrp	x3, 2f000 <__FRAME_END__+0x18e30>
   10ab4:	f947c463 	ldr	x3, [x3,#3976]
   10ab8:	4b0003e0 	neg	w0, w0
   10abc:	12800001 	mov	w1, #0xffffffff            	// #-1
   10ac0:	910203ff 	add	sp, sp, #0x80
   10ac4:	b8236840 	str	w0, [x2,x3]
   10ac8:	2a0103e0 	mov	w0, w1
   10acc:	d65f03c0 	ret

0000000000010ad0 <__pread_nocancel>:
   10ad0:	d2800868 	mov	x8, #0x43                  	// #67
   10ad4:	d4000001 	svc	#0x0
   10ad8:	b13ffc1f 	cmn	x0, #0xfff
   10adc:	54000042 	b.cs	10ae4 <__pread_nocancel+0x14>
   10ae0:	d65f03c0 	ret
   10ae4:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10ae8:	4b0003e2 	neg	w2, w0
   10aec:	f947c421 	ldr	x1, [x1,#3976]
   10af0:	d53bd043 	mrs	x3, tpidr_el0
   10af4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10af8:	b8236822 	str	w2, [x1,x3]
   10afc:	d65f03c0 	ret

0000000000010b00 <__pread64>:
   10b00:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10b04:	b9432a10 	ldr	w16, [x16,#808]
   10b08:	34fffe50 	cbz	w16, 10ad0 <__pread_nocancel>
   10b0c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10b10:	a9010be1 	stp	x1, x2, [sp,#16]
   10b14:	f90013e3 	str	x3, [sp,#32]
   10b18:	97fffac0 	bl	f618 <__pthread_enable_asynccancel>
   10b1c:	aa0003f0 	mov	x16, x0
   10b20:	a94087e0 	ldp	x0, x1, [sp,#8]
   10b24:	a9418fe2 	ldp	x2, x3, [sp,#24]
   10b28:	d2800868 	mov	x8, #0x43                  	// #67
   10b2c:	d4000001 	svc	#0x0
   10b30:	f90007e0 	str	x0, [sp,#8]
   10b34:	aa1003e0 	mov	x0, x16
   10b38:	97fffae8 	bl	f6d8 <__pthread_disable_asynccancel>
   10b3c:	a8c403fe 	ldp	x30, x0, [sp],#64
   10b40:	17ffffe6 	b	10ad8 <__pread_nocancel+0x8>
   10b44:	d503201f 	nop
   10b48:	d503201f 	nop
   10b4c:	d503201f 	nop

0000000000010b50 <__pwrite_nocancel>:
   10b50:	d2800888 	mov	x8, #0x44                  	// #68
   10b54:	d4000001 	svc	#0x0
   10b58:	b13ffc1f 	cmn	x0, #0xfff
   10b5c:	54000042 	b.cs	10b64 <__pwrite_nocancel+0x14>
   10b60:	d65f03c0 	ret
   10b64:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10b68:	4b0003e2 	neg	w2, w0
   10b6c:	f947c421 	ldr	x1, [x1,#3976]
   10b70:	d53bd043 	mrs	x3, tpidr_el0
   10b74:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10b78:	b8236822 	str	w2, [x1,x3]
   10b7c:	d65f03c0 	ret

0000000000010b80 <__pwrite64>:
   10b80:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10b84:	b9432a10 	ldr	w16, [x16,#808]
   10b88:	34fffe50 	cbz	w16, 10b50 <__pwrite_nocancel>
   10b8c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10b90:	a9010be1 	stp	x1, x2, [sp,#16]
   10b94:	f90013e3 	str	x3, [sp,#32]
   10b98:	97fffaa0 	bl	f618 <__pthread_enable_asynccancel>
   10b9c:	aa0003f0 	mov	x16, x0
   10ba0:	a94087e0 	ldp	x0, x1, [sp,#8]
   10ba4:	a9418fe2 	ldp	x2, x3, [sp,#24]
   10ba8:	d2800888 	mov	x8, #0x44                  	// #68
   10bac:	d4000001 	svc	#0x0
   10bb0:	f90007e0 	str	x0, [sp,#8]
   10bb4:	aa1003e0 	mov	x0, x16
   10bb8:	97fffac8 	bl	f6d8 <__pthread_disable_asynccancel>
   10bbc:	a8c403fe 	ldp	x30, x0, [sp],#64
   10bc0:	17ffffe6 	b	10b58 <__pwrite_nocancel+0x8>
   10bc4:	d503201f 	nop
   10bc8:	d503201f 	nop
   10bcc:	d503201f 	nop

0000000000010bd0 <tcdrain>:
   10bd0:	90000121 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
   10bd4:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
   10bd8:	910003fd 	mov	x29, sp
   10bdc:	b9432821 	ldr	w1, [x1,#808]
   10be0:	f9000bf3 	str	x19, [sp,#16]
   10be4:	350001a1 	cbnz	w1, 10c18 <tcdrain+0x48>
   10be8:	93407c00 	sxtw	x0, w0
   10bec:	d28a8121 	mov	x1, #0x5409                	// #21513
   10bf0:	d2800022 	mov	x2, #0x1                   	// #1
   10bf4:	d28003a8 	mov	x8, #0x1d                  	// #29
   10bf8:	d4000001 	svc	#0x0
   10bfc:	b140041f 	cmn	x0, #0x1, lsl #12
   10c00:	540002e8 	b.hi	10c5c <tcdrain+0x8c>
   10c04:	2a0003f3 	mov	w19, w0
   10c08:	2a1303e0 	mov	w0, w19
   10c0c:	f9400bf3 	ldr	x19, [sp,#16]
   10c10:	a8c27bfd 	ldp	x29, x30, [sp],#32
   10c14:	d65f03c0 	ret
   10c18:	2a0003f3 	mov	w19, w0
   10c1c:	97fffa7f 	bl	f618 <__pthread_enable_asynccancel>
   10c20:	d28a8121 	mov	x1, #0x5409                	// #21513
   10c24:	2a0003e3 	mov	w3, w0
   10c28:	d2800022 	mov	x2, #0x1                   	// #1
   10c2c:	93407e60 	sxtw	x0, w19
   10c30:	d28003a8 	mov	x8, #0x1d                  	// #29
   10c34:	d4000001 	svc	#0x0
   10c38:	b140041f 	cmn	x0, #0x1, lsl #12
   10c3c:	540001e8 	b.hi	10c78 <tcdrain+0xa8>
   10c40:	2a0003f3 	mov	w19, w0
   10c44:	2a0303e0 	mov	w0, w3
   10c48:	97fffaa4 	bl	f6d8 <__pthread_disable_asynccancel>
   10c4c:	2a1303e0 	mov	w0, w19
   10c50:	f9400bf3 	ldr	x19, [sp,#16]
   10c54:	a8c27bfd 	ldp	x29, x30, [sp],#32
   10c58:	d65f03c0 	ret
   10c5c:	d53bd041 	mrs	x1, tpidr_el0
   10c60:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10c64:	f947c442 	ldr	x2, [x2,#3976]
   10c68:	4b0003e0 	neg	w0, w0
   10c6c:	12800013 	mov	w19, #0xffffffff            	// #-1
   10c70:	b8226820 	str	w0, [x1,x2]
   10c74:	17ffffe5 	b	10c08 <tcdrain+0x38>
   10c78:	d53bd041 	mrs	x1, tpidr_el0
   10c7c:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10c80:	f947c442 	ldr	x2, [x2,#3976]
   10c84:	4b0003e0 	neg	w0, w0
   10c88:	12800013 	mov	w19, #0xffffffff            	// #-1
   10c8c:	b8226820 	str	w0, [x1,x2]
   10c90:	17ffffed 	b	10c44 <tcdrain+0x74>

0000000000010c94 <__wait>:
   10c94:	aa0003e1 	mov	x1, x0
   10c98:	90000120 	adrp	x0, 34000 <__GI___pthread_keys+0x3d78>
   10c9c:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   10ca0:	910003fd 	mov	x29, sp
   10ca4:	b9432800 	ldr	w0, [x0,#808]
   10ca8:	f9000bf3 	str	x19, [sp,#16]
   10cac:	350001a0 	cbnz	w0, 10ce0 <__wait+0x4c>
   10cb0:	d2800002 	mov	x2, #0x0                   	// #0
   10cb4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10cb8:	aa0203e3 	mov	x3, x2
   10cbc:	d2802088 	mov	x8, #0x104                 	// #260
   10cc0:	d4000001 	svc	#0x0
   10cc4:	b140041f 	cmn	x0, #0x1, lsl #12
   10cc8:	54000308 	b.hi	10d28 <__wait+0x94>
   10ccc:	2a0003f3 	mov	w19, w0
   10cd0:	2a1303e0 	mov	w0, w19
   10cd4:	f9400bf3 	ldr	x19, [sp,#16]
   10cd8:	a8c37bfd 	ldp	x29, x30, [sp],#48
   10cdc:	d65f03c0 	ret
   10ce0:	f90017a1 	str	x1, [x29,#40]
   10ce4:	97fffa4d 	bl	f618 <__pthread_enable_asynccancel>
   10ce8:	d2800002 	mov	x2, #0x0                   	// #0
   10cec:	2a0003e4 	mov	w4, w0
   10cf0:	f94017a1 	ldr	x1, [x29,#40]
   10cf4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10cf8:	aa0203e3 	mov	x3, x2
   10cfc:	d2802088 	mov	x8, #0x104                 	// #260
   10d00:	d4000001 	svc	#0x0
   10d04:	b140041f 	cmn	x0, #0x1, lsl #12
   10d08:	540001e8 	b.hi	10d44 <__wait+0xb0>
   10d0c:	2a0003f3 	mov	w19, w0
   10d10:	2a0403e0 	mov	w0, w4
   10d14:	97fffa71 	bl	f6d8 <__pthread_disable_asynccancel>
   10d18:	2a1303e0 	mov	w0, w19
   10d1c:	f9400bf3 	ldr	x19, [sp,#16]
   10d20:	a8c37bfd 	ldp	x29, x30, [sp],#48
   10d24:	d65f03c0 	ret
   10d28:	d53bd041 	mrs	x1, tpidr_el0
   10d2c:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10d30:	f947c442 	ldr	x2, [x2,#3976]
   10d34:	4b0003e0 	neg	w0, w0
   10d38:	12800013 	mov	w19, #0xffffffff            	// #-1
   10d3c:	b8226820 	str	w0, [x1,x2]
   10d40:	17ffffe4 	b	10cd0 <__wait+0x3c>
   10d44:	d53bd041 	mrs	x1, tpidr_el0
   10d48:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10d4c:	f947c442 	ldr	x2, [x2,#3976]
   10d50:	4b0003e0 	neg	w0, w0
   10d54:	12800013 	mov	w19, #0xffffffff            	// #-1
   10d58:	b8226820 	str	w0, [x1,x2]
   10d5c:	17ffffed 	b	10d10 <__wait+0x7c>

0000000000010d60 <waitpid>:
   10d60:	90000123 	adrp	x3, 34000 <__GI___pthread_keys+0x3d78>
   10d64:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   10d68:	910003fd 	mov	x29, sp
   10d6c:	b9432863 	ldr	w3, [x3,#808]
   10d70:	a90153f3 	stp	x19, x20, [sp,#16]
   10d74:	350001a3 	cbnz	w3, 10da8 <waitpid+0x48>
   10d78:	93407c00 	sxtw	x0, w0
   10d7c:	93407c42 	sxtw	x2, w2
   10d80:	d2800003 	mov	x3, #0x0                   	// #0
   10d84:	d2802088 	mov	x8, #0x104                 	// #260
   10d88:	d4000001 	svc	#0x0
   10d8c:	b140041f 	cmn	x0, #0x1, lsl #12
   10d90:	54000348 	b.hi	10df8 <waitpid+0x98>
   10d94:	2a0003f3 	mov	w19, w0
   10d98:	2a1303e0 	mov	w0, w19
   10d9c:	a94153f3 	ldp	x19, x20, [sp,#16]
   10da0:	a8c37bfd 	ldp	x29, x30, [sp],#48
   10da4:	d65f03c0 	ret
   10da8:	2a0203f3 	mov	w19, w2
   10dac:	2a0003f4 	mov	w20, w0
   10db0:	f90017a1 	str	x1, [x29,#40]
   10db4:	97fffa19 	bl	f618 <__pthread_enable_asynccancel>
   10db8:	f94017a1 	ldr	x1, [x29,#40]
   10dbc:	2a0003e4 	mov	w4, w0
   10dc0:	93407e62 	sxtw	x2, w19
   10dc4:	93407e80 	sxtw	x0, w20
   10dc8:	d2800003 	mov	x3, #0x0                   	// #0
   10dcc:	d2802088 	mov	x8, #0x104                 	// #260
   10dd0:	d4000001 	svc	#0x0
   10dd4:	b140041f 	cmn	x0, #0x1, lsl #12
   10dd8:	540001e8 	b.hi	10e14 <waitpid+0xb4>
   10ddc:	2a0003f3 	mov	w19, w0
   10de0:	2a0403e0 	mov	w0, w4
   10de4:	97fffa3d 	bl	f6d8 <__pthread_disable_asynccancel>
   10de8:	2a1303e0 	mov	w0, w19
   10dec:	a94153f3 	ldp	x19, x20, [sp,#16]
   10df0:	a8c37bfd 	ldp	x29, x30, [sp],#48
   10df4:	d65f03c0 	ret
   10df8:	d53bd041 	mrs	x1, tpidr_el0
   10dfc:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10e00:	f947c442 	ldr	x2, [x2,#3976]
   10e04:	4b0003e0 	neg	w0, w0
   10e08:	12800013 	mov	w19, #0xffffffff            	// #-1
   10e0c:	b8226820 	str	w0, [x1,x2]
   10e10:	17ffffe2 	b	10d98 <waitpid+0x38>
   10e14:	d53bd041 	mrs	x1, tpidr_el0
   10e18:	f00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   10e1c:	f947c442 	ldr	x2, [x2,#3976]
   10e20:	4b0003e0 	neg	w0, w0
   10e24:	12800013 	mov	w19, #0xffffffff            	// #-1
   10e28:	b8226820 	str	w0, [x1,x2]
   10e2c:	17ffffed 	b	10de0 <waitpid+0x80>

0000000000010e30 <__msgrcv_nocancel>:
   10e30:	d2801788 	mov	x8, #0xbc                  	// #188
   10e34:	d4000001 	svc	#0x0
   10e38:	b13ffc1f 	cmn	x0, #0xfff
   10e3c:	54000042 	b.cs	10e44 <__msgrcv_nocancel+0x14>
   10e40:	d65f03c0 	ret
   10e44:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10e48:	4b0003e2 	neg	w2, w0
   10e4c:	f947c421 	ldr	x1, [x1,#3976]
   10e50:	d53bd043 	mrs	x3, tpidr_el0
   10e54:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10e58:	b8236822 	str	w2, [x1,x3]
   10e5c:	d65f03c0 	ret

0000000000010e60 <__msgrcv>:
   10e60:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10e64:	b9432a10 	ldr	w16, [x16,#808]
   10e68:	34fffe50 	cbz	w16, 10e30 <__msgrcv_nocancel>
   10e6c:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10e70:	a9010be1 	stp	x1, x2, [sp,#16]
   10e74:	a90213e3 	stp	x3, x4, [sp,#32]
   10e78:	97fff9e8 	bl	f618 <__pthread_enable_asynccancel>
   10e7c:	aa0003f0 	mov	x16, x0
   10e80:	f94007e0 	ldr	x0, [sp,#8]
   10e84:	a9410be1 	ldp	x1, x2, [sp,#16]
   10e88:	a94213e3 	ldp	x3, x4, [sp,#32]
   10e8c:	d2801788 	mov	x8, #0xbc                  	// #188
   10e90:	d4000001 	svc	#0x0
   10e94:	f90007e0 	str	x0, [sp,#8]
   10e98:	aa1003e0 	mov	x0, x16
   10e9c:	97fffa0f 	bl	f6d8 <__pthread_disable_asynccancel>
   10ea0:	a8c403fe 	ldp	x30, x0, [sp],#64
   10ea4:	17ffffe5 	b	10e38 <__msgrcv_nocancel+0x8>
   10ea8:	d503201f 	nop
   10eac:	d503201f 	nop

0000000000010eb0 <__msgsnd_nocancel>:
   10eb0:	d28017a8 	mov	x8, #0xbd                  	// #189
   10eb4:	d4000001 	svc	#0x0
   10eb8:	b13ffc1f 	cmn	x0, #0xfff
   10ebc:	54000042 	b.cs	10ec4 <__msgsnd_nocancel+0x14>
   10ec0:	d65f03c0 	ret
   10ec4:	f00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   10ec8:	4b0003e2 	neg	w2, w0
   10ecc:	f947c421 	ldr	x1, [x1,#3976]
   10ed0:	d53bd043 	mrs	x3, tpidr_el0
   10ed4:	92800000 	mov	x0, #0xffffffffffffffff    	// #-1
   10ed8:	b8236822 	str	w2, [x1,x3]
   10edc:	d65f03c0 	ret

0000000000010ee0 <__msgsnd>:
   10ee0:	90000130 	adrp	x16, 34000 <__GI___pthread_keys+0x3d78>
   10ee4:	b9432a10 	ldr	w16, [x16,#808]
   10ee8:	34fffe50 	cbz	w16, 10eb0 <__msgsnd_nocancel>
   10eec:	a9bc03fe 	stp	x30, x0, [sp,#-64]!
   10ef0:	a9010be1 	stp	x1, x2, [sp,#16]
   10ef4:	f90013e3 	str	x3, [sp,#32]
   10ef8:	97fff9c8 	bl	f618 <__pthread_enable_asynccancel>
   10efc:	aa0003f0 	mov	x16, x0
   10f00:	a94087e0 	ldp	x0, x1, [sp,#8]
   10f04:	a9418fe2 	ldp	x2, x3, [sp,#24]
   10f08:	d28017a8 	mov	x8, #0xbd                  	// #189
   10f0c:	d4000001 	svc	#0x0
   10f10:	f90007e0 	str	x0, [sp,#8]
   10f14:	aa1003e0 	mov	x0, x16
   10f18:	97fff9f0 	bl	f6d8 <__pthread_disable_asynccancel>
   10f1c:	a8c403fe 	ldp	x30, x0, [sp],#64
   10f20:	17ffffe6 	b	10eb8 <__msgsnd_nocancel+0x8>
   10f24:	d503201f 	nop
   10f28:	d503201f 	nop
   10f2c:	d503201f 	nop

0000000000010f30 <sigwait>:
   10f30:	90000122 	adrp	x2, 34000 <__GI___pthread_keys+0x3d78>
   10f34:	aa0103e5 	mov	x5, x1
   10f38:	a9b67bfd 	stp	x29, x30, [sp,#-160]!
   10f3c:	910003fd 	mov	x29, sp
   10f40:	b9432842 	ldr	w2, [x2,#808]
   10f44:	a90153f3 	stp	x19, x20, [sp,#16]
   10f48:	350003e2 	cbnz	w2, 10fc4 <sigwait+0x94>
   10f4c:	b4000080 	cbz	x0, 10f5c <sigwait+0x2c>
   10f50:	f9400002 	ldr	x2, [x0]
   10f54:	f261045f 	tst	x2, #0x180000000
   10f58:	540002e1 	b.ne	10fb4 <sigwait+0x84>
   10f5c:	aa0003e4 	mov	x4, x0
   10f60:	14000003 	b	10f6c <sigwait+0x3c>
   10f64:	3140041f 	cmn	w0, #0x1, lsl #12
   10f68:	54000169 	b.ls	10f94 <sigwait+0x64>
   10f6c:	d2800001 	mov	x1, #0x0                   	// #0
   10f70:	aa0403e0 	mov	x0, x4
   10f74:	aa0103e2 	mov	x2, x1
   10f78:	d2800103 	mov	x3, #0x8                   	// #8
   10f7c:	d2801128 	mov	x8, #0x89                  	// #137
   10f80:	d4000001 	svc	#0x0
   10f84:	3100101f 	cmn	w0, #0x4
   10f88:	2a0003e2 	mov	w2, w0
   10f8c:	2a0003e3 	mov	w3, w0
   10f90:	54fffea0 	b.eq	10f64 <sigwait+0x34>
   10f94:	3140047f 	cmn	w3, #0x1, lsl #12
   10f98:	4b0203e0 	neg	w0, w2
   10f9c:	54000068 	b.hi	10fa8 <sigwait+0x78>
   10fa0:	b90000a2 	str	w2, [x5]
   10fa4:	52800000 	mov	w0, #0x0                   	// #0
   10fa8:	a94153f3 	ldp	x19, x20, [sp,#16]
   10fac:	a8ca7bfd 	ldp	x29, x30, [sp],#160
   10fb0:	d65f03c0 	ret
   10fb4:	925ff442 	and	x2, x2, #0xfffffffe7fffffff
   10fb8:	910083a4 	add	x4, x29, #0x20
   10fbc:	f90013a2 	str	x2, [x29,#32]
   10fc0:	17ffffeb 	b	10f6c <sigwait+0x3c>
   10fc4:	aa0003f3 	mov	x19, x0
   10fc8:	aa0103f4 	mov	x20, x1
   10fcc:	97fff993 	bl	f618 <__pthread_enable_asynccancel>
   10fd0:	2a0003e5 	mov	w5, w0
   10fd4:	b4000093 	cbz	x19, 10fe4 <sigwait+0xb4>
   10fd8:	f9400260 	ldr	x0, [x19]
   10fdc:	f261041f 	tst	x0, #0x180000000
   10fe0:	54000341 	b.ne	11048 <sigwait+0x118>
   10fe4:	aa1303e4 	mov	x4, x19
   10fe8:	14000003 	b	10ff4 <sigwait+0xc4>
   10fec:	3140041f 	cmn	w0, #0x1, lsl #12
   10ff0:	54000169 	b.ls	1101c <sigwait+0xec>
   10ff4:	d2800001 	mov	x1, #0x0                   	// #0
   10ff8:	aa0403e0 	mov	x0, x4
   10ffc:	aa0103e2 	mov	x2, x1
   11000:	d2800103 	mov	x3, #0x8                   	// #8
   11004:	d2801128 	mov	x8, #0x89                  	// #137
   11008:	d4000001 	svc	#0x0
   1100c:	3100101f 	cmn	w0, #0x4
   11010:	2a0003e1 	mov	w1, w0
   11014:	2a0003e2 	mov	w2, w0
   11018:	54fffea0 	b.eq	10fec <sigwait+0xbc>
   1101c:	3140045f 	cmn	w2, #0x1, lsl #12
   11020:	4b0103f3 	neg	w19, w1
   11024:	54000068 	b.hi	11030 <sigwait+0x100>
   11028:	b9000281 	str	w1, [x20]
   1102c:	52800013 	mov	w19, #0x0                   	// #0
   11030:	2a0503e0 	mov	w0, w5
   11034:	97fff9a9 	bl	f6d8 <__pthread_disable_asynccancel>
   11038:	2a1303e0 	mov	w0, w19
   1103c:	a94153f3 	ldp	x19, x20, [sp,#16]
   11040:	a8ca7bfd 	ldp	x29, x30, [sp],#160
   11044:	d65f03c0 	ret
   11048:	925ff400 	and	x0, x0, #0xfffffffe7fffffff
   1104c:	910083a4 	add	x4, x29, #0x20
   11050:	f90013a0 	str	x0, [x29,#32]
   11054:	17ffffe8 	b	10ff4 <sigwait+0xc4>

0000000000011058 <__libc_sigsuspend>:
   11058:	f0000101 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
   1105c:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
   11060:	910003fd 	mov	x29, sp
   11064:	b9432821 	ldr	w1, [x1,#808]
   11068:	f9000bf3 	str	x19, [sp,#16]
   1106c:	35000161 	cbnz	w1, 11098 <__libc_sigsuspend+0x40>
   11070:	d2800101 	mov	x1, #0x8                   	// #8
   11074:	d28010a8 	mov	x8, #0x85                  	// #133
   11078:	d4000001 	svc	#0x0
   1107c:	b140041f 	cmn	x0, #0x1, lsl #12
   11080:	540002c8 	b.hi	110d8 <__libc_sigsuspend+0x80>
   11084:	2a0003f3 	mov	w19, w0
   11088:	2a1303e0 	mov	w0, w19
   1108c:	f9400bf3 	ldr	x19, [sp,#16]
   11090:	a8c27bfd 	ldp	x29, x30, [sp],#32
   11094:	d65f03c0 	ret
   11098:	aa0003f3 	mov	x19, x0
   1109c:	97fff95f 	bl	f618 <__pthread_enable_asynccancel>
   110a0:	d2800101 	mov	x1, #0x8                   	// #8
   110a4:	2a0003e2 	mov	w2, w0
   110a8:	d28010a8 	mov	x8, #0x85                  	// #133
   110ac:	aa1303e0 	mov	x0, x19
   110b0:	d4000001 	svc	#0x0
   110b4:	b140041f 	cmn	x0, #0x1, lsl #12
   110b8:	540001e8 	b.hi	110f4 <__libc_sigsuspend+0x9c>
   110bc:	2a0003f3 	mov	w19, w0
   110c0:	2a0203e0 	mov	w0, w2
   110c4:	97fff985 	bl	f6d8 <__pthread_disable_asynccancel>
   110c8:	2a1303e0 	mov	w0, w19
   110cc:	f9400bf3 	ldr	x19, [sp,#16]
   110d0:	a8c27bfd 	ldp	x29, x30, [sp],#32
   110d4:	d65f03c0 	ret
   110d8:	d53bd041 	mrs	x1, tpidr_el0
   110dc:	d00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   110e0:	f947c442 	ldr	x2, [x2,#3976]
   110e4:	4b0003e0 	neg	w0, w0
   110e8:	12800013 	mov	w19, #0xffffffff            	// #-1
   110ec:	b8226820 	str	w0, [x1,x2]
   110f0:	17ffffe6 	b	11088 <__libc_sigsuspend+0x30>
   110f4:	d53bd041 	mrs	x1, tpidr_el0
   110f8:	d00000e3 	adrp	x3, 2f000 <__FRAME_END__+0x18e30>
   110fc:	f947c463 	ldr	x3, [x3,#3976]
   11100:	4b0003e0 	neg	w0, w0
   11104:	12800013 	mov	w19, #0xffffffff            	// #-1
   11108:	b8236820 	str	w0, [x1,x3]
   1110c:	17ffffed 	b	110c0 <__libc_sigsuspend+0x68>

0000000000011110 <__sigsuspend_nocancel>:
   11110:	d2800101 	mov	x1, #0x8                   	// #8
   11114:	d28010a8 	mov	x8, #0x85                  	// #133
   11118:	d4000001 	svc	#0x0
   1111c:	b140041f 	cmn	x0, #0x1, lsl #12
   11120:	54000048 	b.hi	11128 <__sigsuspend_nocancel+0x18>
   11124:	d65f03c0 	ret
   11128:	d53bd041 	mrs	x1, tpidr_el0
   1112c:	d00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   11130:	f947c442 	ldr	x2, [x2,#3976]
   11134:	4b0003e3 	neg	w3, w0
   11138:	12800000 	mov	w0, #0xffffffff            	// #-1
   1113c:	b8226823 	str	w3, [x1,x2]
   11140:	d65f03c0 	ret

0000000000011144 <raise>:
   11144:	d53bd043 	mrs	x3, tpidr_el0
   11148:	93407c02 	sxtw	x2, w0
   1114c:	d11bc061 	sub	x1, x3, #0x6f0
   11150:	d2801068 	mov	x8, #0x83                  	// #131
   11154:	b980d420 	ldrsw	x0, [x1,#212]
   11158:	b980d021 	ldrsw	x1, [x1,#208]
   1115c:	ca80fc04 	eor	x4, x0, x0, asr #63
   11160:	cb80fc80 	sub	x0, x4, x0, asr #63
   11164:	93407c00 	sxtw	x0, w0
   11168:	d4000001 	svc	#0x0
   1116c:	b140041f 	cmn	x0, #0x1, lsl #12
   11170:	54000048 	b.hi	11178 <raise+0x34>
   11174:	d65f03c0 	ret
   11178:	d00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   1117c:	f947c421 	ldr	x1, [x1,#3976]
   11180:	4b0003e2 	neg	w2, w0
   11184:	12800000 	mov	w0, #0xffffffff            	// #-1
   11188:	b8216862 	str	w2, [x3,x1]
   1118c:	d65f03c0 	ret

0000000000011190 <system>:
   11190:	17ffcfb4 	b	5060 <__libc_system@plt>

0000000000011194 <_IO_flockfile>:
   11194:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   11198:	910003fd 	mov	x29, sp
   1119c:	f9404401 	ldr	x1, [x0,#136]
   111a0:	a90153f3 	stp	x19, x20, [sp,#16]
   111a4:	d53bd054 	mrs	x20, tpidr_el0
   111a8:	f9400422 	ldr	x2, [x1,#8]
   111ac:	d11bc294 	sub	x20, x20, #0x6f0
   111b0:	eb14005f 	cmp	x2, x20
   111b4:	540001a0 	b.eq	111e8 <_IO_flockfile+0x54>
   111b8:	aa0003f3 	mov	x19, x0
   111bc:	b9002fbf 	str	wzr, [x29,#44]
   111c0:	52800020 	mov	w0, #0x1                   	// #1
   111c4:	885ffc22 	ldaxr	w2, [x1]
   111c8:	6b1f005f 	cmp	w2, wzr
   111cc:	54000061 	b.ne	111d8 <_IO_flockfile+0x44>
   111d0:	88037c20 	stxr	w3, w0, [x1]
   111d4:	35ffff83 	cbnz	w3, 111c4 <_IO_flockfile+0x30>
   111d8:	54000141 	b.ne	11200 <_IO_flockfile+0x6c>
   111dc:	f9404660 	ldr	x0, [x19,#136]
   111e0:	aa0003e1 	mov	x1, x0
   111e4:	f9000414 	str	x20, [x0,#8]
   111e8:	b9400420 	ldr	w0, [x1,#4]
   111ec:	a94153f3 	ldp	x19, x20, [sp,#16]
   111f0:	11000400 	add	w0, w0, #0x1
   111f4:	b9000420 	str	w0, [x1,#4]
   111f8:	a8c37bfd 	ldp	x29, x30, [sp],#48
   111fc:	d65f03c0 	ret
   11200:	aa0103e0 	mov	x0, x1
   11204:	b9002fa2 	str	w2, [x29,#44]
   11208:	97fff958 	bl	f768 <__lll_lock_wait_private>
   1120c:	17fffff4 	b	111dc <_IO_flockfile+0x48>

0000000000011210 <_IO_ftrylockfile>:
   11210:	f9404402 	ldr	x2, [x0,#136]
   11214:	d53bd041 	mrs	x1, tpidr_el0
   11218:	d11bc021 	sub	x1, x1, #0x6f0
   1121c:	d10043ff 	sub	sp, sp, #0x10
   11220:	f9400443 	ldr	x3, [x2,#8]
   11224:	eb01007f 	cmp	x3, x1
   11228:	54000180 	b.eq	11258 <_IO_ftrylockfile+0x48>
   1122c:	b9000fff 	str	wzr, [sp,#12]
   11230:	52800023 	mov	w3, #0x1                   	// #1
   11234:	885ffc44 	ldaxr	w4, [x2]
   11238:	6b1f009f 	cmp	w4, wzr
   1123c:	54000061 	b.ne	11248 <_IO_ftrylockfile+0x38>
   11240:	88057c43 	stxr	w5, w3, [x2]
   11244:	35ffff85 	cbnz	w5, 11234 <_IO_ftrylockfile+0x24>
   11248:	54000140 	b.eq	11270 <_IO_ftrylockfile+0x60>
   1124c:	52800200 	mov	w0, #0x10                  	// #16
   11250:	910043ff 	add	sp, sp, #0x10
   11254:	d65f03c0 	ret
   11258:	b9400441 	ldr	w1, [x2,#4]
   1125c:	52800000 	mov	w0, #0x0                   	// #0
   11260:	910043ff 	add	sp, sp, #0x10
   11264:	11000421 	add	w1, w1, #0x1
   11268:	b9000441 	str	w1, [x2,#4]
   1126c:	d65f03c0 	ret
   11270:	f9404402 	ldr	x2, [x0,#136]
   11274:	52800000 	mov	w0, #0x0                   	// #0
   11278:	f9000441 	str	x1, [x2,#8]
   1127c:	b9000443 	str	w3, [x2,#4]
   11280:	17fffff4 	b	11250 <_IO_ftrylockfile+0x40>

0000000000011284 <_IO_funlockfile>:
   11284:	f9404400 	ldr	x0, [x0,#136]
   11288:	b9400401 	ldr	w1, [x0,#4]
   1128c:	51000421 	sub	w1, w1, #0x1
   11290:	b9000401 	str	w1, [x0,#4]
   11294:	34000041 	cbz	w1, 1129c <_IO_funlockfile+0x18>
   11298:	d65f03c0 	ret
   1129c:	f900041f 	str	xzr, [x0,#8]
   112a0:	885f7c02 	ldxr	w2, [x0]
   112a4:	8803fc01 	stlxr	w3, w1, [x0]
   112a8:	35ffffc3 	cbnz	w3, 112a0 <_IO_funlockfile+0x1c>
   112ac:	7100045f 	cmp	w2, #0x1
   112b0:	54ffff4d 	b.le	11298 <_IO_funlockfile+0x14>
   112b4:	d2801021 	mov	x1, #0x81                  	// #129
   112b8:	d2800022 	mov	x2, #0x1                   	// #1
   112bc:	d2800003 	mov	x3, #0x0                   	// #0
   112c0:	d2800c48 	mov	x8, #0x62                  	// #98
   112c4:	d4000001 	svc	#0x0
   112c8:	d65f03c0 	ret

00000000000112cc <__libc_sigaction>:
   112cc:	a9aa7bfd 	stp	x29, x30, [sp,#-352]!
   112d0:	910003fd 	mov	x29, sp
   112d4:	a9025bf5 	stp	x21, x22, [sp,#32]
   112d8:	a90153f3 	stp	x19, x20, [sp,#16]
   112dc:	2a0003f6 	mov	w22, w0
   112e0:	aa0103f4 	mov	x20, x1
   112e4:	aa0203f5 	mov	x21, x2
   112e8:	b4000581 	cbz	x1, 11398 <__libc_sigaction+0xcc>
   112ec:	9100c3b3 	add	x19, x29, #0x30
   112f0:	f8408420 	ldr	x0, [x1],#8
   112f4:	aa1303e3 	mov	x3, x19
   112f8:	d2801002 	mov	x2, #0x80                  	// #128
   112fc:	f8018460 	str	x0, [x3],#24
   11300:	aa0303e0 	mov	x0, x3
   11304:	97ffcf17 	bl	4f60 <memcpy@plt>
   11308:	b9408a81 	ldr	w1, [x20,#136]
   1130c:	93407c20 	sxtw	x0, w1
   11310:	f9000660 	str	x0, [x19,#8]
   11314:	37d003c1 	tbnz	w1, #26, 1138c <__libc_sigaction+0xc0>
   11318:	aa1503e2 	mov	x2, x21
   1131c:	b4000055 	cbz	x21, 11324 <__libc_sigaction+0x58>
   11320:	910323a2 	add	x2, x29, #0xc8
   11324:	aa1303e1 	mov	x1, x19
   11328:	93407ec0 	sxtw	x0, w22
   1132c:	d2800103 	mov	x3, #0x8                   	// #8
   11330:	d28010c8 	mov	x8, #0x86                  	// #134
   11334:	d4000001 	svc	#0x0
   11338:	b140041f 	cmn	x0, #0x1, lsl #12
   1133c:	540003e8 	b.hi	113b8 <__libc_sigaction+0xec>
   11340:	2a0003f3 	mov	w19, w0
   11344:	37f801a0 	tbnz	w0, #31, 11378 <__libc_sigaction+0xac>
   11348:	37f80180 	tbnz	w0, #31, 11378 <__libc_sigaction+0xac>
   1134c:	b4000175 	cbz	x21, 11378 <__libc_sigaction+0xac>
   11350:	aa1503e0 	mov	x0, x21
   11354:	f94067a3 	ldr	x3, [x29,#200]
   11358:	910383a1 	add	x1, x29, #0xe0
   1135c:	d2801002 	mov	x2, #0x80                  	// #128
   11360:	f8008403 	str	x3, [x0],#8
   11364:	97ffceff 	bl	4f60 <memcpy@plt>
   11368:	f9406ba0 	ldr	x0, [x29,#208]
   1136c:	b9008aa0 	str	w0, [x21,#136]
   11370:	f9406fa0 	ldr	x0, [x29,#216]
   11374:	f9004aa0 	str	x0, [x21,#144]
   11378:	2a1303e0 	mov	w0, w19
   1137c:	a94153f3 	ldp	x19, x20, [sp,#16]
   11380:	a9425bf5 	ldp	x21, x22, [sp,#32]
   11384:	a8d67bfd 	ldp	x29, x30, [sp],#352
   11388:	d65f03c0 	ret
   1138c:	f9404a80 	ldr	x0, [x20,#144]
   11390:	f9000a60 	str	x0, [x19,#16]
   11394:	17ffffe1 	b	11318 <__libc_sigaction+0x4c>
   11398:	b4000262 	cbz	x2, 113e4 <__libc_sigaction+0x118>
   1139c:	910323a2 	add	x2, x29, #0xc8
   113a0:	93407ec0 	sxtw	x0, w22
   113a4:	d2800103 	mov	x3, #0x8                   	// #8
   113a8:	d28010c8 	mov	x8, #0x86                  	// #134
   113ac:	d4000001 	svc	#0x0
   113b0:	b140041f 	cmn	x0, #0x1, lsl #12
   113b4:	54fffc69 	b.ls	11340 <__libc_sigaction+0x74>
   113b8:	4b0003e1 	neg	w1, w0
   113bc:	d00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   113c0:	f947c442 	ldr	x2, [x2,#3976]
   113c4:	d53bd040 	mrs	x0, tpidr_el0
   113c8:	12800013 	mov	w19, #0xffffffff            	// #-1
   113cc:	a9425bf5 	ldp	x21, x22, [sp,#32]
   113d0:	b8226801 	str	w1, [x0,x2]
   113d4:	2a1303e0 	mov	w0, w19
   113d8:	a94153f3 	ldp	x19, x20, [sp,#16]
   113dc:	a8d67bfd 	ldp	x29, x30, [sp],#352
   113e0:	d65f03c0 	ret
   113e4:	aa1503e2 	mov	x2, x21
   113e8:	aa1503e1 	mov	x1, x21
   113ec:	17ffffcf 	b	11328 <__libc_sigaction+0x5c>

00000000000113f0 <__sigaction>:
   113f0:	51008003 	sub	w3, w0, #0x20
   113f4:	7100047f 	cmp	w3, #0x1
   113f8:	54000049 	b.ls	11400 <__sigaction+0x10>
   113fc:	17ffffb4 	b	112cc <__libc_sigaction>
   11400:	d53bd040 	mrs	x0, tpidr_el0
   11404:	d00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   11408:	f947c421 	ldr	x1, [x1,#3976]
   1140c:	528002c2 	mov	w2, #0x16                  	// #22
   11410:	b8216802 	str	w2, [x0,x1]
   11414:	12800000 	mov	w0, #0xffffffff            	// #-1
   11418:	d65f03c0 	ret

000000000001141c <__h_errno_location>:
   1141c:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
   11420:	f00000e0 	adrp	x0, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   11424:	f9412001 	ldr	x1, [x0,#576]
   11428:	91090000 	add	x0, x0, #0x240
   1142c:	d63f0020 	blr	x1
   11430:	910003fd 	mov	x29, sp
   11434:	d53bd041 	mrs	x1, tpidr_el0
   11438:	8b000020 	add	x0, x1, x0
   1143c:	a8c17bfd 	ldp	x29, x30, [sp],#16
   11440:	d65f03c0 	ret

0000000000011444 <__res_state>:
   11444:	d00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   11448:	f947cc21 	ldr	x1, [x1,#3992]
   1144c:	d53bd040 	mrs	x0, tpidr_el0
   11450:	f8616800 	ldr	x0, [x0,x1]
   11454:	d65f03c0 	ret

0000000000011458 <__libc_current_sigrtmin>:
   11458:	17ffcf92 	b	52a0 <__libc_current_sigrtmin_private@plt>

000000000001145c <__libc_current_sigrtmax>:
   1145c:	17ffcf4d 	b	5190 <__libc_current_sigrtmax_private@plt>

0000000000011460 <__libc_allocate_rtsig>:
   11460:	17ffcfcc 	b	5390 <__libc_allocate_rtsig_private@plt>

0000000000011464 <pthread_kill_other_threads_np@GLIBC_2.17>:
   11464:	d65f03c0 	ret

0000000000011468 <pthread_getaffinity_np@@GLIBC_2.17>:
   11468:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
   1146c:	aa0103e4 	mov	x4, x1
   11470:	b2407be1 	mov	x1, #0x7fffffff            	// #2147483647
   11474:	eb01009f 	cmp	x4, x1
   11478:	910003fd 	mov	x29, sp
   1147c:	aa0203e5 	mov	x5, x2
   11480:	9a819081 	csel	x1, x4, x1, ls
   11484:	b980d000 	ldrsw	x0, [x0,#208]
   11488:	d2800f68 	mov	x8, #0x7b                  	// #123
   1148c:	d4000001 	svc	#0x0
   11490:	3140041f 	cmn	w0, #0x1, lsl #12
   11494:	54000089 	b.ls	114a4 <pthread_getaffinity_np@@GLIBC_2.17+0x3c>
   11498:	4b0003e0 	neg	w0, w0
   1149c:	a8c17bfd 	ldp	x29, x30, [sp],#16
   114a0:	d65f03c0 	ret
   114a4:	93407c02 	sxtw	x2, w0
   114a8:	52800001 	mov	w1, #0x0                   	// #0
   114ac:	8b0200a0 	add	x0, x5, x2
   114b0:	cb020082 	sub	x2, x4, x2
   114b4:	97ffcef7 	bl	5090 <memset@plt>
   114b8:	52800000 	mov	w0, #0x0                   	// #0
   114bc:	a8c17bfd 	ldp	x29, x30, [sp],#16
   114c0:	d65f03c0 	ret

00000000000114c4 <__determine_cpumask_size>:
   114c4:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
   114c8:	93407c06 	sxtw	x6, w0
   114cc:	d2801004 	mov	x4, #0x80                  	// #128
   114d0:	910003fd 	mov	x29, sp
   114d4:	d10243ff 	sub	sp, sp, #0x90
   114d8:	910003e5 	mov	x5, sp
   114dc:	1400000b 	b	11508 <__determine_cpumask_size+0x44>
   114e0:	927cec63 	and	x3, x3, #0xfffffffffffffff0
   114e4:	340002a1 	cbz	w1, 11538 <__determine_cpumask_size+0x74>
   114e8:	91004061 	add	x1, x3, #0x10
   114ec:	910003e0 	mov	x0, sp
   114f0:	cb21601f 	sub	sp, x0, x1
   114f4:	8b040064 	add	x4, x3, x4
   114f8:	8b2363e2 	add	x2, sp, x3
   114fc:	eb0200bf 	cmp	x5, x2
   11500:	910003e5 	mov	x5, sp
   11504:	9a830084 	csel	x4, x4, x3, eq
   11508:	aa0603e0 	mov	x0, x6
   1150c:	aa0403e1 	mov	x1, x4
   11510:	aa0503e2 	mov	x2, x5
   11514:	d2800f68 	mov	x8, #0x7b                  	// #123
   11518:	d4000001 	svc	#0x0
   1151c:	3140041f 	cmn	w0, #0x1, lsl #12
   11520:	d37ff883 	lsl	x3, x4, #1
   11524:	1a9f97e1 	cset	w1, hi
   11528:	3100581f 	cmn	w0, #0x16
   1152c:	93407c02 	sxtw	x2, w0
   11530:	91003c63 	add	x3, x3, #0xf
   11534:	54fffd60 	b.eq	114e0 <__determine_cpumask_size+0x1c>
   11538:	34000102 	cbz	w2, 11558 <__determine_cpumask_size+0x94>
   1153c:	350000e1 	cbnz	w1, 11558 <__determine_cpumask_size+0x94>
   11540:	2a0103e0 	mov	w0, w1
   11544:	f0000101 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
   11548:	910003bf 	mov	sp, x29
   1154c:	f901d422 	str	x2, [x1,#936]
   11550:	a8c17bfd 	ldp	x29, x30, [sp],#16
   11554:	d65f03c0 	ret
   11558:	910003bf 	mov	sp, x29
   1155c:	4b0203e0 	neg	w0, w2
   11560:	a8c17bfd 	ldp	x29, x30, [sp],#16
   11564:	d65f03c0 	ret

0000000000011568 <pthread_setaffinity_np@@GLIBC_2.17>:
   11568:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   1156c:	910003fd 	mov	x29, sp
   11570:	a90153f3 	stp	x19, x20, [sp,#16]
   11574:	f0000113 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
   11578:	aa0003f4 	mov	x20, x0
   1157c:	f941d663 	ldr	x3, [x19,#936]
   11580:	b40002a3 	cbz	x3, 115d4 <pthread_setaffinity_np@@GLIBC_2.17+0x6c>
   11584:	eb01007f 	cmp	x3, x1
   11588:	540000a3 	b.cc	1159c <pthread_setaffinity_np@@GLIBC_2.17+0x34>
   1158c:	1400000a 	b	115b4 <pthread_setaffinity_np@@GLIBC_2.17+0x4c>
   11590:	91000463 	add	x3, x3, #0x1
   11594:	eb03003f 	cmp	x1, x3
   11598:	540000e9 	b.ls	115b4 <pthread_setaffinity_np@@GLIBC_2.17+0x4c>
   1159c:	38636844 	ldrb	w4, [x2,x3]
   115a0:	34ffff84 	cbz	w4, 11590 <pthread_setaffinity_np@@GLIBC_2.17+0x28>
   115a4:	528002c0 	mov	w0, #0x16                  	// #22
   115a8:	a94153f3 	ldp	x19, x20, [sp,#16]
   115ac:	a8c37bfd 	ldp	x29, x30, [sp],#48
   115b0:	d65f03c0 	ret
   115b4:	b980d280 	ldrsw	x0, [x20,#208]
   115b8:	d2800f48 	mov	x8, #0x7a                  	// #122
   115bc:	d4000001 	svc	#0x0
   115c0:	3140041f 	cmn	w0, #0x1, lsl #12
   115c4:	5a8097e0 	csneg	w0, wzr, w0, ls
   115c8:	a94153f3 	ldp	x19, x20, [sp,#16]
   115cc:	a8c37bfd 	ldp	x29, x30, [sp],#48
   115d0:	d65f03c0 	ret
   115d4:	b940d000 	ldr	w0, [x0,#208]
   115d8:	f90013a2 	str	x2, [x29,#32]
   115dc:	f90017a1 	str	x1, [x29,#40]
   115e0:	97ffffb9 	bl	114c4 <__determine_cpumask_size>
   115e4:	35fffe20 	cbnz	w0, 115a8 <pthread_setaffinity_np@@GLIBC_2.17+0x40>
   115e8:	f941d663 	ldr	x3, [x19,#936]
   115ec:	f94013a2 	ldr	x2, [x29,#32]
   115f0:	f94017a1 	ldr	x1, [x29,#40]
   115f4:	17ffffe4 	b	11584 <pthread_setaffinity_np@@GLIBC_2.17+0x1c>

00000000000115f8 <pthread_attr_getaffinity_np@@GLIBC_2.17>:
   115f8:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   115fc:	910003fd 	mov	x29, sp
   11600:	f9401405 	ldr	x5, [x0,#40]
   11604:	a90153f3 	stp	x19, x20, [sp,#16]
   11608:	f90013f5 	str	x21, [sp,#32]
   1160c:	aa0103f4 	mov	x20, x1
   11610:	aa0003f5 	mov	x21, x0
   11614:	b4000525 	cbz	x5, 116b8 <pthread_attr_getaffinity_np@@GLIBC_2.17+0xc0>
   11618:	f9401806 	ldr	x6, [x0,#48]
   1161c:	eb06003f 	cmp	x1, x6
   11620:	54000142 	b.cs	11648 <pthread_attr_getaffinity_np@@GLIBC_2.17+0x50>
   11624:	386168a1 	ldrb	w1, [x5,x1]
   11628:	350002c1 	cbnz	w1, 11680 <pthread_attr_getaffinity_np@@GLIBC_2.17+0x88>
   1162c:	aa1403e3 	mov	x3, x20
   11630:	14000003 	b	1163c <pthread_attr_getaffinity_np@@GLIBC_2.17+0x44>
   11634:	386368a4 	ldrb	w4, [x5,x3]
   11638:	35000244 	cbnz	w4, 11680 <pthread_attr_getaffinity_np@@GLIBC_2.17+0x88>
   1163c:	91000463 	add	x3, x3, #0x1
   11640:	eb06007f 	cmp	x3, x6
   11644:	54ffff83 	b.cc	11634 <pthread_attr_getaffinity_np@@GLIBC_2.17+0x3c>
   11648:	eb06029f 	cmp	x20, x6
   1164c:	aa0203e0 	mov	x0, x2
   11650:	aa0503e1 	mov	x1, x5
   11654:	9a869282 	csel	x2, x20, x6, ls
   11658:	52800013 	mov	w19, #0x0                   	// #0
   1165c:	97ffcf01 	bl	5260 <mempcpy@plt>
   11660:	f9401aa2 	ldr	x2, [x21,#48]
   11664:	eb02029f 	cmp	x20, x2
   11668:	54000188 	b.hi	11698 <pthread_attr_getaffinity_np@@GLIBC_2.17+0xa0>
   1166c:	2a1303e0 	mov	w0, w19
   11670:	f94013f5 	ldr	x21, [sp,#32]
   11674:	a94153f3 	ldp	x19, x20, [sp,#16]
   11678:	a8c37bfd 	ldp	x29, x30, [sp],#48
   1167c:	d65f03c0 	ret
   11680:	528002d3 	mov	w19, #0x16                  	// #22
   11684:	f94013f5 	ldr	x21, [sp,#32]
   11688:	2a1303e0 	mov	w0, w19
   1168c:	a94153f3 	ldp	x19, x20, [sp,#16]
   11690:	a8c37bfd 	ldp	x29, x30, [sp],#48
   11694:	d65f03c0 	ret
   11698:	2a1303e1 	mov	w1, w19
   1169c:	cb020282 	sub	x2, x20, x2
   116a0:	97ffce7c 	bl	5090 <memset@plt>
   116a4:	f94013f5 	ldr	x21, [sp,#32]
   116a8:	2a1303e0 	mov	w0, w19
   116ac:	a94153f3 	ldp	x19, x20, [sp,#16]
   116b0:	a8c37bfd 	ldp	x29, x30, [sp],#48
   116b4:	d65f03c0 	ret
   116b8:	aa0203e0 	mov	x0, x2
   116bc:	12800001 	mov	w1, #0xffffffff            	// #-1
   116c0:	aa1403e2 	mov	x2, x20
   116c4:	2a0503f3 	mov	w19, w5
   116c8:	97ffce72 	bl	5090 <memset@plt>
   116cc:	17ffffe8 	b	1166c <pthread_attr_getaffinity_np@@GLIBC_2.17+0x74>

00000000000116d0 <pthread_attr_setaffinity_np@@GLIBC_2.17>:
   116d0:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
   116d4:	910003fd 	mov	x29, sp
   116d8:	a90153f3 	stp	x19, x20, [sp,#16]
   116dc:	f90013f5 	str	x21, [sp,#32]
   116e0:	aa0003f4 	mov	x20, x0
   116e4:	aa0103f3 	mov	x19, x1
   116e8:	b40004c2 	cbz	x2, 11780 <pthread_attr_setaffinity_np@@GLIBC_2.17+0xb0>
   116ec:	b40004a1 	cbz	x1, 11780 <pthread_attr_setaffinity_np@@GLIBC_2.17+0xb0>
   116f0:	f0000115 	adrp	x21, 34000 <__GI___pthread_keys+0x3d78>
   116f4:	f941d6a3 	ldr	x3, [x21,#936]
   116f8:	b4000563 	cbz	x3, 117a4 <pthread_attr_setaffinity_np@@GLIBC_2.17+0xd4>
   116fc:	eb03027f 	cmp	x19, x3
   11700:	540001a9 	b.ls	11734 <pthread_attr_setaffinity_np@@GLIBC_2.17+0x64>
   11704:	38636840 	ldrb	w0, [x2,x3]
   11708:	34000100 	cbz	w0, 11728 <pthread_attr_setaffinity_np@@GLIBC_2.17+0x58>
   1170c:	528002c0 	mov	w0, #0x16                  	// #22
   11710:	a94153f3 	ldp	x19, x20, [sp,#16]
   11714:	f94013f5 	ldr	x21, [sp,#32]
   11718:	a8c47bfd 	ldp	x29, x30, [sp],#64
   1171c:	d65f03c0 	ret
   11720:	38636844 	ldrb	w4, [x2,x3]
   11724:	35ffff44 	cbnz	w4, 1170c <pthread_attr_setaffinity_np@@GLIBC_2.17+0x3c>
   11728:	91000463 	add	x3, x3, #0x1
   1172c:	eb03027f 	cmp	x19, x3
   11730:	54ffff88 	b.hi	11720 <pthread_attr_setaffinity_np@@GLIBC_2.17+0x50>
   11734:	f9401a80 	ldr	x0, [x20,#48]
   11738:	eb13001f 	cmp	x0, x19
   1173c:	54000460 	b.eq	117c8 <pthread_attr_setaffinity_np@@GLIBC_2.17+0xf8>
   11740:	f9401680 	ldr	x0, [x20,#40]
   11744:	aa1303e1 	mov	x1, x19
   11748:	f9001fa2 	str	x2, [x29,#56]
   1174c:	97ffce71 	bl	5110 <realloc@plt>
   11750:	b4000400 	cbz	x0, 117d0 <pthread_attr_setaffinity_np@@GLIBC_2.17+0x100>
   11754:	f9401fa2 	ldr	x2, [x29,#56]
   11758:	f9001680 	str	x0, [x20,#40]
   1175c:	f9001a93 	str	x19, [x20,#48]
   11760:	aa0203e1 	mov	x1, x2
   11764:	aa1303e2 	mov	x2, x19
   11768:	97ffcdfe 	bl	4f60 <memcpy@plt>
   1176c:	f94013f5 	ldr	x21, [sp,#32]
   11770:	52800000 	mov	w0, #0x0                   	// #0
   11774:	a94153f3 	ldp	x19, x20, [sp,#16]
   11778:	a8c47bfd 	ldp	x29, x30, [sp],#64
   1177c:	d65f03c0 	ret
   11780:	f9401680 	ldr	x0, [x20,#40]
   11784:	97ffcea7 	bl	5220 <free@plt>
   11788:	f900169f 	str	xzr, [x20,#40]
   1178c:	f9001a9f 	str	xzr, [x20,#48]
   11790:	52800000 	mov	w0, #0x0                   	// #0
   11794:	a94153f3 	ldp	x19, x20, [sp,#16]
   11798:	f94013f5 	ldr	x21, [sp,#32]
   1179c:	a8c47bfd 	ldp	x29, x30, [sp],#64
   117a0:	d65f03c0 	ret
   117a4:	d53bd040 	mrs	x0, tpidr_el0
   117a8:	f9001fa2 	str	x2, [x29,#56]
   117ac:	d11bc000 	sub	x0, x0, #0x6f0
   117b0:	b940d000 	ldr	w0, [x0,#208]
   117b4:	97ffff44 	bl	114c4 <__determine_cpumask_size>
   117b8:	35fffac0 	cbnz	w0, 11710 <pthread_attr_setaffinity_np@@GLIBC_2.17+0x40>
   117bc:	f941d6a3 	ldr	x3, [x21,#936]
   117c0:	f9401fa2 	ldr	x2, [x29,#56]
   117c4:	17ffffce 	b	116fc <pthread_attr_setaffinity_np@@GLIBC_2.17+0x2c>
   117c8:	f9401680 	ldr	x0, [x20,#40]
   117cc:	17ffffe5 	b	11760 <pthread_attr_setaffinity_np@@GLIBC_2.17+0x90>
   117d0:	52800180 	mov	w0, #0xc                   	// #12
   117d4:	17ffffcf 	b	11710 <pthread_attr_setaffinity_np@@GLIBC_2.17+0x40>

00000000000117d8 <pthread_mutexattr_getrobust>:
   117d8:	b9400002 	ldr	w2, [x0]
   117dc:	52800000 	mov	w0, #0x0                   	// #0
   117e0:	d35e7842 	ubfx	x2, x2, #30, #1
   117e4:	b9000022 	str	w2, [x1]
   117e8:	d65f03c0 	ret

00000000000117ec <pthread_mutexattr_setrobust>:
   117ec:	350000c1 	cbnz	w1, 11804 <pthread_mutexattr_setrobust+0x18>
   117f0:	b9400002 	ldr	w2, [x0]
   117f4:	12017842 	and	w2, w2, #0xbfffffff
   117f8:	b9000002 	str	w2, [x0]
   117fc:	2a0103e0 	mov	w0, w1
   11800:	d65f03c0 	ret
   11804:	7100043f 	cmp	w1, #0x1
   11808:	540000e1 	b.ne	11824 <pthread_mutexattr_setrobust+0x38>
   1180c:	b9400002 	ldr	w2, [x0]
   11810:	52800001 	mov	w1, #0x0                   	// #0
   11814:	32020042 	orr	w2, w2, #0x40000000
   11818:	b9000002 	str	w2, [x0]
   1181c:	2a0103e0 	mov	w0, w1
   11820:	d65f03c0 	ret
   11824:	528002c1 	mov	w1, #0x16                  	// #22
   11828:	17fffff5 	b	117fc <pthread_mutexattr_setrobust+0x10>

000000000001182c <pthread_mutex_consistent>:
   1182c:	b9401002 	ldr	w2, [x0,#16]
   11830:	528002c1 	mov	w1, #0x16                  	// #22
   11834:	37200062 	tbnz	w2, #4, 11840 <pthread_mutex_consistent+0x14>
   11838:	2a0103e0 	mov	w0, w1
   1183c:	d65f03c0 	ret
   11840:	b9400803 	ldr	w3, [x0,#8]
   11844:	12b00002 	mov	w2, #0x7fffffff            	// #2147483647
   11848:	6b02007f 	cmp	w3, w2
   1184c:	54ffff61 	b.ne	11838 <pthread_mutex_consistent+0xc>
   11850:	d53bd042 	mrs	x2, tpidr_el0
   11854:	52800001 	mov	w1, #0x0                   	// #0
   11858:	d11bc042 	sub	x2, x2, #0x6f0
   1185c:	b940d042 	ldr	w2, [x2,#208]
   11860:	b9000802 	str	w2, [x0,#8]
   11864:	17fffff5 	b	11838 <pthread_mutex_consistent+0xc>

0000000000011868 <__pthread_cleanup_routine>:
   11868:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
   1186c:	910003fd 	mov	x29, sp
   11870:	b9401001 	ldr	w1, [x0,#16]
   11874:	34000081 	cbz	w1, 11884 <__pthread_cleanup_routine+0x1c>
   11878:	f9400001 	ldr	x1, [x0]
   1187c:	f9400400 	ldr	x0, [x0,#8]
   11880:	d63f0020 	blr	x1
   11884:	a8c17bfd 	ldp	x29, x30, [sp],#16
   11888:	d65f03c0 	ret

000000000001188c <pthread_cancel_init>:
   1188c:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
   11890:	910003fd 	mov	x29, sp
   11894:	a90153f3 	stp	x19, x20, [sp,#16]
   11898:	f0000113 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
   1189c:	a9025bf5 	stp	x21, x22, [sp,#32]
   118a0:	a90363f7 	stp	x23, x24, [sp,#48]
   118a4:	f9417660 	ldr	x0, [x19,#744]
   118a8:	910ba274 	add	x20, x19, #0x2e8
   118ac:	b40000c0 	cbz	x0, 118c4 <pthread_cancel_init+0x38>
   118b0:	a94153f3 	ldp	x19, x20, [sp,#16]
   118b4:	a9425bf5 	ldp	x21, x22, [sp,#32]
   118b8:	a94363f7 	ldp	x23, x24, [sp,#48]
   118bc:	a8c47bfd 	ldp	x29, x30, [sp],#64
   118c0:	d65f03c0 	ret
   118c4:	b0000000 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
   118c8:	320107e1 	mov	w1, #0x80000001            	// #-2147483647
   118cc:	91354000 	add	x0, x0, #0xd50
   118d0:	97ffce38 	bl	51b0 <__libc_dlopen_mode@plt>
   118d4:	aa0003f5 	mov	x21, x0
   118d8:	b40004a0 	cbz	x0, 1196c <pthread_cancel_init+0xe0>
   118dc:	b0000001 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
   118e0:	91368021 	add	x1, x1, #0xda0
   118e4:	97ffce67 	bl	5280 <__libc_dlsym@plt>
   118e8:	aa0003f6 	mov	x22, x0
   118ec:	b4000400 	cbz	x0, 1196c <pthread_cancel_init+0xe0>
   118f0:	b0000001 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
   118f4:	aa1503e0 	mov	x0, x21
   118f8:	9136c021 	add	x1, x1, #0xdb0
   118fc:	97ffce61 	bl	5280 <__libc_dlsym@plt>
   11900:	aa0003f8 	mov	x24, x0
   11904:	b4000340 	cbz	x0, 1196c <pthread_cancel_init+0xe0>
   11908:	b0000001 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
   1190c:	aa1503e0 	mov	x0, x21
   11910:	91372021 	add	x1, x1, #0xdc8
   11914:	97ffce5b 	bl	5280 <__libc_dlsym@plt>
   11918:	aa0003f7 	mov	x23, x0
   1191c:	b4000280 	cbz	x0, 1196c <pthread_cancel_init+0xe0>
   11920:	b0000001 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
   11924:	aa1503e0 	mov	x0, x21
   11928:	91378021 	add	x1, x1, #0xde0
   1192c:	97ffce55 	bl	5280 <__libc_dlsym@plt>
   11930:	b40001e0 	cbz	x0, 1196c <pthread_cancel_init+0xe0>
   11934:	d00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   11938:	f947d042 	ldr	x2, [x2,#4000]
   1193c:	f9400041 	ldr	x1, [x2]
   11940:	ca0102d6 	eor	x22, x22, x1
   11944:	ca010318 	eor	x24, x24, x1
   11948:	ca0102f7 	eor	x23, x23, x1
   1194c:	f9000696 	str	x22, [x20,#8]
   11950:	f9000a98 	str	x24, [x20,#16]
   11954:	ca010001 	eor	x1, x0, x1
   11958:	f9000e97 	str	x23, [x20,#24]
   1195c:	f9001281 	str	x1, [x20,#32]
   11960:	d5033bbf 	dmb	ish
   11964:	f9017675 	str	x21, [x19,#744]
   11968:	17ffffd2 	b	118b0 <pthread_cancel_init+0x24>
   1196c:	b0000000 	adrp	x0, 12000 <__pthread_current_priority+0xa8>
   11970:	91358000 	add	x0, x0, #0xd60
   11974:	97ffcd9f 	bl	4ff0 <__libc_fatal@plt>

0000000000011978 <_Unwind_Resume>:
   11978:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   1197c:	910003fd 	mov	x29, sp
   11980:	f9000bf3 	str	x19, [sp,#16]
   11984:	f0000113 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
   11988:	f9417661 	ldr	x1, [x19,#744]
   1198c:	b4000181 	cbz	x1, 119bc <_Unwind_Resume+0x44>
   11990:	d5033bbf 	dmb	ish
   11994:	d00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   11998:	910ba273 	add	x19, x19, #0x2e8
   1199c:	f947d042 	ldr	x2, [x2,#4000]
   119a0:	f9400663 	ldr	x3, [x19,#8]
   119a4:	f9400041 	ldr	x1, [x2]
   119a8:	ca010061 	eor	x1, x3, x1
   119ac:	d63f0020 	blr	x1
   119b0:	f9400bf3 	ldr	x19, [sp,#16]
   119b4:	a8c37bfd 	ldp	x29, x30, [sp],#48
   119b8:	d65f03c0 	ret
   119bc:	f90017a0 	str	x0, [x29,#40]
   119c0:	97ffffb3 	bl	1188c <pthread_cancel_init>
   119c4:	f94017a0 	ldr	x0, [x29,#40]
   119c8:	17fffff3 	b	11994 <_Unwind_Resume+0x1c>

00000000000119cc <__gcc_personality_v0>:
   119cc:	a9bb7bfd 	stp	x29, x30, [sp,#-80]!
   119d0:	910003fd 	mov	x29, sp
   119d4:	f9000bf3 	str	x19, [sp,#16]
   119d8:	f0000113 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
   119dc:	f9417665 	ldr	x5, [x19,#744]
   119e0:	b4000185 	cbz	x5, 11a10 <__gcc_personality_v0+0x44>
   119e4:	d5033bbf 	dmb	ish
   119e8:	d00000e6 	adrp	x6, 2f000 <__FRAME_END__+0x18e30>
   119ec:	910ba273 	add	x19, x19, #0x2e8
   119f0:	f947d0c6 	ldr	x6, [x6,#4000]
   119f4:	f9400a67 	ldr	x7, [x19,#16]
   119f8:	f94000c5 	ldr	x5, [x6]
   119fc:	ca0500e5 	eor	x5, x7, x5
   11a00:	d63f00a0 	blr	x5
   11a04:	f9400bf3 	ldr	x19, [sp,#16]
   11a08:	a8c57bfd 	ldp	x29, x30, [sp],#80
   11a0c:	d65f03c0 	ret
   11a10:	f90017a4 	str	x4, [x29,#40]
   11a14:	f9001ba3 	str	x3, [x29,#48]
   11a18:	f9001fa2 	str	x2, [x29,#56]
   11a1c:	f90023a1 	str	x1, [x29,#64]
   11a20:	f90027a0 	str	x0, [x29,#72]
   11a24:	97ffff9a 	bl	1188c <pthread_cancel_init>
   11a28:	f94027a0 	ldr	x0, [x29,#72]
   11a2c:	f94023a1 	ldr	x1, [x29,#64]
   11a30:	f9401fa2 	ldr	x2, [x29,#56]
   11a34:	f9401ba3 	ldr	x3, [x29,#48]
   11a38:	f94017a4 	ldr	x4, [x29,#40]
   11a3c:	17ffffeb 	b	119e8 <__gcc_personality_v0+0x1c>

0000000000011a40 <_Unwind_ForcedUnwind>:
   11a40:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
   11a44:	910003fd 	mov	x29, sp
   11a48:	f9000bf3 	str	x19, [sp,#16]
   11a4c:	f0000113 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
   11a50:	f9417663 	ldr	x3, [x19,#744]
   11a54:	b4000183 	cbz	x3, 11a84 <_Unwind_ForcedUnwind+0x44>
   11a58:	d5033bbf 	dmb	ish
   11a5c:	d00000e4 	adrp	x4, 2f000 <__FRAME_END__+0x18e30>
   11a60:	910ba273 	add	x19, x19, #0x2e8
   11a64:	f947d084 	ldr	x4, [x4,#4000]
   11a68:	f9400e65 	ldr	x5, [x19,#24]
   11a6c:	f9400083 	ldr	x3, [x4]
   11a70:	ca0300a3 	eor	x3, x5, x3
   11a74:	d63f0060 	blr	x3
   11a78:	f9400bf3 	ldr	x19, [sp,#16]
   11a7c:	a8c47bfd 	ldp	x29, x30, [sp],#64
   11a80:	d65f03c0 	ret
   11a84:	f90017a2 	str	x2, [x29,#40]
   11a88:	f9001ba1 	str	x1, [x29,#48]
   11a8c:	f9001fa0 	str	x0, [x29,#56]
   11a90:	97ffff7f 	bl	1188c <pthread_cancel_init>
   11a94:	f9401fa0 	ldr	x0, [x29,#56]
   11a98:	f9401ba1 	ldr	x1, [x29,#48]
   11a9c:	f94017a2 	ldr	x2, [x29,#40]
   11aa0:	17ffffef 	b	11a5c <_Unwind_ForcedUnwind+0x1c>

0000000000011aa4 <_Unwind_GetCFA>:
   11aa4:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   11aa8:	910003fd 	mov	x29, sp
   11aac:	f9000bf3 	str	x19, [sp,#16]
   11ab0:	f0000113 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
   11ab4:	f9417661 	ldr	x1, [x19,#744]
   11ab8:	b4000181 	cbz	x1, 11ae8 <_Unwind_GetCFA+0x44>
   11abc:	d5033bbf 	dmb	ish
   11ac0:	d00000e2 	adrp	x2, 2f000 <__FRAME_END__+0x18e30>
   11ac4:	910ba273 	add	x19, x19, #0x2e8
   11ac8:	f947d042 	ldr	x2, [x2,#4000]
   11acc:	f9401263 	ldr	x3, [x19,#32]
   11ad0:	f9400041 	ldr	x1, [x2]
   11ad4:	ca010061 	eor	x1, x3, x1
   11ad8:	d63f0020 	blr	x1
   11adc:	f9400bf3 	ldr	x19, [sp,#16]
   11ae0:	a8c37bfd 	ldp	x29, x30, [sp],#48
   11ae4:	d65f03c0 	ret
   11ae8:	f90017a0 	str	x0, [x29,#40]
   11aec:	97ffff68 	bl	1188c <pthread_cancel_init>
   11af0:	f94017a0 	ldr	x0, [x29,#40]
   11af4:	17fffff3 	b	11ac0 <_Unwind_GetCFA+0x1c>

0000000000011af8 <pthread_mutexattr_getprotocol>:
   11af8:	b9400002 	ldr	w2, [x0]
   11afc:	52800000 	mov	w0, #0x0                   	// #0
   11b00:	d35c7442 	ubfx	x2, x2, #28, #2
   11b04:	b9000022 	str	w2, [x1]
   11b08:	d65f03c0 	ret

0000000000011b0c <pthread_mutexattr_setprotocol>:
   11b0c:	7100083f 	cmp	w1, #0x2
   11b10:	54000108 	b.hi	11b30 <pthread_mutexattr_setprotocol+0x24>
   11b14:	b9400002 	ldr	w2, [x0]
   11b18:	52800003 	mov	w3, #0x0                   	// #0
   11b1c:	12027442 	and	w2, w2, #0xcfffffff
   11b20:	2a017041 	orr	w1, w2, w1, lsl #28
   11b24:	b9000001 	str	w1, [x0]
   11b28:	2a0303e0 	mov	w0, w3
   11b2c:	d65f03c0 	ret
   11b30:	528002c3 	mov	w3, #0x16                  	// #22
   11b34:	2a0303e0 	mov	w0, w3
   11b38:	d65f03c0 	ret

0000000000011b3c <pthread_mutexattr_getprioceiling>:
   11b3c:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   11b40:	910003fd 	mov	x29, sp
   11b44:	b9400000 	ldr	w0, [x0]
   11b48:	f9000bf3 	str	x19, [sp,#16]
   11b4c:	d34c5c00 	ubfx	x0, x0, #12, #12
   11b50:	350000e0 	cbnz	w0, 11b6c <pthread_mutexattr_getprioceiling+0x30>
   11b54:	f00000f3 	adrp	x19, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   11b58:	b9427e60 	ldr	w0, [x19,#636]
   11b5c:	3100041f 	cmn	w0, #0x1
   11b60:	54000100 	b.eq	11b80 <pthread_mutexattr_getprioceiling+0x44>
   11b64:	6b1f001f 	cmp	w0, wzr
   11b68:	1a9fa000 	csel	w0, w0, wzr, ge
   11b6c:	b9000020 	str	w0, [x1]
   11b70:	52800000 	mov	w0, #0x0                   	// #0
   11b74:	f9400bf3 	ldr	x19, [sp,#16]
   11b78:	a8c37bfd 	ldp	x29, x30, [sp],#48
   11b7c:	d65f03c0 	ret
   11b80:	f90017a1 	str	x1, [x29,#40]
   11b84:	94000026 	bl	11c1c <__init_sched_fifo_prio>
   11b88:	b9427e60 	ldr	w0, [x19,#636]
   11b8c:	f94017a1 	ldr	x1, [x29,#40]
   11b90:	17fffff5 	b	11b64 <pthread_mutexattr_getprioceiling+0x28>

0000000000011b94 <pthread_mutexattr_setprioceiling>:
   11b94:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   11b98:	910003fd 	mov	x29, sp
   11b9c:	a90153f3 	stp	x19, x20, [sp,#16]
   11ba0:	f00000f4 	adrp	x20, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   11ba4:	aa0003f3 	mov	x19, x0
   11ba8:	b9427e82 	ldr	w2, [x20,#636]
   11bac:	3100045f 	cmn	w2, #0x1
   11bb0:	540002c0 	b.eq	11c08 <pthread_mutexattr_setprioceiling+0x74>
   11bb4:	6b02003f 	cmp	w1, w2
   11bb8:	5400020b 	b.lt	11bf8 <pthread_mutexattr_setprioceiling+0x64>
   11bbc:	f00000e0 	adrp	x0, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   11bc0:	b9427800 	ldr	w0, [x0,#632]
   11bc4:	6b00003f 	cmp	w1, w0
   11bc8:	5400018c 	b.gt	11bf8 <pthread_mutexattr_setprioceiling+0x64>
   11bcc:	12002c20 	and	w0, w1, #0xfff
   11bd0:	6b00003f 	cmp	w1, w0
   11bd4:	54000121 	b.ne	11bf8 <pthread_mutexattr_setprioceiling+0x64>
   11bd8:	b9400262 	ldr	w2, [x19]
   11bdc:	52800000 	mov	w0, #0x0                   	// #0
   11be0:	12084c42 	and	w2, w2, #0xff000fff
   11be4:	2a013041 	orr	w1, w2, w1, lsl #12
   11be8:	b9000261 	str	w1, [x19]
   11bec:	a94153f3 	ldp	x19, x20, [sp,#16]
   11bf0:	a8c37bfd 	ldp	x29, x30, [sp],#48
   11bf4:	d65f03c0 	ret
   11bf8:	528002c0 	mov	w0, #0x16                  	// #22
   11bfc:	a94153f3 	ldp	x19, x20, [sp,#16]
   11c00:	a8c37bfd 	ldp	x29, x30, [sp],#48
   11c04:	d65f03c0 	ret
   11c08:	f90017a1 	str	x1, [x29,#40]
   11c0c:	94000004 	bl	11c1c <__init_sched_fifo_prio>
   11c10:	b9427e82 	ldr	w2, [x20,#636]
   11c14:	f94017a1 	ldr	x1, [x29,#40]
   11c18:	17ffffe7 	b	11bb4 <pthread_mutexattr_setprioceiling+0x20>

0000000000011c1c <__init_sched_fifo_prio>:
   11c1c:	a9be7bfd 	stp	x29, x30, [sp,#-32]!
   11c20:	52800020 	mov	w0, #0x1                   	// #1
   11c24:	910003fd 	mov	x29, sp
   11c28:	f9000bf3 	str	x19, [sp,#16]
   11c2c:	97ffcd45 	bl	5140 <sched_get_priority_max@plt>
   11c30:	f00000e1 	adrp	x1, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   11c34:	9109e033 	add	x19, x1, #0x278
   11c38:	b9027820 	str	w0, [x1,#632]
   11c3c:	52800020 	mov	w0, #0x1                   	// #1
   11c40:	d5033bbf 	dmb	ish
   11c44:	97ffcd7b 	bl	5230 <sched_get_priority_min@plt>
   11c48:	b9000660 	str	w0, [x19,#4]
   11c4c:	f9400bf3 	ldr	x19, [sp,#16]
   11c50:	a8c27bfd 	ldp	x29, x30, [sp],#32
   11c54:	d65f03c0 	ret

0000000000011c58 <__pthread_tpp_change_priority>:
   11c58:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
   11c5c:	910003fd 	mov	x29, sp
   11c60:	a90363f7 	stp	x23, x24, [sp,#48]
   11c64:	d53bd058 	mrs	x24, tpidr_el0
   11c68:	f90023f9 	str	x25, [sp,#64]
   11c6c:	a9025bf5 	stp	x21, x22, [sp,#32]
   11c70:	d11bc315 	sub	x21, x24, #0x6f0
   11c74:	2a0003f6 	mov	w22, w0
   11c78:	a90153f3 	stp	x19, x20, [sp,#16]
   11c7c:	2a0103f3 	mov	w19, w1
   11c80:	f9425ab4 	ldr	x20, [x21,#1200]
   11c84:	b4000df4 	cbz	x20, 11e40 <__pthread_tpp_change_priority+0x1e8>
   11c88:	b9400297 	ldr	w23, [x20]
   11c8c:	3100067f 	cmn	w19, #0x1
   11c90:	54000640 	b.eq	11d58 <__pthread_tpp_change_priority+0x100>
   11c94:	f00000e0 	adrp	x0, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   11c98:	52800164 	mov	w4, #0xb                   	// #11
   11c9c:	b9427c01 	ldr	w1, [x0,#636]
   11ca0:	4b010260 	sub	w0, w19, w1
   11ca4:	8b20ca80 	add	x0, x20, w0, sxtw #2
   11ca8:	b9400402 	ldr	w2, [x0,#4]
   11cac:	3100045f 	cmn	w2, #0x1
   11cb0:	54000460 	b.eq	11d3c <__pthread_tpp_change_priority+0xe4>
   11cb4:	11000442 	add	w2, w2, #0x1
   11cb8:	6b17027f 	cmp	w19, w23
   11cbc:	b9000402 	str	w2, [x0,#4]
   11cc0:	540004cd 	b.le	11d58 <__pthread_tpp_change_priority+0x100>
   11cc4:	310006df 	cmn	w22, #0x1
   11cc8:	54000680 	b.eq	11d98 <__pthread_tpp_change_priority+0x140>
   11ccc:	4b0102c2 	sub	w2, w22, w1
   11cd0:	8b22ca82 	add	x2, x20, w2, sxtw #2
   11cd4:	b9400440 	ldr	w0, [x2,#4]
   11cd8:	51000400 	sub	w0, w0, #0x1
   11cdc:	b9000440 	str	w0, [x2,#4]
   11ce0:	350005c0 	cbnz	w0, 11d98 <__pthread_tpp_change_priority+0x140>
   11ce4:	6b1702df 	cmp	w22, w23
   11ce8:	54000581 	b.ne	11d98 <__pthread_tpp_change_priority+0x140>
   11cec:	6b1302df 	cmp	w22, w19
   11cf0:	5400054d 	b.le	11d98 <__pthread_tpp_change_priority+0x140>
   11cf4:	510006d3 	sub	w19, w22, #0x1
   11cf8:	6b01027f 	cmp	w19, w1
   11cfc:	540001ab 	b.lt	11d30 <__pthread_tpp_change_priority+0xd8>
   11d00:	4b010260 	sub	w0, w19, w1
   11d04:	8b20ca80 	add	x0, x20, w0, sxtw #2
   11d08:	b9400400 	ldr	w0, [x0,#4]
   11d0c:	34000080 	cbz	w0, 11d1c <__pthread_tpp_change_priority+0xc4>
   11d10:	14000008 	b	11d30 <__pthread_tpp_change_priority+0xd8>
   11d14:	b9400400 	ldr	w0, [x0,#4]
   11d18:	350000c0 	cbnz	w0, 11d30 <__pthread_tpp_change_priority+0xd8>
   11d1c:	51000673 	sub	w19, w19, #0x1
   11d20:	6b01027f 	cmp	w19, w1
   11d24:	4b010260 	sub	w0, w19, w1
   11d28:	8b20ca80 	add	x0, x20, w0, sxtw #2
   11d2c:	54ffff4a 	b.ge	11d14 <__pthread_tpp_change_priority+0xbc>
   11d30:	6b17027f 	cmp	w19, w23
   11d34:	54000321 	b.ne	11d98 <__pthread_tpp_change_priority+0x140>
   11d38:	52800004 	mov	w4, #0x0                   	// #0
   11d3c:	2a0403e0 	mov	w0, w4
   11d40:	f94023f9 	ldr	x25, [sp,#64]
   11d44:	a94153f3 	ldp	x19, x20, [sp,#16]
   11d48:	a9425bf5 	ldp	x21, x22, [sp,#32]
   11d4c:	a94363f7 	ldp	x23, x24, [sp,#48]
   11d50:	a8c67bfd 	ldp	x29, x30, [sp],#96
   11d54:	d65f03c0 	ret
   11d58:	310006df 	cmn	w22, #0x1
   11d5c:	54fffee0 	b.eq	11d38 <__pthread_tpp_change_priority+0xe0>
   11d60:	f00000e0 	adrp	x0, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   11d64:	b9427c01 	ldr	w1, [x0,#636]
   11d68:	4b0102c2 	sub	w2, w22, w1
   11d6c:	8b22ca82 	add	x2, x20, w2, sxtw #2
   11d70:	b9400440 	ldr	w0, [x2,#4]
   11d74:	51000400 	sub	w0, w0, #0x1
   11d78:	b9000440 	str	w0, [x2,#4]
   11d7c:	35fffde0 	cbnz	w0, 11d38 <__pthread_tpp_change_priority+0xe0>
   11d80:	6b1702df 	cmp	w22, w23
   11d84:	54fffda1 	b.ne	11d38 <__pthread_tpp_change_priority+0xe0>
   11d88:	6b1302df 	cmp	w22, w19
   11d8c:	54fffb4c 	b.gt	11cf4 <__pthread_tpp_change_priority+0x9c>
   11d90:	52800004 	mov	w4, #0x0                   	// #0
   11d94:	17ffffea 	b	11d3c <__pthread_tpp_change_priority+0xe4>
   11d98:	b9005bbf 	str	wzr, [x29,#88]
   11d9c:	911062b6 	add	x22, x21, #0x418
   11da0:	52800020 	mov	w0, #0x1                   	// #1
   11da4:	885ffec1 	ldaxr	w1, [x22]
   11da8:	6b1f003f 	cmp	w1, wzr
   11dac:	54000061 	b.ne	11db8 <__pthread_tpp_change_priority+0x160>
   11db0:	88027ec0 	stxr	w2, w0, [x22]
   11db4:	35ffff82 	cbnz	w2, 11da4 <__pthread_tpp_change_priority+0x14c>
   11db8:	54000080 	b.eq	11dc8 <__pthread_tpp_change_priority+0x170>
   11dbc:	aa1603e0 	mov	x0, x22
   11dc0:	b9005ba1 	str	w1, [x29,#88]
   11dc4:	97fff669 	bl	f768 <__lll_lock_wait_private>
   11dc8:	b9410ea0 	ldr	w0, [x21,#268]
   11dcc:	b9000293 	str	w19, [x20]
   11dd0:	362805c0 	tbz	w0, #5, 11e88 <__pthread_tpp_change_priority+0x230>
   11dd4:	121a0004 	and	w4, w0, #0x40
   11dd8:	363007a0 	tbz	w0, #6, 11ecc <__pthread_tpp_change_priority+0x274>
   11ddc:	b94432a0 	ldr	w0, [x21,#1072]
   11de0:	b9005ba0 	str	w0, [x29,#88]
   11de4:	6b00027f 	cmp	w19, w0
   11de8:	540006cd 	b.le	11ec0 <__pthread_tpp_change_priority+0x268>
   11dec:	b9005bb3 	str	w19, [x29,#88]
   11df0:	b940d2a0 	ldr	w0, [x21,#208]
   11df4:	910163a2 	add	x2, x29, #0x58
   11df8:	b94436a1 	ldr	w1, [x21,#1076]
   11dfc:	97ffcd25 	bl	5290 <__sched_setscheduler@plt>
   11e00:	37f80840 	tbnz	w0, #31, 11f08 <__pthread_tpp_change_priority+0x2b0>
   11e04:	52800014 	mov	w20, #0x0                   	// #0
   11e08:	52800001 	mov	w1, #0x0                   	// #0
   11e0c:	885f7ec0 	ldxr	w0, [x22]
   11e10:	8802fec1 	stlxr	w2, w1, [x22]
   11e14:	35ffffc2 	cbnz	w2, 11e0c <__pthread_tpp_change_priority+0x1b4>
   11e18:	7100041f 	cmp	w0, #0x1
   11e1c:	2a1403e4 	mov	w4, w20
   11e20:	54fff8ed 	b.le	11d3c <__pthread_tpp_change_priority+0xe4>
   11e24:	aa1603e0 	mov	x0, x22
   11e28:	d2801021 	mov	x1, #0x81                  	// #129
   11e2c:	d2800022 	mov	x2, #0x1                   	// #1
   11e30:	d2800003 	mov	x3, #0x0                   	// #0
   11e34:	d2800c48 	mov	x8, #0x62                  	// #98
   11e38:	d4000001 	svc	#0x0
   11e3c:	17ffffc0 	b	11d3c <__pthread_tpp_change_priority+0xe4>
   11e40:	f00000f4 	adrp	x20, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   11e44:	9109e299 	add	x25, x20, #0x278
   11e48:	b9400737 	ldr	w23, [x25,#4]
   11e4c:	310006ff 	cmn	w23, #0x1
   11e50:	54000720 	b.eq	11f34 <__pthread_tpp_change_priority+0x2dc>
   11e54:	b9427a82 	ldr	w2, [x20,#632]
   11e58:	d2800021 	mov	x1, #0x1                   	// #1
   11e5c:	d2800080 	mov	x0, #0x4                   	// #4
   11e60:	4b170042 	sub	w2, w2, w23
   11e64:	11000442 	add	w2, w2, #0x1
   11e68:	8b22c800 	add	x0, x0, w2, sxtw #2
   11e6c:	97ffcca1 	bl	50f0 <calloc@plt>
   11e70:	aa0003f4 	mov	x20, x0
   11e74:	b4000520 	cbz	x0, 11f18 <__pthread_tpp_change_priority+0x2c0>
   11e78:	510006f7 	sub	w23, w23, #0x1
   11e7c:	f9025aa0 	str	x0, [x21,#1200]
   11e80:	b9000017 	str	w23, [x0]
   11e84:	17ffff82 	b	11c8c <__pthread_tpp_change_priority+0x34>
   11e88:	b940d2a0 	ldr	w0, [x21,#208]
   11e8c:	9110c2a1 	add	x1, x21, #0x430
   11e90:	97ffcca8 	bl	5130 <__sched_getparam@plt>
   11e94:	34000320 	cbz	w0, 11ef8 <__pthread_tpp_change_priority+0x2a0>
   11e98:	d00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   11e9c:	f947c421 	ldr	x1, [x1,#3976]
   11ea0:	b9410ea0 	ldr	w0, [x21,#268]
   11ea4:	b8616b14 	ldr	w20, [x24,x1]
   11ea8:	36300140 	tbz	w0, #6, 11ed0 <__pthread_tpp_change_priority+0x278>
   11eac:	35fffaf4 	cbnz	w20, 11e08 <__pthread_tpp_change_priority+0x1b0>
   11eb0:	b94432a0 	ldr	w0, [x21,#1072]
   11eb4:	b9005ba0 	str	w0, [x29,#88]
   11eb8:	6b00027f 	cmp	w19, w0
   11ebc:	54fff98c 	b.gt	11dec <__pthread_tpp_change_priority+0x194>
   11ec0:	6b17001f 	cmp	w0, w23
   11ec4:	54fff96b 	b.lt	11df0 <__pthread_tpp_change_priority+0x198>
   11ec8:	17ffffcf 	b	11e04 <__pthread_tpp_change_priority+0x1ac>
   11ecc:	2a0403f4 	mov	w20, w4
   11ed0:	b940d2a0 	ldr	w0, [x21,#208]
   11ed4:	97ffcd17 	bl	5330 <__sched_getscheduler@plt>
   11ed8:	3100041f 	cmn	w0, #0x1
   11edc:	b90436a0 	str	w0, [x21,#1076]
   11ee0:	54000200 	b.eq	11f20 <__pthread_tpp_change_priority+0x2c8>
   11ee4:	b9410ea0 	ldr	w0, [x21,#268]
   11ee8:	321a0000 	orr	w0, w0, #0x40
   11eec:	b9010ea0 	str	w0, [x21,#268]
   11ef0:	35fff8d4 	cbnz	w20, 11e08 <__pthread_tpp_change_priority+0x1b0>
   11ef4:	17ffffef 	b	11eb0 <__pthread_tpp_change_priority+0x258>
   11ef8:	b9410ea0 	ldr	w0, [x21,#268]
   11efc:	321b0000 	orr	w0, w0, #0x20
   11f00:	b9010ea0 	str	w0, [x21,#268]
   11f04:	17ffffb4 	b	11dd4 <__pthread_tpp_change_priority+0x17c>
   11f08:	d00000e0 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
   11f0c:	f947c400 	ldr	x0, [x0,#3976]
   11f10:	b8606b14 	ldr	w20, [x24,x0]
   11f14:	17ffffbd 	b	11e08 <__pthread_tpp_change_priority+0x1b0>
   11f18:	52800184 	mov	w4, #0xc                   	// #12
   11f1c:	17ffff88 	b	11d3c <__pthread_tpp_change_priority+0xe4>
   11f20:	d00000e0 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
   11f24:	f947c400 	ldr	x0, [x0,#3976]
   11f28:	b8606b14 	ldr	w20, [x24,x0]
   11f2c:	35fff6f4 	cbnz	w20, 11e08 <__pthread_tpp_change_priority+0x1b0>
   11f30:	17ffffe0 	b	11eb0 <__pthread_tpp_change_priority+0x258>
   11f34:	52800020 	mov	w0, #0x1                   	// #1
   11f38:	97ffcc82 	bl	5140 <sched_get_priority_max@plt>
   11f3c:	b9027a80 	str	w0, [x20,#632]
   11f40:	52800020 	mov	w0, #0x1                   	// #1
   11f44:	d5033bbf 	dmb	ish
   11f48:	97ffccba 	bl	5230 <sched_get_priority_min@plt>
   11f4c:	2a0003f7 	mov	w23, w0
   11f50:	b9000720 	str	w0, [x25,#4]
   11f54:	17ffffc0 	b	11e54 <__pthread_tpp_change_priority+0x1fc>

0000000000011f58 <__pthread_current_priority>:
   11f58:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   11f5c:	910003fd 	mov	x29, sp
   11f60:	a90153f3 	stp	x19, x20, [sp,#16]
   11f64:	d53bd053 	mrs	x19, tpidr_el0
   11f68:	d11bc273 	sub	x19, x19, #0x6f0
   11f6c:	b9410e60 	ldr	w0, [x19,#268]
   11f70:	121b0400 	and	w0, w0, #0x60
   11f74:	7101801f 	cmp	w0, #0x60
   11f78:	540003c0 	b.eq	11ff0 <__pthread_current_priority+0x98>
   11f7c:	b9002fbf 	str	wzr, [x29,#44]
   11f80:	91106274 	add	x20, x19, #0x418
   11f84:	52800020 	mov	w0, #0x1                   	// #1
   11f88:	885ffe81 	ldaxr	w1, [x20]
   11f8c:	6b1f003f 	cmp	w1, wzr
   11f90:	54000061 	b.ne	11f9c <__pthread_current_priority+0x44>
   11f94:	88027e80 	stxr	w2, w0, [x20]
   11f98:	35ffff82 	cbnz	w2, 11f88 <__pthread_current_priority+0x30>
   11f9c:	540001e1 	b.ne	11fd8 <__pthread_current_priority+0x80>
   11fa0:	b9410e60 	ldr	w0, [x19,#268]
   11fa4:	36280300 	tbz	w0, #5, 12004 <__pthread_current_priority+0xac>
   11fa8:	363003e0 	tbz	w0, #6, 12024 <__pthread_current_priority+0xcc>
   11fac:	b9443264 	ldr	w4, [x19,#1072]
   11fb0:	52800001 	mov	w1, #0x0                   	// #0
   11fb4:	885f7e80 	ldxr	w0, [x20]
   11fb8:	8802fe81 	stlxr	w2, w1, [x20]
   11fbc:	35ffffc2 	cbnz	w2, 11fb4 <__pthread_current_priority+0x5c>
   11fc0:	7100041f 	cmp	w0, #0x1
   11fc4:	5400060c 	b.gt	12084 <__pthread_current_priority+0x12c>
   11fc8:	2a0403e0 	mov	w0, w4
   11fcc:	a94153f3 	ldp	x19, x20, [sp,#16]
   11fd0:	a8c37bfd 	ldp	x29, x30, [sp],#48
   11fd4:	d65f03c0 	ret
   11fd8:	aa1403e0 	mov	x0, x20
   11fdc:	b9002fa1 	str	w1, [x29,#44]
   11fe0:	97fff5e2 	bl	f768 <__lll_lock_wait_private>
   11fe4:	b9410e60 	ldr	w0, [x19,#268]
   11fe8:	372ffe00 	tbnz	w0, #5, 11fa8 <__pthread_current_priority+0x50>
   11fec:	14000006 	b	12004 <__pthread_current_priority+0xac>
   11ff0:	b9443264 	ldr	w4, [x19,#1072]
   11ff4:	a94153f3 	ldp	x19, x20, [sp,#16]
   11ff8:	2a0403e0 	mov	w0, w4
   11ffc:	a8c37bfd 	ldp	x29, x30, [sp],#48
   12000:	d65f03c0 	ret
   12004:	b940d260 	ldr	w0, [x19,#208]
   12008:	9110c261 	add	x1, x19, #0x430
   1200c:	97ffcc49 	bl	5130 <__sched_getparam@plt>
   12010:	350001e0 	cbnz	w0, 1204c <__pthread_current_priority+0xf4>
   12014:	b9410e60 	ldr	w0, [x19,#268]
   12018:	321b0000 	orr	w0, w0, #0x20
   1201c:	b9010e60 	str	w0, [x19,#268]
   12020:	3737fc60 	tbnz	w0, #6, 11fac <__pthread_current_priority+0x54>
   12024:	b940d260 	ldr	w0, [x19,#208]
   12028:	97ffccc2 	bl	5330 <__sched_getscheduler@plt>
   1202c:	3100041f 	cmn	w0, #0x1
   12030:	b9043660 	str	w0, [x19,#1076]
   12034:	54000100 	b.eq	12054 <__pthread_current_priority+0xfc>
   12038:	b9410e60 	ldr	w0, [x19,#268]
   1203c:	b9443264 	ldr	w4, [x19,#1072]
   12040:	321a0000 	orr	w0, w0, #0x40
   12044:	b9010e60 	str	w0, [x19,#268]
   12048:	17ffffda 	b	11fb0 <__pthread_current_priority+0x58>
   1204c:	b9410e60 	ldr	w0, [x19,#268]
   12050:	36300060 	tbz	w0, #6, 1205c <__pthread_current_priority+0x104>
   12054:	12800004 	mov	w4, #0xffffffff            	// #-1
   12058:	17ffffd6 	b	11fb0 <__pthread_current_priority+0x58>
   1205c:	b940d260 	ldr	w0, [x19,#208]
   12060:	97ffccb4 	bl	5330 <__sched_getscheduler@plt>
   12064:	3100041f 	cmn	w0, #0x1
   12068:	b9043660 	str	w0, [x19,#1076]
   1206c:	54ffff40 	b.eq	12054 <__pthread_current_priority+0xfc>
   12070:	b9410e60 	ldr	w0, [x19,#268]
   12074:	12800004 	mov	w4, #0xffffffff            	// #-1
   12078:	321a0000 	orr	w0, w0, #0x40
   1207c:	b9010e60 	str	w0, [x19,#268]
   12080:	17ffffcc 	b	11fb0 <__pthread_current_priority+0x58>
   12084:	aa1403e0 	mov	x0, x20
   12088:	d2801021 	mov	x1, #0x81                  	// #129
   1208c:	d2800022 	mov	x2, #0x1                   	// #1
   12090:	d2800003 	mov	x3, #0x0                   	// #0
   12094:	d2800c48 	mov	x8, #0x62                  	// #98
   12098:	d4000001 	svc	#0x0
   1209c:	17ffffcb 	b	11fc8 <__pthread_current_priority+0x70>

00000000000120a0 <pthread_mutex_getprioceiling>:
   120a0:	b9401002 	ldr	w2, [x0,#16]
   120a4:	363000c2 	tbz	w2, #6, 120bc <pthread_mutex_getprioceiling+0x1c>
   120a8:	b9400002 	ldr	w2, [x0]
   120ac:	52800000 	mov	w0, #0x0                   	// #0
   120b0:	53137c42 	lsr	w2, w2, #19
   120b4:	b9000022 	str	w2, [x1]
   120b8:	d65f03c0 	ret
   120bc:	528002c0 	mov	w0, #0x16                  	// #22
   120c0:	d65f03c0 	ret

00000000000120c4 <pthread_mutex_setprioceiling>:
   120c4:	a9bc7bfd 	stp	x29, x30, [sp,#-64]!
   120c8:	910003fd 	mov	x29, sp
   120cc:	a90153f3 	stp	x19, x20, [sp,#16]
   120d0:	a9025bf5 	stp	x21, x22, [sp,#32]
   120d4:	aa0003f3 	mov	x19, x0
   120d8:	2a0103f4 	mov	w20, w1
   120dc:	b9401000 	ldr	w0, [x0,#16]
   120e0:	aa0203f5 	mov	x21, x2
   120e4:	373000c0 	tbnz	w0, #6, 120fc <pthread_mutex_setprioceiling+0x38>
   120e8:	528002c0 	mov	w0, #0x16                  	// #22
   120ec:	a94153f3 	ldp	x19, x20, [sp,#16]
   120f0:	a9425bf5 	ldp	x21, x22, [sp,#32]
   120f4:	a8c47bfd 	ldp	x29, x30, [sp],#64
   120f8:	d65f03c0 	ret
   120fc:	d00000f6 	adrp	x22, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   12100:	b9427ec0 	ldr	w0, [x22,#636]
   12104:	3100041f 	cmn	w0, #0x1
   12108:	54000dc0 	b.eq	122c0 <pthread_mutex_setprioceiling+0x1fc>
   1210c:	6b00029f 	cmp	w20, w0
   12110:	54fffecb 	b.lt	120e8 <pthread_mutex_setprioceiling+0x24>
   12114:	d00000e0 	adrp	x0, 30000 <_GLOBAL_OFFSET_TABLE_+0x88>
   12118:	b9427800 	ldr	w0, [x0,#632]
   1211c:	6b00029f 	cmp	w20, w0
   12120:	54fffe4c 	b.gt	120e8 <pthread_mutex_setprioceiling+0x24>
   12124:	12002e80 	and	w0, w20, #0xfff
   12128:	6b00029f 	cmp	w20, w0
   1212c:	54fffde1 	b.ne	120e8 <pthread_mutex_setprioceiling+0x24>
   12130:	d53bd040 	mrs	x0, tpidr_el0
   12134:	b9400a61 	ldr	w1, [x19,#8]
   12138:	d11bc000 	sub	x0, x0, #0x6f0
   1213c:	b9401262 	ldr	w2, [x19,#16]
   12140:	b940d000 	ldr	w0, [x0,#208]
   12144:	6b00003f 	cmp	w1, w0
   12148:	54000960 	b.eq	12274 <pthread_mutex_setprioceiling+0x1b0>
   1214c:	b9400264 	ldr	w4, [x19]
   12150:	120d3084 	and	w4, w4, #0xfff80000
   12154:	b9003fa4 	str	w4, [x29,#60]
   12158:	32000086 	orr	w6, w4, #0x1
   1215c:	885ffe60 	ldaxr	w0, [x19]
   12160:	6b04001f 	cmp	w0, w4
   12164:	54000061 	b.ne	12170 <pthread_mutex_setprioceiling+0xac>
   12168:	88017e66 	stxr	w1, w6, [x19]
   1216c:	35ffff81 	cbnz	w1, 1215c <pthread_mutex_setprioceiling+0x98>
   12170:	54000040 	b.eq	12178 <pthread_mutex_setprioceiling+0xb4>
   12174:	b9003fa0 	str	w0, [x29,#60]
   12178:	b9403fa0 	ldr	w0, [x29,#60]
   1217c:	6b00009f 	cmp	w4, w0
   12180:	540009c0 	b.eq	122b8 <pthread_mutex_setprioceiling+0x1f4>
   12184:	321f0087 	orr	w7, w4, #0x2
   12188:	9100f3aa 	add	x10, x29, #0x3c
   1218c:	93407ce9 	sxtw	x9, w7
   12190:	b9003fa6 	str	w6, [x29,#60]
   12194:	2a0603e0 	mov	w0, w6
   12198:	885ffe61 	ldaxr	w1, [x19]
   1219c:	6b00003f 	cmp	w1, w0
   121a0:	54000061 	b.ne	121ac <pthread_mutex_setprioceiling+0xe8>
   121a4:	88027e67 	stxr	w2, w7, [x19]
   121a8:	35ffff82 	cbnz	w2, 12198 <pthread_mutex_setprioceiling+0xd4>
   121ac:	54000040 	b.eq	121b4 <pthread_mutex_setprioceiling+0xf0>
   121b0:	b9000141 	str	w1, [x10]
   121b4:	b9403fa5 	ldr	w5, [x29,#60]
   121b8:	aa1303e0 	mov	x0, x19
   121bc:	aa0903e2 	mov	x2, x9
   121c0:	d2800003 	mov	x3, #0x0                   	// #0
   121c4:	120d30a1 	and	w1, w5, #0xfff80000
   121c8:	d2800c48 	mov	x8, #0x62                  	// #98
   121cc:	6b04003f 	cmp	w1, w4
   121d0:	54000261 	b.ne	1221c <pthread_mutex_setprioceiling+0x158>
   121d4:	6b05009f 	cmp	w4, w5
   121d8:	540000a0 	b.eq	121ec <pthread_mutex_setprioceiling+0x128>
   121dc:	b9401261 	ldr	w1, [x19,#16]
   121e0:	2a2103e1 	mvn	w1, w1
   121e4:	92790021 	and	x1, x1, #0x80
   121e8:	d4000001 	svc	#0x0
   121ec:	b9003fa4 	str	w4, [x29,#60]
   121f0:	2a0403e0 	mov	w0, w4
   121f4:	885ffe61 	ldaxr	w1, [x19]
   121f8:	6b00003f 	cmp	w1, w0
   121fc:	54000061 	b.ne	12208 <pthread_mutex_setprioceiling+0x144>
   12200:	88027e67 	stxr	w2, w7, [x19]
   12204:	35ffff82 	cbnz	w2, 121f4 <pthread_mutex_setprioceiling+0x130>
   12208:	54000040 	b.eq	12210 <pthread_mutex_setprioceiling+0x14c>
   1220c:	b9000141 	str	w1, [x10]
   12210:	b9403fa0 	ldr	w0, [x29,#60]
   12214:	6b00009f 	cmp	w4, w0
   12218:	54fffbc1 	b.ne	12190 <pthread_mutex_setprioceiling+0xcc>
   1221c:	53137ca5 	lsr	w5, w5, #19
   12220:	b4000075 	cbz	x21, 1222c <pthread_mutex_setprioceiling+0x168>
   12224:	b90002a5 	str	w5, [x21]
   12228:	52800015 	mov	w21, #0x0                   	// #0
   1222c:	2a144eb4 	orr	w20, w21, w20, lsl #19
   12230:	b9000274 	str	w20, [x19]
   12234:	aa1303e0 	mov	x0, x19
   12238:	b2407be2 	mov	x2, #0x7fffffff            	// #2147483647
   1223c:	d5033bbf 	dmb	ish
   12240:	b9401261 	ldr	w1, [x19,#16]
   12244:	d2800003 	mov	x3, #0x0                   	// #0
   12248:	d2800c48 	mov	x8, #0x62                  	// #98
   1224c:	12190024 	and	w4, w1, #0x80
   12250:	52801021 	mov	w1, #0x81                  	// #129
   12254:	4a010081 	eor	w1, w4, w1
   12258:	93407c21 	sxtw	x1, w1
   1225c:	d4000001 	svc	#0x0
   12260:	2a0303e0 	mov	w0, w3
   12264:	a94153f3 	ldp	x19, x20, [sp,#16]
   12268:	a9425bf5 	ldp	x21, x22, [sp,#32]
   1226c:	a8c47bfd 	ldp	x29, x30, [sp],#64
   12270:	d65f03c0 	ret
   12274:	12001842 	and	w2, w2, #0x7f
   12278:	52800460 	mov	w0, #0x23                  	// #35
   1227c:	7101085f 	cmp	w2, #0x42
   12280:	54fff360 	b.eq	120ec <pthread_mutex_setprioceiling+0x28>
   12284:	7101045f 	cmp	w2, #0x41
   12288:	54fff621 	b.ne	1214c <pthread_mutex_setprioceiling+0x88>
   1228c:	b9400276 	ldr	w22, [x19]
   12290:	2a1403e1 	mov	w1, w20
   12294:	53137ed6 	lsr	w22, w22, #19
   12298:	2a1603e0 	mov	w0, w22
   1229c:	97fffe6f 	bl	11c58 <__pthread_tpp_change_priority>
   122a0:	35fff260 	cbnz	w0, 120ec <pthread_mutex_setprioceiling+0x28>
   122a4:	b4000055 	cbz	x21, 122ac <pthread_mutex_setprioceiling+0x1e8>
   122a8:	b90002b6 	str	w22, [x21]
   122ac:	b9400275 	ldr	w21, [x19]
   122b0:	12004ab5 	and	w21, w21, #0x7ffff
   122b4:	17ffffde 	b	1222c <pthread_mutex_setprioceiling+0x168>
   122b8:	2a0403e5 	mov	w5, w4
   122bc:	17ffffd8 	b	1221c <pthread_mutex_setprioceiling+0x158>
   122c0:	97fffe57 	bl	11c1c <__init_sched_fifo_prio>
   122c4:	b9427ec0 	ldr	w0, [x22,#636]
   122c8:	17ffff91 	b	1210c <pthread_mutex_setprioceiling+0x48>

00000000000122cc <pthread_setname_np>:
   122cc:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
   122d0:	910003fd 	mov	x29, sp
   122d4:	a9025bf5 	stp	x21, x22, [sp,#32]
   122d8:	aa0003f5 	mov	x21, x0
   122dc:	aa0103e0 	mov	x0, x1
   122e0:	a90153f3 	stp	x19, x20, [sp,#16]
   122e4:	f9001bf7 	str	x23, [sp,#48]
   122e8:	aa0103f4 	mov	x20, x1
   122ec:	97ffcb25 	bl	4f80 <strlen@plt>
   122f0:	f1003c1f 	cmp	x0, #0xf
   122f4:	aa0003f3 	mov	x19, x0
   122f8:	52800440 	mov	w0, #0x22                  	// #34
   122fc:	540000c9 	b.ls	12314 <pthread_setname_np+0x48>
   12300:	a94153f3 	ldp	x19, x20, [sp,#16]
   12304:	a9425bf5 	ldp	x21, x22, [sp,#32]
   12308:	f9401bf7 	ldr	x23, [sp,#48]
   1230c:	a8c67bfd 	ldp	x29, x30, [sp],#96
   12310:	d65f03c0 	ret
   12314:	d53bd056 	mrs	x22, tpidr_el0
   12318:	d11bc2c0 	sub	x0, x22, #0x6f0
   1231c:	eb0002bf 	cmp	x21, x0
   12320:	540004e0 	b.eq	123bc <pthread_setname_np+0xf0>
   12324:	b940d2a2 	ldr	w2, [x21,#208]
   12328:	90000001 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
   1232c:	910103a0 	add	x0, x29, #0x40
   12330:	9137c021 	add	x1, x1, #0xdf0
   12334:	aa0003f5 	mov	x21, x0
   12338:	97ffcb3a 	bl	5020 <sprintf@plt>
   1233c:	aa1503e0 	mov	x0, x21
   12340:	52800041 	mov	w1, #0x2                   	// #2
   12344:	97fff8f8 	bl	10724 <__open_nocancel>
   12348:	3100041f 	cmn	w0, #0x1
   1234c:	2a0003f5 	mov	w21, w0
   12350:	540003e0 	b.eq	123cc <pthread_setname_np+0x100>
   12354:	b00000f7 	adrp	x23, 2f000 <__FRAME_END__+0x18e30>
   12358:	f947c6f7 	ldr	x23, [x23,#3976]
   1235c:	14000004 	b	1236c <pthread_setname_np+0xa0>
   12360:	b8776ac3 	ldr	w3, [x22,x23]
   12364:	7100107f 	cmp	w3, #0x4
   12368:	54000181 	b.ne	12398 <pthread_setname_np+0xcc>
   1236c:	2a1503e0 	mov	w0, w21
   12370:	aa1403e1 	mov	x1, x20
   12374:	aa1303e2 	mov	x2, x19
   12378:	97fff61e 	bl	fbf0 <__write_nocancel>
   1237c:	93407c00 	sxtw	x0, w0
   12380:	b100041f 	cmn	x0, #0x1
   12384:	54fffee0 	b.eq	12360 <pthread_setname_np+0x94>
   12388:	b7f802a0 	tbnz	x0, #63, 123dc <pthread_setname_np+0x110>
   1238c:	eb13001f 	cmp	x0, x19
   12390:	528000a3 	mov	w3, #0x5                   	// #5
   12394:	1a8303e3 	csel	w3, wzr, w3, eq
   12398:	93407ea0 	sxtw	x0, w21
   1239c:	d2800728 	mov	x8, #0x39                  	// #57
   123a0:	d4000001 	svc	#0x0
   123a4:	2a0303e0 	mov	w0, w3
   123a8:	f9401bf7 	ldr	x23, [sp,#48]
   123ac:	a94153f3 	ldp	x19, x20, [sp,#16]
   123b0:	a9425bf5 	ldp	x21, x22, [sp,#32]
   123b4:	a8c67bfd 	ldp	x29, x30, [sp],#96
   123b8:	d65f03c0 	ret
   123bc:	528001e0 	mov	w0, #0xf                   	// #15
   123c0:	aa1403e1 	mov	x1, x20
   123c4:	97ffcbef 	bl	5380 <prctl@plt>
   123c8:	34fff9c0 	cbz	w0, 12300 <pthread_setname_np+0x34>
   123cc:	b00000e0 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
   123d0:	f947c400 	ldr	x0, [x0,#3976]
   123d4:	b8606ac0 	ldr	w0, [x22,x0]
   123d8:	17ffffca 	b	12300 <pthread_setname_np+0x34>
   123dc:	b00000e0 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
   123e0:	f947c400 	ldr	x0, [x0,#3976]
   123e4:	b8606ac3 	ldr	w3, [x22,x0]
   123e8:	17ffffec 	b	12398 <pthread_setname_np+0xcc>

00000000000123ec <pthread_getname_np>:
   123ec:	a9ba7bfd 	stp	x29, x30, [sp,#-96]!
   123f0:	f1003c5f 	cmp	x2, #0xf
   123f4:	aa0003e3 	mov	x3, x0
   123f8:	910003fd 	mov	x29, sp
   123fc:	a90153f3 	stp	x19, x20, [sp,#16]
   12400:	a9025bf5 	stp	x21, x22, [sp,#32]
   12404:	f9001bf7 	str	x23, [sp,#48]
   12408:	aa0103f4 	mov	x20, x1
   1240c:	52800440 	mov	w0, #0x22                  	// #34
   12410:	540000c8 	b.hi	12428 <pthread_getname_np+0x3c>
   12414:	a94153f3 	ldp	x19, x20, [sp,#16]
   12418:	a9425bf5 	ldp	x21, x22, [sp,#32]
   1241c:	f9401bf7 	ldr	x23, [sp,#48]
   12420:	a8c67bfd 	ldp	x29, x30, [sp],#96
   12424:	d65f03c0 	ret
   12428:	d53bd056 	mrs	x22, tpidr_el0
   1242c:	aa0203f3 	mov	x19, x2
   12430:	d11bc2c0 	sub	x0, x22, #0x6f0
   12434:	eb00007f 	cmp	x3, x0
   12438:	54000580 	b.eq	124e8 <pthread_getname_np+0xfc>
   1243c:	910103b5 	add	x21, x29, #0x40
   12440:	b940d062 	ldr	w2, [x3,#208]
   12444:	90000001 	adrp	x1, 12000 <__pthread_current_priority+0xa8>
   12448:	aa1503e0 	mov	x0, x21
   1244c:	9137c021 	add	x1, x1, #0xdf0
   12450:	97ffcaf4 	bl	5020 <sprintf@plt>
   12454:	aa1503e0 	mov	x0, x21
   12458:	52800001 	mov	w1, #0x0                   	// #0
   1245c:	97fff8b2 	bl	10724 <__open_nocancel>
   12460:	3100041f 	cmn	w0, #0x1
   12464:	2a0003f5 	mov	w21, w0
   12468:	54000460 	b.eq	124f4 <pthread_getname_np+0x108>
   1246c:	b00000f7 	adrp	x23, 2f000 <__FRAME_END__+0x18e30>
   12470:	f947c6f7 	ldr	x23, [x23,#3976]
   12474:	14000004 	b	12484 <pthread_getname_np+0x98>
   12478:	b8776ac3 	ldr	w3, [x22,x23]
   1247c:	7100107f 	cmp	w3, #0x4
   12480:	54000221 	b.ne	124c4 <pthread_getname_np+0xd8>
   12484:	2a1503e0 	mov	w0, w21
   12488:	aa1403e1 	mov	x1, x20
   1248c:	aa1303e2 	mov	x2, x19
   12490:	97fff5f4 	bl	fc60 <__read_nocancel>
   12494:	93407c03 	sxtw	x3, w0
   12498:	b100047f 	cmn	x3, #0x1
   1249c:	54fffee0 	b.eq	12478 <pthread_getname_np+0x8c>
   124a0:	b7f803c3 	tbnz	x3, #63, 12518 <pthread_getname_np+0x12c>
   124a4:	d1000460 	sub	x0, x3, #0x1
   124a8:	38606a81 	ldrb	w1, [x20,x0]
   124ac:	7100283f 	cmp	w1, #0xa
   124b0:	540002a0 	b.eq	12504 <pthread_getname_np+0x118>
   124b4:	eb03027f 	cmp	x19, x3
   124b8:	540002c0 	b.eq	12510 <pthread_getname_np+0x124>
   124bc:	38236a9f 	strb	wzr, [x20,x3]
   124c0:	52800003 	mov	w3, #0x0                   	// #0
   124c4:	93407ea0 	sxtw	x0, w21
   124c8:	d2800728 	mov	x8, #0x39                  	// #57
   124cc:	d4000001 	svc	#0x0
   124d0:	2a0303e0 	mov	w0, w3
   124d4:	f9401bf7 	ldr	x23, [sp,#48]
   124d8:	a94153f3 	ldp	x19, x20, [sp,#16]
   124dc:	a9425bf5 	ldp	x21, x22, [sp,#32]
   124e0:	a8c67bfd 	ldp	x29, x30, [sp],#96
   124e4:	d65f03c0 	ret
   124e8:	52800200 	mov	w0, #0x10                  	// #16
   124ec:	97ffcba5 	bl	5380 <prctl@plt>
   124f0:	34fff920 	cbz	w0, 12414 <pthread_getname_np+0x28>
   124f4:	b00000e0 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
   124f8:	f947c400 	ldr	x0, [x0,#3976]
   124fc:	b8606ac0 	ldr	w0, [x22,x0]
   12500:	17ffffc5 	b	12414 <pthread_getname_np+0x28>
   12504:	38206a9f 	strb	wzr, [x20,x0]
   12508:	52800003 	mov	w3, #0x0                   	// #0
   1250c:	17ffffee 	b	124c4 <pthread_getname_np+0xd8>
   12510:	52800443 	mov	w3, #0x22                  	// #34
   12514:	17ffffec 	b	124c4 <pthread_getname_np+0xd8>
   12518:	b00000e0 	adrp	x0, 2f000 <__FRAME_END__+0x18e30>
   1251c:	f947c400 	ldr	x0, [x0,#3976]
   12520:	b8606ac3 	ldr	w3, [x22,x0]
   12524:	17ffffe8 	b	124c4 <pthread_getname_np+0xd8>

0000000000012528 <pthread_setattr_default_np>:
   12528:	a9b87bfd 	stp	x29, x30, [sp,#-128]!
   1252c:	910003fd 	mov	x29, sp
   12530:	a90153f3 	stp	x19, x20, [sp,#16]
   12534:	b9400413 	ldr	w19, [x0,#4]
   12538:	a9025bf5 	stp	x21, x22, [sp,#32]
   1253c:	a90363f7 	stp	x23, x24, [sp,#48]
   12540:	a9046bf9 	stp	x25, x26, [sp,#64]
   12544:	a90573fb 	stp	x27, x28, [sp,#80]
   12548:	71000a7f 	cmp	w19, #0x2
   1254c:	fd0033e8 	str	d8, [sp,#96]
   12550:	54000188 	b.hi	12580 <pthread_setattr_default_np+0x58>
   12554:	b9400015 	ldr	w21, [x0]
   12558:	aa0003f4 	mov	x20, x0
   1255c:	6b1f02bf 	cmp	w21, wzr
   12560:	540002cd 	b.le	125b8 <pthread_setattr_default_np+0x90>
   12564:	2a1303e0 	mov	w0, w19
   12568:	97ffcb06 	bl	5180 <__sched_get_priority_min@plt>
   1256c:	2a0003f6 	mov	w22, w0
   12570:	2a1303e0 	mov	w0, w19
   12574:	97ffcb77 	bl	5350 <__sched_get_priority_max@plt>
   12578:	37f80056 	tbnz	w22, #31, 12580 <pthread_setattr_default_np+0x58>
   1257c:	36f80160 	tbz	w0, #31, 125a8 <pthread_setattr_default_np+0x80>
   12580:	528002c4 	mov	w4, #0x16                  	// #22
   12584:	2a0403e0 	mov	w0, w4
   12588:	a94153f3 	ldp	x19, x20, [sp,#16]
   1258c:	a9425bf5 	ldp	x21, x22, [sp,#32]
   12590:	a94363f7 	ldp	x23, x24, [sp,#48]
   12594:	a9446bf9 	ldp	x25, x26, [sp,#64]
   12598:	a94573fb 	ldp	x27, x28, [sp,#80]
   1259c:	fd4033e8 	ldr	d8, [sp,#96]
   125a0:	a8c87bfd 	ldp	x29, x30, [sp],#128
   125a4:	d65f03c0 	ret
   125a8:	6b1602bf 	cmp	w21, w22
   125ac:	54fffeab 	b.lt	12580 <pthread_setattr_default_np+0x58>
   125b0:	6b0002bf 	cmp	w21, w0
   125b4:	54fffe6c 	b.gt	12580 <pthread_setattr_default_np+0x58>
   125b8:	d0000116 	adrp	x22, 34000 <__GI___pthread_keys+0x3d78>
   125bc:	f9401695 	ldr	x21, [x20,#40]
   125c0:	f9401a93 	ldr	x19, [x20,#48]
   125c4:	f941d6c1 	ldr	x1, [x22,#936]
   125c8:	b40009c1 	cbz	x1, 12700 <pthread_setattr_default_np+0x1d8>
   125cc:	eb01027f 	cmp	x19, x1
   125d0:	54000129 	b.ls	125f4 <pthread_setattr_default_np+0xcc>
   125d4:	38616aa0 	ldrb	w0, [x21,x1]
   125d8:	34000080 	cbz	w0, 125e8 <pthread_setattr_default_np+0xc0>
   125dc:	17ffffe9 	b	12580 <pthread_setattr_default_np+0x58>
   125e0:	38616aa2 	ldrb	w2, [x21,x1]
   125e4:	35fffce2 	cbnz	w2, 12580 <pthread_setattr_default_np+0x58>
   125e8:	91000421 	add	x1, x1, #0x1
   125ec:	eb01027f 	cmp	x19, x1
   125f0:	54ffff88 	b.hi	125e0 <pthread_setattr_default_np+0xb8>
   125f4:	f9401295 	ldr	x21, [x20,#32]
   125f8:	b27f3fe0 	mov	x0, #0x1fffe               	// #131070
   125fc:	d10006a1 	sub	x1, x21, #0x1
   12600:	eb00003f 	cmp	x1, x0
   12604:	54fffbe9 	b.ls	12580 <pthread_setattr_default_np+0x58>
   12608:	b9400a93 	ldr	w19, [x20,#8]
   1260c:	121d0260 	and	w0, w19, #0x8
   12610:	371ffb93 	tbnz	w19, #3, 12580 <pthread_setattr_default_np+0x58>
   12614:	b9007fa0 	str	w0, [x29,#124]
   12618:	d0000116 	adrp	x22, 34000 <__GI___pthread_keys+0x3d78>
   1261c:	b9400280 	ldr	w0, [x20]
   12620:	52800021 	mov	w1, #0x1                   	// #1
   12624:	b940069c 	ldr	w28, [x20,#4]
   12628:	f9400a9b 	ldr	x27, [x20,#16]
   1262c:	f9400e9a 	ldr	x26, [x20,#24]
   12630:	f9401699 	ldr	x25, [x20,#40]
   12634:	f9401a97 	ldr	x23, [x20,#48]
   12638:	1e270008 	fmov	s8, w0
   1263c:	910a22c0 	add	x0, x22, #0x288
   12640:	885ffc02 	ldaxr	w2, [x0]
   12644:	6b1f005f 	cmp	w2, wzr
   12648:	54000061 	b.ne	12654 <pthread_setattr_default_np+0x12c>
   1264c:	88037c01 	stxr	w3, w1, [x0]
   12650:	35ffff83 	cbnz	w3, 12640 <pthread_setattr_default_np+0x118>
   12654:	54000060 	b.eq	12660 <pthread_setattr_default_np+0x138>
   12658:	b9007fa2 	str	w2, [x29,#124]
   1265c:	97fff443 	bl	f768 <__lll_lock_wait_private>
   12660:	d0000118 	adrp	x24, 34000 <__GI___pthread_keys+0x3d78>
   12664:	910cc300 	add	x0, x24, #0x330
   12668:	b40005d7 	cbz	x23, 12720 <pthread_setattr_default_np+0x1f8>
   1266c:	f9401801 	ldr	x1, [x0,#48]
   12670:	eb17003f 	cmp	x1, x23
   12674:	540005c0 	b.eq	1272c <pthread_setattr_default_np+0x204>
   12678:	f9401400 	ldr	x0, [x0,#40]
   1267c:	aa1703e1 	mov	x1, x23
   12680:	97ffcaa4 	bl	5110 <realloc@plt>
   12684:	aa0003f9 	mov	x25, x0
   12688:	b40005e0 	cbz	x0, 12744 <pthread_setattr_default_np+0x21c>
   1268c:	f9401681 	ldr	x1, [x20,#40]
   12690:	aa1703e2 	mov	x2, x23
   12694:	97ffca33 	bl	4f60 <memcpy@plt>
   12698:	b5000075 	cbnz	x21, 126a4 <pthread_setattr_default_np+0x17c>
   1269c:	910cc300 	add	x0, x24, #0x330
   126a0:	f9401015 	ldr	x21, [x0,#32]
   126a4:	910cc303 	add	x3, x24, #0x330
   126a8:	bd033308 	str	s8, [x24,#816]
   126ac:	52800004 	mov	w4, #0x0                   	// #0
   126b0:	b900047c 	str	w28, [x3,#4]
   126b4:	b9000873 	str	w19, [x3,#8]
   126b8:	f900087b 	str	x27, [x3,#16]
   126bc:	f9000c7a 	str	x26, [x3,#24]
   126c0:	f9001075 	str	x21, [x3,#32]
   126c4:	f9001479 	str	x25, [x3,#40]
   126c8:	f9001877 	str	x23, [x3,#48]
   126cc:	910a22c0 	add	x0, x22, #0x288
   126d0:	52800002 	mov	w2, #0x0                   	// #0
   126d4:	885f7c01 	ldxr	w1, [x0]
   126d8:	8803fc02 	stlxr	w3, w2, [x0]
   126dc:	35ffffc3 	cbnz	w3, 126d4 <pthread_setattr_default_np+0x1ac>
   126e0:	7100043f 	cmp	w1, #0x1
   126e4:	54fff50d 	b.le	12584 <pthread_setattr_default_np+0x5c>
   126e8:	d2801021 	mov	x1, #0x81                  	// #129
   126ec:	d2800022 	mov	x2, #0x1                   	// #1
   126f0:	d2800003 	mov	x3, #0x0                   	// #0
   126f4:	d2800c48 	mov	x8, #0x62                  	// #98
   126f8:	d4000001 	svc	#0x0
   126fc:	17ffffa2 	b	12584 <pthread_setattr_default_np+0x5c>
   12700:	d53bd040 	mrs	x0, tpidr_el0
   12704:	d11bc000 	sub	x0, x0, #0x6f0
   12708:	b940d000 	ldr	w0, [x0,#208]
   1270c:	97fffb6e 	bl	114c4 <__determine_cpumask_size>
   12710:	2a0003e4 	mov	w4, w0
   12714:	35fff380 	cbnz	w0, 12584 <pthread_setattr_default_np+0x5c>
   12718:	f941d6c1 	ldr	x1, [x22,#936]
   1271c:	17ffffac 	b	125cc <pthread_setattr_default_np+0xa4>
   12720:	f9401400 	ldr	x0, [x0,#40]
   12724:	97ffcabf 	bl	5220 <free@plt>
   12728:	17ffffdc 	b	12698 <pthread_setattr_default_np+0x170>
   1272c:	f9401419 	ldr	x25, [x0,#40]
   12730:	aa1703e2 	mov	x2, x23
   12734:	f9401681 	ldr	x1, [x20,#40]
   12738:	aa1903e0 	mov	x0, x25
   1273c:	97ffca09 	bl	4f60 <memcpy@plt>
   12740:	17ffffd6 	b	12698 <pthread_setattr_default_np+0x170>
   12744:	52800184 	mov	w4, #0xc                   	// #12
   12748:	17ffffe1 	b	126cc <pthread_setattr_default_np+0x1a4>

000000000001274c <pthread_getattr_default_np>:
   1274c:	a9bd7bfd 	stp	x29, x30, [sp,#-48]!
   12750:	52800021 	mov	w1, #0x1                   	// #1
   12754:	910003fd 	mov	x29, sp
   12758:	a90153f3 	stp	x19, x20, [sp,#16]
   1275c:	d0000113 	adrp	x19, 34000 <__GI___pthread_keys+0x3d78>
   12760:	aa0003f4 	mov	x20, x0
   12764:	b9002fbf 	str	wzr, [x29,#44]
   12768:	910a2260 	add	x0, x19, #0x288
   1276c:	885ffc02 	ldaxr	w2, [x0]
   12770:	6b1f005f 	cmp	w2, wzr
   12774:	54000061 	b.ne	12780 <pthread_getattr_default_np+0x34>
   12778:	88037c01 	stxr	w3, w1, [x0]
   1277c:	35ffff83 	cbnz	w3, 1276c <pthread_getattr_default_np+0x20>
   12780:	540002c1 	b.ne	127d8 <pthread_getattr_default_np+0x8c>
   12784:	d0000101 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
   12788:	910a2262 	add	x2, x19, #0x288
   1278c:	910cc021 	add	x1, x1, #0x330
   12790:	a9401424 	ldp	x4, x5, [x1]
   12794:	a9001684 	stp	x4, x5, [x20]
   12798:	a9411424 	ldp	x4, x5, [x1,#16]
   1279c:	a9011684 	stp	x4, x5, [x20,#16]
   127a0:	a9421424 	ldp	x4, x5, [x1,#32]
   127a4:	a9021684 	stp	x4, x5, [x20,#32]
   127a8:	f9401820 	ldr	x0, [x1,#48]
   127ac:	52800001 	mov	w1, #0x0                   	// #0
   127b0:	f9001a80 	str	x0, [x20,#48]
   127b4:	885f7c40 	ldxr	w0, [x2]
   127b8:	8803fc41 	stlxr	w3, w1, [x2]
   127bc:	35ffffc3 	cbnz	w3, 127b4 <pthread_getattr_default_np+0x68>
   127c0:	7100041f 	cmp	w0, #0x1
   127c4:	5400010c 	b.gt	127e4 <pthread_getattr_default_np+0x98>
   127c8:	52800000 	mov	w0, #0x0                   	// #0
   127cc:	a94153f3 	ldp	x19, x20, [sp,#16]
   127d0:	a8c37bfd 	ldp	x29, x30, [sp],#48
   127d4:	d65f03c0 	ret
   127d8:	b9002fa2 	str	w2, [x29,#44]
   127dc:	97fff3e3 	bl	f768 <__lll_lock_wait_private>
   127e0:	17ffffe9 	b	12784 <pthread_getattr_default_np+0x38>
   127e4:	aa0203e0 	mov	x0, x2
   127e8:	d2801021 	mov	x1, #0x81                  	// #129
   127ec:	d2800022 	mov	x2, #0x1                   	// #1
   127f0:	d2800003 	mov	x3, #0x0                   	// #0
   127f4:	d2800c48 	mov	x8, #0x62                  	// #98
   127f8:	d4000001 	svc	#0x0
   127fc:	52800000 	mov	w0, #0x0                   	// #0
   12800:	a94153f3 	ldp	x19, x20, [sp,#16]
   12804:	a8c37bfd 	ldp	x29, x30, [sp],#48
   12808:	d65f03c0 	ret

000000000001280c <__errno_location>:
   1280c:	b00000e1 	adrp	x1, 2f000 <__FRAME_END__+0x18e30>
   12810:	f947c421 	ldr	x1, [x1,#3976]
   12814:	d53bd040 	mrs	x0, tpidr_el0
   12818:	8b010000 	add	x0, x0, x1
   1281c:	d65f03c0 	ret

Disassembly of section __libc_freeres_fn:

0000000000012820 <nptl_freeres>:
   12820:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
   12824:	910003fd 	mov	x29, sp
   12828:	94000004 	bl	12838 <__unwind_freeres>
   1282c:	a8c17bfd 	ldp	x29, x30, [sp],#16
   12830:	d2800000 	mov	x0, #0x0                   	// #0
   12834:	17ffccdb 	b	5ba0 <__free_stacks>

0000000000012838 <__unwind_freeres>:
   12838:	d0000101 	adrp	x1, 34000 <__GI___pthread_keys+0x3d78>
   1283c:	f9417420 	ldr	x0, [x1,#744]
   12840:	b4000060 	cbz	x0, 1284c <__unwind_freeres+0x14>
   12844:	f901743f 	str	xzr, [x1,#744]
   12848:	17ffc9e6 	b	4fe0 <__libc_dlclose@plt>
   1284c:	d65f03c0 	ret

Disassembly of section .fini:

0000000000012850 <_fini>:
   12850:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
   12854:	910003fd 	mov	x29, sp
   12858:	a8c17bfd 	ldp	x29, x30, [sp],#16
   1285c:	d65f03c0 	ret
