#ifndef ROSE_POWERPC_INSTRUCTION_ENUM_H
#define ROSE_POWERPC_INSTRUCTION_ENUM_H

#include <string>

// DQ (10/11/2008): Started work to support PowerPC Instruction Set for BGL Performance Tool
// JJW (10/12/2008): Cleaned up so automatic enum printer generator would work
enum PowerpcInstructionKind
   {
     powerpc_unknown_instruction = 0,
     powerpc_add,                                       // Add
     powerpc_add_record,                                // Add
     powerpc_addo,                                      // Add
     powerpc_addo_record,                               // Add
     powerpc_addc,                                      // Add Carrying
     powerpc_addc_record,                               // Add Carrying
     powerpc_addco,                                     // Add Carrying
     powerpc_addco_record,                              // Add Carrying
     powerpc_adde,                                      // Add Extended
     powerpc_adde_record,                               // Add Extended
     powerpc_addeo,                                     // Add Extended
     powerpc_addeo_record,                              // Add Extended
     powerpc_addi,                                      // Add Immediate
     powerpc_addic,                                     // Add Immediate Carrying
     powerpc_addic_record,                              // Add Immediate Carrying and Record
     powerpc_addis,                                     // Add Immediate Shifted
     powerpc_addme,                                     // Add to Minus One Extended
     powerpc_addme_record,                              // Add to Minus One Extended
     powerpc_addmeo,                                    // Add to Minus One Extended
     powerpc_addmeo_record,                             // Add to Minus One Extended
     powerpc_addze,                                     // Add to Zero Extended
     powerpc_addze_record,                              // Add to Zero Extended
     powerpc_addzeo,                                    // Add to Zero Extended
     powerpc_addzeo_record,                             // Add to Zero Extended
     powerpc_and,                                       // AND
     powerpc_and_record,                                // AND
     powerpc_andc,                                      // AND with Complement
     powerpc_andc_record,                               // AND with Complement
     powerpc_andi_record,                               // AND Immediate
     powerpc_andis_record,                              // AND Immediate Shifted
     powerpc_b,                                         // Branch
     powerpc_ba,                                        // Branch
     powerpc_bl,                                        // Branch
     powerpc_bla,                                       // Branch
     powerpc_bc,                                        // Branch Conditional
     powerpc_bca,                                       // Branch Conditional
     powerpc_bcl,                                       // Branch Conditional
     powerpc_bcla,                                      // Branch Conditional
     powerpc_bcctr,                                     // Branch Conditional to Count Register
     powerpc_bcctrl,                                    // Branch Conditional to Count Register
     powerpc_bclr,                                      // Branch Conditional Link Register
     powerpc_bclrl,                                     // Branch Conditional Link Register
     powerpc_cmp,                                       // Compare
     powerpc_cmpi,                                      // Compare Immediate
     powerpc_cmpl,                                      // Compare Logical
     powerpc_cmpli,                                     // Compare Logical Immediate
     powerpc_cntlzd,                                    // Count Leading Zeros Doubleword
     powerpc_cntlzd_record,                             // Count Leading Zeros Doubleword
     powerpc_cntlzw,                                    // Count Leading Zeros Word
     powerpc_cntlzw_record,                             // Count Leading Zeros Word
     powerpc_crand,                                     // Condition Register AND
     powerpc_crandc,                                    // Condition Register AND with Complement
     powerpc_creqv,                                     // Condition Register Equivalent
     powerpc_crnand,                                    // Condition Register NAND
     powerpc_crnor,                                     // Condition Register NOR
     powerpc_cror,                                      // Condition Register OR
     powerpc_crorc,                                     // Condition Register OR with Complement
     powerpc_crxor,                                     // Condition Register XOR
     powerpc_dcbf,                                      // Data Cache Block Flush
     powerpc_dcba,                                      // Data Cache Block Allocate
     powerpc_dcbi,                                      // Data Cache Block Invalidate
     powerpc_dcbst,                                     // Data Cache Block Store
     powerpc_dcbt,                                      // Data Cache Block Touch
     powerpc_dcbtst,                                    // Data Cache Block Touch for Store
     powerpc_dcbz,                                      // Data Cache Block Set to Zero
     powerpc_divd,                                      // Divide Doubleword
     powerpc_divd_record,                               // Divide Doubleword
     powerpc_divdo,                                     // Divide Doubleword
     powerpc_divdo_record,                              // Divide Doubleword
     powerpc_divdu,                                     // Divide Doubleword Unsigned
     powerpc_divdu_record,                              // Divide Doubleword Unsigned
     powerpc_divduo,                                    // Divide Doubleword Unsigned
     powerpc_divduo_record,                             // Divide Doubleword Unsigned
     powerpc_divw,                                      // Divide Word
     powerpc_divw_record,                               // Divide Word
     powerpc_divwo,                                     // Divide Word
     powerpc_divwo_record,                              // Divide Word
     powerpc_divwu,                                     // Divide Word Unsigned
     powerpc_divwu_record,                              // Divide Word Unsigned
     powerpc_divwuo,                                    // Divide Word Unsigned
     powerpc_divwuo_record,                             // Divide Word Unsigned
     powerpc_dst,                                       // Data Stream Touch
     powerpc_dstt,                                      // Data Stream Touch
     powerpc_dstst,                                     // Data Stream Touch for store
     powerpc_dststt,                                    // Data Stream Touch for store
     powerpc_dss,                                       // Data Stream Stop
     powerpc_dssall,                                    // Data Stream Stop All
     powerpc_eciwx,                                     // External Control in Word Indexed (opt.)
     powerpc_ecowx,                                     // External Control out Word Indexed (opt.)
     powerpc_eieio,                                     // Enforce In-order Execution of I/O
     powerpc_eqv,                                       // Equivalent
     powerpc_eqv_record,                                // Equivalent
     powerpc_extsb,                                     // Extend Sign Byte
     powerpc_extsb_record,                              // Extend Sign Byte
     powerpc_extsh,                                     // Extend Sign Halfword
     powerpc_extsh_record,                              // Extend Sign Halfword
     powerpc_extsw,                                     // Extend Sign Word
     powerpc_extsw_record,                              // Extend Sign Word
     powerpc_fabs,                                      // Floating Absolute Value
     powerpc_fabs_record,                               // Floating Absolute Value
     powerpc_fadd,                                      // Floating Add
     powerpc_fadd_record,                               // Floating Add
     powerpc_fadds,                                     // Floating Add Single
     powerpc_fadds_record,                              // Floating Add Single
     powerpc_fcfid,                                     // Floating Convert from Integer Doubleword
     powerpc_fcfid_record,                              // Floating Convert from Integer Doubleword
     powerpc_fcmpo,                                     // Floating Compare Ordered
     powerpc_fcmpu,                                     // Floating Compare Unordered
     powerpc_fctid,                                     // Floating Convert to Integer Doubleword
     powerpc_fctid_record,                              // Floating Convert to Integer Doubleword
     powerpc_fctidz,                                    // Floating Convert to Integer Doubleword with Round Toward Zero
     powerpc_fctidz_record,                             // Floating Convert to Integer Doubleword with Round Toward Zero
     powerpc_fctiw,                                     // Floating Convert to Integer Word
     powerpc_fctiw_record,                              // Floating Convert to Integer Word
     powerpc_fctiwz,                                    // Floating Convert to Integer Word with Round to Zero
     powerpc_fctiwz_record,                             // Floating Convert to Integer Word with Round to Zero
     powerpc_fdiv,                                      // Floating Divide
     powerpc_fdiv_record,                               // Floating Divide
     powerpc_fdivs,                                     // Floating Divide Single
     powerpc_fdivs_record,                              // Floating Divide Single
     powerpc_fmadd,                                     // Floating Multiply-Add
     powerpc_fmadd_record,                              // Floating Multiply-Add
     powerpc_fmadds,                                    // Floating Multiply-Add Single
     powerpc_fmadds_record,                             // Floating Multiply-Add Single
     powerpc_fmr,                                       // Floating Move Register
     powerpc_fmr_record,                                // Floating Move Register
     powerpc_fmsub,                                     // Floating Multiply-Subtract
     powerpc_fmsub_record,                              // Floating Multiply-Subtract
     powerpc_fmsubs,                                    // Floating Multiply-Subtract Single
     powerpc_fmsubs_record,                             // Floating Multiply-Subtract Single
     powerpc_fmul,                                      // Floating Multiply
     powerpc_fmul_record,                               // Floating Multiply
     powerpc_fmuls,                                     // Floating Multiply Single
     powerpc_fmuls_record,                              // Floating Multiply Single
     powerpc_fnabs,                                     // Floating Negative Absolute Value
     powerpc_fnabs_record,                              // Floating Negative Absolute Value
     powerpc_fneg,                                      // Floating Negate
     powerpc_fneg_record,                               // Floating Negate
     powerpc_fnmadd,                                    // Floating Negative Multiply-Add
     powerpc_fnmadd_record,                             // Floating Negative Multiply-Add
     powerpc_fnmadds,                                   // Floating Negative Multiply-Add Single
     powerpc_fnmadds_record,                            // Floating Negative Multiply-Add Single
     powerpc_fnmsub,                                    // Floating Negative Multiply-Subtract
     powerpc_fnmsub_record,                             // Floating Negative Multiply-Subtract
     powerpc_fnmsubs,                                   // Floating Negative Multiply-Subtract Single
     powerpc_fnmsubs_record,                            // Floating Negative Multiply-Subtract Single
     powerpc_fpmul,                                     // FP2 Floating Parallel Multiply (BGL specific)
     powerpc_fxmul,                                     // FP2 Floating Cross Multiply (BGL specific)
     powerpc_fxpmul,                                    // FP2 Floating Cross Copy-Primary Multiply (BGL specific)
     powerpc_fxsmul,                                    // FP2 Floating Cross Copy-Secondary Multiply (BGL specific)
     powerpc_fpadd,                                     // FP2 Floating Parallel Add (BGL specific)
     powerpc_fpsub,                                     // FP2 Floating Parallel Subtract (BGL specific)
     powerpc_fpre,                                      // FP2 Floating Parallel Reciprocal Estimate (BGL specific)
     powerpc_fprsqrte,                                  // FP2 Floating Parallel Reciprocal Square Root Estimate (BGL specific)
     powerpc_fpmr,
     powerpc_fpabs,
     powerpc_lfssx,
     powerpc_fpneg,
     powerpc_lfssux,
     powerpc_fprsp,
     powerpc_lfsdx,
     powerpc_fpnabs,
     powerpc_lfsdux,
     powerpc_lfxsx,
     powerpc_fsmr,
     powerpc_lfxsux,
     powerpc_lfxdx,
     powerpc_fsabs,
     powerpc_lfxdux,
     powerpc_lfpsx,
     powerpc_fsneg,
     powerpc_lfpsux,
     powerpc_lfpdx,
     powerpc_fsnabs,
     powerpc_lfpdux,
     powerpc_stfpiwx,
     powerpc_fxmr,
     powerpc_fpctiw,
     powerpc_stfssx,
     powerpc_stfssux,
     powerpc_fpctiwz,
     powerpc_stfsdx,
     powerpc_stfsdux,
     powerpc_stfxsx,
     powerpc_fsmtp,
     powerpc_stfxsux,
     powerpc_stfxdx,
     powerpc_stfxdux,
     powerpc_stfpsx,
     powerpc_fsmfp,
     powerpc_stfpsux,
     powerpc_stfpdx,
     powerpc_stfpdux,
     powerpc_fpsel,
     powerpc_fpmadd,
     powerpc_fpmsub,
     powerpc_fxmadd,
     powerpc_fxcpmadd,
     powerpc_fxcsmadd,
     powerpc_fpnmadd,
     powerpc_fxnmadd,
     powerpc_fxcpnmadd,
     powerpc_fxcsnmadd,
     powerpc_fxcpnpma,
     powerpc_fxmsub,
     powerpc_fxcsnpma,
     powerpc_fxcpmsub,
     powerpc_fxcpnsma,
     powerpc_fxcsmsub,
     powerpc_fxcsnsma,
     powerpc_fpnmsub,
     powerpc_fxcxma,
     powerpc_fxnmsub,
     powerpc_fxcxnpma,
     powerpc_fxcpnmsub,
     powerpc_fxcxnsma,
     powerpc_fxcsnmsub,
     powerpc_fxcxnms,                                   // Last FP2 specific enum value
     powerpc_fre,                                       // Floating Reciprocal Estimate Single (optional)
     powerpc_fre_record,                                // Floating Reciprocal Estimate Single (optional)
     powerpc_fres,                                      // Floating Reciprocal Estimate Single (optional)
     powerpc_fres_record,                               // Floating Reciprocal Estimate Single (optional)
     powerpc_frsp,                                      // Floating Round to Single Precision
     powerpc_frsp_record,                               // Floating Round to Single Precision
     powerpc_frsqrte,                                   // Floating Reciprocal Square Root Estimate (optional)
     powerpc_frsqrte_record,                            // Floating Reciprocal Square Root Estimate (optional)
     powerpc_frsqrtes,                                  // Floating Reciprocal Square Root Estimate (optional)
     powerpc_frsqrtes_record,                           // Floating Reciprocal Square Root Estimate (optional)
     powerpc_fsel,                                      // Floating-Point Select (optional)
     powerpc_fsel_record,                               // Floating-Point Select (optional)
     powerpc_fsqrt,                                     // Floating-Point Square Root (optional)
     powerpc_fsqrt_record,                              // Floating-Point Square Root (optional)
     powerpc_fsqrts,                                    // Floating-Point Square Root (optional)
     powerpc_fsqrts_record,                             // Floating-Point Square Root (optional)
     powerpc_fsub,                                      // Floating Subtract
     powerpc_fsub_record,                               // Floating Subtract
     powerpc_fsubs,                                     // Floating Subtract Single
     powerpc_fsubs_record,                              // Floating Subtract Single
     powerpc_icbi,                                      // Instruction Cache Block Invalidate
     powerpc_isync,                                     // Instruction Synchronize
     powerpc_lbz,                                       // Load Byte and Zero
     powerpc_lbzu,                                      // Load Byte and Zero with Update
     powerpc_lbzux,                                     // Load Byte and Zero with Update Indexed
     powerpc_lbzx,                                      // Load Byte and Zero Indexed
     powerpc_ld,                                        // Load Doubleword
     powerpc_ldarx,                                     // Load Doubleword and Reserve Indexed
     powerpc_ldu,                                       // Load Doubleword with Update
     powerpc_ldux,                                      // Load Doubleword with Update Indexed
     powerpc_ldx,                                       // Load Doubleword Indexed
     powerpc_lfd,                                       // Load Floating-Point Double
     powerpc_lfdu,                                      // Load Floating-Point Double with Update
     powerpc_lfdux,                                     // Load Floating-Point Double with Update Indexed
     powerpc_lfdx,                                      // Load Floating-Point Double Indexed
     powerpc_lfs,                                       // Load Floating-Point Single
     powerpc_lfsu,                                      // Load Floating-Point Single with Update
     powerpc_lfsux,                                     // Load Floating-Point Single with Update Indexed
     powerpc_lfsx,                                      // Load Floating-Point Single Indexed
     powerpc_lha,                                       // Load Half Algebraic
     powerpc_lhau,                                      // Load Half Algebraic with Update
     powerpc_lhaux,                                     // Load Half Algebraic with Update Indexed
     powerpc_lhax,                                      // Load Half Algebraic Indexed
     powerpc_lhbrx,                                     // Load Half Byte-Reversed Indexed
     powerpc_lhz,                                       // Load Half and Zero
     powerpc_lhzu,                                      // Load Half and Zero with Update
     powerpc_lhzux,                                     // Load Half and Zero with Update Indexed
     powerpc_lhzx,                                      // Load Half and Zero Indexed
     powerpc_lmw,                                       // Load Multiple Word
     powerpc_lswi,                                      // Load String Word Immediate
     powerpc_lswx,                                      // Load String Word Indexed
     powerpc_lwa,                                       // Load Word Algebraic
     powerpc_lwarx,                                     // Load Word and Reserve Indexed
     powerpc_lwaux,                                     // Load Word Algebraic with Update Indexed
     powerpc_lwax,                                      // Load Word Algebraic Indexed
     powerpc_lwbrx,                                     // Load Word Byte-Reversed Indexed
     powerpc_lwz,                                       // Load Word and Zero
     powerpc_lwzu,                                      // Load Word with Zero Update
     powerpc_lwzux,                                     // Load Word and Zero with Update Indexed
     powerpc_lwzx,                                      // Load Word and Zero Indexed
     powerpc_mcrf,                                      // Move Condition Register Field
     powerpc_mcrfs,                                     // Move to Condition Register from FPSCR
     powerpc_mcrxr,                                     // Move to Condition Register from XER
     powerpc_mfcr,                                      // Move from Condition Register
     powerpc_mffs,                                      // Move from FPSCR
     powerpc_mffs_record,                               // Move from FPSCR
     powerpc_mfmsr,                                     // Move from Machine State Register
     powerpc_mfspr,                                     // Move from Special-Purpose Register
     powerpc_mfsr,                                      // Move from Segment Register
     powerpc_mfsrin,                                    // Move from Segment Register Indirect
     powerpc_mftb,                                      // Move from Time Base
     powerpc_mtcrf,                                     // Move to Condition Register Fields
     powerpc_mtfsb0,                                    // Move to FPSCR Bit 0
     powerpc_mtfsb0_record,                             // Move to FPSCR Bit 0
     powerpc_mtfsb1,                                    // Move to FPSCR Bit 1
     powerpc_mtfsb1_record,                             // Move to FPSCR Bit 1
     powerpc_mtfsf,                                     // Move to FPSCR Fields
     powerpc_mtfsf_record,                              // Move to FPSCR Fields
     powerpc_mtfsfi,                                    // Move to FPSCR Field Immediate
     powerpc_mtfsfi_record,                             // Move to FPSCR Field Immediate
     powerpc_mtmsr,                                     // Move to Machine State Register
     powerpc_mtmsrd,                                    // Move to Machine State Register
     powerpc_mtspr,                                     // Move to Special-Purpose Register
     powerpc_mtsr,                                      // Move to Segment Register
     powerpc_mtsrd,                                     // Move to Segment Register
     powerpc_mtsrdin,                                   // Move to Segment Register Indirect
     powerpc_mtsrin,                                    // Move to Segment Register Indirect
     powerpc_mulhd,                                     // Multiply High Doubleword
     powerpc_mulhd_record,                              // Multiply High Doubleword
     powerpc_mulhdu,                                    // Multiply High Doubleword Unsigned
     powerpc_mulhdu_record,                             // Multiply High Doubleword Unsigned
     powerpc_mulhw,                                     // Multiply High Word
     powerpc_mulhw_record,                              // Multiply High Word
     powerpc_mulhwu,                                    // Multiply High Word Unsigned
     powerpc_mulhwu_record,                             // Multiply High Word Unsigned
     powerpc_mulld,                                     // Multiply Low Doubleword
     powerpc_mulld_record,                              // Multiply Low Doubleword
     powerpc_mulldo,                                    // Multiply Low Doubleword
     powerpc_mulldo_record,                             // Multiply Low Doubleword
     powerpc_mulli,                                     // Multiply Low Immediate
     powerpc_mullw,                                     // Multiply Low Word
     powerpc_mullw_record,                              // Multiply Low Word
     powerpc_mullwo,                                    // Multiply Low Word
     powerpc_mullwo_record,                             // Multiply Low Word
     powerpc_nand,                                      // NAND
     powerpc_nand_record,                               // NAND
     powerpc_neg,                                       // Negate
     powerpc_neg_record,                                // Negate
     powerpc_nego,                                      // Negate
     powerpc_nego_record,                               // Negate
     powerpc_nor,                                       // NOR
     powerpc_nor_record,                                // NOR
     powerpc_or,                                        // OR
     powerpc_or_record,                                 // OR
     powerpc_orc,                                       // OR with Complement
     powerpc_orc_record,                                // OR with Complement
     powerpc_ori,                                       // OR Immediate
     powerpc_oris,                                      // OR Immediate Shifted
     powerpc_rfi,                                       // Return from Interrupt
     powerpc_rfid,                                      // Return from Interrupt
     powerpc_rldcl,                                     // Rotate Left Doubleword then Clear Left
     powerpc_rldcl_record,                              // Rotate Left Doubleword then Clear Left
     powerpc_rldcr,                                     // Rotate Left Doubleword then Clear Right
     powerpc_rldcr_record,                              // Rotate Left Doubleword then Clear Right
     powerpc_rldic,                                     // Rotate Left Doubleword Immediate then Clear
     powerpc_rldic_record,                              // Rotate Left Doubleword Immediate then Clear
     powerpc_rldicl,                                    // Rotate Left Doubleword Immediate then Clear Left
     powerpc_rldicl_record,                             // Rotate Left Doubleword Immediate then Clear Left
     powerpc_rldicr,                                    // Rotate Left Doubleword Immediate then Clear Right
     powerpc_rldicr_record,                             // Rotate Left Doubleword Immediate then Clear Right
     powerpc_rldimi,                                    // Rotate Left Doubleword Immediate then Mask Insert
     powerpc_rldimi_record,                             // Rotate Left Doubleword Immediate then Mask Insert
     powerpc_rlwimi,                                    // Rotate Left Word Immediate then Mask Insert
     powerpc_rlwimi_record,                             // Rotate Left Word Immediate then Mask Insert
     powerpc_rlwinm,                                    // Rotate Left Word Immediate then AND with Mask
     powerpc_rlwinm_record,                             // Rotate Left Word Immediate then AND with Mask
     powerpc_rlwnm,                                     // Rotate Left Word then AND with Mask
     powerpc_rlwnm_record,                              // Rotate Left Word then AND with Mask
     powerpc_sc,                                        // System Call
     powerpc_slbia,                                     // SLB Invalidate All
     powerpc_slbie,                                     // SLB Invalidate Entry
     powerpc_sld,                                       // Shift Left Doubleword
     powerpc_sld_record,                                // Shift Left Doubleword
     powerpc_slw,                                       // Shift Left Word
     powerpc_slw_record,                                // Shift Left Word
     powerpc_srad,                                      // Shift Right Algebraic Doubleword
     powerpc_srad_record,                               // Shift Right Algebraic Doubleword
     powerpc_sradi,                                     // Shift Right Algebraic Doubleword Immediate
     powerpc_sradi_record,                              // Shift Right Algebraic Doubleword Immediate
     powerpc_srd,                                       // Shift Right Doubleword
     powerpc_srd_record,                                // Shift Right Doubleword
     powerpc_sraw,                                      // Shift Right Algebraic Word
     powerpc_sraw_record,                               // Shift Right Algebraic Word
     powerpc_srawi,                                     // Shift Right Algebraic Word Immediate
     powerpc_srawi_record,                              // Shift Right Algebraic Word Immediate
     powerpc_srw,                                       // Shift Right Word
     powerpc_srw_record,                                // Shift Right Word
     powerpc_stb,                                       // Store Byte
     powerpc_stbu,                                      // Store Byte with Update
     powerpc_stbux,                                     // Store Byte with Update Indexed
     powerpc_stbx,                                      // Store Byte Indexed
     powerpc_std,                                       // Store Doubleword
     powerpc_stdcx_record,                              // Store Doubleword Conditional Indexed
     powerpc_stdu,                                      // Store Doubleword with Update
     powerpc_stdux,                                     // Store Doubleword with Update Indexed
     powerpc_stdx,                                      // Store Doubleword Indexed
     powerpc_stfd,                                      // Store Floating-Point Double
     powerpc_stfdu,                                     // Store Floating-Point Double with Update
     powerpc_stfdux,                                    // Store Floating-Point Double with Update Indexed
     powerpc_stfdx,                                     // Store Floating-Point Double Indexed
     powerpc_stfiwx,                                    // Store Floating-Point as Integer Word Indexed (optional)
     powerpc_stfs,                                      // Store Floating-Point Single
     powerpc_stfsu,                                     // Store Floating-Point Single with Update
     powerpc_stfsux,                                    // Store Floating-Point Single with Update Indexed
     powerpc_stfsx,                                     // Store Floating-Point Single Indexed
     powerpc_sth,                                       // Store Half
     powerpc_sthbrx,                                    // Store Half Byte-Reverse Indexed
     powerpc_sthu,                                      // Store Half with Update
     powerpc_sthux,                                     // Store Half with Update Indexed
     powerpc_sthx,                                      // Store Half Indexed
     powerpc_stmw,                                      // Store Multiple Word
     powerpc_stswi,                                     // Store String Word Immediate
     powerpc_stswx,                                     // Store String Word Indexed
     powerpc_stw,                                       // Store
     powerpc_stwbrx,                                    // Store Word Byte-Reversed Indexed
     powerpc_stwcx_record,                              // Store Word Conditional Indexed
     powerpc_stwu,                                      // Store Word with Update
     powerpc_stwux,                                     // Store Word with Update Indexed
     powerpc_stwx,                                      // Store Word Indexed
     powerpc_subf,                                      // Subtract from
     powerpc_subf_record,                               // Subtract from
     powerpc_subfo,                                     // Subtract from
     powerpc_subfo_record,                              // Subtract from
     powerpc_subfc,                                     // Subtract from Carrying
     powerpc_subfc_record,                              // Subtract from Carrying
     powerpc_subfco,                                    // Subtract from Carrying
     powerpc_subfco_record,                             // Subtract from Carrying
     powerpc_subfe,                                     // Subtract from Extended
     powerpc_subfe_record,                              // Subtract from Extended
     powerpc_subfeo,                                    // Subtract from Extended
     powerpc_subfeo_record,                             // Subtract from Extended
     powerpc_subfic,                                    // Subtract from Immediate Carrying
     powerpc_subfme,                                    // Subtract from Minus One Extended
     powerpc_subfme_record,                             // Subtract from Minus One Extended
     powerpc_subfmeo,                                   // Subtract from Minus One Extended
     powerpc_subfmeo_record,                            // Subtract from Minus One Extended
     powerpc_subfze,                                    // Subtract from Zero Extended
     powerpc_subfze_record,                             // Subtract from Zero Extended
     powerpc_subfzeo,                                   // Subtract from Zero Extended
     powerpc_subfzeo_record,                            // Subtract from Zero Extended
     powerpc_sync,                                      // Synchronize
     powerpc_td,                                        // Trap Doubleword
     powerpc_tdi,                                       // Trap Doubleword Immediate
     powerpc_tlbia,                                     // Translation Look-aside Buffer Invalidate All (optional)
     powerpc_tlbie,                                     // Translation Look-aside Buffer Invalidate Entry (optional)
     powerpc_tlbsync,                                   // Translation Look-aside Buffer Synchronize (optional)
     powerpc_tw,                                        // Trap Word
     powerpc_twi,                                       // Trap Word Immediate
     powerpc_xor,                                       // XOR
     powerpc_xor_record,                                // XOR
     powerpc_xori,                                      // XOR Immediate
     powerpc_xoris,                                     // XOR Immediate Shift
     powerpc_last_instruction
   };

