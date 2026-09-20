#pragma once

namespace game::random
{
    // Deterministic xorshift32 — good enough for "random-ish" enemy timing,
    // not for anything security- or fairness-sensitive. Same seed every
    // boot (no hardware RNG source is wired in), which is fine here: it
    // just needs to not look mechanical to the player.

    // Returns the next pseudo-random value in the sequence.
    unsigned int next();

    // Returns a pseudo-random integer in [min, max] inclusive.
    int range(int min, int max);
}
