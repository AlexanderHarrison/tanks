#include "game.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game");
    SetTargetFPS(60);

    SceneTransition transition = { .next_scene_type = SceneType_MainMenu };
    //SceneTransition transition = { .next_scene_type = SceneType_SinglePlayer };

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
}

SceneTransition run_multiplayer() {
    reset_game_state(&st);

    return (SceneTransition) {
        .next_scene_type = SceneType_MainMenu
    };
}

void draw_wall(Entity* e) {
    Vector2 pos = e->position;
    Vector2 size = e->collision_size;
    F32 r = e->collision_roundness;

    Rectangle rect = {
        .x = pos.x - size.x,
        .y = pos.y - size.y,
        .width = size.x*2.0f,
        .height = size.y*2.0f,
    };
    draw_rectangle_rounded_aa(rect, r / fmin(size.x, size.y), GRAY);
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

    insert_tank(
        (Tank) {
            .stats = &default_tank,
            .body_colour = { 190, 33, 55, 255 },
            .controls = &player1_controls,
            .velocity = 0,
            .angle = -90.0f,
            .angle_velocity = 0.0f,
            .bullet_timer = 0,
            .health = 100,
        },
        (Entity) {
            .entity_type = ENTITY_TYPE_TANK,
            .collision_mask = ENTITY_TYPE_WALL,
            .collision_roundness = default_tank.size,
            .collision_size = { default_tank.size, default_tank.size },
            .entity_flags = ENTITY_FLAGS_RENDER_LAYER_1,
            .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 3 * SCREEN_HEIGHT / 4 },
            .on_collide = handle_collision_tank_player,
            .draw = draw_tank,
            .update = update_tank_player,
            .destroy = destroy_tank,
        }
    );

    SceneTransition transistion = {
        .next_scene_type = SceneType_MainMenu
    };

    while (true) {
        run_entity_updates();
        run_collision_checks();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_entities();
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

static void reset_training() {
    reset_game_state(&st);

    // player
    insert_tank(
        (Tank) {
            .stats = &default_tank,
            .body_colour = { 190, 33, 55, 255 },
            .controls = &player1_controls,
            .velocity = 0,
            .angle = -90.0f,
            .angle_velocity = 0.0f,
            .bullet_timer = 0,
            .health = 100,
        },
        (Entity) {
            .entity_type = ENTITY_TYPE_TANK,
            .collision_mask = ENTITY_TYPE_WALL,
            .collision_roundness = default_tank.size,
            .collision_size = { default_tank.size, default_tank.size },
            .entity_flags = ENTITY_FLAGS_RENDER_LAYER_1,
            .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 3 * SCREEN_HEIGHT / 4 },
            .on_collide = handle_collision_tank_player,
            .draw = draw_tank,
            .update = update_tank_player,
            .destroy = destroy_tank,
        }
    );

    // training dummy
    insert_tank(
        (Tank) {
            .stats = &default_tank,
            .body_colour = { 100, 100, 100, 255 },
            .controls = NULL,
            .velocity = 0,
            .angle = 90.0f,
            .angle_velocity = 0.0f,
            .bullet_timer = 0,
            .health = 100,
        },
        (Entity) {
            .entity_type = ENTITY_TYPE_TANK,
            .collision_mask = ENTITY_TYPE_WALL,
            .collision_roundness = default_tank.size,
            .collision_size = { default_tank.size, default_tank.size },
            .entity_flags = ENTITY_FLAGS_RENDER_LAYER_1,
            .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 1 * SCREEN_HEIGHT / 4 },
            .on_collide = handle_collision_tank_player,
            .draw = draw_tank,
            .update = update_tank_training_dummy,
            .destroy = destroy_tank,
        }
    );

    insert_borders();
}

