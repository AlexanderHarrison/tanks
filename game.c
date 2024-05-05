#include "game.h"

TODO line 471

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game");
    SetTargetFPS(60);

    SceneTransition transition = { .next_scene_type = SceneType_MainMenu };

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
            case SceneType_SetControls: {
                printf("scene transition to Set controls\n");
                transition = run_set_controls(transition.set_controls_target);
                break;
            }
            case SceneType_SinglePlayer: {
                printf("scene transition to Singleplayer\n");
                transition = run_singleplayer();
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
}

SceneTransition run_set_controls(Controls* controls) {
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
        .err = "invalid transition from run_set_controls"
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
    entities = arena_create_Entity();
    tanks = arena_create_Tank();
    bullets = arena_create_Bullet();

    arena_dealloc_Entity(&entities);
    arena_dealloc_Tank(&tanks);
    arena_dealloc_Bullet(&bullets);

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
    DrawRectangleRounded(rect, e->collision_roundness / fmin(size.x, size.y), 5, GRAY);
    //draw_circle_v_aa(e->position, size, GRAY);
    //DrawRectangleV(e->position, size, GRAY);
}

SceneTransition run_singleplayer() {
    entities = arena_create_Entity();
    tanks = arena_create_Tank();
    bullets = arena_create_Bullet();

    arena_insert_Entity(&entities, (Entity) {
        .entity_type = ENTITY_TYPE_WALL,
        .entity_flags = 0,
        .collision_roundness = 45.0f,
        .collision_size = { 50.0f, 50.0f },
        .position = { .x = SCREEN_WIDTH / 2, .y = SCREEN_HEIGHT / 2 },
        .on_collide = NULL,
        .draw = draw_wall,
        .update = NULL,
    });

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
            .entity_type = ENTITY_TYPE_TANK_PLAYER,
            .collision_mask = ENTITY_TYPE_WALL,
            .collision_roundness = default_tank.size,
            .collision_size = { default_tank.size, default_tank.size },
            .entity_flags = ENTITY_FLAGS_RENDER_LAYER_1,
            .position = (Vector2) { .x = SCREEN_WIDTH / 2, .y = 3 * SCREEN_HEIGHT / 4 },
            .on_collide = handle_collision_tank_player,
            .draw = draw_tank_player,
            .update = update_tank_player,
        }
    );

    U64 frame_num = 0;

    SceneTransition transistion = {
        .next_scene_type = SceneType_MainMenu
    };

    while (true) {
        //if ((frame_num & 63) == 0) {
        //    float angle = GetRandomValue(0, 360);
        //    Vector2 position;
        //    if (angle < 90.0f) {
        //        position = (Vector2) { .x = (angle / 90.0f) * SCREEN_WIDTH, .y = 0.0f };
        //    } else if (angle < 180.0f) {
        //        position = (Vector2) { .x = ((angle - 90.0f) / 90.0f) * SCREEN_WIDTH, .y = SCREEN_HEIGHT };
        //    } else if (angle < 270.0f) {
        //        position = (Vector2) { .x = 0.0f, .y = ((angle - 180.0f) / 90.0f) * SCREEN_HEIGHT };
        //    } else {
        //        position = (Vector2) { .x = SCREEN_WIDTH, .y = ((angle - 270.0f) / 90.0f) * SCREEN_HEIGHT };
        //    }

        //    spawn_enemy(enemy_test(position));
        //}

        //// update ----------------------------------------------------

        run_entity_updates();
        run_collision_checks();

        // draw ----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        draw_entities();

        EndDrawing();

        ++frame_num;

        if (WindowShouldClose()) {
            transistion = (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
            break;
        }

        //if (player.dead) break;
    }

    arena_dealloc_Entity(&entities);
    arena_dealloc_Tank(&tanks);
    arena_dealloc_Bullet(&bullets);

    return transistion;
}

