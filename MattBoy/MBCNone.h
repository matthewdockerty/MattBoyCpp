#ifndef MBC_NONE_H_
#define MBC_NONE_H_

#include "MBC.h"

#include <vector>

namespace mattboy {

	class MBCNone : public MBC
	{
	public:
		MBCNone(CartridgeType type, int rom_size, int ram_size, bool battery, const std::vector<char>& data);
		~MBCNone();

		uint8_t* GetRomBank0();
		uint8_t* GetROMBankN(int n);
		uint8_t* GetRAMBankN(int n);

	private:
		uint8_t rom_bank_0_[ROM_BANK_SIZE];
		uint8_t rom_bank_1_[ROM_BANK_SIZE];
		uint8_t* ram_;
		static const int RAM_SIZE = 8 * 1024;
	};

}

#endif