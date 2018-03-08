/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
bool power_entry::built_tables = false;

std::vector<power_entry> power_entry::main_opcode_table;
power_table power_entry::extended_op_0;
power_table power_entry::extended_op_4;
power_table power_entry::extended_op_19;
power_table power_entry::extended_op_30;
power_table power_entry::extended_op_31;
power_table power_entry::extended_op_58;
power_table power_entry::extended_op_59;
power_table power_entry::extended_op_63;

void power_entry::buildTables()
{
    if(built_tables) return;
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_0), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_INVALID, "INVALID", NULL, operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_tdi, "tdi", NULL, list_of(fn(TO))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_twi, "twi", NULL, list_of(fn(TO))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_INVALID, "INVALID", NULL, operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_INVALID, "INVALID", NULL, operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_INVALID, "INVALID", NULL, operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_mulli, "mulli", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_subfic, "subfic", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_dozi, "dozi", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_cmpli, "cmpli", NULL, list_of(fn(BF))(fn(L))(fn(RA))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_cmpi, "cmpi", NULL, list_of(fn(BF))(fn(L))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_addic, "addic", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_addic_rc, "addic.", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_addi, "addi", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_addis, "addis", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_bc, "bc", NULL, list_of(fn(BO))(fn(BI))(fn(BD))(fn(AA))(fn(LK))));
    main_opcode_table.push_back(power_entry(power_op_svcs, "sc", NULL, list_of(fn(syscall))));
    main_opcode_table.push_back(power_entry(power_op_b, "b", NULL, list_of(fn(LL))(fn(AA))(fn(LK))));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_19), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_rlwimi, "rlwimi", NULL,
                                list_of(fn(RA))(fn(RS))(fn(SH))(fn(ME))(fn(MB))(fn(Rc))));
    main_opcode_table.push_back(power_entry(power_op_rlwinm, "rlwinm", NULL,
                                list_of(fn(RA))(fn(RS))(fn(SH))(fn(MB))(fn(ME))(fn(Rc))));
    main_opcode_table.push_back(power_entry(power_op_rlmi, "rlmi", NULL,
                                list_of(fn(RA))(fn(RS))(fn(RB))(fn(MB))(fn(ME))(fn(Rc))));
    main_opcode_table.push_back(power_entry(power_op_rlwnm, "rlwnm", NULL,
                                list_of(fn(RA))(fn(RS))(fn(RB))(fn(MB))(fn(ME))(fn(Rc))));
    main_opcode_table.push_back(power_entry(power_op_ori, "ori", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_oris, "oris", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_xori, "xori", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_xoris, "xoris", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_andi_rc, "andi.", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_andis_rc, "andis.", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_30), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_31), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_lwz, "lwz", NULL, list_of(fn(RT))(fn(L<u32>))));
    main_opcode_table.push_back(power_entry(power_op_lwzu, "lwzu", NULL, list_of(fn(RT))(fn(LU<u32>))));
    main_opcode_table.push_back(power_entry(power_op_lbz, "lbz", NULL, list_of(fn(RT))(fn(L<u8>))));
    main_opcode_table.push_back(power_entry(power_op_lbzu, "lbzu", NULL, list_of(fn(RT))(fn(LU<u8>))));
    main_opcode_table.push_back(power_entry(power_op_stw, "stw", NULL, list_of(fn(RS))(fn(ST<u32>))));
    main_opcode_table.push_back(power_entry(power_op_stwu, "stwu", NULL, list_of(fn(RS))(fn(STU<u32>))));
    main_opcode_table.push_back(power_entry(power_op_stb, "stb", NULL, list_of(fn(RS))(fn(ST<u8>))));
    main_opcode_table.push_back(power_entry(power_op_stbu, "stbu", NULL, list_of(fn(RS))(fn(STU<u8>))));
    main_opcode_table.push_back(power_entry(power_op_lhz, "lhz", NULL, list_of(fn(RT))(fn(L<u16>))));
    main_opcode_table.push_back(power_entry(power_op_lhzu, "lhzu", NULL, list_of(fn(RT))(fn(LU<u16>))));
    main_opcode_table.push_back(power_entry(power_op_lha, "lha", NULL, list_of(fn(RT))(fn(L<u16>))));
    main_opcode_table.push_back(power_entry(power_op_lhau, "lhau", NULL, list_of(fn(RT))(fn(LU<u16>))));
    main_opcode_table.push_back(power_entry(power_op_sth, "sth", NULL, list_of(fn(RS))(fn(ST<u16>))));
    main_opcode_table.push_back(power_entry(power_op_sthu, "sthu", NULL, list_of(fn(RS))(fn(STU<u16>))));
    main_opcode_table.push_back(power_entry(power_op_lmw, "lmw", NULL, list_of(fn(RT))(fn(L<u32>))));
    main_opcode_table.push_back(power_entry(power_op_stmw, "stmw", NULL, list_of(fn(RT))(fn(ST<u32>))));
    main_opcode_table.push_back(power_entry(power_op_lfs, "lfs", NULL, list_of(fn(FRT))(fn(L<sp_float>))));
    main_opcode_table.push_back(power_entry(power_op_lfsu, "lfsu", NULL, list_of(fn(FRT))(fn(LU<sp_float>))));
    main_opcode_table.push_back(power_entry(power_op_lfd, "lfd", NULL, list_of(fn(FRT))(fn(L<dp_float>))));
    main_opcode_table.push_back(power_entry(power_op_lfdu, "lfdu", NULL, list_of(fn(FRT))(fn(LU<dp_float>))));
    main_opcode_table.push_back(power_entry(power_op_stfs, "stfs", NULL, list_of(fn(FRS))(fn(ST<sp_float>))));
    main_opcode_table.push_back(power_entry(power_op_stfsu, "stfsu", NULL, list_of(fn(FRS))(fn(STU<sp_float>))));
    main_opcode_table.push_back(power_entry(power_op_stfd, "stfd", NULL, list_of(fn(FRS))(fn(ST<dp_float>))));
    main_opcode_table.push_back(power_entry(power_op_stfdu, "stfdu", NULL, list_of(fn(FRS))(fn(STU<dp_float>))));
    main_opcode_table.push_back(power_entry(power_op_lfq, "lfq", NULL, list_of(fn(FRT2))(fn(L<dbl128>))));
    main_opcode_table.push_back(power_entry(power_op_lfqu, "lfqu", NULL, list_of(fn(FRT2))(fn(LU<dbl128>))));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_58), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_59), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_stfq, "stfq", NULL, list_of(fn(FRS2))(fn(ST<dbl128>))));
    main_opcode_table.push_back(power_entry(power_op_stfqu, "stfqu", NULL, list_of(fn(FRS2))(fn(STU<dbl128>))));
    main_opcode_table.push_back(power_entry(power_op_stdu, "stdu", NULL, list_of(fn(RS))(fn(STU<u64>))));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_63), operandSpec()));

    extended_op_0[1] = power_entry(power_op_qvfxxmadds, "qvfxxmadds", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
    extended_op_0[3] = power_entry(power_op_qvfxxcpnmadds, "qvfxxcpnmadds", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
    extended_op_0[5] = power_entry(power_op_fpsel, "fpsel", NULL,
        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
    extended_op_0[8] = power_entry(power_op_fpmul, "fpmul", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRCP)));
       extended_op_0[10] = power_entry(power_op_fxpmul, "fxpmul", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRCP)));
       extended_op_0[12] = power_entry(power_op_fpadd, "fpadd", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP)));
       extended_op_0[13] = power_entry(power_op_fpsub, "fpsub", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP)));
       extended_op_0[14] = power_entry(power_op_fpre, "fpre", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[15] = power_entry(power_op_fprsqrte, "fprsqrte", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[16] = power_entry(power_op_fpmadd, "fpmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
       extended_op_0[17] = power_entry(power_op_fxmadd, "fxmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
       extended_op_0[18] = power_entry(power_op_fxcpmadd, "fxcpmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
       extended_op_0[19] = power_entry(power_op_fxcsmadd, "fxcsmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
#ifdef os_bgq
       extended_op_0[9] = power_entry(power_op_qvfxmadds, "qvfxmadds", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_0[11] = power_entry(power_op_qvfxxnpmadds, "qvfxxnpmadds", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_0[17] = power_entry(power_op_qvfxmuls, "qvfxmuls", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QFRCP)));
       extended_op_0[20] = power_entry(power_op_qvfsubs, "qvfsubs", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_0[21] = power_entry(power_op_qvfadds, "qvfadds", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_0[24] = power_entry(power_op_qvfres, "qvfres", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_0[25] = power_entry(power_op_qvfmuls, "qvfmuls", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QFRCP)));
       extended_op_0[26] = power_entry(power_op_qvfrsqrtes, "qvfrsqrtes", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_0[28] = power_entry(power_op_qvfmsubs, "qvfmsubs", NULL,
          list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_0[29] = power_entry(power_op_qvfmadds, "qvfmadds", NULL,
          list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_0[30] = power_entry(power_op_qvfnmsubs, "qvfnmsubs", NULL,
          list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_0[31] = power_entry(power_op_qvfnmadds, "qvfnmadds", NULL,
          list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
#else
       extended_op_0[9] = power_entry(power_op_fxmul, "fxmul", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRCP)));
       extended_op_0[11] = power_entry(power_op_fxsmul, "fxsmul", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRCP)));
       extended_op_0[20] = power_entry(power_op_fpnmadd, "fpnmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
       extended_op_0[21] = power_entry(power_op_fxnmadd, "fxnmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
       extended_op_0[24] = power_entry(power_op_fpmsub, "fpmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
       extended_op_0[25] = power_entry(power_op_fxmsub, "fxmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
       extended_op_0[26] = power_entry(power_op_fxcpmsub, "fxcpmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
       extended_op_0[28] = power_entry(power_op_fpnmsub, "fpnmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
       extended_op_0[29] = power_entry(power_op_fxnmsub, "fxnmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
       extended_op_0[30] = power_entry(power_op_fxcpnmsub, "fxcpnmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
       extended_op_0[31] = power_entry(power_op_fxcsnmsub, "fxcsnmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
#endif
       extended_op_0[22] = power_entry(power_op_fxcpnmadd, "fxcpnmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
       extended_op_0[23] = power_entry(power_op_fxcsnmadd, "fxcsnmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
       extended_op_0[27] = power_entry(power_op_fxcsmsub, "fxcsmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
       extended_op_0[32] = power_entry(power_op_fpmr, "fpmr", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[96] = power_entry(power_op_fpabs, "fpabs", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[160] = power_entry(power_op_fpneg, "fpneg", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[192] = power_entry(power_op_fprsp, "fprsp", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[224] = power_entry(power_op_fpnabs, "fpnabs", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[288] = power_entry(power_op_fsmr, "fsmr", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRBS)));
       extended_op_0[320] = power_entry(power_op_fscmp, "fscmp", NULL, list_of(fn(setFPMode))(fn(BF))(fn(FRAS))(fn(FRBS)));
       extended_op_0[352] = power_entry(power_op_fsabs, "fsabs", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRBS)));
       extended_op_0[416] = power_entry(power_op_fsneg, "fsneg", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRBS)));
       extended_op_0[480] = power_entry(power_op_fsnabs, "fsnabs", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRBS)));
       extended_op_0[544] = power_entry(power_op_fxmr, "fxmr", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[576] = power_entry(power_op_fpctiw, "fpctiw", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[704] = power_entry(power_op_fpctiwz, "fpctiwz", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[800] = power_entry(power_op_fsmfp, "fpmr", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRB)));
       extended_op_0[846] = power_entry(power_op_qvfcfids, "qvfcfids", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_0[928] = power_entry(power_op_fsmtp, "fpmr", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRBS)));
       extended_op_0[974] = power_entry(power_op_qvfcfidus, "qvfcfidus", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_4[0] = power_entry(power_op_qvfcmpeq, "qvfcmpeq", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_4[1]   = power_entry(power_op_qvfxxmadd, "qvfxxmadd", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_4[3] = power_entry(power_op_qvfxxcpnmadd, "qvfxxcpnmadd", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_4[4] = power_entry(power_op_qvflogical, "qvflogical", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QTT)));
       extended_op_4[5] = power_entry(power_op_qvfaligni, "qvfaligni", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QVD)));
       extended_op_4[6] = power_entry(power_op_qvfperm, "qvfperm", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_4[8] = power_entry(power_op_qvfcpsgn, "qvfcpsgn", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_4[9] = power_entry(power_op_qvfxmadd, "qvfxmadd", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_4[11] = power_entry(power_op_qvfxxnpmadd, "qvfxxnpmadd", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_4[12] = power_entry(power_op_qvfrsp, "qvfrsp", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_4[14] = power_entry(power_op_qvfctiw, "qvfctiw", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_4[15] = power_entry(power_op_qvfctiwz, "qvfctiwz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_4[17] = power_entry(power_op_qvfxmul, "qvfxmul", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QFRCP)));
       extended_op_4[20] = power_entry(power_op_qvfsub, "qvfsub", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_4[21] = power_entry(power_op_qvfadd, "qvfadd", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_4[23] = power_entry(power_op_qvfsel, "qvfsel", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
#ifdef os_bgq
  extended_op_4[24] = power_entry(power_op_qvfre, "qvfre", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
  extended_op_4[25] = power_entry(power_op_qvfmul, "qvfmul", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QFRCP)));
  extended_op_4[26] = power_entry(power_op_qvfrsqrte, "qvfrsqrte", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
  extended_op_4[28] = power_entry(power_op_qvfmsub, "qvfmsub", NULL,
    list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
  extended_op_4[29] = power_entry(power_op_qvfmadd, "qvfmadd", NULL,
    list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
  extended_op_4[30] = power_entry(power_op_qvfnmsub, "qvfnmsub", NULL,
    list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
  extended_op_4[31] = power_entry(power_op_qvfnmadd, "qvfnmadd", NULL,
    list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
#else
   extended_op_4[24] = power_entry(power_op_fxcpnpma, "fxcpnpma", NULL,
     list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
   extended_op_4[25] = power_entry(power_op_fxcsnpma, "fxcsnpma", NULL,
     list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
   extended_op_4[26] = power_entry(power_op_fxcpnsma, "fxcpnsma", NULL,
     list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
   extended_op_4[28] = power_entry(power_op_fxcxnpma, "fxcxnpma", NULL,
     list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
   extended_op_4[29] = power_entry(power_op_fxcxnsma, "fxcxnsma", NULL,
     list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
   extended_op_4[30] = power_entry(power_op_fxcxma, "fxcxma", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
   extended_op_4[31] = power_entry(power_op_fxcxnms, "fxcxnms", NULL,
     list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
#endif
extended_op_4[27] = power_entry(power_op_fxcsnsma, "fxcsnsma", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
extended_op_4[32] = power_entry(power_op_qvfcmpgt, "qvfcmpgt", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
extended_op_4[37] = power_entry(power_op_qvesplati, "qvesplati", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QVD)));
extended_op_4[40] = power_entry(power_op_qvfneg, "fpneg", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[64] = power_entry(power_op_qvftstnan, "qvftstnan", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
extended_op_4[72] = power_entry(power_op_qvfmr, "qvfmr", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[96] = power_entry(power_op_qvfcmplt, "qvfcmplt", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
extended_op_4[133] = power_entry(power_op_qvgpci, "qvgpci", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QGPC)));
extended_op_4[136] = power_entry(power_op_qvfnabs, "qvfnabs", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[142] = power_entry(power_op_qvfctiwu, "qvfctiwu", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[143] = power_entry(power_op_qvfctiwuz, "qvfctiwuz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[264] = power_entry(power_op_qvfabs, "fpabs", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[392] = power_entry(power_op_qvfrin, "qvfrin", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[424] = power_entry(power_op_qvfriz, "qvfriz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[456] = power_entry(power_op_qvfrip, "qvfrip", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[488] = power_entry(power_op_qvfrim, "qvfrim", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[814] = power_entry(power_op_qvfctid, "qvfctid", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[815] = power_entry(power_op_qvfctidz, "qvfctidz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[846] = power_entry(power_op_qvfcfid, "qvfcfid", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[942] = power_entry(power_op_qvfctidu, "qvfctidu", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[943] = power_entry(power_op_qvfctiduz, "qvfctiduz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[974] = power_entry(power_op_qvfcfidu, "qvfcfidu", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_19[0] = power_entry(power_op_mcrf, "mcrf", NULL, list_of(fn(BF))(fn(BFA)));
extended_op_19[16] = power_entry(power_op_bclr, "bclr", NULL, list_of(fn(BO))(fn(BI))(fn(LK)));
extended_op_19[18] = power_entry(power_op_rfid, "rfid", NULL, operandSpec());
extended_op_19[33] = power_entry(power_op_crnor, "crnor", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[50] = power_entry(power_op_rfi, "rfi", NULL, operandSpec());
extended_op_19[82] = power_entry(power_op_rfsvc, "rfsvc", NULL, list_of(fn(LK)));
extended_op_19[129] = power_entry(power_op_crandc, "crandc", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[150] = power_entry(power_op_isync, "isync", NULL, operandSpec());
extended_op_19[193] = power_entry(power_op_crxor, "crxor", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[225] = power_entry(power_op_crnand, "crnand", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[257] = power_entry(power_op_crand, "crand", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[289] = power_entry(power_op_creqv, "creqv", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[417] = power_entry(power_op_crorc, "crorc", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[449] = power_entry(power_op_cror, "cror", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[528] = power_entry(power_op_bcctr, "bcctr", NULL, list_of(fn(BO))(fn(BI))(fn(LK)));

    power_entry::extended_op_30[1] = power_entry(power_op_rldicr, "rldicr", NULL,
            list_of(fn(RA))(fn(RS))(fn(SH))(fn(ME))(fn(Rc)));
extended_op_30[2] = power_entry(power_op_rldic, "rldic", NULL, list_of(fn(RA))(fn(RS))(fn(SH))(fn(MB))(fn(Rc)));
extended_op_30[3] = power_entry(power_op_rldimi, "rldimi", NULL, list_of(fn(RA))(fn(RS))(fn(SH))(fn(MB))(fn(Rc)));
extended_op_30[8] = power_entry(power_op_rldcl, "rldcl", NULL, list_of(fn(RA))(fn(RS))(fn(RB))(fn(MB))(fn(Rc)));
extended_op_30[9] = power_entry(power_op_rldcr, "rldcr", NULL, list_of(fn(RA))(fn(RS))(fn(RB))(fn(ME))(fn(Rc)));
extended_op_30[0] = power_entry(power_op_rldicl, "rldicl", NULL, list_of(fn(RA))(fn(RS))(fn(SH))(fn(MB))(fn(SH))(fn(Rc)));

    power_entry::extended_op_31[0] = power_entry(power_op_cmp, "cmp", NULL, list_of(fn(BF))(fn(L))(fn(RA))(fn(RB)));
extended_op_31[4] = power_entry(power_op_tw, "tw", NULL, list_of(fn(TO))(fn(RA))(fn(RB)));
extended_op_31[6] = power_entry(power_op_qvlpcrsx, "qvlpcrsx", NULL, list_of(fn(QFRTP))(fn(LX<sp_float>)));
extended_op_31[7] = power_entry(power_op_qvlfcsx, "qvlfcsx", NULL, list_of(fn(QFRTP))(fn(LX<sp_float>)));
extended_op_31[8] = power_entry(power_op_subfc, "subfc", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[9] = power_entry(power_op_mulhdu, "mulhdu", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[10] = power_entry(power_op_addc, "addc", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[11] = power_entry(power_op_mulhwu, "mulhwu", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[19] = power_entry(power_op_mfcr, "mfcr", NULL, list_of(fn(RT))(fn(Rc)));
extended_op_31[20] = power_entry(power_op_lwarx, "lwarx", NULL, list_of(fn(RT))(fn(LX<u32>)));
extended_op_31[21] = power_entry(power_op_ldx, "ldx", NULL, list_of(fn(RT))(fn(LX<u64>)));
extended_op_31[23] = power_entry(power_op_lwzx, "lwzx", NULL, list_of(fn(RT))(fn(LX<u32>)));
extended_op_31[24] = power_entry(power_op_slw, "slw", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[26] = power_entry(power_op_cntlzw, "cntlzw", NULL, list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[27] = power_entry(power_op_sld, "sld", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[28] = power_entry(power_op_and, "and", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[29] = power_entry(power_op_maskg, "maskg", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[32] = power_entry(power_op_cmpl, "cmpl", NULL, list_of(fn(BF))(fn(L))(fn(RA))(fn(RB)));
extended_op_31[39] = power_entry(power_op_qvlfcsux, "qvlfcsux", NULL, list_of(fn(QFRTP))(fn(LUX<sp_float>)));
extended_op_31[40] = power_entry(power_op_subf, "subf", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[53] = power_entry(power_op_ldux, "ldux", NULL, list_of(fn(RT))(fn(LUX<u64>)));
extended_op_31[54] = power_entry(power_op_dcbst, "dcbst", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[55] = power_entry(power_op_lwzux, "lwzux", NULL, list_of(fn(RT))(fn(LUX<u32>)));
extended_op_31[58] = power_entry(power_op_cntlzd, "cntlzd", NULL, list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[60] = power_entry(power_op_andc, "andc", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[62] = power_entry(power_op_wait, "wait", NULL, list_of(fn(WC)));
extended_op_31[68] = power_entry(power_op_td, "td", NULL, list_of(fn(TO))(fn(RA))(fn(RB)));
extended_op_31[70] = power_entry(power_op_qvlpcrdx, "qvlpcrdx", NULL, list_of(fn(QFRTP))(fn(LX<dbl128>)));
extended_op_31[71] = power_entry(power_op_qvlfcdx, "qvlfcdx", NULL, list_of(fn(QFRTP))(fn(LX<dbl128>)));
extended_op_31[73] = power_entry(power_op_mulhd, "mulhd", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[75] = power_entry(power_op_mulhw, "mulhw", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[83] = power_entry(power_op_mfmsr, "mfmsr", NULL, list_of(fn(RT)));
extended_op_31[84] = power_entry(power_op_ldarx, "ldarx", NULL, list_of(fn(RT))(fn(LX<u64>)));
extended_op_31[86] = power_entry(power_op_dcbf, "dcbf", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[87] = power_entry(power_op_lbzx, "lbzx", NULL, list_of(fn(RT))(fn(LX<u8>)));
extended_op_31[103] = power_entry(power_op_qvlfcdux, "qvlfcdux", NULL, list_of(fn(QFRTP))(fn(LUX<dbl128>)));
extended_op_31[104] = power_entry(power_op_neg, "neg", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[107] = power_entry(power_op_mul, "mul", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[118] = power_entry(power_op_clf, "clf", NULL, list_of(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[119] = power_entry(power_op_lbzux, "lbzux", NULL, list_of(fn(RT))(fn(LUX<u8>)));
extended_op_31[122] = power_entry(power_op_popcntb, "popcntb", NULL, list_of(fn(RS))(fn(RA)));
extended_op_31[124] = power_entry(power_op_nor, "nor", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[133] = power_entry(power_op_qvstfcsxi, "qvstfcsxi", NULL,  list_of(fn(QFRSP))(fn(STX<sp_float>)));
extended_op_31[135] = power_entry(power_op_qvstfcsx, "qvstfcsx", NULL,  list_of(fn(QFRSP))(fn(STX<sp_float>)));
extended_op_31[136] = power_entry(power_op_subfe, "subfe", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[138] = power_entry(power_op_adde, "adde", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[142] = power_entry(power_op_lfssx, "lfssx", NULL, list_of(fn(FRTS))(fn(LX<u32>)));
extended_op_31[144] = power_entry(power_op_mtcrf, "mtcrf", NULL, list_of(fn(RS))(fn(FXM))(fn(Rc)));
extended_op_31[149] = power_entry(power_op_stdx, "stdx", NULL, list_of(fn(RS))(fn(STX<u64>)));
extended_op_31[150] = power_entry(power_op_stwcx_rc, "stwcx.", NULL, list_of(fn(RS))(fn(STX<u32>)));
extended_op_31[151] = power_entry(power_op_stwx, "stwx", NULL, list_of(fn(RS))(fn(STX<u32>)));
extended_op_31[152] = power_entry(power_op_slq, "slq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[153] = power_entry(power_op_sle, "sle", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[165] = power_entry(power_op_qvstfcsuxi, "qvstfcsuxi", NULL, list_of(fn(QFRSP))(fn(STUX<sp_float>)));
extended_op_31[167] = power_entry(power_op_qvstfcsux, "qvstfcsux", NULL, list_of(fn(QFRSP))(fn(STUX<sp_float>)));
extended_op_31[174] = power_entry(power_op_lfssux, "lfssux", NULL, list_of(fn(FRTS))(fn(LUX<u32>)));
extended_op_31[181] = power_entry(power_op_stdux, "stdux", NULL, list_of(fn(RS))(fn(STUX<u64>)));
extended_op_31[183] = power_entry(power_op_stwux, "stwux", NULL, list_of(fn(RS))(fn(STUX<u32>)));
extended_op_31[184] = power_entry(power_op_sliq, "sliq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[197] = power_entry(power_op_qvstfcdxi, "qvstfcdxi", NULL, list_of(fn(QFRSP))(fn(STX<dbl128>)));
extended_op_31[199] = power_entry(power_op_qvstfcdx, "qvstfcdx", NULL, list_of(fn(QFRSP))(fn(STX<dbl128>)));
extended_op_31[202] = power_entry(power_op_addze, "addze", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[206] = power_entry(power_op_lfsdx, "lfsdx", NULL, list_of(fn(FRTS))(fn(LX<dbl128>)));
extended_op_31[214] = power_entry(power_op_stdcx_rc, "stdcx.", NULL, list_of(fn(RS))(fn(STX<u64>)));
extended_op_31[215] = power_entry(power_op_stbx, "stbx", NULL, list_of(fn(RS))(fn(STX<u8>)));
extended_op_31[216] = power_entry(power_op_sllq, "sllq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[217] = power_entry(power_op_sleq, "sleq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[229] = power_entry(power_op_qvstfcduxi, "qvstfcduxi", NULL, list_of(fn(QFRSP))(fn(STUX<dbl128>)));
extended_op_31[231] = power_entry(power_op_qvstfcdux, "qvstfcdux", NULL, list_of(fn(QFRSP))(fn(STUX<dbl128>)));
extended_op_31[232] = power_entry(power_op_subfme, "subfme", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[233] = power_entry(power_op_mulld, "mulld", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[234] = power_entry(power_op_addme, "addme", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[235] = power_entry(power_op_mullw, "mullw", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[238] = power_entry(power_op_lfsdux, "lfsdux", NULL, list_of(fn(FRTS))(fn(LUX<dbl128>)));
extended_op_31[246] = power_entry(power_op_dcbtst, "dcbtst", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[247] = power_entry(power_op_stbux, "stbux", NULL, list_of(fn(RS))(fn(STUX<u8>)));
extended_op_31[248] = power_entry(power_op_slliq, "slliq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[264] = power_entry(power_op_doz, "doz", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[266] = power_entry(power_op_add, "add", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[270] = power_entry(power_op_lfxsx, "lfxsx", NULL, list_of(fn(FRTP))(fn(LX<dp_float>)));
extended_op_31[277] = power_entry(power_op_lscbx, "lscbx", NULL, list_of(fn(RT))(fn(LX<u8>))(fn(Rc)));
extended_op_31[278] = power_entry(power_op_dcbt, "dcbt", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[279] = power_entry(power_op_lhzx, "lhzx", NULL, list_of(fn(RT))(fn(LX<u16>)));
extended_op_31[284] = power_entry(power_op_eqv, "eqv", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[302] = power_entry(power_op_lfxsux, "lfxsux", NULL, list_of(fn(FRTP))(fn(LUX<dp_float>)));
extended_op_31[306] = power_entry(power_op_tlbie, "tlbie", NULL, list_of(fn(RB)));
extended_op_31[310] = power_entry(power_op_eciwx, "eciwx", NULL, list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[316] = power_entry(power_op_xor, "xor", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[311] = power_entry(power_op_lhzux, "lhzux", NULL, list_of(fn(RT))(fn(LUX<u16>)));
extended_op_31[334] = power_entry(power_op_lfxdx, "lfxdx", NULL, list_of(fn(FRTP))(fn(LX<dbl128>)));
extended_op_31[339] = power_entry(power_op_mfspr, "mfspr", NULL, list_of(fn(RT))(fn(spr))(fn(Rc)));
extended_op_31[341] = power_entry(power_op_lwax, "lwax", NULL, list_of(fn(RT))(fn(LX<u32>)));
extended_op_31[343] = power_entry(power_op_lhax, "lhax", NULL, list_of(fn(RT))(fn(LX<u16>)));
extended_op_31[360] = power_entry(power_op_abs, "abs", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[363] = power_entry(power_op_divs, "divs", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[366] = power_entry(power_op_lfxdux, "lfxdux", NULL, list_of(fn(FRTP))(fn(LUX<dbl128>)));

// xop 371 is mftb (Move from time base). It is phased-out and equivalent to mfspr Rx, 268
extended_op_31[371] = power_entry(power_op_mfspr, "mfspr", NULL, list_of(fn(RT))(fn(spr))(fn(Rc)));
extended_op_31[373] = power_entry(power_op_lwaux, "lwaux", NULL, list_of(fn(RT))(fn(LUX<u32>)));
extended_op_31[375] = power_entry(power_op_lhaux, "lhaux", NULL, list_of(fn(RT))(fn(LUX<u16>)));
extended_op_31[378] = power_entry(power_op_popcntw, "popcntw", NULL, list_of(fn(RS))(fn(RA)));

extended_op_31[398] = power_entry(power_op_lfpsx, "lfpsx", NULL, list_of(fn(FRTP))(fn(LX<sp_float>)));
extended_op_31[407] = power_entry(power_op_sthx, "sthx", NULL, list_of(fn(RS))(fn(STX<u16>)));
extended_op_31[412] = power_entry(power_op_orc, "orc", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[413] = power_entry(power_op_sradi, "sradi", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[430] = power_entry(power_op_lfpsux, "lfpsux", NULL, list_of(fn(FRTP))(fn(LUX<sp_float>)));
extended_op_31[438] = power_entry(power_op_ecowx, "ecowx", NULL, list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[439] = power_entry(power_op_sthux, "sthux", NULL, list_of(fn(RS))(fn(STUX<u16>)));
extended_op_31[444] = power_entry(power_op_or, "or", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[457] = power_entry(power_op_divdu, "divdu", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[459] = power_entry(power_op_divwu, "divwu", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[462] = power_entry(power_op_lfpdx, "lfpdx", NULL, list_of(fn(FRTP))(fn(LX<dbl128>)));
extended_op_31[467] = power_entry(power_op_mtspr, "mtspr", NULL, list_of(fn(RS))(fn(spr))(fn(Rc)));
extended_op_31[470] = power_entry(power_op_dcbi, "dcbi", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[476] = power_entry(power_op_nand, "nand", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[488] = power_entry(power_op_nabs, "nabs", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[489] = power_entry(power_op_divd, "divd", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[491] = power_entry(power_op_divw, "divw", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[494] = power_entry(power_op_lfpdux, "lfpdux", NULL, list_of(fn(FRTP))(fn(LUX<dbl128>)));
extended_op_31[502] = power_entry(power_op_cli, "cli", NULL, list_of(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[506] = power_entry(power_op_popcntd, "popcntd", NULL, list_of(fn(RS))(fn(RA)));
extended_op_31[512] = power_entry(power_op_mcrxr, "mcrxr", NULL, list_of(fn(BF)));
extended_op_31[518] = power_entry(power_op_qvlpclsx, "qvlpclsx", NULL, list_of(fn(QFRTP))(fn(LX<sp_float>)));
extended_op_31[519] = power_entry(power_op_qvlfsx, "qvlfsx", NULL,  list_of(fn(QFRTP))(fn(LX<sp_float>)));
extended_op_31[526] = power_entry(power_op_stfpiwx, "stfpiwx", NULL, list_of(fn(FRSP))(fn(STUX<u64>)));
extended_op_31[531] = power_entry(power_op_clcs, "clcs", NULL, list_of(fn(RT))(fn(RA))(fn(Rc)));
extended_op_31[533] = power_entry(power_op_lswx, "lswx", NULL, list_of(fn(RT))(fn(LX<u8>)));
extended_op_31[534] = power_entry(power_op_lwbrx, "lwbrx", NULL, list_of(fn(RT))(fn(LX<u32>)));
extended_op_31[535] = power_entry(power_op_lfsx, "lfsx", NULL, list_of(fn(FRT))(fn(LX<sp_float>)));
extended_op_31[536] = power_entry(power_op_srw, "srw", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[537] = power_entry(power_op_rrib, "rrib", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[539] = power_entry(power_op_srd, "srd", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[541] = power_entry(power_op_maskir, "maskir", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[551] = power_entry(power_op_qvlfsux, "qvlfsux", NULL, list_of(fn(QFRTP))(fn(LUX<sp_float>)));
extended_op_31[566] = power_entry(power_op_tlbsync, "tlbsync", NULL, operandSpec());
extended_op_31[567] = power_entry(power_op_lfsux, "lfsux", NULL, list_of(fn(FRT))(fn(LUX<sp_float>)));
extended_op_31[582] = power_entry(power_op_qvlpcldx, "qvlpcldx", NULL, list_of(fn(QFRTP))(fn(LX<dbl128>)));
extended_op_31[583] = power_entry(power_op_qvlfdx, "qvlfdx", NULL, list_of(fn(QFRTP))(fn(LX<dbl128>)));
extended_op_31[595] = power_entry(power_op_mfsr, "mfsr", NULL, list_of(fn(RT))(fn(SR)));
extended_op_31[597] = power_entry(power_op_lswi, "lswi", NULL, list_of(fn(RT))(fn(L<u8>))(fn(NB)));
extended_op_31[598] = power_entry(power_op_sync, "sync", NULL, operandSpec());
extended_op_31[599] = power_entry(power_op_lfdx, "lfdx", NULL, list_of(fn(FRT))(fn(LX<dp_float>)));
extended_op_31[615] = power_entry(power_op_qvlfdux, "qvlfdux", NULL, list_of(fn(QFRTP))(fn(LUX<dbl128>)));
extended_op_31[627] = power_entry(power_op_mfsri, "mfsri", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[630] = power_entry(power_op_dclst, "dclst", NULL, list_of(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[631] = power_entry(power_op_lfdux, "lfdux", NULL, list_of(fn(FRT))(fn(LUX<dp_float>)));
extended_op_31[645] = power_entry(power_op_qvstfsxi, "qvstfsxi", NULL, list_of(fn(QFRSP))(fn(STX<sp_float>)));
extended_op_31[647] = power_entry(power_op_qvstfsx, "qvstfsx", NULL, list_of(fn(QFRSP))(fn(STX<sp_float>)));
extended_op_31[654] = power_entry(power_op_stfssx, "stfssx", NULL, list_of(fn(FRSS))(fn(STX<u32>)));
extended_op_31[659] = power_entry(power_op_mfsrin, "mfsrin", NULL, list_of(fn(RT))(fn(RB)));
extended_op_31[661] = power_entry(power_op_stswx, "stswx", NULL, list_of(fn(RS))(fn(STX<u8>)));
extended_op_31[662] = power_entry(power_op_stwbrx, "stwbrx", NULL, list_of(fn(RS))(fn(STX<u32>)));
extended_op_31[663] = power_entry(power_op_stfsx, "stfsx", NULL, list_of(fn(FRS))(fn(STX<sp_float>)));
extended_op_31[664] = power_entry(power_op_srq, "srq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[665] = power_entry(power_op_sre, "sre", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[679] = power_entry(power_op_qvstfsux, "qvstfsux", NULL, list_of(fn(QFRSP))(fn(STUX<sp_float>)));
extended_op_31[686] = power_entry(power_op_stfssux, "stfssux", NULL, list_of(fn(FRSS))(fn(STUX<u32>)));
extended_op_31[695] = power_entry(power_op_stfsux, "stfsux", NULL, list_of(fn(FRS))(fn(STUX<sp_float>)));
extended_op_31[696] = power_entry(power_op_sriq, "sriq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[709] = power_entry(power_op_qvstfdxi, "qvstfdxi", NULL, list_of(fn(QFRSP))(fn(STX<dbl128>)));
extended_op_31[711] = power_entry(power_op_qvstfdx, "qvstfdx", NULL, list_of(fn(QFRSP))(fn(STX<dbl128>)));
extended_op_31[718] = power_entry(power_op_stfsdx, "stfsdx", NULL, list_of(fn(FRSS))(fn(STX<u32>)));
extended_op_31[725] = power_entry(power_op_stswi, "stswi", NULL, list_of(fn(RS))(fn(ST<u8>))(fn(NB)));
extended_op_31[727] = power_entry(power_op_stfdx, "stfdx", NULL, list_of(fn(FRS))(fn(STX<dp_float>)));
extended_op_31[728] = power_entry(power_op_srlq, "srlq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[729] = power_entry(power_op_sreq, "sreq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[741] = power_entry(power_op_qvlstduxi, "qvstfduxi", NULL, list_of(fn(QFRSP))(fn(STUX<dbl128>)));
extended_op_31[743] = power_entry(power_op_qvlstdux, "qvstfdux", NULL, list_of(fn(QFRSP))(fn(STUX<dbl128>)));
extended_op_31[750] = power_entry(power_op_stfsdux, "stfsdux", NULL, list_of(fn(FRSS))(fn(STUX<u32>)));
extended_op_31[759] = power_entry(power_op_stfdux, "stfdux", NULL, list_of(fn(FRS))(fn(STUX<dp_float>)));
extended_op_31[760] = power_entry(power_op_srliq, "srliq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[782] = power_entry(power_op_stfxsx, "stfxsx", NULL, list_of(fn(FRSP))(fn(STX<u64>)));
extended_op_31[790] = power_entry(power_op_lhbrx, "lhbrx", NULL, list_of(fn(RT))(fn(LX<u16>)));
extended_op_31[791] = power_entry(power_op_lfqx, "lfqx", NULL, list_of(fn(FRT))(fn(LX<dbl128>))(fn(Rc)));
extended_op_31[792] = power_entry(power_op_sraw, "sraw", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[794] = power_entry(power_op_srad, "srad", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[814] = power_entry(power_op_stfxsux, "stfxsux", NULL, list_of(fn(FRSP))(fn(STUX<u64>)));
extended_op_31[818] = power_entry(power_op_rac, "rac", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[823] = power_entry(power_op_lfqux, "lfqux", NULL, list_of(fn(FRT))(fn(LUX<dbl128>))(fn(Rc)));
extended_op_31[824] = power_entry(power_op_srawi, "srawi", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[839] = power_entry(power_op_qvlfiwzx, "qvlfiwzx", NULL, list_of(fn(QFRTP))(fn(LX<u32>)));
extended_op_31[846] = power_entry(power_op_stfxdx, "stfxdx", NULL, list_of(fn(FRSP))(fn(STX<dbl128>)));
extended_op_31[854] = power_entry(power_op_eieio, "eieio", NULL, operandSpec());
extended_op_31[871] = power_entry(power_op_qvlfiwax, "qvlfiwax", NULL, list_of(fn(QFRTP))(fn(LX<u32>)));
extended_op_31[878] = power_entry(power_op_stfxdux, "stfxdux", NULL, list_of(fn(FRSP))(fn(STUX<dbl128>)));
extended_op_31[910] = power_entry(power_op_stfpsx, "stfpsx", NULL, list_of(fn(FRSP))(fn(STX<u32>)));
extended_op_31[918] = power_entry(power_op_sthbrx, "sthbrx", NULL, list_of(fn(RS))(fn(STX<u16>)));
extended_op_31[919] = power_entry(power_op_stfqx, "stfqx", NULL, list_of(fn(FRS))(fn(STX<dbl128>))(fn(Rc)));
extended_op_31[920] = power_entry(power_op_sraq, "sraq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[921] = power_entry(power_op_srea, "srea", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[922] = power_entry(power_op_extsh, "extsh", NULL, list_of(fn(RS))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[942] = power_entry(power_op_stfpsux, "stfpsux", NULL, list_of(fn(FRSP))(fn(STUX<u32>)));
extended_op_31[951] = power_entry(power_op_stfqux, "stfqux", NULL, list_of(fn(FRS))(fn(STUX<dbl128>))(fn(Rc)));
extended_op_31[952] = power_entry(power_op_sraiq, "sraiq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[954] = power_entry(power_op_extsb, "extsb", NULL, list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[967] = power_entry(power_op_qvstfiwx, "qvstfiwx", NULL, list_of(fn(QFRSP))(fn(STX<u32>)));
extended_op_31[974] = power_entry(power_op_stfpdx, "stfpdx", NULL, list_of(fn(FRSP))(fn(STX<dbl128>)));
extended_op_31[978] = power_entry(power_op_tlbld, "tlbld", NULL, list_of(fn(RB)));
extended_op_31[982] = power_entry(power_op_icbi, "icbi", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[983] = power_entry(power_op_stfiwx, "stfiwx", NULL, list_of(fn(FRS))(fn(STX<u32>)));
extended_op_31[986] = power_entry(power_op_extsw, "extsw", NULL, list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[1006] = power_entry(power_op_stfpdux, "stfpdux", NULL, list_of(fn(FRSP))(fn(STUX<dbl128>)));
extended_op_31[1010] = power_entry(power_op_tlbli, "tlbli", NULL, list_of(fn(RB)));
extended_op_31[1014] = power_entry(power_op_dcbz, "dcbz", NULL, list_of(fn(RA))(fn(RB)));

extended_op_58[0] = power_entry(power_op_ld, "ld", NULL, list_of(fn(RT))(fn(L<u64>)));
extended_op_58[1] = power_entry(power_op_ldu, "ldu", NULL, list_of(fn(RT))(fn(LU<u64>)));
extended_op_58[2] = power_entry(power_op_lwa, "lwa", NULL, list_of(fn(RT))(fn(L<s32>)));

extended_op_59[18] = power_entry(power_op_fdivs, "fdivs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_59[20] = power_entry(power_op_fsubs, "fsubs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_59[21] = power_entry(power_op_fadds, "fadds", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_59[22] = power_entry(power_op_fsqrts, "fsqrts", NULL, list_of(fn(setFPMode))(fn(RT))(fn(RB))(fn(Rc)));
extended_op_59[24] = power_entry(power_op_fres, "fres", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_59[25] = power_entry(power_op_fmuls, "fmuls", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRC))(fn(Rc)));
extended_op_59[28] = power_entry(power_op_fmsubs, "fmsubs", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_59[29] = power_entry(power_op_fmadds, "fmadds", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_59[30] = power_entry(power_op_fnmsubs, "fnmsubs", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_59[31] = power_entry(power_op_fnmadds, "fnmadds", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));

    extended_op_63[0] = power_entry(power_op_fcmpu, "fcmpu", NULL, list_of(fn(setFPMode))(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[12] = power_entry(power_op_frsp, "frsp", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[14] = power_entry(power_op_fctiw, "fctiw", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[15] = power_entry(power_op_fctiwz, "fctiwz", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[18] = power_entry(power_op_fdiv, "fdiv", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_63[20] = power_entry(power_op_fsub, "fsub", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_63[21] = power_entry(power_op_fadd, "fadd", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_63[22] = power_entry(power_op_fsqrt, "fsqrt", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[23] = power_entry(power_op_fsel, "fsel", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[25] = power_entry(power_op_fmul, "fmul", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRC))(fn(Rc)));
extended_op_63[26] = power_entry(power_op_frsqrte, "frsqrte", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[28] = power_entry(power_op_fmsub, "fmsub", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[29] = power_entry(power_op_fmadd, "fmadd", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[30] = power_entry(power_op_fnmsub, "fnmsub", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[31] = power_entry(power_op_fnmadd, "fnmadd", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[32] = power_entry(power_op_fcmpo, "fcmpo", NULL, list_of(fn(setFPMode))(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[38] = power_entry(power_op_mtfsb1, "mtfsb1", NULL, list_of(fn(setFPMode))(fn(BT))(fn(Rc)));
extended_op_63[40] = power_entry(power_op_fneg, "fneg", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[64] = power_entry(power_op_mcrfs, "mcrfs", NULL, list_of(fn(BF))(fn(BFA)));
extended_op_63[70] = power_entry(power_op_mtfsb0, "mtfsb0", NULL, list_of(fn(BT))(fn(Rc)));
extended_op_63[72] = power_entry(power_op_fmr, "fmr", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[134] = power_entry(power_op_mtfsfi, "mtfsfi", NULL, list_of(fn(BF))(fn(U))(fn(Rc)));
extended_op_63[136] = power_entry(power_op_fnabs, "fnabs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB)));
extended_op_63[264] = power_entry(power_op_fabs, "fabs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[583] = power_entry(power_op_mffs, "mffs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(Rc)));
extended_op_63[711] = power_entry(power_op_mtfsf, "mtfsf", NULL, list_of(fn(setFPMode))(fn(FLM))(fn(FRB))(fn(Rc)));
extended_op_63[814] = power_entry(power_op_fctid, "fctid", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[815] = power_entry(power_op_fctidz, "fctidz", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[846] = power_entry(power_op_fcfid, "fcfid", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));

built_tables = true;
}
