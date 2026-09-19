#include "player.h"

#include "difficulty.h"
#include "level.h"

namespace game
{
    namespace
    {
        constexpr fixed min_x = to_fixed(-level::screen_half_width + player_half_width);
        constexpr fixed max_x = to_fixed(level::screen_half_width - player_half_width);

        fixed grounded_y_at(fixed x)
        {
            return level::ground_top_y_at(x) - to_fixed(player_half_height);
        }

        void apply_horizontal_input(player_state& player, const input_state& input)
        {
            if(input.left && !input.right)
            {
                player.velocity_x = -difficulty::player_run_speed;
                player.facing = -1;
            }
            else if(input.right && !input.left)
            {
                player.velocity_x = difficulty::player_run_speed;
                player.facing = 1;
            }
            else
            {
                player.velocity_x = 0;
            }
        }

        void apply_jump_buffer(player_state& player, const input_state& input)
        {
            bool jump_pressed = input.jump_held && ! player.prev_jump_held;

            if(jump_pressed)
            {
                player.jump_buffer_frames = difficulty::player_input_buffer_frames;
            }
            else if(player.jump_buffer_frames > 0)
            {
                --player.jump_buffer_frames;
            }
        }

        void apply_gravity(player_state& player)
        {
            player.velocity_y += difficulty::player_gravity;

            if(player.velocity_y > difficulty::player_max_fall_speed)
            {
                player.velocity_y = difficulty::player_max_fall_speed;
            }
        }

        void try_jump(player_state& player)
        {
            bool can_coyote_jump = player.frames_since_grounded <= difficulty::player_coyote_frames;

            if(player.jump_buffer_frames > 0 && (player.grounded || can_coyote_jump))
            {
                player.velocity_y = difficulty::player_jump_velocity;
                player.grounded = false;
                player.jump_buffer_frames = 0;
                player.frames_since_grounded = difficulty::player_coyote_frames + 1;
            }
        }

        void apply_jump_cut(player_state& player, const input_state& input)
        {
            bool jump_released = ! input.jump_held && player.prev_jump_held;

            if(jump_released && player.velocity_y < difficulty::player_jump_cut_velocity)
            {
                player.velocity_y = difficulty::player_jump_cut_velocity;
            }
        }

        void move_and_collide(player_state& player)
        {
            player.x += player.velocity_x;
            player.y += player.velocity_y;

            if(player.x < min_x)
            {
                player.x = min_x;
            }
            else if(player.x > max_x)
            {
                player.x = max_x;
            }

            fixed ground_y = grounded_y_at(player.x);

            if(player.y >= ground_y)
            {
                player.y = ground_y;
                player.velocity_y = 0;
                player.grounded = true;
                player.frames_since_grounded = 0;
            }
            else
            {
                player.grounded = false;

                if(player.frames_since_grounded < 999)
                {
                    ++player.frames_since_grounded;
                }
            }
        }

        void update_mp_regen(player_state& player)
        {
            if(player.mp_regen_pause_frames > 0)
            {
                --player.mp_regen_pause_frames;
                player.mp_regen_counter = 0;
                return;
            }

            if(player.mp >= difficulty::player_max_mp)
            {
                return;
            }

            ++player.mp_regen_counter;

            if(player.mp_regen_counter >= difficulty::player_mp_regen_frames)
            {
                player.mp_regen_counter = 0;
                ++player.mp;
            }
        }

        void start_attack(player_state& player)
        {
            player.action = action_kind::attack;
            player.action_timer = 0;
        }

        void start_dodge(player_state& player)
        {
            player.action = action_kind::dodge;
            player.action_timer = 0;

            if(player.mp >= difficulty::player_dodge_mp_cost)
            {
                player.mp -= difficulty::player_dodge_mp_cost;
                player.dodge_has_iframes = true;
            }
            else
            {
                player.dodge_has_iframes = false;
            }

            player.mp_regen_pause_frames = difficulty::player_dodge_regen_pause_frames;
        }

        void update_normal(player_state& player, const input_state& input, bool attack_pressed, bool dodge_pressed)
        {
            if(dodge_pressed && player.grounded)
            {
                start_dodge(player);
                return;
            }

            if(attack_pressed)
            {
                start_attack(player);
                return;
            }

            apply_horizontal_input(player, input);
            apply_jump_buffer(player, input);
            apply_gravity(player);
            try_jump(player);
            apply_jump_cut(player, input);
            move_and_collide(player);
        }

