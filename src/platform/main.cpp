#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"

#include "bn_sprite_items_player.h"
#include "bn_sprite_items_rival.h"
#include "bn_sprite_items_ground.h"
#include "bn_sprite_items_ledge.h"
#include "bn_sprite_items_hitbox.h"
#include "bn_sprite_items_skeleton.h"
#include "bn_sprite_items_bone.h"
#include "bn_sprite_items_bat.h"
#include "bn_sprite_items_archer.h"
#include "bn_sprite_items_arrow.h"
#include "bn_sprite_items_hud_hp.h"
#include "bn_sprite_items_hud_mp.h"
#include "bn_sprite_items_zombie.h"
#include "bn_sprite_items_save_point.h"

#include "enemy.h"
#include "enemy_spawner.h"
#include "fixed.h"
#include "input.h"
#include "level.h"
#include "player.h"
#include "world.h"

namespace
{
    constexpr int hud_segments = 10;

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

    void render_zombies(bn::sprite_ptr zombie_sprites[])
    {
        int count = game::active_zombie_count();

        for(int i = 0; i < game::max_enemies_per_type; ++i)
        {
            bool alive = i < count && game::active_zombie(i).alive;
            zombie_sprites[i].set_visible(alive);

            if(alive)
            {
                const game::zombie_state& z = game::active_zombie(i);
                zombie_sprites[i].set_position(game::to_pixels(z.x), game::to_pixels(z.y));
                zombie_sprites[i].set_horizontal_flip(z.facing < 0);
            }
        }
    }

    // Basic HUD: a row of segments per bar, lit left-to-right by percentage.
    // Real bars (per SPEC.md's mockup) come with real art later.
    void render_hud(bn::sprite_ptr hp_sprites[hud_segments], bn::sprite_ptr mp_sprites[hud_segments],
            const game::player_state& player)
    {
        int hp_lit = (player.hp * hud_segments) / player.max_hp;
        int mp_lit = (player.mp * hud_segments) / player.max_mp;

        for(int i = 0; i < hud_segments; ++i)
        {
            hp_sprites[i].set_visible(i < hp_lit);
            mp_sprites[i].set_visible(i < mp_lit);
        }
    }
}

