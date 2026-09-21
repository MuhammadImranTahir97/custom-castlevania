#include "save_data.h"

namespace game
{
    save_slot_data build_save_slot_data(const player_state& player, int checkpoint_room_index,
            fixed checkpoint_x, fixed checkpoint_y)
    {
        save_slot_data slot{};
        slot.magic = save_magic;
        slot.in_use = true;

        slot.character = player.character;
        slot.level = player.level;
        slot.exp = player.exp;
        slot.max_hp = player.max_hp;
        slot.max_mp = player.max_mp;
        slot.hp = player.hp;
        slot.mp = player.mp;
        slot.hearts = player.hearts;
        slot.str = player.str;
        slot.def = player.def;
        slot.intelligence = player.intelligence;
        slot.lck = player.lck;

        slot.has_double_jump = player.has_double_jump;
        slot.rival_unlocked = player.rival_unlocked;

        slot.checkpoint_room_index = checkpoint_room_index;
        slot.checkpoint_x = checkpoint_x;
        slot.checkpoint_y = checkpoint_y;

        return slot;
    }

    void apply_save_slot_data(const save_slot_data& slot, player_state& player)
    {
        player.character = slot.character;
        player.level = slot.level;
        player.exp = slot.exp;
        player.max_hp = slot.max_hp;
        player.max_mp = slot.max_mp;
        player.hp = slot.hp;
        player.mp = slot.mp;
        player.hearts = slot.hearts;
        player.str = slot.str;
        player.def = slot.def;
        player.intelligence = slot.intelligence;
        player.lck = slot.lck;

        player.has_double_jump = slot.has_double_jump;
        player.rival_unlocked = slot.rival_unlocked;
    }
}
