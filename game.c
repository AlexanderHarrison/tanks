#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <raylib.h>

#include "game.h"
// #include "net_uv.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Holy Cow! Game!");
    SetTargetFPS(60);

    uint64_t frame_num = 0;

    spawn_enemy(enemy_test((Vector2) { .x = 0.0, .y = 0.0 }));
    spawn_enemy(enemy_test((Vector2) { .x = 100.0, .y = 0.0 }));
    spawn_enemy(enemy_test((Vector2) { .x = 200.0, .y = 0.0 }));

    while (!WindowShouldClose()) {
        // update ----------------------------------------------------

        update_player(&player);
        update_bullet_positions();
        update_enemy_positions();
        update_enemy_collisions();

        // draw ----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        draw_bullets();
        draw_player();
        draw_enemies();

        EndDrawing();

        ++frame_num;
    }

    CloseWindow();

    return 0;
}

void update_player() {
    // values -----------------------------------------------------------------------

    if (IsKeyDown(KEY_W)) player.velocity += player.acceleration;
    if (IsKeyDown(KEY_S)) player.velocity -= player.acceleration;
    if (!IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) player.velocity = 0.0f;
    player.velocity = clamp(player.velocity, -player.max_speed, player.max_speed);

    if (IsKeyDown(KEY_A)) player.angle_velocity -= player.angle_acceleration;
    if (IsKeyDown(KEY_D)) player.angle_velocity += player.angle_acceleration;
    if (!IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) player.angle_velocity = 0.0f;

    float max_angle_speed;
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        max_angle_speed = player.angle_max_speed;
    } else {
        max_angle_speed = player.angle_max_speed_fast;
    }
    player.angle_velocity = clamp(player.angle_velocity, -max_angle_speed, max_angle_speed);
    player.angle += player.angle_velocity;

    if (player.angle >= 360.0f) player.angle -= 360.0f;
    if (player.angle < 0.0f) player.angle += 360.0f;

    // position ----------------------------------------------------------------------

    Vector2 update = vector2_dir(player.angle, player.velocity);
    player.position = vector2_add(player.position, update);

    player.position.x = clamp(player.position.x, 0, SCREEN_WIDTH);
    player.position.y = clamp(player.position.y, 0, SCREEN_HEIGHT);

    // bullets -----------------------------------------------------------------------

    if (player.bullet_timer != 0) {
        --player.bullet_timer;
    }

    if (IsKeyPressed(KEY_H)) {
        // 1 frame buffer
        if (player.bullet_timer <= 1) {
            Bullet new_bullet = {
                .position = player.position,
                .update_vector = vector2_dir(player.angle, 15.0),
                .size = 3,
                .damage = 20,
            };

            spawn_bullet(new_bullet);
            player.bullet_timer = player.bullet_cooldown;
        }
    }
}

void draw_player() {
    Color player_colour;
    if (player.bullet_timer != 0) {
        player_colour = BLACK;
    } else {
        player_colour = MAROON;
    }
    DrawCircleV(player.position, 20, player_colour);
    Vector2 head = vector2_add(player.position, vector2_dir(player.angle, 16)); 
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
        DrawCircleV(bullets[i].position, bullets[i].size, BLUE);
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

void update_enemy_positions() {
    for (int i = 0; i < enemy_count; ++i) {
        EnemyTag e = enemies[i];

        Vector2 dir = normalize(vector2_sub(player.position, e.position));
        e.position = vector2_add(e.position, vector2_scale(dir, ENEMY_SPEED));
        enemies[i] = e;
    }
}

void update_enemy_collisions() {
    while (1) {
        bool collided = false;

        for (int i = 0; i < enemy_count; ++i) {
            for (int j = i+1; j < enemy_count; ++j) {
                EnemyTag e1 = enemies[i];
                EnemyTag e2 = enemies[j];

                if (colliding(
                    e1.position,
                    e1.size,
                    e2.position,
                    e2.size
                )) {
                    collided = true;

                    float diff = distance(e1.position, e2.position);
                    float overlap = (e1.size + e2.size) - diff + 0.01;
                    Vector2 dir = normalize(vector2_sub(e1.position, e2.position));
                    Vector2 shift1 = vector2_scale(dir, 0.5 * overlap);
                    Vector2 shift2 = vector2_scale(dir, -0.5 * overlap);

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

void draw_enemies() {
    for (int i = 0; i < enemy_count; ++i) {
        DrawCircleV(enemies[i].position, ENEMY_VISUAL_SIZE, RED);
    }
}
