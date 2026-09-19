#include "level.h"

namespace game::level
{
    namespace
    {
        struct platform
        {
            fixed left_x;
            fixed right_x;
            fixed top_y;
        };

        constexpr fixed no_ground_y = to_fixed(10000);

        constexpr platform platforms[] = {
            { to_fixed(-screen_half_width), to_fixed(screen_half_width), to_fixed(48) }, // main floor
            { to_fixed(-32), to_fixed(32), to_fixed(16) },                               // raised ledge
        };
    }

    fixed ground_top_y_at(fixed x)
    {
        fixed best = no_ground_y;

        for(const platform& p : platforms)
        {
            if(x >= p.left_x && x <= p.right_x && p.top_y < best)
            {
                best = p.top_y;
            }
        }

        return best;
    }
}