int main()
{
    bn::core::init();

    bn::sprite_ptr hunter_sprite = bn::sprite_items::player.create_sprite(0, 0);
    bn::sprite_ptr rival_sprite = bn::sprite_items::rival.create_sprite(0, 0);
    rival_sprite.set_visible(false);

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
        bn::sprite_items::skeleton.create_sprite(0, 0), bn::sprite_items::skeleton.create_sprite(0, 0),
    };
    bn::sprite_ptr bone_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::bone.create_sprite(0, 0), bn::sprite_items::bone.create_sprite(0, 0),
        bn::sprite_items::bone.create_sprite(0, 0), bn::sprite_items::bone.create_sprite(0, 0),
    };
    bn::sprite_ptr bat_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::bat.create_sprite(0, 0), bn::sprite_items::bat.create_sprite(0, 0),
        bn::sprite_items::bat.create_sprite(0, 0), bn::sprite_items::bat.create_sprite(0, 0),
    };
    bn::sprite_ptr archer_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::archer.create_sprite(0, 0), bn::sprite_items::archer.create_sprite(0, 0),
        bn::sprite_items::archer.create_sprite(0, 0), bn::sprite_items::archer.create_sprite(0, 0),
    };
    bn::sprite_ptr arrow_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::arrow.create_sprite(0, 0), bn::sprite_items::arrow.create_sprite(0, 0),
        bn::sprite_items::arrow.create_sprite(0, 0), bn::sprite_items::arrow.create_sprite(0, 0),
    };
    bn::sprite_ptr zombie_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::zombie.create_sprite(0, 0), bn::sprite_items::zombie.create_sprite(0, 0),
        bn::sprite_items::zombie.create_sprite(0, 0), bn::sprite_items::zombie.create_sprite(0, 0),
    };

    for(bn::sprite_ptr& s : skeleton_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : bone_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : bat_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : archer_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : arrow_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : zombie_sprites) { s.set_visible(false); }

    bn::sprite_ptr save_point_sprite = bn::sprite_items::save_point.create_sprite(0, 0);
    save_point_sprite.set_visible(false);

    // HUD: top-left corner, HP row above MP row, 10 segments of 8px each.
    bn::sprite_ptr hp_hud_sprites[hud_segments] = {
        bn::sprite_items::hud_hp.create_sprite(-112, -72), bn::sprite_items::hud_hp.create_sprite(-104, -72),
        bn::sprite_items::hud_hp.create_sprite(-96, -72), bn::sprite_items::hud_hp.create_sprite(-88, -72),
        bn::sprite_items::hud_hp.create_sprite(-80, -72), bn::sprite_items::hud_hp.create_sprite(-72, -72),
        bn::sprite_items::hud_hp.create_sprite(-64, -72), bn::sprite_items::hud_hp.create_sprite(-56, -72),
        bn::sprite_items::hud_hp.create_sprite(-48, -72), bn::sprite_items::hud_hp.create_sprite(-40, -72),
    };
    bn::sprite_ptr mp_hud_sprites[hud_segments] = {
        bn::sprite_items::hud_mp.create_sprite(-112, -64), bn::sprite_items::hud_mp.create_sprite(-104, -64),
        bn::sprite_items::hud_mp.create_sprite(-96, -64), bn::sprite_items::hud_mp.create_sprite(-88, -64),
        bn::sprite_items::hud_mp.create_sprite(-80, -64), bn::sprite_items::hud_mp.create_sprite(-72, -64),
        bn::sprite_items::hud_mp.create_sprite(-64, -64), bn::sprite_items::hud_mp.create_sprite(-56, -64),
        bn::sprite_items::hud_mp.create_sprite(-48, -64), bn::sprite_items::hud_mp.create_sprite(-40, -64),
    };

    game::level::spawn_point spawn = game::level::load_room(0);

    game::player_state player;
    game::init_player(player, spawn.x, spawn.y);
    game::set_checkpoint(0, spawn.x, spawn.y);

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
        input.swap_held = bn::keypad::held(bn::keypad::key_type::L);

        game::update_player(player, input);
        game::update_world(player);

        int current_room = game::level::current_room_index();

        if(current_room != last_drawn_room)
        {
            draw_room(ground_tiles, ledge_sprite);
            game::spawn_room_enemies();
            last_drawn_room = current_room;

            if(game::level::current_room_is_save_room())
            {
                game::level::spawn_point room_spawn = game::level::current_room_spawn_point();
                save_point_sprite.set_position(game::to_pixels(room_spawn.x), game::to_pixels(room_spawn.y));
                save_point_sprite.set_visible(true);
            }
            else
            {
                save_point_sprite.set_visible(false);
            }
        }

        game::update_enemies(player.x, player.y);

        game::attack_hitbox hitbox = game::get_attack_hitbox(player);
        game::apply_attacks_to_enemies(hitbox, player);
        game::apply_enemy_contact_to_player(player);

        bool is_rival = player.character == game::character_kind::rival;
        bn::sprite_ptr& active_sprite = is_rival ? rival_sprite : hunter_sprite;
        bn::sprite_ptr& inactive_sprite = is_rival ? hunter_sprite : rival_sprite;

        inactive_sprite.set_visible(false);
        active_sprite.set_visible(true);
        active_sprite.set_position(game::to_pixels(player.x), game::to_pixels(player.y));
        active_sprite.set_horizontal_flip(player.facing < 0);

        hitbox_sprite.set_visible(hitbox.active);

        if(hitbox.active)
        {
            hitbox_sprite.set_position(game::to_pixels(hitbox.x), game::to_pixels(hitbox.y));
        }

        render_skeletons(skeleton_sprites, bone_sprites);
        render_bats(bat_sprites);
        render_archers(archer_sprites, arrow_sprites);
        render_zombies(zombie_sprites);
        render_hud(hp_hud_sprites, mp_hud_sprites, player);

        bn::core::update();
    }
}
