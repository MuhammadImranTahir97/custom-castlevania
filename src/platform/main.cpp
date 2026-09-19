#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"

#include "bn_sprite_items_player.h"
#include "bn_sprite_items_ground.h"
#include "bn_sprite_items_ledge.h"

#include "fixed.h"
#include "input.h"
#include "player.h"

int main()
{
    bn::core::init();

    bn::sprite_ptr player_sprite = bn::sprite_items::player.create_sprite(0, 0);

    // M1's hand-built room: a flat floor made of 4 tiled placeholder sprites,
    // plus one raised ledge platform to walk off of (see src/game/level.cpp).
    bn::sprite_ptr ground_sprite_0 = bn::sprite_items::ground.create_sprite(-96, 64);
    bn::sprite_ptr ground_sprite_1 = bn::sprite_items::ground.create_sprite(-32, 64);
    bn::sprite_ptr ground_sprite_2 = bn::sprite_items::ground.create_sprite(32, 64);
    bn::sprite_ptr ground_sprite_3 = bn::sprite_items::ground.create_sprite(96, 64);
    bn::sprite_ptr ledge_sprite = bn::sprite_items::ledge.create_sprite(0, 32);

    game::player_state player;
    game::init_player(player);

    while(true)
    {
        game::input_state input;
        input.left = bn::keypad::held(bn::keypad::key_type::LEFT);
        input.right = bn::keypad::held(bn::keypad::key_type::RIGHT);
        input.jump_held = bn::keypad::held(bn::keypad::key_type::A);

        game::update_player(player, input);

        player_sprite.set_position(game::to_pixels(player.x), game::to_pixels(player.y));

        bn::core::update();
    }
}
