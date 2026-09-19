#include "player.h"

#include "difficulty.h"

namespace game
{
    void update_player(player_state& player, const input_state& input)
    {
        int speed = difficulty::player_move_speed;

        if(input.left)
        {
            player.x -= speed;
        }
        if(input.right)
        {
            player.x += speed;
        }
        if(input.up)
        {
            player.y -= speed;
        }
        if(input.down)
        {
            player.y += speed;
        }
    }
}
