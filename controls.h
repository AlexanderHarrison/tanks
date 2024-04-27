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
