#include "game.h"

// set by hot reload
Animation test_animation;
Animation shoot_animation;
TankStats default_tank;

EffectStats* bullet_hit_wall_effect_stats;
U32 bullet_hit_wall_effect_stats_count;
EffectStats* hit_tank_effect_stats;
U32 hit_tank_effect_stats_count;

#include <dlfcn.h>
#include <sys/stat.h>
void* hot_reload_lib = NULL;
I64 hot_reload_changed_time = 0;

#define RELOAD(sym) sym = *(typeof(sym)*) dlsym(hot_reload_lib, #sym)

static void check_hot_reload() {
    struct stat lib_stat;
    stat("./libhot_reload.so", &lib_stat);

    if (hot_reload_lib == NULL)
        goto reload;
    
    if (lib_stat.st_mtime > hot_reload_changed_time) {
        dlclose(hot_reload_lib);
        goto reload;
    }

    return;

reload:
    hot_reload_changed_time = lib_stat.st_mtime;
    hot_reload_lib = dlopen("./libhot_reload.so", RTLD_LAZY);

    if (hot_reload_lib == NULL) {
        fprintf(stderr, "could not open hot reload library\n");
        exit(1);
    }

    RELOAD(test_animation);
    RELOAD(shoot_animation);
    RELOAD(default_tank);
    RELOAD(bullet_hit_wall_effect_stats);
    RELOAD(bullet_hit_wall_effect_stats_count);
    RELOAD(hit_tank_effect_stats);
    RELOAD(hit_tank_effect_stats_count);
}

int main(void) {
    check_hot_reload();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game");
    SetTargetFPS(60);

    SceneTransition transition = { .next_scene_type = SceneType_MainMenu };
    //SceneTransition transition = { .next_scene_type = SceneType_Training };

    st = init_game_state();

    while (true) {
        switch (transition.next_scene_type) {
            case SceneType_Err: {
                printf("scene transition to Err\n");
                fprintf(stderr, "%s\n", transition.err);
                CloseWindow();
                return 1;
            }
            case SceneType_Exit: {
                printf("scene transition to Exit\n");
                CloseWindow();
                return 0;
            }
            case SceneType_MainMenu: {
                printf("scene transition to Main menu\n");
                transition = run_menu(main_menu, MAIN_MENU_SIZE);
                break;
            }
            case SceneType_ConfigMenu: {
                printf("scene transition to Config menu\n");
                transition = run_menu(config_menu, CONFIG_MENU_SIZE);
                break;
            }
            case SceneType_SetControlsMenu: {
                printf("scene transition to Set controls menu\n");
                transition = run_menu(controls_menu, CONTROLS_MENU_SIZE);
                break;
            }
            case SceneType_SetPlayerControls: {
                printf("scene transition to Set player controls\n");
                transition = run_set_player_controls(transition.set_player_controls_target);
                break;
            }
            case SceneType_SetTrainingControls: {
                printf("scene transition to set training controls\n");
                transition = run_set_training_controls(transition.set_training_controls_target);
                break;
            }
            case SceneType_SinglePlayer: {
                printf("scene transition to Singleplayer\n");
                transition = run_singleplayer();
                break;
            }
            case SceneType_Training: {
                printf("scene transition to Singleplayer\n");
                transition = run_training();
                break;
            }
            case SceneType_MultiPlayer: {
                printf("scene transition to Multiplayer\n");
                lockout = 20;
                transition = run_multiplayer();
                break;
            }
            default: {
                fprintf(stderr, "ERROR: scene transistion case %i not handled\n", transition.next_scene_type);
                CloseWindow();
                return 1;
            }
        }
    }

    dealloc_game_state(&st);
}

