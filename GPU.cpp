#include "GPU.h"

#include <iostream>

namespace mattboy {

	GPU::GPU() : mode_(HBLANK), current_cycles_(0), line_(0)
	{

	}

	GPU::~GPU()
	{

	}

	// TODO: Write values to memory & change MMU to only be able to access memory at the correct times.
	void GPU::Cycle(int cycles, MMU& mmu, InterruptHandler& interrupt_handler)
	{
		current_cycles_ += cycles;

		switch (mode_)
		{
		case HBLANK:
			if (current_cycles_ >= 204)
			{
				line_++;

				if (line_ > 143)
				{
					mode_ = VBLANK;
					// TODO: Should redraw screen
					printf("VBLANK!\n");
					interrupt_handler.RequestInterrupt(InterruptHandler::VBLANK);
				}
				else
				{
					mode_ = OAM;
				}
			}
			break;

		case VBLANK:
			if (current_cycles_ >= 456)
			{
				current_cycles_ -= 456;
				line_++;

				if (line_ > 153) // Overdraw 10 lines during vblank
				{
					mode_ = OAM;
					line_ = 0;
				}
			}
			break;

		case OAM:
			if (current_cycles_ >= 80)
			{
				mode_ = VRAM;
			}
			break;

		case VRAM:
			if (current_cycles_ >= 172)
			{
				mode_ = HBLANK;
				// TODO: Render line
			}
			break;
		}
		mmu.WriteByte(0xFF44, line_);
	}

	void GPU::Reset()
	{
		std::cout << "TODO: GPU Reset!" << std::endl;
	}

}