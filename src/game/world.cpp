#include "world.h"

#include "difficulty.h"
#include "level.h"

namespace game
{
    namespace
    {
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
