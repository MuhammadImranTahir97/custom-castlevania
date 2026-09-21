#pragma once

#include "arcana_types.h"
#include "difficulty.h"
#include "fixed.h"
#include "player.h"
#include "projectile.h"

// The Arcana System (SPEC.md section 4). M3 ships 2 Action cards (Mercury,
// Diana) x 3 Attribute cards (Salamander, Serpent, Mandragora) = 6 of the
// full 10x10 grid in arcana.md; the rest come later, past M3.
//
// The status-effect pieces below (arcana_effect, apply_arcana_hit,
// tick_arcana_status, arcana_is_slow_skip_frame, apply_arcana_projectile_to_
// enemy) are templated on EnemyState the same way enemy.h's
// apply_whip_to_enemy already is, so every enemy_spawner.cpp entity type
// gets Arcana support from one shared implementation rather than eight
// near-duplicate ones. Deliberately NOT wired up for the Bone Colossus
// (enemy.h's bone_colossus_state has no arcana_status member) -- arcana.md's
// balance rule "no combo may trivialise a boss" means its already-tuned
// fight stays untouched by burn/slow for now.

namespace game
{
    // What a combo does to whatever it hits, independent of which Action
    // card fired it. Mercury's own weapon-damage bonus (applied to the
    // whip's attack_power before the hit lands, not after) is looked up
    // separately via arcana_mercury_damage_percent -- it isn't "on hit",
    // it's already baked into the damage a hit computes.
    struct arcana_effect
    {
        int burn_frames = 0;
        int slow_frames = 0;
        int lifesteal_percent = 0;
    };

    // Mercury's weapon-damage bonus for the given attribute, as a percent
    // (0 = no change). Applied to current_attack_power before a whip hit is
    // resolved.
    int arcana_mercury_damage_percent(arcana_attribute attribute);

    // base_power boosted by Mercury if it's the player's active, affordable
    // sustained effect this frame; unchanged otherwise. enemy_spawner.cpp
    // calls this once per frame in place of a bare current_attack_power().
    int arcana_boosted_attack_power(const player_state& player, int base_power);

    // Diana's direct hit power for the given INT, before defense.
    int arcana_diana_hit_power(int player_intelligence);

    // Advances the player's Arcana state for this frame. arcana_pressed is
    // the down+attack edge (SPEC.md's control map) -- Mercury toggles on/off
    // on that press, Diana fires on it; either way it's a tap, never a hold.
    // Ticks Mercury's per-second MP drain and Diana's in-flight projectile
    // every frame regardless of arcana_pressed. Call once per frame from
    // update_player. Loadout (which combo is equipped) isn't set here -- see
    // player.arcana's own comment.
    void update_player_arcana(player_state& player, bool arcana_pressed);

    // Combo effect (burn/slow/lifesteal) for the given action+attribute.
    arcana_effect get_arcana_effect(arcana_action action, arcana_attribute attribute);

    // Applies effect to whatever a Mercury-enhanced whip hit or a Diana
    // projectile just landed on: sets its burn/slow status, and heals
    // `player` for effect.lifesteal_percent of damage_dealt if set.
    template<typename EnemyState>
    void apply_arcana_hit(EnemyState& enemy, const arcana_effect& effect, int damage_dealt,
            int player_intelligence, player_state& player)
    {
        if(effect.burn_frames > 0)
        {
            enemy.status.burn_frames = effect.burn_frames;
            enemy.status.burn_tick_timer = 0;

            int tick_damage = (player_intelligence * difficulty::arcana_burn_tick_damage_percent) / 100;
            enemy.status.burn_damage_per_tick = tick_damage < 1 ? 1 : tick_damage;
        }

        if(effect.slow_frames > 0)
        {
            enemy.status.slow_frames = effect.slow_frames;
        }

        if(effect.lifesteal_percent > 0)
        {
            player.hp += (damage_dealt * effect.lifesteal_percent) / 100;

            if(player.hp > player.max_hp)
            {
                player.hp = player.max_hp;
            }
        }
    }

    // Ticks burn/slow down by one frame and applies burn tick damage.
    // Returns true if this tick's burn damage is what killed the enemy (so
    // the caller can grant EXP exactly once, the same as a whip kill).
    template<typename EnemyState>
    bool tick_arcana_status(EnemyState& enemy)
    {
        if(! enemy.alive)
        {
            return false;
        }

        if(enemy.status.slow_frames > 0)
        {
            --enemy.status.slow_frames;
        }

        if(enemy.status.burn_frames <= 0)
        {
            return false;
        }

        --enemy.status.burn_frames;
        ++enemy.status.burn_tick_timer;

        if(enemy.status.burn_tick_timer < difficulty::arcana_burn_tick_interval_frames)
        {
            return false;
        }

        enemy.status.burn_tick_timer = 0;
        enemy.hp -= enemy.status.burn_damage_per_tick;

        if(enemy.hp <= 0)
        {
            enemy.hp = 0;
            enemy.alive = false;
            return true;
        }

        return false;
    }

    // True while `enemy`'s own update_X this frame should be skipped -- a
    // type-agnostic ~50% slow (arcana.md's Serpent rule) achieved by
    // halving how often an entity's update runs, rather than editing every
    // enemy type's own movement code to consult a slow multiplier.
    template<typename EnemyState>
    bool arcana_is_slow_skip_frame(const EnemyState& enemy)
    {
        return enemy.status.slow_frames > 0 && (enemy.status.slow_frames % 2) == 0;
    }

    // Diana's projectile hitting an enemy with .x, .y, .hp, .alive fields
    // (the same contract apply_whip_to_enemy uses) -- consumes the
    // projectile on the first enemy it overlaps. Returns true if this hit
    // killed it.
    template<typename EnemyState>
    bool apply_arcana_projectile_to_enemy(EnemyState& enemy, int half_width, int half_height, int defense,
            int hit_power, arc_projectile& proj, int* out_damage_dealt = nullptr)
    {
        if(! proj.active || ! enemy.alive)
        {
            return false;
        }

        fixed half_w = to_fixed(half_width);
        fixed half_h = to_fixed(half_height);
        fixed proj_half_w = to_fixed(difficulty::arcana_diana_projectile_half_width);
        fixed proj_half_h = to_fixed(difficulty::arcana_diana_projectile_half_height);

        bool overlap_x = (proj.x - proj_half_w) < (enemy.x + half_w) && (proj.x + proj_half_w) > (enemy.x - half_w);
        bool overlap_y = (proj.y - proj_half_h) < (enemy.y + half_h) && (proj.y + proj_half_h) > (enemy.y - half_h);

        if(! (overlap_x && overlap_y))
        {
            return false;
        }

        proj.active = false;

        int damage = apply_defense(hit_power, defense);

        if(out_damage_dealt)
        {
            *out_damage_dealt = damage;
        }

        enemy.hp -= damage;

        if(enemy.hp <= 0)
        {
            enemy.hp = 0;
            enemy.alive = false;
            return true;
        }

        return false;
    }
}
