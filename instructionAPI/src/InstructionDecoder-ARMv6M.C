// This content was generated on Mon Oct 31 17:04:25 CET 2016
// Do not edit directly.
// Contact: eda@tum

#include "InstructionDecoder-ARMv6M.h"
#include "Immediate.h"
#include "common/src/singleton_object_pool.h"


template <typename R, typename T>
static R CastBits(T value)
{
	return static_cast<R>(value);
}

template <typename R, typename T>
static R CoerceBits(T value)
{
	return static_cast<R>(value);
}

template <typename T>
static T RotateLeft(T value, int amount)
{
	static_assert(std::is_unsigned<T>::value, "Type must be unsigned");
	return (value << amount) | (value >> (8*sizeof(T) - amount));
}

template <typename T>
static T RotateRight(T value, int amount)
{
	static_assert(std::is_unsigned<T>::value, "Type must be unsigned");
	return (value >> amount) | (value << (8*sizeof(T) - amount));
}

template <typename T>
static T MakeSliceMask(T high, T low)
{
	static_assert(std::is_unsigned<T>::value, "Type must be unsigned");
	T ones = high - low + 1;
	// Shift a full 1-bit pattern left to generate zeroes. Then negate to get ones.
	return ~(~static_cast<T>(0) << ones) << low;
}


namespace Dyninst {
namespace InstructionAPI {

InstructionDecoder_ARMv6M::InstructionDecoder_ARMv6M(Architecture arch)
	: InstructionDecoderImpl(arch)
{
}

void InstructionDecoder_ARMv6M::decodeOpcode(InstructionDecoder::buffer &buf) {
	doDecode(buf);
}

Instruction::Ptr InstructionDecoder_ARMv6M::decode(InstructionDecoder::buffer &buf)
{
	doDecode(buf);
	return m_instrInProgress;
}

bool InstructionDecoder_ARMv6M::decodeOperands(const Instruction *insn_to_complete) {
	return true;
}

Result_Type InstructionDecoder_ARMv6M::makeSizeType(unsigned int) {
	throw std::runtime_error("Not implemented");
}

void InstructionDecoder_ARMv6M::doDecode(InstructionDecoder::buffer &buf) {
	m_bytesRead = 0;
	
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf000) == 0xd000)
	{
		// Mask:    0b1111'0000'0000'0000
		// Compare: 0b1101'0000'0000'0000
		printf("Mask:    0b1111'0000'0000'0000\n");
		printf("Compare: 0b1101'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x0)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BEQ
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BEQ.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BEQ, "BEQ", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_Z{ Type: Bit } == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x100)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0001'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0001'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BNE
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BNE.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BNE, "BNE", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_Z{ Type: Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x200)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0010'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0010'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BCS
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BCS.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BCS, "BCS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_C{ Type: Bit } == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x300)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0011'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0011'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BCC
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BCC.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BCC, "BCC", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_C{ Type: Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x200)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0010'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0010'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BHS
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BHS.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BHS, "BHS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_C{ Type: Bit } == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x300)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0011'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0011'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BLO
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BLO.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BLO, "BLO", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_C{ Type: Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x400)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0100'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0100'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BMI
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BMI.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BMI, "BMI", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_N{ Type: Bit } == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x500)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0101'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0101'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BPL
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BPL.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BPL, "BPL", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_N{ Type: Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x600)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0110'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0110'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BVS
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BVS.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BVS, "BVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_V{ Type: Bit } == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x700)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0111'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0111'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BVC
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BVC.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BVC, "BVC", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_V{ Type: Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x800)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1000'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BHI
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BHI.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BHI, "BHI", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if (((MEMREF_C{ Type: Bit } == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes } && (MEMREF_Z{ Type: Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// read Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x900)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1001'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1001'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BLS
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BLS.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BLS, "BLS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if (((MEMREF_C{ Type: Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes } || (MEMREF_Z{ Type: Bit } == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// read Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xa00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1010'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1010'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BGE
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BGE.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BGE, "BGE", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_N{ Type: Bit } == MEMREF_V{ Type: Bit }){ Type: Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), true, false);
				// read V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xb00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1011'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1011'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BLT
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BLT.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BLT, "BLT", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if ((MEMREF_N{ Type: Bit } != MEMREF_V{ Type: Bit }){ Type: Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), true, false);
				// read V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xc00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1100'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1100'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BGT
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BGT.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BGT, "BGT", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if (((MEMREF_Z{ Type: Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes } && (MEMREF_N{ Type: Bit } == MEMREF_V{ Type: Bit }){ Type: Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), true, false);
				// read N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), true, false);
				// read V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xd00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1101'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1101'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BLE
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				// mnemonic: BLE.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 8
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BLE, "BLE", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				struct { Word field : 8; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }if (((MEMREF_Z{ Type: Bit } == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes } || (MEMREF_N{ Type: Bit } != MEMREF_V{ Type: Bit }){ Type: Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// read Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), true, false);
				// read N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), true, false);
				// read V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), true, false);
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, true, false);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u8, 2)), u32), false, false, true, true);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xe00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1110'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1110'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: imm
				preString: #
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: imm
				preString: #
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_UDF, "UDF", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				Debug string end */
				
				/*
				
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xe00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1110'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1110'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: imm
				preString: 
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: imm
				preString: 
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_UDF2, "UDF", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				Debug string end */
				
				/*
				
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xf00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1111'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1111'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: imm
				preString: #
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: imm
				preString: #
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SVC, "SVC", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				 { EXCEPTION: SVCall } 
				Debug string end */
				
				/*
				
				;
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xf00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1111'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1111'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: imm
				preString: 
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: imm
				preString: 
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SVC2, "SVC", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				 { EXCEPTION: SVCall } 
				Debug string end */
				
				/*
				
				;
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe000) == 0xe000)
	{
		// Mask:    0b1110'0000'0000'0000
		// Compare: 0b1110'0000'0000'0000
		printf("Mask:    0b1110'0000'0000'0000\n");
		printf("Compare: 0b1110'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x0)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b0'0000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b0'0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: BAL
				Syn param:
				operand: label
				preString: 
				bitWidth: 11
				signed: true
				type: str
				// mnemonic: BAL.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 11
				signed: true
				type: str
				// mnemonic: B
				Syn param:
				operand: label
				preString: 
				bitWidth: 11
				signed: true
				type: str
				// mnemonic: B.N
				Syn param:
				operand: label
				preString: 
				bitWidth: 11
				signed: true
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BAL, "BAL", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1111'1111
				struct { Word field : 11; } signExtender_label;
				Word param_label = signExtender_label.field = ((m_instrWord >> 16) & 0x7ff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (LITERAL_label << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				int32_t local_imm32;
				
				local_imm32 = CoerceBits<int32_t>((param_label << 0x1));
				// write PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
				m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), false, false, false, false);
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 0) & 0xf8000000) == 0xf0000000)
	{
		// Mask:    0b1111'1000'0000'0000'0000'0000'0000'0000
		// Compare: 0b1111'0000'0000'0000'0000'0000'0000'0000
		printf("Mask:    0b1111'1000'0000'0000'0000'0000'0000'0000\n");
		printf("Compare: 0b1111'0000'0000'0000'0000'0000'0000'0000\n");
		
		if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0xc000) == 0xc000)
		{
			// Mask:    0b1100'0000'0000'0000
			// Compare: 0b1100'0000'0000'0000
			printf("Mask:    0b1100'0000'0000'0000\n");
			printf("Compare: 0b1100'0000'0000'0000\n");
			
			if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x1000) == 0x1000)
			{
				// Mask:    0b1'0000'0000'0000
				// Compare: 0b1'0000'0000'0000
				printf("Mask:    0b1'0000'0000'0000\n");
				printf("Compare: 0b1'0000'0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: BL
					Syn param:
					operand: label
					preString: 
					bitWidth: 24
					signed: false
					type: str
					// mnemonic: BL.W
					Syn param:
					operand: label
					preString: 
					bitWidth: 24
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BL, "BL", 4, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(3, buf);
					
					// TODO resolve slice definitions: [11]
					// Mask: 0b1000'0000'0000
					UWord param_label = ((m_instrWord >> 0) & 0x800) >> 11;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(4, buf);
					
					// TODO resolve slice definitions: [10, 0]
					// Mask: 0b111'1111'1111
					struct { Word field : 11; } signExtender_label;
					Word param_label = signExtender_label.field = ((m_instrWord >> 0) & 0x7ff) >> 0;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(3, buf);
					
					// TODO resolve slice definitions: [12]
					// Mask: 0b10'0000'0000'0000
					UWord param_label = ((m_instrWord >> 0) & 0x2000) >> 13;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(1, buf);
					
					// TODO resolve slice definitions: [23]
					// Mask: 0b100'0000'0000'0000'0000'0000'0000
					UWord param_label = ((m_instrWord >> 0) & 0x4000000) >> 26;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: [22, 13]
					// Mask: 0b11'1111'1111'0000'0000'0000'0000
					struct { Word field : 10; } signExtender_label;
					Word param_label = signExtender_label.field = ((m_instrWord >> 0) & 0x3ff0000) >> 16;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_S = LITERAL_label<CONSTNUM_0x17..CONSTNUM_0x17>){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_J1 = LITERAL_label<CONSTNUM_0xc..CONSTNUM_0xc>){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_J2 = LITERAL_label<CONSTNUM_0xb..CONSTNUM_0xb>){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_I1 = (~(LOCAL_LITERAL_J1 ^ LOCAL_LITERAL_S){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_I2 = (~(LOCAL_LITERAL_J2 ^ LOCAL_LITERAL_S){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm10 = LITERAL_label<CONSTNUM_0x16..CONSTNUM_0xd>){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm11 = LITERAL_label<CONSTNUM_0xa..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(SIGNED, 32, (((((LOCAL_LITERAL_S << CONSTNUM_0x18){ Type: Unsigned32Bit, MemRef: no } | (LOCAL_LITERAL_I1 << CONSTNUM_0x17){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no } | (LOCAL_LITERAL_I2 << CONSTNUM_0x16){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no } | (LOCAL_LITERAL_imm10 << CONSTNUM_0xc){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no } | (LOCAL_LITERAL_imm11 << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no })){ Type: Signed32Bit, MemRef: no }(MEMREF_LR{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } | CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_PC{ Type: Unsigned32Bit } = (MEMREF_PC{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm10;
					int32_t local_imm32;
					uint32_t local_S;
					uint32_t local_imm11;
					uint32_t local_J1;
					uint32_t local_J2;
					uint32_t local_I1;
					uint32_t local_I2;
					
					local_S = ((param_label & 0x800000) >> 0x17);
					local_J1 = ((param_label & 0x1000) >> 0xc);
					local_J2 = ((param_label & 0x800) >> 0xb);
					local_I1 = ~(local_J1 ^ local_S);
					local_I2 = ~(local_J2 ^ local_S);
					local_imm10 = ((param_label & 0x0) >> 0xd);
					local_imm11 = ((param_label & 0x0) >> 0x0);
					local_imm32 = CoerceBits<int32_t>((((((local_S << 0x18) | (local_I1 << 0x17)) | (local_I2 << 0x16)) | (local_imm10 << 0xc)) | (local_imm11 << 0x1)));
					// write LR
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::LR), false, true);
					// read PC
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
					// write PC
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
					m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(s32, local_imm32)), u32), true, false, false, false);
					// read PC
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 0) & 0x4000000) == 0x0)
		{
			// Mask:    0b100'0000'0000'0000'0000'0000'0000
			// Compare: 0b000'0000'0000'0000'0000'0000'0000
			printf("Mask:    0b100'0000'0000'0000'0000'0000'0000\n");
			printf("Compare: 0b000'0000'0000'0000'0000'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x3800000) == 0x3800000)
			{
				// Mask:    0b11'1000'0000'0000'0000'0000'0000
				// Compare: 0b11'1000'0000'0000'0000'0000'0000
				printf("Mask:    0b11'1000'0000'0000'0000'0000'0000\n");
				printf("Compare: 0b11'1000'0000'0000'0000'0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x600000) == 0x200000)
				{
					// Mask:    0b110'0000'0000'0000'0000'0000
					// Compare: 0b010'0000'0000'0000'0000'0000
					printf("Mask:    0b110'0000'0000'0000'0000'0000\n");
					printf("Compare: 0b010'0000'0000'0000'0000'0000\n");
					
					if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x100000) == 0x100000)
					{
						// Mask:    0b1'0000'0000'0000'0000'0000
						// Compare: 0b1'0000'0000'0000'0000'0000
						printf("Mask:    0b1'0000'0000'0000'0000'0000\n");
						printf("Compare: 0b1'0000'0000'0000'0000'0000\n");
						
						if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0xf0000) == 0xf0000)
						{
							// Mask:    0b1111'0000'0000'0000'0000
							// Compare: 0b1111'0000'0000'0000'0000
							printf("Mask:    0b1111'0000'0000'0000'0000\n");
							printf("Compare: 0b1111'0000'0000'0000'0000\n");
							
							if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0xc000) == 0x8000)
							{
								// Mask:    0b1100'0000'0000'0000
								// Compare: 0b1000'0000'0000'0000
								printf("Mask:    0b1100'0000'0000'0000\n");
								printf("Compare: 0b1000'0000'0000'0000\n");
								
								if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x2000) == 0x0)
								{
									// Mask:    0b10'0000'0000'0000
									// Compare: 0b00'0000'0000'0000
									printf("Mask:    0b10'0000'0000'0000\n");
									printf("Compare: 0b00'0000'0000'0000\n");
									
									if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x1000) == 0x0)
									{
										// Mask:    0b1'0000'0000'0000
										// Compare: 0b0'0000'0000'0000
										printf("Mask:    0b1'0000'0000'0000\n");
										printf("Compare: 0b0'0000'0000'0000\n");
										
										if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0xf00) == 0xf00)
										{
											// Mask:    0b1111'0000'0000
											// Compare: 0b1111'0000'0000
											printf("Mask:    0b1111'0000'0000\n");
											printf("Compare: 0b1111'0000'0000\n");
											
											if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf0) == 0x50)
											{
												// Mask:    0b1111'0000
												// Compare: 0b0101'0000
												printf("Mask:    0b1111'0000\n");
												printf("Compare: 0b0101'0000\n");
												
												if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf) == 0xf)
												{
													// Mask:    0b1111
													// Compare: 0b1111
													printf("Mask:    0b1111\n");
													printf("Compare: 0b1111\n");
													
													{
														/* SyntaxNode dump:
														// mnemonic: 
														Syn param:
														operand: 
														preString: SY
														bitWidth: 0
														signed: false
														type: str
														// mnemonic: 
														Syn param:
														operand: 
														preString: SY
														bitWidth: 0
														signed: false
														type: str
														End SyntaxNode dump */
														
														m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_DMB, "DMB", 4, buf.start));
														
													
														/* Debug string begin
														Debug string end */
														
