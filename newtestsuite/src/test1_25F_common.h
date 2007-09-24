	IMPLICIT NONE

	INTEGER test1_25_globalVariable1,

#if defined(alpha_dec_osf4_0)
     & test1_25_globalVariable2*8,
#else
     & test1_25_globalVariable2,
#endif
     & test1_25_globalVariable3, test1_25_globalVariable4,
     & test1_25_globalVariable5,
     & test1_25_globalVariable6, test1_25_globalVariable7, passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_25f_globals/ test1_25_globalVariable1, test1_25_globalVariable2,
     & test1_25_globalVariable3,
     & test1_25_globalVariable4, test1_25_globalVariable5,
     & test1_25_globalVariable6,
     & test1_25_globalVariable7,
     & passedTest

