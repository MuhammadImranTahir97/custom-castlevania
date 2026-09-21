#include "arcana.h"

namespace game
{
    void cycle_arcana_loadout(arcana_loadout& loadout)
    {
        if(loadout.attribute == arcana_attribute::mandragora)
        {
            loadout.attribute = arcana_attribute::salamander;
            loadout.action = loadout.action == arcana_action::mercury
                    ? arcana_action::diana : arcana_action::mercury;
        }
        else if(loadout.attribute == arcana_attribute::salamander)
        {
            loadout.attribute = arcana_attribute::serpent;
        }
        else
        {
            loadout.attribute = arcana_attribute::mandragora;
        }
    }

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

    void update_player_arcana(player_state& player, const input_state& input)
    {
        bool cycle_pressed = input.arcana_cycle_held && ! player.prev_arcana_cycle_held;
        player.prev_arcana_cycle_held = input.arcana_cycle_held;

        if(cycle_pressed)
        {
            cycle_arcana_loadout(player.arcana);
        }

        bool cast_pressed = input.arcana_cast_held && ! player.prev_arcana_cast_held;
        player.prev_arcana_cast_held = input.arcana_cast_held;

        // Kept alive/updated regardless of the current loadout -- switching
        // away from Diana mid-flight (cycle_pressed above) shouldn't freeze
        // or teleport a shot already in the air.
        update_arc_projectile(player.arcana_projectile, 0, difficulty::arcana_diana_projectile_lifetime_frames);

        if(player.arcana.action == arcana_action::mercury)
        {
            player.arcana_active = input.arcana_cast_held && player.mp > 0;

            if(player.arcana_active)
            {
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
            else
            {
                player.arcana_mp_drain_counter = 0;
            }

            return;
        }

        // Diana -- per-use, fire-and-forget straight shot.
        player.arcana_active = false;

        if(cast_pressed && player.mp >= difficulty::arcana_diana_mp_cost_per_use
                && ! player.arcana_projectile.active)
        {
            player.mp -= difficulty::arcana_diana_mp_cost_per_use;
            fixed velocity_x = player.facing * difficulty::arcana_diana_projectile_speed;
            launch_arc_projectile(player.arcana_projectile, player.x, player.y, velocity_x, 0);
        }
    }
}
