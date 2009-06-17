	IMPLICIT NONE

#if !defined (F77)
	TYPE struct26_1
		INTEGER field1, field2
	END TYPE struct26_1

	TYPE struct26_2
		INTEGER field1, field2, field3 (10)
		TYPE (struct26_1) field4
	END TYPE struct26_2
#endif

	INTEGER test1_26_globalVariable2,
     & test1_26_globalVariable3, test1_26_globalVariable4,
     & test1_26_globalVariable5,
     & test1_26_globalVariable6, test1_26_globalVariable7,
     & test1_26_globalVariable8,
     & test1_26_globalVariable9, test1_26_globalVariable10, 
     & test1_26_globalVariable11, test1_26_globalVariable12,
     & test1_26_globalVariable13, passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_26f_globals/ test1_26_globalVariable2,
     & test1_26_globalVariable3, test1_26_globalVariable4,
     & test1_26_globalVariable5,
     & test1_26_globalVariable6, test1_26_globalVariable7,
     & test1_26_globalVariable8,
     & test1_26_globalVariable9, test1_26_globalVariable10,
     & test1_26_globalVariable11,
     & test1_26_globalVariable12, test1_26_globalVariable13, passedTest

