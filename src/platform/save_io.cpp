#include "save_io.h"

#include "bn_sram.h"

namespace platform_save
{
    namespace
    {
        int slot_offset(int slot_index)
        {
            return slot_index * game::save_slot_reserved_bytes;
        }
    }

    bool slot_has_data(int slot_index)
    {
        game::save_slot_data data{};
        bn::sram::read_offset(data, slot_offset(slot_index));
        return data.magic == game::save_magic && data.in_use;
    }

    game::save_slot_data read_slot(int slot_index)
    {
        game::save_slot_data data{};
        bn::sram::read_offset(data, slot_offset(slot_index));
        return data;
    }

    void write_slot(int slot_index, const game::save_slot_data& data)
    {
        bn::sram::write_offset(data, slot_offset(slot_index));
    }
}
