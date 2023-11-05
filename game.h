#pragma once

#include "net_uv.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

// GENERAL -------------------------------------------------------------

void resolve_collisions();

// PLAYER ---------------------------------------------------------------

typedef struct {
    float max_speed;
    float acceleration;
    float angle_acceleration;
    float angle_max_speed;
    float angle_max_speed_fast;
    int bullet_cooldown;

    Vector2 position;
    float velocity;
    float angle; // degrees
    float angle_velocity;
    int bullet_timer;
} Player;

Player default_player = {
    .max_speed = 8.0f,
    .acceleration = 0.5f,
    .angle_max_speed = 4.5f,
    .angle_max_speed_fast = 9.0f,
    .angle_acceleration = 1.5f,
    .bullet_cooldown = 10,

    .position = { .x = SCREEN_WIDTH / 2, .y = 3 * SCREEN_HEIGHT / 4 },
    .velocity = 0,
    .angle = -90.0f,
    .angle_velocity = 0.0f,
    .bullet_timer = 3
};

void update_player(Player* player, Message* message);

Vector2 swap_orientation(Vector2 point) {
    Vector2 transformed = {
        .x = SCREEN_WIDTH - point.x,
        .y = SCREEN_HEIGHT - point.y
    };
    return transformed;
}

// Opponent ----------------------------------------------------------

typedef struct {
    Vector2 position;
    float angle; // degrees
} Opponent;

Opponent default_opponent = {
    .position = { 
        .x = SCREEN_WIDTH - SCREEN_WIDTH / 2, 
        .y = SCREEN_HEIGHT - 3 * SCREEN_HEIGHT / 4 
    },
    .angle = 90.0f,
};

// BULLET -------------------------------------------------------------

#define MAX_BULLETS 512

typedef struct {
    Vector2 position;
    Vector2 update_vector;
    float size;
    float damage;
} Bullet;

Bullet bullets[MAX_BULLETS];
int bullet_count = 0;

int spawn_bullet(Bullet);
void update_bullet_positions();
void remove_bullet(int i);

// MATH -------------------------------------------------------------

Vector2 vector2_add(Vector2 a, Vector2 b) {
    Vector2 new_v = { .x = a.x + b.x, .y = a.y + b.y };
    return new_v;
}

// angle in degrees
Vector2 vector2_dir(float angle, float length) {
    float angle_rad = (angle / 180.0f) * PI;
    Vector2 dir = {
        .x = cos(angle_rad) * length,
        .y = sin(angle_rad) * length,
    };

    return dir;
}

float dist_squared(Vector2 a, Vector2 b) {
    float dx = a.x-b.x;
    float dy = a.y-b.y;
    return dx*dx + dy*dy;
}

float clamp(float val, float a, float b) {
    if (val < a) {
        return a;
    } else if (val > b) {
        return b;
    } else {
        return val;
    }
}

bool colliding(Vector2 p1, float s1, Vector2 p2, float s2) {
    float tsize = s1 + s2;
    return tsize*tsize >= dist_squared(p1, p2);
}
