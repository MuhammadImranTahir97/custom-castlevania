#include "enemy.h"

#include "difficulty.h"
#include "level.h"
#include "random.h"

namespace game
{
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
        // reference_y is the enemy's own y to check from (see
        // level::ground_top_y_at) -- typically its current position, so a
        // platform above it isn't mistaken for the ground it's standing on.
        fixed ground_y_for(fixed x, int half_height, fixed reference_y)
        {
            fixed reference_feet_y = reference_y + to_fixed(half_height);
            return level::ground_top_y_at(x, reference_feet_y) - to_fixed(half_height);
        }
    }

    // --- Skeleton — walker ---

    void init_skeleton(skeleton_state& skeleton, fixed spawn_x, fixed spawn_y)
    {
        skeleton.x = spawn_x;
        skeleton.y = ground_y_for(spawn_x, skeleton_half_height, spawn_y);
        skeleton.facing = -1;
        skeleton.hp = difficulty::skeleton_max_hp;
        skeleton.alive = true;
        skeleton.throw_timer = 0;
        skeleton.prev_attack_active = false;
        skeleton.bone = arc_projectile{};
        skeleton.status = arcana_status{};
    }

    void update_skeleton(skeleton_state& skeleton)
    {
        if(! skeleton.alive)
        {
            update_arc_projectile(skeleton.bone, difficulty::skeleton_bone_gravity,
                    difficulty::skeleton_bone_lifetime_frames);
            return;
        }

        fixed current_ground_y = ground_y_for(skeleton.x, skeleton_half_height, skeleton.y);
        fixed next_x = skeleton.x + (skeleton.facing * difficulty::skeleton_speed);
        fixed next_ground_y = ground_y_for(next_x, skeleton_half_height, skeleton.y);

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
        bat.status = arcana_status{};
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

    void init_archer(archer_state& archer, fixed spawn_x, fixed spawn_y)
    {
        archer.x = spawn_x;
        archer.y = ground_y_for(spawn_x, archer_half_height, spawn_y);
        archer.facing = -1;
        archer.hp = difficulty::archer_max_hp;
        archer.alive = true;
        archer.prev_attack_active = false;
        archer.shoot_timer = 0;
        archer.arrow = arc_projectile{};
        archer.status = arcana_status{};
    }

    void update_archer(archer_state& archer, fixed player_x)
    {
        if(! archer.alive)
        {
            update_arc_projectile(archer.arrow, difficulty::archer_arrow_gravity,
                    difficulty::archer_arrow_lifetime_frames);
            return;
        }

        archer.y = ground_y_for(archer.x, archer_half_height, archer.y);

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

    void init_zombie(zombie_state& zombie, fixed spawn_x, fixed spawn_y)
    {
        zombie.x = spawn_x;
        zombie.y = ground_y_for(spawn_x, zombie_half_height, spawn_y);
        zombie.velocity_y = 0;
        zombie.facing = -1;
        zombie.hp = difficulty::zombie_max_hp;
        zombie.alive = true;
        zombie.prev_attack_active = false;
        zombie.falling = false;
        zombie.status = arcana_status{};
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

        fixed next_ground_y = level::ground_top_y_at(next_x, zombie.y + to_fixed(zombie_half_height));
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
        pillar.status = arcana_status{};
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

    void init_fleaman(fleaman_state& fleaman, fixed spawn_x, fixed spawn_y)
    {
        fleaman.x = spawn_x;
        fleaman.y = ground_y_for(spawn_x, fleaman_half_height, spawn_y);
        fleaman.velocity_y = 0;
        fleaman.facing = -1;
        fleaman.hp = difficulty::fleaman_max_hp;
        fleaman.alive = true;
        fleaman.prev_attack_active = false;
        fleaman.grounded = true;
        fleaman.hop_timer = random::range(difficulty::fleaman_hop_interval_min_frames,
                difficulty::fleaman_hop_interval_max_frames);
        fleaman.status = arcana_status{};
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

        fixed old_y = fleaman.y;
        fleaman.y += fleaman.velocity_y;

        // Reference has to be old_y, not the just-updated fleaman.y -- see
        // the matching comment in player.cpp's move_and_collide.
        fixed ground_y = ground_y_for(fleaman.x, fleaman_half_height, old_y);

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
        medusa.status = arcana_status{};
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

    // --- Bone Colossus — first boss (rooms.md: catacombs_17) ---

    namespace
    {
        int bone_colossus_phase_for_hp(int hp, int max_hp)
        {
            if(hp * 100 <= max_hp * difficulty::bone_colossus_phase3_hp_percent)
            {
                return 3;
            }

            if(hp * 100 <= max_hp * difficulty::bone_colossus_phase2_hp_percent)
            {
                return 2;
            }

            return 1;
        }

        // Phase 3's "+25% attack speed" (rooms.md) shrinks recovery/cooldown
        // frame counts, never the locked windup tells — see difficulty.h's
        // comment on bone_colossus_phase3_speed_percent.
        int bone_colossus_speed_scaled(int frames, int phase)
        {
            if(phase >= 3)
            {
                return (frames * 100) / difficulty::bone_colossus_phase3_speed_percent;
            }

            return frames;
        }

        void bone_colossus_end_attack(bone_colossus_state& boss, int phase)
        {
            boss.attack = bone_colossus_attack::none;
            boss.attack_timer = 0;
            boss.attack_hit_applied = false;
            boss.decision_timer = bone_colossus_speed_scaled(
                    difficulty::bone_colossus_decision_cooldown_frames, phase);
        }
    }

    void init_bone_colossus(bone_colossus_state& boss, fixed spawn_x, fixed spawn_y)
    {
        boss.x = spawn_x;
        boss.y = spawn_y;
        boss.facing = -1;
        boss.hp = difficulty::bone_colossus_max_hp;
        boss.alive = true;
        boss.prev_attack_active = false;
        boss.attack = bone_colossus_attack::none;
        boss.attack_timer = 0;
        boss.decision_timer = difficulty::bone_colossus_decision_cooldown_frames;
        boss.attack_hit_applied = false;
        boss.summon_timer = difficulty::bone_colossus_summon_interval_frames;

        for(arc_projectile& rib : boss.ribs)
        {
            rib = arc_projectile{};
        }
    }

    bool update_bone_colossus(bone_colossus_state& boss, fixed player_x)
    {
        bool want_summon = false;

        if(boss.alive)
        {
            int phase = bone_colossus_phase_for_hp(boss.hp, difficulty::bone_colossus_max_hp);

            // Only turn to face the player while idle -- locking facing for
            // the duration of an attack keeps its tell honest (rib_spread
            // fires in boss.facing; flipping mid-windup would fire it
            // somewhere other than what the telegraph showed).
            if(boss.attack == bone_colossus_attack::none)
            {
                boss.facing = player_x >= boss.x ? 1 : -1;
            }

            // Phase 2+: periodic Skeleton adds, independent of the attack
            // state machine below (enemy_spawner.cpp acts on the request).
            if(phase >= 2)
            {
                if(boss.summon_timer > 0)
                {
                    --boss.summon_timer;
                }
                else
                {
                    boss.summon_timer = difficulty::bone_colossus_summon_interval_frames;
                    want_summon = true;
                }
            }

            if(boss.attack == bone_colossus_attack::none)
            {
                if(boss.decision_timer > 0)
                {
                    --boss.decision_timer;
                }
                else
                {
                    // Phase 1/2 alternate slam and sweep; phase 3 adds the rib
                    // spread. Random order so the pattern can't just be
                    // memorized by position (enemies.md only fixes *which*
                    // attacks exist per phase, not their sequence).
                    int choice_count = phase >= 3 ? 3 : 2;
                    int choice = random::range(0, choice_count - 1);

                    if(choice == 0)
                    {
                        boss.attack = bone_colossus_attack::slam;
                    }
                    else if(choice == 1)
                    {
                        boss.attack = bone_colossus_attack::sweep;
                    }
                    else
                    {
                        boss.attack = bone_colossus_attack::rib_spread;
                    }

                    boss.attack_timer = 0;
                    boss.attack_hit_applied = false;
                }
            }
            else if(boss.attack == bone_colossus_attack::slam)
            {
                int windup = difficulty::bone_colossus_slam_windup_frames;
                int active = bone_colossus_speed_scaled(difficulty::bone_colossus_slam_active_frames, phase);
                int recovery = bone_colossus_speed_scaled(difficulty::bone_colossus_slam_recovery_frames, phase);

                ++boss.attack_timer;

                if(boss.attack_timer >= windup + active + recovery)
                {
                    // Phase 2+: a 2-hit combo (rooms.md) -- the second hit
                    // gets its own fresh windup rather than ending here.
                    if(phase >= 2)
                    {
                        boss.attack = bone_colossus_attack::slam_second;
                        boss.attack_timer = 0;
                        boss.attack_hit_applied = false;
                    }
                    else
                    {
                        bone_colossus_end_attack(boss, phase);
                    }
                }
            }
            else if(boss.attack == bone_colossus_attack::slam_second)
            {
                int windup = difficulty::bone_colossus_slam2_windup_frames;
                int active = bone_colossus_speed_scaled(difficulty::bone_colossus_slam2_active_frames, phase);
                int recovery = bone_colossus_speed_scaled(difficulty::bone_colossus_slam2_recovery_frames, phase);

                ++boss.attack_timer;

                if(boss.attack_timer >= windup + active + recovery)
                {
                    bone_colossus_end_attack(boss, phase);
                }
            }
            else if(boss.attack == bone_colossus_attack::sweep)
            {
                int windup = difficulty::bone_colossus_sweep_windup_frames;
                int active = bone_colossus_speed_scaled(difficulty::bone_colossus_sweep_active_frames, phase);
                int recovery = bone_colossus_speed_scaled(difficulty::bone_colossus_sweep_recovery_frames, phase);

                ++boss.attack_timer;

                if(boss.attack_timer >= windup + active + recovery)
                {
                    bone_colossus_end_attack(boss, phase);
                }
            }
            else if(boss.attack == bone_colossus_attack::rib_spread)
            {
                int windup = difficulty::bone_colossus_ribs_windup_frames;

                ++boss.attack_timer;

                if(boss.attack_timer == windup)
                {
                    // Fired together, fanned by initial vertical velocity —
                    // gravity then spreads them further apart over their
                    // flight, opening the gaps rooms.md calls for ("must be
                    // positioned between them").
                    for(int i = 0; i < bone_colossus_rib_count; ++i)
                    {
                        int offset_index = i - bone_colossus_rib_count / 2; // -2..2
                        fixed vy = -to_fixed(2) + (offset_index * difficulty::bone_colossus_rib_spread_step);
                        launch_arc_projectile(boss.ribs[i], boss.x, boss.y,
                                boss.facing * difficulty::bone_colossus_rib_speed, vy);
                    }
                }

                if(boss.attack_timer >= windup)
                {
                    bone_colossus_end_attack(boss, phase);
                }
            }
        }

        for(arc_projectile& rib : boss.ribs)
        {
            update_arc_projectile(rib, difficulty::bone_colossus_rib_gravity,
                    difficulty::bone_colossus_rib_lifetime_frames);
        }

        return want_summon;
    }

    boss_attack_hitbox get_bone_colossus_attack_hitbox(const bone_colossus_state& boss)
    {
        boss_attack_hitbox box{};

        if(! boss.alive)
        {
            return box;
        }

        int phase = bone_colossus_phase_for_hp(boss.hp, difficulty::bone_colossus_max_hp);

        if(boss.attack == bone_colossus_attack::slam || boss.attack == bone_colossus_attack::slam_second)
        {
            bool is_second = boss.attack == bone_colossus_attack::slam_second;
            int windup = is_second ? difficulty::bone_colossus_slam2_windup_frames
                    : difficulty::bone_colossus_slam_windup_frames;
            int active_frames = is_second ? difficulty::bone_colossus_slam2_active_frames
                    : difficulty::bone_colossus_slam_active_frames;
            int active = bone_colossus_speed_scaled(active_frames, phase);

            if(boss.attack_timer >= windup && boss.attack_timer < windup + active)
            {
                box.active = true;
                box.unrollable = false;
                box.x = boss.x;
                box.y = boss.y;
                box.half_width = difficulty::bone_colossus_slam_half_width;
                box.half_height = difficulty::bone_colossus_slam_half_height;
                box.damage = difficulty::bone_colossus_slam_damage;
            }
        }
        else if(boss.attack == bone_colossus_attack::sweep)
        {
            int windup = difficulty::bone_colossus_sweep_windup_frames;
            int active = bone_colossus_speed_scaled(difficulty::bone_colossus_sweep_active_frames, phase);

            if(boss.attack_timer >= windup && boss.attack_timer < windup + active)
            {
                box.active = true;
                box.unrollable = true;
                box.x = boss.x;

                // A low band at the boss's feet, not centered on its (much
                // taller) body -- "must JUMP not roll" (rooms.md) needs the
                // band to actually sit near the ground.
                box.y = boss.y + to_fixed(bone_colossus_half_height)
                        - difficulty::bone_colossus_sweep_half_height;
                box.half_width = difficulty::bone_colossus_sweep_half_width;
                box.half_height = difficulty::bone_colossus_sweep_half_height;
                box.damage = difficulty::bone_colossus_sweep_damage;
            }
        }

        // rib_spread deals damage via the ribs[] projectiles directly (see
        // apply_bone_colossus_attack_to_player) rather than this hitbox.

        return box;
    }

    void apply_bone_colossus_attack_to_player(bone_colossus_state& boss, player_state& player)
    {
        boss_attack_hitbox box = get_bone_colossus_attack_hitbox(boss);

        if(box.active && ! boss.attack_hit_applied)
        {
            fixed player_half_w = to_fixed(player_half_width);
            fixed player_half_h = to_fixed(player_half_height);

            bool overlap_x = (box.x - box.half_width) < (player.x + player_half_w)
                    && (box.x + box.half_width) > (player.x - player_half_w);
            bool overlap_y = (box.y - box.half_height) < (player.y + player_half_h)
                    && (box.y + box.half_height) > (player.y - player_half_h);

            if(overlap_x && overlap_y)
            {
                if(box.unrollable)
                {
                    damage_player_unrollable(player, box.damage);
                }
                else
                {
                    damage_player(player, box.damage);
                }

                boss.attack_hit_applied = true;
            }
        }

        // Rib-cage bones (phase 3) damage on contact like any other arc
        // projectile, but unrollable -- only standing between them avoids
        // it (rooms.md).
        for(arc_projectile& rib : boss.ribs)
        {
            if(! rib.active)
            {
                continue;
            }

            fixed half_w = to_fixed(projectile_half_width);
            fixed half_h = to_fixed(projectile_half_height);
            fixed player_half_w = to_fixed(player_half_width);
            fixed player_half_h = to_fixed(player_half_height);

            bool overlap_x = (rib.x - half_w) < (player.x + player_half_w)
                    && (rib.x + half_w) > (player.x - player_half_w);
            bool overlap_y = (rib.y - half_h) < (player.y + player_half_h)
                    && (rib.y + half_h) > (player.y - player_half_h);

            if(overlap_x && overlap_y)
            {
                damage_player_unrollable(player, difficulty::bone_colossus_ribs_damage);
                rib.active = false;
            }
        }
    }
}