SceneTransition run_set_player_controls(PlayerControls* controls) {
    U64 button_idx = 0;

    // clear button press queue so you don't instantly map key
    while (GetKeyPressed()) {}

    while (true) {
        // update -------------------------------------------------
        if (WindowShouldClose()) {
            return (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
        }

        for (U64 gamepad = 0; IsGamepadAvailable(gamepad); ++gamepad) {
            for (U64 button = 0; button < 256; ++button) {
                if (IsGamepadButtonPressed(gamepad, button)) {
                    controls->map[button_idx] = (PlayerInputMapping) {
                        .source_type = PlayerInputSourceType_Gamepad,
                        .gamepad_input = { gamepad, button }
                    };
                    button_idx += 1;
                }
            }
        }

        U64 key = GetKeyPressed();
        if (key) {
            controls->map[button_idx] = (PlayerInputMapping) {
                .source_type = PlayerInputSourceType_Keyboard,
                .keyboard_input = key,
            };
            button_idx += 1;
        }

        if (button_idx == 9) {
            return (SceneTransition) {
                .next_scene_type = SceneType_SetControlsMenu,
            };
        }

        // draw ----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);
        
        const char* text;
        switch (button_idx) {
            case 0: text = "Press button for moving forwards..."; break;
            case 1: text = "Press button for moving backwards..."; break;
            case 2: text = "Press button for turning left..."; break;
            case 3: text = "Press button for turning right..."; break;
            case 4: text = "Press button for turning slowly..."; break;
            case 5: text = "Press button for attack 1..."; break;
            case 6: text = "Press button for attack 2..."; break;
            case 7: text = "Press button for attack 3..."; break;
            case 8: text = "Press button for attack 4..."; break;
            default: text = "ERROR";
        }

        U64 text_size = 40;
        U64 text_height = 40; // approximate

        U64 text_width = MeasureText(text, text_size);
        U64 text_x = (SCREEN_WIDTH - text_width) / 2;
        U64 text_y = (SCREEN_HEIGHT - text_height) / 2;

        DrawText(text, text_x, text_y, text_size, (Color) MENU_TEXT_COLOUR);

        EndDrawing();
    }

    return (SceneTransition) {
        .next_scene_type = SceneType_Err,
        .err = "invalid transition from run_set_player_controls"
    };
}

SceneTransition run_set_training_controls(TrainingControls* controls) {
    U64 button_idx = 0;

    // clear button press queue so you don't instantly map key
    while (GetKeyPressed()) {}

    while (true) {
        // update -------------------------------------------------
        if (WindowShouldClose()) {
            return (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
        }

        for (U64 gamepad = 0; IsGamepadAvailable(gamepad); ++gamepad) {
            for (U64 button = 0; button < 256; ++button) {
                if (IsGamepadButtonPressed(gamepad, button)) {
                    controls->map[button_idx] = (PlayerInputMapping) {
                        .source_type = PlayerInputSourceType_Gamepad,
                        .gamepad_input = { gamepad, button }
                    };
                    button_idx += 1;
                }
            }
        }

        U64 key = GetKeyPressed();
        if (key) {
            controls->map[button_idx] = (PlayerInputMapping) {
                .source_type = PlayerInputSourceType_Keyboard,
                .keyboard_input = key,
            };
            button_idx += 1;
        }

        if (button_idx == 9) {
            return (SceneTransition) {
                .next_scene_type = SceneType_SetControlsMenu,
            };
        }

        // draw ----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);
        
        const char* text;
        switch (button_idx) {
            case 0: text = "Press button for resetting training mode..."; break;
            default: text = "ERROR";
        }

        U64 text_size = 40;
        U64 text_height = 40; // approximate

        U64 text_width = MeasureText(text, text_size);
        U64 text_x = (SCREEN_WIDTH - text_width) / 2;
        U64 text_y = (SCREEN_HEIGHT - text_height) / 2;

        DrawText(text, text_x, text_y, text_size, (Color) MENU_TEXT_COLOUR);

        EndDrawing();
    }

    return (SceneTransition) {
        .next_scene_type = SceneType_Err,
        .err = "invalid transition from run_set_training_controls"
    };
}

SceneTransition run_menu(const MenuItem* menu, U32 menu_len) {
    U64 menu_idx = 0;

    while (true) {
        // update -------------------------------------------------
        if (WindowShouldClose()) {
            return (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
        }

        if (lockout == 0) {
            if (IsKeyPressed(KEY_DOWN) 
                    || IsKeyPressed(KEY_S)
                    || player_input_pressed(&player1_controls, PlayerInput_Backward)
                    || player_input_pressed(&player2_controls, PlayerInput_Backward)
            ) {
                menu_idx += 1;
                if (menu_idx == menu_len) menu_idx = 0;
            }

            if (IsKeyPressed(KEY_UP) 
                    || IsKeyPressed(KEY_W)
                    || player_input_pressed(&player1_controls, PlayerInput_Forward)
                    || player_input_pressed(&player2_controls, PlayerInput_Forward)
            ) {
                if (menu_idx == 0) menu_idx = menu_len;
                menu_idx -= 1;
            }
        }

        // draw ----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        U64 y = MENU_ITEM_Y_PADDING;
        U64 x = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;

        Rectangle rect = { x, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT };

        for (U64 i = 0; i < menu_len; ++i) {
            // background ------------------------------

            float roundness = 0.2f;
            U64 segments = 5;

            if (i != menu_idx) {
                DrawRectangleRounded(rect, roundness, segments, menu[i].base_colour);
            } else {
                DrawRectangleRounded(rect, roundness, segments, menu[i].selected_colour);
                DrawRectangleRoundedLines(rect, roundness, segments, 8.0, BLACK);
            }

            // text ------------------------------------

            U64 text_size = 40;
            U64 text_height = 40; // approximate

            U64 text_width = MeasureText(menu[i].text, text_size);
            U64 text_x = rect.x + (MENU_ITEM_WIDTH - text_width) / 2;
            U64 text_y = rect.y + (MENU_ITEM_HEIGHT - text_height) / 2;

            DrawText(menu[i].text, text_x, text_y, text_size, menu[i].text_colour);
            rect.y += MENU_ITEM_HEIGHT + MENU_ITEM_Y_PADDING;
        }

        EndDrawing();

        if (lockout == 0) {
            if (IsKeyPressed(KEY_ENTER)
                    || player_input_pressed(&player1_controls, PlayerInput_Attack1)
                    || player_input_pressed(&player2_controls, PlayerInput_Attack1)
            ) {
                return menu[menu_idx].action;
            }

            if (IsKeyPressed(KEY_BACKSPACE)
                    || player_input_pressed(&player1_controls, PlayerInput_Attack2)
                    || player_input_pressed(&player2_controls, PlayerInput_Attack2)
            ) {
                for (U64 i = 0; i < menu_len; ++i) {
                    if (strcmp(menu[i].text, "Back") == 0) {
                        return menu[i].action;
                    }
                }
            }
        }

        if (lockout > 0)
            lockout--;
    }
}

SceneTransition run_multiplayer() {
    reset_game_state(&st);
    insert_borders();

    TankRef p1 = insert_tank((TankInit) {
        .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 3 * SCREEN_HEIGHT / 4 },
        .stats = &default_tank,
        .body_colour = { 190, 33, 55, 255 },
        .controls = &player1_controls,
        .angle = -90.0f,
        .update = update_tank_player,
    });
    insert_player_hud(p1, (Vector2) { 0.0f, 0.0f }, (Vector2) { SCREEN_WIDTH * 0.5f, 20.0f });

    TankRef p2 = insert_tank((TankInit) {
        .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 1 * SCREEN_HEIGHT / 4 },
        .stats = &default_tank,
        .body_colour = {  33, 190, 55, 255 },
        .controls = &player2_controls,
        .angle = 90.0f,
        .update = update_tank_player,
    });
    insert_player_hud(p2, (Vector2) { SCREEN_WIDTH * 0.5f, 0.0f }, (Vector2) { SCREEN_WIDTH * 0.5f, 20.0f });

    Entity border = {
        .entity_type = ENTITY_TYPE_WALL,
        .entity_flags = 0,
        .collision_roundness = 10.0f,
        .on_collide = NULL,
        .draw = draw_wall,
        .update = NULL,
        .destroy = NULL,
        .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = SCREEN_HEIGHT / 2 },
        .collision_size = (Vector2) { .x = 100, .y = 50 },
    };
    insert_entity(border);

    SceneTransition transistion = {
        .next_scene_type = SceneType_MainMenu
    };

    while (true) {
        run_debug_update();
        run_entity_updates();
        run_collision_checks();

        BeginDrawing();
        ClearBackground((Color) { 200, 200, 200, 255 });

        draw_game_state();

        EndDrawing();

        if (WindowShouldClose()) {
            transistion = (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
            break;
        }

        if (st.finish)
            break;

        if (lockout > 0)
            lockout--;
    }

    lockout = 30;

    return transistion;
}

void draw_wall(Entity* e) {
    Vector2 pos = e->position;
    Vector2 size = e->collision_size;
    F32 r = e->collision_roundness;
    draw_rounded_rectangle(NULL, pos, size, r / fmin(size.x, size.y), GRAY);
}

void insert_borders() {
    Entity border = {
        .entity_type = ENTITY_TYPE_WALL,
        .entity_flags = 0,
        .collision_roundness = 0.0f,
        .on_collide = NULL,
        .draw = draw_wall,
        .update = NULL,
        .destroy = NULL,
    };

    F32 border_size = 10.0f;

    border.position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 0.0f };
    border.collision_size = (Vector2) { .x = SCREEN_WIDTH, .y = border_size };
    insert_entity(border);
    border.position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = SCREEN_HEIGHT };
    insert_entity(border);

    border.position = (Vector2) { .x = 0.0f, .y = SCREEN_HEIGHT / 2 };
    border.collision_size = (Vector2) { .x = border_size, .y = SCREEN_HEIGHT };
    insert_entity(border);
    border.position = (Vector2) { .x = SCREEN_WIDTH, .y = SCREEN_HEIGHT / 2 };
    insert_entity(border);
}

