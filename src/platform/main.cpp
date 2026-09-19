#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"

#include "bn_sprite_items_player.h"

#include "input.h"
#include "player.h"

int main()
{
    bn::core::init();

    bn::sprite_ptr player_sprite = bn::sprite_items::player.create_sprite(0, 0);
    game::player_state player;

    while(true)
    {
        game::input_state input;
        input.up = bn::keypad::held(bn::keypad::key_type::UP);
        input.down = bn::keypad::held(bn::keypad::key_type::DOWN);
        input.left = bn::keypad::held(bn::keypad::key_type::LEFT);
        input.right = bn::keypad::held(bn::keypad::key_type::RIGHT);

        game::update_player(player, input);

        player_sprite.set_position(player.x, player.y);

        bn::core::update();
    }
}
