#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <raylib.h>

#include "controls.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define AA_SIZE_INCREMENT 0.3

U64 lockout = 0;

#define NULL_REF ((ArenaKey) { .gen = 0, .idx = 0 })
bool is_null_ref(ArenaKey k) {
    return (k.idx == 0) & (k.gen == 0);
}

// Animation ----------------------------------------------

typedef struct {
    F32 r, g, b, a;
} Colourf;

Colourf to_colourf(Color c) {
    return (Colourf) {
        .r = ((F32) c.r) / 255.0f,
        .g = ((F32) c.g) / 255.0f,
        .b = ((F32) c.b) / 255.0f,
        .a = ((F32) c.a) / 255.0f,
    };
}

Color to_color(Colourf c) {
    return (Color) {
        .r = c.r * 255u,
        .g = c.g * 255u,
        .b = c.b * 255u,
        .a = c.a * 255u,
    };
}

typedef struct {
    Colourf colour_mul;
    Colourf colour_sum;
    Vector2 scale;
    Vector2 position;
} AnimationFrame;

_Static_assert(
    sizeof(AnimationFrame) == sizeof(F32) * (4+4+2+2),
    "AnimationFrame must not contain padding"
);

AnimationFrame default_animation_frame = {
    .colour_mul = { 1.0, 1.0, 1.0, 1.0 },
    .colour_sum = { 0.0, 0.0, 0.0, 0.0 },
    .scale = { 1.0, 1.0 },
    .position = { 0.0, 0.0 }
};

typedef U32 AnimDirective;
#define ANIM_DIRECTIVE_COMMAND_START        0u
#define ANIM_DIRECTIVE_COMMAND_BITS         3u
#define ANIM_DIRECTIVE_COMMAND_MASK         (((1u << ANIM_DIRECTIVE_COMMAND_BITS) - 1) << ANIM_DIRECTIVE_COMMAND_START)
#define ANIM_DIRECTIVE_COMMAND_END          0u
#define ANIM_DIRECTIVE_COMMAND_WAIT_UNTIL   1u
#define ANIM_DIRECTIVE_COMMAND_WAIT_FOR     2u
#define ANIM_DIRECTIVE_COMMAND_SET          3u
#define ANIM_DIRECTIVE_COMMAND_LINEAR_INCREMENT 4u

// offset in AnimationFrame and number of floats to modify
#define ANIM_DIRECTIVE_OFFSET_START         (ANIM_DIRECTIVE_COMMAND_START+ANIM_DIRECTIVE_COMMAND_BITS)
#define ANIM_DIRECTIVE_OFFSET_BITS          8u
#define ANIM_DIRECTIVE_OFFSET_MASK          (((1u << ANIM_DIRECTIVE_OFFSET_BITS) - 1) << ANIM_DIRECTIVE_OFFSET_START)

#define ANIM_DIRECTIVE_LEN_START            (ANIM_DIRECTIVE_OFFSET_START+ANIM_DIRECTIVE_OFFSET_BITS)
#define ANIM_DIRECTIVE_LEN_BITS             8u
#define ANIM_DIRECTIVE_LEN_MASK             (((1u << ANIM_DIRECTIVE_LEN_BITS) - 1) << ANIM_DIRECTIVE_LEN_START)

typedef union {
    AnimDirective directive;
    F32 f;
} AnimData;

typedef struct {
    AnimData* data; 
} Animation;

AnimationFrame animation_frame_from_scratch(Animation * anim, F32 frame);

#define ANIM_DIRECTIVE(cmd, offset, len) \
    { .directive = cmd | (offset << ANIM_DIRECTIVE_OFFSET_START) | (len << ANIM_DIRECTIVE_LEN_START) }

#define ANIM_WAIT_UNTIL(frame) \
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_WAIT_UNTIL, 0, 1), \
    { .f = frame }

#define ANIM_WAIT_FOR(frame) \
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_WAIT_FOR, 0, 1), \
    { .f = frame }

#define ANIM_COLOUR_MUL_SET(r, g, b, a) \
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_SET, offsetof(AnimationFrame, colour_mul)/4, 4), \
    { .f = r }, { .f = g }, { .f = b }, { .f = a }

#define ANIM_COLOUR_MUL_INCR(r, g, b, a) \
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_LINEAR_INCREMENT, offsetof(AnimationFrame, colour_mul)/4, 4), \
    { .f = r }, { .f = g }, { .f = b }, { .f = a }

#define ANIM_SCALE_SET(x, y) \
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_SET, offsetof(AnimationFrame, scale)/4, 2), \
    { .f = x }, { .f = y }

#define ANIM_SCALE_INCR(x, y) \
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_LINEAR_INCREMENT, offsetof(AnimationFrame, scale)/4, 2), \
    { .f = x }, { .f = y }

