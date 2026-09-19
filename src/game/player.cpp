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
            }
            else if(input.right && !input.left)
            {
                player.velocity_x = difficulty::player_run_speed;
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
    }

    void init_player(player_state& player)
    {
        player.x = 0;
        player.y = grounded_y_at(player.x);
        player.grounded = true;
    }

    void update_player(player_state& player, const input_state& input)
    {
        apply_horizontal_input(player, input);
        apply_jump_buffer(player, input);
        apply_gravity(player);
        try_jump(player);
        apply_jump_cut(player, input);
        move_and_collide(player);

        player.prev_jump_held = input.jump_held;
    }
}