        void update_attack(player_state& player, const input_state& input, bool /*attack_pressed*/, bool dodge_pressed)
        {
            constexpr int recovery_start = difficulty::player_attack_startup_frames
                    + difficulty::player_attack_active_frames;
            constexpr int total_frames = recovery_start + difficulty::player_attack_recovery_frames;

            bool in_recovery = player.action_timer >= recovery_start;
            bool jump_pressed_edge = input.jump_held && ! player.prev_jump_held;

            // Attack cancel: recovery frames let you move, jump or dodge immediately.
            if(in_recovery && dodge_pressed && player.grounded)
            {
                player.action = action_kind::none;
                start_dodge(player);
                return;
            }

            if(in_recovery && (input.left || input.right))
            {
                player.action = action_kind::none;
                apply_horizontal_input(player, input);
                apply_gravity(player);
                move_and_collide(player);
                return;
            }

            if(in_recovery && jump_pressed_edge)
            {
                player.action = action_kind::none;
                player.velocity_x = 0;
                player.jump_buffer_frames = difficulty::player_input_buffer_frames;
                apply_gravity(player);
                try_jump(player);
                move_and_collide(player);
                return;
            }

            player.velocity_x = 0;
            apply_gravity(player);
            move_and_collide(player);
            ++player.action_timer;

            if(player.action_timer >= total_frames)
            {
                player.action = action_kind::none;
            }
        }

        void update_dodge(player_state& player, const input_state& /*input*/, bool attack_pressed, bool /*dodge_pressed*/)
        {
            player.velocity_x = player.facing * difficulty::player_dodge_velocity;
            player.x += player.velocity_x;

            if(player.x < min_x)
            {
                player.x = min_x;
            }
            else if(player.x > max_x)
            {
                player.x = max_x;
            }

            // A roll hugs the ground it crosses rather than falling mid-roll.
            player.y = grounded_y_at(player.x);
            player.velocity_y = 0;
            player.grounded = true;
            player.frames_since_grounded = 0;

            ++player.action_timer;

            bool can_cancel_to_attack = player.action_timer >= difficulty::player_dodge_iframe_end;

            if(can_cancel_to_attack && attack_pressed)
            {
                start_attack(player);
                return;
            }

            if(player.action_timer >= difficulty::player_dodge_duration_frames)
            {
                player.action = action_kind::none;
                player.velocity_x = 0;
            }
        }
    }

    void init_player(player_state& player)
    {
        player.x = 0;
        player.y = grounded_y_at(player.x);
        player.grounded = true;
    }

    void update_player(player_state& player, const input_state& input)
    {
        bool attack_pressed = input.attack_held && ! player.prev_attack_held;
        bool dodge_pressed = input.dodge_held && ! player.prev_dodge_held;

        switch(player.action)
        {

        case action_kind::attack:
            update_attack(player, input, attack_pressed, dodge_pressed);
            break;

        case action_kind::dodge:
            update_dodge(player, input, attack_pressed, dodge_pressed);
            break;

        case action_kind::none:
        default:
            update_normal(player, input, attack_pressed, dodge_pressed);
            break;
        }

        update_mp_regen(player);

        player.prev_jump_held = input.jump_held;
        player.prev_attack_held = input.attack_held;
        player.prev_dodge_held = input.dodge_held;
    }

    attack_hitbox get_attack_hitbox(const player_state& player)
    {
        attack_hitbox box{};

        if(player.action == action_kind::attack)
        {
            int active_start = difficulty::player_attack_startup_frames;
            int active_end = active_start + difficulty::player_attack_active_frames;

            if(player.action_timer >= active_start && player.action_timer < active_end)
            {
                box.active = true;
                box.x = player.x + (player.facing * difficulty::player_attack_range);
                box.y = player.y;
                box.half_width = difficulty::player_attack_hitbox_half_width;
                box.half_height = difficulty::player_attack_hitbox_half_height;
            }
        }

        return box;
    }

    bool player_is_invulnerable(const player_state& player)
    {
        if(player.action != action_kind::dodge || ! player.dodge_has_iframes)
        {
            return false;
        }

        return player.action_timer >= difficulty::player_dodge_iframe_start
                && player.action_timer <= difficulty::player_dodge_iframe_end;
    }
}
