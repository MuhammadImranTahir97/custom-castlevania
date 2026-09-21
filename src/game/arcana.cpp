#include "arcana.h"

namespace game
{
    int arcana_mercury_damage_percent(arcana_attribute attribute)
    {
        switch(attribute)
        {
        case arcana_attribute::salamander:
            return difficulty::arcana_mercury_salamander_damage_percent;

        case arcana_attribute::serpent:
            return difficulty::arcana_mercury_serpent_damage_percent;

        case arcana_attribute::mandragora:
        default:
            return difficulty::arcana_mercury_mandragora_damage_percent;
        }
    }

    int arcana_boosted_attack_power(const player_state& player, int base_power)
    {
        if(! player.arcana_active || player.arcana.action != arcana_action::mercury)
        {
            return base_power;
        }

        int percent = arcana_mercury_damage_percent(player.arcana.attribute);
        return (base_power * (100 + percent)) / 100;
    }

    int arcana_diana_hit_power(int player_intelligence)
    {
        return (player_intelligence * difficulty::arcana_diana_damage_percent) / 100;
    }

    arcana_effect get_arcana_effect(arcana_action action, arcana_attribute attribute)
    {
        switch(attribute)
        {
        case arcana_attribute::salamander:
            return { difficulty::arcana_burn_duration_frames, 0, 0 };

        case arcana_attribute::serpent:
            return { 0, difficulty::arcana_slow_duration_frames, 0 };

        case arcana_attribute::mandragora:
        default:
            return { 0, 0, action == arcana_action::mercury
                    ? difficulty::arcana_mercury_mandragora_lifesteal_percent
                    : difficulty::arcana_diana_mandragora_lifesteal_percent };
        }
    }

    void update_player_arcana(player_state& player, bool arcana_pressed)
    {
        // Kept alive/updated regardless of the current loadout or action
        // state -- switching loadouts in the pause menu, or attacking/
        // dodging, mid-flight shouldn't freeze or teleport a shot already
        // in the air.
        update_arc_projectile(player.arcana_projectile, 0, difficulty::arcana_diana_projectile_lifetime_frames);

        if(arcana_pressed)
        {
            if(player.arcana.action == arcana_action::mercury)
            {
                // Toggle, not hold (locked by SPEC.md's control map).
                if(player.arcana_active)
                {
                    player.arcana_active = false;
                }
                else if(player.mp > 0)
                {
                    player.arcana_active = true;
                }
            }
            else if(player.mp >= difficulty::arcana_diana_mp_cost_per_use && ! player.arcana_projectile.active)
            {
                // Diana -- per-use, fire-and-forget straight shot.
                player.mp -= difficulty::arcana_diana_mp_cost_per_use;
                fixed velocity_x = player.facing * difficulty::arcana_diana_projectile_speed;
                launch_arc_projectile(player.arcana_projectile, player.x, player.y, velocity_x, 0);
            }
        }

        if(player.arcana.action != arcana_action::mercury || ! player.arcana_active)
        {
            player.arcana_mp_drain_counter = 0;
            return;
        }

        // Mercury's sustained drain -- runs every frame while toggled on,
        // independent of whether this is the press that turned it on.
        player.arcana_mp_drain_counter += difficulty::arcana_mercury_mp_cost_per_second;

        if(player.arcana_mp_drain_counter >= 60)
        {
            player.arcana_mp_drain_counter -= 60;
            --player.mp;

            if(player.mp <= 0)
            {
                player.mp = 0;
                player.arcana_active = false;
            }
        }
    }
}
