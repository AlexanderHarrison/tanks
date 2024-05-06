#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <raylib.h>

#include "controls.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define AA_SIZE_INCREMENT 0.3

void draw_circle_v_aa(Vector2 centre, F32 radius, Color colour) {
    DrawCircleV(centre, radius+AA_SIZE_INCREMENT*2.0, Fade(colour, 0.2));
    DrawCircleV(centre, radius+AA_SIZE_INCREMENT, Fade(colour, 0.5));
    DrawCircleV(centre, radius, colour);
}

void draw_rectangle_rounded_aa(Rectangle r, F32 roundness, Color colour) {
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

// GENERAL -------------------------------------------------------------

typedef enum {
    Side_Left,
    Side_Right,
} Side;

typedef enum {
    SceneType_Err,
    SceneType_Exit,
    SceneType_MainMenu,
    SceneType_ConfigMenu,
    SceneType_SetControlsMenu,
    SceneType_SetPlayerControls,
    SceneType_SetTrainingControls,
    SceneType_SinglePlayer,
    SceneType_Training,
    SceneType_MultiPlayer,
} SceneType;

typedef union {
    char* err;
    PlayerControls* set_player_controls_target;
    TrainingControls* set_training_controls_target;
} SceneData;

typedef struct {
    SceneType next_scene_type;
    union {
        const char* err;
        PlayerControls* set_player_controls_target;
        TrainingControls* set_training_controls_target;
    };
} SceneTransition;

#define ENTITY_TYPE_DEBUG (1u << 0)
#define ENTITY_TYPE_WALL (1u << 1)
#define ENTITY_TYPE_BULLET (1u << 2)
#define ENTITY_TYPE_TANK (1u << 3)
#define ENTITY_TYPE_ALL (~0)
typedef U16 EntityTypeMask;

// MENUS ----------------------------------------------------------------

#define MENU_ITEM_WIDTH 400
#define MENU_ITEM_HEIGHT 130
#define MENU_ITEM_Y_PADDING 20

#define MENU_TEXT_COLOUR { 0, 0, 0, 255 }

typedef struct {
    const char* text;  
    Color base_colour;
    Color text_colour;
    Color selected_colour;
    SceneTransition action;
} MenuItem;

#define MAIN_MENU_SIZE 5
static const MenuItem main_menu[MAIN_MENU_SIZE] = {
    {
        .text = "Multiplayer",
        .base_colour = { 225, 161, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 200, 50, 255 },
        .action = { SceneType_MultiPlayer }
    },
    {
        .text = "Training",
        .base_colour = { 200, 40, 200, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 220, 80, 220, 255 },
        .action = { SceneType_Training }
    },
    {
        .text = "Singleplayer",
        .base_colour = { 225, 161, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 200, 50, 255 },
        .action = { SceneType_SinglePlayer }
    },
    {
        .text = "Config",
        .base_colour = { 161, 205, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 200, 235, 50, 255 },
        .action = { SceneType_ConfigMenu }
    },
    {
        .text = "Exit",
        .base_colour = { 170, 170, 170, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 210, 210, 210, 255 },
        .action = { SceneType_Exit }
    },
};

#define CONFIG_MENU_SIZE 2
static const MenuItem config_menu[CONFIG_MENU_SIZE] = {
    {
        .text = "Set Controls",
        .base_colour = { 225, 0, 161, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 50, 200, 255 },
        .action = { SceneType_SetControlsMenu }
    },
    {
        .text = "Back",
        .base_colour = { 170, 170, 170, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 210, 210, 210, 255 },
        .action = { SceneType_MainMenu }
    },
};

#define CONTROLS_MENU_SIZE 4
static const MenuItem controls_menu[CONTROLS_MENU_SIZE] = {
    {
        .text = "Set P1 Controls",
        .base_colour = { 225, 161, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 200, 50, 255 },
        .action = { 
            .next_scene_type = SceneType_SetPlayerControls,
            .set_player_controls_target = &player1_controls,
        }
    },
    {
        .text = "Set P2 Controls",
        .base_colour = { 225, 161, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 200, 50, 255 },
        .action = { 
            .next_scene_type = SceneType_SetPlayerControls,
            .set_player_controls_target = &player2_controls,
        }
    },
    {
        .text = "Set Training Controls",
        .base_colour = { 225, 161, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 200, 50, 255 },
        .action = { 
            .next_scene_type = SceneType_SetTrainingControls,
            .set_training_controls_target = &training_controls,
        }
    },
    {
        .text = "Back",
        .base_colour = { 170, 170, 170, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 210, 210, 210, 255 },
        .action = { SceneType_ConfigMenu }
    },
};

SceneTransition run_menu(const MenuItem* menu, U32 menu_len);
SceneTransition run_singleplayer(void);
SceneTransition run_training(void);
SceneTransition run_multiplayer(void);
SceneTransition run_set_player_controls(PlayerControls* target_controls);
SceneTransition run_set_training_controls(TrainingControls* target_controls);

// Collision ------------------------------------------------------------

#define COLLISION_EPSILON 0.01f
#define COLLISION_EPSILON_SQ 0.0001f

// Entity ------------------------------------------------------------

#define ENTITY_FLAGS_COLLISION_NONE     0x00000001

#define ENTITY_FLAGS_RENDER_LAYER_BITS 2
#define ENTITY_FLAGS_RENDER_LAYER       0x00000300
#define ENTITY_FLAGS_RENDER_LAYER_0     0x00000000
#define ENTITY_FLAGS_RENDER_LAYER_1     0x00000100
#define ENTITY_FLAGS_RENDER_LAYER_2     0x00000200
#define ENTITY_FLAGS_RENDER_LAYER_3     0x00000300

typedef struct Entity {
    ArenaKey data_ref;
    EntityTypeMask entity_type;
    EntityTypeMask collision_mask;
    U32 entity_flags;

    F32 collision_roundness;
    Vector2 collision_size;

    Vector2 position;
    void (*on_collide)(struct Entity* this, struct Entity* other);
    void (*draw)(struct Entity* this);
    void (*update)(struct Entity* this);
    void (*destroy)(struct Entity* this);
} Entity;

#define ARENA_TYPE Entity
#include <arena.h>
typedef ArenaKey EntityRef;

EntityRef insert_entity(Entity e);
void destroy_entity(EntityRef e);
EntityRef entity_ref(Entity* e);

void run_entity_updates();
void run_collision_checks();
void draw_entities();

bool colliding(Entity* a, Entity* b);
Vector2 eject_collision(Entity* a, Entity* b);

void draw_debug_vector(Entity* e);
void insert_debug_vector(Vector2 base, Vector2 vector);

// Tank ------------------------------------------------------------

typedef struct {
    F32 max_speed;
    F32 acceleration;
    F32 angle_acceleration;
    F32 angle_max_speed;
    F32 angle_max_speed_fast;
    F32 size;
    F32 velocity_decay;
    F32 angle_velocity_decay;
    I32 bullet_cooldown;
    I32 max_health;
} TankStats;

typedef struct Tank {
    EntityRef e;
    const TankStats* stats;
    // might be null
    PlayerControls* controls;
    Color body_colour;
    F32 velocity;
    F32 angle; // degrees
    F32 angle_velocity;
    I32 bullet_timer;
    bool dead;
    I32 health;
} Tank;

static const TankStats default_tank = {
    .max_speed = 8.0f,
    .acceleration = 0.5f,
    .angle_max_speed = 2.0f,
    .angle_max_speed_fast = 9.0f,
    .angle_acceleration = 1.0f,
    .size = 20.0f,
    .velocity_decay = 1.8f,
    .angle_velocity_decay = 1.8f,
    .bullet_cooldown = 10,
    .max_health = 100,
};

#define ARENA_TYPE Tank
#include <arena.h>
typedef ArenaKey TankRef;

typedef struct {
    EntityRef e;
    TankRef t;
} TankInsertReturn;

TankInsertReturn insert_tank(Tank tank, Entity e);
void destroy_tank(Entity* t);

void update_tank_player(Entity* e);
void update_tank_training_dummy(Entity* e);
void handle_collision_tank_player(Entity* this, Entity* other);
void draw_tank(Entity* e);

// BULLET -------------------------------------------------------------

//typedef struct {
//    int type;
//    union data {
//        struct async_timer {
//            F32 time;
//        };
//    };
//} Subaction;

typedef struct {
    EntityRef e;

    Vector2 direction;
    F32 speed;
    //int subaction_index;
} Bullet;

#define ARENA_TYPE Bullet
#include <arena.h>
typedef ArenaKey BulletRef;

typedef struct {
    EntityRef e;
    BulletRef b;
} BulletInsertReturn;

BulletInsertReturn insert_bullet(Bullet tank, Entity e);
void destroy_bullet(Entity* b);

void update_bullet(Entity* e);
void handle_collision_bullet(Entity* this, Entity* other);
void draw_bullet(Entity* e);

typedef struct {
    Arena_Entity entities;
    Arena_Tank tanks;
    Arena_Bullet bullets;
} GameState;

GameState init_game_state(void) {
    return (GameState) {
        .entities = arena_create_Entity(),
        .tanks = arena_create_Tank(),
        .bullets = arena_create_Bullet(),
    };
}

void reset_game_state(GameState* st) {
    arena_tracking_reset(&st->entities.tracking);
    arena_tracking_reset(&st->tanks.tracking);
    arena_tracking_reset(&st->bullets.tracking);
}

void dealloc_game_state(GameState* st) {
    arena_dealloc_Entity(&st->entities);
    arena_dealloc_Tank(&st->tanks);
    arena_dealloc_Bullet(&st->bullets);
}

Entity* lookup_entity(GameState* st, EntityRef e) {
    return arena_lookup_Entity(&st->entities, e);
}

Bullet* lookup_bullet(GameState* st, BulletRef b) {
    return arena_lookup_Bullet(&st->bullets, b);
}

Tank* lookup_tank(GameState* st, TankRef t) {
    return arena_lookup_Tank(&st->tanks, t);
}

GameState st;

// HUD ------------------------------------------------------------

void draw_player_hud(Tank* player, Side side);
void draw_general_hud();

// MATH -------------------------------------------------------------

Vector2 vector2_add(Vector2 a, Vector2 b) {
    return (Vector2) { .x = a.x + b.x, .y = a.y + b.y };
}

Vector2 vector2_mul(Vector2 a, Vector2 b) {
    return (Vector2) { .x = a.x * b.x, .y = a.y * b.y };
}

Vector2 vector2_sub(Vector2 a, Vector2 b) {
    return (Vector2) { .x = a.x - b.x, .y = a.y - b.y };
}

Vector2 vector2_sub_f(Vector2 a, F32 b) {
    return (Vector2) { .x = a.x - b, .y = a.y - b };
}

Vector2 vector2_scale(Vector2 v, F32 scale) {
    return (Vector2) { .x = v.x * scale, .y = v.y * scale };
}

// angle in degrees
Vector2 vector2_dir(F32 angle, F32 length) {
    F32 angle_rad = (angle / 180.0f) * PI;
    return (Vector2) {
        .x = cosf(angle_rad) * length,
        .y = sinf(angle_rad) * length,
    };
}

F32 vector2_dot(Vector2 a, Vector2 b) {
    return a.x*b.x + a.y*b.y;
}

Vector2 vector2_abs(Vector2 v) {
    F32 x = v.x;
    F32 y = v.y;
    return (Vector2) {
        .x = (x > 0.0f) ? x : -x,
        .y = (y > 0.0f) ? y : -y,
    };
}

// assumes normal is normalized. 
// reflected vector has same magnitude as incident.
//
// https://www.desmos.com/calculator/b01p9ptgxi
Vector2 reflect(Vector2 incident, Vector2 normal) {
    F32 a = -2.0f * vector2_dot(normal, incident);
    return vector2_add(incident, vector2_scale(normal, a));
}

F32 length_squared(Vector2 a) {
    return a.x*a.x + a.y*a.y;
}

F32 length(Vector2 a) {
    return sqrtf(length_squared(a));
}

F32 distance_squared(Vector2 a, Vector2 b) {
    return length_squared(vector2_sub(a, b));
}

F32 distance(Vector2 a, Vector2 b) {
    return length(vector2_sub(a, b));
}

F32 clamp(F32 val, F32 a, F32 b) {
    if (val < a) {
        return a;
    } else if (val > b) {
        return b;
    } else {
        return val;
    }
}

Vector2 normalize(Vector2 v) {
    F32 len = length(v);
    return (Vector2) { 
        .x = v.x / len,
        .y = v.y / len 
    };
}
