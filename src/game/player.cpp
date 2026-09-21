#include "player.h"

#include "arcana.h"
#include "difficulty.h"
#include "level.h"

namespace game
{
    namespace
    {
        fixed min_x()
        {
            return to_fixed(-level::room_half_width() + player_half_width);
        }

        fixed max_x()
        {
            return to_fixed(level::room_half_width() - player_half_width);
        }

        // reference_y is the player's own y to check from (see
        // level::ground_top_y_at) -- typically their current position, so a
        // platform above them isn't mistaken for the ground they're on.
        fixed grounded_y_at(fixed x, fixed reference_y)
        {
            fixed reference_feet_y = reference_y + to_fixed(player_half_height);
            return level::ground_top_y_at(x, reference_feet_y) - to_fixed(player_half_height);
        }

        // --- Per-character movement tunables (SPEC.md section 3) ---

        struct movement_tunables
        {
            fixed run_speed;
            fixed gravity;
            fixed jump_velocity;
            fixed jump_cut_velocity;
            fixed dodge_velocity;
            int dodge_duration_frames;
            int dodge_iframe_start;
            int dodge_iframe_end;
        };

        movement_tunables tunables_for(character_kind character)
        {
            if(character == character_kind::rival)
            {
                return {
                    difficulty::rival_run_speed,
                    difficulty::rival_gravity,
                    difficulty::rival_jump_velocity,
                    difficulty::rival_jump_cut_velocity,
                    difficulty::rival_dodge_velocity,
                    difficulty::rival_dodge_duration_frames,
                    difficulty::rival_dodge_iframe_start,
                    difficulty::rival_dodge_iframe_end,
                };
            }

            return {
                difficulty::player_run_speed,
                difficulty::player_gravity,
                difficulty::player_jump_velocity,
                difficulty::player_jump_cut_velocity,
                difficulty::player_dodge_velocity,
                difficulty::player_dodge_duration_frames,
                difficulty::player_dodge_iframe_start,
                difficulty::player_dodge_iframe_end,
            };
        }

        // --- Per-character attack tunables ---

        struct attack_tunables
        {
            int startup_frames;
            int active_frames;
            int recovery_frames;
            fixed range;
            fixed hitbox_half_width;
            fixed hitbox_half_height;
        };

        attack_tunables attack_tunables_for(character_kind character)
        {
            if(character == character_kind::rival)
            {
                return {
                    difficulty::rival_attack_startup_frames,
                    difficulty::rival_attack_active_frames,
                    difficulty::rival_attack_recovery_frames,
                    difficulty::rival_attack_range,
                    difficulty::rival_attack_hitbox_half_width,
                    difficulty::rival_attack_hitbox_half_height,
                };
            }

            return {
                difficulty::player_attack_startup_frames,
                difficulty::player_attack_active_frames,
                difficulty::player_attack_recovery_frames,
                difficulty::player_attack_range,
                difficulty::player_attack_hitbox_half_width,
                difficulty::player_attack_hitbox_half_height,
            };
        }