SceneTransition run_singleplayer() {
    reset_game_state(&st);

    insert_entity((Entity) {
        .entity_type = ENTITY_TYPE_WALL,
        .entity_flags = 0,
        .collision_roundness = 40.0f,
        .collision_size = { 70.0f, 50.0f },
        .position = { .x = SCREEN_WIDTH / 3, .y = SCREEN_HEIGHT / 3 },
        .on_collide = NULL,
        .draw = draw_wall,
        .update = NULL,
        .destroy = NULL,
    });

    insert_entity((Entity) {
        .entity_type = ENTITY_TYPE_WALL,
        .entity_flags = 0,
        .collision_roundness = 50.0f,
        .collision_size = { 70.0f, 50.0f },
        .position = { .x = 2 * SCREEN_WIDTH / 3, .y = 2 * SCREEN_HEIGHT / 3 },
        .on_collide = NULL,
        .draw = draw_wall,
        .update = NULL,
        .destroy = NULL,
    });

    insert_borders();

    TankRef player = insert_tank((TankInit) {
        .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 3 * SCREEN_HEIGHT / 4 },
        .stats = &default_tank,
        .body_colour = { 190, 33, 55, 255 },
        .controls = &player1_controls,
        .angle = -90.0f,
        .update = update_tank_player,
    });
    insert_player_hud(player, (Vector2) { 0.0f, 0.0f }, (Vector2) { SCREEN_WIDTH * 0.5f, 20.0f });

    SceneTransition transistion = {
        .next_scene_type = SceneType_MainMenu
    };

    while (true) {
        run_entity_updates();
        run_collision_checks();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_game_state();

        EndDrawing();

        if (WindowShouldClose()) {
            transistion = (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
            break;
        }
    }

    return transistion;
}

typedef struct {
    TankRef player;
    TankRef dummy;
} ResetTrainingRet;

static ResetTrainingRet reset_training() {
    reset_game_state(&st);

    insert_borders();

    TankRef player = insert_tank((TankInit) {
        .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 3 * SCREEN_HEIGHT / 4 },
        .stats = &default_tank,
        .body_colour = { 190, 33, 55, 255 },
        .controls = &player1_controls,
        .angle = -90.0f,
        .update = update_tank_player,
    });
    insert_player_hud(player, (Vector2) { 0.0f, 0.0f }, (Vector2) { SCREEN_WIDTH * 0.5f, 20.0f });

    TankRef dummy = insert_tank((TankInit) {
        .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 1 * SCREEN_HEIGHT / 4 },
        .stats = &default_tank,
        .body_colour = { 100, 100, 100, 255 },
        .controls = NULL,
        .angle = 90.0f,
        .update = update_tank_training_dummy,
    });
    insert_player_hud(dummy, (Vector2) { SCREEN_WIDTH * 0.5f, 0.0f }, (Vector2) { SCREEN_WIDTH * 0.5f, 20.0f });

    return (ResetTrainingRet) {
        .player = player,
        .dummy = dummy,
    };
}

