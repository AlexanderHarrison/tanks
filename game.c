#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <raylib.h>

#include "game.h"
#include "net_uv.h"

int main(void) {
    Net net;

    if (init_net(&net)) {
        printf("ERROR INITIALISING NET\n");
        return 1;
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Holy Cow! Game!");
    SetTargetFPS(60);

    uint64_t frame_num = 0;
    Player player = default_player;
    Opponent opponent = default_opponent;

    while (!WindowShouldClose()) {
        // update ----------------------------------------------------

        update_player(&player);
        update_bullet_positions();

        // networking woah!!! ---------------------------------------

        Message my_message = {
            .next_message = NULL,
            .frame = frame_num,
            .position = player.position,
            .angle = player.angle
        };
        send_message(&net, &my_message);

        Message* opp_message = check_messages(&net);
        if (opp_message != NULL) {
            while (opp_message->next_message != NULL) {
                Message* prev = opp_message;
                opp_message = opp_message->next_message;
                free(prev);
            }

            Vector2 opp_pos = opp_message->position;
            opponent.position.x = SCREEN_WIDTH - opp_pos.x;
            opponent.position.y = SCREEN_HEIGHT - opp_pos.y;
            opponent.angle = opp_message->angle + 180.0f;
            if (opponent.angle >= 360.0f) opponent.angle -= 360.0f;
            free(opp_message);
        }

        // draw ----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        for (int i = 0; i < bullet_count; ++i) {
            DrawCircleV(bullets[i].position, bullets[i].size, BLUE);
        }
    
        Color player_colour;
        if (player.bullet_timer != 0) {
            player_colour = BLACK;
        } else {
            player_colour = MAROON;
        }
        DrawCircleV(player.position, 20, player_colour);
        Vector2 head = vector2_add(player.position, vector2_dir(player.angle, 16)); 
        DrawCircleV(head, 4, WHITE);

        DrawCircleV(opponent.position, 20, GREEN);
        Vector2 opp_head = vector2_add(opponent.position, vector2_dir(opponent.angle, 16)); 
        DrawCircleV(opp_head, 4, WHITE);

        EndDrawing();

        ++frame_num;
    }

    CloseWindow();

    deinit_net(&net);

    return 0;
}

void update_player(Player* player) {
    // values -----------------------------------------------------------------------

    if (IsKeyDown(KEY_W)) player->velocity += player->acceleration;
    if (IsKeyDown(KEY_S)) player->velocity -= player->acceleration;
    if (!IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) player->velocity = 0.0f;
    player->velocity = clamp(player->velocity, -player->max_speed, player->max_speed);

    if (IsKeyDown(KEY_A)) player->angle_velocity -= player->angle_acceleration;
    if (IsKeyDown(KEY_D)) player->angle_velocity += player->angle_acceleration;
    if (!IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) player->angle_velocity = 0.0f;

    float max_angle_speed;
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
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

    if (IsKeyPressed(KEY_H)) {
        // 1 frame buffer
        if (player->bullet_timer <= 1) {
            Bullet new_bullet = {
                .position = player->position,
                .update_vector = vector2_dir(player->angle, 15.0),
                .size = 3,
                .damage = 20,
            };

            spawn_bullet(new_bullet);
            player->bullet_timer = player->bullet_cooldown;
        }
    }
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
