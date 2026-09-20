#include "enemy.h"

#include "difficulty.h"
#include "level.h"
#include "random.h"

namespace game
{
    void launch_arc_projectile(arc_projectile& proj, fixed x, fixed y, fixed velocity_x, fixed velocity_y)
    {
        proj.active = true;
        proj.x = x;
        proj.y = y;
        proj.velocity_x = velocity_x;
        proj.velocity_y = velocity_y;
        proj.lifetime_frames = 0;
    }

    void update_arc_projectile(arc_projectile& proj, fixed gravity, int lifetime_limit)
    {
        if(! proj.active)
        {
            return;
        }

        proj.velocity_y += gravity;
        proj.x += proj.velocity_x;
        proj.y += proj.velocity_y;
        ++proj.lifetime_frames;

        if(proj.lifetime_frames >= lifetime_limit)
        {
            proj.active = false;
        }
    }

    void apply_projectile_contact(arc_projectile& proj, int half_width, int half_height, int damage,
            player_state& player)
    {
        if(! proj.active)
        {
            return;
        }

        fixed half_w = to_fixed(half_width);
        fixed half_h = to_fixed(half_height);
        fixed player_half_w = to_fixed(player_half_width);
        fixed player_half_h = to_fixed(player_half_height);

        bool overlap_x = (proj.x - half_w) < (player.x + player_half_w)
                && (proj.x + half_w) > (player.x - player_half_w);
        bool overlap_y = (proj.y - half_h) < (player.y + player_half_h)
                && (proj.y + half_h) > (player.y - player_half_h);

        if(overlap_x && overlap_y)
        {
            damage_player(player, damage);
            proj.active = false;
        }
    }

    namespace
    {
        fixed ground_y_for(fixed x, int half_height)
        {
            return level::ground_top_y_at(x) - to_fixed(half_height);
        }
    }

    // --- Skeleton — walker ---

    void init_skeleton(skeleton_state& skeleton, fixed spawn_x)
    {
        skeleton.x = spawn_x;
        skeleton.y = ground_y_for(spawn_x, skeleton_half_height);
        skeleton.facing = -1;
        skeleton.hp = difficulty::skeleton_max_hp;
        skeleton.alive = true;
        skeleton.throw_timer = 0;
        skeleton.prev_attack_active = false;
        skeleton.bone = arc_projectile{};
    }

    void update_skeleton(skeleton_state& skeleton)
    {
        if(! skeleton.alive)
        {
            update_arc_projectile(skeleton.bone, difficulty::skeleton_bone_gravity,
                    difficulty::skeleton_bone_lifetime_frames);
            return;
        }

        fixed current_ground_y = ground_y_for(skeleton.x, skeleton_half_height);
        fixed next_x = skeleton.x + (skeleton.facing * difficulty::skeleton_speed);
        fixed next_ground_y = ground_y_for(next_x, skeleton_half_height);

        fixed min_x = to_fixed(-level::room_half_width() + skeleton_half_width);
        fixed max_x = to_fixed(level::room_half_width() - skeleton_half_width);

        bool would_leave_room = next_x < min_x || next_x > max_x;
        bool would_step_or_fall = next_ground_y != current_ground_y;

        if(would_leave_room || would_step_or_fall)
        {
            // Walker: turns at edges and walls rather than stepping off.
            skeleton.facing = -skeleton.facing;
        }
        else
        {
            skeleton.x = next_x;
            skeleton.y = current_ground_y;
        }

        ++skeleton.throw_timer;

        if(skeleton.throw_timer >= difficulty::skeleton_bone_throw_interval_frames)
        {
            skeleton.throw_timer = 0;
            launch_arc_projectile(skeleton.bone, skeleton.x, skeleton.y,
                    skeleton.facing * difficulty::skeleton_bone_speed_x, difficulty::skeleton_bone_speed_y);
        }

        update_arc_projectile(skeleton.bone, difficulty::skeleton_bone_gravity,
                difficulty::skeleton_bone_lifetime_frames);
    }

    // --- Bat — flyer ---

    void init_bat(bat_state& bat, fixed spawn_x, fixed spawn_y)
    {
        bat.x = spawn_x;
        bat.y = spawn_y;
        bat.facing = -1;
        bat.hp = difficulty::bat_max_hp;
        bat.alive = true;
        bat.prev_attack_active = false;
        bat.triggered = false;
        bat.swoop_timer = 0;
    }