SceneTransition run_training() {
    reset_training();

    SceneTransition transistion = {
        .next_scene_type = SceneType_MainMenu
    };

    bool playing = true;
    bool frame_advance = false;

    U32 frame_advance_down_timer = 0;

    while (true) {
        // training input ----------------------------------------------------

        if (training_input_pressed(&training_controls, TrainingInput_Reset)) {
            reset_training();
        }

        if (training_input_pressed(&training_controls, TrainingInput_TogglePlaying)) {
            playing = !playing;
        }

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

        // game loop ----------------------------------------------------

        if (playing || frame_advance) {
            run_entity_updates();
            run_collision_checks();
            frame_advance = false;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_entities();
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


void handle_collision_tank_player(Entity* this, Entity* other) {
    if (other->entity_type & ENTITY_TYPE_WALL) {
        this->position = vector2_add(this->position, eject_collision(this, other));
    }
}

void draw_tank(Entity* e) {
    Tank* t = arena_lookup_Tank(&st.tanks, e->data_ref);

    Color colour;
    if (t->bullet_timer != 0) {
        colour = BLACK;
    } else {
        colour = t->body_colour;
    }
    draw_circle_v_aa(e->position, t->stats->size, colour);
    Vector2 head = vector2_add(e->position, vector2_dir(t->angle, 16)); 
    draw_circle_v_aa(head, 4, WHITE);
}

// will fill in reference fields
TankInsertReturn insert_tank(Tank t, Entity e) {
    TankRef t_ref = arena_insert_Tank(&st.tanks, t);
    e.data_ref = t_ref;
    EntityRef e_ref = insert_entity(e);
    arena_lookup_Tank(&st.tanks, t_ref)->e = e_ref;

    return (TankInsertReturn) {
        .e = e_ref,
        .t = t_ref,
    };
}

void destroy_tank(Entity* t) {
    arena_remove_Tank(&st.tanks, t->data_ref);
}

static void advance_tank_state(Entity* e, Tank* t, U64 control_states) {
    const TankStats* stats = t->stats;

    // physics ---------------------------------------------------------

    bool forward_down = (control_states & (1 << PlayerInput_Forward)) != 0;
    bool backward_down = (control_states & (1 << PlayerInput_Backward)) != 0;
    bool left_down = (control_states & (1 << PlayerInput_TurnLeft)) != 0;
    bool right_down = (control_states & (1 << PlayerInput_TurnRight)) != 0;

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
    if (control_states & (1 << PlayerInput_TurnModifier)) {
        max_angle_speed = stats->angle_max_speed;
    }

    t->velocity = clamp(t->velocity, -stats->max_speed, stats->max_speed);
    t->angle_velocity = clamp(t->angle_velocity, -max_angle_speed, max_angle_speed);
    t->angle += t->angle_velocity;
    while (t->angle >= 360.0f) t->angle -= 360.0f;
    while (t->angle < 0.0f) t->angle += 360.0f;

    Vector2 update = vector2_dir(t->angle, t->velocity);
    e->position = vector2_add(e->position, update);

    e->position.x = clamp(e->position.x, 0, SCREEN_WIDTH);
    e->position.y = clamp(e->position.y, 0, SCREEN_HEIGHT);

    // bullets -------------------------------------------------------

    if (t->bullet_timer != 0) {
        --t->bullet_timer;
    }

    if (control_states & (1 << PlayerInput_Attack1)) {
        // 1 frame buffer
        if (t->bullet_timer <= 1) {
            insert_bullet(
                (Bullet) {
                    .direction = vector2_dir(t->angle, 1.0),
                    .speed = 10.0,
                },
                (Entity) {
                    .entity_type = ENTITY_TYPE_BULLET,
                    .collision_mask = ENTITY_TYPE_WALL,
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
            t->bullet_timer = stats->bullet_cooldown;
        }
    }
}

void update_tank_player(Entity* e) {
    // physics -----------------------------------------------------------------------

    U64 control_states = 0;

    Tank* player = arena_lookup_Tank(&st.tanks, e->data_ref);
    PlayerControls* controls = player->controls;
    if (controls != NULL) {
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
}

void update_tank_training_dummy(Entity* e) {
    Tank* t = arena_lookup_Tank(&st.tanks, e->data_ref);
    advance_tank_state(e, t, 0);
}


// BULLET ---------------------------------------------------

void update_bullet(Entity* e) {
    Bullet* b = arena_lookup_Bullet(&st.bullets, e->data_ref);
    e->position = vector2_add(e->position, vector2_scale(b->direction, b->speed));
}

void handle_collision_bullet(Entity* this, Entity* other) {
    if (other->entity_type & ENTITY_TYPE_WALL) {
        destroy_entity(entity_ref(this));
        //Vector2 eject = eject_collision(this, other);
        //Vector2 normal = normalize(eject);
        //Vector2 old_pos = this->position;
        //this->position = vector2_add(this->position, eject);
    }
}

void draw_bullet(Entity* e) {
    //Bullet* b = arena_lookup_Bullet(&bullets, e->data_ref);
    draw_circle_v_aa(e->position, e->collision_roundness, RED);
}

// will fill in entity field in tank
BulletInsertReturn insert_bullet(Bullet t, Entity e) {
    BulletRef b_ref = arena_insert_Bullet(&st.bullets, t);
    e.data_ref = b_ref;
    EntityRef e_ref = insert_entity(e);
    arena_lookup_Bullet(&st.bullets, b_ref)->e = e_ref;

    return (BulletInsertReturn) {
        .e = e_ref,
        .b = b_ref,
    };
}

void destroy_bullet(Entity* t) {
    arena_remove_Bullet(&st.bullets, t->data_ref);
}
