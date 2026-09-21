#include "world.h"

#include "difficulty.h"
#include "level.h"

namespace game
{
    namespace
    {
        // Must match tools/convert_rooms.py's RELIC_TYPE_IDS.
        constexpr int relic_type_double_jump = 0;

        // Must match tools/convert_rooms.py's UNLOCK_TYPE_IDS.
        constexpr int unlock_type_rival = 0;

        checkpoint_info current_checkpoint;

        void respawn(player_state& player)
        {
            level::load_room(current_checkpoint.room_index);
            player.x = current_checkpoint.x;
            player.y = current_checkpoint.y;
            player.velocity_x = 0;
            player.velocity_y = 0;
            player.grounded = true;
            player.frames_since_grounded = 0;
            player.action = action_kind::none;
            restore_full(player);
            player.invuln_frames = difficulty::player_invuln_frames; // brief grace period
        }
    }

    void set_checkpoint(int room_index, fixed x, fixed y)
    {
        current_checkpoint.room_index = room_index;
        current_checkpoint.x = x;
        current_checkpoint.y = y;
    }

    checkpoint_info current_checkpoint_info()
    {
        return current_checkpoint;
    }

    void update_world(player_state& player)
    {
        // A pit with nothing underneath it (e.g. catacombs_02) has no floor
        // to catch a missed jump — falling out of the room counts as a death.
        bool fell_out_of_bounds = player.y > to_fixed(level::room_half_height() + 40);

        // No permadeath, no lost items (SPEC.md) — just send the player back
        // to their last save room, fully healed. Death/respawn is otherwise
        // out of scope here (no lost-progress tracking, no death animation).
        if(player.hp <= 0 || fell_out_of_bounds)
        {
            respawn(player);
            return;
        }

        // Relics are placed in the world, never RNG (SPEC.md section 4's
        // card-placement rule, applied the same way to relics). Not
        // suppressed/consumed like a door crossing -- picking it up again
        // on a later visit is harmless (has_double_jump already true stays
        // true), matching enemy_spawner.h's "rooms respawn fresh" scope.
        int relic_type = 0;

        if(level::try_collect_relic(player.x, player.y,
                to_fixed(player_half_width), to_fixed(player_half_height), relic_type)
                && relic_type == relic_type_double_jump)
        {
            player.has_double_jump = true;
        }

        // The Rival, found trapped (SPEC.md section 7's Shape, step 4) --
        // same touch-trigger shape as a relic pickup, but flips a story
        // flag (rival_unlocked) instead of granting an item.
        int unlock_type = 0;

        if(level::try_collect_unlock(player.x, player.y,
                to_fixed(player_half_width), to_fixed(player_half_height), unlock_type)
                && unlock_type == unlock_type_rival)
        {
            player.rival_unlocked = true;
        }

        level::spawn_point target{};

        if(level::try_cross_door(player.x, player.y,
                to_fixed(player_half_width), to_fixed(player_half_height), target))
        {
            player.x = target.x;
            player.y = target.y;
            player.velocity_x = 0;
            player.velocity_y = 0;
            player.grounded = true;
            player.frames_since_grounded = 0;
        }

        if(level::current_room_is_save_room())
        {
            level::spawn_point room_spawn = level::current_room_spawn_point();
            set_checkpoint(level::current_room_index(), room_spawn.x, room_spawn.y);
            restore_full(player);
        }

        level::update_camera(player.x, player.y);
    }
}
