#include "projectile.h"

namespace game
{
    void launch_arc_projectile(arc_projectile& proj, fixed x, fixed y, fixed velocity_x, fixed velocity_y)
    {
        proj.active = true;
        proj.x = x;
        proj.y = y;
        proj.velocity_x = velocity_x;
        proj.velocity_y = velocity_y;
        proj.lifetime_frames = 0;
    }

    void update_arc_projectile(arc_projectile& proj, fixed gravity, int lifetime_limit)
    {
        if(! proj.active)
        {
            return;
        }

        proj.velocity_y += gravity;
        proj.x += proj.velocity_x;
        proj.y += proj.velocity_y;
        ++proj.lifetime_frames;

        if(proj.lifetime_frames >= lifetime_limit)
        {
            proj.active = false;
        }
    }
}
