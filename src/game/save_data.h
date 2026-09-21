#pragma once

#include "fixed.h"
#include "player.h"

namespace game
{
    // SPEC.md section 9: SRAM, 3 slots, ~2KB reserved per slot, manual save
    // only (see current_room_is_save_room() — this module doesn't decide
    // *when* to save, src/platform does that in response to input).
    constexpr int save_slot_count = 3;
    constexpr int save_slot_reserved_bytes = 2048;

    // Written into every saved slot, checked on load. Distinguishes a real
    // save from blank/erased SRAM (which reads as all-zero or all-0xFF
    // depending on the chip) and from a slot written by an incompatible
    // older build — bump this if save_slot_data's layout ever changes.
    constexpr unsigned int save_magic = 0x434f544d; // "COTM"

    // Everything currently meaningful to persist: stats, character, relics,
    // and the checkpoint (SPEC.md: death respawns at the last save room, so
    // that's what "continue" needs to restore). Deliberately NOT here yet:
    // inventory, Arcana cards, map exploration, playtime — SPEC.md lists
    // them as saved data too, but none of those are systems in the game
    // yet (see ROADMAP.md). Add fields here once they exist, rather than
    // inventing placeholder data for systems that don't.
    struct save_slot_data
    {
        unsigned int magic = 0;
        bool in_use = false;

        character_kind character = character_kind::hunter;

        int level = 1;
        int exp = 0;
        int max_hp = 0;
        int max_mp = 0;
        int hp = 0;
        int mp = 0;
        int hearts = 0;
        int str = 0;
        int def = 0;
        int intelligence = 0;
        int lck = 0;

        bool has_double_jump = false;
        bool rival_unlocked = false;

        int checkpoint_room_index = 0;
        fixed checkpoint_x = 0;
        fixed checkpoint_y = 0;
    };

    // Builds a save_slot_data from the current player and checkpoint, ready
    // to write to SRAM. The actual SRAM read/write calls are Butano and
    // live in src/platform (see platform/save_io.h) — this stays in
    // game/ because it's plain data, no Butano involved.
    save_slot_data build_save_slot_data(const player_state& player, int checkpoint_room_index,
            fixed checkpoint_x, fixed checkpoint_y);

    // Restores a player_state's persistent stats from a save_slot_data.
    // Leaves transient/session-only fields (velocity, current action,
    // i-frames, combo state, ...) untouched — the caller is expected to be
    // starting a fresh player_state anyway. Doesn't touch the active room
    // or camera; the caller still needs level::load_room(slot.checkpoint_room_index).
    void apply_save_slot_data(const save_slot_data& slot, player_state& player);
}
