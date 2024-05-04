#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <raylib.h>

#include "controls.h"
#include <tools.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define AA_SIZE_INCREMENT 0.3

void draw_circle_v_aa(Vector2 centre, F32 radius, Color colour) {
    DrawCircleV(centre, radius+AA_SIZE_INCREMENT*2.0, Fade(colour, 0.2));
    DrawCircleV(centre, radius+AA_SIZE_INCREMENT, Fade(colour, 0.5));
    DrawCircleV(centre, radius, colour);
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
    SceneType_SetControls,
    SceneType_SinglePlayer,
    SceneType_MultiPlayer,
} SceneType;

typedef union {
    char* err;
    Controls* set_controls_target;
} SceneData;

typedef struct {
    SceneType next_scene_type;
    union {
        const char* err;
        Controls* set_controls_target;
    };
} SceneTransition;

#define ENTITY_TYPE_WALL (1u << 0)
#define ENTITY_TYPE_BULLET (1u << 1)
#define ENTITY_TYPE_TANK_PLAYER (1u << 2)
#define ENTITY_TYPE_TANK_ENEMY (1u << 3)
#define ENTITY_TYPE_ALL (~0)
typedef U16 EntityTypeMask;

// MENUS ----------------------------------------------------------------

#define MENU_ITEM_WIDTH 400
#define MENU_ITEM_HEIGHT 150
#define MENU_ITEM_Y_PADDING 20

#define MENU_TEXT_COLOUR { 0, 0, 0, 255 }

typedef struct {
    const char* text;  
    Color base_colour;
    Color text_colour;
    Color selected_colour;
    SceneTransition action;
} MenuItem;

