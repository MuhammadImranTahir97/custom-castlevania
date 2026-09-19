#include "enemy.h"

#include "difficulty.h"
#include "level.h"

namespace game
{
    namespace
    {
        fixed ground_y_for(fixed x)
        {
            return level::ground_top_y_at(x) - to_fixed(skeleton_half_height);
        }

        void throw_bone(skeleton_state& skeleton)
        {
            skeleton.bone.active = true;
            skeleton.bone.x = skeleton.x;
            skeleton.bone.y = skeleton.y;
            skeleton.bone.velocity_x = skeleton.facing * difficulty::skeleton_bone_speed_x;
            skeleton.bone.velocity_y = difficulty::skeleton_bone_speed_y;
            skeleton.bone.lifetime_frames = 0;
        }

        void update_bone(bone_projectile& bone)
        {
            if(! bone.active)
            {
                return;
            }

            bone.velocity_y += difficulty::skeleton_bone_gravity;
            bone.x += bone.velocity_x;
            bone.y += bone.velocity_y;
            ++bone.lifetime_frames;

            if(bone.lifetime_frames >= difficulty::skeleton_bone_lifetime_frames)
            {
                bone.active = false;
            }
        }

        void patrol(skeleton_state& skeleton)
        {
            fixed current_ground_y = ground_y_for(skeleton.x);
            fixed next_x = skeleton.x + (skeleton.facing * difficulty::skeleton_speed);
            fixed next_ground_y = ground_y_for(next_x);

            fixed min_x = to_fixed(-level::screen_half_width + skeleton_half_width);
            fixed max_x = to_fixed(level::screen_half_width - skeleton_half_width);

            bool would_leave_screen = next_x < min_x || next_x > max_x;
            bool would_step_or_fall = next_ground_y != current_ground_y;

            if(would_leave_screen || would_step_or_fall)
            {
                // Walker: turns at edges and walls rather than stepping off.
                skeleton.facing = -skeleton.facing;
            }
            else
            {
                skeleton.x = next_x;
                skeleton.y = current_ground_y;
            }
        }
    }

    void init_skeleton(skeleton_state& skeleton, fixed spawn_x)
    {
        skeleton.x = spawn_x;
        skeleton.y = ground_y_for(spawn_x);
        skeleton.facing = -1;
        skeleton.hp = difficulty::skeleton_max_hp;
        skeleton.alive = true;
        skeleton.throw_timer = 0;
        skeleton.prev_attack_active = false;
        skeleton.bone = bone_projectile{};
    }

    void update_skeleton(skeleton_state& skeleton)
    {
        if(! skeleton.alive)
        {
            update_bone(skeleton.bone);
            return;
        }

        patrol(skeleton);

        ++skeleton.throw_timer;

        if(skeleton.throw_timer >= difficulty::skeleton_bone_throw_interval_frames)
        {
            skeleton.throw_timer = 0;
            throw_bone(skeleton);
        }

        update_bone(skeleton.bone);
    }

    void apply_attack_to_skeleton(skeleton_state& skeleton, const attack_hitbox& hitbox)
    {
        bool attack_just_started = hitbox.active && ! skeleton.prev_attack_active;
        skeleton.prev_attack_active = hitbox.active;

        if(! skeleton.alive || ! attack_just_started)
        {
            return;
        }

        fixed skel_half_w = to_fixed(skeleton_half_width);
        fixed skel_half_h = to_fixed(skeleton_half_height);

        bool overlap_x = (hitbox.x - hitbox.half_width) < (skeleton.x + skel_half_w)
                && (hitbox.x + hitbox.half_width) > (skeleton.x - skel_half_w);
        bool overlap_y = (hitbox.y - hitbox.half_height) < (skeleton.y + skel_half_h)
                && (hitbox.y + hitbox.half_height) > (skeleton.y - skel_half_h);

        if(overlap_x && overlap_y)
        {
            int damage = difficulty::player_attack_damage - difficulty::skeleton_defense;

            if(damage < 1)
            {
                damage = 1;
            }

            skeleton.hp -= damage;

            if(skeleton.hp <= 0)
            {
                skeleton.hp = 0;
                skeleton.alive = false;
            }
        }
    }
}
