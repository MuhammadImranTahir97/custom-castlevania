#include "enemy_spawner.h"

#include "difficulty.h"
#include "level.h"

namespace game
{
    namespace
    {
        // Type ids, matching tools/convert_rooms.py's ENEMY_TYPE_IDS.
        constexpr int enemy_type_skeleton = 0;
        constexpr int enemy_type_bat = 1;
        constexpr int enemy_type_archer = 2;

        skeleton_state skeletons[max_enemies_per_type];
        int skeleton_active_count = 0;

        bat_state bats[max_enemies_per_type];
        int bat_active_count = 0;

        archer_state archers[max_enemies_per_type];
        int archer_active_count = 0;
    }

    void spawn_room_enemies()
    {
        skeleton_active_count = 0;
        bat_active_count = 0;
        archer_active_count = 0;

        int count = level::room_enemy_spawn_count();

        for(int i = 0; i < count; ++i)
        {
            level::enemy_spawn_view spawn = level::room_enemy_spawn(i);

            switch(spawn.type)
            {
            case enemy_type_skeleton:
                if(skeleton_active_count < max_enemies_per_type)
                {
                    init_skeleton(skeletons[skeleton_active_count], to_fixed(spawn.x));
                    ++skeleton_active_count;
                }
                break;

            case enemy_type_bat:
                if(bat_active_count < max_enemies_per_type)
                {
                    init_bat(bats[bat_active_count], to_fixed(spawn.x), to_fixed(spawn.y));
                    ++bat_active_count;
                }
                break;

            case enemy_type_archer:
                if(archer_active_count < max_enemies_per_type)
                {
                    init_archer(archers[archer_active_count], to_fixed(spawn.x));
                    ++archer_active_count;
                }
                break;

            default:
                break;
            }
        }
    }

    void update_enemies(fixed player_x, fixed player_y)
    {
        for(int i = 0; i < skeleton_active_count; ++i)
        {
            update_skeleton(skeletons[i]);
        }

        for(int i = 0; i < bat_active_count; ++i)
        {
            update_bat(bats[i], player_x, player_y);
        }

        for(int i = 0; i < archer_active_count; ++i)
        {
            update_archer(archers[i], player_x);
        }
    }

    void apply_attacks_to_enemies(const attack_hitbox& hitbox, player_state& player)
    {
        int power = current_attack_power(player);

        for(int i = 0; i < skeleton_active_count; ++i)
        {
            if(apply_whip_to_enemy(skeletons[i], skeleton_half_width, skeleton_half_height,
                    difficulty::skeleton_defense, power, hitbox))
            {
                grant_exp(player, difficulty::skeleton_exp_reward);
            }
        }

        for(int i = 0; i < bat_active_count; ++i)
        {
            if(apply_whip_to_enemy(bats[i], bat_half_width, bat_half_height, difficulty::bat_defense, power, hitbox))
            {
                grant_exp(player, difficulty::bat_exp_reward);
            }
        }

        for(int i = 0; i < archer_active_count; ++i)
        {
            if(apply_whip_to_enemy(archers[i], archer_half_width, archer_half_height,
                    difficulty::archer_defense, power, hitbox))
            {
                grant_exp(player, difficulty::archer_exp_reward);
            }
        }
    }

    void apply_enemy_contact_to_player(player_state& player)
    {
        for(int i = 0; i < skeleton_active_count; ++i)
        {
            apply_enemy_contact(skeletons[i], skeleton_half_width, skeleton_half_height,
                    difficulty::skeleton_damage, player);
            apply_projectile_contact(skeletons[i].bone, projectile_half_width, projectile_half_height,
                    difficulty::skeleton_damage, player);
        }

        for(int i = 0; i < bat_active_count; ++i)
        {
            apply_enemy_contact(bats[i], bat_half_width, bat_half_height, difficulty::bat_damage, player);
        }

        for(int i = 0; i < archer_active_count; ++i)
        {
            apply_enemy_contact(archers[i], archer_half_width, archer_half_height,
                    difficulty::archer_damage, player);
            apply_projectile_contact(archers[i].arrow, projectile_half_width, projectile_half_height,
                    difficulty::archer_damage, player);
        }
    }

    int active_skeleton_count()
    {
        return skeleton_active_count;
    }

    const skeleton_state& active_skeleton(int index)
    {
        return skeletons[index];
    }

    int active_bat_count()
    {
        return bat_active_count;
    }

    const bat_state& active_bat(int index)
    {
        return bats[index];
    }

    int active_archer_count()
    {
        return archer_active_count;
    }

    const archer_state& active_archer(int index)
    {
        return archers[index];
    }
}
