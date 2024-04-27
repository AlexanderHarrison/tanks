#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <raylib.h>

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
        }
    }
}

SceneTransition run_set_controls(Controls* controls) {
    int button_idx = 0;

    // clear button press queue so you don't instantly map key
    while (GetKeyPressed()) {}

    while (true) {
        // update -------------------------------------------------
        if (WindowShouldClose()) {
            return (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
        }

        for (int gamepad = 0; IsGamepadAvailable(gamepad); ++gamepad) {
            for (int button = 0; button < 256; ++button) {
                if (IsGamepadButtonPressed(gamepad, button)) {
                    controls->map[button_idx] = (PlayerInputMapping) {
                        .source_type = PlayerInputSourceType_Gamepad,
                        .gamepad_input = { gamepad, button }
                    };
                    button_idx += 1;
                }
            }
        }

        int key = GetKeyPressed();
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

        int text_size = 40;
        int text_height = 40; // approximate

        int text_width = MeasureText(text, text_size);
        int text_x = (SCREEN_WIDTH - text_width) / 2;
        int text_y = (SCREEN_HEIGHT - text_height) / 2;

        DrawText(text, text_x, text_y, text_size, (Color) MENU_TEXT_COLOUR);

        EndDrawing();
    }

    return (SceneTransition) {
        .next_scene_type = SceneType_Err,
        .err = "invalid transition from run_set_controls"
    };
}

SceneTransition run_menu(const MenuItem* menu, int menu_len) {
    int menu_idx = 0;

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

        int y = MENU_ITEM_Y_PADDING;
        int x = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;

        Rectangle rect = { x, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT };

        for (int i = 0; i < menu_len; ++i) {
            // background ------------------------------

            float roundness = 0.2f;
            int segments = 5;

            if (i != menu_idx) {
                DrawRectangleRounded(rect, roundness, segments, menu[i].base_colour);
            } else {
                DrawRectangleRounded(rect, roundness, segments, menu[i].selected_colour);
                DrawRectangleRoundedLines(rect, roundness, segments, 8.0, BLACK);
            }

            // text ------------------------------------

            int text_size = 40;
            int text_height = 40; // approximate

            int text_width = MeasureText(menu[i].text, text_size);
            int text_x = rect.x + (MENU_ITEM_WIDTH - text_width) / 2;
            int text_y = rect.y + (MENU_ITEM_HEIGHT - text_height) / 2;

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
            for (int i = 0; i < menu_len; ++i) {
                if (strcmp(menu[i].text, "Back") == 0) {
                    return menu[i].action;
                }
            }
        }
    }
}