SceneTransition run_training() {
    ResetTrainingRet ret = reset_training();
    TankRef player = ret.player;
    TankRef dummy = ret.dummy;
    (void)(player);
    (void)(dummy);

    SceneTransition transistion = {
        .next_scene_type = SceneType_MainMenu
    };

    bool playing = true;
    bool frame_advance = false;
    U32 frame_advance_down_timer = 0;
    bool has_saved_state = false;
    bool menu_showing = false;

    GameState saved_state = init_game_state();
    TrainingMenuState tm_state = {
    };

    while (true) {
        // training input ----------------------------------------------------

        if (training_input_pressed(&training_controls, TrainingInput_Reset)) {
            ret = reset_training();
            player = ret.player;
            dummy = ret.dummy;
        }

        if (training_input_pressed(&training_controls, TrainingInput_TogglePlaying))
            playing = !playing;

        if (training_input_down(&training_controls, TrainingInput_FrameAdvance)) {
            if (frame_advance_down_timer == 0) {
                playing = false;
                frame_advance = true;
            }

            if (frame_advance_down_timer == 30) {
                playing = false;
                frame_advance = true;
                frame_advance_down_timer = 29;
            } else {
                frame_advance_down_timer += 1;
            }
        } else {
            frame_advance_down_timer = 0;
        }

        if (training_input_down(&training_controls, TrainingInput_SaveState)) {
            has_saved_state = true;
            copy_game_state(&st, &saved_state);
        }

        if (training_input_down(&training_controls, TrainingInput_LoadState) && has_saved_state)
            copy_game_state(&saved_state, &st);

        if (training_input_pressed(&training_controls, TrainingInput_ToggleMenu))
            menu_showing = !menu_showing;

        // debug update ----------------------------------------------

        run_debug_update();

        // game loop ----------------------------------------------------

        if (playing || frame_advance) {
            run_entity_updates();
            run_collision_checks();
            frame_advance = false;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_game_state();
        
        if (menu_showing)
            draw_training_menu(&tm_state);
        EndDrawing();

        if (WindowShouldClose()) {
            transistion = (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
            break;
        }
    }

    return transistion;
}

EntityRef insert_entity(Entity e) {
    return arena_insert_Entity(&st.entities, e);
}

EntityRef entity_ref(Entity* e) {
    I64 idx = e - st.entities.backing;
    assert(idx < st.entities.tracking.element_num);
    U16 gen = st.entities.tracking.generations[idx];
    return (EntityRef) {
        .gen = gen,
        .idx = idx,
    };
}

TankRef tank_ref(Tank* t) {
    I64 idx = t - st.tanks.backing;
    assert(idx < st.tanks.tracking.element_num);
    U16 gen = st.tanks.tracking.generations[idx];
    return (TankRef) {
        .gen = gen,
        .idx = idx,
    };
}

void destroy_entity(EntityRef e_ref) {
    Entity* e = arena_lookup_Entity(&st.entities, e_ref);
    if (e->destroy != NULL) {
        e->destroy(e);
    }
    arena_remove_Entity(&st.entities, e_ref);
}

void run_collision_checks() {
    ArenaIter iter1 = arena_iter(&st.entities.tracking);

    while (true) {
        ArenaKey k1 = arena_iter_next(&iter1);
        if (k1.idx == ARENA_INVALID_IDX) break;
        Entity* e1 = arena_lookup_Entity(&st.entities, k1);
        if (e1->entity_flags & ENTITY_FLAGS_COLLISION_NONE)
            continue;

        EntityTypeMask etype1 = e1->entity_type;
        EntityTypeMask coll1 = e1->collision_mask;

        ArenaIter iter2 = iter1;

        while (true) {
            ArenaKey k2 = arena_iter_next(&iter2);
            if (k2.idx == ARENA_INVALID_IDX) break;
            Entity* e2 = arena_lookup_Entity(&st.entities, k2);
            EntityTypeMask etype2 = e2->entity_type;
            EntityTypeMask coll2 = e2->collision_mask;
            
            if (((coll1 & etype2) | (coll2 & etype1)) == 0) continue;

            if (!colliding(e1, e2)) continue;
            
            if ((coll1 & etype2) != 0 && e1->on_collide != NULL) e1->on_collide(e1, e2);
            if ((coll2 & etype1) != 0 && e2->on_collide != NULL) e2->on_collide(e2, e1);
        }
    }
}

static bool rect_colliding(Vector2 pos_a, Vector2 size_a, Vector2 pos_b, Vector2 size_b) {
    Vector2 diff = vector2_abs(vector2_sub(pos_a, pos_b));
    Vector2 size = vector2_add(size_a, size_b);
    bool x_inside = diff.x + COLLISION_EPSILON <= size.x;
    bool y_inside = diff.y + COLLISION_EPSILON <= size.y;
    return x_inside & y_inside;
}

static bool circle_colliding(Vector2 pos_a, F32 size_a, Vector2 pos_b, F32 size_b) {
    Vector2 diff = vector2_sub(pos_a, pos_b);
    F32 diff_len_sq = vector2_dot(diff, diff);
    F32 size = size_a + size_b;
    return diff_len_sq + COLLISION_EPSILON_SQ <= size*size;
}

// vector to move 'a' such that it is just outside of 'b'
// assumes collision occurs
Vector2 eject_collision(Entity* a, Entity* b) {
    Vector2 a_pos = a->position;
    F32 a_roundness = a->collision_roundness;
    Vector2 a_bound_size = a->collision_size;
    Vector2 b_pos = b->position;
    F32 b_roundness = b->collision_roundness;
    Vector2 b_bound_size = b->collision_size;

    Vector2 a_horiz_rect = {
        .x = a_bound_size.x,
        .y = a_bound_size.y - a_roundness,
    };
    Vector2 b_horiz_rect = {
        .x = b_bound_size.x,
        .y = b_bound_size.y - b_roundness,
    };

    bool horiz_inside = rect_colliding(a_pos, a_horiz_rect, b_pos, b_horiz_rect);
    F32 horiz_overlap = (a_bound_size.x + b_bound_size.x) - fabs(a_pos.x - b_pos.x) + COLLISION_EPSILON;

    Vector2 a_vert_rect = {
        .x = a_bound_size.x - a_roundness,
        .y = a_bound_size.y,
    };
    Vector2 b_vert_rect = {
        .x = b_bound_size.x - b_roundness,
        .y = b_bound_size.y,
    };

    bool vert_inside = rect_colliding(a_pos, a_vert_rect, b_pos, b_vert_rect);
    F32 vert_overlap = (a_bound_size.y + b_bound_size.y) - fabs(a_pos.y - b_pos.y) + COLLISION_EPSILON;

    if (vert_inside && horiz_inside) {
        if (horiz_overlap < vert_overlap) {
            vert_inside = false;
        } else {
            horiz_inside = false;
        }
    }

    if (horiz_inside) {
        return (Vector2) {
            .x = a_pos.x > b_pos.x ? horiz_overlap : -horiz_overlap,
            .y = 0.0f,
        };
    }

    if (vert_inside) {
        return (Vector2) {
            .x = 0.0f,
            .y = a_pos.y > b_pos.y ? vert_overlap : -vert_overlap,
        };
    }

    Vector2 a_circle_centre_offset = vector2_sub_f(a_bound_size, a_roundness);
    Vector2 b_circle_centre_offset = vector2_sub_f(b_bound_size, b_roundness);

    Vector2 circles[4] = {
        {.x =  1.0f, .y =  1.0f },
        {.x =  1.0f, .y = -1.0f },
        {.x = -1.0f, .y =  1.0f },
        {.x = -1.0f, .y = -1.0f },
    };

    F32 closest_dist_sq = 10000000.0f;
    Vector2 closest_eject = { .x = 0.0f, .y = 0.0f }; 

    F32 max_circle_dist = a_roundness + b_roundness;
    F32 max_circle_dist_sq = max_circle_dist * max_circle_dist;

    for (U64 i = 0; i < 4; ++i) {
        Vector2 a_c = vector2_add(a_pos, vector2_mul(a_circle_centre_offset, circles[i]));

        for (U64 j = 0; j < 4; ++j) {
            Vector2 b_c = vector2_add(b_pos, vector2_mul(b_circle_centre_offset, circles[j]));
            
            Vector2 diff = vector2_sub(a_c, b_c);
            F32 diff_len_sq = vector2_dot(diff, diff);
            if (diff_len_sq + COLLISION_EPSILON <= max_circle_dist_sq && diff_len_sq < closest_dist_sq) {
                closest_dist_sq = diff_len_sq;

                F32 diff_len = fsqrt(diff_len_sq);
                F32 overlap = max_circle_dist - diff_len + COLLISION_EPSILON;
                closest_eject = vector2_scale(diff, overlap / diff_len);
            }
        }
    }

    return closest_eject;
}

// does not handle rotation yet
bool colliding(Entity* a, Entity* b) {
    if ((a->entity_flags | b->entity_flags) & ENTITY_FLAGS_COLLISION_NONE)
        return false;

    // https://www.desmos.com/calculator/iroxa0pd0p
    // rounded rect collision can be broken into three stages:
    // 1. early bounding box test
    // 2. collision between the inner rectangles (two rectanges that cross cutting out the rounded corners)
    // 3. collision between the four circles making up the rounded edges

    Vector2 a_pos = a->position;
    F32 a_roundness = a->collision_roundness;
    Vector2 a_bound_size = a->collision_size;

    Vector2 b_pos = b->position;
    F32 b_roundness = b->collision_roundness;
    Vector2 b_bound_size = b->collision_size;

    // early bounding box test
    if (!rect_colliding(a_pos, a_bound_size, b_pos, b_bound_size))
        return false;

    Vector2 a_horiz_rect = {
        .x = a_bound_size.x,
        .y = a_bound_size.y - a_roundness,
    };
    Vector2 b_horiz_rect = {
        .x = b_bound_size.x,
        .y = b_bound_size.y - b_roundness,
    };

    bool horiz_inside = rect_colliding(a_pos, a_horiz_rect, b_pos, b_horiz_rect);

    Vector2 a_vert_rect = {
        .x = a_bound_size.x - a_roundness,
        .y = a_bound_size.y,
    };
    Vector2 b_vert_rect = {
        .x = b_bound_size.x - b_roundness,
        .y = b_bound_size.y,
    };

    bool vert_inside = rect_colliding(a_pos, a_vert_rect, b_pos, b_vert_rect);

    if (horiz_inside | vert_inside)
        return true;

    // circles :(

    Vector2 a_circle_centre_offset = vector2_sub_f(a_bound_size, a_roundness);
    Vector2 b_circle_centre_offset = vector2_sub_f(b_bound_size, b_roundness);

    Vector2 circles[4] = {
        {.x =  1.0f, .y =  1.0f },
        {.x =  1.0f, .y = -1.0f },
        {.x = -1.0f, .y =  1.0f },
        {.x = -1.0f, .y = -1.0f },
    };

    for (U64 i = 0; i < 4; ++i) {
        Vector2 a_c = vector2_add(a_pos, vector2_mul(a_circle_centre_offset, circles[i]));

        for (U64 j = 0; j < 4; ++j) {
            Vector2 b_c = vector2_add(b_pos, vector2_mul(b_circle_centre_offset, circles[j]));
            if (circle_colliding(a_c, a_roundness, b_c, b_roundness))
                return true;
        }
    }

    return false;
}

void run_debug_update() {
    if (IsMouseButtonPressed(1)) {
        st.debug_state.follow = NULL_REF;
    }

    if (!is_null_ref(st.debug_state.follow)) {
        Entity* follow = lookup_entity(&st, st.debug_state.follow);

        // stop following if entity was destroyed
        if (follow == NULL) {
            st.debug_state.follow = NULL_REF;
            goto after_follow;
        }

        // rest of follow update 
    }

after_follow:

    if (IsMouseButtonPressed(0)) {
        Vector2 click_pos = GetMousePosition();
        ArenaIter iter = arena_iter(&st.entities.tracking);

        // follow closest entity to mouse click
        EntityRef closest = NULL_REF;
        F32 closest_dist = 10000000.0f;
        while (1) {
            EntityRef k = arena_iter_next(&iter);
            if (k.idx == ARENA_INVALID_IDX) break;

            Entity* e = lookup_entity(&st, k);
            F32 dist = distance(click_pos, e->position);
            if (dist < closest_dist) {
                closest = k;
                closest_dist = dist;
            }
        }


        if (closest_dist < 5000.0f) {
            st.debug_state.follow = closest;
        } else {
            st.debug_state.follow = NULL_REF;
        }
    }
}

void draw_game_state() {
    Entity* debug_follow = NULL;

    if (!is_null_ref(st.debug_state.follow)) {
        debug_follow = lookup_entity(&st, st.debug_state.follow);
    }

    if (debug_follow != NULL) {
        Camera2D debug_cam;
        debug_cam.target = debug_follow->position;
        debug_cam.offset = (Vector2) { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
        debug_cam.rotation = 0.0f;
        debug_cam.zoom = 2.0f;
        
        BeginMode2D(debug_cam);
    }

    draw_entities();

    if (debug_follow != NULL) {
        EndMode2D();
    }
}

void draw_entities() {
    U64 layers = 1u << ENTITY_FLAGS_RENDER_LAYER_BITS;
    for (U64 i = 0; i < layers; ++i) {
        U32 cur_layer = ENTITY_FLAGS_RENDER_LAYER_1 * i;
        ArenaIter iter = arena_iter(&st.entities.tracking);

        while (true) {
            ArenaKey k = arena_iter_next(&iter);
            if (k.idx == ARENA_INVALID_IDX) break;
            Entity* e = arena_lookup_Entity(&st.entities, k);
            if ((e->entity_flags & ENTITY_FLAGS_RENDER_LAYER) == cur_layer)
                if (e->draw != NULL) 
                    e->draw(e);
        }
    }
}

void run_entity_updates() {
    check_hot_reload();

    ArenaIter iter = arena_iter(&st.entities.tracking);

    while (true) {
        ArenaKey k = arena_iter_next(&iter);
        if (k.idx == ARENA_INVALID_IDX) break;
        Entity* e = arena_lookup_Entity(&st.entities, k);

        if (e->update != NULL) e->update(e);
    }
}

void draw_debug_vector(Entity* e) {
    Vector2 vector = e->collision_size;
    Vector2 base = e->position;
    Vector2 dir = vector2_scale(normalize(vector), 2.0f);
    Vector2 p1 = vector2_add(base, (Vector2) { dir.y, -dir.x });
    Vector2 p2 = vector2_add(base, (Vector2) { -dir.y, dir.x });

    DrawTriangle(p1, p2, vector2_add(base, vector), BLACK);
}

void insert_debug_vector(Vector2 base, Vector2 vector) {
    insert_entity((Entity) {
        .entity_type = ENTITY_TYPE_DEBUG,
        .collision_size = vector,
        .position = base,
        .on_collide = NULL,
        .draw = draw_debug_vector,
        .update = NULL,
        .destroy = NULL,
    });
}


void handle_collision_tank(Entity* this, Entity* other) {
    EntityTypeMask entity_type = other->entity_type;
    if (entity_type & ENTITY_TYPE_WALL) {
        Tank* t = arena_lookup_Tank(&st.tanks, this->data_ref);
        Vector2 eject = eject_collision(this, other);
        this->position = vector2_add(this->position, eject);
        t->knockback_velocity = reflect(t->knockback_velocity, normalize(eject));
    } else if (entity_type & ENTITY_TYPE_TANK) {
        this->position = vector2_add(this->position, vector2_scale(eject_collision(this, other), 0.5f));
    }
}

void draw_tank(Entity* e) {
    Tank* t = arena_lookup_Tank(&st.tanks, e->data_ref);

    Color colour;
    Vector2 pos = e->position;
    switch (t->state) {
        case TankState_Normal: {
            colour = t->body_colour;
            break;
        }
        case TankState_Cooldown: {
            colour = t->body_colour;
            //colour = BLACK;
            break;
        }
        case TankState_Hitstop: {
            colour = BLUE;
            pos = vector2_add(pos, t->state_data.hitstop.position_delta);
            break;
        }
        case TankState_Knockback: {
            colour = BLUE;
            break;
        }
        default: {
            printf("unhandled tank state\n");
            break;
        }
    }

    AnimationFrame frame = default_animation_frame;
    if (t->anim != NULL) {
        frame = animation_frame_from_scratch(t->anim, t->anim_frame);
    }

    F32 size = t->stats->size;
    draw_circle(&frame, pos, size, colour);
    F32 head_size = size / 5;
    F32 head_offset = size - head_size;
    Vector2 head = vector2_add(pos, vector2_dir(t->angle, head_offset)); 
    draw_circle(&frame, head, head_size, WHITE);
}

// will fill in reference fields
TankRef insert_tank(TankInit init) {
    Tank t = {
        .stats = init.stats,
        .body_colour = init.body_colour,
        .controls = init.controls,
        .velocity = 0,
        .angle = init.angle,
        .angle_velocity = 0.0f,
        .health = init.stats->max_health,
        .knockback_velocity = { .x = 0.0f, .y = 0.0f },
        .state = TankState_Normal,
    };
    Entity e = {
        .entity_type = ENTITY_TYPE_TANK,
        .collision_mask = ENTITY_TYPE_TANK | ENTITY_TYPE_WALL | ENTITY_TYPE_BULLET,
        .collision_roundness = init.stats->size,
        .collision_size = { init.stats->size, init.stats->size },
        .entity_flags = ENTITY_FLAGS_RENDER_LAYER_2,
        .position = init.position,
        .on_collide = handle_collision_tank,
        .draw = draw_tank,
        .update = init.update,
        .destroy = destroy_tank,
    };

    TankRef t_ref = arena_insert_Tank(&st.tanks, t);
    e.data_ref = t_ref;
    EntityRef e_ref = insert_entity(e);
    arena_lookup_Tank(&st.tanks, t_ref)->e = e_ref;

    return t_ref;
}

void destroy_tank(Entity* t) {
    arena_remove_Tank(&st.tanks, t->data_ref);
}

void hit_tank(Entity* e, Tank* t, HitInfo info) {
    (void)(e);
    set_tank_state(t, TankState_Hitstop, NULL);
    t->velocity = 0.0f;
    e->position = vector2_add(e->position, vector2_scale(info.knockback, 0.5f));
    t->health -= info.damage;
    if (t->health < 0.0f)
        t->health = 0.0f;

    F32 v = length(info.knockback);
    F32 d_f = t->stats->knockback_decay_factor;
    F32 d_c = t->stats->knockback_decay_constant;

    U32 kb_frames = 0;

    while (v > 0.5f) {
        v = v * (1.0f / d_f) - d_c;
        kb_frames += 1;
    }

    t->state_data.hitstop.queued_knockback_timer = kb_frames;
    t->state_data.hitstop.queued_knockback = info.knockback;
    t->state_data.hitstop.timer = 5;

    F32 angle = vector2_angle(info.knockback);
    for (U32 i = 0; i < hit_tank_effect_stats_count; ++i) {
        insert_effect((EffectInit) {
            .position = info.hit_position,
            .angle = angle,
            .stats = &hit_tank_effect_stats[i],
        });
    }
}

static void advance_tank_state(Entity* e, Tank* t, U64 control_states) {
    TankStats* stats = t->stats;

    bool actionable = false;
    bool moveable = false;

switch_state:
    switch (t->state) {
        case TankState_Normal: {
            actionable = true;
            moveable = true;
            break;
        }
        case TankState_Cooldown: {
            moveable = true;
            if (t->state_data.cooldown_timer) {
                t->state_data.cooldown_timer--;
                break;
            } else {
                set_tank_state(t, TankState_Normal, NULL);
                goto switch_state;
            }
        }
        case TankState_Hitstop: {
            if (t->state_data.hitstop.timer) {
                F32 delta = (F32)t->state_data.hitstop.timer * 0.5f;
                t->state_data.hitstop.position_delta = vector2_dir(random_angle(), delta);
                t->state_data.hitstop.timer--;
                return; // does not advance physics
            } else {
                t->knockback_velocity = t->state_data.hitstop.queued_knockback;
                U32 knockback_timer = t->state_data.hitstop.queued_knockback_timer;

                set_tank_state(t, TankState_Knockback, NULL);
                t->state_data.knockback_timer = knockback_timer;
                goto switch_state;
            }
        }
        case TankState_Knockback: {
            if (t->state_data.knockback_timer) {
                t->state_data.knockback_timer--;
                break;
            } else {
                set_tank_state(t, TankState_Normal, NULL);
                goto switch_state;
            }
            break;
        }
        default: {
            printf("unhandled tank state\n");
            break;
        }
    }

    // physics ---------------------------------------------------------

    bool forward_down = false;
    bool backward_down = false;
    bool left_down = false;
    bool right_down = false;
    bool turn_mod_down = false;
    if (moveable) {
        forward_down = (control_states & (1 << PlayerInput_Forward)) != 0;
        backward_down = (control_states & (1 << PlayerInput_Backward)) != 0;
        left_down = (control_states & (1 << PlayerInput_TurnLeft)) != 0;
        right_down = (control_states & (1 << PlayerInput_TurnRight)) != 0;
        turn_mod_down = (control_states & (1 << PlayerInput_TurnModifier)) != 0;
    }

    if (forward_down && !backward_down) {
        if (t->velocity < 0.0f)
            t->velocity = 0.0f;
        t->velocity += stats->acceleration;
    }
    if (backward_down && !forward_down) {
        if (t->velocity > 0.0f)
            t->velocity = 0.0f;
        t->velocity -= stats->acceleration;
    }
    if (!forward_down && !backward_down) t->velocity /= stats->velocity_decay;

    if (left_down && !right_down) {
        if (t->angle_velocity > 0.0f)
            t->angle_velocity = 0.0f;
        t->angle_velocity -= stats->angle_acceleration;
    }
    if (right_down && !left_down) {
        if (t->angle_velocity < 0.0f)
            t->angle_velocity = 0.0f;
        t->angle_velocity += stats->angle_acceleration;
    }
    if (!left_down && !right_down) t->angle_velocity /= stats->angle_velocity_decay;

    float max_angle_speed = stats->angle_max_speed_fast;
    if (turn_mod_down) {
        max_angle_speed = stats->angle_max_speed;
    }

    Vector2 prev_vel = t->knockback_velocity;
    F32 prev_vel_len = length(prev_vel);
    if (prev_vel_len > 0.01f) {
        F32 factor = 1.0f / stats->knockback_decay_factor - stats->knockback_decay_constant / prev_vel_len;
        if (factor > 0.0f) {
            t->knockback_velocity = vector2_scale(prev_vel, factor);
        } else {
            t->knockback_velocity = (Vector2) { .x = 0.0f, .y = 0.0f };
        }
    } else {
        t->knockback_velocity = (Vector2) { .x = 0.0f, .y = 0.0f };
    }

    t->velocity = clamp(t->velocity, -stats->max_speed, stats->max_speed);
    t->angle_velocity = clamp(t->angle_velocity, -max_angle_speed, max_angle_speed);
    t->angle += t->angle_velocity;
    while (t->angle >= 360.0f) t->angle -= 360.0f;
    while (t->angle < 0.0f) t->angle += 360.0f;

    Vector2 update = vector2_add(
        vector2_dir(t->angle, t->velocity),
        t->knockback_velocity
    );
    e->position = vector2_add(e->position, update);

    e->position.x = clamp(e->position.x, 0, SCREEN_WIDTH);
    e->position.y = clamp(e->position.y, 0, SCREEN_HEIGHT);

    t->anim_frame += 1.0f;

    // bullets -------------------------------------------------------

    if (actionable) {
        if (control_states & (1 << PlayerInput_Attack1)) {
            insert_bullet(
                (Bullet) {
                    .owner = tank_ref(t),
                    .direction = vector2_dir(t->angle, 1.0),
                    .speed = 15.0,
                },
                (Entity) {
                    .entity_type = ENTITY_TYPE_BULLET,
                    .collision_mask = ENTITY_TYPE_TANK | ENTITY_TYPE_WALL,
                    .collision_roundness = 8.0f,
                    .collision_size = { 8.0f, 8.0f },
                    .entity_flags = 0,
                    .position = e->position,
                    .on_collide = handle_collision_bullet,
                    .draw = draw_bullet,
                    .update = update_bullet,
                    .destroy = destroy_bullet,
                }
            );
            set_tank_state(t, TankState_Cooldown, &shoot_animation);
            t->state_data.cooldown_timer = t->stats->bullet_cooldown;
        } else if (control_states & (1 << PlayerInput_Attack2)) {
        }
    }
}

void set_tank_state(Tank* t, TankState state, Animation* anim) {
    t->state = state;
    t->anim_frame = 0.0f;
    t->anim = anim;
}

void update_tank_player(Entity* e) {
    // physics -----------------------------------------------------------------------

    U64 control_states = 0;

    Tank* player = arena_lookup_Tank(&st.tanks, e->data_ref);
    PlayerControls* controls = player->controls;
    if (controls != NULL && lockout == 0 && lockout == 0) {
        control_states |= player_input_down(controls, PlayerInput_Forward) << PlayerInput_Forward;
        control_states |= player_input_down(controls, PlayerInput_Backward) << PlayerInput_Backward;
        control_states |= player_input_down(controls, PlayerInput_TurnLeft) << PlayerInput_TurnLeft;
        control_states |= player_input_down(controls, PlayerInput_TurnRight) << PlayerInput_TurnRight;
        control_states |= player_input_down(controls, PlayerInput_TurnModifier) << PlayerInput_TurnModifier;
        control_states |= player_input_pressed(controls, PlayerInput_Attack1) << PlayerInput_Attack1;
        control_states |= player_input_pressed(controls, PlayerInput_Attack2) << PlayerInput_Attack2;
        control_states |= player_input_pressed(controls, PlayerInput_Attack3) << PlayerInput_Attack3;
        control_states |= player_input_pressed(controls, PlayerInput_Attack4) << PlayerInput_Attack4;
    }

    advance_tank_state(e, player, control_states);

    if (player->health <= 0.0f) {
        st.finish = true;
    }
}

void update_tank_training_dummy(Entity* e) {
    Tank* t = arena_lookup_Tank(&st.tanks, e->data_ref);
    advance_tank_state(e, t, 0);

    if (actionable_state(t->state)) {
        t->health = t->stats->max_health;
    }
}


// BULLET ---------------------------------------------------

void update_bullet(Entity* e) {
    Bullet* b = arena_lookup_Bullet(&st.bullets, e->data_ref);
    e->position = vector2_add(e->position, vector2_scale(b->direction, b->speed));
}

void handle_collision_bullet(Entity* this, Entity* other) {
    Bullet *b = arena_lookup_Bullet(&st.bullets, this->data_ref);

    if (other->entity_type & ENTITY_TYPE_WALL) {
        Vector2 eject = eject_collision(this, other);
        Vector2 pos = vector2_add(this->position, eject);
        F32 angle = vector2_angle(eject);

        for (U32 i = 0; i < bullet_hit_wall_effect_stats_count; ++i) {
            insert_effect((EffectInit) {
                .position = pos,
                .angle = angle,
                .stats = &bullet_hit_wall_effect_stats[i],
            });
        }
        destroy_entity(entity_ref(this));
    } else if (other->entity_type & ENTITY_TYPE_TANK) {
        if (arena_key_equal(other->data_ref, b->owner))
            return;

        Tank* t = arena_lookup_Tank(&st.tanks, other->data_ref);
        HitInfo info = {
            .knockback = vector2_scale(b->direction, 30.0f),
            .hit_position = this->position,
            .damage = 10.0,
        };
        hit_tank(other, t, info);
        destroy_entity(entity_ref(this));
    }
}

void draw_bullet(Entity* e) {
    //Bullet* b = arena_lookup_Bullet(&bullets, e->data_ref);
    draw_circle(NULL, e->position, e->collision_roundness, RED);
}

// will fill in entity field in tank
BulletRef insert_bullet(Bullet t, Entity e) {
    BulletRef b_ref = arena_insert_Bullet(&st.bullets, t);
    e.data_ref = b_ref;
    EntityRef e_ref = insert_entity(e);
    arena_lookup_Bullet(&st.bullets, b_ref)->e = e_ref;
    return b_ref;
}

void destroy_bullet(Entity* t) {
    arena_remove_Bullet(&st.bullets, t->data_ref);
}

EntityRef insert_player_hud(TankRef player, Vector2 hud_pos, Vector2 hud_size) {
    return insert_entity((Entity) {
        .data_ref = player,
        .entity_type = ENTITY_TYPE_HUD,
        .collision_mask = 0,
        .entity_flags = ENTITY_FLAGS_COLLISION_NONE | ENTITY_FLAGS_RENDER_LAYER_3,
        .collision_size = hud_size,
        .position = hud_pos,
        .on_collide = NULL,
        .draw = draw_player_hud,
        .update = NULL,
        .destroy = NULL,
    });
}

void draw_player_hud(Entity* e) {
    Tank* t = arena_lookup_Tank(&st.tanks, e->data_ref);
    if (t == NULL) {
        printf("hud removed\n");
        destroy_entity(entity_ref(e));
        return;
    }

    Vector2 pos = e->position;
    Vector2 size = e->collision_size;

    F32 width = ((F32)t->health / (F32)t->stats->max_health) * size.x;

    Rectangle r = {
        .x = pos.x,
        .y = pos.y,
        .width = width,
        .height = size.y,
    };

    Color colour = t->body_colour;

    DrawRectangleRec(r, colour);
}

void draw_training_menu(TrainingMenuState* tm_state) {
    (void)(tm_state);
    DrawRectangle(0, 0, 200, 200, BLACK);
}

AnimationFrame animation_frame_from_scratch(Animation* anim, F32 frame) {
    AnimationFrame f = default_animation_frame;
    U32 idx = 0;
    F32 last_waited_frame = 0.0f;
    while (1) {
        AnimDirective dir = anim->data[idx].directive;
        AnimDirective command = dir & ANIM_DIRECTIVE_COMMAND_MASK;
        U64 offset = (dir & ANIM_DIRECTIVE_OFFSET_MASK) >> ANIM_DIRECTIVE_OFFSET_START;
        U64 len = (dir & ANIM_DIRECTIVE_LEN_MASK) >> ANIM_DIRECTIVE_LEN_START;

        switch (command) {
            case ANIM_DIRECTIVE_COMMAND_END: return f;
            case ANIM_DIRECTIVE_COMMAND_WAIT_UNTIL: {
                last_waited_frame = anim->data[idx+1].f;
                if (frame < last_waited_frame)
                    return f;
                idx += 2;
                break;
            }
            case ANIM_DIRECTIVE_COMMAND_WAIT_FOR: {
                last_waited_frame += anim->data[idx+1].f;
                if (frame < last_waited_frame)
                    return f;
                idx += 2;
                break;
            }
            case ANIM_DIRECTIVE_COMMAND_SET: {
                F32* frame_f32 = (F32*) &f;
                for (U64 i = 0; i < len; ++i) {
                    frame_f32[offset+i] = anim->data[idx+1+i].f;
                }
                idx += len+1;
                break;
            }
            case ANIM_DIRECTIVE_COMMAND_LINEAR_INCREMENT: {
                F32* frame_f32 = (F32*) &f;
                F32 frame_diff = frame - last_waited_frame;
                for (U64 i = 0; i < len; ++i) {
                    frame_f32[offset+i] += anim->data[idx+1+i].f * frame_diff;
                }
                idx += len+1;
                break;
            }
            default: {
                fprintf(stderr, "fn animation_frame_from_scratch: unhandled case\n");
                return f;
            }
        }
    }
}

void draw_ellipse(AnimationFrame* anim, Vector2 centre, Vector2 radius, Color colour) {
    if (anim != NULL) {
        centre = vector2_add(centre, anim->position);
        radius = vector2_mul(radius, anim->scale);
        colour.r = colour.r*anim->colour_mul.r + anim->colour_sum.r;
        colour.g = colour.g*anim->colour_mul.g + anim->colour_sum.g;
        colour.b = colour.b*anim->colour_mul.b + anim->colour_sum.b;
        colour.a = colour.a*anim->colour_mul.a + anim->colour_sum.a;
    }

    DrawEllipse(centre.x, centre.y, radius.x+AA_SIZE_INCREMENT*2.0, radius.y+AA_SIZE_INCREMENT*2.0, Fade(colour, 0.2));
    DrawEllipse(centre.x, centre.y, radius.x+AA_SIZE_INCREMENT    , radius.y+AA_SIZE_INCREMENT    , Fade(colour, 0.5));
    DrawEllipse(centre.x, centre.y, radius.x                      , radius.y                      , colour);
}

void draw_circle(AnimationFrame* anim, Vector2 centre, F32 radius, Color colour) {
    draw_ellipse(anim, centre, (Vector2) { radius, radius }, colour);
}


// anim may be null
void draw_rounded_rectangle(AnimationFrame* anim, Vector2 centre, Vector2 size, F32 roundness, Color colour) {
    Colourf colour_f = to_colourf(colour);
    if (anim != NULL) {
        centre = vector2_add(centre, anim->position);
        size = vector2_mul(size, anim->scale);
        roundness *= fmin(anim->scale.x, anim->scale.y);
        colour_f.r = colour_f.r*anim->colour_mul.r + anim->colour_sum.r;
        colour_f.g = colour_f.g*anim->colour_mul.g + anim->colour_sum.g;
        colour_f.b = colour_f.b*anim->colour_mul.b + anim->colour_sum.b;
        colour_f.a = colour_f.a*anim->colour_mul.a + anim->colour_sum.a;
    }

    if (size.x < roundness)
        roundness = size.x;
    if (size.y < roundness)
        roundness = size.y;

    colour = to_color(colour_f);

    Rectangle r = {
        .x = centre.x - size.x,
        .y = centre.y - size.y,
        .width = 2*size.x,
        .height = 2*size.y,
    };

    r.x -= AA_SIZE_INCREMENT*2.0f;
    r.y -= AA_SIZE_INCREMENT*2.0f;
    r.width += AA_SIZE_INCREMENT*4.0f;
    r.height += AA_SIZE_INCREMENT*4.0f;
    DrawRectangleRounded(r, roundness, 16, Fade(colour, 0.2));

    r.x += AA_SIZE_INCREMENT;
    r.y += AA_SIZE_INCREMENT;
    r.width -= AA_SIZE_INCREMENT*2.0f;
    r.height -= AA_SIZE_INCREMENT*2.0f;
    DrawRectangleRounded(r, roundness, 16, Fade(colour, 0.5));

    r.x += AA_SIZE_INCREMENT;
    r.y += AA_SIZE_INCREMENT;
    r.width -= AA_SIZE_INCREMENT*2.0f;
    r.height -= AA_SIZE_INCREMENT*2.0f;
    DrawRectangleRounded(r, roundness, 16, colour);
}

static F32 compute_range(F32 start, F32 range, I8 variance) {
    return start + range * (F32)variance * 0.0078125f; // variance / 128.0f
}

void update_effect(Entity* e) {
    Effect* ef = arena_lookup_Effect(&st.effects, e->data_ref);

    if (!is_null_ref(ef->follow)) {
        Entity* parent = lookup_entity(&st, ef->follow);
        
        if (parent == NULL) {
            destroy_entity(entity_ref(e));
            return;
        }

        e->position = parent->position;
    }

    EffectStats* stats = ef->stats;
    F32 lifetime = ef->lifetime;
    U64 particle_count = stats->particle_count;
    F32 initial_spawn_rate = stats->initial_spawn_rate;

    F32 new_lifetime = lifetime + 1.0;
    U64 spawned = (U64)(lifetime*initial_spawn_rate);
    U64 new_spawned = (U64)(new_lifetime*initial_spawn_rate);
    if (new_spawned > particle_count)
        new_spawned = particle_count;

    // initial spawning of particles
    while (spawned < new_spawned) {
        ef->spawned[spawned] = random_effect_particle();
        spawned += 1;
    }

    bool spawning = ef->lifetime < stats->total_lifetime;
    bool all_dead = true;

    // update spawned
    for (U64 i = 0; i < new_spawned; ++i) {
        EffectParticle p = ef->spawned[i];

        F32 total_p_lifetime = compute_range(stats->particle_lifetime, stats->particle_lifetime_variance, p.lifetime_delta);
        U64 progress_change = (U64) (65536.0f / total_p_lifetime);
        U64 old_progress = p.progress; 
        U64 new_progress = old_progress + progress_change;
        if (new_progress >= 65536ul) {
            if (spawning) {
                all_dead = false;
                p = random_effect_particle();
            } else {
                p.flags &= (~PARTICLE_FLAGS_ALIVE);
            }
        } else {
            all_dead = false;
            p.progress = (U16) new_progress;
        }

        ef->spawned[i] = p;
    }

    // if all particles are dead
    if (all_dead) {
        destroy_entity(entity_ref(e));
    }

    ef->lifetime += 1.0;
}

void draw_effect(Entity* e) {
    Effect* ef = arena_lookup_Effect(&st.effects, e->data_ref);
    EffectParticle* particles = ef->spawned;
    EffectStats* stats = ef->stats;

    for (U64 i = 0; i < stats->particle_count; ++i) {
        EffectParticle p = particles[i];
        if (p.flags & PARTICLE_FLAGS_ALIVE) {
            F32 progress = (F32)p.progress / 65536.0f;
            F32 time = progress * stats->particle_lifetime;

            F32 angle = compute_range(ef->angle, stats->angle_variance, p.angle_delta);
            F32 speed = compute_range(stats->initial_speed, stats->speed_variance, p.speed_delta);
            F32 offset = time * speed + time*time*0.5f*stats->acceleration;
            Vector2 pos = vector2_add(e->position, vector2_dir(angle, offset));
            F32 size = compute_range(stats->size, stats->size_variance, p.size_delta);

            draw_rounded_rectangle(NULL, pos, (Vector2) { size, size }, stats->roundness, stats->colour);
        }
    }
}

EffectRef insert_effect(EffectInit init) {
    Effect ef = {
        .stats = init.stats,
        .angle = init.angle,
        .follow = init.follow,
        .lifetime = 0.0f,
        .spawned = malloc(init.stats->particle_count * sizeof(EffectParticle)),
    };
    
    memset(ef.spawned, 0, init.stats->particle_count * sizeof(EffectParticle));

    Entity et = {
        .entity_type = ENTITY_TYPE_EFFECT,
        .collision_roundness = 0.0f,
        .collision_size = { 0.0f, 0.0f },
        .entity_flags = ENTITY_FLAGS_COLLISION_NONE | init.stats->render_layer,
        .position = init.position,
        .on_collide = NULL,
        .draw = draw_effect,
        .update = update_effect,
        .destroy = destroy_effect,
    };

    EffectRef ef_ref = arena_insert_Effect(&st.effects, ef);
    et.data_ref = ef_ref;
    EntityRef et_ref = insert_entity(et);
    arena_lookup_Effect(&st.effects, ef_ref)->e = et_ref;

    return et_ref;
}

void destroy_effect(Entity* e) {
    Effect* ef = arena_lookup_Effect(&st.effects, e->data_ref);
    free(ef->spawned);
    arena_remove_Effect(&st.effects, e->data_ref);
}

//void draw_general_hud();
