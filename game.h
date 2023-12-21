#pragma once

#include <math.h>

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

Player player = {
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

void update_player();

void draw_player();

Vector2 swap_orientation(Vector2 point) {
    Vector2 transformed = {
        .x = SCREEN_WIDTH - point.x,
        .y = SCREEN_HEIGHT - point.y
    };
    return transformed;
}

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
void draw_bullets();

// ENEMIES -------------------------------------------------------------

#define MAX_ENEMIES 512

#define ENEMY_VISUAL_SIZE 25.0
#define ENEMY_COLLISION_SIZE 25.0
#define ENEMY_SPEED 1.0

typedef int EnemyData_Test;

typedef enum {
    EnemyType_Test,
} EnemyType;

typedef struct {
    Vector2 position;
    float size;
    EnemyType type;
} EnemyTag;

typedef union {
    EnemyData_Test test;   
} EnemyData;

typedef struct {
    EnemyTag tag;
    EnemyData data;
} Enemy;

EnemyTag enemies[MAX_ENEMIES];
EnemyData enemy_data[MAX_ENEMIES];
int enemy_count = 0;

int spawn_enemy(Enemy);
void update_enemy_positions();
void update_enemy_collisions();
void draw_enemies();

Enemy enemy_test(Vector2 position) {
    EnemyTag tag = {
        .position = position,
        .size = 25.0,
        .type = EnemyType_Test
    };

    EnemyData data = { .test = 0 };

    Enemy test = {
        .tag = tag,
        .data = data
    };
    return test;
}

// MATH -------------------------------------------------------------

Vector2 vector2_add(Vector2 a, Vector2 b) {
    Vector2 new_v = { .x = a.x + b.x, .y = a.y + b.y };
    return new_v;
}

Vector2 vector2_sub(Vector2 a, Vector2 b) {
    Vector2 new_v = { .x = a.x - b.x, .y = a.y - b.y };
    return new_v;
}

Vector2 vector2_scale(Vector2 v, float scale) {
    Vector2 new_v = { .x = v.x * scale, .y = v.y * scale };
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

float length_squared(Vector2 a) {
    return a.x*a.x + a.y*a.y;
}

float length(Vector2 a) {
    return sqrt(length_squared(a));
}

float distance_squared(Vector2 a, Vector2 b) {
    return length_squared(vector2_sub(a, b));
}

float distance(Vector2 a, Vector2 b) {
    return length(vector2_sub(a, b));
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
    return tsize*tsize >= distance_squared(p1, p2);
}

Vector2 normalize(Vector2 v) {
    float len = length(v);
    Vector2 dir = { 
        .x = v.x / len,
        .y = v.y / len 
    };
    return dir;
}
