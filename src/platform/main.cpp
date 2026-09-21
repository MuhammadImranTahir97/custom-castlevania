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
#include "map_layout.h"
#include "player.h"
#include "save_data.h"
#include "world.h"

#include "save_io.h"

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

            // Rounding up here would draw a tile past the platform's actual
            // (narrower) collision width -- ground that looks solid but
            // isn't, so walking onto that sliver drops the player through
            // it. Round down instead: platforms under 64px still get one
            // tile (there's no smaller placeholder piece to draw), but any
            // wider platform never shows more ground than it actually has.
            int tiles_needed = p.width_px / 64;

            if(tiles_needed < 1)
            {
                tiles_needed = 1;
            }

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

    void render_bone_colossus(bn::sprite_ptr& body_sprite, bn::sprite_ptr& attack_sprite,
            bn::sprite_ptr rib_sprites[game::bone_colossus_rib_count])
    {
        bool active = game::bone_colossus_is_active();
        bool alive = active && game::active_bone_colossus().alive;
        body_sprite.set_visible(alive);

        if(alive)
        {
            const game::bone_colossus_state& boss = game::active_bone_colossus();
            set_world_position(body_sprite, boss.x, boss.y);
            body_sprite.set_horizontal_flip(boss.facing < 0);
        }

        game::boss_attack_hitbox hitbox = alive
                ? game::get_bone_colossus_attack_hitbox(game::active_bone_colossus())
                : game::boss_attack_hitbox{};
        attack_sprite.set_visible(hitbox.active);

        if(hitbox.active)
        {
            set_world_position(attack_sprite, hitbox.x, hitbox.y);
        }

        for(int i = 0; i < game::bone_colossus_rib_count; ++i)
        {
            bool rib_active = active && game::active_bone_colossus().ribs[i].active;
            rib_sprites[i].set_visible(rib_active);

            if(rib_active)
            {
                const game::arc_projectile& rib = game::active_bone_colossus().ribs[i];
                set_world_position(rib_sprites[i], rib.x, rib.y);
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

    constexpr int pause_tab_count = 5; // Status, Arcana, Items, Map, Options
    constexpr int arcana_tab_index = 1;
    constexpr int map_tab_index = 3;

    // Screen-space (not world-space — the pause menu isn't affected by the
    // camera) position for a room's map marker.
    void map_marker_screen_position(int room_index, int& out_x, int& out_y)
    {
        constexpr int scale_px = 14;
        constexpr int offset_x_px = -77;
        constexpr int offset_y_px = 7;

        game::map_grid_position pos = game::map_position_for_room(room_index);
        out_x = pos.x * scale_px + offset_x_px;
        out_y = pos.y * scale_px + offset_y_px;
    }

    // The Arcana tab's 5 selectable cards (SPEC.md's control map): index
    // 0-1 are the 2 Action cards (Mercury, Diana), 2-4 are the 3 Attribute
    // cards (Salamander, Serpent, Mandragora) -- arcana_types.h's enum
    // orders match these slots directly (mercury=0/diana=1,
    // salamander=0/serpent=1/mandragora=2 -> +2 for the row offset).
    constexpr int arcana_card_count = 5;
    constexpr int arcana_action_x[2] = { -40, 8 };
    constexpr int arcana_attribute_x[3] = { -56, -8, 40 };
    constexpr int arcana_action_y = -16;
    constexpr int arcana_attribute_y = 24;

    void arcana_card_screen_position(int card_index, int& out_x, int& out_y)
    {
        if(card_index < 2)
        {
            out_x = arcana_action_x[card_index];
            out_y = arcana_action_y;
        }
        else
        {
            out_x = arcana_attribute_x[card_index - 2];
            out_y = arcana_attribute_y;
        }
    }

    // Grey-box pause menu (SPEC.md section 9: 5 tabs, shoulder buttons
    // cycle). Status is covered by the HUD already on screen (HP/MP) —
    // nothing extra to add there. Items/Options are structural only:
    // neither is a system in the game yet (no inventory, no settings), so
    // there's nothing real to show, and inventing placeholder data for
    // them would be more misleading than an empty tab. Arcana and Map are
    // where the real content is: Arcana picks the equipped combo (SPEC.md's
    // control map moved loadout selection here, out of gameplay), Map
    // shows explored rooms, save points, the current room, and sealed-door
    // markers (the Bone Colossus's arena is the only thing that seals
    // doors right now).
    //
    // cursor_sprite triples as "which of the 5 tabs is active" (Status/
    // Items/Options), "current room" (Map tab) and "highlighted card"
    // (Arcana tab) — one sprite for all three keeps this within the
    // sprite budget alongside everything else already on screen (see the
    // palette-budget comments elsewhere in this file for why reusing
    // sprites, not adding new ones, is the house style here).
    void render_pause_menu(int tab, bn::sprite_ptr& cursor_sprite,
            bn::sprite_ptr room_sprites[game::map_room_count], bn::sprite_ptr seal_sprites[2],
            int pause_blink_frame, const game::player_state& player, int arcana_cursor,
            bn::sprite_ptr arcana_card_sprites[arcana_card_count], bn::sprite_ptr arcana_equipped_sprites[2])
    {
        constexpr int tab_slot_x[pause_tab_count] = { -96, -48, 0, 48, 96 };
        constexpr int tab_slot_y = -64;

        bool on_map_tab = tab == map_tab_index;
        bool on_arcana_tab = tab == arcana_tab_index;

        for(int i = 0; i < game::map_room_count; ++i)
        {
            bool visible = on_map_tab && game::level::room_visited(i);
            room_sprites[i].set_visible(visible);

            if(visible)
            {
                int x;
                int y;
                map_marker_screen_position(i, x, y);
                room_sprites[i].set_position(x, y);
            }
        }

        // catacombs_16/17/18 are room_data indices 15/16/17 (alphabetical
        // filename order — see room_data.h).
        bool boss_sealed = ! game::bone_colossus_defeated();
        bool seal_a_visible = on_map_tab && boss_sealed && game::level::room_visited(15);
        bool seal_b_visible = on_map_tab && boss_sealed && game::level::room_visited(16);

        seal_sprites[0].set_visible(seal_a_visible);
        seal_sprites[1].set_visible(seal_b_visible);

        if(seal_a_visible || seal_b_visible)
        {
            int x16;
            int y16;
            int x17;
            int y17;
            int x18;
            int y18;
            map_marker_screen_position(15, x16, y16);
            map_marker_screen_position(16, x17, y17);
            map_marker_screen_position(17, x18, y18);

            if(seal_a_visible)
            {
                seal_sprites[0].set_position((x16 + x17) / 2, (y16 + y17) / 2);
            }

            if(seal_b_visible)
            {
                seal_sprites[1].set_position((x17 + x18) / 2, (y17 + y18) / 2);
            }
        }

        for(int i = 0; i < arcana_card_count; ++i)
        {
            arcana_card_sprites[i].set_visible(on_arcana_tab);

            if(on_arcana_tab)
            {
                int x;
                int y;
                arcana_card_screen_position(i, x, y);
                arcana_card_sprites[i].set_position(x, y);
            }
        }

        arcana_equipped_sprites[0].set_visible(on_arcana_tab);
        arcana_equipped_sprites[1].set_visible(on_arcana_tab);

        if(on_arcana_tab)
        {
            int action_x;
            int action_y;
            int attribute_x;
            int attribute_y;
            arcana_card_screen_position(static_cast<int>(player.arcana.action), action_x, action_y);
            arcana_card_screen_position(2 + static_cast<int>(player.arcana.attribute), attribute_x, attribute_y);

            // A small marker below each equipped card, distinct from
            // cursor_sprite's own position (which one of the 5 is
            // highlighted, not which two are equipped).
            arcana_equipped_sprites[0].set_position(action_x, action_y + 12);
            arcana_equipped_sprites[1].set_position(attribute_x, attribute_y + 12);
        }

        if(on_map_tab)
        {
            int current = game::level::current_room_index();

            if(current >= 0 && current < game::map_room_count)
            {
                int x;
                int y;
                map_marker_screen_position(current, x, y);
                cursor_sprite.set_position(x, y);
                cursor_sprite.set_visible((pause_blink_frame / 8) % 2 == 0);
            }
            else
            {
                cursor_sprite.set_visible(false);
            }
        }
        else if(on_arcana_tab)
        {
            int x;
            int y;
            arcana_card_screen_position(arcana_cursor, x, y);
            cursor_sprite.set_position(x, y);
            cursor_sprite.set_visible(true);
        }
        else
        {
            cursor_sprite.set_position(tab_slot_x[tab], tab_slot_y);
            cursor_sprite.set_visible(true);
        }
    }

    // DEBUG ONLY — jump to any room by index without playing to it. Hold
    // L+R+START together to toggle; LEFT/RIGHT picks a room (wrapping
    // through every room in the generated table, not just the ones
    // map_layout.h has a position for), A confirms, B cancels. Reuses the
    // pause menu's map sprites (mutually exclusive with pause — you can't
    // be in both at once) rather than adding a dedicated sprite pool.
    // Should come out before shipping (see ROADMAP.md's M5 checklist) —
    // it bypasses sealed doors and any other gating entirely on purpose.
    void render_debug_warp_screen(int selected_room, bn::sprite_ptr& cursor_sprite,
            bn::sprite_ptr room_sprites[game::map_room_count])
    {
        for(int i = 0; i < game::map_room_count; ++i)
        {
            room_sprites[i].set_visible(true);

            int x;
            int y;
            map_marker_screen_position(i, x, y);
            room_sprites[i].set_position(x, y);
        }

        if(selected_room >= 0 && selected_room < game::map_room_count)
        {
            int x;
            int y;
            map_marker_screen_position(selected_room, x, y);
            cursor_sprite.set_position(x, y);
            cursor_sprite.set_visible(true);
        }
        else
        {
            // A room this screen has no map position for (e.g. a sandbox
            // room) — still selectable and warpable, just no cursor to show.
            cursor_sprite.set_visible(false);
        }
    }

    // Grey-box slot picker (SPEC.md section 9: 3 slots) — a real title
    // screen with slot details (level, room, playtime) is a text-rendering
    // feature this project doesn't have yet (no sprite_text_generator use
    // anywhere else either); this is deliberately just enough to make the
    // 3 slots real and player-choosable. LEFT/RIGHT moves the cursor, A
    // confirms — an empty slot starts a new game there, an occupied one
    // continues it. All of this screen's sprites are local to this
    // function and freed (via bn::sprite_ptr's RAII) before gameplay
    // starts, so none of it costs OAM budget once playing.
    int run_slot_select_screen()
    {
        constexpr int slot_x[game::save_slot_count] = { -80, 0, 80 };

        bn::sprite_ptr slot_sprites[game::save_slot_count] = {
            bn::sprite_items::ground.create_sprite(slot_x[0], 0),
            bn::sprite_items::ground.create_sprite(slot_x[1], 0),
            bn::sprite_items::ground.create_sprite(slot_x[2], 0),
        };
        // A small marker under each occupied slot -- the only way to tell
        // "continue" from "new game" here without text.
        bn::sprite_ptr occupied_sprites[game::save_slot_count] = {
            bn::sprite_items::bone.create_sprite(slot_x[0], 20),
            bn::sprite_items::bone.create_sprite(slot_x[1], 20),
            bn::sprite_items::bone.create_sprite(slot_x[2], 20),
        };
        bn::sprite_ptr cursor_sprite = bn::sprite_items::hitbox.create_sprite(slot_x[0], -24);

        for(int i = 0; i < game::save_slot_count; ++i)
        {
            occupied_sprites[i].set_visible(platform_save::slot_has_data(i));
        }

        int selected = 0;

        while(true)
        {
            if(bn::keypad::pressed(bn::keypad::key_type::LEFT) && selected > 0)
            {
                --selected;
            }
            else if(bn::keypad::pressed(bn::keypad::key_type::RIGHT) && selected < game::save_slot_count - 1)
            {
                ++selected;
            }

            cursor_sprite.set_x(slot_x[selected]);

            if(bn::keypad::pressed(bn::keypad::key_type::A))
            {
                return selected;
            }

            bn::core::update();
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

    // Diana's Arcana shot (arcana.h) — reuses "arrow" (already loaded for
    // the Skeleton Archer) rather than a new sprite item, same
    // palette-budget reasoning as the M3 enemies further below.
    bn::sprite_ptr arcana_projectile_sprite = bn::sprite_items::arrow.create_sprite(0, 0);
    arcana_projectile_sprite.set_visible(false);

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
    // Same palette-budget reasoning as above: the Colossus's body reuses
    // "skeleton" (thematically the closest fit, and at most one of it is
    // ever on screen at a time, unlike the other reuses above), its ribs
    // reuse "bone" (the skeleton's own bone toss, matching "rib-cage" of
    // bones), and its attack telegraph reuses "hitbox" (the player's own
    // attack marker).
    bn::sprite_ptr bone_colossus_sprite = bn::sprite_items::skeleton.create_sprite(0, 0);
    bn::sprite_ptr bone_colossus_hitbox_sprite = bn::sprite_items::hitbox.create_sprite(0, 0);
    bn::sprite_ptr bone_colossus_rib_sprites[game::bone_colossus_rib_count] = {
        bn::sprite_items::bone.create_sprite(0, 0), bn::sprite_items::bone.create_sprite(0, 0),
        bn::sprite_items::bone.create_sprite(0, 0), bn::sprite_items::bone.create_sprite(0, 0),
        bn::sprite_items::bone.create_sprite(0, 0),
    };

    // Pause menu / map screen (SPEC.md section 9). "bone" is the generic
    // explored-room marker, "save_point" marks the 2 rooms that actually
    // are save rooms (catacombs_03 = index 2, catacombs_13 = index 12),
    // "hitbox" doubles as the tab cursor and the current-room marker, and
    // "arrow" marks sealed doors — same reused-sprite reasoning as above.
    bn::sprite_ptr map_room_sprites[game::map_room_count] = {
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_01
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_02
        bn::sprite_items::save_point.create_sprite(0, 0), // catacombs_03 (save room)
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_04
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_05
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_06
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_07
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_08
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_09
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_10
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_11
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_12
        bn::sprite_items::save_point.create_sprite(0, 0), // catacombs_13 (save room)
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_14
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_15
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_16
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_17 (boss arena)
        bn::sprite_items::bone.create_sprite(0, 0),       // catacombs_18
    };
    bn::sprite_ptr pause_cursor_sprite = bn::sprite_items::hitbox.create_sprite(0, 0);
    bn::sprite_ptr map_seal_sprites[2] = {
        bn::sprite_items::arrow.create_sprite(0, 0),
        bn::sprite_items::arrow.create_sprite(0, 0),
    };

    // Arcana tab (SPEC.md's control map): "ground" for the 5 selectable
    // cards (2 Action, 3 Attribute — reused the same way the room-tile
    // pool above reuses it, still just a grey box), "save_point" for the
    // two small "equipped" markers under the current loadout — same
    // reused-sprite reasoning as map_room_sprites above.
    bn::sprite_ptr arcana_card_sprites[arcana_card_count] = {
        bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0),
        bn::sprite_items::ground.create_sprite(0, 0),
    };
    bn::sprite_ptr arcana_equipped_sprites[2] = {
        bn::sprite_items::save_point.create_sprite(0, 0),
        bn::sprite_items::save_point.create_sprite(0, 0),
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
    bone_colossus_sprite.set_visible(false);
    bone_colossus_hitbox_sprite.set_visible(false);
    for(bn::sprite_ptr& s : bone_colossus_rib_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : map_room_sprites) { s.set_visible(false); }
    pause_cursor_sprite.set_visible(false);
    for(bn::sprite_ptr& s : map_seal_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : arcana_card_sprites) { s.set_visible(false); }
    for(bn::sprite_ptr& s : arcana_equipped_sprites) { s.set_visible(false); }

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

    int active_slot = run_slot_select_screen();

    game::level::spawn_point spawn;
    game::player_state player;

    if(platform_save::slot_has_data(active_slot))
    {
        game::save_slot_data save = platform_save::read_slot(active_slot);
        spawn = game::level::load_room(save.checkpoint_room_index);
        game::init_player(player, save.checkpoint_x, save.checkpoint_y);
        game::apply_save_slot_data(save, player);
        game::set_checkpoint(save.checkpoint_room_index, save.checkpoint_x, save.checkpoint_y);
    }
    else
    {
        spawn = game::level::load_room(0);
        game::init_player(player, spawn.x, spawn.y);
        game::set_checkpoint(0, spawn.x, spawn.y);
    }

    game::spawn_room_enemies();
    populate_room_tiles();

    int last_drawn_room = -1;

    // Frames left in the "just saved" blink on the save point sprite —
    // the only feedback a manual save gets, in place of a real "Saving..."
    // prompt (that needs text rendering, which nothing in this project
    // uses yet).
    constexpr int save_flash_duration_frames = 24;
    int save_flash_frames = 0;

    // SPEC.md section 9: 5-tab pause menu, shoulder buttons cycle. SELECT
    // toggles it (not START — START already means "save" while standing
    // in a save room, and reusing it here would make the same button do
    // two different things depending on where the player happens to be
    // standing).
    bool paused = false;
    int pause_tab = 0;
    int pause_blink_frame = 0;
    int arcana_cursor = 0; // which of the Arcana tab's 5 cards is highlighted

    // DEBUG ONLY — see render_debug_warp_screen's comment.
    bool debug_warp_active = false;
    int debug_warp_room = 0;
    bool debug_combo_prev_held = false;

    while(true)
    {
        bool debug_combo_held = bn::keypad::held(bn::keypad::key_type::L)
                && bn::keypad::held(bn::keypad::key_type::R)
                && bn::keypad::held(bn::keypad::key_type::START);

        if(debug_combo_held && ! debug_combo_prev_held && ! paused)
        {
            debug_warp_active = ! debug_warp_active;
            debug_warp_room = game::level::current_room_index();

            if(! debug_warp_active)
            {
                for(bn::sprite_ptr& s : map_room_sprites) { s.set_visible(false); }
                pause_cursor_sprite.set_visible(false);
            }
        }

        debug_combo_prev_held = debug_combo_held;

        if(debug_warp_active)
        {
            if(bn::keypad::pressed(bn::keypad::key_type::LEFT))
            {
                debug_warp_room = (debug_warp_room + game::level::total_room_count() - 1)
                        % game::level::total_room_count();
            }
            else if(bn::keypad::pressed(bn::keypad::key_type::RIGHT))
            {
                debug_warp_room = (debug_warp_room + 1) % game::level::total_room_count();
            }

            if(bn::keypad::pressed(bn::keypad::key_type::B))
            {
                debug_warp_active = false;
                for(bn::sprite_ptr& s : map_room_sprites) { s.set_visible(false); }
                pause_cursor_sprite.set_visible(false);
            }
            else if(bn::keypad::pressed(bn::keypad::key_type::A))
            {
                spawn = game::level::load_room(debug_warp_room);
                player.x = spawn.x;
                player.y = spawn.y;
                player.velocity_x = 0;
                player.velocity_y = 0;
                player.grounded = true;
                player.frames_since_grounded = 0;
                game::spawn_room_enemies();
                populate_room_tiles();
                last_drawn_room = debug_warp_room;

                debug_warp_active = false;
                for(bn::sprite_ptr& s : map_room_sprites) { s.set_visible(false); }
                pause_cursor_sprite.set_visible(false);
            }
            else
            {
                render_debug_warp_screen(debug_warp_room, pause_cursor_sprite, map_room_sprites);
            }

            bn::core::update();
            continue;
        }

        if(bn::keypad::pressed(bn::keypad::key_type::SELECT))
        {
            paused = ! paused;

            if(! paused)
            {
                for(bn::sprite_ptr& s : map_room_sprites) { s.set_visible(false); }
                for(bn::sprite_ptr& s : map_seal_sprites) { s.set_visible(false); }
                for(bn::sprite_ptr& s : arcana_card_sprites) { s.set_visible(false); }
                for(bn::sprite_ptr& s : arcana_equipped_sprites) { s.set_visible(false); }
                pause_cursor_sprite.set_visible(false);
            }
            else
            {
                arcana_cursor = 0;
            }
        }

        if(paused)
        {
            if(bn::keypad::pressed(bn::keypad::key_type::L))
            {
                pause_tab = (pause_tab + pause_tab_count - 1) % pause_tab_count;
            }
            else if(bn::keypad::pressed(bn::keypad::key_type::R))
            {
                pause_tab = (pause_tab + 1) % pause_tab_count;
            }

            // Arcana tab (SPEC.md's control map): LEFT/RIGHT moves the
            // card cursor, A equips whichever of the 5 it's on -- into
            // the action slot for cards 0-1, the attribute slot for 2-4.
            // Same LEFT/RIGHT-moves-a-cursor/A-confirms shape as the debug
            // warp screen above, just scoped to this tab instead of a
            // full-screen mode.
            if(pause_tab == arcana_tab_index)
            {
                if(bn::keypad::pressed(bn::keypad::key_type::LEFT))
                {
                    arcana_cursor = (arcana_cursor + arcana_card_count - 1) % arcana_card_count;
                }
                else if(bn::keypad::pressed(bn::keypad::key_type::RIGHT))
                {
                    arcana_cursor = (arcana_cursor + 1) % arcana_card_count;
                }
                else if(bn::keypad::pressed(bn::keypad::key_type::A))
                {
                    if(arcana_cursor < 2)
                    {
                        player.arcana.action = static_cast<game::arcana_action>(arcana_cursor);
                    }
                    else
                    {
                        player.arcana.attribute = static_cast<game::arcana_attribute>(arcana_cursor - 2);
                    }
                }
            }

            ++pause_blink_frame;
            render_pause_menu(pause_tab, pause_cursor_sprite, map_room_sprites, map_seal_sprites,
                    pause_blink_frame, player, arcana_cursor, arcana_card_sprites, arcana_equipped_sprites);
            bn::core::update();
            continue;
        }

        game::input_state input;
        input.left = bn::keypad::held(bn::keypad::key_type::LEFT);
        input.right = bn::keypad::held(bn::keypad::key_type::RIGHT);
        input.jump_held = bn::keypad::held(bn::keypad::key_type::A);
        input.attack_held = bn::keypad::held(bn::keypad::key_type::B);
        input.dodge_held = bn::keypad::held(bn::keypad::key_type::R);
        input.swap_held = bn::keypad::held(bn::keypad::key_type::L);
        input.up_held = bn::keypad::held(bn::keypad::key_type::UP);
        input.down_held = bn::keypad::held(bn::keypad::key_type::DOWN);

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

        game::update_enemies(player);

        game::attack_hitbox hitbox = game::get_attack_hitbox(player);
        game::apply_attacks_to_enemies(hitbox, player);
        game::apply_enemy_contact_to_player(player);
        game::sync_boss_room_seal();

        // Manual save only (SPEC.md section 9) -- START while standing in
        // a save room, same room/HP/MP checkpoint entering it already set.
        if(bn::keypad::pressed(bn::keypad::key_type::START) && game::level::current_room_is_save_room())
        {
            game::checkpoint_info checkpoint = game::current_checkpoint_info();
            game::save_slot_data save = game::build_save_slot_data(player, checkpoint.room_index,
                    checkpoint.x, checkpoint.y);
            platform_save::write_slot(active_slot, save);
            save_flash_frames = save_flash_duration_frames;
        }

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

        arcana_projectile_sprite.set_visible(player.arcana_projectile.active);

        if(player.arcana_projectile.active)
        {
            set_world_position(arcana_projectile_sprite, player.arcana_projectile.x, player.arcana_projectile.y);
        }

        if(game::level::current_room_is_save_room())
        {
            game::level::spawn_point room_spawn = game::level::current_room_spawn_point();
            set_world_position(save_point_sprite, room_spawn.x, room_spawn.y);
        }

        if(save_flash_frames > 0)
        {
            --save_flash_frames;
            save_point_sprite.set_visible((save_flash_frames / 3) % 2 == 0);
        }

        update_tile_sprites(tile_sprites);
        render_skeletons(skeleton_sprites, bone_sprites);
        render_bats(bat_sprites);
        render_archers(archer_sprites, arrow_sprites);
        render_zombies(zombie_sprites);
        render_bone_pillars(bone_pillar_sprites, fireball_sprites);
        render_fleamen(fleaman_sprites);
        render_medusa_heads(medusa_head_sprites);
        render_bone_colossus(bone_colossus_sprite, bone_colossus_hitbox_sprite, bone_colossus_rib_sprites);
        render_hud(hp_hud_sprites, mp_hud_sprites, player);

        bn::core::update();
    }
}
