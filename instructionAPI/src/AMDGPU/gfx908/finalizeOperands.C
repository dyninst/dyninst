void InstructionDecoder_amdgpu_gfx908::finalizeENC_DSOperands(){
layout_ENC_DS & layout = insn_layout.ENC_DS;
switch(layout.OP){
case 0:// DS_ADD_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 1:// DS_SUB_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 2:// DS_RSUB_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 3:// DS_INC_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 4:// DS_DEC_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 5:// DS_MIN_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 6:// DS_MAX_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 7:// DS_MIN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 8:// DS_MAX_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 9:// DS_AND_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 10:// DS_OR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 11:// DS_XOR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 12:// DS_MSKOR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 13:// DS_WRITE_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 14:// DS_WRITE2_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 15:// DS_WRITE2ST64_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 16:// DS_CMPST_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 17:// DS_CMPST_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 18:// DS_MIN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 19:// DS_MAX_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 20:// DS_NOP
break;
case 21:// DS_ADD_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 29:// DS_WRITE_ADDTID_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 30:// DS_WRITE_B8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 31:// DS_WRITE_B16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 32:// DS_ADD_RTN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 33:// DS_SUB_RTN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 34:// DS_RSUB_RTN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 35:// DS_INC_RTN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 36:// DS_DEC_RTN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 37:// DS_MIN_RTN_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 38:// DS_MAX_RTN_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 39:// DS_MIN_RTN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 40:// DS_MAX_RTN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 41:// DS_AND_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 42:// DS_OR_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 43:// DS_XOR_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 44:// DS_MSKOR_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 45:// DS_WRXCHG_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 46:// DS_WRXCHG2_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 47:// DS_WRXCHG2ST64_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 48:// DS_CMPST_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 49:// DS_CMPST_RTN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 50:// DS_MIN_RTN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 51:// DS_MAX_RTN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 52:// DS_WRAP_RTN_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1,32),true,false);
break;
case 53:// DS_ADD_RTN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 54:// DS_READ_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 55:// DS_READ2_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 56:// DS_READ2ST64_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 57:// DS_READ_I8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 58:// DS_READ_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 59:// DS_READ_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 60:// DS_READ_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 61:// DS_SWIZZLE_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 62:// DS_PERMUTE_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 63:// DS_BPERMUTE_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 64:// DS_ADD_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 65:// DS_SUB_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 66:// DS_RSUB_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 67:// DS_INC_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 68:// DS_DEC_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 69:// DS_MIN_I64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 70:// DS_MAX_I64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 71:// DS_MIN_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 72:// DS_MAX_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 73:// DS_AND_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 74:// DS_OR_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 75:// DS_XOR_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 76:// DS_MSKOR_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 77:// DS_WRITE_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 78:// DS_WRITE2_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 79:// DS_WRITE2ST64_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 80:// DS_CMPST_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 81:// DS_CMPST_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 82:// DS_MIN_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 83:// DS_MAX_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 84:// DS_WRITE_B8_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 85:// DS_WRITE_B16_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
break;
case 86:// DS_READ_U8_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 87:// DS_READ_U8_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 88:// DS_READ_I8_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 89:// DS_READ_I8_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 90:// DS_READ_U16_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 91:// DS_READ_U16_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 96:// DS_ADD_RTN_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 97:// DS_SUB_RTN_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 98:// DS_RSUB_RTN_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 99:// DS_INC_RTN_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 100:// DS_DEC_RTN_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 101:// DS_MIN_RTN_I64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 102:// DS_MAX_RTN_I64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 103:// DS_MIN_RTN_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 104:// DS_MAX_RTN_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 105:// DS_AND_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 106:// DS_OR_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 107:// DS_XOR_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 108:// DS_MSKOR_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 109:// DS_WRXCHG_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 110:// DS_WRXCHG2_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 111:// DS_WRXCHG2ST64_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 112:// DS_CMPST_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 113:// DS_CMPST_RTN_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA1+1,32),true,false);
break;
case 114:// DS_MIN_RTN_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 115:// DS_MAX_RTN_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 118:// DS_READ_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 119:// DS_READ2_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 120:// DS_READ2ST64_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 126:// DS_CONDXCHG32_RTN_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
break;
case 128:// DS_ADD_SRC2_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 129:// DS_SUB_SRC2_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 130:// DS_RSUB_SRC2_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 131:// DS_INC_SRC2_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 132:// DS_DEC_SRC2_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 133:// DS_MIN_SRC2_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 134:// DS_MAX_SRC2_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 135:// DS_MIN_SRC2_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 136:// DS_MAX_SRC2_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 137:// DS_AND_SRC2_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 138:// DS_OR_SRC2_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 139:// DS_XOR_SRC2_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 141:// DS_WRITE_SRC2_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 146:// DS_MIN_SRC2_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 147:// DS_MAX_SRC2_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 149:// DS_ADD_SRC2_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 152:// DS_GWS_SEMA_RELEASE_ALL
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 153:// DS_GWS_INIT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 154:// DS_GWS_SEMA_V
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 155:// DS_GWS_SEMA_BR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 156:// DS_GWS_SEMA_P
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 157:// DS_GWS_BARRIER
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 182:// DS_READ_ADDTID_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 189:// DS_CONSUME
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 190:// DS_APPEND
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 191:// DS_ORDERED_COUNT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 192:// DS_ADD_SRC2_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 193:// DS_SUB_SRC2_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 194:// DS_RSUB_SRC2_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 195:// DS_INC_SRC2_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 196:// DS_DEC_SRC2_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 197:// DS_MIN_SRC2_I64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 198:// DS_MAX_SRC2_I64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 199:// DS_MIN_SRC2_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 200:// DS_MAX_SRC2_U64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 201:// DS_AND_SRC2_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 202:// DS_OR_SRC2_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 203:// DS_XOR_SRC2_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 205:// DS_WRITE_SRC2_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 210:// DS_MIN_SRC2_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 211:// DS_MAX_SRC2_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 222:// DS_WRITE_B96
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+2,32),true,false);
break;
case 223:// DS_WRITE_B128
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA0+3,32),true,false);
break;
case 254:// DS_READ_B96
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
case 255:// DS_READ_B128
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_FLATOperands(){
layout_ENC_FLAT & layout = insn_layout.ENC_FLAT;
switch(layout.OP){
case 16:// FLAT_LOAD_UBYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 17:// FLAT_LOAD_SBYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 18:// FLAT_LOAD_USHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 19:// FLAT_LOAD_SSHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 20:// FLAT_LOAD_DWORD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 21:// FLAT_LOAD_DWORDX2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 22:// FLAT_LOAD_DWORDX3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 23:// FLAT_LOAD_DWORDX4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 24:// FLAT_STORE_BYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 25:// FLAT_STORE_BYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 26:// FLAT_STORE_SHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 27:// FLAT_STORE_SHORT_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 28:// FLAT_STORE_DWORD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 29:// FLAT_STORE_DWORDX2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 30:// FLAT_STORE_DWORDX3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 31:// FLAT_STORE_DWORDX4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 32:// FLAT_LOAD_UBYTE_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 33:// FLAT_LOAD_UBYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 34:// FLAT_LOAD_SBYTE_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 35:// FLAT_LOAD_SBYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 36:// FLAT_LOAD_SHORT_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 37:// FLAT_LOAD_SHORT_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 64:// FLAT_ATOMIC_SWAP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 65:// FLAT_ATOMIC_CMPSWAP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 66:// FLAT_ATOMIC_ADD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 67:// FLAT_ATOMIC_SUB
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 68:// FLAT_ATOMIC_SMIN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 69:// FLAT_ATOMIC_UMIN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 70:// FLAT_ATOMIC_SMAX
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 71:// FLAT_ATOMIC_UMAX
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 72:// FLAT_ATOMIC_AND
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 73:// FLAT_ATOMIC_OR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 74:// FLAT_ATOMIC_XOR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 75:// FLAT_ATOMIC_INC
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 76:// FLAT_ATOMIC_DEC
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 96:// FLAT_ATOMIC_SWAP_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 97:// FLAT_ATOMIC_CMPSWAP_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 98:// FLAT_ATOMIC_ADD_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 99:// FLAT_ATOMIC_SUB_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 100:// FLAT_ATOMIC_SMIN_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 101:// FLAT_ATOMIC_UMIN_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 102:// FLAT_ATOMIC_SMAX_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 103:// FLAT_ATOMIC_UMAX_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 104:// FLAT_ATOMIC_AND_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 105:// FLAT_ATOMIC_OR_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 106:// FLAT_ATOMIC_XOR_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 107:// FLAT_ATOMIC_INC_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 108:// FLAT_ATOMIC_DEC_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_FLAT_GLBLOperands(){
layout_ENC_FLAT_GLBL & layout = insn_layout.ENC_FLAT_GLBL;
switch(layout.OP){
case 16:// GLOBAL_LOAD_UBYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 17:// GLOBAL_LOAD_SBYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 18:// GLOBAL_LOAD_USHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 19:// GLOBAL_LOAD_SSHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 20:// GLOBAL_LOAD_DWORD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 21:// GLOBAL_LOAD_DWORDX2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 22:// GLOBAL_LOAD_DWORDX3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 23:// GLOBAL_LOAD_DWORDX4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 24:// GLOBAL_STORE_BYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 25:// GLOBAL_STORE_BYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 26:// GLOBAL_STORE_SHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 27:// GLOBAL_STORE_SHORT_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 28:// GLOBAL_STORE_DWORD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 29:// GLOBAL_STORE_DWORDX2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 30:// GLOBAL_STORE_DWORDX3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 31:// GLOBAL_STORE_DWORDX4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 32:// GLOBAL_LOAD_UBYTE_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 33:// GLOBAL_LOAD_UBYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 34:// GLOBAL_LOAD_SBYTE_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 35:// GLOBAL_LOAD_SBYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 36:// GLOBAL_LOAD_SHORT_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 37:// GLOBAL_LOAD_SHORT_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 64:// GLOBAL_ATOMIC_SWAP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 65:// GLOBAL_ATOMIC_CMPSWAP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 66:// GLOBAL_ATOMIC_ADD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 67:// GLOBAL_ATOMIC_SUB
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 68:// GLOBAL_ATOMIC_SMIN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 69:// GLOBAL_ATOMIC_UMIN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 70:// GLOBAL_ATOMIC_SMAX
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 71:// GLOBAL_ATOMIC_UMAX
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 72:// GLOBAL_ATOMIC_AND
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 73:// GLOBAL_ATOMIC_OR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 74:// GLOBAL_ATOMIC_XOR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 75:// GLOBAL_ATOMIC_INC
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 76:// GLOBAL_ATOMIC_DEC
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 77:// GLOBAL_ATOMIC_ADD_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 78:// GLOBAL_ATOMIC_PK_ADD_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 96:// GLOBAL_ATOMIC_SWAP_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 97:// GLOBAL_ATOMIC_CMPSWAP_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 98:// GLOBAL_ATOMIC_ADD_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 99:// GLOBAL_ATOMIC_SUB_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 100:// GLOBAL_ATOMIC_SMIN_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 101:// GLOBAL_ATOMIC_UMIN_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 102:// GLOBAL_ATOMIC_SMAX_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 103:// GLOBAL_ATOMIC_UMAX_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 104:// GLOBAL_ATOMIC_AND_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 105:// GLOBAL_ATOMIC_OR_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 106:// GLOBAL_ATOMIC_XOR_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 107:// GLOBAL_ATOMIC_INC_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 108:// GLOBAL_ATOMIC_DEC_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_FLAT_SCRATCHOperands(){
layout_ENC_FLAT_SCRATCH & layout = insn_layout.ENC_FLAT_SCRATCH;
switch(layout.OP){
case 16:// SCRATCH_LOAD_UBYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 17:// SCRATCH_LOAD_SBYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 18:// SCRATCH_LOAD_USHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 19:// SCRATCH_LOAD_SSHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 20:// SCRATCH_LOAD_DWORD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 21:// SCRATCH_LOAD_DWORDX2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 22:// SCRATCH_LOAD_DWORDX3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 23:// SCRATCH_LOAD_DWORDX4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 24:// SCRATCH_STORE_BYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 25:// SCRATCH_STORE_BYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 26:// SCRATCH_STORE_SHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 27:// SCRATCH_STORE_SHORT_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 28:// SCRATCH_STORE_DWORD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 29:// SCRATCH_STORE_DWORDX2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 30:// SCRATCH_STORE_DWORDX3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 31:// SCRATCH_STORE_DWORDX4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.DATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 32:// SCRATCH_LOAD_UBYTE_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 33:// SCRATCH_LOAD_UBYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 34:// SCRATCH_LOAD_SBYTE_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 35:// SCRATCH_LOAD_SBYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 36:// SCRATCH_LOAD_SHORT_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 37:// SCRATCH_LOAD_SHORT_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.ADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_FLAT_SCRATCH(0,64),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_MIMGOperands(){
layout_ENC_MIMG & layout = insn_layout.ENC_MIMG;
switch(layout.OP){
case 0:// IMAGE_LOAD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 1:// IMAGE_LOAD_MIP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 2:// IMAGE_LOAD_PCK
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 3:// IMAGE_LOAD_PCK_SGN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 4:// IMAGE_LOAD_MIP_PCK
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 5:// IMAGE_LOAD_MIP_PCK_SGN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 8:// IMAGE_STORE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 9:// IMAGE_STORE_MIP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 10:// IMAGE_STORE_PCK
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 11:// IMAGE_STORE_MIP_PCK
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 14:// IMAGE_GET_RESINFO
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 16:// IMAGE_ATOMIC_SWAP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 17:// IMAGE_ATOMIC_CMPSWAP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 18:// IMAGE_ATOMIC_ADD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 19:// IMAGE_ATOMIC_SUB
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 20:// IMAGE_ATOMIC_SMIN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 21:// IMAGE_ATOMIC_UMIN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 22:// IMAGE_ATOMIC_SMAX
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 23:// IMAGE_ATOMIC_UMAX
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 24:// IMAGE_ATOMIC_AND
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 25:// IMAGE_ATOMIC_OR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 26:// IMAGE_ATOMIC_XOR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 27:// IMAGE_ATOMIC_INC
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 28:// IMAGE_ATOMIC_DEC
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 32:// IMAGE_SAMPLE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 33:// IMAGE_SAMPLE_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 34:// IMAGE_SAMPLE_D
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 35:// IMAGE_SAMPLE_D_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 36:// IMAGE_SAMPLE_L
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 37:// IMAGE_SAMPLE_B
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 38:// IMAGE_SAMPLE_B_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 39:// IMAGE_SAMPLE_LZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 40:// IMAGE_SAMPLE_C
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 41:// IMAGE_SAMPLE_C_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 42:// IMAGE_SAMPLE_C_D
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 43:// IMAGE_SAMPLE_C_D_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+10,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 44:// IMAGE_SAMPLE_C_L
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 45:// IMAGE_SAMPLE_C_B
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 46:// IMAGE_SAMPLE_C_B_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 47:// IMAGE_SAMPLE_C_LZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 48:// IMAGE_SAMPLE_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 49:// IMAGE_SAMPLE_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 50:// IMAGE_SAMPLE_D_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 51:// IMAGE_SAMPLE_D_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+10,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 52:// IMAGE_SAMPLE_L_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 53:// IMAGE_SAMPLE_B_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 54:// IMAGE_SAMPLE_B_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 55:// IMAGE_SAMPLE_LZ_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 56:// IMAGE_SAMPLE_C_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 57:// IMAGE_SAMPLE_C_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 58:// IMAGE_SAMPLE_C_D_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+10,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 59:// IMAGE_SAMPLE_C_D_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+10,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+11,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 60:// IMAGE_SAMPLE_C_L_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 61:// IMAGE_SAMPLE_C_B_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 62:// IMAGE_SAMPLE_C_B_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 63:// IMAGE_SAMPLE_C_LZ_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 64:// IMAGE_GATHER4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 65:// IMAGE_GATHER4_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 66:// IMAGE_GATHER4H
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 68:// IMAGE_GATHER4_L
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 69:// IMAGE_GATHER4_B
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 70:// IMAGE_GATHER4_B_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 71:// IMAGE_GATHER4_LZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 72:// IMAGE_GATHER4_C
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 73:// IMAGE_GATHER4_C_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 74:// IMAGE_GATHER4H_PCK
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 75:// IMAGE_GATHER8H_PCK
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 76:// IMAGE_GATHER4_C_L
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 77:// IMAGE_GATHER4_C_B
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 78:// IMAGE_GATHER4_C_B_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 79:// IMAGE_GATHER4_C_LZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 80:// IMAGE_GATHER4_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 81:// IMAGE_GATHER4_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 84:// IMAGE_GATHER4_L_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 85:// IMAGE_GATHER4_B_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 86:// IMAGE_GATHER4_B_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 87:// IMAGE_GATHER4_LZ_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 88:// IMAGE_GATHER4_C_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 89:// IMAGE_GATHER4_C_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 92:// IMAGE_GATHER4_C_L_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 93:// IMAGE_GATHER4_C_B_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 94:// IMAGE_GATHER4_C_B_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 95:// IMAGE_GATHER4_C_LZ_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 96:// IMAGE_GET_LOD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 104:// IMAGE_SAMPLE_CD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 105:// IMAGE_SAMPLE_CD_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 106:// IMAGE_SAMPLE_C_CD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 107:// IMAGE_SAMPLE_C_CD_CL
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+10,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 108:// IMAGE_SAMPLE_CD_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 109:// IMAGE_SAMPLE_CD_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+10,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 110:// IMAGE_SAMPLE_C_CD_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+10,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 111:// IMAGE_SAMPLE_C_CD_CL_O
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+8,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+9,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+10,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+11,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+4,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+5,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+6,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+7,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSAMP+3,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_MTBUFOperands(){
layout_ENC_MTBUF & layout = insn_layout.ENC_MTBUF;
switch(layout.OP){
case 0:// TBUFFER_LOAD_FORMAT_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 1:// TBUFFER_LOAD_FORMAT_XY
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 2:// TBUFFER_LOAD_FORMAT_XYZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 3:// TBUFFER_LOAD_FORMAT_XYZW
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 4:// TBUFFER_STORE_FORMAT_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 5:// TBUFFER_STORE_FORMAT_XY
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 6:// TBUFFER_STORE_FORMAT_XYZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 7:// TBUFFER_STORE_FORMAT_XYZW
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 8:// TBUFFER_LOAD_FORMAT_D16_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 9:// TBUFFER_LOAD_FORMAT_D16_XY
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 10:// TBUFFER_LOAD_FORMAT_D16_XYZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 11:// TBUFFER_LOAD_FORMAT_D16_XYZW
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 12:// TBUFFER_STORE_FORMAT_D16_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 13:// TBUFFER_STORE_FORMAT_D16_XY
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 14:// TBUFFER_STORE_FORMAT_D16_XYZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 15:// TBUFFER_STORE_FORMAT_D16_XYZW
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_MUBUFOperands(){
layout_ENC_MUBUF & layout = insn_layout.ENC_MUBUF;
switch(layout.OP){
case 0:// BUFFER_LOAD_FORMAT_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 1:// BUFFER_LOAD_FORMAT_XY
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 2:// BUFFER_LOAD_FORMAT_XYZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 3:// BUFFER_LOAD_FORMAT_XYZW
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 4:// BUFFER_STORE_FORMAT_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 5:// BUFFER_STORE_FORMAT_XY
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 6:// BUFFER_STORE_FORMAT_XYZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 7:// BUFFER_STORE_FORMAT_XYZW
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 8:// BUFFER_LOAD_FORMAT_D16_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 9:// BUFFER_LOAD_FORMAT_D16_XY
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 10:// BUFFER_LOAD_FORMAT_D16_XYZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 11:// BUFFER_LOAD_FORMAT_D16_XYZW
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 12:// BUFFER_STORE_FORMAT_D16_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 13:// BUFFER_STORE_FORMAT_D16_XY
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 14:// BUFFER_STORE_FORMAT_D16_XYZ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 15:// BUFFER_STORE_FORMAT_D16_XYZW
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 16:// BUFFER_LOAD_UBYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 17:// BUFFER_LOAD_SBYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 18:// BUFFER_LOAD_USHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 19:// BUFFER_LOAD_SSHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 20:// BUFFER_LOAD_DWORD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 21:// BUFFER_LOAD_DWORDX2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 22:// BUFFER_LOAD_DWORDX3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 23:// BUFFER_LOAD_DWORDX4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 24:// BUFFER_STORE_BYTE
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 25:// BUFFER_STORE_BYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 26:// BUFFER_STORE_SHORT
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 27:// BUFFER_STORE_SHORT_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 28:// BUFFER_STORE_DWORD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 29:// BUFFER_STORE_DWORDX2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 30:// BUFFER_STORE_DWORDX3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 31:// BUFFER_STORE_DWORDX4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 32:// BUFFER_LOAD_UBYTE_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 33:// BUFFER_LOAD_UBYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 34:// BUFFER_LOAD_SBYTE_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 35:// BUFFER_LOAD_SBYTE_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 36:// BUFFER_LOAD_SHORT_D16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 37:// BUFFER_LOAD_SHORT_D16_HI
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 38:// BUFFER_LOAD_FORMAT_D16_HI_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 39:// BUFFER_STORE_FORMAT_D16_HI_X
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 61:// BUFFER_STORE_LDS_DWORD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 62:// BUFFER_WBINVL1
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 63:// BUFFER_WBINVL1_VOL
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 64:// BUFFER_ATOMIC_SWAP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 65:// BUFFER_ATOMIC_CMPSWAP
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 66:// BUFFER_ATOMIC_ADD
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 67:// BUFFER_ATOMIC_SUB
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 68:// BUFFER_ATOMIC_SMIN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 69:// BUFFER_ATOMIC_UMIN
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 70:// BUFFER_ATOMIC_SMAX
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 71:// BUFFER_ATOMIC_UMAX
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 72:// BUFFER_ATOMIC_AND
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 73:// BUFFER_ATOMIC_OR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 74:// BUFFER_ATOMIC_XOR
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 75:// BUFFER_ATOMIC_INC
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 76:// BUFFER_ATOMIC_DEC
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 77:// BUFFER_ATOMIC_ADD_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 78:// BUFFER_ATOMIC_PK_ADD_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 96:// BUFFER_ATOMIC_SWAP_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 97:// BUFFER_ATOMIC_CMPSWAP_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 98:// BUFFER_ATOMIC_ADD_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 99:// BUFFER_ATOMIC_SUB_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 100:// BUFFER_ATOMIC_SMIN_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 101:// BUFFER_ATOMIC_UMIN_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 102:// BUFFER_ATOMIC_SMAX_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 103:// BUFFER_ATOMIC_UMAX_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 104:// BUFFER_ATOMIC_AND_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 105:// BUFFER_ATOMIC_OR_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 106:// BUFFER_ATOMIC_XOR_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 107:// BUFFER_ATOMIC_INC_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
case 108:// BUFFER_ATOMIC_DEC_X2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VADDR+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRSRC+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::vmcnt,0,36),true,true,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_SMEMOperands(){
layout_ENC_SMEM & layout = insn_layout.ENC_SMEM;
switch(layout.OP){
case 0:// S_LOAD_DWORD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 1:// S_LOAD_DWORDX2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 2:// S_LOAD_DWORDX4
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 3:// S_LOAD_DWORDX8
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+4,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+5,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+6,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+7,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 4:// S_LOAD_DWORDX16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+4,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+5,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+6,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+7,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+8,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+9,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+10,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+11,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+12,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+13,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+14,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+15,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 5:// S_SCRATCH_LOAD_DWORD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 6:// S_SCRATCH_LOAD_DWORDX2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 7:// S_SCRATCH_LOAD_DWORDX4
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 8:// S_BUFFER_LOAD_DWORD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 9:// S_BUFFER_LOAD_DWORDX2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 10:// S_BUFFER_LOAD_DWORDX4
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 11:// S_BUFFER_LOAD_DWORDX8
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+4,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+5,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+6,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+7,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 12:// S_BUFFER_LOAD_DWORDX16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+4,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+5,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+6,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+7,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+8,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+9,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+10,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+11,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+12,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+13,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+14,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+15,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 16:// S_STORE_DWORD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 17:// S_STORE_DWORDX2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 18:// S_STORE_DWORDX4
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 21:// S_SCRATCH_STORE_DWORD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 22:// S_SCRATCH_STORE_DWORDX2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 23:// S_SCRATCH_STORE_DWORDX4
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 24:// S_BUFFER_STORE_DWORD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 25:// S_BUFFER_STORE_DWORDX2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 26:// S_BUFFER_STORE_DWORDX4
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 32:// S_DCACHE_INV
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 33:// S_DCACHE_WB
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 34:// S_DCACHE_INV_VOL
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 35:// S_DCACHE_WB_VOL
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 36:// S_MEMTIME
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 37:// S_MEMREALTIME
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),false,true);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 38:// S_ATC_PROBE
insn_in_progress->appendOperand(decodeOPR_SIMM8(layout.SDATA),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 39:// S_ATC_PROBE_BUFFER
insn_in_progress->appendOperand(decodeOPR_SIMM8(layout.SDATA),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 40:// S_DCACHE_DISCARD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 41:// S_DCACHE_DISCARD_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 64:// S_BUFFER_ATOMIC_SWAP
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 65:// S_BUFFER_ATOMIC_CMPSWAP
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 66:// S_BUFFER_ATOMIC_ADD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 67:// S_BUFFER_ATOMIC_SUB
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 68:// S_BUFFER_ATOMIC_SMIN
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 69:// S_BUFFER_ATOMIC_UMIN
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 70:// S_BUFFER_ATOMIC_SMAX
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 71:// S_BUFFER_ATOMIC_UMAX
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 72:// S_BUFFER_ATOMIC_AND
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 73:// S_BUFFER_ATOMIC_OR
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 74:// S_BUFFER_ATOMIC_XOR
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 75:// S_BUFFER_ATOMIC_INC
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 76:// S_BUFFER_ATOMIC_DEC
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 96:// S_BUFFER_ATOMIC_SWAP_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 97:// S_BUFFER_ATOMIC_CMPSWAP_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 98:// S_BUFFER_ATOMIC_ADD_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 99:// S_BUFFER_ATOMIC_SUB_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 100:// S_BUFFER_ATOMIC_SMIN_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 101:// S_BUFFER_ATOMIC_UMIN_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 102:// S_BUFFER_ATOMIC_SMAX_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 103:// S_BUFFER_ATOMIC_UMAX_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 104:// S_BUFFER_ATOMIC_AND_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 105:// S_BUFFER_ATOMIC_OR_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 106:// S_BUFFER_ATOMIC_XOR_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 107:// S_BUFFER_ATOMIC_INC_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 108:// S_BUFFER_ATOMIC_DEC_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+3,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 128:// S_ATOMIC_SWAP
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 129:// S_ATOMIC_CMPSWAP
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 130:// S_ATOMIC_ADD
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 131:// S_ATOMIC_SUB
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 132:// S_ATOMIC_SMIN
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 133:// S_ATOMIC_UMIN
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 134:// S_ATOMIC_SMAX
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 135:// S_ATOMIC_UMAX
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 136:// S_ATOMIC_AND
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 137:// S_ATOMIC_OR
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 138:// S_ATOMIC_XOR
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 139:// S_ATOMIC_INC
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 140:// S_ATOMIC_DEC
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 160:// S_ATOMIC_SWAP_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 161:// S_ATOMIC_CMPSWAP_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+2,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+3,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 162:// S_ATOMIC_ADD_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 163:// S_ATOMIC_SUB_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 164:// S_ATOMIC_SMIN_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 165:// S_ATOMIC_UMIN_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 166:// S_ATOMIC_SMAX_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 167:// S_ATOMIC_UMAX_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 168:// S_ATOMIC_AND_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 169:// S_ATOMIC_OR_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 170:// S_ATOMIC_XOR_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 171:// S_ATOMIC_INC_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
case 172:// S_ATOMIC_DEC_X2
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDATA+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SBASE+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SMEM_OFFSET(layout.SOFFSET,32),true,false);
insn_in_progress->appendOperand(makeRegisterExpression(amdgpu_gfx908::lgkmcnt,0,16),true,true,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_SOP1Operands(){
layout_ENC_SOP1 & layout = insn_layout.ENC_SOP1;
switch(layout.OP){
case 0:// S_MOV_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 1:// S_MOV_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
break;
case 2:// S_CMOV_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 3:// S_CMOV_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 4:// S_NOT_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 5:// S_NOT_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 6:// S_WQM_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 7:// S_WQM_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 8:// S_BREV_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 9:// S_BREV_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
break;
case 10:// S_BCNT0_I32_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 11:// S_BCNT0_I32_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 12:// S_BCNT1_I32_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 13:// S_BCNT1_I32_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 14:// S_FF0_I32_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 15:// S_FF0_I32_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
break;
case 16:// S_FF1_I32_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 17:// S_FF1_I32_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
break;
case 18:// S_FLBIT_I32_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 19:// S_FLBIT_I32_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
break;
case 20:// S_FLBIT_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 21:// S_FLBIT_I32_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
break;
case 22:// S_SEXT_I32_I8
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,16),true,false);
break;
case 23:// S_SEXT_I32_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,16),true,false);
break;
case 24:// S_BITSET0_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 25:// S_BITSET0_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 26:// S_BITSET1_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 27:// S_BITSET1_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
case 28:// S_GETPC_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),true,false);
break;
case 29:// S_SETPC_B64
setBranch();
setModifyPC();
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),false,true);
break;
case 30:// S_SWAPPC_B64
setBranch();
setModifyPC();
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),true,false);
break;
case 31:// S_RFE_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),false,true);
break;
case 32:// S_AND_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 33:// S_OR_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 34:// S_XOR_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 35:// S_ANDN2_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 36:// S_ORN2_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 37:// S_NAND_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 38:// S_NOR_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 39:// S_XNOR_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 40:// S_QUADMASK_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 41:// S_QUADMASK_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 42:// S_MOVRELS_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 43:// S_MOVRELS_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 44:// S_MOVRELD_B32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 45:// S_MOVRELD_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 46:// S_CBRANCH_JOIN
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),false,true);
break;
case 48:// S_ABS_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 50:// S_SET_GPR_IDX_IDX
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 51:// S_ANDN1_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 52:// S_ORN1_SAVEEXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 53:// S_ANDN1_WREXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 54:// S_ANDN2_WREXEC_B64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 55:// S_BITREPLICATE_B64_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_SOP2Operands(){
layout_ENC_SOP2 & layout = insn_layout.ENC_SOP2;
switch(layout.OP){
case 0:// S_ADD_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 1:// S_SUB_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 2:// S_ADD_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 3:// S_SUB_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 4:// S_ADDC_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 5:// S_SUBB_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 6:// S_MIN_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 7:// S_MIN_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 8:// S_MAX_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 9:// S_MAX_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 10:// S_CSELECT_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 11:// S_CSELECT_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 12:// S_AND_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 13:// S_AND_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 14:// S_OR_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 15:// S_OR_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 16:// S_XOR_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 17:// S_XOR_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 18:// S_ANDN2_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 19:// S_ANDN2_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 20:// S_ORN2_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 21:// S_ORN2_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 22:// S_NAND_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 23:// S_NAND_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 24:// S_NOR_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 25:// S_NOR_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 26:// S_XNOR_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 27:// S_XNOR_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 28:// S_LSHL_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 29:// S_LSHL_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 30:// S_LSHR_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 31:// S_LSHR_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 32:// S_ASHR_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 33:// S_ASHR_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 34:// S_BFM_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
break;
case 35:// S_BFM_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
break;
case 36:// S_MUL_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
break;
case 37:// S_BFE_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 38:// S_BFE_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 39:// S_BFE_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 40:// S_BFE_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 41:// S_CBRANCH_G_FORK
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),false,true);
break;
case 42:// S_ABSDIFF_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 43:// S_RFE_RESTORE_B64
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),false,true);
break;
case 44:// S_MUL_HI_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
break;
case 45:// S_MUL_HI_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
break;
case 46:// S_LSHL1_ADD_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 47:// S_LSHL2_ADD_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 48:// S_LSHL3_ADD_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 49:// S_LSHL4_ADD_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 50:// S_PACK_LL_B32_B16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,16),true,false);
break;
case 51:// S_PACK_LH_B32_B16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
break;
case 52:// S_PACK_HH_B32_B16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_SOPCOperands(){
layout_ENC_SOPC & layout = insn_layout.ENC_SOPC;
switch(layout.OP){
case 0:// S_CMP_EQ_I32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 1:// S_CMP_LG_I32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 2:// S_CMP_GT_I32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 3:// S_CMP_GE_I32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 4:// S_CMP_LT_I32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 5:// S_CMP_LE_I32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 6:// S_CMP_EQ_U32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 7:// S_CMP_LG_U32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 8:// S_CMP_GT_U32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 9:// S_CMP_GE_U32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 10:// S_CMP_LT_U32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 11:// S_CMP_LE_U32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 12:// S_BITCMP0_B32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 13:// S_BITCMP1_B32
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 14:// S_BITCMP0_B64
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 15:// S_BITCMP1_B64
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 16:// S_SETVSKIP
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1,32),true,false);
break;
case 17:// S_SET_GPR_IDX_ON
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM4(layout.SSRC1),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 18:// S_CMP_EQ_U64
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 19:// S_CMP_LG_U64
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC(layout.SSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_SOPKOperands(){
layout_ENC_SOPK & layout = insn_layout.ENC_SOPK;
switch(layout.OP){
case 0:// S_MOVK_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 1:// S_CMOVK_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 2:// S_CMPK_EQ_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 3:// S_CMPK_LG_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 4:// S_CMPK_GT_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 5:// S_CMPK_GE_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 6:// S_CMPK_LT_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 7:// S_CMPK_LE_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 8:// S_CMPK_EQ_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 9:// S_CMPK_LG_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 10:// S_CMPK_GT_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 11:// S_CMPK_GE_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 12:// S_CMPK_LT_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 13:// S_CMPK_LE_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 14:// S_ADDK_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),false,true);
break;
case 15:// S_MULK_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 16:// S_CBRANCH_I_FORK
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
break;
case 17:// S_GETREG_B32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 18:// S_SETREG_B32
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST,32),true,false);
break;
case 21:// S_CALL_B64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_PC(0,64),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeSOPK_INST_LITERAL_Operands(){
layout_SOPK_INST_LITERAL_ & layout = insn_layout.SOPK_INST_LITERAL_;
switch(layout.OP){
case 20:// S_SETREG_IMM32_B32
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),false,true);
insn_in_progress->appendOperand(decodeOPR_SIMM32(decodeOPR_LITERAL()),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_SOPPOperands(){
layout_ENC_SOPP & layout = insn_layout.ENC_SOPP;
switch(layout.OP){
case 0:// S_NOP
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 1:// S_ENDPGM
break;
case 2:// S_BRANCH
setBranch();
makeBranchTarget(isCall,isConditional,layout.SIMM16);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
break;
case 3:// S_WAKEUP
break;
case 4:// S_CBRANCH_SCC0
setBranch();
setConditionalBranch();
makeBranchTarget(isCall,isConditional,layout.SIMM16);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 5:// S_CBRANCH_SCC1
setBranch();
setConditionalBranch();
makeBranchTarget(isCall,isConditional,layout.SIMM16);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_SPECIAL_SCC(253,1),true,false);
break;
case 6:// S_CBRANCH_VCCZ
setBranch();
setConditionalBranch();
makeBranchTarget(isCall,isConditional,layout.SIMM16);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),true,false);
break;
case 7:// S_CBRANCH_VCCNZ
setBranch();
setConditionalBranch();
makeBranchTarget(isCall,isConditional,layout.SIMM16);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),true,false);
break;
case 8:// S_CBRANCH_EXECZ
setBranch();
setConditionalBranch();
makeBranchTarget(isCall,isConditional,layout.SIMM16);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 9:// S_CBRANCH_EXECNZ
setBranch();
setConditionalBranch();
makeBranchTarget(isCall,isConditional,layout.SIMM16);
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),true,false);
break;
case 10:// S_BARRIER
break;
case 11:// S_SETKILL
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 12:// S_WAITCNT
{
uint32_t vmcnt = ((0x3& (layout.SIMM16 >>14))<<4) | (layout.SIMM16 & 0xf);
uint32_t expcnt  ((layout.SIMM16>>4) & 0x7);
uint32_t lgkmcnt  ((layout.SIMM16>>8) & 0xf);
if (vmcnt != 0x3f) {
	insn_in_progress->appendOperand( makeRegisterExpression(amdgpu_gfx908::vmcnt, 0 ,32),false,true);
	insn_in_progress->appendOperand( Immediate::makeImmediate(Result(u32,vmcnt)),false,false);
}
if (expcnt != 0x7) {
	insn_in_progress->appendOperand( makeRegisterExpression(amdgpu_gfx908::expcnt, 0 ,32),false,true);
	insn_in_progress->appendOperand( Immediate::makeImmediate(Result(u32,expcnt)),false,false);
}
if (lgkmcnt != 0xf) {
	insn_in_progress->appendOperand( makeRegisterExpression(amdgpu_gfx908::lgkmcnt, 0 ,32),false,true);
	insn_in_progress->appendOperand( Immediate::makeImmediate(Result(u32,lgkmcnt)),false,false);
}
}
break;case 13:// S_SETHALT
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 14:// S_SLEEP
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 15:// S_SETPRIO
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 16:// S_SENDMSG
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 17:// S_SENDMSGHALT
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 18:// S_TRAP
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 19:// S_ICACHE_INV
break;
case 20:// S_INCPERFLEVEL
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 21:// S_DECPERFLEVEL
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
break;
case 22:// S_TTRACEDATA
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 23:// S_CBRANCH_CDBGSYS
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
break;
case 24:// S_CBRANCH_CDBGUSER
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
break;
case 25:// S_CBRANCH_CDBGSYS_OR_USER
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
break;
case 26:// S_CBRANCH_CDBGSYS_AND_USER
insn_in_progress->appendOperand(decodeOPR_LABEL(layout.SIMM16),true,false);
break;
case 27:// S_ENDPGM_SAVED
break;
case 28:// S_SET_GPR_IDX_OFF
break;
case 29:// S_SET_GPR_IDX_MODE
insn_in_progress->appendOperand(decodeOPR_SIMM16(layout.SIMM16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 30:// S_ENDPGM_ORDERED_PS_DONE
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VINTRPOperands(){
layout_ENC_VINTRP & layout = insn_layout.ENC_VINTRP;
switch(layout.OP){
case 0:// V_INTERP_P1_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.ATTR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 1:// V_INTERP_P2_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.ATTR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 2:// V_INTERP_MOV_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_PARAM(layout.VSRC,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.ATTR,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VOP3Operands(){
layout_ENC_VOP3 & layout = insn_layout.ENC_VOP3;
switch(layout.OP){
case 624:// V_INTERP_P1_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 625:// V_INTERP_P2_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 626:// V_INTERP_MOV_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_PARAM(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_M0(124,32),true,false);
break;
case 320:// V_NOP
break;
case 321:// V_MOV_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 322:// V_READFIRSTLANE_B32
insn_in_progress->appendOperand(decodeOPR_SREG_NOVCC(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR_OR_LDS(layout.SRC0,32),true,false);
break;
case 323:// V_CVT_I32_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 324:// V_CVT_F64_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 325:// V_CVT_F32_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 326:// V_CVT_F32_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 327:// V_CVT_U32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 328:// V_CVT_I32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 330:// V_CVT_F16_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 331:// V_CVT_F32_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 332:// V_CVT_RPI_I32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 333:// V_CVT_FLR_I32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 334:// V_CVT_OFF_F32_I4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 335:// V_CVT_F32_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 336:// V_CVT_F64_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 337:// V_CVT_F32_UBYTE0
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 338:// V_CVT_F32_UBYTE1
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 339:// V_CVT_F32_UBYTE2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 340:// V_CVT_F32_UBYTE3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 341:// V_CVT_U32_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 342:// V_CVT_F64_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 343:// V_TRUNC_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 344:// V_CEIL_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 345:// V_RNDNE_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 346:// V_FLOOR_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 347:// V_FRACT_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 348:// V_TRUNC_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 349:// V_CEIL_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 350:// V_RNDNE_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 351:// V_FLOOR_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 352:// V_EXP_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 353:// V_LOG_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 354:// V_RCP_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 355:// V_RCP_IFLAG_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 356:// V_RSQ_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 357:// V_RCP_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 358:// V_RSQ_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 359:// V_SQRT_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 360:// V_SQRT_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 361:// V_SIN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 362:// V_COS_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 363:// V_NOT_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 364:// V_BFREV_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 365:// V_FFBH_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 366:// V_FFBL_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 367:// V_FFBH_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 368:// V_FREXP_EXP_I32_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 369:// V_FREXP_MANT_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 370:// V_FRACT_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
break;
case 371:// V_FREXP_EXP_I32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 372:// V_FREXP_MANT_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 373:// V_CLREXCP
break;
case 375:// V_SCREEN_PARTITION_4SE_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 377:// V_CVT_F16_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 378:// V_CVT_F16_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 379:// V_CVT_U16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 380:// V_CVT_I16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 381:// V_RCP_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 382:// V_SQRT_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 383:// V_RSQ_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 384:// V_LOG_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 385:// V_EXP_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 386:// V_FREXP_MANT_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 387:// V_FREXP_EXP_I16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 388:// V_FLOOR_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 389:// V_CEIL_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 390:// V_TRUNC_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 391:// V_RNDNE_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 392:// V_FRACT_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 393:// V_SIN_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 394:// V_COS_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 395:// V_EXP_LEGACY_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 396:// V_LOG_LEGACY_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 397:// V_CVT_NORM_I16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 398:// V_CVT_NORM_U16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
break;
case 399:// V_SAT_PK_U8_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
case 401:// V_SWAP_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC0,32),true,true);
break;
case 256:// V_CNDMASK_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRC2+1,32),true,false);
break;
case 257:// V_ADD_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 258:// V_SUB_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 259:// V_SUBREV_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 260:// V_MUL_LEGACY_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 261:// V_MUL_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 262:// V_MUL_I32_I24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 263:// V_MUL_HI_I32_I24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 264:// V_MUL_U32_U24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 265:// V_MUL_HI_U32_U24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 266:// V_MIN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 267:// V_MAX_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 268:// V_MIN_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 269:// V_MAX_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 270:// V_MIN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 271:// V_MAX_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 272:// V_LSHRREV_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 273:// V_ASHRREV_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 274:// V_LSHLREV_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 275:// V_AND_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 276:// V_OR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 277:// V_XOR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 278:// V_MAC_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 287:// V_ADD_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 288:// V_SUB_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 289:// V_SUBREV_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 290:// V_MUL_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 291:// V_MAC_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 294:// V_ADD_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 295:// V_SUB_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 296:// V_SUBREV_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 297:// V_MUL_LO_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 298:// V_LSHLREV_B16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 299:// V_LSHRREV_B16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 300:// V_ASHRREV_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 301:// V_MAX_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 302:// V_MIN_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 303:// V_MAX_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 304:// V_MAX_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 305:// V_MIN_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 306:// V_MIN_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 307:// V_LDEXP_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 308:// V_ADD_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 309:// V_SUB_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 310:// V_SUBREV_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 311:// V_DOT2C_F32_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 312:// V_DOT2C_I32_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 313:// V_DOT4C_I32_I8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 314:// V_DOT8C_I32_I4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 315:// V_FMAC_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 316:// V_PK_FMAC_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 317:// V_XNOR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 448:// V_MAD_LEGACY_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 449:// V_MAD_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 450:// V_MAD_I32_I24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 451:// V_MAD_U32_U24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 452:// V_CUBEID_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 453:// V_CUBESC_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 454:// V_CUBETC_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 455:// V_CUBEMA_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 456:// V_BFE_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 457:// V_BFE_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 458:// V_BFI_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 459:// V_FMA_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 460:// V_FMA_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+1,32),true,false);
break;
case 461:// V_LERP_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 462:// V_ALIGNBIT_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 463:// V_ALIGNBYTE_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 464:// V_MIN3_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 465:// V_MIN3_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 466:// V_MIN3_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 467:// V_MAX3_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 468:// V_MAX3_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 469:// V_MAX3_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 470:// V_MED3_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 471:// V_MED3_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 472:// V_MED3_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 473:// V_SAD_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 474:// V_SAD_HI_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 475:// V_SAD_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 476:// V_SAD_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 477:// V_CVT_PK_U8_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 478:// V_DIV_FIXUP_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 479:// V_DIV_FIXUP_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+1,32),true,false);
break;
case 482:// V_DIV_FMAS_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),true,false);
break;
case 483:// V_DIV_FMAS_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),true,false);
break;
case 484:// V_MSAD_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 485:// V_QSAD_PK_U16_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+1,32),true,false);
break;
case 486:// V_MQSAD_PK_U16_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+1,32),true,false);
break;
case 487:// V_MQSAD_U32_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+2,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+3,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC2+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC2+2,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC2+3,32),true,false);
break;
case 490:// V_MAD_LEGACY_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 491:// V_MAD_LEGACY_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 492:// V_MAD_LEGACY_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 493:// V_PERM_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 494:// V_FMA_LEGACY_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 495:// V_DIV_FIXUP_LEGACY_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 496:// V_CVT_PKACCUM_U8_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 497:// V_MAD_U32_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 498:// V_MAD_I32_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 499:// V_XAD_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 500:// V_MIN3_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 501:// V_MIN3_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 502:// V_MIN3_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 503:// V_MAX3_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 504:// V_MAX3_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 505:// V_MAX3_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 506:// V_MED3_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 507:// V_MED3_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 508:// V_MED3_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 509:// V_LSHL_ADD_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 510:// V_ADD_LSHL_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 511:// V_ADD3_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 512:// V_LSHL_OR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 513:// V_AND_OR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 514:// V_OR3_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 515:// V_MAD_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 516:// V_MAD_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 517:// V_MAD_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 518:// V_FMA_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 519:// V_DIV_FIXUP_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 628:// V_INTERP_P1LL_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.SRC0,32),true,false);
break;
case 629:// V_INTERP_P1LV_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC2,32),true,false);
break;
case 630:// V_INTERP_P2_LEGACY_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC2,32),true,false);
break;
case 631:// V_INTERP_P2_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_ATTR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC2,32),true,false);
break;
case 640:// V_ADD_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 641:// V_MUL_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 642:// V_MIN_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 643:// V_MAX_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 644:// V_LDEXP_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 645:// V_MUL_LO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 646:// V_MUL_HI_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 647:// V_MUL_HI_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 648:// V_LDEXP_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 649:// V_READLANE_B32
insn_in_progress->appendOperand(decodeOPR_SREG_NOVCC(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR_OR_LDS(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_LANESEL(layout.SRC1,32),true,false);
break;
case 650:// V_WRITELANE_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SSRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SSRC_LANESEL(layout.SRC1,32),true,false);
break;
case 651:// V_BCNT_U32_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 652:// V_MBCNT_LO_U32_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 653:// V_MBCNT_HI_U32_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 655:// V_LSHLREV_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 656:// V_LSHRREV_B64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 657:// V_ASHRREV_I64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 658:// V_TRIG_PREOP_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 659:// V_BFM_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 660:// V_CVT_PKNORM_I16_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 661:// V_CVT_PKNORM_U16_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 662:// V_CVT_PKRTZ_F16_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 663:// V_CVT_PK_U16_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 664:// V_CVT_PK_I16_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 665:// V_CVT_PKNORM_I16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 666:// V_CVT_PKNORM_U16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 668:// V_ADD_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 669:// V_SUB_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 670:// V_ADD_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 671:// V_SUB_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 672:// V_PACK_B32_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 16:// V_CMP_CLASS_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 17:// V_CMPX_CLASS_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 18:// V_CMP_CLASS_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 19:// V_CMPX_CLASS_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 20:// V_CMP_CLASS_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 21:// V_CMPX_CLASS_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 32:// V_CMP_F_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 33:// V_CMP_LT_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 34:// V_CMP_EQ_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 35:// V_CMP_LE_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 36:// V_CMP_GT_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 37:// V_CMP_LG_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 38:// V_CMP_GE_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 39:// V_CMP_O_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 40:// V_CMP_U_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 41:// V_CMP_NGE_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 42:// V_CMP_NLG_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 43:// V_CMP_NGT_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 44:// V_CMP_NLE_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 45:// V_CMP_NEQ_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 46:// V_CMP_NLT_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 47:// V_CMP_TRU_F16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 48:// V_CMPX_F_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 49:// V_CMPX_LT_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 50:// V_CMPX_EQ_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 51:// V_CMPX_LE_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 52:// V_CMPX_GT_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 53:// V_CMPX_LG_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 54:// V_CMPX_GE_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 55:// V_CMPX_O_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 56:// V_CMPX_U_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 57:// V_CMPX_NGE_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 58:// V_CMPX_NLG_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 59:// V_CMPX_NGT_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 60:// V_CMPX_NLE_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 61:// V_CMPX_NEQ_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 62:// V_CMPX_NLT_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 63:// V_CMPX_TRU_F16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 64:// V_CMP_F_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 65:// V_CMP_LT_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 66:// V_CMP_EQ_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 67:// V_CMP_LE_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 68:// V_CMP_GT_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 69:// V_CMP_LG_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 70:// V_CMP_GE_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 71:// V_CMP_O_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 72:// V_CMP_U_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 73:// V_CMP_NGE_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 74:// V_CMP_NLG_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 75:// V_CMP_NGT_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 76:// V_CMP_NLE_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 77:// V_CMP_NEQ_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 78:// V_CMP_NLT_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 79:// V_CMP_TRU_F32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 80:// V_CMPX_F_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 81:// V_CMPX_LT_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 82:// V_CMPX_EQ_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 83:// V_CMPX_LE_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 84:// V_CMPX_GT_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 85:// V_CMPX_LG_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 86:// V_CMPX_GE_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 87:// V_CMPX_O_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 88:// V_CMPX_U_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 89:// V_CMPX_NGE_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 90:// V_CMPX_NLG_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 91:// V_CMPX_NGT_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 92:// V_CMPX_NLE_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 93:// V_CMPX_NEQ_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 94:// V_CMPX_NLT_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 95:// V_CMPX_TRU_F32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 96:// V_CMP_F_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 97:// V_CMP_LT_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 98:// V_CMP_EQ_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 99:// V_CMP_LE_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 100:// V_CMP_GT_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 101:// V_CMP_LG_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 102:// V_CMP_GE_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 103:// V_CMP_O_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 104:// V_CMP_U_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 105:// V_CMP_NGE_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 106:// V_CMP_NLG_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 107:// V_CMP_NGT_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 108:// V_CMP_NLE_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 109:// V_CMP_NEQ_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 110:// V_CMP_NLT_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 111:// V_CMP_TRU_F64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 112:// V_CMPX_F_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 113:// V_CMPX_LT_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 114:// V_CMPX_EQ_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 115:// V_CMPX_LE_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 116:// V_CMPX_GT_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 117:// V_CMPX_LG_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 118:// V_CMPX_GE_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 119:// V_CMPX_O_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 120:// V_CMPX_U_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 121:// V_CMPX_NGE_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 122:// V_CMPX_NLG_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 123:// V_CMPX_NGT_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 124:// V_CMPX_NLE_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 125:// V_CMPX_NEQ_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 126:// V_CMPX_NLT_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 127:// V_CMPX_TRU_F64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 160:// V_CMP_F_I16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 161:// V_CMP_LT_I16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 162:// V_CMP_EQ_I16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 163:// V_CMP_LE_I16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 164:// V_CMP_GT_I16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 165:// V_CMP_NE_I16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 166:// V_CMP_GE_I16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 167:// V_CMP_T_I16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 168:// V_CMP_F_U16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 169:// V_CMP_LT_U16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 170:// V_CMP_EQ_U16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 171:// V_CMP_LE_U16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 172:// V_CMP_GT_U16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 173:// V_CMP_NE_U16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 174:// V_CMP_GE_U16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 175:// V_CMP_T_U16
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 176:// V_CMPX_F_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 177:// V_CMPX_LT_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 178:// V_CMPX_EQ_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 179:// V_CMPX_LE_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 180:// V_CMPX_GT_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 181:// V_CMPX_NE_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 182:// V_CMPX_GE_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 183:// V_CMPX_T_I16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 184:// V_CMPX_F_U16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 185:// V_CMPX_LT_U16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 186:// V_CMPX_EQ_U16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 187:// V_CMPX_LE_U16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 188:// V_CMPX_GT_U16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 189:// V_CMPX_NE_U16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 190:// V_CMPX_GE_U16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 191:// V_CMPX_T_U16
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 192:// V_CMP_F_I32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 193:// V_CMP_LT_I32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 194:// V_CMP_EQ_I32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 195:// V_CMP_LE_I32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 196:// V_CMP_GT_I32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 197:// V_CMP_NE_I32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 198:// V_CMP_GE_I32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 199:// V_CMP_T_I32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 200:// V_CMP_F_U32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 201:// V_CMP_LT_U32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 202:// V_CMP_EQ_U32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 203:// V_CMP_LE_U32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 204:// V_CMP_GT_U32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 205:// V_CMP_NE_U32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 206:// V_CMP_GE_U32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 207:// V_CMP_T_U32
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 208:// V_CMPX_F_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 209:// V_CMPX_LT_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 210:// V_CMPX_EQ_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 211:// V_CMPX_LE_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 212:// V_CMPX_GT_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 213:// V_CMPX_NE_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 214:// V_CMPX_GE_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 215:// V_CMPX_T_I32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 216:// V_CMPX_F_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 217:// V_CMPX_LT_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 218:// V_CMPX_EQ_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 219:// V_CMPX_LE_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 220:// V_CMPX_GT_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 221:// V_CMPX_NE_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 222:// V_CMPX_GE_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 223:// V_CMPX_T_U32
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 224:// V_CMP_F_I64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 225:// V_CMP_LT_I64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 226:// V_CMP_EQ_I64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 227:// V_CMP_LE_I64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 228:// V_CMP_GT_I64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 229:// V_CMP_NE_I64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 230:// V_CMP_GE_I64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 231:// V_CMP_T_I64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 232:// V_CMP_F_U64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 233:// V_CMP_LT_U64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 234:// V_CMP_EQ_U64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 235:// V_CMP_LE_U64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 236:// V_CMP_GT_U64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 237:// V_CMP_NE_U64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 238:// V_CMP_GE_U64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 239:// V_CMP_T_U64
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
break;
case 240:// V_CMPX_F_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 241:// V_CMPX_LT_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 242:// V_CMPX_EQ_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 243:// V_CMPX_LE_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 244:// V_CMPX_GT_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 245:// V_CMPX_NE_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 246:// V_CMPX_GE_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 247:// V_CMPX_T_I64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 248:// V_CMPX_F_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 249:// V_CMPX_LT_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 250:// V_CMPX_EQ_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 251:// V_CMPX_LE_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 252:// V_CMPX_GT_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 253:// V_CMPX_NE_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 254:// V_CMPX_GE_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 255:// V_CMPX_T_U64
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SDST(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VOP1Operands(){
layout_ENC_VOP1 & layout = insn_layout.ENC_VOP1;
switch(layout.OP){
case 0:// V_NOP
break;
case 1:// V_MOV_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 2:// V_READFIRSTLANE_B32
insn_in_progress->appendOperand(decodeOPR_SREG_NOVCC(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR_OR_LDS(layout.SRC0,32),true,false);
break;
case 3:// V_CVT_I32_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 4:// V_CVT_F64_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 5:// V_CVT_F32_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 6:// V_CVT_F32_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 7:// V_CVT_U32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 8:// V_CVT_I32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 10:// V_CVT_F16_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 11:// V_CVT_F32_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 12:// V_CVT_RPI_I32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 13:// V_CVT_FLR_I32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 14:// V_CVT_OFF_F32_I4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 15:// V_CVT_F32_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 16:// V_CVT_F64_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 17:// V_CVT_F32_UBYTE0
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 18:// V_CVT_F32_UBYTE1
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 19:// V_CVT_F32_UBYTE2
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 20:// V_CVT_F32_UBYTE3
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 21:// V_CVT_U32_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 22:// V_CVT_F64_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 23:// V_TRUNC_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 24:// V_CEIL_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 25:// V_RNDNE_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 26:// V_FLOOR_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 27:// V_FRACT_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 28:// V_TRUNC_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 29:// V_CEIL_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 30:// V_RNDNE_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 31:// V_FLOOR_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 32:// V_EXP_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 33:// V_LOG_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 34:// V_RCP_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 35:// V_RCP_IFLAG_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 36:// V_RSQ_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 37:// V_RCP_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 38:// V_RSQ_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 39:// V_SQRT_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 40:// V_SQRT_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 41:// V_SIN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 42:// V_COS_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 43:// V_NOT_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 44:// V_BFREV_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 45:// V_FFBH_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 46:// V_FFBL_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 47:// V_FFBH_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 48:// V_FREXP_EXP_I32_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 49:// V_FREXP_MANT_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 50:// V_FRACT_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
break;
case 51:// V_FREXP_EXP_I32_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 52:// V_FREXP_MANT_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 53:// V_CLREXCP
break;
case 55:// V_SCREEN_PARTITION_4SE_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 57:// V_CVT_F16_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 58:// V_CVT_F16_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 59:// V_CVT_U16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 60:// V_CVT_I16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 61:// V_RCP_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 62:// V_SQRT_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 63:// V_RSQ_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 64:// V_LOG_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 65:// V_EXP_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 66:// V_FREXP_MANT_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 67:// V_FREXP_EXP_I16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 68:// V_FLOOR_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 69:// V_CEIL_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 70:// V_TRUNC_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 71:// V_RNDNE_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 72:// V_FRACT_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 73:// V_SIN_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 74:// V_COS_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 75:// V_EXP_LEGACY_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 76:// V_LOG_LEGACY_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 77:// V_CVT_NORM_I16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 78:// V_CVT_NORM_U16_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
break;
case 79:// V_SAT_PK_U8_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
break;
case 81:// V_SWAP_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR(layout.SRC0,32),true,true);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VOP2Operands(){
layout_ENC_VOP2 & layout = insn_layout.ENC_VOP2;
switch(layout.OP){
case 0:// V_CNDMASK_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),true,false);
break;
case 1:// V_ADD_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 2:// V_SUB_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 3:// V_SUBREV_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 4:// V_MUL_LEGACY_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 5:// V_MUL_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 6:// V_MUL_I32_I24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 7:// V_MUL_HI_I32_I24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 8:// V_MUL_U32_U24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 9:// V_MUL_HI_U32_U24
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 10:// V_MIN_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 11:// V_MAX_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 12:// V_MIN_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 13:// V_MAX_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 14:// V_MIN_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 15:// V_MAX_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 16:// V_LSHRREV_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 17:// V_ASHRREV_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 18:// V_LSHLREV_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 19:// V_AND_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 20:// V_OR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 21:// V_XOR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 22:// V_MAC_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 25:// V_ADD_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 26:// V_SUB_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 27:// V_SUBREV_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 28:// V_ADDC_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),true,false);
break;
case 29:// V_SUBB_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),true,false);
break;
case 30:// V_SUBBREV_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),true,false);
break;
case 31:// V_ADD_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 32:// V_SUB_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 33:// V_SUBREV_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 34:// V_MUL_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 35:// V_MAC_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 38:// V_ADD_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 39:// V_SUB_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 40:// V_SUBREV_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 41:// V_MUL_LO_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 42:// V_LSHLREV_B16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 43:// V_LSHRREV_B16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 44:// V_ASHRREV_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 45:// V_MAX_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 46:// V_MIN_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 47:// V_MAX_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 48:// V_MAX_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 49:// V_MIN_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 50:// V_MIN_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 51:// V_LDEXP_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 52:// V_ADD_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 53:// V_SUB_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 54:// V_SUBREV_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLDS(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 55:// V_DOT2C_F32_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 56:// V_DOT2C_I32_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 57:// V_DOT4C_I32_I8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 58:// V_DOT8C_I32_I4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 59:// V_FMAC_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 60:// V_PK_FMAC_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),true,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 61:// V_XNOR_B32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VOP2_LITERALOperands(){
layout_ENC_VOP2_LITERAL & layout = insn_layout.ENC_VOP2_LITERAL;
switch(layout.OP){
case 23:// V_MADMK_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM32(layout.SIMM32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 24:// V_MADAK_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM32(layout.SIMM32),true,false);
break;
case 36:// V_MADMK_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM32(layout.SIMM32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 37:// V_MADAK_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SIMM32(layout.SIMM32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VOP3BOperands(){
layout_ENC_VOP3B & layout = insn_layout.ENC_VOP3B;
switch(layout.OP){
case 281:// V_ADD_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 282:// V_SUB_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 283:// V_SUBREV_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 284:// V_ADDC_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRC2+1,32),true,false);
break;
case 285:// V_SUBB_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRC2+1,32),true,false);
break;
case 286:// V_SUBBREV_CO_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SRC2+1,32),true,false);
break;
case 480:// V_DIV_SCALE_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 481:// V_DIV_SCALE_F64
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VCC(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+1,32),true,false);
break;
case 488:// V_MAD_U64_U32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+1,32),true,false);
break;
case 489:// V_MAD_I64_I32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+0,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SREG(layout.SDST+1,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2+1,32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VOP3POperands(){
layout_ENC_VOP3P & layout = insn_layout.ENC_VOP3P;
switch(layout.OP){
case 0:// V_PK_MAD_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 1:// V_PK_MUL_LO_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 2:// V_PK_ADD_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 3:// V_PK_SUB_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 4:// V_PK_LSHLREV_B16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 5:// V_PK_LSHRREV_B16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 6:// V_PK_ASHRREV_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
break;
case 7:// V_PK_MAX_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 8:// V_PK_MIN_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 9:// V_PK_MAD_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 10:// V_PK_ADD_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 11:// V_PK_SUB_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 12:// V_PK_MAX_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 13:// V_PK_MIN_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 14:// V_PK_FMA_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,16),true,false);
break;
case 15:// V_PK_ADD_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 16:// V_PK_MUL_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 17:// V_PK_MIN_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 18:// V_PK_MAX_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
break;
case 32:// V_MAD_MIX_F32
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 33:// V_MAD_MIXLO_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 34:// V_MAD_MIXHI_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,16),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 35:// V_DOT2_F32_F16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 38:// V_DOT2_I32_I16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 39:// V_DOT2_U32_U16
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 40:// V_DOT4_I32_I8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 41:// V_DOT4_U32_U8
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 42:// V_DOT8_I32_I4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 43:// V_DOT8_U32_U4
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_SIMPLE(layout.SRC2,32),true,false);
break;
case 88:// V_ACCVGPR_READ
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR(layout.SRC0,32),true,false);
break;
case 89:// V_ACCVGPR_WRITE
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_NOLIT(layout.SRC0,32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VOP3P_MFMAOperands(){
layout_ENC_VOP3P_MFMA & layout = insn_layout.ENC_VOP3P_MFMA;
switch(layout.OP){
case 64:// V_MFMA_F32_32X32X1F32
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 65:// V_MFMA_F32_16X16X1F32
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 66:// V_MFMA_F32_4X4X1F32
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 68:// V_MFMA_F32_32X32X2F32
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 69:// V_MFMA_F32_16X16X4F32
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 72:// V_MFMA_F32_32X32X4F16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 73:// V_MFMA_F32_16X16X4F16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 74:// V_MFMA_F32_4X4X4F16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 76:// V_MFMA_F32_32X32X8F16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 77:// V_MFMA_F32_16X16X16F16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 80:// V_MFMA_I32_32X32X4I8
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 81:// V_MFMA_I32_16X16X4I8
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 82:// V_MFMA_I32_4X4X4I8
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 84:// V_MFMA_I32_32X32X8I8
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 85:// V_MFMA_I32_16X16X16I8
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 104:// V_MFMA_F32_32X32X2BF16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 105:// V_MFMA_F32_16X16X2BF16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 107:// V_MFMA_F32_4X4X2BF16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 108:// V_MFMA_F32_32X32X4BF16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
case 109:// V_MFMA_F32_16X16X8BF16
insn_in_progress->appendOperand(decodeOPR_ACCVGPR(layout.VDST,32),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_VGPR_OR_ACCVGPR(layout.SRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC_ACCVGPR_OR_CONST(layout.SRC2,32),true,false);
break;
}
}
void InstructionDecoder_amdgpu_gfx908::finalizeENC_VOPCOperands(){
layout_ENC_VOPC & layout = insn_layout.ENC_VOPC;
switch(layout.OP){
case 16:// V_CMP_CLASS_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 17:// V_CMPX_CLASS_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 18:// V_CMP_CLASS_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 19:// V_CMPX_CLASS_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 20:// V_CMP_CLASS_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 21:// V_CMPX_CLASS_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 32:// V_CMP_F_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 33:// V_CMP_LT_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 34:// V_CMP_EQ_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 35:// V_CMP_LE_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 36:// V_CMP_GT_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 37:// V_CMP_LG_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 38:// V_CMP_GE_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 39:// V_CMP_O_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 40:// V_CMP_U_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 41:// V_CMP_NGE_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 42:// V_CMP_NLG_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 43:// V_CMP_NGT_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 44:// V_CMP_NLE_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 45:// V_CMP_NEQ_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 46:// V_CMP_NLT_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 47:// V_CMP_TRU_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 48:// V_CMPX_F_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 49:// V_CMPX_LT_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 50:// V_CMPX_EQ_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 51:// V_CMPX_LE_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 52:// V_CMPX_GT_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 53:// V_CMPX_LG_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 54:// V_CMPX_GE_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 55:// V_CMPX_O_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 56:// V_CMPX_U_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 57:// V_CMPX_NGE_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 58:// V_CMPX_NLG_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 59:// V_CMPX_NGT_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 60:// V_CMPX_NLE_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 61:// V_CMPX_NEQ_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 62:// V_CMPX_NLT_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 63:// V_CMPX_TRU_F16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 64:// V_CMP_F_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 65:// V_CMP_LT_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 66:// V_CMP_EQ_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 67:// V_CMP_LE_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 68:// V_CMP_GT_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 69:// V_CMP_LG_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 70:// V_CMP_GE_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 71:// V_CMP_O_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 72:// V_CMP_U_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 73:// V_CMP_NGE_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 74:// V_CMP_NLG_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 75:// V_CMP_NGT_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 76:// V_CMP_NLE_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 77:// V_CMP_NEQ_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 78:// V_CMP_NLT_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 79:// V_CMP_TRU_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 80:// V_CMPX_F_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 81:// V_CMPX_LT_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 82:// V_CMPX_EQ_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 83:// V_CMPX_LE_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 84:// V_CMPX_GT_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 85:// V_CMPX_LG_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 86:// V_CMPX_GE_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 87:// V_CMPX_O_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 88:// V_CMPX_U_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 89:// V_CMPX_NGE_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 90:// V_CMPX_NLG_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 91:// V_CMPX_NGT_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 92:// V_CMPX_NLE_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 93:// V_CMPX_NEQ_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 94:// V_CMPX_NLT_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 95:// V_CMPX_TRU_F32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 96:// V_CMP_F_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 97:// V_CMP_LT_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 98:// V_CMP_EQ_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 99:// V_CMP_LE_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 100:// V_CMP_GT_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 101:// V_CMP_LG_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 102:// V_CMP_GE_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 103:// V_CMP_O_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 104:// V_CMP_U_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 105:// V_CMP_NGE_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 106:// V_CMP_NLG_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 107:// V_CMP_NGT_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 108:// V_CMP_NLE_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 109:// V_CMP_NEQ_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 110:// V_CMP_NLT_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 111:// V_CMP_TRU_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 112:// V_CMPX_F_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 113:// V_CMPX_LT_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 114:// V_CMPX_EQ_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 115:// V_CMPX_LE_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 116:// V_CMPX_GT_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 117:// V_CMPX_LG_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 118:// V_CMPX_GE_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 119:// V_CMPX_O_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 120:// V_CMPX_U_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 121:// V_CMPX_NGE_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 122:// V_CMPX_NLG_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 123:// V_CMPX_NGT_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 124:// V_CMPX_NLE_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 125:// V_CMPX_NEQ_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 126:// V_CMPX_NLT_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 127:// V_CMPX_TRU_F64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 160:// V_CMP_F_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 161:// V_CMP_LT_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 162:// V_CMP_EQ_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 163:// V_CMP_LE_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 164:// V_CMP_GT_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 165:// V_CMP_NE_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 166:// V_CMP_GE_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 167:// V_CMP_T_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 168:// V_CMP_F_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 169:// V_CMP_LT_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 170:// V_CMP_EQ_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 171:// V_CMP_LE_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 172:// V_CMP_GT_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 173:// V_CMP_NE_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 174:// V_CMP_GE_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 175:// V_CMP_T_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
break;
case 176:// V_CMPX_F_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 177:// V_CMPX_LT_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 178:// V_CMPX_EQ_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 179:// V_CMPX_LE_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 180:// V_CMPX_GT_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 181:// V_CMPX_NE_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 182:// V_CMPX_GE_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 183:// V_CMPX_T_I16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 184:// V_CMPX_F_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 185:// V_CMPX_LT_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 186:// V_CMPX_EQ_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 187:// V_CMPX_LE_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 188:// V_CMPX_GT_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 189:// V_CMPX_NE_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 190:// V_CMPX_GE_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 191:// V_CMPX_T_U16
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,16),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,16),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 192:// V_CMP_F_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 193:// V_CMP_LT_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 194:// V_CMP_EQ_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 195:// V_CMP_LE_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 196:// V_CMP_GT_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 197:// V_CMP_NE_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 198:// V_CMP_GE_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 199:// V_CMP_T_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 200:// V_CMP_F_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 201:// V_CMP_LT_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 202:// V_CMP_EQ_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 203:// V_CMP_LE_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 204:// V_CMP_GT_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 205:// V_CMP_NE_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 206:// V_CMP_GE_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 207:// V_CMP_T_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
break;
case 208:// V_CMPX_F_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 209:// V_CMPX_LT_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 210:// V_CMPX_EQ_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 211:// V_CMPX_LE_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 212:// V_CMPX_GT_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 213:// V_CMPX_NE_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 214:// V_CMPX_GE_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 215:// V_CMPX_T_I32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 216:// V_CMPX_F_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 217:// V_CMPX_LT_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 218:// V_CMPX_EQ_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 219:// V_CMPX_LE_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 220:// V_CMPX_GT_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 221:// V_CMPX_NE_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 222:// V_CMPX_GE_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 223:// V_CMPX_T_U32
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 224:// V_CMP_F_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 225:// V_CMP_LT_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 226:// V_CMP_EQ_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 227:// V_CMP_LE_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 228:// V_CMP_GT_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 229:// V_CMP_NE_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 230:// V_CMP_GE_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 231:// V_CMP_T_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 232:// V_CMP_F_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 233:// V_CMP_LT_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 234:// V_CMP_EQ_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 235:// V_CMP_LE_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 236:// V_CMP_GT_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 237:// V_CMP_NE_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 238:// V_CMP_GE_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 239:// V_CMP_T_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
break;
case 240:// V_CMPX_F_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 241:// V_CMPX_LT_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 242:// V_CMPX_EQ_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 243:// V_CMPX_LE_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 244:// V_CMPX_GT_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 245:// V_CMPX_NE_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 246:// V_CMPX_GE_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 247:// V_CMPX_T_I64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 248:// V_CMPX_F_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 249:// V_CMPX_LT_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 250:// V_CMPX_EQ_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 251:// V_CMPX_LE_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 252:// V_CMPX_GT_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 253:// V_CMPX_NE_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 254:// V_CMPX_GE_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
case 255:// V_CMPX_T_U64
insn_in_progress->appendOperand(decodeOPR_VCC(0,64),false,true);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SRC(layout.SRC0+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+0,32),true,false);
insn_in_progress->appendOperand(decodeOPR_VGPR(layout.VSRC1+1,32),true,false);
insn_in_progress->appendOperand(decodeOPR_SDST_EXEC(126,64),false,true);
break;
}
}
