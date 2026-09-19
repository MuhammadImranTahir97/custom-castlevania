#include "world.h"

#include "level.h"

namespace game
{
    void update_world(player_state& player)
    {
        level::spawn_point target{};

        if(level::try_cross_door(player.x, player.y, target))
        {
            player.x = target.x;
            player.y = target.y;
            player.velocity_x = 0;
            player.velocity_y = 0;
            player.grounded = true;
            player.frames_since_grounded = 0;
        }
    }
}