#define ANIM_POSITION_SET(x, y) \
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_SET, offsetof(AnimationFrame, position)/4, 2), \
    { .f = x }, { .f = y }

#define ANIM_POSITION_INCR(x, y) \
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_LINEAR_INCREMENT, offsetof(AnimationFrame, position)/4, 2), \
    { .f = x }, { .f = y }

#define COOLDOWN_COLOUR 0.75f
#define DELTA ((1.0f - COOLDOWN_COLOUR) / 25.0f)

// GFX --------------------------------------------------------------------------

// anim may be null
void draw_ellipse(AnimationFrame* anim, Vector2 centre, Vector2 radius, Color colour);
void draw_circle(AnimationFrame* anim, Vector2 centre, F32 radius, Color colour);
void draw_rounded_rectangle(AnimationFrame* anim, Vector2 centre, Vector2 size, F32 roundness, Color colour);

// GENERAL -------------------------------------------------------------

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
#define ENTITY_TYPE_HUD (1u << 4)
#define ENTITY_TYPE_EFFECT (1u << 5)
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

typedef enum {
    TankState_Normal = 0,
    TankState_Cooldown,
    TankState_Hitstop,
    TankState_Knockback,
} TankState;

bool actionable_state(TankState s) {
    switch (s) {
        case TankState_Normal: return true;
        case TankState_Cooldown: return true;
        case TankState_Hitstop: return false;
        case TankState_Knockback: return false;
        default: {
            fprintf(stderr, "fn actionable_state: unhandled tank state\n");
            return false;
        }
    }
}

typedef union {
    U32 cooldown_timer;
    struct {
        Vector2 queued_knockback;
        Vector2 position_delta;
        U32 queued_knockback_timer;
        U32 timer;
    } hitstop;
    U32 knockback_timer;
} TankStateData;

typedef struct {
    F32 max_speed;
    F32 acceleration;
    F32 angle_acceleration;
    F32 angle_max_speed;
    F32 angle_max_speed_fast;
    F32 size;
    F32 velocity_decay;
    F32 angle_velocity_decay;
    F32 knockback_decay_factor;
    F32 knockback_decay_constant;
    I32 bullet_cooldown;
    F32 max_health;
} TankStats;

typedef struct Tank {
    EntityRef e;
    TankStats* stats; // never null
    PlayerControls* controls; // might be null
    Color body_colour;
    F32 velocity;
    F32 angle; // degrees
    F32 angle_velocity;
    F32 health;

    Vector2 knockback_velocity;
    TankState state;
    TankStateData state_data;

    Animation* anim; // might be null
    F32 anim_frame;
} Tank;

#define ARENA_TYPE Tank
#include <arena.h>
typedef ArenaKey TankRef;

typedef struct {
    Vector2 position;
    TankStats* stats; // never null
    PlayerControls* controls; // might be null
    Color body_colour;
    F32 angle; // degrees
    void (*update)(struct Entity* this);
} TankInit;

TankRef insert_tank(TankInit init);
void destroy_tank(Entity* t);
TankRef tank_ref(Tank* t);

void set_tank_state(Tank* t, TankState state, Animation* anim);
void update_tank_player(Entity* e);
void update_tank_training_dummy(Entity* e);
void handle_collision_tank(Entity* this, Entity* other);
void draw_tank(Entity* e);

typedef struct {
    Vector2 knockback;
    Vector2 hit_position;
    F32 damage;
} HitInfo;

void hit_tank(Entity* e, Tank* t, HitInfo info);

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

    TankRef owner;
    Vector2 direction;
    F32 speed;
    F32 damage;
    //int subaction_index;
} Bullet;

#define ARENA_TYPE Bullet
#include <arena.h>
typedef ArenaKey BulletRef;

EntityRef insert_bullet(Bullet tank, Entity e);
void destroy_bullet(Entity* b);

void update_bullet(Entity* e);
void handle_collision_bullet(Entity* this, Entity* other);
void draw_bullet(Entity* e);

void insert_borders();
void draw_wall(Entity* e);

// Effects -------------------------------------------------

typedef struct {
    U32 particle_count;
    F32 angle_variance;
    F32 initial_speed;
    F32 speed_variance;
    F32 acceleration;
    F32 size;
    F32 size_variance;
    F32 roundness;

    F32 particle_lifetime;
    F32 particle_lifetime_variance;
    F32 total_lifetime;
    F32 initial_spawn_rate;

    Color colour;
} EffectStats;

typedef struct {
    U16 progress;
    U8 flags;
    I8 angle_delta;
    I8 speed_delta;
    I8 angle_speed_delta;
    I8 size_delta;
    I8 lifetime_delta;
} EffectParticle;