#define MAIN_MENU_SIZE 4
static const MenuItem main_menu[MAIN_MENU_SIZE] = {
    {
        .text = "Multiplayer",
        .base_colour = { 225, 161, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 200, 50, 255 },
        .action = { SceneType_MultiPlayer }
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

#define CONTROLS_MENU_SIZE 3
static const MenuItem controls_menu[CONTROLS_MENU_SIZE] = {
    {
        .text = "Set P1 Controls",
        .base_colour = { 225, 161, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 200, 50, 255 },
        .action = { 
            .next_scene_type = SceneType_SetControls,
            .set_controls_target = &player1_controls,
        }
    },
    {
        .text = "Set P2 Controls",
        .base_colour = { 225, 161, 0, 255 },
        .text_colour = MENU_TEXT_COLOUR,
        .selected_colour = { 255, 200, 50, 255 },
        .action = { 
            .next_scene_type = SceneType_SetControls,
            .set_controls_target = &player2_controls,
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
SceneTransition run_multiplayer(void);
SceneTransition run_set_controls(Controls* target_controls);

// INPUT ---------------------------------------------------------------

bool player_input_down(Controls* controls, PlayerInput input) {
    PlayerInputMapping mapping = controls->map[input];
    switch (mapping.source_type) {
        case PlayerInputSourceType_Unmapped: return false;
        case PlayerInputSourceType_Keyboard: {
            return IsKeyDown(mapping.keyboard_input);
        }
        case PlayerInputSourceType_Gamepad: {
            GamepadInput g_input = mapping.gamepad_input;
            return IsGamepadButtonDown(g_input.gamepad, g_input.button);
        }
        default: {
            fprintf(stderr, "ERROR: player input case %i not handled\n", mapping.source_type);
            return false;
        }
    }

    return false;
}

bool player_input_pressed(Controls* controls, PlayerInput input) {
    PlayerInputMapping mapping = controls->map[input];
    switch (mapping.source_type) {
        case PlayerInputSourceType_Unmapped: return false;
        case PlayerInputSourceType_Keyboard: {
            return IsKeyPressed(mapping.keyboard_input);
        }
        case PlayerInputSourceType_Gamepad: {
            GamepadInput g_input = mapping.gamepad_input;
            return IsGamepadButtonPressed(g_input.gamepad, g_input.button);
        }
        default: {
            fprintf(stderr, "ERROR: player input case %i not handled\n", mapping.source_type);
            return false;
        }
    }

    return false;
}

// Collision ------------------------------------------------------------

#define COLLISION_EPSILON 0.01f
#define COLLISION_EPSILON_SQ 0.0001f

typedef union {
    struct {
        F32 size;
    } circle;
    //struct {
    //    Vector2 size;
    //} rect;
} Collision;

#define COLLISION_EVENT_TYPE_NONE 0x0
#define COLLISION_EVENT_TYPE_CIRCLE_CIRCLE 0x5 // 0b0101

// Entity ------------------------------------------------------------

#define ENTITY_FLAGS_COLLISION_BITS 2
#define ENTITY_FLAGS_COLLISION_TYPE     0x00000003
#define ENTITY_FLAGS_COLLISION_NONE     0x00000000
#define ENTITY_FLAGS_COLLISION_CIRCLE   0x00000001
//#define ENTITY_FLAGS_COLLISION_RECT     0x00000002
//#define ENTITY_FLAGS_COLLISION_RESERVED 0x00000003
#define ENTITY_FLAGS_STATIC_COLLISION   0x00000004

typedef struct Entity {
    ArenaKey data_ref;
    EntityTypeMask entity_type;
    EntityTypeMask collision_mask;
    U32 entity_flags;
    Collision collision;
    Vector2 position;
    void (*on_collide)(struct Entity* this, struct Entity* other);
    void (*draw)(struct Entity* this);
    void (*update)(struct Entity* this);
} Entity;

#define ARENA_TYPE Entity
#include <arena.h>
Arena_Entity entities;
typedef ArenaKey EntityRef;

void run_updates();
void run_collision_checks();
void draw_entities();

U32 collision_event_type(Entity* a, Entity* b);
bool colliding(Entity* a, Entity* b);
Vector2 eject_collision(Entity* a, Entity* b);

// Tank ------------------------------------------------------------

typedef struct {
    F32 max_speed;
    F32 acceleration;
    F32 angle_acceleration;
    F32 angle_max_speed;
    F32 angle_max_speed_fast;
    F32 size;
    I32 bullet_cooldown;
    I32 max_health;
} TankStats;

typedef struct Tank {
    EntityRef e;
    const TankStats* stats;
    // might be null
    Controls* controls;
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
    .angle_acceleration = 1.5f,
    .size = 20.0f,
    .bullet_cooldown = 10,
    .max_health = 100,
};

#define ARENA_TYPE Tank
#include <arena.h>
Arena_Tank tanks;
typedef ArenaKey TankRef;

TankRef insert_tank(Tank tank, Entity e);
void remove_tank(TankRef t);

void update_tank_player(Entity* e);
void handle_collision_tank_player(Entity* this, Entity* other);
void draw_tank_player(Entity* e);

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
Arena_Bullet bullets;
typedef ArenaKey BulletRef;

BulletRef insert_bullet(Bullet tank, Entity e);
void remove_bullet(BulletRef t);

void update_bullet(Entity* e);
void handle_collision_bullet(Entity* this, Entity* other);
void draw_bullet(Entity* e);

// HUD ------------------------------------------------------------

void draw_player_hud(Tank* player, Side side);
void draw_general_hud();

// MATH -------------------------------------------------------------

Vector2 vector2_add(Vector2 a, Vector2 b) {
    Vector2 new_v = { .x = a.x + b.x, .y = a.y + b.y };
    return new_v;
}

Vector2 vector2_sub(Vector2 a, Vector2 b) {
    Vector2 new_v = { .x = a.x - b.x, .y = a.y - b.y };
    return new_v;
}

Vector2 vector2_scale(Vector2 v, F32 scale) {
    Vector2 new_v = { .x = v.x * scale, .y = v.y * scale };
    return new_v;
}

// angle in degrees
Vector2 vector2_dir(F32 angle, F32 length) {
    F32 angle_rad = (angle / 180.0f) * PI;
    Vector2 dir = {
        .x = cosf(angle_rad) * length,
        .y = sinf(angle_rad) * length,
    };

    return dir;
}

F32 vector2_dot(Vector2 a, Vector2 b) {
    return a.x*b.x + a.y*b.y;
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
    Vector2 dir = { 
        .x = v.x / len,
        .y = v.y / len 
    };
    return dir;
}
