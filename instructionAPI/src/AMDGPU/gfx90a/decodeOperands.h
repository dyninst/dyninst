#include <stdint.h>

Expression::Ptr decodeOPR_ACCVGPR(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_DSMEM(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_FLAT_SCRATCH(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_PC(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SDST(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SDST_EXEC(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SDST_M0(uint64_t input, uint32_t _num_elements = 1 );
void processOPR_SMEM_OFFSET(layout_ENC_SMEM & layout  );
Expression::Ptr decodeOPR_SRC(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SRC_ACCVGPR(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SRC_NOLDS(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SRC_NOLIT(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SRC_SIMPLE(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SRC_VGPR(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SRC_VGPR_OR_ACCVGPR(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SREG(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SREG_NOVCC(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SSRC(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SSRC_LANESEL(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SSRC_NOLIT(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_SSRC_SPECIAL_SCC(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_VCC(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_VGPR(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_VGPR_OR_ACCVGPR(uint64_t input, uint32_t _num_elements = 1 );
Expression::Ptr decodeOPR_VGPR_OR_LDS(uint64_t input, uint32_t _num_elements = 1 );