														/*
														
														*/
													}
												}
												else
												if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf) == 0xf)
												{
													// Mask:    0b1111
													// Compare: 0b1111
													printf("Mask:    0b1111\n");
													printf("Compare: 0b1111\n");
													
													{
														/* SyntaxNode dump:
														// mnemonic: 
														// mnemonic: 
														End SyntaxNode dump */
														
														m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_DMB2, "DMB", 4, buf.start));
														
													
														/* Debug string begin
														Debug string end */
														
														/*
														
														*/
													}
												}
												else
												{
												// Invalid instruction.
												m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
												}
											}
											else
											if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf0) == 0x40)
											{
												// Mask:    0b1111'0000
												// Compare: 0b0100'0000
												printf("Mask:    0b1111'0000\n");
												printf("Compare: 0b0100'0000\n");
												
												if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf) == 0xf)
												{
													// Mask:    0b1111
													// Compare: 0b1111
													printf("Mask:    0b1111\n");
													printf("Compare: 0b1111\n");
													
													{
														/* SyntaxNode dump:
														// mnemonic: 
														// mnemonic: 
														End SyntaxNode dump */
														
														m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_DSB, "DSB", 4, buf.start));
														
													
														/* Debug string begin
														Debug string end */
														
														/*
														
														*/
													}
												}
												else
												if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf) == 0xf)
												{
													// Mask:    0b1111
													// Compare: 0b1111
													printf("Mask:    0b1111\n");
													printf("Compare: 0b1111\n");
													
													{
														/* SyntaxNode dump:
														// mnemonic: 
														Syn param:
														operand: 
														preString: SY
														bitWidth: 0
														signed: false
														type: str
														// mnemonic: 
														Syn param:
														operand: 
														preString: SY
														bitWidth: 0
														signed: false
														type: str
														End SyntaxNode dump */
														
														m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_DSB2, "DSB", 4, buf.start));
														
													
														/* Debug string begin
														Debug string end */
														
														/*
														
														*/
													}
												}
												else
												{
												// Invalid instruction.
												m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
												}
											}
											else
											if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf0) == 0x60)
											{
												// Mask:    0b1111'0000
												// Compare: 0b0110'0000
												printf("Mask:    0b1111'0000\n");
												printf("Compare: 0b0110'0000\n");
												
												if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf) == 0xf)
												{
													// Mask:    0b1111
													// Compare: 0b1111
													printf("Mask:    0b1111\n");
													printf("Compare: 0b1111\n");
													
													{
														/* SyntaxNode dump:
														// mnemonic: 
														Syn param:
														operand: 
														preString: SY
														bitWidth: 0
														signed: false
														type: str
														// mnemonic: 
														Syn param:
														operand: 
														preString: SY
														bitWidth: 0
														signed: false
														type: str
														End SyntaxNode dump */
														
														m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ISB, "ISB", 4, buf.start));
														
													
														/* Debug string begin
														Debug string end */
														
														/*
														
														*/
													}
												}
												else
												if (ensureWeHave(4, buf), ((m_instrWord >> 0) & 0xf) == 0xf)
												{
													// Mask:    0b1111
													// Compare: 0b1111
													printf("Mask:    0b1111\n");
													printf("Compare: 0b1111\n");
													
													{
														/* SyntaxNode dump:
														// mnemonic: 
														// mnemonic: 
														End SyntaxNode dump */
														
														m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ISB2, "ISB", 4, buf.start));
														
													
														/* Debug string begin
														Debug string end */
														
														/*
														
														*/
													}
												}
												else
												{
												// Invalid instruction.
												m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
												}
											}
											else
											{
											// Invalid instruction.
											m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
											}
										}
										else
										{
										// Invalid instruction.
										m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
										}
									}
									else
									{
									// Invalid instruction.
									m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
									}
								}
								else
								{
								// Invalid instruction.
								m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
								}
							}
							else
							{
							// Invalid instruction.
							m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
							}
						}
						else
						{
						// Invalid instruction.
						m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
						}
					}
					else
					{
					// Invalid instruction.
					m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x3c00000) == 0x3c00000)
			{
				// Mask:    0b11'1100'0000'0000'0000'0000'0000
				// Compare: 0b11'1100'0000'0000'0000'0000'0000
				printf("Mask:    0b11'1100'0000'0000'0000'0000'0000\n");
				printf("Compare: 0b11'1100'0000'0000'0000'0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x200000) == 0x200000)
				{
					// Mask:    0b10'0000'0000'0000'0000'0000
					// Compare: 0b10'0000'0000'0000'0000'0000
					printf("Mask:    0b10'0000'0000'0000'0000'0000\n");
					printf("Compare: 0b10'0000'0000'0000'0000'0000\n");
					
					if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x100000) == 0x0)
					{
						// Mask:    0b1'0000'0000'0000'0000'0000
						// Compare: 0b0'0000'0000'0000'0000'0000
						printf("Mask:    0b1'0000'0000'0000'0000'0000\n");
						printf("Compare: 0b0'0000'0000'0000'0000'0000\n");
						
						if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0xf0000) == 0xf0000)
						{
							// Mask:    0b1111'0000'0000'0000'0000
							// Compare: 0b1111'0000'0000'0000'0000
							printf("Mask:    0b1111'0000'0000'0000'0000\n");
							printf("Compare: 0b1111'0000'0000'0000'0000\n");
							
							if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0xc000) == 0x8000)
							{
								// Mask:    0b1100'0000'0000'0000
								// Compare: 0b1000'0000'0000'0000
								printf("Mask:    0b1100'0000'0000'0000\n");
								printf("Compare: 0b1000'0000'0000'0000\n");
								
								if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x2000) == 0x0)
								{
									// Mask:    0b10'0000'0000'0000
									// Compare: 0b00'0000'0000'0000
									printf("Mask:    0b10'0000'0000'0000\n");
									printf("Compare: 0b00'0000'0000'0000\n");
									
									if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x1000) == 0x0)
									{
										// Mask:    0b1'0000'0000'0000
										// Compare: 0b0'0000'0000'0000
										printf("Mask:    0b1'0000'0000'0000\n");
										printf("Compare: 0b0'0000'0000'0000\n");
										
										{
											/* SyntaxNode dump:
											// mnemonic: MRS
											Syn param:
											operand: d
											preString: R
											bitWidth: 4
											signed: false
											type: dec
											Syn param:
											operand: sysm
											preString: ,
											bitWidth: 8
											signed: false
											type: str
											// mnemonic: MRS.W
											Syn param:
											operand: d
											preString: R
											bitWidth: 4
											signed: false
											type: dec
											Syn param:
											operand: sysm
											preString: ,
											bitWidth: 8
											signed: false
											type: str
											End SyntaxNode dump */
											
											m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MRS, "MRS", 4, buf.start));
											
											// EncodingParameter begin
											/*ensureWeHave(3, buf);
											
											// TODO resolve slice definitions: []
											// Mask: 0b1111'0000'0000
											UWord param_d = ((m_instrWord >> 0) & 0xf00) >> 8;
											*/
											// EncodingParameter end
											// EncodingParameter begin
											/*ensureWeHave(4, buf);
											
											// TODO resolve slice definitions: []
											// Mask: 0b1111'1111
											UWord param_sysm = ((m_instrWord >> 0) & 0xff) >> 0;
											*/
											// EncodingParameter end
										
											/* Debug string begin
											(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }if ((LITERAL_sysm<CONSTNUM_0x7..CONSTNUM_0x3> == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
											if ((LITERAL_sysm<CONSTNUM_0x0..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
											(MEMREF_R[LITERAL_d]<CONSTNUM_0x8..CONSTNUM_0x0>{ Type: Unsigned32Bit } = MEMREF_PSR<CONSTNUM_0x8..CONSTNUM_0x0>{ Type: Bit }){ Type: Bit, MemRef: yes }}
											if ((LITERAL_sysm<CONSTNUM_0x1..CONSTNUM_0x1> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
											(MEMREF_R[LITERAL_d]<CONSTNUM_0x18..CONSTNUM_0x18>{ Type: Unsigned32Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
											if ((LITERAL_sysm<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
											(MEMREF_R[LITERAL_d]<CONSTNUM_0x1f..CONSTNUM_0x1b>{ Type: Unsigned32Bit } = MEMREF_PSR<CONSTNUM_0x1f..CONSTNUM_0x1b>{ Type: Bit }){ Type: Bit, MemRef: yes }}
											} else {
											if ((LITERAL_sysm<CONSTNUM_0x7..CONSTNUM_0x3> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
											if ((LITERAL_sysm<CONSTNUM_0x2..CONSTNUM_0x0> == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
											(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }} else {
											if ((LITERAL_sysm<CONSTNUM_0x2..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
											(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
											}
											} else {
											if ((LITERAL_sysm<CONSTNUM_0x7..CONSTNUM_0x3> == CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no }) {
											if ((LITERAL_sysm<CONSTNUM_0x2..CONSTNUM_0x0> == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
											(MEMREF_R[LITERAL_d]<CONSTNUM_0x0..CONSTNUM_0x0>{ Type: Unsigned32Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }} else {
											if ((LITERAL_sysm<CONSTNUM_0x2..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
											(MEMREF_R[LITERAL_d]<CONSTNUM_0x1..CONSTNUM_0x0>{ Type: Unsigned32Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
											}
											}
											}
											}
											Debug string end */
											
											/*
											
											// write R
											m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
											// Maybe PC write:
											if (param_d == 15)
											{
												m_instrInProgress->addSuccessor(Immediate::makeImmediate(Result(u32, 0x0)), false, false, false, false);
											}
											if ((((param_sysm & 0x0) >> 0x3) == 0x0))
											{
												if ((((param_sysm & 0x1) >> 0x0) == 0x1))
												{
													// write R
													m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
													// Maybe PC write:
													if (param_d == 15)
													{
														m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::PSR), false, true, false, false);
													}
													// read PSR
													m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PSR), true, false);
												}
												if ((((param_sysm & 0x2) >> 0x1) == 0x1))
												{
													// write R
													m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
													// Maybe PC write:
													if (param_d == 15)
													{
														m_instrInProgress->addSuccessor(Immediate::makeImmediate(Result(u32, 0x0)), false, false, false, false);
													}
												}
												if ((((param_sysm & 0x4) >> 0x2) == 0x0))
												{
													// write R
													m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
													// Maybe PC write:
													if (param_d == 15)
													{
														m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::PSR), false, true, false, false);
													}
													// read PSR
													m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PSR), true, false);
												}
											} else {
												if ((((param_sysm & 0x0) >> 0x3) == 0x1))
												{
													if ((((param_sysm & 0x0) >> 0x0) == 0x0))
													{
														// write R
														m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
														// Maybe PC write:
														if (param_d == 15)
														{
															m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::SP), false, true, false, false);
														}
														// read SP
														m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
													} else {
														if ((((param_sysm & 0x0) >> 0x0) == 0x1))
														{
															// write R
															m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
															// Maybe PC write:
															if (param_d == 15)
															{
																m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::SP), false, true, false, false);
															}
															// read SP
															m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
														}
													}
												} else {
													if ((((param_sysm & 0x0) >> 0x3) == 0x2))
													{
														if ((((param_sysm & 0x0) >> 0x0) == 0x0))
														{
															// write R
															m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
															// Maybe PC write:
															if (param_d == 15)
															{
																m_instrInProgress->addSuccessor(Immediate::makeImmediate(Result(u32, 0x0)), false, false, false, false);
															}
														} else {
															if ((((param_sysm & 0x0) >> 0x0) == 0x1))
															{
																// write R
																m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
																// Maybe PC write:
																if (param_d == 15)
																{
																	m_instrInProgress->addSuccessor(Immediate::makeImmediate(Result(u32, 0x0)), false, false, false, false);
																}
															}
														}
													}
												}
											}
											*/
										}
									}
									else
									{
									// Invalid instruction.
									m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
									}
								}
								else
								{
								// Invalid instruction.
								m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
								}
							}
							else
							{
							// Invalid instruction.
							m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
							}
						}
						else
						{
						// Invalid instruction.
						m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
						}
					}
					else
					{
					// Invalid instruction.
					m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x3c00000) == 0x3800000)
			{
				// Mask:    0b11'1100'0000'0000'0000'0000'0000
				// Compare: 0b11'1000'0000'0000'0000'0000'0000
				printf("Mask:    0b11'1100'0000'0000'0000'0000'0000\n");
				printf("Compare: 0b11'1000'0000'0000'0000'0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x200000) == 0x0)
				{
					// Mask:    0b10'0000'0000'0000'0000'0000
					// Compare: 0b00'0000'0000'0000'0000'0000
					printf("Mask:    0b10'0000'0000'0000'0000'0000\n");
					printf("Compare: 0b00'0000'0000'0000'0000'0000\n");
					
					if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0x100000) == 0x0)
					{
						// Mask:    0b1'0000'0000'0000'0000'0000
						// Compare: 0b0'0000'0000'0000'0000'0000
						printf("Mask:    0b1'0000'0000'0000'0000'0000\n");
						printf("Compare: 0b0'0000'0000'0000'0000'0000\n");
						
						if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0xc000) == 0x8000)
						{
							// Mask:    0b1100'0000'0000'0000
							// Compare: 0b1000'0000'0000'0000
							printf("Mask:    0b1100'0000'0000'0000\n");
							printf("Compare: 0b1000'0000'0000'0000\n");
							
							if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x2000) == 0x0)
							{
								// Mask:    0b10'0000'0000'0000
								// Compare: 0b00'0000'0000'0000
								printf("Mask:    0b10'0000'0000'0000\n");
								printf("Compare: 0b00'0000'0000'0000\n");
								
								if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x1000) == 0x0)
								{
									// Mask:    0b1'0000'0000'0000
									// Compare: 0b0'0000'0000'0000
									printf("Mask:    0b1'0000'0000'0000\n");
									printf("Compare: 0b0'0000'0000'0000\n");
									
									if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x800) == 0x800)
									{
										// Mask:    0b1000'0000'0000
										// Compare: 0b1000'0000'0000
										printf("Mask:    0b1000'0000'0000\n");
										printf("Compare: 0b1000'0000'0000\n");
										
										if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x400) == 0x0)
										{
											// Mask:    0b100'0000'0000
											// Compare: 0b000'0000'0000
											printf("Mask:    0b100'0000'0000\n");
											printf("Compare: 0b000'0000'0000\n");
											
											if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x200) == 0x0)
											{
												// Mask:    0b10'0000'0000
												// Compare: 0b00'0000'0000
												printf("Mask:    0b10'0000'0000\n");
												printf("Compare: 0b00'0000'0000\n");
												
												if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0x100) == 0x0)
												{
													// Mask:    0b1'0000'0000
													// Compare: 0b0'0000'0000
													printf("Mask:    0b1'0000'0000\n");
													printf("Compare: 0b0'0000'0000\n");
													
													{
														/* SyntaxNode dump:
														// mnemonic: MSR
														Syn param:
														operand: sysm
														preString: 
														bitWidth: 8
														signed: false
														type: str
														Syn param:
														operand: n
														preString: ,R
														bitWidth: 4
														signed: false
														type: dec
														// mnemonic: MSR.W
														Syn param:
														operand: sysm
														preString: 
														bitWidth: 8
														signed: false
														type: str
														Syn param:
														operand: n
														preString: ,R
														bitWidth: 4
														signed: false
														type: dec
														End SyntaxNode dump */
														
														m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MSR, "MSR", 4, buf.start));
														
														// EncodingParameter begin
														/*ensureWeHave(4, buf);
														
														// TODO resolve slice definitions: []
														// Mask: 0b1111'1111
														UWord param_sysm = ((m_instrWord >> 0) & 0xff) >> 0;
														*/
														// EncodingParameter end
														// EncodingParameter begin
														/*ensureWeHave(2, buf);
														
														// TODO resolve slice definitions: []
														// Mask: 0b1111'0000'0000'0000'0000
														UWord param_n = ((m_instrWord >> 0) & 0xf0000) >> 16;
														*/
														// EncodingParameter end
													
														/* Debug string begin
														if ((LITERAL_sysm<CONSTNUM_0x7..CONSTNUM_0x3> == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
														if ((LITERAL_sysm<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
														(MEMREF_PSR<CONSTNUM_0x1f..CONSTNUM_0x1b>{ Type: Bit } = MEMREF_R[LITERAL_n]<CONSTNUM_0x1f..CONSTNUM_0x1b>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
														} else {
														}
														Debug string end */
														
														/*
														
														if ((((param_sysm & 0x0) >> 0x3) == 0x0))
														{
															if ((((param_sysm & 0x4) >> 0x2) == 0x0))
															{
																// write PSR
																m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PSR), false, true);
																// read R
																m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
															}
														}
														*/
													}
												}
												else
												{
												// Invalid instruction.
												m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
												}
											}
											else
											{
											// Invalid instruction.
											m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
											}
										}
										else
										{
										// Invalid instruction.
										m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
										}
									}
									else
									{
									// Invalid instruction.
									m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
									}
								}
								else
								{
								// Invalid instruction.
								m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
								}
							}
							else
							{
							// Invalid instruction.
							m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
							}
						}
						else
						{
						// Invalid instruction.
						m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
						}
					}
					else
					{
					// Invalid instruction.
					m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xfc00) == 0x4400)
	{
		// Mask:    0b1111'1100'0000'0000
		// Compare: 0b0100'0100'0000'0000
		printf("Mask:    0b1111'1100'0000'0000\n");
		printf("Compare: 0b0100'0100'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x300) == 0x300)
		{
			// Mask:    0b11'0000'0000
			// Compare: 0b11'0000'0000
			printf("Mask:    0b11'0000'0000\n");
			printf("Compare: 0b11'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x80) == 0x80)
			{
				// Mask:    0b1000'0000
				// Compare: 0b1000'0000
				printf("Mask:    0b1000'0000\n");
				printf("Compare: 0b1000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7) == 0x0)
				{
					// Mask:    0b111
					// Compare: 0b000
					printf("Mask:    0b111\n");
					printf("Compare: 0b000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: BLX
						Syn param:
						operand: m
						preString: R
						bitWidth: 4
						signed: false
						type: dec
						// mnemonic: BLX.N
						Syn param:
						operand: m
						preString: R
						bitWidth: 4
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BLX, "BLX", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111'1000
						UWord param_m = ((m_instrWord >> 16) & 0x78) >> 3;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL_next_instr = (MEMREF_PC{ Type: Unsigned32Bit } - CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_LR{ Type: Unsigned32Bit } = (LOCAL_LITERAL_next_instr | CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_PC{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local_next_instr;
						
						// read PC
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
						// write LR
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::LR), false, true);
						// write PC
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), true, true, false, false);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x80) == 0x0)
			{
				// Mask:    0b1000'0000
				// Compare: 0b0000'0000
				printf("Mask:    0b1000'0000\n");
				printf("Compare: 0b0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7) == 0x0)
				{
					// Mask:    0b111
					// Compare: 0b000
					printf("Mask:    0b111\n");
					printf("Compare: 0b000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: BX
						Syn param:
						operand: m
						preString: R
						bitWidth: 4
						signed: false
						type: dec
						// mnemonic: BX.N
						Syn param:
						operand: m
						preString: R
						bitWidth: 4
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BX, "BX", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111'1000
						UWord param_m = ((m_instrWord >> 16) & 0x78) >> 3;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(MEMREF_PC{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }
						Debug string end */
						
						/*
						
						// write PC
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x300) == 0x0)
		{
			// Mask:    0b11'0000'0000
			// Compare: 0b00'0000'0000
			printf("Mask:    0b11'0000'0000\n");
			printf("Compare: 0b00'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 4
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 4
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADD, "ADD", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: [3]
				// Mask: 0b1000'0000
				UWord param_dn = ((m_instrWord >> 16) & 0x80) >> 7;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1000
				UWord param_m = ((m_instrWord >> 16) & 0x78) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: [2, 0]
				// Mask: 0b111
				struct { Word field : 3; } signExtender_dn;
				Word param_dn = signExtender_dn.field = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				if (((LITERAL_dn == CONSTNUM_0xf){ Type: Unsigned32Bit, MemRef: no } && (LITERAL_m == CONSTNUM_0xf){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				} else {
				(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				if (((param_dn == 0xf) && (param_m == 0xf)))
				{
				} else {
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					local__i_add_c = 0x0;
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
					// Maybe PC write:
					if (param_dn == 15)
					{
						m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
					}
					// write N
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
					// write Z
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
					// write C
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
					// write V
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x300) == 0x100)
		{
			// Mask:    0b11'0000'0000
			// Compare: 0b01'0000'0000
			printf("Mask:    0b11'0000'0000\n");
			printf("Compare: 0b01'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 4
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 4
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_CMP, "CMP", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: [3]
				// Mask: 0b1000'0000
				UWord param_n = ((m_instrWord >> 16) & 0x80) >> 7;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1000
				UWord param_m = ((m_instrWord >> 16) & 0x78) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: [2, 0]
				// Mask: 0b111
				struct { Word field : 3; } signExtender_n;
				Word param_n = signExtender_n.field = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				if ((((LITERAL_n < CONSTNUM_0x8){ Type: Unsigned32Bit, MemRef: no } && (LITERAL_m < CONSTNUM_0x8){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no } || ((LITERAL_n == CONSTNUM_0xf){ Type: Unsigned32Bit, MemRef: no } && (LITERAL_m == CONSTNUM_0xf){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				} else {
				(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				if ((((param_n < 0x8) && (param_m < 0x8)) || ((param_n == 0xf) && (param_m == 0xf))))
				{
				} else {
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					local__i_add_c = 0x1;
					// write N
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
					// write Z
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
					// write C
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
					// write V
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x300) == 0x200)
		{
			// Mask:    0b11'0000'0000
			// Compare: 0b10'0000'0000
			printf("Mask:    0b11'0000'0000\n");
			printf("Compare: 0b10'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: MOV
				Syn param:
				operand: d
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 4
				signed: false
				type: dec
				// mnemonic: MOV.N
				Syn param:
				operand: d
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 4
				signed: false
				type: dec
				// mnemonic: CPY
				Syn param:
				operand: d
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 4
				signed: false
				type: dec
				// mnemonic: CPY.N
				Syn param:
				operand: d
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 4
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOV, "MOV", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: [3]
				// Mask: 0b1000'0000
				UWord param_d = ((m_instrWord >> 16) & 0x80) >> 7;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1000
				UWord param_m = ((m_instrWord >> 16) & 0x78) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: [2, 0]
				// Mask: 0b111
				struct { Word field : 3; } signExtender_d;
				Word param_d = signExtender_d.field = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
				}
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe000) == 0x0)
	{
		// Mask:    0b1110'0000'0000'0000
		// Compare: 0b0000'0000'0000'0000
		printf("Mask:    0b1110'0000'0000'0000\n");
		printf("Compare: 0b0000'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x1000)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b1'0000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b1'0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: true
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ASRS, "ASRS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1100'0000
				struct { Word field : 5; } signExtender_imm;
				Word param_imm = signExtender_imm.field = ((m_instrWord >> 16) & 0x7c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				if ((LITERAL_imm == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_shift = CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL_shift = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }}
				(LOCAL_LITERAL_operand = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }>){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				int32_t local_operand;
				
				if ((param_imm == 0x0))
				{
					local_shift = 0x20;
				} else {
					local_shift = CoerceBits<uint32_t>(param_imm);
				}
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, local_shift)), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x0)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b0'0000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b0'0000'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7c0) == 0x0)
			{
				// Mask:    0b111'1100'0000
				// Compare: 0b000'0000'0000
				printf("Mask:    0b111'1100'0000\n");
				printf("Compare: 0b000'0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: 
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: 
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: 
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS, "MOVS", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0x1f..CONSTNUM_0x1f>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit } == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write N
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write Z
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					*/
				}
			}
			else
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LSLS, "LSLS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1100'0000
				UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand << LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: no }..(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: no }>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				local_shift = CoerceBits<uint32_t>(param_imm);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeLeftShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, local_shift)), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x800)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b0'1000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b0'1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LSRS, "LSRS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1100'0000
				UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				if ((LITERAL_imm == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_shift = CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL_shift = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }}
				(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				if ((param_imm == 0x0))
				{
					local_shift = 0x20;
				} else {
					local_shift = CoerceBits<uint32_t>(param_imm);
				}
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, local_shift)), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x1800)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b1'1000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b1'1000'0000'0000\n");
			
			if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x400) == 0x400)
			{
				// Mask:    0b100'0000'0000
				// Compare: 0b100'0000'0000
				printf("Mask:    0b100'0000'0000\n");
				printf("Compare: 0b100'0000'0000\n");
				
				if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x200) == 0x0)
				{
					// Mask:    0b10'0000'0000
					// Compare: 0b00'0000'0000
					printf("Mask:    0b10'0000'0000\n");
					printf("Compare: 0b00'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: d
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: imm
						preString: ,#
						bitWidth: 3
						signed: false
						type: dec
						// mnemonic: 
						Syn param:
						operand: d
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: imm
						preString: ,#
						bitWidth: 3
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADDS, "ADDS", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b1'1100'0000
						UWord param_imm = ((m_instrWord >> 16) & 0x1c0) >> 6;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local__i_add_c;
						int32_t local___signed_sum;
						uint32_t local___unsigned_sum;
						uint32_t local_imm32;
						uint32_t local__o_add_c;
						uint32_t local__i_add_x;
						uint32_t local__i_add_y;
						uint32_t local__o_add_result;
						int32_t local__o_add_v;
						
						local_imm32 = CoerceBits<uint32_t>(param_imm);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						local__i_add_y = local_imm32;
						local__i_add_c = 0x0;
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
						// Maybe PC write:
						if (param_d == 15)
						{
							m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
						}
						// write N
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
						// write Z
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
						// write C
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
						// write V
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
						*/
					}
				}
				else
				if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x200) == 0x200)
				{
					// Mask:    0b10'0000'0000
					// Compare: 0b10'0000'0000
					printf("Mask:    0b10'0000'0000\n");
					printf("Compare: 0b10'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: d
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: imm
						preString: ,#
						bitWidth: 3
						signed: false
						type: dec
						// mnemonic: 
						Syn param:
						operand: d
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: imm
						preString: ,#
						bitWidth: 3
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SUBS, "SUBS", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b1'1100'0000
						UWord param_imm = ((m_instrWord >> 16) & 0x1c0) >> 6;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local__i_add_c;
						int32_t local___signed_sum;
						uint32_t local___unsigned_sum;
						uint32_t local_imm32;
						uint32_t local__o_add_c;
						uint32_t local__i_add_x;
						uint32_t local__i_add_y;
						uint32_t local__o_add_result;
						int32_t local__o_add_v;
						
						local_imm32 = CoerceBits<uint32_t>(param_imm);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						local__i_add_y = ~local_imm32;
						local__i_add_c = 0x1;
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
						// Maybe PC write:
						if (param_d == 15)
						{
							m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
						}
						// write N
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
						// write Z
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
						// write C
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
						// write V
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x400) == 0x0)
			{
				// Mask:    0b100'0000'0000
				// Compare: 0b000'0000'0000
				printf("Mask:    0b100'0000'0000\n");
				printf("Compare: 0b000'0000'0000\n");
				
				if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x200) == 0x0)
				{
					// Mask:    0b10'0000'0000
					// Compare: 0b00'0000'0000
					printf("Mask:    0b10'0000'0000\n");
					printf("Compare: 0b00'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: d
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: m
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						// mnemonic: 
						Syn param:
						operand: d
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: m
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADDS2, "ADDS", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b1'1100'0000
						UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local__i_add_c;
						int32_t local___signed_sum;
						uint32_t local___unsigned_sum;
						uint32_t local__o_add_c;
						uint32_t local__i_add_x;
						uint32_t local__i_add_y;
						uint32_t local__o_add_result;
						int32_t local__o_add_v;
						
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
						local__i_add_c = 0x0;
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
						// Maybe PC write:
						if (param_d == 15)
						{
							m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
						}
						// write N
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
						// write Z
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
						// write C
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
						// write V
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
						*/
					}
				}
				else
				if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x200) == 0x200)
				{
					// Mask:    0b10'0000'0000
					// Compare: 0b10'0000'0000
					printf("Mask:    0b10'0000'0000\n");
					printf("Compare: 0b10'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: d
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: m
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						// mnemonic: 
						Syn param:
						operand: d
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: m
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SUBS2, "SUBS", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b1'1100'0000
						UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local__i_add_c;
						int32_t local___signed_sum;
						uint32_t local___unsigned_sum;
						uint32_t local__o_add_c;
						uint32_t local__i_add_x;
						uint32_t local__i_add_y;
						uint32_t local__o_add_result;
						int32_t local__o_add_v;
						
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
						local__i_add_c = 0x1;
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
						// Maybe PC write:
						if (param_d == 15)
						{
							m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
						}
						// write N
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
						// write Z
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
						// write C
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
						// write V
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
						*/
					}
				}
				else
				if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x200) == 0x200)
				{
					// Mask:    0b10'0000'0000
					// Compare: 0b10'0000'0000
					printf("Mask:    0b10'0000'0000\n");
					printf("Compare: 0b10'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: dn
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: m
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						// mnemonic: 
						Syn param:
						operand: dn
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: m
						preString: ,R
						bitWidth: 3
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SUBS3, "SUBS", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b1'1100'0000
						UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_dn = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL_d = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL_n = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local__i_add_c;
						int32_t local___signed_sum;
						uint32_t local___unsigned_sum;
						uint32_t local__o_add_c;
						uint8_t local_d;
						uint32_t local__i_add_x;
						uint32_t local__i_add_y;
						uint32_t local__o_add_result;
						int32_t local__o_add_v;
						uint8_t local_n;
						
						local_d = param_dn;
						local_n = param_dn;
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
						local__i_add_c = 0x1;
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
						// Maybe PC write:
						if (param_d == 15)
						{
							m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
						}
						// write N
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
						// write Z
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
						// write C
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
						// write V
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x1000)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b1'0000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b1'0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,ASR#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,ASR#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,ASR#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,ASR#
				bitWidth: 5
				signed: true
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS2, "MOVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1100'0000
				struct { Word field : 5; } signExtender_imm;
				Word param_imm = signExtender_imm.field = ((m_instrWord >> 16) & 0x7c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				if ((LITERAL_imm == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_shift = CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL_shift = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }}
				(LOCAL_LITERAL_operand = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }>){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				int32_t local_operand;
				
				if ((param_imm == 0x0))
				{
					local_shift = 0x20;
				} else {
					local_shift = CoerceBits<uint32_t>(param_imm);
				}
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, local_shift)), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x0)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b0'0000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b0'0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,LSL#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,LSL#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,LSL#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,LSL#
				bitWidth: 5
				signed: true
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS3, "MOVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1100'0000
				UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand << LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: no }..(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: no }>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				local_shift = CoerceBits<uint32_t>(param_imm);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeLeftShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, local_shift)), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x800)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b0'1000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b0'1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,LSR#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,LSR#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,LSR#
				bitWidth: 5
				signed: true
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,LSR#
				bitWidth: 5
				signed: true
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS4, "MOVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1100'0000
				UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				if ((LITERAL_imm == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_shift = CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL_shift = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }}
				(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				if ((param_imm == 0x0))
				{
					local_shift = 0x20;
				} else {
					local_shift = CoerceBits<uint32_t>(param_imm);
				}
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, local_shift)), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xfc00) == 0x4000)
	{
		// Mask:    0b1111'1100'0000'0000
		// Compare: 0b0100'0000'0000'0000
		printf("Mask:    0b1111'1100'0000'0000\n");
		printf("Compare: 0b0100'0000'0000'0000\n");
		
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x100)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'0000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ASRS2, "ASRS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift < CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Signed32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				int32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x100)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'0000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ASRS3, "ASRS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift < CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Signed32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				int32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x80)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'1000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'1000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LSLS2, "LSLS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand << LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift <= CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }..(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Unsigned32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeLeftShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x80)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'1000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'1000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LSLS3, "LSLS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand << LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift <= CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }..(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Unsigned32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeLeftShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0xc0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'1100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'1100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LSRS2, "LSRS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift < CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Unsigned32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0xc0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'1100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'1100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LSRS3, "LSRS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift < CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Unsigned32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x1c0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'1100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'1100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_RORS, "RORS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } >>> CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightRotateExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x1c0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'1100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'1100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_RORS2, "RORS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } >>> CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightRotateExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x140)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADCS, "ADCS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = MEMREF_C{ Type: Bit }){ Type: Bit, MemRef: yes }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				bool local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), makeRegisterExpression(ARMv6M::C), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x140)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADCS2, "ADCS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = MEMREF_C{ Type: Bit }){ Type: Bit, MemRef: yes }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				bool local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), makeRegisterExpression(ARMv6M::C), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'0000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ANDS, "ANDS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } & MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAndExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'0000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ANDS2, "ANDS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } & MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAndExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x380)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b11'1000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b11'1000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BICS, "BICS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } & (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAndExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x380)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b11'1000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b11'1000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BICS2, "BICS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } & (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAndExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x2c0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b10'1100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b10'1100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: CMN
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: CMN.N
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_CMN, "CMN", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_n = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				local__i_add_c = 0x0;
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x280)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b10'1000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b10'1000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_CMP2, "CMP", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_n = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				local__i_add_c = 0x1;
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x40)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_EORS, "EORS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } ^ MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x40)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_EORS2, "EORS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } ^ MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x100)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'0000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,ASRR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,ASRR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,ASRR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,ASRR
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS5, "MOVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift < CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Signed32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				int32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x80)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'1000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'1000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,LSLR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,LSLR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,LSLR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,LSLR
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS6, "MOVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand << LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift <= CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }..(CONSTNUM_0x20 - LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Unsigned32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeLeftShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0xc0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b00'1100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b00'1100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,LSRR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,LSRR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,LSRR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,LSRR
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS7, "MOVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_shift = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_operand = CAST(STRICT, 32, MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_operand >> LOCAL_LITERAL_shift){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LOCAL_LITERAL_shift < CONSTNUM_0x20){ Type: Unsigned32Bit, MemRef: yes }) {
				(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_operand<(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }..(LOCAL_LITERAL_shift - CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }>){ Type: Unsigned32Bit, MemRef: yes }} else {
				(MEMREF_C{ Type: Bit } = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local_result;
				uint32_t local_shift;
				uint32_t local_operand;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightArithmeticShiftExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x1c0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'1100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'1100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,RORR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,RORR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,RORR
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,RORR
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS8, "MOVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } >>> CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeRightRotateExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x3c0)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b11'1100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b11'1100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: MVNS
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: MVNS.N
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MVNS, "MVNS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x300)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b11'0000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b11'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ORRS, "ORRS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } | MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeOrExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x300)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b11'0000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b11'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ORRS2, "ORRS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } | MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeOrExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeRegisterExpression(ARMv6M::R0 + param_m), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x240)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b10'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b10'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ,#0
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ,#0
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_RSBS, "RSBS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = (~MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local__i_add_y = CoerceBits<uint32_t>(0x0);
				local__i_add_c = 0x1;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x240)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b10'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b10'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ,#0
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ,#0
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_RSBS2, "RSBS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_dn = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_d = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL_n = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = (~MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint8_t local_d;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				uint8_t local_n;
				
				local_d = param_dn;
				local_n = param_dn;
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local__i_add_y = CoerceBits<uint32_t>(0x0);
				local__i_add_c = 0x1;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x180)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'1000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'1000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SBCS, "SBCS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = MEMREF_C{ Type: Bit }){ Type: Bit, MemRef: yes }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				bool local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), u32), makeRegisterExpression(ARMv6M::C), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x180)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b01'1000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b01'1000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SBCS2, "SBCS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = MEMREF_C{ Type: Bit }){ Type: Bit, MemRef: yes }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				bool local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_m), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), u32), makeRegisterExpression(ARMv6M::C), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x200)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b10'0000'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b10'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: TST
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: TST.N
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_TST, "TST", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_n = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_result = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } & MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_result;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x240)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b10'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b10'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_NEG, "NEG", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_dn = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dn = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_d = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL_n = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = (~MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint8_t local_d;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				uint8_t local_n;
				
				local_d = param_dn;
				local_n = param_dn;
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local__i_add_y = CoerceBits<uint32_t>(0x0);
				local__i_add_c = 0x1;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x240)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b10'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b10'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_NEG2, "NEG", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = (~MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local__i_add_y = CoerceBits<uint32_t>(0x0);
				local__i_add_c = 0x1;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeBitwiseXorExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, ~static_cast<uint32_t>(0))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x340)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b11'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b11'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dm
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dm
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MULS, "MULS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dm = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_op1 = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_op2 = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_dm]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_op1 * LOCAL_LITERAL_op2){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dm]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Signed32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Signed32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				int32_t local_op2;
				int32_t local_result;
				int32_t local_op1;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dm), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dm), false, true);
				// Maybe PC write:
				if (param_dm == 15)
				{
					m_instrInProgress->addSuccessor(makeMultiplyExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_dm), s32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x3c0) == 0x340)
		{
			// Mask:    0b11'1100'0000
			// Compare: 0b11'0100'0000
			printf("Mask:    0b11'1100'0000\n");
			printf("Compare: 0b11'0100'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dm
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dm
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dm
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dm
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MULS2, "MULS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_dm = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_op1 = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_op2 = CAST(STRICT, SIGNED, 32, MEMREF_R[LITERAL_dm]{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL_result = (LOCAL_LITERAL_op1 * LOCAL_LITERAL_op2){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dm]{ Type: Unsigned32Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Signed32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Signed32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				int32_t local_op2;
				int32_t local_result;
				int32_t local_op1;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dm), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dm), false, true);
				// Maybe PC write:
				if (param_dm == 15)
				{
					m_instrInProgress->addSuccessor(makeMultiplyExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_dm), s32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe000) == 0x2000)
	{
		// Mask:    0b1110'0000'0000'0000
		// Compare: 0b0010'0000'0000'0000
		printf("Mask:    0b1110'0000'0000'0000\n");
		printf("Compare: 0b0010'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x1000)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b1'0000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b1'0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADDS3, "ADDS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_dn = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_n = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL_d = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local_imm32;
				uint32_t local__o_add_c;
				uint8_t local_d;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				uint8_t local_n;
				
				local_n = param_dn;
				local_d = param_dn;
				local_imm32 = CoerceBits<uint32_t>(param_imm);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local__i_add_y = local_imm32;
				local__i_add_c = 0x0;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x1000)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b1'0000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b1'0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADDS4, "ADDS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_dn = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_n = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL_d = LITERAL_dn){ Type: Unsigned8Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local_imm32;
				uint32_t local__o_add_c;
				uint8_t local_d;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				uint8_t local_n;
				
				local_n = param_dn;
				local_d = param_dn;
				local_imm32 = CoerceBits<uint32_t>(param_imm);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local__i_add_y = local_imm32;
				local__i_add_c = 0x0;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x800)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b0'1000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b0'1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_CMP3, "CMP", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_n = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local_imm32;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				local_imm32 = CoerceBits<uint32_t>(param_imm);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local__i_add_y = ~local_imm32;
				local__i_add_c = 0x1;
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x0)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b0'0000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b0'0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_MOVS9, "MOVS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_d = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL_imm32<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL_imm32 == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_imm32;
				
				local_imm32 = CoerceBits<uint32_t>(param_imm);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(Immediate::makeImmediate(Result(u32, local_imm32)), false, false, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x1800)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b1'1000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b1'1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SUBS4, "SUBS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_dn = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local_imm32;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				local_imm32 = CoerceBits<uint32_t>(param_imm);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				local__i_add_y = ~local_imm32;
				local__i_add_c = 0x1;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1800) == 0x1800)
		{
			// Mask:    0b1'1000'0000'0000
			// Compare: 0b1'1000'0000'0000
			printf("Mask:    0b1'1000'0000'0000\n");
			printf("Compare: 0b1'1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dn
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: dn
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SUBS5, "SUBS", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_dn = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dn]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_N{ Type: Bit } = LOCAL_LITERAL__o_add_result<CONSTNUM_0x1f..CONSTNUM_0x1f>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_Z{ Type: Bit } = (LOCAL_LITERAL__o_add_result == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_C{ Type: Bit } = LOCAL_LITERAL__o_add_c){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_V{ Type: Bit } = LOCAL_LITERAL__o_add_v){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local_imm32;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				local_imm32 = CoerceBits<uint32_t>(param_imm);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), true, false);
				local__i_add_y = ~local_imm32;
				local__i_add_c = 0x1;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dn), false, true);
				// Maybe PC write:
				if (param_dn == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_dn), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				// write N
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::N), false, true);
				// write Z
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::Z), false, true);
				// write C
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::C), false, true);
				// write V
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::V), false, true);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf000) == 0xa000)
	{
		// Mask:    0b1111'0000'0000'0000
		// Compare: 0b1010'0000'0000'0000
		printf("Mask:    0b1111'0000'0000'0000\n");
		printf("Compare: 0b1010'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x800)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b1000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,SP,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,SP,#
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADD2, "ADD", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_d = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local_imm32;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
				// read SP
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
				local__i_add_y = local_imm32;
				local__i_add_c = 0x0;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x0)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: ADR
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,
				bitWidth: 8
				signed: false
				type: str
				// mnemonic: ADR.N
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,
				bitWidth: 8
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADR, "ADR", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_d = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = ((MEMREF_PC{ Type: Unsigned32Bit } & CONSTNUM_0xfffffffc){ Type: Unsigned32Bit, MemRef: yes } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_imm32;
				
				local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), false, false, false, false);
				}
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x0)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,PC,#
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: d
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,PC,#
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADD3, "ADD", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_d = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = ((MEMREF_PC{ Type: Unsigned32Bit } & CONSTNUM_0xfffffffc){ Type: Unsigned32Bit, MemRef: yes } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_imm32;
				
				local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
				// Maybe PC write:
				if (param_d == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), false, false, false, false);
				}
				// read PC
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf000) == 0xb000)
	{
		// Mask:    0b1111'0000'0000'0000
		// Compare: 0b1011'0000'0000'0000
		printf("Mask:    0b1111'0000'0000'0000\n");
		printf("Compare: 0b1011'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x0)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x80) == 0x0)
			{
				// Mask:    0b1000'0000
				// Compare: 0b0000'0000
				printf("Mask:    0b1000'0000\n");
				printf("Compare: 0b0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: imm
					preString: SP,SP,#
					bitWidth: 7
					signed: false
					type: dec
					// mnemonic: 
					Syn param:
					operand: imm
					preString: SP,SP,#
					bitWidth: 7
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADD4, "ADD", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1111
					UWord param_imm = ((m_instrWord >> 16) & 0x7f) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_d = CONSTNUM_0xd){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local__i_add_c;
					int32_t local___signed_sum;
					uint32_t local___unsigned_sum;
					uint32_t local_imm32;
					uint32_t local__o_add_c;
					uint32_t local_d;
					uint32_t local__i_add_x;
					uint32_t local__i_add_y;
					uint32_t local__o_add_result;
					int32_t local__o_add_v;
					
					local_d = 0xd;
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					local__i_add_y = local_imm32;
					local__i_add_c = 0x0;
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
					}
					*/
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x80) == 0x0)
			{
				// Mask:    0b1000'0000
				// Compare: 0b0000'0000
				printf("Mask:    0b1000'0000\n");
				printf("Compare: 0b0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: imm
					preString: SP,#
					bitWidth: 7
					signed: false
					type: dec
					// mnemonic: 
					Syn param:
					operand: imm
					preString: SP,#
					bitWidth: 7
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADD5, "ADD", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1111
					UWord param_imm = ((m_instrWord >> 16) & 0x7f) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_d = CONSTNUM_0xd){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local__i_add_c;
					int32_t local___signed_sum;
					uint32_t local___unsigned_sum;
					uint32_t local_imm32;
					uint32_t local__o_add_c;
					uint32_t local_d;
					uint32_t local__i_add_x;
					uint32_t local__i_add_y;
					uint32_t local__o_add_result;
					int32_t local__o_add_v;
					
					local_d = 0xd;
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					local__i_add_y = local_imm32;
					local__i_add_c = 0x0;
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_y))), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
					}
					*/
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x80) == 0x80)
			{
				// Mask:    0b1000'0000
				// Compare: 0b1000'0000
				printf("Mask:    0b1000'0000\n");
				printf("Compare: 0b1000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: imm
					preString: SP,SP,#
					bitWidth: 7
					signed: false
					type: dec
					// mnemonic: 
					Syn param:
					operand: imm
					preString: SP,SP,#
					bitWidth: 7
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SUB, "SUB", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1111
					UWord param_imm = ((m_instrWord >> 16) & 0x7f) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_SP{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local__i_add_c;
					int32_t local___signed_sum;
					uint32_t local___unsigned_sum;
					uint32_t local_imm32;
					uint32_t local__o_add_c;
					uint32_t local__i_add_x;
					uint32_t local__i_add_y;
					uint32_t local__o_add_result;
					int32_t local__o_add_v;
					
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					local__i_add_y = ~local_imm32;
					local__i_add_c = 0x1;
					// write SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), false, true);
					*/
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x80) == 0x80)
			{
				// Mask:    0b1000'0000
				// Compare: 0b1000'0000
				printf("Mask:    0b1000'0000\n");
				printf("Compare: 0b1000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: imm
					preString: SP,#
					bitWidth: 7
					signed: false
					type: dec
					// mnemonic: 
					Syn param:
					operand: imm
					preString: SP,#
					bitWidth: 7
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SUB2, "SUB", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1111
					UWord param_imm = ((m_instrWord >> 16) & 0x7f) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_x = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = (~LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_SP{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local__i_add_c;
					int32_t local___signed_sum;
					uint32_t local___unsigned_sum;
					uint32_t local_imm32;
					uint32_t local__o_add_c;
					uint32_t local__i_add_x;
					uint32_t local__i_add_y;
					uint32_t local__o_add_result;
					int32_t local__o_add_v;
					
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					local__i_add_y = ~local_imm32;
					local__i_add_c = 0x1;
					// write SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), false, true);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xa00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1010'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1010'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xc0) == 0x0)
			{
				// Mask:    0b1100'0000
				// Compare: 0b0000'0000
				printf("Mask:    0b1100'0000\n");
				printf("Compare: 0b0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: REV
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: REV.N
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_REV, "REV", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(MEMREF_R[LITERAL_d]<CONSTNUM_0x1f..CONSTNUM_0x18>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]<CONSTNUM_0x17..CONSTNUM_0x10>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0xf..CONSTNUM_0x8>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]<CONSTNUM_0xf..CONSTNUM_0x8>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0x17..CONSTNUM_0x10>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0x1f..CONSTNUM_0x18>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					*/
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xc0) == 0x40)
			{
				// Mask:    0b1100'0000
				// Compare: 0b0100'0000
				printf("Mask:    0b1100'0000\n");
				printf("Compare: 0b0100'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: REV16
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: REV16.N
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_REV16, "REV16", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(MEMREF_R[LITERAL_d]<CONSTNUM_0x1f..CONSTNUM_0x18>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0x17..CONSTNUM_0x10>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]<CONSTNUM_0x17..CONSTNUM_0x10>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0x1f..CONSTNUM_0x18>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]<CONSTNUM_0xf..CONSTNUM_0x8>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_R[LITERAL_d]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0xf..CONSTNUM_0x8>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					*/
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xc0) == 0xc0)
			{
				// Mask:    0b1100'0000
				// Compare: 0b1100'0000
				printf("Mask:    0b1100'0000\n");
				printf("Compare: 0b1100'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: REVSH
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: REVSH.N
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_REVSH, "REVSH", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(MEMREF_R[LITERAL_d]<CONSTNUM_0x1f..CONSTNUM_0x8>{ Type: Unsigned32Bit } = CAST(SIGNED, 24, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unknown, MemRef: yes }(MEMREF_R[LITERAL_d]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit } = MEMREF_R[LITERAL_m]<CONSTNUM_0xf..CONSTNUM_0x8>{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x200)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0010'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0010'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xc0) == 0x40)
			{
				// Mask:    0b1100'0000
				// Compare: 0b0100'0000
				printf("Mask:    0b1100'0000\n");
				printf("Compare: 0b0100'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: SXTB
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: SXTB.N
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SXTB, "SXTB", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = CAST(SIGNED, 32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }
					Debug string end */
					
					/*
					
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					*/
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xc0) == 0x0)
			{
				// Mask:    0b1100'0000
				// Compare: 0b0000'0000
				printf("Mask:    0b1100'0000\n");
				printf("Compare: 0b0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: SXTH
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: SXTH.N
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SXTH, "SXTH", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = CAST(SIGNED, 32, MEMREF_R[LITERAL_m]<CONSTNUM_0xf..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Signed32Bit, MemRef: yes }
					Debug string end */
					
					/*
					
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					*/
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xc0) == 0xc0)
			{
				// Mask:    0b1100'0000
				// Compare: 0b1100'0000
				printf("Mask:    0b1100'0000\n");
				printf("Compare: 0b1100'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: UXTB
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: UXTB.N
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_UXTB, "UXTB", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0x7..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					*/
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xc0) == 0x80)
			{
				// Mask:    0b1100'0000
				// Compare: 0b1000'0000
				printf("Mask:    0b1100'0000\n");
				printf("Compare: 0b1000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: UXTH
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					// mnemonic: UXTH.N
					Syn param:
					operand: d
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: m
					preString: ,R
					bitWidth: 3
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_UXTH, "UXTH", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_m = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_d = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(MEMREF_R[LITERAL_d]{ Type: Unsigned32Bit } = CAST(32, MEMREF_R[LITERAL_m]<CONSTNUM_0xf..CONSTNUM_0x0>{ Type: Unsigned32Bit })){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_d), false, true);
					// Maybe PC write:
					if (param_d == 15)
					{
						m_instrInProgress->addSuccessor(makeRegisterExpression(ARMv6M::R0 + param_m), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x800)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b1000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b1000'0000'0000\n");
			
			if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x600) == 0x400)
			{
				// Mask:    0b110'0000'0000
				// Compare: 0b100'0000'0000
				printf("Mask:    0b110'0000'0000\n");
				printf("Compare: 0b100'0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: POP
					Syn param:
					operand: list
					preString: {
					bitWidth: 9
					signed: false
					type: str
					Syn param:
					operand: 
					preString: }
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: POP.N
					Syn param:
					operand: list
					preString: {
					bitWidth: 9
					signed: false
					type: str
					Syn param:
					operand: 
					preString: }
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_POP, "POP", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b1'1111'1111
					UWord param_list = ((m_instrWord >> 16) & 0x1ff) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL__i_address = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }if ((LITERAL_list<CONSTNUM_0x0..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x0]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x1..CONSTNUM_0x1> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x1]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x2]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x3..CONSTNUM_0x3> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x3]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x4..CONSTNUM_0x4> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x4]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x5..CONSTNUM_0x5> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x5]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x6..CONSTNUM_0x6> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x6]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x7..CONSTNUM_0x7> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x7]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x8..CONSTNUM_0x8> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_PC{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					(MEMREF_SP{ Type: Unsigned32Bit } = (MEMREF_SP{ Type: Unsigned32Bit } + (CONSTNUM_0x4 * LOCAL_LITERAL_bit_count){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local__i_address;
					uint32_t local_bit_count;
					uint8_t local__o_data;
					
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					local_bit_count = 0x0;
					if ((((param_list & 0x1) >> 0x0) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x0), false, true);
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x2) >> 0x1) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x1), false, true);
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x4) >> 0x2) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x2), false, true);
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x8) >> 0x3) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x3), false, true);
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x10) >> 0x4) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x4), false, true);
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x20) >> 0x5) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x5), false, true);
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x40) >> 0x6) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x6), false, true);
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x80) >> 0x7) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x7), false, true);
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x100) >> 0x8) == 0x1))
					{
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write PC
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), false, true);
						m_instrInProgress->addSuccessor(makeDereferenceExpression(makeRegisterExpression(ARMv6M::SP), u8), false, true, false, false);
						local_bit_count = (local_bit_count + 0x1);
					}
					// write SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), false, true);
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x0)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x600) == 0x400)
			{
				// Mask:    0b110'0000'0000
				// Compare: 0b100'0000'0000
				printf("Mask:    0b110'0000'0000\n");
				printf("Compare: 0b100'0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: PUSH
					Syn param:
					operand: list
					preString: {
					bitWidth: 9
					signed: false
					type: str
					Syn param:
					operand: 
					preString: }
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: PUSH.N
					Syn param:
					operand: list
					preString: {
					bitWidth: 9
					signed: false
					type: str
					Syn param:
					operand: 
					preString: }
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_PUSH, "PUSH", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b1'1111'1111
					UWord param_list = ((m_instrWord >> 16) & 0x1ff) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_bit_count = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }if ((LITERAL_list<CONSTNUM_0x0..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x1..CONSTNUM_0x1> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x3..CONSTNUM_0x3> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x4..CONSTNUM_0x4> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x5..CONSTNUM_0x5> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x6..CONSTNUM_0x6> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					if ((LITERAL_list<CONSTNUM_0x7..CONSTNUM_0x7> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
					(LOCAL_LITERAL__i_address = (MEMREF_SP{ Type: Unsigned32Bit } - (CONSTNUM_0x4 * LOCAL_LITERAL_bit_count){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }if ((LITERAL_list<CONSTNUM_0x0..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x0]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					if ((LITERAL_list<CONSTNUM_0x1..CONSTNUM_0x1> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x1]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					if ((LITERAL_list<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x2]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					if ((LITERAL_list<CONSTNUM_0x3..CONSTNUM_0x3> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x3]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					if ((LITERAL_list<CONSTNUM_0x4..CONSTNUM_0x4> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x4]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					if ((LITERAL_list<CONSTNUM_0x5..CONSTNUM_0x5> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x5]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					if ((LITERAL_list<CONSTNUM_0x6..CONSTNUM_0x6> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x6]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					if ((LITERAL_list<CONSTNUM_0x7..CONSTNUM_0x7> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x7]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					if ((LITERAL_list<CONSTNUM_0x8..CONSTNUM_0x8> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
					(LOCAL_LITERAL__i_data = MEMREF_LR{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
					(MEMREF_SP{ Type: Unsigned32Bit } = (MEMREF_SP{ Type: Unsigned32Bit } - (CONSTNUM_0x4 * LOCAL_LITERAL_bit_count){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local__i_address;
					uint32_t local_bit_count;
					uint32_t local__i_data;
					
					local_bit_count = 0x0;
					if ((((param_list & 0x1) >> 0x0) == 0x1))
					{
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x2) >> 0x1) == 0x1))
					{
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x4) >> 0x2) == 0x1))
					{
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x8) >> 0x3) == 0x1))
					{
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x10) >> 0x4) == 0x1))
					{
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x20) >> 0x5) == 0x1))
					{
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x40) >> 0x6) == 0x1))
					{
						local_bit_count = (local_bit_count + 0x1);
					}
					if ((((param_list & 0x80) >> 0x7) == 0x1))
					{
						local_bit_count = (local_bit_count + 0x1);
					}
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					if ((((param_list & 0x1) >> 0x0) == 0x1))
					{
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x0), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					if ((((param_list & 0x2) >> 0x1) == 0x1))
					{
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x1), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					if ((((param_list & 0x4) >> 0x2) == 0x1))
					{
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x2), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					if ((((param_list & 0x8) >> 0x3) == 0x1))
					{
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x3), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					if ((((param_list & 0x10) >> 0x4) == 0x1))
					{
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x4), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					if ((((param_list & 0x20) >> 0x5) == 0x1))
					{
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x5), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					if ((((param_list & 0x40) >> 0x6) == 0x1))
					{
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x6), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					if ((((param_list & 0x80) >> 0x7) == 0x1))
					{
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x7), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					if ((((param_list & 0x100) >> 0x8) == 0x1))
					{
						// read LR
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::LR), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeSubExpression(makeRegisterExpression(ARMv6M::SP), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					}
					// write SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), false, true);
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0x600)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b0110'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b0110'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xe0) == 0x60)
			{
				// Mask:    0b1110'0000
				// Compare: 0b0110'0000
				printf("Mask:    0b1110'0000\n");
				printf("Compare: 0b0110'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x10) == 0x0)
				{
					// Mask:    0b1'0000
					// Compare: 0b0'0000
					printf("Mask:    0b1'0000\n");
					printf("Compare: 0b0'0000\n");
					
					if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf) == 0x2)
					{
						// Mask:    0b1111
						// Compare: 0b0010
						printf("Mask:    0b1111\n");
						printf("Compare: 0b0010\n");
						
						{
							/* SyntaxNode dump:
							// mnemonic: CPSIE
							Syn param:
							operand: 
							preString: i
							bitWidth: 0
							signed: false
							type: str
							// mnemonic: CPSIE.N
							Syn param:
							operand: 
							preString: i
							bitWidth: 0
							signed: false
							type: str
							End SyntaxNode dump */
							
							m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_CPSIE, "CPSIE", 2, buf.start));
							
						
							/* Debug string begin
							Debug string end */
							
							/*
							
							*/
						}
					}
					else
					{
					// Invalid instruction.
					m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
					}
				}
				else
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x10) == 0x10)
				{
					// Mask:    0b1'0000
					// Compare: 0b1'0000
					printf("Mask:    0b1'0000\n");
					printf("Compare: 0b1'0000\n");
					
					if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf) == 0x2)
					{
						// Mask:    0b1111
						// Compare: 0b0010
						printf("Mask:    0b1111\n");
						printf("Compare: 0b0010\n");
						
						{
							/* SyntaxNode dump:
							// mnemonic: CPSID
							Syn param:
							operand: 
							preString: i
							bitWidth: 0
							signed: false
							type: str
							// mnemonic: CPSID.N
							Syn param:
							operand: 
							preString: i
							bitWidth: 0
							signed: false
							type: str
							End SyntaxNode dump */
							
							m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_CPSID, "CPSID", 2, buf.start));
							
						
							/* Debug string begin
							Debug string end */
							
							/*
							
							*/
						}
					}
					else
					{
					// Invalid instruction.
					m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xf00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1111'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1111'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf0) == 0x0)
			{
				// Mask:    0b1111'0000
				// Compare: 0b0000'0000
				printf("Mask:    0b1111'0000\n");
				printf("Compare: 0b0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf) == 0x0)
				{
					// Mask:    0b1111
					// Compare: 0b0000
					printf("Mask:    0b1111\n");
					printf("Compare: 0b0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: NOP
						// mnemonic: NOP.N
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_NOP, "NOP", 2, buf.start));
						
					
						/* Debug string begin
						Debug string end */
						
						/*
						
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf0) == 0x10)
			{
				// Mask:    0b1111'0000
				// Compare: 0b0001'0000
				printf("Mask:    0b1111'0000\n");
				printf("Compare: 0b0001'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf) == 0x0)
				{
					// Mask:    0b1111
					// Compare: 0b0000
					printf("Mask:    0b1111\n");
					printf("Compare: 0b0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: YIELD
						// mnemonic: YIELD.N
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_YIELD, "YIELD", 2, buf.start));
						
					
						/* Debug string begin
						Debug string end */
						
						/*
						
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf0) == 0x40)
			{
				// Mask:    0b1111'0000
				// Compare: 0b0100'0000
				printf("Mask:    0b1111'0000\n");
				printf("Compare: 0b0100'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf) == 0x0)
				{
					// Mask:    0b1111
					// Compare: 0b0000
					printf("Mask:    0b1111\n");
					printf("Compare: 0b0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: SEV
						// mnemonic: SEV.N
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_SEV, "SEV", 2, buf.start));
						
					
						/* Debug string begin
						Debug string end */
						
						/*
						
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf0) == 0x20)
			{
				// Mask:    0b1111'0000
				// Compare: 0b0010'0000
				printf("Mask:    0b1111'0000\n");
				printf("Compare: 0b0010'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf) == 0x0)
				{
					// Mask:    0b1111
					// Compare: 0b0000
					printf("Mask:    0b1111\n");
					printf("Compare: 0b0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: WFE
						// mnemonic: WFE.N
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_WFE, "WFE", 2, buf.start));
						
					
						/* Debug string begin
						Debug string end */
						
						/*
						
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf0) == 0x30)
			{
				// Mask:    0b1111'0000
				// Compare: 0b0011'0000
				printf("Mask:    0b1111'0000\n");
				printf("Compare: 0b0011'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xf) == 0x0)
				{
					// Mask:    0b1111
					// Compare: 0b0000
					printf("Mask:    0b1111\n");
					printf("Compare: 0b0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: WFI
						// mnemonic: WFI.N
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_WFI, "WFI", 2, buf.start));
						
					
						/* Debug string begin
						Debug string end */
						
						/*
						
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xe00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1110'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1110'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: imm
				preString: #
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: imm
				preString: #
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BKPT, "BKPT", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				Debug string end */
				
				/*
				
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf00) == 0xe00)
		{
			// Mask:    0b1111'0000'0000
			// Compare: 0b1110'0000'0000
			printf("Mask:    0b1111'0000'0000\n");
			printf("Compare: 0b1110'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: imm
				preString: 
				bitWidth: 8
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: imm
				preString: 
				bitWidth: 8
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_BKPT2, "BKPT", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				Debug string end */
				
				/*
				
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xff00) == 0x4400)
	{
		// Mask:    0b1111'1111'0000'0000
		// Compare: 0b0100'0100'0000'0000
		printf("Mask:    0b1111'1111'0000'0000\n");
		printf("Compare: 0b0100'0100'0000'0000\n");
		
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x78) == 0x68)
		{
			// Mask:    0b111'1000
			// Compare: 0b110'1000
			printf("Mask:    0b111'1000\n");
			printf("Compare: 0b110'1000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: dm
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: dm
				preString: ,SP,R
				bitWidth: 4
				signed: false
				type: dec
				// mnemonic: 
				Syn param:
				operand: dm
				preString: R
				bitWidth: 4
				signed: false
				type: dec
				Syn param:
				operand: dm
				preString: ,SP,R
				bitWidth: 4
				signed: false
				type: dec
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADD6, "ADD", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: [2, 0]
				// Mask: 0b111
				struct { Word field : 3; } signExtender_dm;
				Word param_dm = signExtender_dm.field = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: [3]
				// Mask: 0b1000'0000
				UWord param_dm = ((m_instrWord >> 16) & 0x80) >> 7;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_add_x = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = MEMREF_R[LITERAL_dm]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[LITERAL_dm]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_add_c;
				int32_t local___signed_sum;
				uint32_t local___unsigned_sum;
				uint32_t local__o_add_c;
				uint32_t local__i_add_x;
				uint32_t local__i_add_y;
				uint32_t local__o_add_result;
				int32_t local__o_add_v;
				
				// read SP
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dm), true, false);
				local__i_add_c = 0x0;
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_dm), false, true);
				// Maybe PC write:
				if (param_dm == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::SP), makeRegisterExpression(ARMv6M::R0 + param_dm), u32), Immediate::makeImmediate(Result(u32, CastBits<uint32_t>(local__i_add_c))), u32), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x80) == 0x80)
		{
			// Mask:    0b1000'0000
			// Compare: 0b1000'0000
			printf("Mask:    0b1000'0000\n");
			printf("Compare: 0b1000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7) == 0x5)
			{
				// Mask:    0b111
				// Compare: 0b101
				printf("Mask:    0b111\n");
				printf("Compare: 0b101\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: m
					preString: SP,R
					bitWidth: 4
					signed: false
					type: dec
					// mnemonic: 
					Syn param:
					operand: m
					preString: SP,R
					bitWidth: 4
					signed: false
					type: dec
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_ADD7, "ADD", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1000
					UWord param_m = ((m_instrWord >> 16) & 0x78) >> 3;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL__i_add_x = MEMREF_SP{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_y = MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_add_c = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL___unsigned_sum = ((CAST(STRICT, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, 32, LOCAL_LITERAL__i_add_y)){ Type: Unsigned32Bit, MemRef: yes } + CAST(STRICT, 32, LOCAL_LITERAL__i_add_c)){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL___signed_sum = ((CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_x) + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_y)){ Type: Signed32Bit, MemRef: yes } + CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__i_add_c)){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_result = LOCAL_LITERAL___unsigned_sum<CONSTNUM_0x1f..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_c = (not(CAST(STRICT, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___unsigned_sum){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_add_v = (not(CAST(STRICT, SIGNED, 32, LOCAL_LITERAL__o_add_result) == LOCAL_LITERAL___signed_sum){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }){ Type: Signed32Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0xd]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_add_result){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local__i_add_c;
					int32_t local___signed_sum;
					uint32_t local___unsigned_sum;
					uint32_t local__o_add_c;
					uint32_t local__i_add_x;
					uint32_t local__i_add_y;
					uint32_t local__o_add_result;
					int32_t local__o_add_v;
					
					// read SP
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::SP), true, false);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
					local__i_add_c = 0x0;
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0xd), false, true);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf000) == 0xc000)
	{
		// Mask:    0b1111'0000'0000'0000
		// Compare: 0b1100'0000'0000'0000
		printf("Mask:    0b1111'0000'0000'0000\n");
		printf("Compare: 0b1100'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x800)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b1000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: ,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: ,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: ,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: ,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: ,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: ,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDM, "LDM", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_n = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_list = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }if ((LITERAL_list<CONSTNUM_0x0..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x0]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x1..CONSTNUM_0x1> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x1]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x2]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x3..CONSTNUM_0x3> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x3]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x4..CONSTNUM_0x4> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x4]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x5..CONSTNUM_0x5> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x5]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x6..CONSTNUM_0x6> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x6]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x7..CONSTNUM_0x7> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x7]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<LITERAL_n..LITERAL_n> == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
				(MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + (CONSTNUM_0x4 * LOCAL_LITERAL_bit_count){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint32_t local_bit_count;
				uint8_t local__o_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local_bit_count = 0x0;
				if ((((param_list & 0x1) >> 0x0) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x0), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x2) >> 0x1) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x1), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x4) >> 0x2) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x2), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x8) >> 0x3) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x3), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x10) >> 0x4) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x4), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x20) >> 0x5) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x5), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x40) >> 0x6) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x6), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x80) >> 0x7) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x7), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & MakeSliceMask(param_n, param_n)) >> param_n) == 0x0))
				{
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), false, true);
					// Maybe PC write:
					if (param_n == 15)
					{
						m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x800)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b1000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDM2, "LDM", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_n = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_list = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }if ((LITERAL_list<CONSTNUM_0x0..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x0]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x1..CONSTNUM_0x1> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x1]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x2]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x3..CONSTNUM_0x3> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x3]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x4..CONSTNUM_0x4> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x4]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x5..CONSTNUM_0x5> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x5]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x6..CONSTNUM_0x6> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x6]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x7..CONSTNUM_0x7> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[CONSTNUM_0x7]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_count = (LOCAL_LITERAL_bit_count + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<LITERAL_n..LITERAL_n> == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }) {
				(MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + (CONSTNUM_0x4 * LOCAL_LITERAL_bit_count){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }}
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint32_t local_bit_count;
				uint8_t local__o_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local_bit_count = 0x0;
				if ((((param_list & 0x1) >> 0x0) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x0), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x2) >> 0x1) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x1), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x4) >> 0x2) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x2), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x8) >> 0x3) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x3), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x10) >> 0x4) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x4), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x20) >> 0x5) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x5), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x40) >> 0x6) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x6), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & 0x80) >> 0x7) == 0x1))
				{
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x7), false, true);
					local_bit_count = (local_bit_count + 0x1);
				}
				if ((((param_list & MakeSliceMask(param_n, param_n)) >> param_n) == 0x0))
				{
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), false, true);
					// Maybe PC write:
					if (param_n == 15)
					{
						m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, (0x4 * local_bit_count))), u32), false, true, false, false);
					}
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x0)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: STM
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: STM.N
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: STMIA
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: STMIA.N
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: STMEA
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: STMEA.N
				Syn param:
				operand: n
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: list
				preString: !,{
				bitWidth: 8
				signed: false
				type: str
				Syn param:
				operand: 
				preString: }
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STM, "STM", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_n = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_list = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_bit_counter = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x8){ Type: Unsigned32Bit, MemRef: no }if ((LITERAL_list<CONSTNUM_0x0..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				if ((LITERAL_list<CONSTNUM_0x1..CONSTNUM_0x1> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }} else {
				if ((LITERAL_list<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no }} else {
				if ((LITERAL_list<CONSTNUM_0x3..CONSTNUM_0x3> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: no }} else {
				if ((LITERAL_list<CONSTNUM_0x4..CONSTNUM_0x4> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: no }} else {
				if ((LITERAL_list<CONSTNUM_0x5..CONSTNUM_0x5> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x5){ Type: Unsigned32Bit, MemRef: no }} else {
				if ((LITERAL_list<CONSTNUM_0x6..CONSTNUM_0x6> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x6){ Type: Unsigned32Bit, MemRef: no }} else {
				if ((LITERAL_list<CONSTNUM_0x7..CONSTNUM_0x7> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL_lowest_set_bit = CONSTNUM_0x7){ Type: Unsigned32Bit, MemRef: no }}
				}
				}
				}
				}
				}
				}
				}
				if ((LITERAL_list<CONSTNUM_0x0..CONSTNUM_0x0> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				if (((LITERAL_n == CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no } && (LOCAL_LITERAL_lowest_set_bit != CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__i_data = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x0]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
				(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_counter = (LOCAL_LITERAL_bit_counter + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x1..CONSTNUM_0x1> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				if (((LITERAL_n == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no } && (LOCAL_LITERAL_lowest_set_bit != CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__i_data = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x1]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
				(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_counter = (LOCAL_LITERAL_bit_counter + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x2..CONSTNUM_0x2> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				if (((LITERAL_n == CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no } && (LOCAL_LITERAL_lowest_set_bit != CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__i_data = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x2]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
				(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_counter = (LOCAL_LITERAL_bit_counter + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x3..CONSTNUM_0x3> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				if (((LITERAL_n == CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: no } && (LOCAL_LITERAL_lowest_set_bit != CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__i_data = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x3]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
				(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_counter = (LOCAL_LITERAL_bit_counter + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x4..CONSTNUM_0x4> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				if (((LITERAL_n == CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: no } && (LOCAL_LITERAL_lowest_set_bit != CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__i_data = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x4]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
				(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_counter = (LOCAL_LITERAL_bit_counter + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x5..CONSTNUM_0x5> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				if (((LITERAL_n == CONSTNUM_0x5){ Type: Unsigned32Bit, MemRef: no } && (LOCAL_LITERAL_lowest_set_bit != CONSTNUM_0x5){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__i_data = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x5]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
				(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_counter = (LOCAL_LITERAL_bit_counter + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x6..CONSTNUM_0x6> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				if (((LITERAL_n == CONSTNUM_0x6){ Type: Unsigned32Bit, MemRef: no } && (LOCAL_LITERAL_lowest_set_bit != CONSTNUM_0x6){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__i_data = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x6]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
				(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_counter = (LOCAL_LITERAL_bit_counter + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				if ((LITERAL_list<CONSTNUM_0x7..CONSTNUM_0x7> == CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }) {
				if (((LITERAL_n == CONSTNUM_0x7){ Type: Unsigned32Bit, MemRef: no } && (LOCAL_LITERAL_lowest_set_bit != CONSTNUM_0x7){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }) {
				(LOCAL_LITERAL__i_data = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }} else {
				(LOCAL_LITERAL__i_data = MEMREF_R[CONSTNUM_0x7]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }}
				(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL__i_address + CONSTNUM_0x4){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL_bit_counter = (LOCAL_LITERAL_bit_counter + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no }){ Type: Unsigned32Bit, MemRef: no }}
				(MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_bit_counter){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint32_t local_lowest_set_bit;
				uint32_t local__i_data;
				uint32_t local_bit_counter;
				
				local_bit_counter = 0x0;
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				local_lowest_set_bit = 0x8;
				if ((((param_list & 0x1) >> 0x0) == 0x1))
				{
					local_lowest_set_bit = 0x0;
				} else {
					if ((((param_list & 0x2) >> 0x1) == 0x1))
					{
						local_lowest_set_bit = 0x1;
					} else {
						if ((((param_list & 0x4) >> 0x2) == 0x1))
						{
							local_lowest_set_bit = 0x2;
						} else {
							if ((((param_list & 0x8) >> 0x3) == 0x1))
							{
								local_lowest_set_bit = 0x3;
							} else {
								if ((((param_list & 0x10) >> 0x4) == 0x1))
								{
									local_lowest_set_bit = 0x4;
								} else {
									if ((((param_list & 0x20) >> 0x5) == 0x1))
									{
										local_lowest_set_bit = 0x5;
									} else {
										if ((((param_list & 0x40) >> 0x6) == 0x1))
										{
											local_lowest_set_bit = 0x6;
										} else {
											if ((((param_list & 0x80) >> 0x7) == 0x1))
											{
												local_lowest_set_bit = 0x7;
											}
										}
									}
								}
							}
						}
					}
				}
				if ((((param_list & 0x1) >> 0x0) == 0x1))
				{
					if (((param_n == 0x0) && (local_lowest_set_bit != 0x0)))
					{
						local__i_data = 0x0;
					} else {
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x0), true, false);
					}
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					local_bit_counter = (local_bit_counter + 0x1);
				}
				if ((((param_list & 0x2) >> 0x1) == 0x1))
				{
					if (((param_n == 0x1) && (local_lowest_set_bit != 0x1)))
					{
						local__i_data = 0x0;
					} else {
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x1), true, false);
					}
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					local_bit_counter = (local_bit_counter + 0x1);
				}
				if ((((param_list & 0x4) >> 0x2) == 0x1))
				{
					if (((param_n == 0x2) && (local_lowest_set_bit != 0x2)))
					{
						local__i_data = 0x0;
					} else {
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x2), true, false);
					}
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					local_bit_counter = (local_bit_counter + 0x1);
				}
				if ((((param_list & 0x8) >> 0x3) == 0x1))
				{
					if (((param_n == 0x3) && (local_lowest_set_bit != 0x3)))
					{
						local__i_data = 0x0;
					} else {
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x3), true, false);
					}
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					local_bit_counter = (local_bit_counter + 0x1);
				}
				if ((((param_list & 0x10) >> 0x4) == 0x1))
				{
					if (((param_n == 0x4) && (local_lowest_set_bit != 0x4)))
					{
						local__i_data = 0x0;
					} else {
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x4), true, false);
					}
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					local_bit_counter = (local_bit_counter + 0x1);
				}
				if ((((param_list & 0x20) >> 0x5) == 0x1))
				{
					if (((param_n == 0x5) && (local_lowest_set_bit != 0x5)))
					{
						local__i_data = 0x0;
					} else {
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x5), true, false);
					}
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					local_bit_counter = (local_bit_counter + 0x1);
				}
				if ((((param_list & 0x40) >> 0x6) == 0x1))
				{
					if (((param_n == 0x6) && (local_lowest_set_bit != 0x6)))
					{
						local__i_data = 0x0;
					} else {
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x6), true, false);
					}
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					local_bit_counter = (local_bit_counter + 0x1);
				}
				if ((((param_list & 0x80) >> 0x7) == 0x1))
				{
					if (((param_n == 0x7) && (local_lowest_set_bit != 0x7)))
					{
						local__i_data = 0x0;
					} else {
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + 0x7), true, false);
					}
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeRegisterExpression(ARMv6M::R0 + param_n), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					local_bit_counter = (local_bit_counter + 0x1);
				}
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), false, true);
				// Maybe PC write:
				if (param_n == 15)
				{
					m_instrInProgress->addSuccessor(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_bit_counter)), u32), false, true, false, false);
				}
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe000) == 0x6000)
	{
		// Mask:    0b1110'0000'0000'0000
		// Compare: 0b0110'0000'0000'0000
		printf("Mask:    0b1110'0000'0000'0000\n");
		printf("Compare: 0b0110'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1000) == 0x0)
		{
			// Mask:    0b1'0000'0000'0000
			// Compare: 0b0'0000'0000'0000
			printf("Mask:    0b1'0000'0000'0000\n");
			printf("Compare: 0b0'0000'0000'0000\n");
			
			if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x800)
			{
				// Mask:    0b1000'0000'0000
				// Compare: 0b1000'0000'0000
				printf("Mask:    0b1000'0000'0000\n");
				printf("Compare: 0b1000'0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7c0) == 0x0)
				{
					// Mask:    0b111'1100'0000
					// Compare: 0b000'0000'0000
					printf("Mask:    0b111'1100'0000\n");
					printf("Compare: 0b000'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: t
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,[R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: 
						preString: ]
						bitWidth: 0
						signed: false
						type: str
						// mnemonic: 
						Syn param:
						operand: t
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,[R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: 
						preString: ]
						bitWidth: 0
						signed: false
						type: str
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDR, "LDR", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL_imm = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unknown, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local_imm32;
						uint32_t local__i_address;
						uint32_t local_imm;
						uint8_t local__o_data;
						
						local_imm = CoerceBits<uint32_t>(0x0);
						local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
						// Maybe PC write:
						if (param_t == 15)
						{
							m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true, false, false);
						}
						*/
					}
				}
				else
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: imm
					preString: ,#
					bitWidth: 5
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: imm
					preString: ,#
					bitWidth: 5
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDR2, "LDR", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1100'0000
					UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm32;
					uint32_t local__i_address;
					uint8_t local__o_data;
					
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
					// Maybe PC write:
					if (param_t == 15)
					{
						m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true, false, false);
					}
					*/
				}
			}
			else
			if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x0)
			{
				// Mask:    0b1000'0000'0000
				// Compare: 0b0000'0000'0000
				printf("Mask:    0b1000'0000'0000\n");
				printf("Compare: 0b0000'0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7c0) == 0x0)
				{
					// Mask:    0b111'1100'0000
					// Compare: 0b000'0000'0000
					printf("Mask:    0b111'1100'0000\n");
					printf("Compare: 0b000'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: t
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,[R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: 
						preString: ]
						bitWidth: 0
						signed: false
						type: str
						// mnemonic: 
						Syn param:
						operand: t
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,[R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: 
						preString: ]
						bitWidth: 0
						signed: false
						type: str
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STR, "STR", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL_imm = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unknown, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local_imm32;
						uint32_t local__i_address;
						uint32_t local_imm;
						uint32_t local__i_data;
						
						local_imm = CoerceBits<uint32_t>(0x0);
						local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
						*/
					}
				}
				else
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: imm
					preString: ,#
					bitWidth: 5
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: imm
					preString: ,#
					bitWidth: 5
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STR2, "STR", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1100'0000
					UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm32;
					uint32_t local__i_address;
					uint32_t local__i_data;
					
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x1000) == 0x1000)
		{
			// Mask:    0b1'0000'0000'0000
			// Compare: 0b1'0000'0000'0000
			printf("Mask:    0b1'0000'0000'0000\n");
			printf("Compare: 0b1'0000'0000'0000\n");
			
			if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x800)
			{
				// Mask:    0b1000'0000'0000
				// Compare: 0b1000'0000'0000
				printf("Mask:    0b1000'0000'0000\n");
				printf("Compare: 0b1000'0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7c0) == 0x0)
				{
					// Mask:    0b111'1100'0000
					// Compare: 0b000'0000'0000
					printf("Mask:    0b111'1100'0000\n");
					printf("Compare: 0b000'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: t
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,[R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: 
						preString: ]
						bitWidth: 0
						signed: false
						type: str
						// mnemonic: 
						Syn param:
						operand: t
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,[R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: 
						preString: ]
						bitWidth: 0
						signed: false
						type: str
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDRB, "LDRB", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL_imm = CONSTNUM_0x0){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = CAST(SIGNED, 32, LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0>)){ Type: Signed32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local_imm32;
						uint32_t local__i_address;
						uint32_t local_imm;
						uint8_t local__o_data;
						
						local_imm = 0x0;
						local_imm32 = CoerceBits<uint32_t>(param_imm);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						// read M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
						// write R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
						// Maybe PC write:
						if (param_t == 15)
						{
							m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true, false, false);
						}
						*/
					}
				}
				else
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: imm
					preString: ,#
					bitWidth: 5
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: imm
					preString: ,#
					bitWidth: 5
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDRB2, "LDRB", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1100'0000
					UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = CAST(SIGNED, 32, LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0>)){ Type: Signed32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm32;
					uint32_t local__i_address;
					uint8_t local__o_data;
					
					local_imm32 = CoerceBits<uint32_t>(param_imm);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
					// Maybe PC write:
					if (param_t == 15)
					{
						m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true, false, false);
					}
					*/
				}
			}
			else
			if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x0)
			{
				// Mask:    0b1000'0000'0000
				// Compare: 0b0000'0000'0000
				printf("Mask:    0b1000'0000'0000\n");
				printf("Compare: 0b0000'0000'0000\n");
				
				if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7c0) == 0x0)
				{
					// Mask:    0b111'1100'0000
					// Compare: 0b000'0000'0000
					printf("Mask:    0b111'1100'0000\n");
					printf("Compare: 0b000'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: t
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,[R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: 
						preString: ]
						bitWidth: 0
						signed: false
						type: str
						// mnemonic: 
						Syn param:
						operand: t
						preString: R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: n
						preString: ,[R
						bitWidth: 3
						signed: false
						type: dec
						Syn param:
						operand: 
						preString: ]
						bitWidth: 0
						signed: false
						type: str
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STRB, "STRB", 2, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b11'1000
						UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: []
						// Mask: 0b111
						UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						(LOCAL_LITERAL_imm = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
						Debug string end */
						
						/*
						uint32_t local_imm32;
						uint32_t local__i_address;
						uint32_t local_imm;
						uint32_t local__i_data;
						
						local_imm = CoerceBits<uint32_t>(0x0);
						local_imm32 = CoerceBits<uint32_t>(param_imm);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
						// read R
						m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
						// write M
						m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true);
						*/
					}
				}
				else
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: imm
					preString: ,#
					bitWidth: 5
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: imm
					preString: ,#
					bitWidth: 5
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STRB2, "STRB", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'1100'0000
					UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm32 = CAST(32, LITERAL_imm)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm32;
					uint32_t local__i_address;
					uint32_t local__i_data;
					
					local_imm32 = CoerceBits<uint32_t>(param_imm);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true);
					*/
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf000) == 0x9000)
	{
		// Mask:    0b1111'0000'0000'0000
		// Compare: 0b1001'0000'0000'0000
		printf("Mask:    0b1111'0000'0000'0000\n");
		printf("Compare: 0b1001'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x800)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b1000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b1000'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xff) == 0x0)
			{
				// Mask:    0b1111'1111
				// Compare: 0b0000'0000
				printf("Mask:    0b1111'1111\n");
				printf("Compare: 0b0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ,[SP]
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ,[SP]
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDR3, "LDR", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(1, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'0000'0000
					UWord param_t = ((m_instrWord >> 16) & 0x700) >> 8;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_n = CONSTNUM_0xd){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unknown, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm32;
					uint32_t local__i_address;
					uint32_t local_imm;
					uint8_t local__o_data;
					uint32_t local_n;
					
					local_imm = CoerceBits<uint32_t>(0x0);
					local_n = 0xd;
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
					// Maybe PC write:
					if (param_t == 15)
					{
						m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true, false, false);
					}
					*/
				}
			}
			else
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,[SP,#
				bitWidth: 8
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,[SP,#
				bitWidth: 8
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDR4, "LDR", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_t = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_n = CONSTNUM_0xd){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_imm32;
				uint32_t local__i_address;
				uint8_t local__o_data;
				uint32_t local_n;
				
				local_n = 0xd;
				local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
				// Maybe PC write:
				if (param_t == 15)
				{
					m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x0)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0xff) == 0x0)
			{
				// Mask:    0b1111'1111
				// Compare: 0b0000'0000
				printf("Mask:    0b1111'1111\n");
				printf("Compare: 0b0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ,[SP]
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ,[SP]
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STR3, "STR", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(1, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111'0000'0000
					UWord param_t = ((m_instrWord >> 16) & 0x700) >> 8;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unknown, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm32;
					uint32_t local__i_address;
					uint32_t local_imm;
					uint32_t local__i_data;
					
					local_imm = CoerceBits<uint32_t>(0x0);
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
					*/
				}
			}
			else
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,[SP,#
				bitWidth: 8
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,[SP,#
				bitWidth: 8
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STR4, "STR", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(1, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'0000'0000
				UWord param_t = ((m_instrWord >> 16) & 0x700) >> 8;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1111'1111
				UWord param_imm = ((m_instrWord >> 16) & 0xff) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_imm32;
				uint32_t local__i_address;
				uint32_t local__i_data;
				
				local_imm32 = CoerceBits<uint32_t>((param_imm << 0x2));
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf800) == 0x4800)
	{
		// Mask:    0b1111'1000'0000'0000
		// Compare: 0b0100'1000'0000'0000
		printf("Mask:    0b1111'1000'0000'0000\n");
		printf("Compare: 0b0100'1000'0000'0000\n");
		
		{
			/* SyntaxNode dump:
			// mnemonic: 
			Syn param:
			operand: t
			preString: R
			bitWidth: 3
			signed: false
			type: dec
			Syn param:
			operand: label
			preString: ,
			bitWidth: 8
			signed: false
			type: str
			// mnemonic: 
			Syn param:
			operand: t
			preString: R
			bitWidth: 3
			signed: false
			type: dec
			Syn param:
			operand: label
			preString: ,
			bitWidth: 8
			signed: false
			type: str
			End SyntaxNode dump */
			
			m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDR5, "LDR", 2, buf.start));
			
			// EncodingParameter begin
			/*ensureWeHave(1, buf);
			
			// TODO resolve slice definitions: []
			// Mask: 0b111'0000'0000
			UWord param_t = ((m_instrWord >> 16) & 0x700) >> 8;
			*/
			// EncodingParameter end
			// EncodingParameter begin
			/*ensureWeHave(2, buf);
			
			// TODO resolve slice definitions: []
			// Mask: 0b1111'1111
			UWord param_label = ((m_instrWord >> 16) & 0xff) >> 0;
			*/
			// EncodingParameter end
		
			/* Debug string begin
			(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_label << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_base = (MEMREF_PC{ Type: Unsigned32Bit } & CONSTNUM_0xfffffffc){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL_base + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }
			Debug string end */
			
			/*
			uint32_t local_imm32;
			uint32_t local__i_address;
			uint8_t local__o_data;
			uint32_t local_base;
			
			local_imm32 = CoerceBits<uint32_t>((param_label << 0x2));
			// read PC
			m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
			// read M
			m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), true, false);
			// read M
			m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
			// read M
			m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
			// read M
			m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
			// write R
			m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
			// Maybe PC write:
			if (param_t == 15)
			{
				m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true, false, false);
			}
			*/
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf800) == 0x4800)
	{
		// Mask:    0b1111'1000'0000'0000
		// Compare: 0b0100'1000'0000'0000
		printf("Mask:    0b1111'1000'0000'0000\n");
		printf("Compare: 0b0100'1000'0000'0000\n");
		
		{
			/* SyntaxNode dump:
			// mnemonic: 
			Syn param:
			operand: t
			preString: R
			bitWidth: 3
			signed: false
			type: dec
			Syn param:
			operand: label
			preString: ,[PC,#
			bitWidth: 8
			signed: false
			type: dec
			Syn param:
			operand: 
			preString: ]
			bitWidth: 0
			signed: false
			type: str
			// mnemonic: 
			Syn param:
			operand: t
			preString: R
			bitWidth: 3
			signed: false
			type: dec
			Syn param:
			operand: label
			preString: ,[PC,#
			bitWidth: 8
			signed: false
			type: dec
			Syn param:
			operand: 
			preString: ]
			bitWidth: 0
			signed: false
			type: str
			End SyntaxNode dump */
			
			m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDR6, "LDR", 2, buf.start));
			
			// EncodingParameter begin
			/*ensureWeHave(1, buf);
			
			// TODO resolve slice definitions: []
			// Mask: 0b111'0000'0000
			UWord param_t = ((m_instrWord >> 16) & 0x700) >> 8;
			*/
			// EncodingParameter end
			// EncodingParameter begin
			/*ensureWeHave(2, buf);
			
			// TODO resolve slice definitions: []
			// Mask: 0b1111'1111
			UWord param_label = ((m_instrWord >> 16) & 0xff) >> 0;
			*/
			// EncodingParameter end
		
			/* Debug string begin
			(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_label << CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_base = (MEMREF_PC{ Type: Unsigned32Bit } & CONSTNUM_0xfffffffc){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_address = (LOCAL_LITERAL_base + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }
			Debug string end */
			
			/*
			uint32_t local_imm32;
			uint32_t local__i_address;
			uint8_t local__o_data;
			uint32_t local_base;
			
			local_imm32 = CoerceBits<uint32_t>((param_label << 0x2));
			// read PC
			m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::PC), true, false);
			// read M
			m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), true, false);
			// read M
			m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
			// read M
			m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
			// read M
			m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
			// write R
			m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
			// Maybe PC write:
			if (param_t == 15)
			{
				m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeAndExpression(makeRegisterExpression(ARMv6M::PC), Immediate::makeImmediate(Result(u32, 0xfffffffc)), u32), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true, false, false);
			}
			*/
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf000) == 0x5000)
	{
		// Mask:    0b1111'0000'0000'0000
		// Compare: 0b0101'0000'0000'0000
		printf("Mask:    0b1111'0000'0000'0000\n");
		printf("Compare: 0b0101'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe00) == 0x800)
		{
			// Mask:    0b1110'0000'0000
			// Compare: 0b1000'0000'0000
			printf("Mask:    0b1110'0000'0000\n");
			printf("Compare: 0b1000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDR7, "LDR", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1'1100'0000
				UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x1f..CONSTNUM_0x18> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x17..CONSTNUM_0x10> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = LOCAL_LITERAL__o_data){ Type: Unsigned8Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint8_t local__o_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
				// Maybe PC write:
				if (param_t == 15)
				{
					m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe00) == 0xc00)
		{
			// Mask:    0b1110'0000'0000
			// Compare: 0b1100'0000'0000
			printf("Mask:    0b1110'0000'0000\n");
			printf("Compare: 0b1100'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDRB3, "LDRB", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1'1100'0000
				UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = CAST(32, LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0>)){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint8_t local__o_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
				// Maybe PC write:
				if (param_t == 15)
				{
					m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe00) == 0xa00)
		{
			// Mask:    0b1110'0000'0000
			// Compare: 0b1010'0000'0000
			printf("Mask:    0b1110'0000'0000\n");
			printf("Compare: 0b1010'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDRH, "LDRH", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1'1100'0000
				UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = CAST(32, LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x0>)){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint8_t local__o_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
				// Maybe PC write:
				if (param_t == 15)
				{
					m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe00) == 0x600)
		{
			// Mask:    0b1110'0000'0000
			// Compare: 0b0110'0000'0000
			printf("Mask:    0b1110'0000'0000\n");
			printf("Compare: 0b0110'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: LDRSB
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: LDRSB.N
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDRSB, "LDRSB", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1'1100'0000
				UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = CAST(SIGNED, 32, LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0>)){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint8_t local__o_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
				// Maybe PC write:
				if (param_t == 15)
				{
					m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe00) == 0xe00)
		{
			// Mask:    0b1110'0000'0000
			// Compare: 0b1110'0000'0000
			printf("Mask:    0b1110'0000'0000\n");
			printf("Compare: 0b1110'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: LDRSH
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: LDRSH.N
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDRSH, "LDRSH", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1'1100'0000
				UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = CAST(SIGNED, 32, LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x0>)){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint8_t local__o_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
				// Maybe PC write:
				if (param_t == 15)
				{
					m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe00) == 0x0)
		{
			// Mask:    0b1110'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1110'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STR5, "STR", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1'1100'0000
				UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x1f..CONSTNUM_0x18>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x17..CONSTNUM_0x10>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x2){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x3){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint32_t local__i_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), false, true);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x2)), u32), u8), false, true);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x3)), u32), u8), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe00) == 0x200)
		{
			// Mask:    0b1110'0000'0000
			// Compare: 0b0010'0000'0000
			printf("Mask:    0b1110'0000'0000\n");
			printf("Compare: 0b0010'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STRH, "STRH", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1'1100'0000
				UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint32_t local__i_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), false, true);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xe00) == 0x400)
		{
			// Mask:    0b1110'0000'0000
			// Compare: 0b0100'0000'0000
			printf("Mask:    0b1110'0000'0000\n");
			printf("Compare: 0b0100'0000'0000\n");
			
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: m
				preString: ,R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STRB3, "STRB", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b1'1100'0000
				UWord param_m = ((m_instrWord >> 16) & 0x1c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + MEMREF_R[LITERAL_m]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local__i_address;
				uint32_t local__i_data;
				
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_m), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), makeRegisterExpression(ARMv6M::R0 + param_m), u32), u8), false, true);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0xf000) == 0x8000)
	{
		// Mask:    0b1111'0000'0000'0000
		// Compare: 0b1000'0000'0000'0000
		printf("Mask:    0b1111'0000'0000'0000\n");
		printf("Compare: 0b1000'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x800)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b1000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b1000'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7c0) == 0x0)
			{
				// Mask:    0b111'1100'0000
				// Compare: 0b000'0000'0000
				printf("Mask:    0b111'1100'0000\n");
				printf("Compare: 0b000'0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDRH2, "LDRH", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x1){ Type: Unknown, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = CAST(SIGNED, 32, LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x0>)){ Type: Signed32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm32;
					uint32_t local__i_address;
					uint32_t local_imm;
					uint8_t local__o_data;
					
					local_imm = CoerceBits<uint32_t>(0x0);
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x1));
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), true, false);
					// read M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
					// write R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
					// Maybe PC write:
					if (param_t == 15)
					{
						m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true, false, false);
					}
					*/
				}
			}
			else
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_LDRH3, "LDRH", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1100'0000
				UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x8> = MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(LOCAL_LITERAL__o_data<CONSTNUM_0x7..CONSTNUM_0x0> = MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit }){ Type: Unsigned8Bit, MemRef: yes }(MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit } = CAST(SIGNED, 32, LOCAL_LITERAL__o_data<CONSTNUM_0xf..CONSTNUM_0x0>)){ Type: Signed32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_imm32;
				uint32_t local__i_address;
				uint8_t local__o_data;
				
				local_imm32 = CoerceBits<uint32_t>((param_imm << 0x1));
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), true, false);
				// read M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), true, false);
				// write R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), false, true);
				// Maybe PC write:
				if (param_t == 15)
				{
					m_instrInProgress->addSuccessor(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true, false, false);
				}
				*/
			}
		}
		else
		if (ensureWeHave(1, buf), ((m_instrWord >> 16) & 0x800) == 0x0)
		{
			// Mask:    0b1000'0000'0000
			// Compare: 0b0000'0000'0000
			printf("Mask:    0b1000'0000'0000\n");
			printf("Compare: 0b0000'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 16) & 0x7c0) == 0x0)
			{
				// Mask:    0b111'1100'0000
				// Compare: 0b000'0000'0000
				printf("Mask:    0b111'1100'0000\n");
				printf("Compare: 0b000'0000'0000\n");
				
				{
					/* SyntaxNode dump:
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					// mnemonic: 
					Syn param:
					operand: t
					preString: R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: n
					preString: ,[R
					bitWidth: 3
					signed: false
					type: dec
					Syn param:
					operand: 
					preString: ]
					bitWidth: 0
					signed: false
					type: str
					End SyntaxNode dump */
					
					m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STRH2, "STRH", 2, buf.start));
					
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b11'1000
					UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
					*/
					// EncodingParameter end
					// EncodingParameter begin
					/*ensureWeHave(2, buf);
					
					// TODO resolve slice definitions: []
					// Mask: 0b111
					UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
					*/
					// EncodingParameter end
				
					/* Debug string begin
					(LOCAL_LITERAL_imm = CAST(32, CONSTNUM_0x0)){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x1){ Type: Unknown, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
					Debug string end */
					
					/*
					uint32_t local_imm32;
					uint32_t local__i_address;
					uint32_t local_imm;
					uint32_t local__i_data;
					
					local_imm = CoerceBits<uint32_t>(0x0);
					local_imm32 = CoerceBits<uint32_t>((param_imm << 0x1));
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
					// read R
					m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true);
					// write M
					m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
					*/
				}
			}
			else
			{
				/* SyntaxNode dump:
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				// mnemonic: 
				Syn param:
				operand: t
				preString: R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: n
				preString: ,[R
				bitWidth: 3
				signed: false
				type: dec
				Syn param:
				operand: imm
				preString: ,#
				bitWidth: 5
				signed: false
				type: dec
				Syn param:
				operand: 
				preString: ]
				bitWidth: 0
				signed: false
				type: str
				End SyntaxNode dump */
				
				m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_STRH3, "STRH", 2, buf.start));
				
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111'1100'0000
				UWord param_imm = ((m_instrWord >> 16) & 0x7c0) >> 6;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b11'1000
				UWord param_n = ((m_instrWord >> 16) & 0x38) >> 3;
				*/
				// EncodingParameter end
				// EncodingParameter begin
				/*ensureWeHave(2, buf);
				
				// TODO resolve slice definitions: []
				// Mask: 0b111
				UWord param_t = ((m_instrWord >> 16) & 0x7) >> 0;
				*/
				// EncodingParameter end
			
				/* Debug string begin
				(LOCAL_LITERAL_imm32 = CAST(32, (LITERAL_imm << CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: no })){ Type: Unsigned32Bit, MemRef: no }(LOCAL_LITERAL__i_address = (MEMREF_R[LITERAL_n]{ Type: Unsigned32Bit } + LOCAL_LITERAL_imm32){ Type: Unsigned32Bit, MemRef: yes }){ Type: Unsigned32Bit, MemRef: yes }(LOCAL_LITERAL__i_data = MEMREF_R[LITERAL_t]{ Type: Unsigned32Bit }){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[LOCAL_LITERAL__i_address]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0xf..CONSTNUM_0x8>){ Type: Unsigned32Bit, MemRef: yes }(MEMREF_M[(LOCAL_LITERAL__i_address + CONSTNUM_0x1){ Type: Unsigned32Bit, MemRef: yes }]{ Type: Unsigned8Bit } = LOCAL_LITERAL__i_data<CONSTNUM_0x7..CONSTNUM_0x0>){ Type: Unsigned32Bit, MemRef: yes }
				Debug string end */
				
				/*
				uint32_t local_imm32;
				uint32_t local__i_address;
				uint32_t local__i_data;
				
				local_imm32 = CoerceBits<uint32_t>((param_imm << 0x1));
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_n), true, false);
				// read R
				m_instrInProgress->appendOperand(makeRegisterExpression(ARMv6M::R0 + param_t), true, false);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), u8), false, true);
				// write M
				m_instrInProgress->appendOperand(makeDereferenceExpression(makeAddExpression(makeAddExpression(makeRegisterExpression(ARMv6M::R0 + param_n), Immediate::makeImmediate(Result(u32, local_imm32)), u32), Immediate::makeImmediate(Result(u32, 0x1)), u32), u8), false, true);
				*/
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	if (ensureWeHave(1, buf), ((m_instrWord >> 0) & 0xf0000000) == 0xf0000000)
	{
		// Mask:    0b1111'0000'0000'0000'0000'0000'0000'0000
		// Compare: 0b1111'0000'0000'0000'0000'0000'0000'0000
		printf("Mask:    0b1111'0000'0000'0000'0000'0000'0000'0000\n");
		printf("Compare: 0b1111'0000'0000'0000'0000'0000'0000'0000\n");
		
		if (ensureWeHave(1, buf), ((m_instrWord >> 0) & 0xf000000) == 0x7000000)
		{
			// Mask:    0b1111'0000'0000'0000'0000'0000'0000
			// Compare: 0b0111'0000'0000'0000'0000'0000'0000
			printf("Mask:    0b1111'0000'0000'0000'0000'0000'0000\n");
			printf("Compare: 0b0111'0000'0000'0000'0000'0000'0000\n");
			
			if (ensureWeHave(2, buf), ((m_instrWord >> 0) & 0xf00000) == 0xf00000)
			{
				// Mask:    0b1111'0000'0000'0000'0000'0000
				// Compare: 0b1111'0000'0000'0000'0000'0000
				printf("Mask:    0b1111'0000'0000'0000'0000'0000\n");
				printf("Compare: 0b1111'0000'0000'0000'0000'0000\n");
				
				if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0xf000) == 0xa000)
				{
					// Mask:    0b1111'0000'0000'0000
					// Compare: 0b1010'0000'0000'0000
					printf("Mask:    0b1111'0000'0000'0000\n");
					printf("Compare: 0b1010'0000'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: imm
						preString: #
						bitWidth: 16
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_UDF_W, "UDF.W", 4, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(4, buf);
						
						// TODO resolve slice definitions: [11, 0]
						// Mask: 0b1111'1111'1111
						struct { Word field : 12; } signExtender_imm;
						Word param_imm = signExtender_imm.field = ((m_instrWord >> 0) & 0xfff) >> 0;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: [15, 12]
						// Mask: 0b1111'0000'0000'0000'0000
						struct { Word field : 4; } signExtender_imm;
						Word param_imm = signExtender_imm.field = ((m_instrWord >> 0) & 0xf0000) >> 16;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						Debug string end */
						
						/*
						
						*/
					}
				}
				else
				if (ensureWeHave(3, buf), ((m_instrWord >> 0) & 0xf000) == 0xa000)
				{
					// Mask:    0b1111'0000'0000'0000
					// Compare: 0b1010'0000'0000'0000
					printf("Mask:    0b1111'0000'0000'0000\n");
					printf("Compare: 0b1010'0000'0000'0000\n");
					
					{
						/* SyntaxNode dump:
						// mnemonic: 
						Syn param:
						operand: imm
						preString: 
						bitWidth: 16
						signed: false
						type: dec
						End SyntaxNode dump */
						
						m_instrInProgress = make_shared(makeInstruction(ARMv6M_op_UDF_W2, "UDF.W", 4, buf.start));
						
						// EncodingParameter begin
						/*ensureWeHave(4, buf);
						
						// TODO resolve slice definitions: [11, 0]
						// Mask: 0b1111'1111'1111
						struct { Word field : 12; } signExtender_imm;
						Word param_imm = signExtender_imm.field = ((m_instrWord >> 0) & 0xfff) >> 0;
						*/
						// EncodingParameter end
						// EncodingParameter begin
						/*ensureWeHave(2, buf);
						
						// TODO resolve slice definitions: [15, 12]
						// Mask: 0b1111'0000'0000'0000'0000
						struct { Word field : 4; } signExtender_imm;
						Word param_imm = signExtender_imm.field = ((m_instrWord >> 0) & 0xf0000) >> 16;
						*/
						// EncodingParameter end
					
						/* Debug string begin
						Debug string end */
						
						/*
						
						*/
					}
				}
				else
				{
				// Invalid instruction.
				m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
				}
			}
			else
			{
			// Invalid instruction.
			m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
			}
		}
		else
		{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
		}
	}
	else
	{
		// Invalid instruction.
		m_instrInProgress = make_shared(makeInstruction(e_No_Entry, "", 1, buf.start));
	}
	
	m_instrInProgress->arch_decoded_from = Arch_ARMv6M;
	m_Operation = m_instrInProgress->m_InsnOp;
	
	buf.start += m_instrInProgress->size();
}

