#include "game.h"

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
    F32 size = e->collision.circle.size;
    draw_circle_v_aa(e->position, size, GRAY);
    //DrawRectangleV(e->position, size, GRAY);
}

SceneTransition run_singleplayer() {
    entities = arena_create_Entity();
    tanks = arena_create_Tank();
    bullets = arena_create_Bullet();

    arena_insert_Entity(&entities, (Entity) {
        .entity_type = ENTITY_TYPE_WALL,
        .entity_flags = ENTITY_FLAGS_COLLISION_CIRCLE | ENTITY_FLAGS_STATIC_COLLISION,
        .collision = { .circle = { .size = 50.0 } },
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
            .collision_mask = ENTITY_TYPE_ALL,
            .entity_flags = ENTITY_FLAGS_COLLISION_CIRCLE,
            .collision = { .circle = { .size = default_tank.size } },
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

        run_updates();
        //update_player(&player, &player1_controls);

        //update_bullets();
        //update_enemies(&player);
        //resolve_enemy_enemy_collisions();
        //resolve_enemy_bullet_collisions();

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

//CollisionInfo compute_collision(Entity* a, Entity* b) {
//    return (CollisionInfo) {};
//            //// m = (r_b a + r_a b) / (r_a + r_b)
//            //// https://www.desmos.com/calculator/52rxnfpfzn
//            //out->point = vector2_scale(
//            //    vector2_add(
//            //        vector2_scale(pos_a, col_b.circle.size), 
//            //        vector2_scale(pos_b, col_a.circle.size)
//            //    ),
//            //    1.0f / (col_a.circle.size + col_b.circle.size)
//            //);
//            //out->normal = vector2_scale(diff, fsqrt(dist_sq));
//
//            //return true;
//}

U32 collision_event_type(Entity* a, Entity* b) {
    U32 type_a = a->entity_flags & ENTITY_FLAGS_COLLISION_TYPE;
    U32 type_b = b->entity_flags & ENTITY_FLAGS_COLLISION_TYPE;

    if (type_a == ENTITY_FLAGS_COLLISION_NONE || type_b == ENTITY_FLAGS_COLLISION_NONE)
        return COLLISION_EVENT_TYPE_NONE;

    return (type_a << ENTITY_FLAGS_COLLISION_BITS) ^ type_b;
}

// vector to move 'a' such that it is just outside of 'b'
Vector2 eject_collision(Entity* a, Entity* b) {
    Collision col_a = a->collision;
    Vector2 pos_a = a->position;
    Collision col_b = b->collision;
    Vector2 pos_b = b->position;

    switch (collision_event_type(a, b)) {
        case COLLISION_EVENT_TYPE_NONE: {
            return (Vector2) { .x = 0.0f, .y = 0.0f };
        }
        case COLLISION_EVENT_TYPE_CIRCLE_CIRCLE: {
            Vector2 diff = vector2_sub(pos_a, pos_b);
            F32 diff_len = length(diff);
            Vector2 norm = vector2_scale(diff, 1.0f / diff_len);
            F32 overlap = col_a.circle.size + col_b.circle.size - diff_len;
            return vector2_scale(norm, overlap + COLLISION_EPSILON);
        }
        default: {
            fprintf(stderr, "Invalid collision type\n");
            return (Vector2) { .x = 0.0f, .y = 0.0f };
        }
    }
}

// returns true if colliding, writing info into out
bool colliding(Entity* a, Entity* b) {
    U32 type_a = a->entity_flags & ENTITY_FLAGS_COLLISION_TYPE;
    Collision col_a = a->collision;
    Vector2 pos_a = a->position;
    U32 type_b = b->entity_flags & ENTITY_FLAGS_COLLISION_TYPE;
    Collision col_b = b->collision;
    Vector2 pos_b = b->position;

    switch ((type_a << ENTITY_FLAGS_COLLISION_BITS) ^ type_b) {
        case COLLISION_EVENT_TYPE_NONE: return false;
        case COLLISION_EVENT_TYPE_CIRCLE_CIRCLE: {
            Vector2 diff = vector2_sub(pos_a, pos_b);
            F32 dist_sq = diff.x*diff.x + diff.y*diff.y;
            F32 max_collision_dist = col_a.circle.size + col_b.circle.size;
            return dist_sq+COLLISION_EPSILON_SQ < max_collision_dist*max_collision_dist;
        }
        default: {
            fprintf(stderr, "Invalid collision type\n");
            return false;
        }
    }
}

void draw_entities() {
    ArenaIter iter = arena_iter(&entities.tracking);

    while (true) {
        ArenaKey k = arena_iter_next(&iter);
        if (k.idx == ARENA_INVALID_IDX) break;
        Entity* e = arena_lookup_Entity(&entities, k);

        if (e->draw != NULL) e->draw(e);
    }
}

void run_updates() {
    ArenaIter iter = arena_iter(&entities.tracking);

    while (true) {
        ArenaKey k = arena_iter_next(&iter);
        if (k.idx == ARENA_INVALID_IDX) break;
        Entity* e = arena_lookup_Entity(&entities, k);

        if (e->update != NULL) e->update(e);
    }
}

void handle_collision_tank_player(Entity* this, Entity* other) {
    if (other->entity_flags & ENTITY_FLAGS_STATIC_COLLISION) {
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
TankRef insert_tank(Tank t, Entity e) {
    TankRef t_ref = arena_insert_Tank(&tanks, t);
    e.data_ref = t_ref;
    EntityRef e_ref = arena_insert_Entity(&entities, e);
    arena_lookup_Tank(&tanks, t_ref)->e = e_ref;

    return t_ref;
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
            //Bullet new_bullet = {
            //    .data = &default_bullet_data,
            //    .position = e->position,
            //    .direction = vector2_dir(player->angle, 1.0),
            //    .owner = { .type = OwnerType_Player, .player = player }
            //};

            insert_bullet(
                (Bullet) {
                    .direction = vector2_dir(player->angle, 1.0),
                    .speed = 10.0,
                },
                (Entity) {
                    .entity_type = ENTITY_TYPE_BULLET,
                    .collision_mask = ENTITY_TYPE_ALL,
                    .entity_flags = ENTITY_FLAGS_COLLISION_CIRCLE,
                    .collision = { .circle = { .size = 8.0 } },
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
    } else if (e->position.y > SCREEN_HEIGHT) {
        b->direction = reflect(b->direction, (Vector2) { .x = 0.0f, .y = -1.0f });
    }

    if (e->position.x < 0.0f) {
        b->direction = reflect(b->direction, (Vector2) { .x = 1.0f, .y = 0.0f });
    } else if (e->position.x > SCREEN_WIDTH) {
        b->direction = reflect(b->direction, (Vector2) { .x = -1.0f, .y = 0.0f });
    }
}

void handle_collision_bullet(Entity* this, Entity* other) {
    if (other->entity_flags & ENTITY_FLAGS_STATIC_COLLISION) {
        //remove_bullet(this->data_ref);

        Vector2 eject = eject_collision(this, other);
        this->position = vector2_add(this->position, eject);
        Vector2 normal = normalize(eject);
        
        Bullet* b = arena_lookup_Bullet(&bullets, this->data_ref);
        b->direction = reflect(b->direction, normal);
    }
}

void draw_bullet(Entity* e) {
    //Bullet* b = arena_lookup_Bullet(&bullets, e->data_ref);
    draw_circle_v_aa(e->position, e->collision.circle.size, RED);
}

// will fill in entity field in tank
BulletRef insert_bullet(Bullet t, Entity e) {
    BulletRef b_ref = arena_insert_Bullet(&bullets, t);
    e.data_ref = b_ref;
    EntityRef e_ref = arena_insert_Entity(&entities, e);
    arena_lookup_Bullet(&bullets, b_ref)->e = e_ref;

    return b_ref;
}

void remove_bullet(BulletRef t) {
    EntityRef e = arena_lookup_Bullet(&bullets, t)->e;
    arena_remove_Bullet(&bullets, t);
    arena_remove_Entity(&entities, e);
}


//int spawn_bullet(Bullet new_bullet) {
//    if (bullet_count == MAX_BULLETS) {
//        return 1;
//    }
//
//    bullets[bullet_count] = new_bullet;
//    bullet_count += 1;
//
//    return 0;
//}
//
//void update_bullets() {
//    for (int i = 0; i < bullet_count; ++i) {
//        Bullet* b = &bullets[i];
//        b->data->update(b);
//
//        Vector2 pos = b->position;
//        if (pos.x < 0 || pos.x > SCREEN_WIDTH || pos.y < 0 || pos.y > SCREEN_HEIGHT) {
//            remove_bullet(i);
//        } 
//    }
//}
//
//void remove_bullet(int i) {
//    memmove(&bullets[i], &bullets[i+1], (bullet_count-i-1) * sizeof(Bullet));
//    --bullet_count;
//}
//
//void draw_bullets() {
//    for (int i = 0; i < bullet_count; ++i) {
//        draw_circle_v_aa(bullets[i].position, 6.0, BLUE);
//    }
//}
//
//int spawn_enemy(Enemy new_enemy) {
//    if (enemy_count == MAX_ENEMIES) {
//        return 1;
//    }
//
//    enemies[enemy_count] = new_enemy.tag;
//    enemy_data[enemy_count] = new_enemy.data;
//    enemy_count += 1;
//
//    return 0;
//}
//
//void remove_enemy(int i) {
//    memmove(&enemies[i], &enemies[i+1], (enemy_count-i-1) * sizeof(EnemyTag));
//    memmove(&enemy_data[i], &enemy_data[i+1], (enemy_count-i-1) * sizeof(EnemyData));
//    --enemy_count;
//}
//
//void update_enemies(Player* player) {
//    for (int i = 0; i < enemy_count; ++i) {
//        EnemyTag e = enemies[i];
//        EnemyData data = enemy_data[i];
//
//        switch (e.type) {
//            case EnemyType_Test: {
//                Vector2 dir = normalize(vector2_sub(player->position, e.position));
//                e.position = vector2_add(e.position, vector2_scale(dir, ENEMY_SPEED));
//                break;
//            }
//            default: {
//                printf("Unknown enemy type\n");
//                break;
//            }
//        }
//
//        if (data.invincibility_timer > 0) {
//            data.invincibility_timer -= 1;
//
//            if (data.invincibility_timer == 0) {
//                e.flags &= ~EnemyFlag_Invincible;
//            } 
//        }
//
//        if (data.knockback_timer > 0) {
//            data.knockback_timer -= 1;
//
//            e.position = vector2_add(e.position, data.knockback);
//        }
//
//        e.position.x = clamp(e.position.x, 0, SCREEN_WIDTH);
//        e.position.y = clamp(e.position.y, 0, SCREEN_HEIGHT);
//
//        if (data.death_timer > 0) {
//            data.death_timer -= 1;
//
//            if (data.death_timer == 0) {
//                remove_enemy(i);
//                i -= 1;
//                continue;
//            }
//        }
//
//        enemies[i] = e;
//        enemy_data[i] = data;
//    }
//}
//
//void resolve_enemy_enemy_collisions() {
//    while (1) {
//        bool collided = false;
//
//        for (int i = 0; i < enemy_count; ++i) {
//            for (int j = i+1; j < enemy_count; ++j) {
//                EnemyTag e1 = enemies[i];
//                EnemyTag e2 = enemies[j];
//
//                if (colliding(
//                    e1.position,
//                    e1.enemy_collision_size,
//                    e2.position,
//                    e2.enemy_collision_size
//                )) {
//                    collided = true;
//
//                    float diff = distance(e1.position, e2.position);
//                    float overlap = (e1.enemy_collision_size + e2.enemy_collision_size) - diff + 0.01f;
//                    Vector2 dir = normalize(vector2_sub(e1.position, e2.position));
//                    Vector2 shift1 = vector2_scale(dir, 0.5f * overlap);
//                    Vector2 shift2 = vector2_scale(dir, -0.5f * overlap);
//
//                    enemies[i].position = vector2_add(e1.position, shift1);
//                    enemies[j].position = vector2_add(e2.position, shift2);
//                }
//            }
//        }
//
//        if (!collided) { 
//            break; 
//        }
//    }
//}
//
//void resolve_entity_collisions(Tank** tanks, int tank_count) {
//    for (int i = 0; i < tank_count; ++i) {
//        Tank* t = tanks[i];
//        TankStats* stats = t->stats;
//
//        Vector2 position = t->position;
//        float hurtbox_size = stats->hurtbox_size;
//
//        for (int j = 0; j < bullet_count; ++j) {
//            Bullet b = bullets[j];
//            
//            if ((b.interaction_mask & ) {
//                continue;
//            }
//
//            if (colliding(
//                position,
//                hurtbox_size,
//                b.position,
//                b.collision_size
//            )) {
//                remove_bullet(j);
//                j -= 1;
//                player_take_damage(b.damage);
//            }
//        }
//    }
//}
//
//void resolve_player_player_collisions(Player** players, int player_count) {
//    while (1) {
//        bool collided = false;
//
//        for (int i = 0; i < player_count; ++i) {
//            for (int j = i+1; j < player_count; ++j) {
//                Player* p1 = players[i];
//                Vector2 p1_position = p1->position;
//                float p1_hurtbox_size = p1->hurtbox_size;
//
//                Player* p2 = players[j];
//                Vector2 p2_position = p2->position;
//                float p2_hurtbox_size = p2->hurtbox_size;
//
//                if (colliding(
//                    p1_position,
//                    p1_hurtbox_size,
//                    p2_position,
//                    p2_hurtbox_size
//                )) {
//                    collided = true;
//
//                    float diff = distance(p1_position, p2_position);
//                    float overlap = (p1_hurtbox_size + p2_hurtbox_size) - diff + 0.01f;
//                    Vector2 dir = normalize(vector2_sub(p1_position, p2_position));
//                    Vector2 shift1 = vector2_scale(dir, 0.5f * overlap);
//                    Vector2 shift2 = vector2_scale(dir, -0.5f * overlap);
//
//                    players[i]->position = vector2_add(p1_position, shift1);
//                    players[j]->position = vector2_add(p2_position, shift2);
//                }
//            }
//        }
//
//        if (!collided) { 
//            break; 
//        }
//    }
//}
//
//void draw_enemies() {
//    for (int i = 0; i < enemy_count; ++i) {
//        EnemyTag e = enemies[i];
//        EnemyData data = enemy_data[i];
//
//        Color colour = RED;
//
//        if (data.knockback_timer > 0) {
//            colour = BLACK;
//        }
//
//        float scale = 1.0;
//
//        if (data.death_timer > 0) {
//            scale = ((float) data.death_timer) / ((float) data.death_time);
//        }
//
//        draw_circle_v_aa(e.position, ENEMY_VISUAL_SIZE * scale, colour);
//    }
//}
//
//void draw_player_hud(Player* player, Side side) {
//    float health_ratio = (float)player->health / (float)player->max_health;
//    int x_start;
//    switch (side) {
//        case Side_Left: {
//            x_start = 0;
//            break;
//        }
//        case Side_Right: {
//            x_start = SCREEN_WIDTH / 2;
//            break;
//        }
//        default: {
//            fprintf(stderr, "ERROR: invalid side\n");
//            x_start = 0;
//        }
//    }
//
//    int width = (float)(SCREEN_WIDTH) * 0.5 * health_ratio;
//    DrawRectangle(x_start, 0, width, 20, player->body_colour);
//}
//
//void draw_general_hud() {
//}
