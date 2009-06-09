	IMPLICIT NONE

	INTEGER test1_36_globalVariable1, test1_36_globalVariable2,
     & test1_36_globalVariable3,
     & test1_36_globalVariable4, test1_36_globalVariable5,
     & test1_36_globalVariable6,
     & test1_36_globalVariable7, test1_36_globalVariable8,
     & test1_36_globalVariable9,
     & test1_36_globalVariable10, passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_36f_globals/ test1_36_globalVariable1,
     & test1_36_globalVariable2, test1_36_globalVariable3,
     & test1_36_globalVariable4, test1_36_globalVariable5,
     & test1_36_globalVariable6,
     & test1_36_globalVariable7, test1_36_globalVariable8,
     & test1_36_globalVariable9,
     & test1_36_globalVariable10,
     & passedTest

