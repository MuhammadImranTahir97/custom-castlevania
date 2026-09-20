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
        constexpr int enemy_type_zombie = 3;
        constexpr int enemy_type_bone_pillar = 4;
        constexpr int enemy_type_fleaman = 5;
        constexpr int enemy_type_medusa_head = 6;
        constexpr int enemy_type_bone_colossus = 7;

        skeleton_state skeletons[max_enemies_per_type];
        int skeleton_active_count = 0;

        bat_state bats[max_enemies_per_type];
        int bat_active_count = 0;

        archer_state archers[max_enemies_per_type];
        int archer_active_count = 0;

        zombie_state zombies[max_enemies_per_type];
        int zombie_active_count = 0;

        bone_pillar_state bone_pillars[max_enemies_per_type];
        int bone_pillar_active_count = 0;

        fleaman_state fleamen[max_enemies_per_type];
        int fleaman_active_count = 0;

        medusa_head_state medusa_heads[max_enemies_per_type];
        int medusa_head_active_count = 0;

        bone_colossus_state bone_colossus;
        bool bone_colossus_active = false;

        // See bone_colossus_defeated -- deliberately NOT reset by
        // spawn_room_enemies, unlike bone_colossus_active above.
        bool bone_colossus_ever_defeated = false;
    }

    void spawn_room_enemies()
    {
        skeleton_active_count = 0;
        bat_active_count = 0;
        archer_active_count = 0;
        zombie_active_count = 0;
        bone_pillar_active_count = 0;
        fleaman_active_count = 0;
        medusa_head_active_count = 0;
        bone_colossus_active = false;

        int count = level::room_enemy_spawn_count();

        for(int i = 0; i < count; ++i)
        {
            level::enemy_spawn_view spawn = level::room_enemy_spawn(i);

            switch(spawn.type)
            {
            case enemy_type_skeleton:
                if(skeleton_active_count < max_enemies_per_type)
                {
                    init_skeleton(skeletons[skeleton_active_count], to_fixed(spawn.x), to_fixed(spawn.y));
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
                    init_archer(archers[archer_active_count], to_fixed(spawn.x), to_fixed(spawn.y));
                    ++archer_active_count;
                }
                break;

            case enemy_type_zombie:
                if(zombie_active_count < max_enemies_per_type)
                {
                    init_zombie(zombies[zombie_active_count], to_fixed(spawn.x), to_fixed(spawn.y));
                    ++zombie_active_count;
                }
                break;

            case enemy_type_bone_pillar:
                if(bone_pillar_active_count < max_enemies_per_type)
                {
                    init_bone_pillar(bone_pillars[bone_pillar_active_count], to_fixed(spawn.x),
                            to_fixed(spawn.y), spawn.facing);
                    ++bone_pillar_active_count;
                }
                break;

            case enemy_type_fleaman:
                if(fleaman_active_count < max_enemies_per_type)
                {
                    init_fleaman(fleamen[fleaman_active_count], to_fixed(spawn.x), to_fixed(spawn.y));
                    ++fleaman_active_count;
                }
                break;

            case enemy_type_medusa_head:
                if(medusa_head_active_count < max_enemies_per_type)
                {
                    init_medusa_head(medusa_heads[medusa_head_active_count], to_fixed(spawn.x),
                            to_fixed(spawn.y), spawn.facing);
                    ++medusa_head_active_count;
                }
                break;

            case enemy_type_bone_colossus:
                init_bone_colossus(bone_colossus, to_fixed(spawn.x), to_fixed(spawn.y));
                bone_colossus_active = true;
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

        for(int i = 0; i < zombie_active_count; ++i)
        {
            update_zombie(zombies[i]);
        }

        for(int i = 0; i < bone_pillar_active_count; ++i)
        {
            update_bone_pillar(bone_pillars[i]);
        }

        for(int i = 0; i < fleaman_active_count; ++i)
        {
            update_fleaman(fleamen[i]);
        }

        for(int i = 0; i < medusa_head_active_count; ++i)
        {
            update_medusa_head(medusa_heads[i]);
        }

        if(bone_colossus_active)
        {
            bool want_summon = update_bone_colossus(bone_colossus, player_x);

            if(want_summon)
            {
                // Adds enter from both edges of the wide arena (rooms.md:
                // "summons 2 Skeletons every 15 seconds") -- exact position
                // isn't specified, first draft.
                fixed edge_x = to_fixed(level::room_half_width() - skeleton_half_width - 8);

                if(skeleton_active_count < max_enemies_per_type)
                {
                    init_skeleton(skeletons[skeleton_active_count], -edge_x, bone_colossus.y);
                    ++skeleton_active_count;
                }

                if(skeleton_active_count < max_enemies_per_type)
                {
                    init_skeleton(skeletons[skeleton_active_count], edge_x, bone_colossus.y);
                    ++skeleton_active_count;
                }
            }
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

        for(int i = 0; i < zombie_active_count; ++i)
        {
            if(apply_whip_to_enemy(zombies[i], zombie_half_width, zombie_half_height,
                    difficulty::zombie_defense, power, hitbox))
            {
                grant_exp(player, difficulty::zombie_exp_reward);
            }
        }

        for(int i = 0; i < bone_pillar_active_count; ++i)
        {
            if(apply_whip_to_enemy(bone_pillars[i], bone_pillar_half_width, bone_pillar_half_height,
                    difficulty::bone_pillar_defense, power, hitbox))
            {
                grant_exp(player, difficulty::bone_pillar_exp_reward);
            }
        }

        for(int i = 0; i < fleaman_active_count; ++i)
        {
            if(apply_whip_to_enemy(fleamen[i], fleaman_half_width, fleaman_half_height,
                    difficulty::fleaman_defense, power, hitbox))
            {
                grant_exp(player, difficulty::fleaman_exp_reward);
            }
        }

        for(int i = 0; i < medusa_head_active_count; ++i)
        {
            if(apply_whip_to_enemy(medusa_heads[i], medusa_head_half_width, medusa_head_half_height,
                    difficulty::medusa_head_defense, power, hitbox))
            {
                grant_exp(player, difficulty::medusa_head_exp_reward);

                // Continuous spawner (enemies.md) — a kill isn't permanent,
                // it just resets to its spawn edge after this cooldown.
                medusa_heads[i].respawn_timer = difficulty::medusa_head_respawn_cooldown_frames;
            }
        }

        if(bone_colossus_active)
        {
            if(apply_whip_to_enemy(bone_colossus, bone_colossus_half_width, bone_colossus_half_height,
                    difficulty::bone_colossus_defense, power, hitbox))
            {
                grant_exp(player, difficulty::bone_colossus_exp_reward);
                bone_colossus_ever_defeated = true;
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

        for(int i = 0; i < zombie_active_count; ++i)
        {
            apply_enemy_contact(zombies[i], zombie_half_width, zombie_half_height,
                    difficulty::zombie_damage, player);
        }

        for(int i = 0; i < bone_pillar_active_count; ++i)
        {
            apply_enemy_contact(bone_pillars[i], bone_pillar_half_width, bone_pillar_half_height,
                    difficulty::bone_pillar_damage, player);
            apply_projectile_contact(bone_pillars[i].fireball, projectile_half_width, projectile_half_height,
                    difficulty::bone_pillar_damage, player);
        }

        for(int i = 0; i < fleaman_active_count; ++i)
        {
            apply_enemy_contact(fleamen[i], fleaman_half_width, fleaman_half_height,
                    difficulty::fleaman_damage, player);
        }

        for(int i = 0; i < medusa_head_active_count; ++i)
        {
            apply_enemy_contact(medusa_heads[i], medusa_head_half_width, medusa_head_half_height,
                    difficulty::medusa_head_damage, player);
        }

        // No passive body-contact damage for the Colossus itself (unlike
        // the enemies above) — enemies.md's boss design rules are built
        // entirely around telegraphed attacks having a tell, so damage only
        // comes from its named attacks, not from standing near its body.
        if(bone_colossus_active)
        {
            apply_bone_colossus_attack_to_player(bone_colossus, player);
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

    int active_zombie_count()
    {
        return zombie_active_count;
    }

    const zombie_state& active_zombie(int index)
    {
        return zombies[index];
    }

    int active_bone_pillar_count()
    {
        return bone_pillar_active_count;
    }

    const bone_pillar_state& active_bone_pillar(int index)
    {
        return bone_pillars[index];
    }

    int active_fleaman_count()
    {
        return fleaman_active_count;
    }

    const fleaman_state& active_fleaman(int index)
    {
        return fleamen[index];
    }

    int active_medusa_head_count()
    {
        return medusa_head_active_count;
    }

    const medusa_head_state& active_medusa_head(int index)
    {
        return medusa_heads[index];
    }

    bool bone_colossus_is_active()
    {
        return bone_colossus_active;
    }

    const bone_colossus_state& active_bone_colossus()
    {
        return bone_colossus;
    }

    void sync_boss_room_seal()
    {
        level::set_room_sealed(bone_colossus_active && bone_colossus.alive);
    }

    bool bone_colossus_defeated()
    {
        return bone_colossus_ever_defeated;
    }
}
