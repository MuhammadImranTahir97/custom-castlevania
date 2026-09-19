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
#include "level.h"
#include "player.h"
#include "world.h"

namespace
{
    // The skeleton isn't part of the room data yet (per-room enemy lists are
    // a later step) — it's hardcoded to this one room until then.
    constexpr int skeleton_room_index = 0;

    // Placeholder-only room renderer: any platform 64px wide or narrower is
    // drawn as the single ledge tile; anything wider is sliced into 64px
    // ground tiles (up to 4). Real tile-based room art replaces this later
    // (see ROADMAP.md M4c) — for grey-box rooms this is enough.
    void draw_room(bn::sprite_ptr* ground_tiles[4], bn::sprite_ptr& ledge_sprite)
    {
        for(int i = 0; i < 4; ++i)
        {
            ground_tiles[i]->set_visible(false);
        }

        ledge_sprite.set_visible(false);

        int ground_tile_index = 0;
        bool ledge_placed = false;

        for(int i = 0; i < game::level::room_platform_count(); ++i)
        {
            game::level::platform_view p = game::level::room_platform(i);

            if(p.width_px <= 64 && ! ledge_placed)
            {
                ledge_sprite.set_position(p.left_x_px + p.width_px / 2, p.top_y_px + 16);
                ledge_sprite.set_visible(true);
                ledge_placed = true;
                continue;
            }

            int tiles_needed = (p.width_px + 63) / 64; // round up so the platform is fully covered

            for(int t = 0; t < tiles_needed && ground_tile_index < 4; ++t, ++ground_tile_index)
            {
                int center_x = p.left_x_px + 32 + (t * 64);
                int center_y = p.top_y_px + 16;
                ground_tiles[ground_tile_index]->set_position(center_x, center_y);
                ground_tiles[ground_tile_index]->set_visible(true);
            }
        }
    }
}

int main()
{
    bn::core::init();

    bn::sprite_ptr player_sprite = bn::sprite_items::player.create_sprite(0, 0);

    // Room geometry is data-driven (assets/rooms/*.json, see src/game/level.cpp)
    // but every room reuses the same handful of placeholder tile sprites.
    bn::sprite_ptr ground_sprite_0 = bn::sprite_items::ground.create_sprite(0, 0);
    bn::sprite_ptr ground_sprite_1 = bn::sprite_items::ground.create_sprite(0, 0);
    bn::sprite_ptr ground_sprite_2 = bn::sprite_items::ground.create_sprite(0, 0);
    bn::sprite_ptr ground_sprite_3 = bn::sprite_items::ground.create_sprite(0, 0);
    bn::sprite_ptr* ground_tiles[4] = { &ground_sprite_0, &ground_sprite_1, &ground_sprite_2, &ground_sprite_3 };
    bn::sprite_ptr ledge_sprite = bn::sprite_items::ledge.create_sprite(0, 0);

    bn::sprite_ptr hitbox_sprite = bn::sprite_items::hitbox.create_sprite(0, 0);
    hitbox_sprite.set_visible(false);

    bn::sprite_ptr skeleton_sprite = bn::sprite_items::skeleton.create_sprite(0, 0);
    bn::sprite_ptr bone_sprite = bn::sprite_items::bone.create_sprite(0, 0);
    bone_sprite.set_visible(false);

    game::level::spawn_point spawn = game::level::load_room(0);

    game::player_state player;
    game::init_player(player, spawn.x, spawn.y);

    game::skeleton_state skeleton;
    game::init_skeleton(skeleton, game::to_fixed(-16)); // patrols the ledge, offset from player spawn

    int last_drawn_room = -1;

    while(true)
    {
        game::input_state input;
        input.left = bn::keypad::held(bn::keypad::key_type::LEFT);
        input.right = bn::keypad::held(bn::keypad::key_type::RIGHT);
        input.jump_held = bn::keypad::held(bn::keypad::key_type::A);
        input.attack_held = bn::keypad::held(bn::keypad::key_type::B);
        input.dodge_held = bn::keypad::held(bn::keypad::key_type::R);

        game::update_player(player, input);
        game::update_world(player);

        int current_room = game::level::current_room_index();

        if(current_room != last_drawn_room)
        {
            draw_room(ground_tiles, ledge_sprite);
            last_drawn_room = current_room;
        }

        bool skeleton_room_active = current_room == skeleton_room_index;

        if(skeleton_room_active)
        {
            game::update_skeleton(skeleton);
        }

        game::attack_hitbox hitbox = game::get_attack_hitbox(player);

        if(skeleton_room_active)
        {
            game::apply_attack_to_skeleton(skeleton, hitbox);
        }

        player_sprite.set_position(game::to_pixels(player.x), game::to_pixels(player.y));
        player_sprite.set_horizontal_flip(player.facing < 0);

        hitbox_sprite.set_visible(hitbox.active);

        if(hitbox.active)
        {
            hitbox_sprite.set_position(game::to_pixels(hitbox.x), game::to_pixels(hitbox.y));
        }

        skeleton_sprite.set_visible(skeleton_room_active && skeleton.alive);

        if(skeleton_room_active && skeleton.alive)
        {
            skeleton_sprite.set_position(game::to_pixels(skeleton.x), game::to_pixels(skeleton.y));
            skeleton_sprite.set_horizontal_flip(skeleton.facing < 0);
        }

        bone_sprite.set_visible(skeleton_room_active && skeleton.bone.active);

        if(skeleton_room_active && skeleton.bone.active)
        {
            bone_sprite.set_position(game::to_pixels(skeleton.bone.x), game::to_pixels(skeleton.bone.y));
        }

        bn::core::update();
    }
}
