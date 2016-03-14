#include <map>
#include <vector>
#include <iostream>
#include <boost/assign/list_of.hpp>
#include <bitset>
using namespace boost::assign;
#include "test_aarch64_decoder_table.h"

using namespace std;

int findInsnTableIndex(unsigned int insn, unsigned int decoder_table_index)
{
    cout << "index: "<< decoder_table_index << endl;

/*    cout << " insn: "<< bitset<32>(insn) << endl;
    cout << " mask: "<< bitset<32>(cur_mask) << endl;*/

	aarch64_mask_entry *cur_entry = &aarch64_mask_entry::main_decoder_table[decoder_table_index];
		unsigned int cur_mask = cur_entry->mask;

		if(cur_mask == 0)
			return cur_entry->insnTableIndex;

		unsigned int insn_iter_index = 0, map_key_index = 0, branch_map_key = 0;
		branchMap cur_branches = cur_entry->nodeBranches;

		while(insn_iter_index < 32)
		{
			if(((cur_mask>>insn_iter_index) & 1) == 1)
			{
				branch_map_key = branch_map_key | (((insn>>insn_iter_index) & 1)<<map_key_index);
				map_key_index++;
			}
			insn_iter_index++;
		}

		if(cur_branches.count(branch_map_key) <= 0)
			return 0;

		return findInsnTableIndex(insn, cur_branches[branch_map_key]);
}

int main()
{
	aarch64_mask_entry::buildDecoderTable();
	aarch64_insn_entry::buildInsnTable();
	unsigned int insn;

	std::cin>>std::hex>>insn;
	while(insn != -1)
	{
		//std::cout<<std::hex<<insn<<":"<<std::dec<<findInsnTableIndex(insn, 0)<<std::endl;
		const char *mnemonic = aarch64_insn_entry::main_insn_table[findInsnTableIndex(insn, 0)].mnemonic;
		std::cout<<mnemonic<<std::endl;
		std::cin>>std::hex>>insn;
	}

	return 0;
}
