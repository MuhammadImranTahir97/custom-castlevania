#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"

#include "bn_sprite_items_player.h"
#include "bn_sprite_items_ground.h"
#include "bn_sprite_items_ledge.h"
#include "bn_sprite_items_hitbox.h"
#include "bn_sprite_items_skeleton.h"
#include "bn_sprite_items_bone.h"
#include "bn_sprite_items_bat.h"
#include "bn_sprite_items_archer.h"
#include "bn_sprite_items_arrow.h"

#include "enemy.h"
#include "enemy_spawner.h"
#include "fixed.h"
#include "input.h"
#include "level.h"
#include "player.h"
#include "world.h"

namespace
{
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

    void render_skeletons(bn::sprite_ptr skeleton_sprites[], bn::sprite_ptr bone_sprites[])
    {
        int count = game::active_skeleton_count();

        for(int i = 0; i < game::max_enemies_per_type; ++i)
        {
            bool active = i < count;
            bool alive = active && game::active_skeleton(i).alive;
            skeleton_sprites[i].set_visible(alive);

            if(alive)
            {
                const game::skeleton_state& s = game::active_skeleton(i);
                skeleton_sprites[i].set_position(game::to_pixels(s.x), game::to_pixels(s.y));
                skeleton_sprites[i].set_horizontal_flip(s.facing < 0);
            }

            bool bone_active = active && game::active_skeleton(i).bone.active;
            bone_sprites[i].set_visible(bone_active);

            if(bone_active)
            {
                const game::arc_projectile& bone = game::active_skeleton(i).bone;
                bone_sprites[i].set_position(game::to_pixels(bone.x), game::to_pixels(bone.y));
            }
        }
    }

    void render_bats(bn::sprite_ptr bat_sprites[])
    {
        int count = game::active_bat_count();

        for(int i = 0; i < game::max_enemies_per_type; ++i)
        {
            bool alive = i < count && game::active_bat(i).alive;
            bat_sprites[i].set_visible(alive);

            if(alive)
            {
                const game::bat_state& b = game::active_bat(i);
                bat_sprites[i].set_position(game::to_pixels(b.x), game::to_pixels(b.y));
                bat_sprites[i].set_horizontal_flip(b.facing < 0);
            }
        }
    }

    void render_archers(bn::sprite_ptr archer_sprites[], bn::sprite_ptr arrow_sprites[])
    {
        int count = game::active_archer_count();

        for(int i = 0; i < game::max_enemies_per_type; ++i)
        {
            bool active = i < count;
            bool alive = active && game::active_archer(i).alive;
            archer_sprites[i].set_visible(alive);

            if(alive)
            {
                const game::archer_state& a = game::active_archer(i);
                archer_sprites[i].set_position(game::to_pixels(a.x), game::to_pixels(a.y));
                archer_sprites[i].set_horizontal_flip(a.facing < 0);
            }

            bool arrow_active = active && game::active_archer(i).arrow.active;
            arrow_sprites[i].set_visible(arrow_active);

            if(arrow_active)
            {
                const game::arc_projectile& arrow = game::active_archer(i).arrow;
                arrow_sprites[i].set_position(game::to_pixels(arrow.x), game::to_pixels(arrow.y));
            }
        }
    }
}

int main()
{
    bn::core::init();

    bn::sprite_ptr player_sprite = bn::sprite_items::player.create_sprite(0, 0);

    // Room geometry is data-driven (assets/rooms/*.tmj, see src/game/level.cpp)
    // but every room reuses the same handful of placeholder tile sprites.
    bn::sprite_ptr ground_sprite_0 = bn::sprite_items::ground.create_sprite(0, 0);
    bn::sprite_ptr ground_sprite_1 = bn::sprite_items::ground.create_sprite(0, 0);
    bn::sprite_ptr ground_sprite_2 = bn::sprite_items::ground.create_sprite(0, 0);
    bn::sprite_ptr ground_sprite_3 = bn::sprite_items::ground.create_sprite(0, 0);
    bn::sprite_ptr* ground_tiles[4] = { &ground_sprite_0, &ground_sprite_1, &ground_sprite_2, &ground_sprite_3 };
    bn::sprite_ptr ledge_sprite = bn::sprite_items::ledge.create_sprite(0, 0);

    bn::sprite_ptr hitbox_sprite = bn::sprite_items::hitbox.create_sprite(0, 0);
    hitbox_sprite.set_visible(false);

    // Fixed sprite pools, one per game::max_enemies_per_type slot per type.
    bn::sprite_ptr skeleton_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::skeleton.create_sprite(0, 0), bn::sprite_items::skeleton.create_sprite(0, 0),
    };
    bn::sprite_ptr bone_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::bone.create_sprite(0, 0), bn::sprite_items::bone.create_sprite(0, 0),
    };
    bn::sprite_ptr bat_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::bat.create_sprite(0, 0), bn::sprite_items::bat.create_sprite(0, 0),
    };
    bn::sprite_ptr archer_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::archer.create_sprite(0, 0), bn::sprite_items::archer.create_sprite(0, 0),
    };
    bn::sprite_ptr arrow_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::arrow.create_sprite(0, 0), bn::sprite_items::arrow.create_sprite(0, 0),
    };

    for(bn::sprite_ptr& s : skeleton_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : bone_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : bat_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : archer_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : arrow_sprites) { s.set_visible(false); }

    game::level::spawn_point spawn = game::level::load_room(0);

    game::player_state player;
    game::init_player(player, spawn.x, spawn.y);

    game::spawn_room_enemies();

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
            game::spawn_room_enemies();
            last_drawn_room = current_room;
        }

        game::update_enemies(player.x, player.y);

        game::attack_hitbox hitbox = game::get_attack_hitbox(player);
        game::apply_attacks_to_enemies(hitbox);
        game::apply_enemy_contact_to_player(player);

        player_sprite.set_position(game::to_pixels(player.x), game::to_pixels(player.y));
        player_sprite.set_horizontal_flip(player.facing < 0);

        hitbox_sprite.set_visible(hitbox.active);

        if(hitbox.active)
        {
            hitbox_sprite.set_position(game::to_pixels(hitbox.x), game::to_pixels(hitbox.y));
        }

        render_skeletons(skeleton_sprites, bone_sprites);
        render_bats(bat_sprites);
        render_archers(archer_sprites, arrow_sprites);

        bn::core::update();
    }
}
