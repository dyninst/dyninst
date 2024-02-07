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

/* Functions of the relocationEntry class specific to AArch64 ELF */
#include <elf.h>
#include "Symtab.h"
#include "annotations.h"

using namespace Dyninst;
using namespace SymtabAPI;

const char *relocationEntry::relType2Str(unsigned long r, unsigned /*addressWidth*/) {
//#warning "This function is not verified yet!"
	//some of the defined symbol should not be included
	//how could I know which ones?
	switch(r){
		CASE_RETURN_STR(R_AARCH64_NONE);
        CASE_RETURN_STR(R_AARCH64_ABS64);
        CASE_RETURN_STR(R_AARCH64_ABS32);
        CASE_RETURN_STR(R_AARCH64_ABS16);
        CASE_RETURN_STR(R_AARCH64_PREL64);
        CASE_RETURN_STR(R_AARCH64_PREL32	);
        CASE_RETURN_STR(R_AARCH64_PREL16	);
        CASE_RETURN_STR(R_AARCH64_MOVW_UABS_G0	);
        CASE_RETURN_STR(R_AARCH64_MOVW_UABS_G0_NC);
        CASE_RETURN_STR(R_AARCH64_MOVW_UABS_G1	);
        CASE_RETURN_STR(R_AARCH64_MOVW_UABS_G1_NC);
        CASE_RETURN_STR(R_AARCH64_MOVW_UABS_G2	);
        CASE_RETURN_STR(R_AARCH64_MOVW_UABS_G2_NC);
        CASE_RETURN_STR(R_AARCH64_MOVW_UABS_G3	);
        CASE_RETURN_STR(R_AARCH64_MOVW_SABS_G0		);
        CASE_RETURN_STR(R_AARCH64_MOVW_SABS_G1	);
        CASE_RETURN_STR(R_AARCH64_MOVW_SABS_G2		);
        CASE_RETURN_STR(R_AARCH64_LD_PREL_LO19	);
        CASE_RETURN_STR(R_AARCH64_ADR_PREL_LO21		);
        CASE_RETURN_STR(R_AARCH64_ADR_PREL_PG_HI21 );
        CASE_RETURN_STR(R_AARCH64_ADR_PREL_PG_HI21_NC);
        CASE_RETURN_STR(R_AARCH64_ADD_ABS_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_LDST8_ABS_LO12_NC);
        CASE_RETURN_STR(R_AARCH64_TSTBR14	);
        CASE_RETURN_STR(R_AARCH64_CONDBR19);
        CASE_RETURN_STR(R_AARCH64_JUMP26	);
        CASE_RETURN_STR(R_AARCH64_CALL26	);
        CASE_RETURN_STR(R_AARCH64_LDST16_ABS_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_LDST32_ABS_LO12_NC);
        CASE_RETURN_STR(R_AARCH64_LDST64_ABS_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_MOVW_PREL_G0	);
        CASE_RETURN_STR(R_AARCH64_MOVW_PREL_G0_NC);
        CASE_RETURN_STR(R_AARCH64_MOVW_PREL_G1	);
        CASE_RETURN_STR(R_AARCH64_MOVW_PREL_G1_NC);
        CASE_RETURN_STR(R_AARCH64_MOVW_PREL_G2	);
        CASE_RETURN_STR(R_AARCH64_MOVW_PREL_G2_NC 	);
        CASE_RETURN_STR(R_AARCH64_MOVW_PREL_G3);
        CASE_RETURN_STR(R_AARCH64_LDST128_ABS_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_MOVW_GOTOFF_G0 );
        CASE_RETURN_STR(R_AARCH64_MOVW_GOTOFF_G0_NC );
        CASE_RETURN_STR(R_AARCH64_MOVW_GOTOFF_G1 );
        CASE_RETURN_STR(R_AARCH64_MOVW_GOTOFF_G1_NC 	);
        CASE_RETURN_STR(R_AARCH64_MOVW_GOTOFF_G2 );
        CASE_RETURN_STR(R_AARCH64_MOVW_GOTOFF_G2_NC  );
        CASE_RETURN_STR(R_AARCH64_MOVW_GOTOFF_G3 );
        CASE_RETURN_STR(R_AARCH64_GOTREL64	);
        CASE_RETURN_STR(R_AARCH64_GOTREL32	);
        CASE_RETURN_STR(R_AARCH64_GOT_LD_PREL19	);
        CASE_RETURN_STR(R_AARCH64_LD64_GOTOFF_LO15 );
        CASE_RETURN_STR(R_AARCH64_ADR_GOT_PAGE	);
        CASE_RETURN_STR(R_AARCH64_LD64_GOT_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_LD64_GOTPAGE_LO15 );
        CASE_RETURN_STR(R_AARCH64_TLSGD_ADR_PREL21 );
        CASE_RETURN_STR(R_AARCH64_TLSGD_ADR_PAGE21);
        CASE_RETURN_STR(R_AARCH64_TLSGD_ADD_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSGD_MOVW_G1	);
        CASE_RETURN_STR(R_AARCH64_TLSGD_MOVW_G0_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLD_ADR_PREL21    );
        CASE_RETURN_STR(R_AARCH64_TLSLD_ADR_PAGE21   );
        CASE_RETURN_STR(R_AARCH64_TLSLD_ADD_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLD_MOVW_G1	);
        CASE_RETURN_STR(R_AARCH64_TLSLD_MOVW_G0_NC 	);
        CASE_RETURN_STR(R_AARCH64_TLSLD_LD_PREL19 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_MOVW_DTPREL_G2 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_MOVW_DTPREL_G1 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_MOVW_DTPREL_G1_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLD_MOVW_DTPREL_G0 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_MOVW_DTPREL_G0_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLD_ADD_DTPREL_HI12 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_ADD_DTPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_ADD_DTPREL_LO12_NC);
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST8_DTPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST16_DTPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST32_DTPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST64_DTPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSIE_MOVW_GOTTPREL_G1  );
        CASE_RETURN_STR(R_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC  );
        CASE_RETURN_STR(R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21  );
        CASE_RETURN_STR(R_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSIE_LD_GOTTPREL_PREL19 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_MOVW_TPREL_G2 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_MOVW_TPREL_G1 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_MOVW_TPREL_G1_NC);
        CASE_RETURN_STR(R_AARCH64_TLSLE_MOVW_TPREL_G0 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_MOVW_TPREL_G0_NC);
        CASE_RETURN_STR(R_AARCH64_TLSLE_ADD_TPREL_HI12 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_ADD_TPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_ADD_TPREL_LO12_NC);
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST8_TPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST8_TPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST16_TPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST16_TPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST32_TPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST32_TPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST64_TPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST64_TPREL_LO12_NC );
        CASE_RETURN_STR(R_AARCH64_TLSDESC_LD_PREL19 );
        CASE_RETURN_STR(R_AARCH64_TLSDESC_ADR_PREL21);
        CASE_RETURN_STR(R_AARCH64_TLSDESC_ADR_PAGE21 );
        CASE_RETURN_STR(R_AARCH64_TLSDESC_LD64_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSDESC_ADD_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSDESC_OFF_G1 );
        CASE_RETURN_STR(R_AARCH64_TLSDESC_OFF_G0_NC );
        CASE_RETURN_STR(R_AARCH64_TLSDESC_LDR	);
        CASE_RETURN_STR(R_AARCH64_TLSDESC_ADD	);
        CASE_RETURN_STR(R_AARCH64_TLSDESC_CALL	);
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST128_TPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLE_LDST128_TPREL_LO12_NC);
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST128_DTPREL_LO12 );
        CASE_RETURN_STR(R_AARCH64_TLSLD_LDST128_DTPREL_LO12_NC);
        CASE_RETURN_STR(R_AARCH64_COPY);
        CASE_RETURN_STR(R_AARCH64_GLOB_DAT);
        CASE_RETURN_STR(R_AARCH64_JUMP_SLOT);
        CASE_RETURN_STR(R_AARCH64_RELATIVE);
	//        CASE_RETURN_STR(R_AARCH64_TLS_DTPMOD );
        // CASE_RETURN_STR(R_AARCH64_TLS_DTPREL);
        // CASE_RETURN_STR(R_AARCH64_TLS_TPREL);
        CASE_RETURN_STR(R_AARCH64_TLSDESC   );
        CASE_RETURN_STR(R_AARCH64_IRELATIVE);
		default:
			return "Unknown relocation type";
	}
    return "?";
}

