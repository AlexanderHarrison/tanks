#pragma once

#include <math.h>

#include "controls.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

// GENERAL -------------------------------------------------------------

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
        char* err;
        Controls* set_controls_target;
    };
} SceneTransition;

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
    }

    return false;
}

// PLAYER ------------------------------------------------------------

typedef struct {
    float max_speed;
    float acceleration;
    float angle_acceleration;
    float angle_max_speed;
    float angle_max_speed_fast;
    float size;
    float visual_size;
    float hurtbox_size;
    int bullet_cooldown;
    Color body_colour;

    Vector2 position;
    float velocity;
    float angle; // degrees
    float angle_velocity;
    int bullet_timer;
    bool dead;
} Player;

static const Player default_player = {
    .max_speed = 8.0f,
    .acceleration = 0.5f,
    .angle_max_speed = 2.0f,
    .angle_max_speed_fast = 9.0f,
    .angle_acceleration = 1.5f,
    .visual_size = 20.0f,
    .hurtbox_size = 18.0f,
    .bullet_cooldown = 10,
    .body_colour = { 190, 33, 55, 255 },
    .max_health = 100,

    .position = { .x = SCREEN_WIDTH / 2, .y = 3 * SCREEN_HEIGHT / 4 },
    .velocity = 0,
    .angle = -90.0f,
    .angle_velocity = 0.0f,
    .bullet_timer = 3,
    .health = 100;
};

Controls player1_controls = {
    .map = {
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_W }, // forward
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_S }, // backward
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_A }, // turn cc
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_D }, // turn cl
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_LEFT_SHIFT }, // turn mod
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_ONE }, // atk1
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_TWO }, // atk2
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_THREE }, // atk3
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_FOUR }, // atk4
    }
};

Controls player2_controls = {
    .map = {
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_I }, // forward
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_K }, // backward
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_J }, // turn cc
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_L }, // turn cl
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_SPACE }, // turn mod
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_NINE }, // atk1
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_ZERO }, // atk2
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_MINUS }, // atk3
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_EQUAL }, // atk4
    }
};

void update_player(Player* player, Controls* controls);

void draw_player(Player* player);

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

SceneTransition run_menu(const MenuItem* menu, int menu_len);
SceneTransition run_singleplayer(void);
SceneTransition run_multiplayer(void);
SceneTransition run_set_controls(Controls* target_controls);

// BULLET -------------------------------------------------------------

typedef enum {
    OwnerType_Player,
    OwnerType_Enemy,
} OwnerType;

typedef struct {
    OwnerType type;
    union {
        Player* player;
    };
} Owner;

#define MAX_BULLETS 512

typedef struct {
    Vector2 position;
    Vector2 update_vector;
    float collision_size;
    float damage;
    Owner owner;
} Bullet;

Bullet bullets[MAX_BULLETS];
int bullet_count = 0;

int spawn_bullet(Bullet);
void update_bullet_positions(void);
void remove_bullet(int i);
void draw_bullets(void);

// ENEMIES -------------------------------------------------------------

#define MAX_ENEMIES 512

#define ENEMY_VISUAL_SIZE 25.0
#define ENEMY_COLLISION_SIZE 15.0
#define ENEMY_SPEED 1.0

typedef int EnemyUniqueData_None;

typedef enum {
    EnemyType_Test,
} EnemyType;

typedef enum {
    EnemyFlag_Invincible = 1,
} EnemyFlags;

typedef struct {
    Vector2 position;
    float enemy_collision_size;
    float bullet_collision_size;
    float health;
    EnemyFlags flags;
    EnemyType type;
} EnemyTag;

typedef union {
    EnemyUniqueData_None none;   
} EnemyUniqueData;

// used mostly in update function
typedef struct {
    Vector2 knockback;
    int knockback_timer;
    int invincibility_timer;
    int death_time;
    int death_timer;
    EnemyUniqueData unique_data;
} EnemyData;

typedef struct {
    EnemyTag tag;
    EnemyData data;
} Enemy;

EnemyTag enemies[MAX_ENEMIES];
EnemyData enemy_data[MAX_ENEMIES];
int enemy_count = 0;

int spawn_enemy(Enemy);
void remove_enemy(int);
void update_enemies(Player* player);
void resolve_enemy_enemy_collisions(void);
void resolve_enemy_bullet_collisions(void);
void resolve_player_bullet_collisions(Player** players, int player_count);
void resolve_player_player_collisions(Player** players, int player_count);
void draw_enemies(void);

Enemy enemy_test(Vector2 position) {
    EnemyTag tag = {
        .position = position,
        .enemy_collision_size = ENEMY_COLLISION_SIZE,
        .bullet_collision_size = ENEMY_VISUAL_SIZE,
        .health = 30.0,
        .flags = 0,
        .type = EnemyType_Test
    };

    EnemyUniqueData unique_data = { .none = 0 };
    EnemyData data = {
        .knockback = { .x = 0.0, .y = 0.0 },
        .knockback_timer = 0,
        .invincibility_timer = 0,
        .death_time = 0,
        .death_timer = 0,
        .unique_data = unique_data
    };

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