    void update_bat(bat_state& bat, fixed player_x, fixed player_y)
    {
        if(! bat.alive)
        {
            return;
        }

        fixed dx = player_x - bat.x;
        fixed dy = player_y - bat.y;
        fixed abs_dx = dx < 0 ? -dx : dx;
        fixed abs_dy = dy < 0 ? -dy : dy;

        if(! bat.triggered && abs_dx <= difficulty::bat_trigger_range && abs_dy <= difficulty::bat_trigger_range)
        {
            bat.triggered = true;
        }

        if(! bat.triggered)
        {
            return; // ignores gravity, idles in place until triggered
        }

        bat.facing = dx >= 0 ? 1 : -1;
        bat.x += bat.facing * difficulty::bat_speed;

        ++bat.swoop_timer;

        bool wiggle_down = (bat.swoop_timer / difficulty::bat_wiggle_period_frames) % 2 == 0;
        bat.y += wiggle_down ? difficulty::bat_wiggle_speed : -difficulty::bat_wiggle_speed;

        // A flyer that leaves the room is lost forever, so it stays within
        // the room's full bounds (which may be bigger than one screen).
        fixed min_x = to_fixed(-level::room_half_width() + bat_half_width);
        fixed max_x = to_fixed(level::room_half_width() - bat_half_width);
        fixed min_y = to_fixed(-level::room_half_height() + bat_half_height);
        fixed max_y = to_fixed(level::room_half_height() - bat_half_height);

        if(bat.x < min_x)
        {
            bat.x = min_x;
        }
        else if(bat.x > max_x)
        {
            bat.x = max_x;
        }

        if(bat.y < min_y)
        {
            bat.y = min_y;
        }
        else if(bat.y > max_y)
        {
            bat.y = max_y;
        }
    }

    // --- Skeleton Archer — shooter ---

    void init_archer(archer_state& archer, fixed spawn_x)
    {
        archer.x = spawn_x;
        archer.y = ground_y_for(spawn_x, archer_half_height);
        archer.facing = -1;
        archer.hp = difficulty::archer_max_hp;
        archer.alive = true;
        archer.prev_attack_active = false;
        archer.shoot_timer = 0;
        archer.arrow = arc_projectile{};
    }

    void update_archer(archer_state& archer, fixed player_x)
    {
        if(! archer.alive)
        {
            update_arc_projectile(archer.arrow, difficulty::archer_arrow_gravity,
                    difficulty::archer_arrow_lifetime_frames);
            return;
        }

        archer.y = ground_y_for(archer.x, archer_half_height);

        fixed dx = player_x - archer.x;
        archer.facing = dx >= 0 ? 1 : -1;

        fixed abs_dx = dx < 0 ? -dx : dx;

        if(abs_dx < difficulty::archer_retreat_range)
        {
            fixed move = dx >= 0 ? -difficulty::archer_speed : difficulty::archer_speed;
            fixed next_x = archer.x + move;
            fixed min_x = to_fixed(-level::room_half_width() + archer_half_width);
            fixed max_x = to_fixed(level::room_half_width() - archer_half_width);

            if(next_x >= min_x && next_x <= max_x)
            {
                archer.x = next_x;
            }
        }

        ++archer.shoot_timer;

        if(archer.shoot_timer >= difficulty::archer_shoot_interval_frames)
        {
            archer.shoot_timer = 0;
            launch_arc_projectile(archer.arrow, archer.x, archer.y,
                    archer.facing * difficulty::archer_arrow_speed_x, difficulty::archer_arrow_speed_y);
        }

        update_arc_projectile(archer.arrow, difficulty::archer_arrow_gravity,
                difficulty::archer_arrow_lifetime_frames);
    }

    // --- Zombie — walker, doesn't turn at ledges ---

    void init_zombie(zombie_state& zombie, fixed spawn_x)
    {
        zombie.x = spawn_x;
        zombie.y = ground_y_for(spawn_x, zombie_half_height);
        zombie.velocity_y = 0;
        zombie.facing = -1;
        zombie.hp = difficulty::zombie_max_hp;
        zombie.alive = true;
        zombie.prev_attack_active = false;
        zombie.falling = false;
    }

    void update_zombie(zombie_state& zombie)
    {
        if(! zombie.alive)
        {
            return;
        }

        if(zombie.falling)
        {
            zombie.velocity_y += difficulty::zombie_fall_gravity;
            zombie.y += zombie.velocity_y;

            fixed fall_limit = to_fixed(level::room_half_height() + 40);

            if(zombie.y > fall_limit)
            {
                zombie.alive = false; // gone — into the pit
            }

            return;
        }

        fixed next_x = zombie.x + (zombie.facing * difficulty::zombie_speed);

        fixed min_x = to_fixed(-level::room_half_width() + zombie_half_width);
        fixed max_x = to_fixed(level::room_half_width() - zombie_half_width);

        if(next_x < min_x || next_x > max_x)
        {
            // Still turns at walls/screen edges — only ledges are ignored.
            zombie.facing = -zombie.facing;
            return;
        }

        fixed next_ground_y = level::ground_top_y_at(next_x);
        zombie.x = next_x;

        if(next_ground_y >= level::no_ground_y)
        {
            zombie.falling = true;
            zombie.velocity_y = 0;
        }
        else
        {
            zombie.y = next_ground_y - to_fixed(zombie_half_height);
        }
    }

    // --- Bone Pillar — stationary, fires straight down its authored facing ---

