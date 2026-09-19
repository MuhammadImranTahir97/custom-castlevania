#pragma once

#include "difficulty.h"
#include "fixed.h"
#include "input.h"

namespace game
{
    constexpr int player_half_width = 8;
    constexpr int player_half_height = 8;

    enum class action_kind
    {
        none,
        attack,
        dodge
    };

    struct player_state
    {
        fixed x = 0;
        fixed y = 0;
        fixed velocity_x = 0;
        fixed velocity_y = 0;
        bool grounded = false;
        int frames_since_grounded = 0;
        int jump_buffer_frames = 0;
        bool prev_jump_held = false;

        int facing = 1; // +1 = right, -1 = left

        int mp = difficulty::player_max_mp;
        int mp_regen_counter = 0;
        int mp_regen_pause_frames = 0;

        action_kind action = action_kind::none;
        int action_timer = 0;
        bool dodge_has_iframes = false;

        bool prev_attack_held = false;
        bool prev_dodge_held = false;
    };

    struct attack_hitbox
    {
        bool active = false;
        fixed x = 0;
        fixed y = 0;
        fixed half_width = 0;
        fixed half_height = 0;
    };

    void init_player(player_state& player);
    void update_player(player_state& player, const input_state& input);

    attack_hitbox get_attack_hitbox(const player_state& player);
    bool player_is_invulnerable(const player_state& player);
}
