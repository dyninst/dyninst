set (SRC ${PROJECT_SOURCE_DIR}/src)
find_package (Dyninst REQUIRED COMPONENTS common OPTIONAL_COMPONENTS symtabAPI dyninstAPI instructionAPI proccontrol)
set (SOURCE_LIST_0 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test2_6_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_1 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test4_1_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_2 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_7F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_7F_fortran.F
 )
set (SOURCE_LIST_3 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_temp_detach_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_4 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_hw_breakpoint_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_5 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_thread_8_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_6 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_12_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_7 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_6_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_8 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_thread_cont_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_9 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_mem_8_mutatee.c
 	${SRC}/dyninst/test_mem_util.c
 	${SRC}/dyninst/test6LS-x86.asm
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_10 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_32F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_32F_fortran.F
 )
set (SOURCE_LIST_11 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_exec_targ_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_12 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_11F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_11F_fortran.F
 )
set (SOURCE_LIST_13 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_25F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_25F_fortran.F
 )
set (SOURCE_LIST_14 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_library_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_15 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_thread_1_mutatee.c
 	${SRC}/dyninst/test_thread.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_16 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_mem_6_mutatee.c
 	${SRC}/dyninst/test_mem_util.c
 	${SRC}/dyninst/test6LS-x86.asm
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_17 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_thread_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_18 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_mem_2_mutatee.c
 	${SRC}/dyninst/test_mem_util.c
 	${SRC}/dyninst/test6LS-x86.asm
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_19 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_mem_1_mutatee.c
 	${SRC}/dyninst/test_mem_util.c
 	${SRC}/dyninst/test6LS-x86.asm
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_20 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_10F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_10F_fortran.F
 )
set (SOURCE_LIST_21 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_launch_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_22 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_mem_3_mutatee.c
 	${SRC}/dyninst/test_mem_util.c
 	${SRC}/dyninst/test6LS-x86.asm
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_23 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_16F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_16F_fortran.F
 )