        void apply_horizontal_input(player_state& player, const input_state& input, const movement_tunables& t)
        {
            if(input.left && !input.right)
            {
                player.velocity_x = -t.run_speed;
                player.facing = -1;
            }
            else if(input.right && !input.left)
            {
                player.velocity_x = t.run_speed;
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

        void apply_gravity(player_state& player, const movement_tunables& t)
        {
            player.velocity_y += t.gravity;

            if(player.velocity_y > difficulty::player_max_fall_speed)
            {
                player.velocity_y = difficulty::player_max_fall_speed;
            }
        }

        void try_jump(player_state& player, const movement_tunables& t)
        {
            bool can_coyote_jump = player.frames_since_grounded <= difficulty::player_coyote_frames;

            if(player.jump_buffer_frames <= 0)
            {
                return;
            }

            if(player.grounded || can_coyote_jump)
            {
                player.velocity_y = t.jump_velocity;
                player.grounded = false;
                player.jump_buffer_frames = 0;
                player.frames_since_grounded = difficulty::player_coyote_frames + 1;
                return;
            }

            // Double -- one extra mid-air jump, recharged on landing (see
            // move_and_collide). Same jump_velocity as the first jump, from
            // wherever the player currently is, not added to existing
            // velocity -- a fresh jump arc, not a boost.
            if(player.has_double_jump && ! player.used_double_jump)
            {
                player.velocity_y = t.jump_velocity;
                player.used_double_jump = true;
                player.jump_buffer_frames = 0;
            }
        }

        void apply_jump_cut(player_state& player, const input_state& input, const movement_tunables& t)
        {
            bool jump_released = ! input.jump_held && player.prev_jump_held;

            if(jump_released && player.velocity_y < t.jump_cut_velocity)
            {
                player.velocity_y = t.jump_cut_velocity;
            }
        }

        void move_and_collide(player_state& player)
        {
            fixed old_y = player.y;
            fixed new_x = player.x + player.velocity_x;

            if(new_x < min_x())
            {
                new_x = min_x();
            }
            else if(new_x > max_x())
            {
                new_x = max_x();
            }

            // A platform taller than where the player currently is blocks
            // horizontal movement like a wall, rather than being an
            // automatic step up onto it. Without this, walking into the
            // side of any platform — regardless of height — instantly
            // climbs it, which defeats jump-height-gated progression
            // (SPEC.md's relics: higher jumps reach higher ledges).
            //
            // grounded_y_at can't answer this: a platform taller than the
            // player is exactly what it excludes (it isn't ground the
            // player is standing on), so it reports the open column below
            // as if the taller platform weren't there at all, and the
            // player would just walk straight through it. Checking whether
            // a closer overhead obstacle exists at new_x than at the
            // player's current x is the actual wall test.
            fixed old_feet_y = old_y + to_fixed(player_half_height);
            fixed old_overhead = level::lowest_overhead_top_at(player.x, old_feet_y);
            fixed new_overhead = level::lowest_overhead_top_at(new_x, old_feet_y);

            if(new_overhead > old_overhead)
            {
                new_x = player.x;
            }

            player.x = new_x;
            player.y += player.velocity_y;

            // The reference has to be old_y, not the just-updated player.y:
            // gravity always moves the player slightly past a surface
            // before this check runs (that's what "player.y >= ground_y"
            // below is for), so using the post-move position here would
            // exclude the very platform being landed on as "already passed"
            // — every single frame, which is a permanent fall, not a landing.
            fixed ground_y = grounded_y_at(player.x, old_y);

            if(player.y >= ground_y)
            {
                player.y = ground_y;
                player.velocity_y = 0;
                player.grounded = true;
                player.frames_since_grounded = 0;
                player.used_double_jump = false;
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

        void update_invuln(player_state& player)
        {
            if(player.invuln_frames > 0)
            {
                --player.invuln_frames;
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

            if(player.mp >= player.max_mp)
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

            if(player.character == character_kind::rival)
            {
                if(player.combo_reset_timer > difficulty::rival_combo_window_frames)
                {
                    player.combo_step = 0;
                }
                else
                {
                    player.combo_step = (player.combo_step + 1) % 3;
                }

                player.combo_reset_timer = 0;
            }
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
            movement_tunables t = tunables_for(player.character);

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

            apply_horizontal_input(player, input, t);
            apply_jump_buffer(player, input);
            apply_gravity(player, t);
            try_jump(player, t);
            apply_jump_cut(player, input, t);
            move_and_collide(player);
        }

        void update_attack(player_state& player, const input_state& input, bool /*attack_pressed*/, bool dodge_pressed)
        {
            movement_tunables mt = tunables_for(player.character);
            attack_tunables at = attack_tunables_for(player.character);

            // Buffered here too, not just in update_normal -- otherwise a
            // jump pressed during startup/active frames has no effect
            // (recovery's cancel check below only sees a same-frame press,
            // not one held from earlier in the attack) and is silently
            // dropped instead of firing the moment recovery begins.
            apply_jump_buffer(player, input);

            int recovery_start = at.startup_frames + at.active_frames;
            int total_frames = recovery_start + at.recovery_frames;

            bool in_recovery = player.action_timer >= recovery_start;

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
                apply_horizontal_input(player, input, mt);
                apply_gravity(player, mt);
                move_and_collide(player);
                return;
            }

            if(in_recovery && player.jump_buffer_frames > 0)
            {
                player.action = action_kind::none;
                player.velocity_x = 0;
                apply_gravity(player, mt);
                try_jump(player, mt);
                move_and_collide(player);
                return;
            }

            player.velocity_x = 0;
            apply_gravity(player, mt);
            move_and_collide(player);
            ++player.action_timer;

            if(player.action_timer >= total_frames)
            {
                player.action = action_kind::none;
            }
        }

        void update_dodge(player_state& player, const input_state& /*input*/, bool attack_pressed, bool /*dodge_pressed*/)
        {
            movement_tunables t = tunables_for(player.character);

            player.velocity_x = player.facing * t.dodge_velocity;
            player.x += player.velocity_x;

            if(player.x < min_x())
            {
                player.x = min_x();
            }
            else if(player.x > max_x())
            {
                player.x = max_x();
            }

            // A roll/dash hugs the ground it crosses rather than falling mid-way.
            player.y = grounded_y_at(player.x, player.y);
            player.velocity_y = 0;
            player.grounded = true;
            player.frames_since_grounded = 0;

            ++player.action_timer;

            bool can_cancel_to_attack = player.action_timer >= t.dodge_iframe_end;

            if(can_cancel_to_attack && attack_pressed)
            {
                start_attack(player);
                return;
            }

            if(player.action_timer >= t.dodge_duration_frames)
            {
                player.action = action_kind::none;
                player.velocity_x = 0;
            }
        }
    }

    void init_player(player_state& player, fixed spawn_x, fixed spawn_y)
    {
        player.x = spawn_x;
        player.y = spawn_y;
        player.velocity_x = 0;
        player.velocity_y = 0;
        player.grounded = true;
    }

    void update_player(player_state& player, const input_state& input)
    {
        bool attack_pressed = input.attack_held && ! player.prev_attack_held;
        bool dodge_pressed = input.dodge_held && ! player.prev_dodge_held;
        bool swap_pressed = input.swap_held && ! player.prev_swap_held;

        // SPEC.md's control map: up+attack is reserved for a future
        // subweapon system -- not built yet, so it's withheld here rather
        // than falling through to a whip swing, leaving the input clean for
        // whatever calls it later. down+attack is Arcana (arcana_pressed).
        // Attack alone, with neither held, is the ordinary whip.
        bool arcana_pressed = attack_pressed && input.down_held;
        bool whip_attack_pressed = attack_pressed && ! input.up_held && ! input.down_held;

        if(swap_pressed)
        {
            player.character = player.character == character_kind::hunter
                    ? character_kind::rival : character_kind::hunter;

            // Instant and free, but the two movesets' timings don't line up,
            // so any in-progress attack/dodge is cut short rather than
            // continued with the other character's numbers.
            player.action = action_kind::none;
            player.combo_step = 0;
        }

        switch(player.action)
        {

        case action_kind::attack:
            update_attack(player, input, whip_attack_pressed, dodge_pressed);
            break;

        case action_kind::dodge:
            update_dodge(player, input, whip_attack_pressed, dodge_pressed);
            break;

        case action_kind::none:
        default:
            update_normal(player, input, whip_attack_pressed, dodge_pressed);
            break;
        }

        if(player.action != action_kind::attack)
        {
            ++player.combo_reset_timer;
        }

        update_mp_regen(player);
        update_invuln(player);
        update_player_arcana(player, arcana_pressed);

        player.prev_jump_held = input.jump_held;
        player.prev_attack_held = input.attack_held;
        player.prev_dodge_held = input.dodge_held;
        player.prev_swap_held = input.swap_held;
    }

    attack_hitbox get_attack_hitbox(const player_state& player)
    {
        attack_hitbox box{};

        if(player.action != action_kind::attack)
        {
            return box;
        }

        attack_tunables at = attack_tunables_for(player.character);
        int active_start = at.startup_frames;
        int active_end = active_start + at.active_frames;

        if(player.action_timer >= active_start && player.action_timer < active_end)
        {
            box.active = true;
            box.x = player.x + (player.facing * at.range);
            box.y = player.y;
            box.half_width = at.hitbox_half_width;
            box.half_height = at.hitbox_half_height;
        }

        return box;
    }

    int current_attack_power(const player_state& player)
    {
        if(player.character == character_kind::rival)
        {
            return (player.str * difficulty::rival_damage_percent) / 100;
        }

        return player.str;
    }

    bool player_is_invulnerable(const player_state& player)
    {
        if(player.invuln_frames > 0)
        {
            return true;
        }

        if(player.action != action_kind::dodge || ! player.dodge_has_iframes)
        {
            return false;
        }

        movement_tunables t = tunables_for(player.character);
        return player.action_timer >= t.dodge_iframe_start && player.action_timer <= t.dodge_iframe_end;
    }

    int apply_defense(int raw_damage, int defense)
    {
        int damage = (raw_damage * 100) / (100 + defense);

        if(damage < 1)
        {
            damage = 1;
        }

        return damage;
    }

    namespace
    {
        void apply_damage_and_iframes(player_state& player, int raw_damage)
        {
            int damage = apply_defense(raw_damage, player.def);

            player.hp -= damage;

            if(player.hp < 0)
            {
                player.hp = 0;
            }

            player.invuln_frames = difficulty::player_invuln_frames;
        }
    }

    void damage_player(player_state& player, int raw_damage)
    {
        if(player_is_invulnerable(player))
        {
            return;
        }

        apply_damage_and_iframes(player, raw_damage);
    }

    void damage_player_unrollable(player_state& player, int raw_damage)
    {
        if(player.invuln_frames > 0)
        {
            return;
        }

        apply_damage_and_iframes(player, raw_damage);
    }

    void grant_exp(player_state& player, int amount)
    {
        player.exp += amount;

        while(player.exp >= difficulty::exp_for_next_level(player.level))
        {
            player.exp -= difficulty::exp_for_next_level(player.level);
            ++player.level;

            player.max_hp += difficulty::player_hp_per_level;
            player.max_mp += difficulty::player_mp_per_level;
            player.str += difficulty::player_str_per_level;
            player.def += difficulty::player_def_per_level;
            player.intelligence += difficulty::player_int_per_level;
            player.lck += difficulty::player_lck_per_level;
        }
    }

    void restore_full(player_state& player)
    {
        player.hp = player.max_hp;
        player.mp = player.max_mp;
    }
}
