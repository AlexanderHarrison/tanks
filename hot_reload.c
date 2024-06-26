#include "game.h"

AnimData test_animation_data[] = {
    ANIM_WAIT_UNTIL(5.0f),
    ANIM_POSITION_SET(10.0f, 0.0f),
    ANIM_WAIT_UNTIL(10.0f),
    ANIM_POSITION_SET(0.0f, 10.0f),

    ANIM_WAIT_UNTIL(15.0f),
    ANIM_POSITION_INCR(0.0f, -1.0f),
    ANIM_WAIT_UNTIL(25.0f),
    ANIM_POSITION_SET(0.0f, 0.0f),

    ANIM_SCALE_INCR(0.0f, 0.2f),
    ANIM_WAIT_UNTIL(35.0f),
    ANIM_SCALE_SET(1.0f, 1.0f),

    ANIM_WAIT_UNTIL(40.0f),
    ANIM_COLOUR_MUL_INCR(-0.1f, 0.0f, 0.0f, 1.0f),
    ANIM_WAIT_UNTIL(45.0f),
    ANIM_COLOUR_MUL_INCR(-0.1f, -0.1f, 0.0f, 1.0f),
    ANIM_WAIT_UNTIL(50.0f),
    ANIM_COLOUR_MUL_INCR(0.0f, -0.1f, -0.1f, 1.0f),
    ANIM_WAIT_UNTIL(55.0f),
    ANIM_COLOUR_MUL_INCR(0.0f, 0.0f, -0.1f, 1.0f),
    ANIM_WAIT_UNTIL(60.0f),
    ANIM_COLOUR_MUL_SET(0.0f, 0.0f, 0.0f, 1.0f),
    ANIM_WAIT_UNTIL(70.0f),
    ANIM_COLOUR_MUL_SET(1.0f, 1.0f, 1.0f, 1.0f),
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_END, 0, 0)
};
Animation test_animation = { .data = test_animation_data, };

AnimData shoot_animation_data[] = {
    ANIM_SCALE_INCR(-0.1f, -0.1f),

    ANIM_COLOUR_MUL_SET(COOLDOWN_COLOUR, COOLDOWN_COLOUR, COOLDOWN_COLOUR, 1.0f),
    ANIM_COLOUR_MUL_INCR(DELTA, DELTA, DELTA, 0.0f),

    ANIM_WAIT_UNTIL(5.0f),
    ANIM_SCALE_SET(0.8f, 0.8f),
    ANIM_SCALE_INCR(0.1f, 0.1f),
    ANIM_WAIT_UNTIL(10.0f),
    ANIM_SCALE_SET(1.2f, 1.2f),
    ANIM_SCALE_INCR(-0.01f, -0.01f),
    ANIM_WAIT_UNTIL(25.0f),
    ANIM_DIRECTIVE(ANIM_DIRECTIVE_COMMAND_END, 0, 0)
};
Animation shoot_animation = { .data = shoot_animation_data, };

// tank ------------------------------------------------------------

TankStats default_tank = {
    .max_speed = 8.0f,
    .acceleration = 0.5f,
    .angle_max_speed = 2.0f,
    .angle_max_speed_fast = 7.0f,
    .angle_acceleration = 1.0f,
    .size = 25.0f,
    .velocity_decay = 1.8f,
    .angle_velocity_decay = 1.8f,
    .knockback_decay_factor = 1.08f,
    .knockback_decay_constant = 0.2f,
    .bullet_cooldown = 25,
    .max_health = 100.0f,
};

// effects

EffectStats bullet_hit_wall_effect_stats_list[] = {
    {
        .particle_count = 40,
        .angle_variance = 70.0f,
        .acceleration = -0.2f,
        .initial_speed = 4.0f,
        .speed_variance = 1.0f,
        .size = 2.0f,
        .size_variance = 1.0f,
        .roundness = 2.0f,
        .particle_lifetime = 13.0f,
        .particle_lifetime_variance = 7.0f,
        .total_lifetime = 10.0f,
        .initial_spawn_rate = 100.0f,
        .colour = { 255, 20, 20, 100 },
        .render_layer = ENTITY_FLAGS_RENDER_LAYER_1,
    },
    {
        .particle_count = 30,
        .angle_variance = 80.0f,
        .acceleration = -0.1f,
        .initial_speed = 2.0f,
        .speed_variance = 1.0f,
        .size = 2.0f,
        .size_variance = 1.0f,
        .roundness = 2.0f,
        .particle_lifetime = 13.0f,
        .particle_lifetime_variance = 3.0f,
        .total_lifetime = 0.0f,
        .initial_spawn_rate = 100.0f,
        .colour = { 255, 200, 30, 140 },
        .render_layer = ENTITY_FLAGS_RENDER_LAYER_1,
    },
    {
        .particle_count = 20,
        .angle_variance = 50.0f,
        .acceleration = 0.1f,
        .initial_speed = 0.15f,
        .speed_variance = 0.1f,
        .size = 4.0f,
        .size_variance = 1.0f,
        .roundness = 1.0f,
        .particle_lifetime = 13.0f,
        .particle_lifetime_variance = 7.0f,
        .total_lifetime = 2.0f,
        .initial_spawn_rate = 5.0f,
        .colour = { 140, 80, 100, 100 },
        .render_layer = ENTITY_FLAGS_RENDER_LAYER_0,
    },
};
EffectStats* bullet_hit_wall_effect_stats = bullet_hit_wall_effect_stats_list;
U32 bullet_hit_wall_effect_stats_count = sizeof(bullet_hit_wall_effect_stats_list)/sizeof(*bullet_hit_wall_effect_stats_list);

EffectStats hit_tank_effect_stats_list[] = {
    {
        .particle_count = 120,
        .angle_variance = 80.0f,
        .acceleration = -0.1f,
        .initial_speed = 5.0f,
        .speed_variance = 1.0f,
        .size = 2.0f,
        .size_variance = 1.0f,
        .roundness = 2.0f,
        .particle_lifetime = 15.0f,
        .particle_lifetime_variance = 10.0f,
        .total_lifetime = 10.0f,
        .initial_spawn_rate = 20.0f,
        .colour = { 120, 80, 80, 150 },
        .render_layer = ENTITY_FLAGS_RENDER_LAYER_1,
    }
};
EffectStats* hit_tank_effect_stats = hit_tank_effect_stats_list;
U32 hit_tank_effect_stats_count = sizeof(hit_tank_effect_stats_list)/sizeof(*hit_tank_effect_stats_list);
