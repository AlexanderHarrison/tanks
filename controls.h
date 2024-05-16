#include <tools.h>

typedef enum {
    PlayerInput_Forward = 0,
    PlayerInput_Backward,
    PlayerInput_TurnLeft,
    PlayerInput_TurnRight,

    PlayerInput_TurnModifier,

    PlayerInput_Attack1,
    PlayerInput_Attack2,
    PlayerInput_Attack3,
    PlayerInput_Attack4,
} PlayerInput;

typedef enum {
    TrainingInput_Reset,
    TrainingInput_TogglePlaying,
    TrainingInput_FrameAdvance,
    TrainingInput_SaveState,
    TrainingInput_LoadState,
    TrainingInput_ToggleMenu,
} TrainingInput;

typedef struct {
    int gamepad;
    int button;
} GamepadInput;

typedef enum {
    PlayerInputSourceType_Unmapped,
    PlayerInputSourceType_Keyboard,
    PlayerInputSourceType_Gamepad,
} PlayerInputSourceType;

typedef struct {
    PlayerInputSourceType source_type;
    union {
        int keyboard_input;
        GamepadInput gamepad_input;
    };
} PlayerInputMapping;

#define TANK_CONTROLS_COUNT 9
typedef struct {
    PlayerInputMapping map[TANK_CONTROLS_COUNT];
} PlayerControls;

#define TRAINING_CONTROLS_COUNT 6
typedef struct {
    PlayerInputMapping map[TRAINING_CONTROLS_COUNT];
} TrainingControls;

PlayerControls player1_controls = {
    .map = {
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_W }, // forward
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_S }, // backward
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_A }, // turn cc
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_D }, // turn cl
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_LEFT_SHIFT }, // turn mod
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_H}, // atk1
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_J }, // atk2
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_K }, // atk3
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_L }, // atk4
    }
};

PlayerControls player2_controls = {
    .map = {
        { .source_type = PlayerInputSourceType_Unmapped }, // forward
        { .source_type = PlayerInputSourceType_Unmapped }, // backward
        { .source_type = PlayerInputSourceType_Unmapped }, // turn cc
        { .source_type = PlayerInputSourceType_Unmapped }, // turn cl
        { .source_type = PlayerInputSourceType_Unmapped }, // turn mod
        { .source_type = PlayerInputSourceType_Unmapped }, // atk1
        { .source_type = PlayerInputSourceType_Unmapped }, // atk2
        { .source_type = PlayerInputSourceType_Unmapped }, // atk3
        { .source_type = PlayerInputSourceType_Unmapped }, // atk4
    }
};

TrainingControls training_controls = {
    .map = {
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_R }, // reset
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_P }, // pause / play
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_F }, // frame advance
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_U }, // save state
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_I }, // load state
        { .source_type = PlayerInputSourceType_Keyboard, .keyboard_input = KEY_TAB }, // toggle menu
    }
};

bool training_input_pressed(TrainingControls* controls, TrainingInput input) {
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
            fprintf(stderr, "ERROR: training input case %i not handled\n", mapping.source_type);
            return false;
        }
    }

    return false;
}

bool training_input_down(TrainingControls* controls, TrainingInput input) {
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

bool player_input_down(PlayerControls* controls, PlayerInput input) {
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

bool player_input_pressed(PlayerControls* controls, PlayerInput input) {
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