typedef struct {
    EntityRef e;
    F32 lifetime;
    F32 angle;
    EntityRef follow; // may be a null ref
    EffectStats* stats;
    EffectParticle* spawned;
} Effect;

#define ARENA_TYPE Effect
#include <arena.h>
typedef ArenaKey EffectRef;

#define PARTICLE_FLAGS_ALIVE 1

EffectParticle random_effect_particle() {
    return (EffectParticle) {
        .progress = 0u,
        .flags = PARTICLE_FLAGS_ALIVE,
        .angle_delta = (I8)GetRandomValue(-128, 127),
        .speed_delta = (I8)GetRandomValue(-128, 127),
        .angle_speed_delta = (I8)GetRandomValue(-128, 127),
        .size_delta = (I8)GetRandomValue(-128, 127),
        .lifetime_delta = (I8)GetRandomValue(-128, 127),
    };
}

void draw_effect(Entity* e);
void update_effect(Entity* e);

typedef struct {
    Vector2 position;
    U32 layer;
    F32 angle;
    EntityRef follow; // may be a null ref
    EffectStats* stats;
} EffectInit;

EffectRef insert_effect(EffectInit init);
void destroy_effect(Entity* t);
EffectRef effect_ref(Effect* t);

// State ------------------------------------------------

typedef struct {
    Arena_Entity entities;
    Arena_Tank tanks;
    Arena_Bullet bullets;
    Arena_Effect effects;

    bool finish;
} GameState;

GameState init_game_state(void) {
    return (GameState) {
        .entities = arena_create_Entity(),
        .tanks = arena_create_Tank(),
        .bullets = arena_create_Bullet(),
        .effects = arena_create_Effect(),

        .finish = false,
    };
}

// assumes that new is initialized
void copy_game_state(GameState* old, GameState* new) {
    memcpy(new->entities.tracking.free, old->entities.tracking.free, ARENA_MAX_ELEMENTS / 8);
    memcpy(new->entities.tracking.generations, old->entities.tracking.generations, ARENA_MAX_ELEMENTS * sizeof(ArenaGen));
    memcpy(new->entities.backing, old->entities.backing, ARENA_MAX_ELEMENTS * sizeof(Entity));
    new->entities.tracking.element_num = old->entities.tracking.element_num;

    memcpy(new->tanks.tracking.free, old->tanks.tracking.free, ARENA_MAX_ELEMENTS / 8);
    memcpy(new->tanks.tracking.generations, old->tanks.tracking.generations, ARENA_MAX_ELEMENTS * sizeof(ArenaGen));
    memcpy(new->tanks.backing, old->tanks.backing, ARENA_MAX_ELEMENTS * sizeof(Tank));
    new->tanks.tracking.element_num = old->tanks.tracking.element_num;

    memcpy(new->bullets.tracking.free, old->bullets.tracking.free, ARENA_MAX_ELEMENTS / 8);
    memcpy(new->bullets.tracking.generations, old->bullets.tracking.generations, ARENA_MAX_ELEMENTS * sizeof(ArenaGen));
    memcpy(new->bullets.backing, old->bullets.backing, ARENA_MAX_ELEMENTS * sizeof(Bullet));
    new->bullets.tracking.element_num = old->bullets.tracking.element_num;

    memcpy(new->effects.tracking.free, old->effects.tracking.free, ARENA_MAX_ELEMENTS / 8);
    memcpy(new->effects.tracking.generations, old->effects.tracking.generations, ARENA_MAX_ELEMENTS * sizeof(ArenaGen));
    memcpy(new->effects.backing, old->effects.backing, ARENA_MAX_ELEMENTS * sizeof(Effect));
    new->effects.tracking.element_num = old->effects.tracking.element_num;
}

void reset_game_state(GameState* st) {
    arena_tracking_reset(&st->entities.tracking);
    arena_tracking_reset(&st->tanks.tracking);
    arena_tracking_reset(&st->bullets.tracking);
    arena_tracking_reset(&st->effects.tracking);
    st->finish = false;
}

void dealloc_game_state(GameState* st) {
    arena_dealloc_Entity(&st->entities);
    arena_dealloc_Tank(&st->tanks);
    arena_dealloc_Bullet(&st->bullets);
    arena_dealloc_Effect(&st->effects);
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

EntityRef insert_player_hud(TankRef player, Vector2 hud_pos, Vector2 hud_size);
void draw_player_hud(Entity* e);
//void draw_general_hud();

typedef struct {
} TrainingMenuState;

void draw_training_menu(TrainingMenuState* tm_state);


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

F32 vector2_angle(Vector2 a) {
    F32 res = atan2(a.y, a.x);   
    if (res < 0.0f)
        res += 2.0*PI;

    // convert to degrees
    return res * 57.29577950f;
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

F32 random_angle() {
    int r = GetRandomValue(0, 359);
    return (F32)r;
}
