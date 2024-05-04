typedef enum {
    PlayerInput_Forward,
    PlayerInput_Backward,
    PlayerInput_TurnLeft,
    PlayerInput_TurnRight,

    PlayerInput_TurnModifier,

    PlayerInput_Attack1,
    PlayerInput_Attack2,
    PlayerInput_Attack3,
    PlayerInput_Attack4,
} PlayerInput;

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

typedef struct {
    PlayerInputMapping map[9];
} Controls;

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

