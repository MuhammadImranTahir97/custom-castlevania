#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"

#include "bn_sprite_items_player.h"
#include "bn_sprite_items_ground.h"
#include "bn_sprite_items_ledge.h"
#include "bn_sprite_items_hitbox.h"
#include "bn_sprite_items_skeleton.h"
#include "bn_sprite_items_bone.h"

#include "enemy.h"
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

    bn::sprite_ptr hitbox_sprite = bn::sprite_items::hitbox.create_sprite(0, 0);
    hitbox_sprite.set_visible(false);

    bn::sprite_ptr skeleton_sprite = bn::sprite_items::skeleton.create_sprite(0, 0);
    bn::sprite_ptr bone_sprite = bn::sprite_items::bone.create_sprite(0, 0);
    bone_sprite.set_visible(false);

    game::player_state player;
    game::init_player(player);

    game::skeleton_state skeleton;
    game::init_skeleton(skeleton, game::to_fixed(-16)); // patrols the ledge, offset from player spawn

    while(true)
    {
        game::input_state input;
        input.left = bn::keypad::held(bn::keypad::key_type::LEFT);
        input.right = bn::keypad::held(bn::keypad::key_type::RIGHT);
        input.jump_held = bn::keypad::held(bn::keypad::key_type::A);
        input.attack_held = bn::keypad::held(bn::keypad::key_type::B);
        input.dodge_held = bn::keypad::held(bn::keypad::key_type::R);

        game::update_player(player, input);
        game::update_skeleton(skeleton);

        game::attack_hitbox hitbox = game::get_attack_hitbox(player);
        game::apply_attack_to_skeleton(skeleton, hitbox);

        player_sprite.set_position(game::to_pixels(player.x), game::to_pixels(player.y));
        player_sprite.set_horizontal_flip(player.facing < 0);

        hitbox_sprite.set_visible(hitbox.active);

        if(hitbox.active)
        {
            hitbox_sprite.set_position(game::to_pixels(hitbox.x), game::to_pixels(hitbox.y));
        }

        skeleton_sprite.set_visible(skeleton.alive);

        if(skeleton.alive)
        {
            skeleton_sprite.set_position(game::to_pixels(skeleton.x), game::to_pixels(skeleton.y));
            skeleton_sprite.set_horizontal_flip(skeleton.facing < 0);
        }

        bone_sprite.set_visible(skeleton.bone.active);

        if(skeleton.bone.active)
        {
            bone_sprite.set_position(game::to_pixels(skeleton.bone.x), game::to_pixels(skeleton.bone.y));
        }

        bn::core::update();
    }
}