    void init_bone_pillar(bone_pillar_state& pillar, fixed spawn_x, fixed spawn_y, int facing)
    {
        pillar.x = spawn_x;
        pillar.y = spawn_y;
        pillar.facing = facing;
        pillar.hp = difficulty::bone_pillar_max_hp;
        pillar.alive = true;
        pillar.prev_attack_active = false;
        pillar.fire_timer = 0;
        pillar.fireball = arc_projectile{};
    }

    void update_bone_pillar(bone_pillar_state& pillar)
    {
        if(pillar.alive)
        {
            ++pillar.fire_timer;

            if(pillar.fire_timer >= difficulty::bone_pillar_fire_interval_frames)
            {
                pillar.fire_timer = 0;
                launch_arc_projectile(pillar.fireball, pillar.x, pillar.y,
                        pillar.facing * difficulty::bone_pillar_fireball_speed, 0);
            }
        }

        // Zero gravity — a straight horizontal shot down the corridor, not an arc.
        update_arc_projectile(pillar.fireball, 0, difficulty::bone_pillar_fireball_lifetime_frames);
    }

    // --- Fleaman — jumper, random hop height/direction while grounded ---

    void init_fleaman(fleaman_state& fleaman, fixed spawn_x)
    {
        fleaman.x = spawn_x;
        fleaman.y = ground_y_for(spawn_x, fleaman_half_height);
        fleaman.velocity_y = 0;
        fleaman.facing = -1;
        fleaman.hp = difficulty::fleaman_max_hp;
        fleaman.alive = true;
        fleaman.prev_attack_active = false;
        fleaman.grounded = true;
        fleaman.hop_timer = random::range(difficulty::fleaman_hop_interval_min_frames,
                difficulty::fleaman_hop_interval_max_frames);
    }

    void update_fleaman(fleaman_state& fleaman)
    {
        if(! fleaman.alive)
        {
            return;
        }

        if(fleaman.grounded)
        {
            if(fleaman.hop_timer > 0)
            {
                --fleaman.hop_timer;
                return;
            }

            fleaman.facing = random::range(0, 1) == 0 ? -1 : 1;
            fleaman.velocity_y = -to_fixed(random::range(2, 4));
            fleaman.grounded = false;
            return;
        }

        fleaman.velocity_y += difficulty::fleaman_gravity;
        fleaman.x += fleaman.facing * difficulty::fleaman_speed;

        fixed min_x = to_fixed(-level::room_half_width() + fleaman_half_width);
        fixed max_x = to_fixed(level::room_half_width() - fleaman_half_width);

        if(fleaman.x < min_x)
        {
            fleaman.x = min_x;
            fleaman.facing = -fleaman.facing;
        }
        else if(fleaman.x > max_x)
        {
            fleaman.x = max_x;
            fleaman.facing = -fleaman.facing;
        }

        fleaman.y += fleaman.velocity_y;
        fixed ground_y = ground_y_for(fleaman.x, fleaman_half_height);

        if(fleaman.y >= ground_y)
        {
            fleaman.y = ground_y;
            fleaman.velocity_y = 0;
            fleaman.grounded = true;
            fleaman.hop_timer = random::range(difficulty::fleaman_hop_interval_min_frames,
                    difficulty::fleaman_hop_interval_max_frames);
        }
    }

    // --- Medusa Head — flies its authored facing direction in a wave,
    //     respawns at its spawn edge after death or reaching the far wall ---

    void init_medusa_head(medusa_head_state& medusa, fixed spawn_x, fixed spawn_y, int facing)
    {
        medusa.x = spawn_x;
        medusa.y = spawn_y;
        medusa.origin_x = spawn_x;
        medusa.origin_y = spawn_y;
        medusa.facing = facing;
        medusa.hp = difficulty::medusa_head_max_hp;
        medusa.alive = true;
        medusa.prev_attack_active = false;
        medusa.wave_timer = 0;
        medusa.respawn_timer = 0;
    }

    void update_medusa_head(medusa_head_state& medusa)
    {
        if(! medusa.alive)
        {
            if(medusa.respawn_timer > 0)
            {
                --medusa.respawn_timer;
                return;
            }

            medusa.x = medusa.origin_x;
            medusa.y = medusa.origin_y;
            medusa.hp = difficulty::medusa_head_max_hp;
            medusa.wave_timer = 0;
            medusa.alive = true;
            return;
        }

        medusa.x += medusa.facing * difficulty::medusa_head_speed;
        ++medusa.wave_timer;

        int period = difficulty::medusa_head_wave_period_frames;
        int phase = medusa.wave_timer % (period * 2);
        fixed amplitude = difficulty::medusa_head_wave_amplitude;
        fixed offset;

        if(phase < period)
        {
            offset = -amplitude + (amplitude * 2 * phase) / period;
        }
        else
        {
            int p2 = phase - period;
            offset = amplitude - (amplitude * 2 * p2) / period;
        }

        medusa.y = medusa.origin_y + offset;

        fixed bound = to_fixed(level::room_half_width() + medusa_head_half_width);

        if(medusa.x > bound || medusa.x < -bound)
        {
            medusa.alive = false;
            medusa.respawn_timer = difficulty::medusa_head_respawn_cooldown_frames;
        }
    }
}
