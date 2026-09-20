#include "random.h"

namespace game::random
{
    namespace
    {
        unsigned int state = 0x9E3779B9u; // any non-zero seed
    }

    unsigned int next()
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }

    int range(int min, int max)
    {
        if(max <= min)
        {
            return min;
        }

        unsigned int span = static_cast<unsigned int>(max - min + 1);
        return min + static_cast<int>(next() % span);
    }
}