void InstructionDecoder_ARMv6M::doDelayedDecode(const Instruction *insn_to_complete) {
	InstructionDecoder::buffer buf(insn_to_complete->ptr(), insn_to_complete->size());
	decode(buf);
	decodeOperands(insn_to_complete);
}


void InstructionDecoder_ARMv6M::ensureWeHave(int bytes, const InstructionDecoder::buffer &buf) {
	if (bytes > m_bytesRead) {
		switch (bytes) {
		case 1:
		case 2:
			m_instrWord = buf.start[1] << 24 | buf.start[0] << 16;
			m_bytesRead = 2;
			break;
		case 3:
		case 4:
			m_instrWord = buf.start[1] << 24 | buf.start[0] << 16 | buf.start[3] << 8 | buf.start[2];
			m_bytesRead = 4;
			break;
		default:
			throw std::runtime_error("Read of this size is not implemented");
		}
	}
}

}
}

/* MemDescription dump:
// mem desc start
MemDesc:
name: M
arraySize: 4294967296
value: 0
bitWidth: 8
slice: []
// mem desc end
// mem desc start
MemDesc:
name: R
arraySize: 16
value: 0
bitWidth: 32
slice: []
// mem desc start
MemDesc:
name: SP
arraySize: 1
value: 0
bitWidth: 0
slice: [13]
// mem desc end
// mem desc start
MemDesc:
name: LR
arraySize: 1
value: 0
bitWidth: 0
slice: [14]
// mem desc end
// mem desc start
MemDesc:
name: PC
arraySize: 1
value: 0
bitWidth: 0
slice: [15]
// mem desc end
// mem desc end
// mem desc start
MemDesc:
name: PSR
arraySize: 32
value: 0
bitWidth: 1
slice: []
// mem desc start
MemDesc:
name: APSR
arraySize: 4
value: 0
bitWidth: 0
slice: [28, 31]
// mem desc start
MemDesc:
name: N
arraySize: 1
value: 0
bitWidth: 0
slice: [3]
// mem desc end
// mem desc start
MemDesc:
name: Z
arraySize: 1
value: 0
bitWidth: 0
slice: [2]
// mem desc end
// mem desc start
MemDesc:
name: C
arraySize: 1
value: 0
bitWidth: 0
slice: [1]
// mem desc end
// mem desc start
MemDesc:
name: V
arraySize: 1
value: 0
bitWidth: 0
slice: [0]
// mem desc end
// mem desc end
// mem desc start
MemDesc:
name: EPSR
arraySize: 1
value: 0
bitWidth: 0
slice: [24]
// mem desc start
MemDesc:
name: T
arraySize: 1
value: 0
bitWidth: 0
slice: [0]
// mem desc end
// mem desc end
// mem desc start
MemDesc:
name: IPSR
arraySize: 6
value: 0
bitWidth: 0
slice: [0, 5]
// mem desc end
// mem desc end
*/

// file end
