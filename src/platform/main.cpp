#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"

#include "bn_sprite_items_player.h"
#include "bn_sprite_items_rival.h"
#include "bn_sprite_items_ground.h"
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

    // A room's platforms/enemies live in world space (level::room_platform,
    // enemy x/y) — this subtracts the scrolling camera (level::camera_x/y)
    // so anything drawn with it stays correctly placed as the camera pans
    // across a room bigger than one screen. Every sprite that represents
    // something in the room (not a fixed HUD element) goes through this.
    void set_world_pixel_position(bn::sprite_ptr& sprite, int world_x_px, int world_y_px)
    {
        sprite.set_position(world_x_px - game::to_pixels(game::level::camera_x()),
                world_y_px - game::to_pixels(game::level::camera_y()));
    }

    void set_world_position(bn::sprite_ptr& sprite, game::fixed world_x, game::fixed world_y)
    {
        set_world_pixel_position(sprite, game::to_pixels(world_x), game::to_pixels(world_y));
    }

    // Placeholder-only room renderer: every platform is sliced into 64px
    // ground tiles from one shared pool, sized generously enough to cover
    // the biggest room (see assets/rooms/*.tmj). Real tile-based room art
    // replaces this later (see ROADMAP.md M4c).
    constexpr int tile_pool_size = 20;

    int tile_world_x[tile_pool_size];
    int tile_world_y[tile_pool_size];
    int tile_active_count = 0;

    // Recomputes which world-space tiles cover the active room's platforms.
    // Called once per room change — the room's platform layout is static.
    void populate_room_tiles()
    {
        tile_active_count = 0;

        for(int i = 0; i < game::level::room_platform_count(); ++i)
        {
            game::level::platform_view p = game::level::room_platform(i);
            int tiles_needed = (p.width_px + 63) / 64; // round up so the platform is fully covered

            for(int t = 0; t < tiles_needed && tile_active_count < tile_pool_size; ++t, ++tile_active_count)
            {
                tile_world_x[tile_active_count] = p.left_x_px + 32 + (t * 64);
                tile_world_y[tile_active_count] = p.top_y_px + 16;
            }
        }
    }

    // Re-positions the tile sprites relative to the camera every frame —
    // unlike populate_room_tiles, this has to run continuously, since a
    // multi-screen room's tiles move on-screen as the camera scrolls even
    // though their world position never changes.
    void update_tile_sprites(bn::sprite_ptr tile_sprites[tile_pool_size])
    {
        for(int i = 0; i < tile_pool_size; ++i)
        {
            bool active = i < tile_active_count;
            tile_sprites[i].set_visible(active);

            if(active)
            {
                set_world_pixel_position(tile_sprites[i], tile_world_x[i], tile_world_y[i]);
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
                set_world_position(skeleton_sprites[i], s.x, s.y);
                skeleton_sprites[i].set_horizontal_flip(s.facing < 0);
            }

            bool bone_active = active && game::active_skeleton(i).bone.active;
            bone_sprites[i].set_visible(bone_active);

            if(bone_active)
            {
                const game::arc_projectile& bone = game::active_skeleton(i).bone;
                set_world_position(bone_sprites[i], bone.x, bone.y);
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
                set_world_position(bat_sprites[i], b.x, b.y);
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
                set_world_position(archer_sprites[i], a.x, a.y);
                archer_sprites[i].set_horizontal_flip(a.facing < 0);
            }

            bool arrow_active = active && game::active_archer(i).arrow.active;
            arrow_sprites[i].set_visible(arrow_active);

            if(arrow_active)
            {
                const game::arc_projectile& arrow = game::active_archer(i).arrow;
                set_world_position(arrow_sprites[i], arrow.x, arrow.y);
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
                set_world_position(zombie_sprites[i], z.x, z.y);
                zombie_sprites[i].set_horizontal_flip(z.facing < 0);
            }
        }
    }

    void render_bone_pillars(bn::sprite_ptr pillar_sprites[], bn::sprite_ptr fireball_sprites[])
    {
        int count = game::active_bone_pillar_count();

        for(int i = 0; i < game::max_enemies_per_type; ++i)
        {
            bool active = i < count;
            bool alive = active && game::active_bone_pillar(i).alive;
            pillar_sprites[i].set_visible(alive);

            if(alive)
            {
                const game::bone_pillar_state& p = game::active_bone_pillar(i);
                set_world_position(pillar_sprites[i], p.x, p.y);
                pillar_sprites[i].set_horizontal_flip(p.facing < 0);
            }

            bool fireball_active = active && game::active_bone_pillar(i).fireball.active;
            fireball_sprites[i].set_visible(fireball_active);

            if(fireball_active)
            {
                const game::arc_projectile& fireball = game::active_bone_pillar(i).fireball;
                set_world_position(fireball_sprites[i], fireball.x, fireball.y);
            }
        }
    }

    void render_fleamen(bn::sprite_ptr fleaman_sprites[])
    {
        int count = game::active_fleaman_count();

        for(int i = 0; i < game::max_enemies_per_type; ++i)
        {
            bool alive = i < count && game::active_fleaman(i).alive;
            fleaman_sprites[i].set_visible(alive);

            if(alive)
            {
                const game::fleaman_state& f = game::active_fleaman(i);
                set_world_position(fleaman_sprites[i], f.x, f.y);
                fleaman_sprites[i].set_horizontal_flip(f.facing < 0);
            }
        }
    }

    void render_medusa_heads(bn::sprite_ptr medusa_sprites[])
    {
        int count = game::active_medusa_head_count();

        for(int i = 0; i < game::max_enemies_per_type; ++i)
        {
            bool alive = i < count && game::active_medusa_head(i).alive;
            medusa_sprites[i].set_visible(alive);

            if(alive)
            {
                const game::medusa_head_state& m = game::active_medusa_head(i);
                set_world_position(medusa_sprites[i], m.x, m.y);
                medusa_sprites[i].set_horizontal_flip(m.facing < 0);
            }
        }
    }

    // Basic HUD: a row of segments per bar, lit left-to-right by percentage.
    // Real bars (per SPEC.md's mockup) come with real art later. Fixed to
    // the screen, not the world — no camera offset here.
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
    bn::sprite_ptr tile_sprites[tile_pool_size] = {
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0), bn::sprite_items::ground.create_sprite(0, 0),
    };

    for(bn::sprite_ptr& s : tile_sprites) { s.set_visible(false); }

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
    // The 3 newest enemy types reuse existing sprite graphics rather than
    // distinct sprite items — the GBA only has 16 4bpp sprite palette banks
    // total, and every extra distinct sprite item costs one (empirically,
    // even 16 was one too many once Butano's own internal usage is counted).
    // Same placeholder shapes are fine; behavior is what distinguishes them.
    bn::sprite_ptr bone_pillar_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::zombie.create_sprite(0, 0), bn::sprite_items::zombie.create_sprite(0, 0),
        bn::sprite_items::zombie.create_sprite(0, 0), bn::sprite_items::zombie.create_sprite(0, 0),
    };
    bn::sprite_ptr fireball_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::bone.create_sprite(0, 0), bn::sprite_items::bone.create_sprite(0, 0),
        bn::sprite_items::bone.create_sprite(0, 0), bn::sprite_items::bone.create_sprite(0, 0),
    };
    bn::sprite_ptr fleaman_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::archer.create_sprite(0, 0), bn::sprite_items::archer.create_sprite(0, 0),
        bn::sprite_items::archer.create_sprite(0, 0), bn::sprite_items::archer.create_sprite(0, 0),
    };
    bn::sprite_ptr medusa_head_sprites[game::max_enemies_per_type] = {
        bn::sprite_items::bat.create_sprite(0, 0), bn::sprite_items::bat.create_sprite(0, 0),
        bn::sprite_items::bat.create_sprite(0, 0), bn::sprite_items::bat.create_sprite(0, 0),
    };

    for(bn::sprite_ptr& s : skeleton_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : bone_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : bat_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : archer_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : arrow_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : zombie_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : bone_pillar_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : fireball_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : fleaman_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : medusa_head_sprites) { s.set_visible(false); }

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
    populate_room_tiles();

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
            populate_room_tiles();
            game::spawn_room_enemies();
            last_drawn_room = current_room;

            if(game::level::current_room_is_save_room())
            {
                game::level::spawn_point room_spawn = game::level::current_room_spawn_point();
                set_world_position(save_point_sprite, room_spawn.x, room_spawn.y);
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
        set_world_position(active_sprite, player.x, player.y);
        active_sprite.set_horizontal_flip(player.facing < 0);

        hitbox_sprite.set_visible(hitbox.active);

        if(hitbox.active)
        {
            set_world_position(hitbox_sprite, hitbox.x, hitbox.y);
        }

        if(game::level::current_room_is_save_room())
        {
            game::level::spawn_point room_spawn = game::level::current_room_spawn_point();
            set_world_position(save_point_sprite, room_spawn.x, room_spawn.y);
        }

        update_tile_sprites(tile_sprites);
        render_skeletons(skeleton_sprites, bone_sprites);
        render_bats(bat_sprites);
        render_archers(archer_sprites, arrow_sprites);
        render_zombies(zombie_sprites);
        render_bone_pillars(bone_pillar_sprites, fireball_sprites);
        render_fleamen(fleaman_sprites);
        render_medusa_heads(medusa_head_sprites);
        render_hud(hp_hud_sprites, mp_hud_sprites, player);

        bn::core::update();
    }
}
