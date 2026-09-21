#pragma once

// The small, dependency-free Arcana types (SPEC.md section 4 / arcana.md).
// Split out of arcana.h so player.h can hold an arcana_loadout and enemy.h
// can hold an arcana_status without either including arcana.h itself --
// arcana.h includes player.h (for player_state in its function signatures),
// so player.h including arcana.h back would cycle.

namespace game
{
    // M3 ships 2 Action x 3 Attribute = 6 of the full 10x10 grid
    // (arcana.md). The rest come later, past M3.
    enum class arcana_action
    {
        mercury, // weapon enhancer, per-second
        diana,   // weapon projectile, per-use
    };

    enum class arcana_attribute
    {
        salamander, // fire
        serpent,    // ice
        mandragora, // plant
    };

    struct arcana_loadout
    {
        arcana_action action = arcana_action::mercury;
        arcana_attribute attribute = arcana_attribute::salamander;
    };

    // A status effect an Arcana hit can leave on whatever it hits. Embedded
    // as a plain member in each enemy_spawner.cpp entity type (enemy.h),
    // the same way prev_attack_active already is.
    struct arcana_status
    {
        int burn_frames = 0;
        int burn_damage_per_tick = 0;
        int burn_tick_timer = 0;
        int slow_frames = 0;
    };
}