void run_collision_checks() {
    ArenaIter iter1 = arena_iter(&entities.tracking);

    while (true) {
        ArenaKey k1 = arena_iter_next(&iter1);
        if (k1.idx == ARENA_INVALID_IDX) break;
        Entity* e1 = arena_lookup_Entity(&entities, k1);
        if (e1->entity_flags & ENTITY_FLAGS_COLLISION_NONE)
            continue;

        EntityTypeMask etype1 = e1->entity_type;
        EntityTypeMask coll1 = e1->collision_mask;

        ArenaIter iter2 = iter1;

        while (true) {
            ArenaKey k2 = arena_iter_next(&iter2);
            if (k2.idx == ARENA_INVALID_IDX) break;
            Entity* e2 = arena_lookup_Entity(&entities, k2);
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
    printf("%f %f %f ; %f %f %f\n", pos_a.x, pos_a.y, size_a, pos_b.x, pos_b.y, size_b);
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

    for (U64 i = 0; i < 4; ++i) {
        Vector2 a_c = vector2_add(a_pos, vector2_mul(a_circle_centre_offset, circles[i]));

        for (U64 j = 0; j < 4; ++j) {
            Vector2 b_c = vector2_add(b_pos, vector2_mul(b_circle_centre_offset, circles[j]));
            // TODO find closest circle
            if (circle_colliding(a_c, a_roundness, b_c, b_roundness)) {
                Vector2 diff = vector2_sub(a_c, b_c);
                F32 diff_len = length(diff);
                F32 overlap = (a_roundness + b_roundness) - diff_len + COLLISION_EPSILON;
                return vector2_scale(diff, overlap / diff_len);
            }
        }
    }

    return (Vector2) { .x = 0.0f, .y = 0.0f };
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
            //printf("%i %f %f, %f %f\n", i*4+j, a_c.x, a_c.y, b_c.x, b_c.y);
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
        ArenaIter iter = arena_iter(&entities.tracking);

        while (true) {
            ArenaKey k = arena_iter_next(&iter);
            if (k.idx == ARENA_INVALID_IDX) break;
            Entity* e = arena_lookup_Entity(&entities, k);
            if ((e->entity_flags & ENTITY_FLAGS_RENDER_LAYER) == cur_layer)
                if (e->draw != NULL) 
                    e->draw(e);
        }
    }
}

void run_entity_updates() {
    ArenaIter iter = arena_iter(&entities.tracking);

    while (true) {
        ArenaKey k = arena_iter_next(&iter);
        if (k.idx == ARENA_INVALID_IDX) break;
        Entity* e = arena_lookup_Entity(&entities, k);

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
    arena_insert_Entity(&entities, (Entity) {
        .entity_type = ENTITY_TYPE_DEBUG,
        .collision_size = vector,
        .position = base,
        .on_collide = NULL,
        .draw = draw_debug_vector,
        .update = NULL,
    });
}


void handle_collision_tank_player(Entity* this, Entity* other) {
    if (other->entity_type & ENTITY_TYPE_WALL) {
        this->position = vector2_add(this->position, eject_collision(this, other));
    }
}

void draw_tank_player(Entity* e) {
    Tank* player = arena_lookup_Tank(&tanks, e->data_ref);

    Color player_colour;
    if (player->bullet_timer != 0) {
        player_colour = BLACK;
    } else {
        player_colour = player->body_colour;
    }
    draw_circle_v_aa(e->position, player->stats->size, player_colour);
    Vector2 head = vector2_add(e->position, vector2_dir(player->angle, 16)); 
    draw_circle_v_aa(head, 4, WHITE);
}

// will fill in entity field in tank
TankInsertReturn insert_tank(Tank t, Entity e) {
    TankRef t_ref = arena_insert_Tank(&tanks, t);
    e.data_ref = t_ref;
    EntityRef e_ref = arena_insert_Entity(&entities, e);
    arena_lookup_Tank(&tanks, t_ref)->e = e_ref;

    return (TankInsertReturn) {
        .e = e_ref,
        .t = t_ref,
    };
}

void remove_tank(TankRef t) {
    EntityRef e = arena_lookup_Tank(&tanks, t)->e;
    arena_remove_Tank(&tanks, t);
    arena_remove_Entity(&entities, e);
}

void update_tank_player(Entity* e) {
    Tank* player = arena_lookup_Tank(&tanks, e->data_ref);
    assert(player != NULL);
    const TankStats* stats = player->stats;

    // values -----------------------------------------------------------------------

    Controls* controls = player->controls;
    if (controls != NULL) {
        bool forward_down = player_input_down(controls, PlayerInput_Forward);
        bool backward_down = player_input_down(controls, PlayerInput_Backward);
        if (forward_down && !backward_down) {
            if (player->velocity < 0.0f)
                player->velocity = 0.0f;
            player->velocity += stats->acceleration;
        }
        if (backward_down && !forward_down) {
            if (player->velocity > 0.0f)
                player->velocity = 0.0f;
            player->velocity -= stats->acceleration;
        }
        if (!forward_down && !backward_down) player->velocity /= 1.8f;

        bool left_down = player_input_down(controls, PlayerInput_TurnLeft);
        bool right_down = player_input_down(controls, PlayerInput_TurnRight);

        if (left_down) player->angle_velocity -= stats->angle_acceleration;
        if (right_down) player->angle_velocity += stats->angle_acceleration;
        if (!left_down && !right_down) player->angle_velocity /= 1.8f;
    }

    float max_angle_speed = stats->angle_max_speed_fast;
    if (controls != NULL && player_input_down(controls, PlayerInput_TurnModifier)) {
        max_angle_speed = stats->angle_max_speed;
    }

    player->velocity = clamp(player->velocity, -stats->max_speed, stats->max_speed);
    player->angle_velocity = clamp(player->angle_velocity, -max_angle_speed, max_angle_speed);
    player->angle += player->angle_velocity;
    if (player->angle >= 360.0f) player->angle -= 360.0f;
    if (player->angle < 0.0f) player->angle += 360.0f;

    // position ----------------------------------------------------------------------

    Vector2 update = vector2_dir(player->angle, player->velocity);
    e->position = vector2_add(e->position, update);

    e->position.x = clamp(e->position.x, 0, SCREEN_WIDTH);
    e->position.y = clamp(e->position.y, 0, SCREEN_HEIGHT);

    // bullets -----------------------------------------------------------------------

    if (player->bullet_timer != 0) {
        --player->bullet_timer;
    }

    if (controls != NULL && player_input_pressed(controls, PlayerInput_Attack1)) {
        // 1 frame buffer
        if (player->bullet_timer <= 1) {
            insert_bullet(
                (Bullet) {
                    .direction = vector2_dir(player->angle, 1.0),
                    .speed = 10.0,
                    .bounces = 5,
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
                }
            );
            player->bullet_timer = stats->bullet_cooldown;
        }
    }
}


// BULLET ---------------------------------------------------

void update_bullet(Entity* e) {
    Bullet* b = arena_lookup_Bullet(&bullets, e->data_ref);
    e->position = vector2_add(e->position, vector2_scale(b->direction, b->speed));

    if (e->position.y < 0.0f) {
        b->direction = reflect(b->direction, (Vector2) { .x = 0.0f, .y = 1.0f });
        b->bounces -= 1;
    } else if (e->position.y > SCREEN_HEIGHT) {
        b->direction = reflect(b->direction, (Vector2) { .x = 0.0f, .y = -1.0f });
        b->bounces -= 1;
    }

    if (e->position.x < 0.0f) {
        b->direction = reflect(b->direction, (Vector2) { .x = 1.0f, .y = 0.0f });
        b->bounces -= 1;
    } else if (e->position.x > SCREEN_WIDTH) {
        b->direction = reflect(b->direction, (Vector2) { .x = -1.0f, .y = 0.0f });
        b->bounces -= 1;
    }

    if (b->bounces <= 0) {
        remove_bullet(e->data_ref);
    }
}

void handle_collision_bullet(Entity* this, Entity* other) {
    if (other->entity_type & ENTITY_TYPE_WALL) {
        //remove_bullet(this->data_ref);

        Vector2 eject = eject_collision(this, other);
        Vector2 normal = normalize(eject);
        insert_debug_vector(this->position, vector2_scale(normal, 20.0f));
        this->position = vector2_add(this->position, eject);
        assert(!colliding(this, other));
        printf("normal %f %f\n", normal.x, normal.y);
        
        Bullet* b = arena_lookup_Bullet(&bullets, this->data_ref);

        b->direction = reflect(b->direction, normal);
        remove_bullet(this->data_ref);
        //insert_bullet(
        //    (Bullet) {
        //        .direction = reflect(b->direction, normal),
        //        .speed = 10.0,
        //        .bounces = 5,
        //    },
        //    (Entity) {
        //        .entity_type = ENTITY_TYPE_BULLET,
        //        .collision_mask = ENTITY_TYPE_WALL,
        //        .collision_roundness = 8.0f,
        //        .collision_size = { 8.0f, 8.0f },
        //        .entity_flags = 0,
        //        .position = this->position,
        //        .on_collide = handle_collision_bullet,
        //        .draw = draw_bullet,
        //        .update = update_bullet,
        //    }
        //);

        b->direction = vector2_scale(b->direction, -1.0f);
    }
}

void draw_bullet(Entity* e) {
    //Bullet* b = arena_lookup_Bullet(&bullets, e->data_ref);
    draw_circle_v_aa(e->position, e->collision_roundness, RED);
}

// will fill in entity field in tank
BulletInsertReturn insert_bullet(Bullet t, Entity e) {
    BulletRef b_ref = arena_insert_Bullet(&bullets, t);
    e.data_ref = b_ref;
    EntityRef e_ref = arena_insert_Entity(&entities, e);
    arena_lookup_Bullet(&bullets, b_ref)->e = e_ref;

    return (BulletInsertReturn) {
        .e = e_ref,
        .b = b_ref,
    };
}

void remove_bullet(BulletRef t) {
    EntityRef e = arena_lookup_Bullet(&bullets, t)->e;
    arena_remove_Bullet(&bullets, t);
    arena_remove_Entity(&entities, e);
}