set (SOURCE_LIST_24 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_8_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_25 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_stack_1_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_26 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_stack_2_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_27 
	${SRC}/mutatee_driver.c	${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/symtab_group_test_group.c
	${SRC}/symtab/test_lookup_func_mutatee.c
 	${SRC}/symtab/test_lookup_var_mutatee.c
 	${SRC}/symtab/test_line_info_mutatee.c
 	${SRC}/symtab/test_module_mutatee.c
 	${SRC}/symtab/test_relocations_mutatee.c
 	${SRC}/symtab/test_symtab_ser_funcs_mutatee.c
 	${SRC}/symtab/test_ser_anno_mutatee.c
 	${SRC}/symtab/test_type_info_mutatee.c
 	${SRC}/symtab/test_anno_basic_types_mutatee.c
 	${SRC}/symtab/test_add_symbols_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_28 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_5_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_29 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_thread_7_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_30 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_36F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_36F_fortran.F
 )
set (SOURCE_LIST_31 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test4_3b_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_32 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_mem_4_mutatee.c
 	${SRC}/dyninst/test_mem_util.c
 	${SRC}/dyninst/test6LS-x86.asm
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_33 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test3_6_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_34 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_mem_5_mutatee.c
 	${SRC}/dyninst/test_mem_util.c
 	${SRC}/dyninst/test6LS-x86.asm
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_35 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_17F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_17F_fortran.F
 )
set (SOURCE_LIST_36 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_thread_6_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_37 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_mem_7_mutatee.c
 	${SRC}/dyninst/test_mem_util.c
 	${SRC}/dyninst/test6LS-x86.asm
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_38 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_2F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_2F_fortran.F
 )
set (SOURCE_LIST_39 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_19_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_40 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_7_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_41 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_12F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_12F_fortran.F
 )
set (SOURCE_LIST_42 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_terminate_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_43 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test3_1_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_44 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_singlestep_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_45 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_29F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_29F_fortran.F
 )
set (SOURCE_LIST_46 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test3_2_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_47 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_40_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_48 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_9_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_49 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_9F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_9F_fortran.F
 )
set (SOURCE_LIST_50 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test4_4_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_51 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_addlibrary_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_52 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_20F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_20F_fortran.F
 )
set (SOURCE_LIST_53 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_34F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_34F_fortran.F
 )
set (SOURCE_LIST_54 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test3_4_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_55 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_5F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_5F_fortran.F
 )
set (SOURCE_LIST_56 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test4_2_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_57 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_1F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_1F_fortran.F
 )
set (SOURCE_LIST_58 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_29_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_59 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_13_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_60 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_detach_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_61 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_thread_3_mutatee.c
 	${SRC}/dyninst/test_thread.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_62 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_callback_1_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_63 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_31F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_31F_fortran.F
 )
set (SOURCE_LIST_64 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test2_8_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_65 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_6F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_6F_fortran.F
 )
set (SOURCE_LIST_66 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_callback_2_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_67 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test4_4b_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_68 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_breakpoint_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_69 
	${SRC}/mutatee_driver.c	${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/dyninst_group_test_group.c
	${SRC}/dyninst/test1_1_mutatee.c
 	${SRC}/dyninst/test1_2_mutatee.c
 	${SRC}/dyninst/test1_3_mutatee.c
 	${SRC}/dyninst/test1_4_mutatee.c
 	${SRC}/dyninst/test1_5_mutatee.c
 	${SRC}/dyninst/test1_6_mutatee.c
 	${SRC}/dyninst/test1_7_mutatee.c
 	${SRC}/dyninst/test1_8_mutatee.c
 	${SRC}/dyninst/test1_9_mutatee.c
 	${SRC}/dyninst/test1_10_mutatee.c
 	${SRC}/dyninst/test1_11_mutatee.c
 	${SRC}/dyninst/test1_13_mutatee.c
 	${SRC}/dyninst/test1_16_mutatee.c
 	${SRC}/dyninst/test1_17_mutatee.c
 	${SRC}/dyninst/test1_18_mutatee.c
 	${SRC}/dyninst/test1_20_mutatee.c
 	${SRC}/dyninst/test1_21_mutatee.c
 	${SRC}/dyninst/test1_22_mutatee.c
 	${SRC}/dyninst/snip_ref_shlib_var_mutatee.c
 	${SRC}/dyninst/snip_change_shlib_var_mutatee.c
 	${SRC}/dyninst/test1_23_mutatee.c
 	${SRC}/dyninst/test1_24_mutatee.c
 	${SRC}/dyninst/test1_25_mutatee.c
 	${SRC}/dyninst/test1_26_mutatee.c
 	${SRC}/dyninst/test1_27_mutatee.c
 	${SRC}/dyninst/test1_28_mutatee.c
 	${SRC}/dyninst/test1_30_mutatee.c
 	${SRC}/dyninst/test1_31_mutatee.c
 	${SRC}/dyninst/test1_32_mutatee.c
 	${SRC}/dyninst/test1_33_mutatee.c
 	${SRC}/dyninst/test1_34_mutatee.c
 	${SRC}/dyninst/test1_36_mutatee.c
 	${SRC}/dyninst/test1_37_mutatee.c
 	${SRC}/dyninst/test1_38_mutatee.c
 	${SRC}/dyninst/test1_39_mutatee.c
 	${SRC}/dyninst/test2_5_mutatee.c
 	${SRC}/dyninst/test2_7_mutatee.c
 	${SRC}/dyninst/test2_9_mutatee.c
 	${SRC}/dyninst/test2_11_mutatee.c
 	${SRC}/dyninst/test2_12_mutatee.c
 	${SRC}/dyninst/test2_13_mutatee.c
 	${SRC}/dyninst/test_write_param_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_70 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_14_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_71 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_10_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_72 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_snip_remove_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_73 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_3F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_3F_fortran.F
 )
set (SOURCE_LIST_74 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/init_fini_callback_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_75 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test2_14_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_76 
	${SRC}/mutatee_driver.c	${SRC}/symtab/test_exception_mutatee.C
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_77 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_18F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_18F_fortran.F
 )
set (SOURCE_LIST_78 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_12_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_79 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_irpc_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_80 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_fork_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_81 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_13F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_13F_fortran.F
 )
set (SOURCE_LIST_82 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test3_7_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_83 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_41_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_84 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_stat_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_85 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_stack_4_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_86 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_thread_5_mutatee.c
 	${SRC}/dyninst/test_thread.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_87 
	${SRC}/mutatee_driver.c	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_88 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_11_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_89 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_19F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_19F_fortran.F
 )
set (SOURCE_LIST_90 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_thread_2_mutatee.c
 	${SRC}/dyninst/test_thread.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_91 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_35_mutatee.c
 	${SRC}/dyninst/call35_1_x86_linux.s
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_92 
	${SRC}/mutatee_driver.c	${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/dyninst_cxx_group_test_group.c
	${SRC}/dyninst/test5_1_mutatee.C
 	${SRC}/dyninst/test5_2_mutatee.C
 	${SRC}/dyninst/test5_3_mutatee.C
 	${SRC}/dyninst/test5_4_mutatee.C
 	${SRC}/dyninst/test5_5_mutatee.C
 	${SRC}/dyninst/test5_6_mutatee.C
 	${SRC}/dyninst/test5_7_mutatee.C
 	${SRC}/dyninst/test5_8_mutatee.C
 	${SRC}/dyninst/test5_9_mutatee.C
 	${SRC}/dyninst/cpp_test.C
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_93 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_groups_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_94 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test3_5_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_95 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_fork_14_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_96 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test3_3_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_97 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_terminate_stopped_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_98 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test_stack_3_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_99 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_fork_exec_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_100 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_14F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_14F_fortran.F
 )
set (SOURCE_LIST_101 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_8F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_8F_fortran.F
 )
set (SOURCE_LIST_102 
	${SRC}/mutatee_driver.c	${SRC}/proccontrol/pc_tls_mutatee.c
 	${SRC}/proccontrol/pcontrol_mutatee_tools.c
 	${SRC}/mutatee_util.c
 	${SRC}/mutatee_util_mt.c
 )
set (SOURCE_LIST_103 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test4_3_mutatee.c
 	${SRC}/mutatee_util.c
 )
set (SOURCE_LIST_104 
	${SRC}/mutatee_driver.c	${SRC}/dyninst/test1_4F_mutatee.c
 	${SRC}/mutatee_util.c
 	${SRC}/test1_4F_fortran.F
 )
set_property (SOURCE ${SRC}/dyninst/test5_3_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_3)
set_property (SOURCE ${SRC}/dyninst/test5_3_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_13F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_13F)
set_property (SOURCE ${SRC}/dyninst/test1_13F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pcontrol_mutatee_tools.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pcontrol_mutat)
set_property (SOURCE ${SRC}/proccontrol/pcontrol_mutatee_tools.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_terminate_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_terminate)
set_property (SOURCE ${SRC}/proccontrol/pc_terminate_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test4_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test4_1)
set_property (SOURCE ${SRC}/dyninst/test4_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_24_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_24)
set_property (SOURCE ${SRC}/dyninst/test1_24_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/snip_ref_shlib_var_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=snip_ref_shlib_var)
set_property (SOURCE ${SRC}/dyninst/snip_ref_shlib_var_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_mem_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_mem_5)
set_property (SOURCE ${SRC}/dyninst/test_mem_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_14_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_14)
set_property (SOURCE ${SRC}/dyninst/test1_14_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_18F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_18F)
set_property (SOURCE ${SRC}/dyninst/test1_18F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_18_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_18)
set_property (SOURCE ${SRC}/dyninst/test1_18_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/symtab/test_module_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_module)
set_property (SOURCE ${SRC}/symtab/test_module_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_callback_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_callback_2)
set_property (SOURCE ${SRC}/dyninst/test_callback_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test2_14_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_14)
set_property (SOURCE ${SRC}/dyninst/test2_14_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/symtab/test_lookup_var_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_lookup_var)
set_property (SOURCE ${SRC}/symtab/test_lookup_var_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/symtab/test_ser_anno_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_ser_anno)
set_property (SOURCE ${SRC}/symtab/test_ser_anno_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_9F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_9F)
set_property (SOURCE ${SRC}/dyninst/test1_9F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_19_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_19)
set_property (SOURCE ${SRC}/dyninst/test1_19_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_27_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_27)
set_property (SOURCE ${SRC}/dyninst/test1_27_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_stack_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_stack_3)
set_property (SOURCE ${SRC}/dyninst/test_stack_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_5F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_5F)
set_property (SOURCE ${SRC}/dyninst/test1_5F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_37_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_37)
set_property (SOURCE ${SRC}/dyninst/test1_37_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_10_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_10)
set_property (SOURCE ${SRC}/dyninst/test1_10_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/proccontrol/pc_irpc_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_irpc)
set_property (SOURCE ${SRC}/proccontrol/pc_irpc_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_fork_13_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_13)
set_property (SOURCE ${SRC}/dyninst/test_fork_13_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_mem_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_mem_1)
set_property (SOURCE ${SRC}/dyninst/test_mem_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/symtab/test_lookup_func_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_lookup_func)
set_property (SOURCE ${SRC}/symtab/test_lookup_func_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/proccontrol/pc_terminate_stopped_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_terminate_stopped)
set_property (SOURCE ${SRC}/proccontrol/pc_terminate_stopped_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_thread_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_thread_8)
set_property (SOURCE ${SRC}/dyninst/test_thread_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_39_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_39)
set_property (SOURCE ${SRC}/dyninst/test1_39_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_1)
set_property (SOURCE ${SRC}/dyninst/test1_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_16F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_16F)
set_property (SOURCE ${SRC}/dyninst/test1_16F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/init_fini_callback_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=init_fini_callback)
set_property (SOURCE ${SRC}/dyninst/init_fini_callback_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test2_13_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_13)
set_property (SOURCE ${SRC}/dyninst/test2_13_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_34F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_34F)
set_property (SOURCE ${SRC}/dyninst/test1_34F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_stat_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_stat)
set_property (SOURCE ${SRC}/proccontrol/pc_stat_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test4_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test4_2)
set_property (SOURCE ${SRC}/dyninst/test4_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test3_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test3_6)
set_property (SOURCE ${SRC}/dyninst/test3_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_1F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_1F)
set_property (SOURCE ${SRC}/dyninst/test1_1F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_7)
set_property (SOURCE ${SRC}/dyninst/test1_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test5_1_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_1)
set_property (SOURCE ${SRC}/dyninst/test5_1_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/symtab/test_line_info_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_line_info)
set_property (SOURCE ${SRC}/symtab/test_line_info_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_41_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_41)
set_property (SOURCE ${SRC}/dyninst/test1_41_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_12_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_12)
set_property (SOURCE ${SRC}/dyninst/test1_12_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_25F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_25F)
set_property (SOURCE ${SRC}/dyninst/test1_25F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_2F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_2F)
set_property (SOURCE ${SRC}/dyninst/test1_2F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_20F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_20F)
set_property (SOURCE ${SRC}/dyninst/test1_20F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_launch_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_launch)
set_property (SOURCE ${SRC}/proccontrol/pc_launch_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_32_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_32)
set_property (SOURCE ${SRC}/dyninst/test1_32_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_36F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_36F)
set_property (SOURCE ${SRC}/dyninst/test1_36F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test2_9_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_9)
set_property (SOURCE ${SRC}/dyninst/test2_9_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_fork_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_6)
set_property (SOURCE ${SRC}/dyninst/test_fork_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_26_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_26)
set_property (SOURCE ${SRC}/dyninst/test1_26_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_callback_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_callback_1)
set_property (SOURCE ${SRC}/dyninst/test_callback_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test5_8_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_8)
set_property (SOURCE ${SRC}/dyninst/test5_8_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test4_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test4_3)
set_property (SOURCE ${SRC}/dyninst/test4_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/cpp_test.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=)
set_property (SOURCE ${SRC}/dyninst/cpp_test.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_9_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_9)
set_property (SOURCE ${SRC}/dyninst/test1_9_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_stack_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_stack_2)
set_property (SOURCE ${SRC}/dyninst/test_stack_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_singlestep_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_singlestep)
set_property (SOURCE ${SRC}/proccontrol/pc_singlestep_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_library_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_library)
set_property (SOURCE ${SRC}/proccontrol/pc_library_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_34_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_34)
set_property (SOURCE ${SRC}/dyninst/test1_34_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test3_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test3_2)
set_property (SOURCE ${SRC}/dyninst/test3_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test5_4_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_4)
set_property (SOURCE ${SRC}/dyninst/test5_4_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_mem_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_mem_7)
set_property (SOURCE ${SRC}/dyninst/test_mem_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_fork_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_fork)
set_property (SOURCE ${SRC}/proccontrol/pc_fork_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_31_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_31)
set_property (SOURCE ${SRC}/dyninst/test1_31_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/proccontrol/pc_tls_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_tls)
set_property (SOURCE ${SRC}/proccontrol/pc_tls_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_29F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_29F)
set_property (SOURCE ${SRC}/dyninst/test1_29F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_3F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_3F)
set_property (SOURCE ${SRC}/dyninst/test1_3F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test5_5_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_5)
set_property (SOURCE ${SRC}/dyninst/test5_5_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test5_2_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_2)
set_property (SOURCE ${SRC}/dyninst/test5_2_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_20_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_20)
set_property (SOURCE ${SRC}/dyninst/test1_20_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_19F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_19F)
set_property (SOURCE ${SRC}/dyninst/test1_19F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test3_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test3_5)
set_property (SOURCE ${SRC}/dyninst/test3_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_17F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_17F)
set_property (SOURCE ${SRC}/dyninst/test1_17F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_temp_detach_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_temp_detach)
set_property (SOURCE ${SRC}/proccontrol/pc_temp_detach_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_38_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_38)
set_property (SOURCE ${SRC}/dyninst/test1_38_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test5_7_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_7)
set_property (SOURCE ${SRC}/dyninst/test5_7_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/proccontrol/pc_thread_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_thread)
set_property (SOURCE ${SRC}/proccontrol/pc_thread_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test4_3b_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test4_3b)
set_property (SOURCE ${SRC}/dyninst/test4_3b_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_fork_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_5)
set_property (SOURCE ${SRC}/dyninst/test_fork_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/snip_change_shlib_var_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=snip_change_shlib_var)
set_property (SOURCE ${SRC}/dyninst/snip_change_shlib_var_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test5_9_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_9)
set_property (SOURCE ${SRC}/dyninst/test5_9_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_2)
set_property (SOURCE ${SRC}/dyninst/test1_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_25_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_25)
set_property (SOURCE ${SRC}/dyninst/test1_25_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_fork_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_7)
set_property (SOURCE ${SRC}/dyninst/test_fork_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_8)
set_property (SOURCE ${SRC}/dyninst/test1_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_22_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_22)
set_property (SOURCE ${SRC}/dyninst/test1_22_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_fork_14_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_14)
set_property (SOURCE ${SRC}/dyninst/test_fork_14_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_31F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_31F)
set_property (SOURCE ${SRC}/dyninst/test1_31F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_snip_remove_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_snip_remove)
set_property (SOURCE ${SRC}/dyninst/test_snip_remove_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_5)
set_property (SOURCE ${SRC}/dyninst/test1_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test3_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test3_3)
set_property (SOURCE ${SRC}/dyninst/test3_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_7F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_7F)
set_property (SOURCE ${SRC}/dyninst/test1_7F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/symtab/test_type_info_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_type_info)
set_property (SOURCE ${SRC}/symtab/test_type_info_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_8F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_8F)
set_property (SOURCE ${SRC}/dyninst/test1_8F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_mem_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_mem_8)
set_property (SOURCE ${SRC}/dyninst/test_mem_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_thread_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_thread_7)
set_property (SOURCE ${SRC}/dyninst/test_thread_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_13_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_13)
set_property (SOURCE ${SRC}/dyninst/test1_13_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_12F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_12F)
set_property (SOURCE ${SRC}/dyninst/test1_12F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_fork_10_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_10)
set_property (SOURCE ${SRC}/dyninst/test_fork_10_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_4F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_4F)
set_property (SOURCE ${SRC}/dyninst/test1_4F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test3_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test3_1)
set_property (SOURCE ${SRC}/dyninst/test3_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_thread_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_thread_1)
set_property (SOURCE ${SRC}/dyninst/test_thread_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_11F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_11F)
set_property (SOURCE ${SRC}/dyninst/test1_11F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_thread_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_thread_3)
set_property (SOURCE ${SRC}/dyninst/test_thread_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test2_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_7)
set_property (SOURCE ${SRC}/dyninst/test2_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_30_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_30)
set_property (SOURCE ${SRC}/dyninst/test1_30_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/symtab/test_relocations_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_relocations)
set_property (SOURCE ${SRC}/symtab/test_relocations_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/proccontrol/pc_fork_exec_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_fork_exec)
set_property (SOURCE ${SRC}/proccontrol/pc_fork_exec_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_stack_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_stack_1)
set_property (SOURCE ${SRC}/dyninst/test_stack_1_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test2_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_6)
set_property (SOURCE ${SRC}/dyninst/test2_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_23_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_23)
set_property (SOURCE ${SRC}/dyninst/test1_23_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_stack_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_stack_4)
set_property (SOURCE ${SRC}/dyninst/test_stack_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_mem_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_mem_6)
set_property (SOURCE ${SRC}/dyninst/test_mem_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test3_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test3_4)
set_property (SOURCE ${SRC}/dyninst/test3_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/symtab/test_exception_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_exception)
set_property (SOURCE ${SRC}/symtab/test_exception_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_6F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_6F)
set_property (SOURCE ${SRC}/dyninst/test1_6F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_groups_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_groups)
set_property (SOURCE ${SRC}/proccontrol/pc_groups_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_fork_11_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_11)
set_property (SOURCE ${SRC}/dyninst/test_fork_11_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/symtab/test_anno_basic_types_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_anno_basic_types)
set_property (SOURCE ${SRC}/symtab/test_anno_basic_types_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test6LS-x86.asm APPEND PROPERTY COMPILE_FLAGS -dPLATFORM=i386-unknown-linux2.4)
set_property (SOURCE ${SRC}/dyninst/test1_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_4)
set_property (SOURCE ${SRC}/dyninst/test1_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test2_11_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_11)
set_property (SOURCE ${SRC}/dyninst/test2_11_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/symtab/test_add_symbols_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_add_symbols)
set_property (SOURCE ${SRC}/symtab/test_add_symbols_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_40_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_40)
set_property (SOURCE ${SRC}/dyninst/test1_40_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_28_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_28)
set_property (SOURCE ${SRC}/dyninst/test1_28_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_fork_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_8)
set_property (SOURCE ${SRC}/dyninst/test_fork_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/symtab/test_symtab_ser_funcs_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_symtab_ser_funcs)
set_property (SOURCE ${SRC}/symtab/test_symtab_ser_funcs_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_thread.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=tes)
set_property (SOURCE ${SRC}/dyninst/test_thread.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_thread_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_thread_6)
set_property (SOURCE ${SRC}/dyninst/test_thread_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_thread_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_thread_5)
set_property (SOURCE ${SRC}/dyninst/test_thread_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_33_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_33)
set_property (SOURCE ${SRC}/dyninst/test1_33_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test2_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_8)
set_property (SOURCE ${SRC}/dyninst/test2_8_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test3_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test3_7)
set_property (SOURCE ${SRC}/dyninst/test3_7_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_mem_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_mem_3)
set_property (SOURCE ${SRC}/dyninst/test_mem_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_16_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_16)
set_property (SOURCE ${SRC}/dyninst/test1_16_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_32F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_32F)
set_property (SOURCE ${SRC}/dyninst/test1_32F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_write_param_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_write_param)
set_property (SOURCE ${SRC}/dyninst/test_write_param_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_mem_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_mem_4)
set_property (SOURCE ${SRC}/dyninst/test_mem_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_exec_targ_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_exec_targ)
set_property (SOURCE ${SRC}/proccontrol/pc_exec_targ_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test4_4b_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test4_4b)
set_property (SOURCE ${SRC}/dyninst/test4_4b_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_breakpoint_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_breakpoint)
set_property (SOURCE ${SRC}/proccontrol/pc_breakpoint_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_36_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_36)
set_property (SOURCE ${SRC}/dyninst/test1_36_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test2_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_5)
set_property (SOURCE ${SRC}/dyninst/test2_5_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_3)
set_property (SOURCE ${SRC}/dyninst/test1_3_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_mem_util.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_)
set_property (SOURCE ${SRC}/dyninst/test_mem_util.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_hw_breakpoint_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_hw_breakpoint)
set_property (SOURCE ${SRC}/proccontrol/pc_hw_breakpoint_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_fork_12_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_12)
set_property (SOURCE ${SRC}/dyninst/test_fork_12_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_35_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_35)
set_property (SOURCE ${SRC}/dyninst/test1_35_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test5_6_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test5_6)
set_property (SOURCE ${SRC}/dyninst/test5_6_mutatee.C APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_17_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_17)
set_property (SOURCE ${SRC}/dyninst/test1_17_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/proccontrol/pc_detach_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_detach)
set_property (SOURCE ${SRC}/proccontrol/pc_detach_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_10F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_10F)
set_property (SOURCE ${SRC}/dyninst/test1_10F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_14F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_14F)
set_property (SOURCE ${SRC}/dyninst/test1_14F_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_6)
set_property (SOURCE ${SRC}/dyninst/test1_6_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test1_21_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_21)
set_property (SOURCE ${SRC}/dyninst/test1_21_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test4_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test4_4)
set_property (SOURCE ${SRC}/dyninst/test4_4_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_11_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_11)
set_property (SOURCE ${SRC}/dyninst/test1_11_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/proccontrol/pc_addlibrary_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_addlibrary)
set_property (SOURCE ${SRC}/proccontrol/pc_addlibrary_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test2_12_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test2_12)
set_property (SOURCE ${SRC}/dyninst/test2_12_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=1)
set_property (SOURCE ${SRC}/dyninst/test_mem_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_mem_2)
set_property (SOURCE ${SRC}/dyninst/test_mem_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test1_29_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test1_29)
set_property (SOURCE ${SRC}/dyninst/test1_29_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_thread_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_thread_2)
set_property (SOURCE ${SRC}/dyninst/test_thread_2_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/dyninst/test_fork_9_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=test_fork_9)
set_property (SOURCE ${SRC}/dyninst/test_fork_9_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
set_property (SOURCE ${SRC}/proccontrol/pc_thread_cont_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=pc_thread_cont)
set_property (SOURCE ${SRC}/proccontrol/pc_thread_cont_mutatee.c APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=0)