SYMTAB_EXPORT unsigned long relocationEntry::getGlobalRelType(unsigned addressWidth, Symbol *sym) {
	//#warning "This functions is not verified yet!"

	//if it is 32bit arch?
	if(addressWidth == 4)
		return R_ARM_GLOB_DAT;

	//else it is 64bit arch
	if(!sym)
		return R_AARCH64_GLOB_DAT;
	if(sym->getType() == Symbol::ST_FUNCTION)
		return R_AARCH64_JUMP_SLOT;
	else
		return R_AARCH64_GLOB_DAT;

	//should never return this.
    return relocationEntry::dynrel;
}


relocationEntry::category
relocationEntry::getCategory( unsigned addressWidth )
{
    if( addressWidth == 8 ) {
       switch( getRelType() )
       {
           case R_AARCH64_RELATIVE:
           case R_AARCH64_IRELATIVE:
               return category::relative; 
           case R_AARCH64_JUMP_SLOT:
               return category::jump_slot; 
           default:
               return category::absolute;
       }
    }else{
       switch( getRelType() )
       {
           case R_ARM_RELATIVE:
           case R_ARM_IRELATIVE:
               return category::relative; 
           case R_ARM_JUMP_SLOT:
               return category::jump_slot; 
           default:
               return category::absolute;
       }
    }
}