SceneTransition run_multiplayer() {
    bullet_count = 0;
    enemy_count = 0;

    Player player1 = default_player;
    Player player2 = default_player;
    Player* players[2] = { &player1, &player2 };
    player2.position.x = SCREEN_WIDTH - player2.position.x;
    player2.position.y = SCREEN_WIDTH - player2.position.y;
    player2.angle = player2.angle + 180.0f;
    Color prev_colour = player2.body_colour;
    player2.body_colour = (Color) { prev_colour.g, prev_colour.r, prev_colour.b, prev_colour.a };

    uint64_t frame_num = 0;

    while (true) {
        // update ----------------------------------------------------

        update_player(&player1, &player1_controls);
        update_player(&player2, &player2_controls);

        update_bullet_positions();
        resolve_player_bullet_collisions(players, 2);
        resolve_player_player_collisions(players, 2);

        // draw ----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        draw_bullets();
        draw_player(&player1);
        draw_player(&player2);

        EndDrawing();

        ++frame_num;

        if (WindowShouldClose()) {
            return (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
        }

        if (player1.dead) break;
        if (player2.dead) break;
    }

    return (SceneTransition) {
        .next_scene_type = SceneType_MainMenu
    };
}

SceneTransition run_singleplayer() {
    bullet_count = 0;
    enemy_count = 0;

    Player player = default_player;

    uint64_t frame_num = 0;

    while (true) {
        if ((frame_num & 63) == 0) {
            float angle = GetRandomValue(0, 360);
            Vector2 position;
            if (angle < 90.0f) {
                position = (Vector2) { .x = (angle / 90.0f) * SCREEN_WIDTH, .y = 0.0f };
            } else if (angle < 180.0f) {
                position = (Vector2) { .x = ((angle - 90.0f) / 90.0f) * SCREEN_WIDTH, .y = SCREEN_HEIGHT };
            } else if (angle < 270.0f) {
                position = (Vector2) { .x = 0.0f, .y = ((angle - 180.0f) / 90.0f) * SCREEN_HEIGHT };
            } else {
                position = (Vector2) { .x = SCREEN_WIDTH, .y = ((angle - 270.0f) / 90.0f) * SCREEN_HEIGHT };
            }

            spawn_enemy(enemy_test(position));
        }


        // update ----------------------------------------------------

        update_player(&player, &player1_controls);

        update_bullet_positions();
        update_enemies(&player);
        resolve_enemy_enemy_collisions();
        resolve_enemy_bullet_collisions();

        // draw ----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        draw_bullets();
        draw_player(&player);
        draw_enemies();

        EndDrawing();

        ++frame_num;

        if (WindowShouldClose()) {
            return (SceneTransition) {
                .next_scene_type = SceneType_Exit
            };
        }

        if (player.dead) break;
    }

    return (SceneTransition) {
        .next_scene_type = SceneType_MainMenu
    };
}

void update_player(Player* player, Controls* controls) {
    Vector2 player_position = player->position;
    float player_hurtbox_size = player->hurtbox_size;

    for (int i = 0; i < enemy_count; ++i) {
        EnemyTag e = enemies[i];

        if (colliding(
            e.position,
            e.bullet_collision_size,
            player_position,
            player_hurtbox_size
        )) {
            player->dead = true;
            return;
        }
    }

    // values -----------------------------------------------------------------------

    bool forward_down = player_input_down(controls, PlayerInput_Forward);
    bool backward_down = player_input_down(controls, PlayerInput_Backward);

    if (forward_down) player->velocity += player->acceleration;
    if (backward_down) player->velocity -= player->acceleration;
    if (!forward_down && !backward_down) player->velocity = 0.0f;
    player->velocity = clamp(player->velocity, -player->max_speed, player->max_speed);

    bool left_down = player_input_down(controls, PlayerInput_TurnLeft);
    bool right_down = player_input_down(controls, PlayerInput_TurnRight);

    if (left_down) player->angle_velocity -= player->angle_acceleration;
    if (right_down) player->angle_velocity += player->angle_acceleration;
    if (!left_down && !right_down) player->angle_velocity = 0.0f;

    float max_angle_speed;
    if (player_input_down(controls, PlayerInput_TurnModifier)) {
        max_angle_speed = player->angle_max_speed;
    } else {
        max_angle_speed = player->angle_max_speed_fast;
    }
    player->angle_velocity = clamp(player->angle_velocity, -max_angle_speed, max_angle_speed);
    player->angle += player->angle_velocity;

    if (player->angle >= 360.0f) player->angle -= 360.0f;
    if (player->angle < 0.0f) player->angle += 360.0f;

    // position ----------------------------------------------------------------------

    Vector2 update = vector2_dir(player->angle, player->velocity);
    player->position = vector2_add(player->position, update);

    player->position.x = clamp(player->position.x, 0, SCREEN_WIDTH);
    player->position.y = clamp(player->position.y, 0, SCREEN_HEIGHT);

    // bullets -----------------------------------------------------------------------

    if (player->bullet_timer != 0) {
        --player->bullet_timer;
    }

    if (player_input_pressed(controls, PlayerInput_Attack1)) {
        // 1 frame buffer
        if (player->bullet_timer <= 1) {
            Bullet new_bullet = {
                .position = player->position,
                .update_vector = vector2_dir(player->angle, 15.0),
                .collision_size = 6.0,
                .damage = 20,
                .owner = { .type = OwnerType_Player, .player = player }
            };

            spawn_bullet(new_bullet);
            player->bullet_timer = player->bullet_cooldown;
        }
    }
}

void draw_player(Player* player) {
    Color player_colour;
    if (player->bullet_timer != 0) {
        player_colour = BLACK;
    } else {
        player_colour = player->body_colour;
    }
    DrawCircleV(player->position, player->visual_size, player_colour);
    Vector2 head = vector2_add(player->position, vector2_dir(player->angle, 16)); 
    DrawCircleV(head, 4, WHITE);
}

int spawn_bullet(Bullet new_bullet) {
    if (bullet_count == MAX_BULLETS) {
        return 1;
    }

    bullets[bullet_count] = new_bullet;
    bullet_count += 1;

    return 0;
}

void update_bullet_positions() {
    for (int i = 0; i < bullet_count; ++i) {
        Bullet b = bullets[i];
        b.position = vector2_add(b.position, b.update_vector);

        if (b.position.x < 0 || b.position.x > SCREEN_WIDTH || b.position.y < 0 || b.position.y > SCREEN_HEIGHT) {
            remove_bullet(i);
        } else {
            bullets[i] = b;
        }
    }
}

void remove_bullet(int i) {
    memmove(&bullets[i], &bullets[i+1], (bullet_count-i-1) * sizeof(Bullet));
    --bullet_count;
}

void draw_bullets() {
    for (int i = 0; i < bullet_count; ++i) {
        DrawCircleV(bullets[i].position, 6.0, BLUE);
    }
}

int spawn_enemy(Enemy new_enemy) {
    if (enemy_count == MAX_ENEMIES) {
        return 1;
    }

    enemies[enemy_count] = new_enemy.tag;
    enemy_data[enemy_count] = new_enemy.data;
    enemy_count += 1;

    return 0;
}

void remove_enemy(int i) {
    memmove(&enemies[i], &enemies[i+1], (enemy_count-i-1) * sizeof(EnemyTag));
    memmove(&enemy_data[i], &enemy_data[i+1], (enemy_count-i-1) * sizeof(EnemyData));
    --enemy_count;
}

void update_enemies(Player* player) {
    for (int i = 0; i < enemy_count; ++i) {
        EnemyTag e = enemies[i];
        EnemyData data = enemy_data[i];

        switch (e.type) {
            case EnemyType_Test: {
                Vector2 dir = normalize(vector2_sub(player->position, e.position));
                e.position = vector2_add(e.position, vector2_scale(dir, ENEMY_SPEED));
                break;
            }
            default: {
                printf("Unknown enemy type\n");
                break;
            }
        }

        if (data.invincibility_timer > 0) {
            data.invincibility_timer -= 1;

            if (data.invincibility_timer == 0) {
                e.flags &= ~EnemyFlag_Invincible;
            } 
        }

        if (data.knockback_timer > 0) {
            data.knockback_timer -= 1;

            e.position = vector2_add(e.position, data.knockback);
        }

        e.position.x = clamp(e.position.x, 0, SCREEN_WIDTH);
        e.position.y = clamp(e.position.y, 0, SCREEN_HEIGHT);

        if (data.death_timer > 0) {
            data.death_timer -= 1;

            if (data.death_timer == 0) {
                remove_enemy(i);
                i -= 1;
                continue;
            }
        }

        enemies[i] = e;
        enemy_data[i] = data;
    }
}

void resolve_enemy_enemy_collisions() {
    while (1) {
        bool collided = false;

        for (int i = 0; i < enemy_count; ++i) {
            for (int j = i+1; j < enemy_count; ++j) {
                EnemyTag e1 = enemies[i];
                EnemyTag e2 = enemies[j];

                if (colliding(
                    e1.position,
                    e1.enemy_collision_size,
                    e2.position,
                    e2.enemy_collision_size
                )) {
                    collided = true;

                    float diff = distance(e1.position, e2.position);
                    float overlap = (e1.enemy_collision_size + e2.enemy_collision_size) - diff + 0.01f;
                    Vector2 dir = normalize(vector2_sub(e1.position, e2.position));
                    Vector2 shift1 = vector2_scale(dir, 0.5f * overlap);
                    Vector2 shift2 = vector2_scale(dir, -0.5f * overlap);

                    enemies[i].position = vector2_add(e1.position, shift1);
                    enemies[j].position = vector2_add(e2.position, shift2);
                }
            }
        }

        if (!collided) { 
            break; 
        }
    }
}

void resolve_enemy_bullet_collisions() {
    for (int i = 0; i < enemy_count; ++i) {
        EnemyTag e = enemies[i];

        if (e.flags & EnemyFlag_Invincible) {
            continue;
        }

        for (int j = 0; j < bullet_count; ++j) {
            Bullet b = bullets[j];

            if (b.owner.type == OwnerType_Enemy) {
                continue;
            }

            if (colliding(
                e.position,
                e.bullet_collision_size,
                b.position,
                b.collision_size
            )) {
                e.health -= b.damage;
                remove_bullet(j);
                j -= 1;

                EnemyData data = enemy_data[i];

                if (e.health <= 0.0f) {
                    data.death_time = 10;
                    data.death_timer = 10;
                }

                e.flags |= EnemyFlag_Invincible;
                data.invincibility_timer = 10;
                data.knockback_timer = 10;

                Vector2 dir = normalize(b.update_vector);
                data.knockback = vector2_scale(dir, 10.0f);

                enemy_data[i] = data;
                enemies[i] = e;
            }
        }
    }
}

void resolve_player_bullet_collisions(Player** players, int player_count) {
    for (int i = 0; i < player_count; ++i) {
        Player* player = players[i];
        Vector2 position = player->position;
        float hurtbox_size = player->hurtbox_size;

        for (int j = 0; j < bullet_count; ++j) {
            Bullet b = bullets[j];
            
            if (b.owner.type == OwnerType_Player && b.owner.player == player) {
                continue;
            }

            if (colliding(
                position,
                hurtbox_size,
                b.position,
                b.collision_size
            )) {
                player->dead = true;
                remove_bullet(j);
                j -= 1;
            }
        }
    }
}

void resolve_player_player_collisions(Player** players, int player_count) {
    while (1) {
        bool collided = false;

        for (int i = 0; i < player_count; ++i) {
            for (int j = i+1; j < player_count; ++j) {
                Player* p1 = players[i];
                Vector2 p1_position = p1->position;
                float p1_hurtbox_size = p1->hurtbox_size;

                Player* p2 = players[j];
                Vector2 p2_position = p2->position;
                float p2_hurtbox_size = p2->hurtbox_size;

                if (colliding(
                    p1_position,
                    p1_hurtbox_size,
                    p2_position,
                    p2_hurtbox_size
                )) {
                    collided = true;

                    float diff = distance(p1_position, p2_position);
                    float overlap = (p1_hurtbox_size + p2_hurtbox_size) - diff + 0.01f;
                    Vector2 dir = normalize(vector2_sub(p1_position, p2_position));
                    Vector2 shift1 = vector2_scale(dir, 0.5f * overlap);
                    Vector2 shift2 = vector2_scale(dir, -0.5f * overlap);

                    players[i]->position = vector2_add(p1_position, shift1);
                    players[j]->position = vector2_add(p2_position, shift2);
                }
            }
        }

        if (!collided) { 
            break; 
        }
    }
}

void draw_enemies() {
    for (int i = 0; i < enemy_count; ++i) {
        EnemyTag e = enemies[i];
        EnemyData data = enemy_data[i];

        Color color = RED;

        if (data.knockback_timer > 0) {
            color = BLACK;
        }

        float scale = 1.0;

        if (data.death_timer > 0) {
            scale = ((float) data.death_timer) / ((float) data.death_time);
        }

        DrawCircleV(e.position, ENEMY_VISUAL_SIZE * scale, color);
    }
}