enum PowerpcRegisterClass
{
    powerpc_regclass_unknown,
    powerpc_regclass_gpr,       // General Purpose Register (0..31)
    powerpc_regclass_fpr,       // Floating-Point Register (0..31; 64 bits each)
    powerpc_regclass_cr,        // Condition Register (only particular fields or bits may be used)
    powerpc_regclass_fpscr,     // Floating point status and control register
    powerpc_regclass_spr,       // Special-purpose register (0..1023)
    powerpc_regclass_tbr,       // Time base register (0..1023)
    powerpc_regclass_msr,       // Machine state register
    powerpc_regclass_sr,        // Segment register
    powerpc_regclass_iar,       // instruction address (pseudo) register
    powerpc_regclass_pvr,       // processor version register
    powerpc_last_register_class // last enum value
};

enum PowerpcConditionRegisterAccessGranularity
{
  powerpc_condreggranularity_whole, // Whole CR (or unknown or not using a CR)
  powerpc_condreggranularity_field, // Four-bit field
  powerpc_condreggranularity_bit // Single bit
};

enum PowerpcSpecialPurposeRegister
{ // These must match the processor's numbers
    powerpc_spr_xer = 1,        // fixed-point exception register
    powerpc_spr_lr = 8,         // link register
    powerpc_spr_ctr = 9,        // count register
    powerpc_spr_dsisr = 18,
    powerpc_spr_dar = 19,
    powerpc_spr_dec = 22        // FIXME: fill in the rest of these
};

enum PowerpcTimeBaseRegister
{ // These must match the processor's numbers
  powerpc_tbr_tbl = 268,
  powerpc_tbr_tbu = 269
};

#endif /* ROSE_POWERPC_INSTRUCTION_ENUM_H */
