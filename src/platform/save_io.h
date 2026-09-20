#pragma once

#include "save_data.h"

namespace platform_save
{
    // True if slot_index (0..game::save_slot_count - 1) holds a real save
    // (its stored magic matches game::save_magic) rather than blank SRAM
    // or a slot from an incompatible older build.
    bool slot_has_data(int slot_index);

    // Reads slot_index's data. Only meaningful when slot_has_data() is true
    // -- otherwise this is whatever garbage/zeroes are in that SRAM region.
    game::save_slot_data read_slot(int slot_index);

    void write_slot(int slot_index, const game::save_slot_data& data);
}
