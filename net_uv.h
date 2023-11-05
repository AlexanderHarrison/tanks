#pragma once

#include <uv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <raylib.h>
#include <net/if.h>
#include <time.h>

typedef struct {
    uv_loop_t loop;
    //struct sockaddr_in6* ip_mine;
    //struct sockaddr_in6* ip_opponent;
    struct sockaddr_in* ip_mine;
    struct sockaddr_in* ip_opponent;

    uv_udp_t send_socket;
    uv_udp_t recv_socket;
} Net;

typedef struct {
    Vector2 position;
    Vector2 update_vector;
    float size;
    float damage;
} BulletSpawnMessage;

typedef struct Message {
    struct Message* next_message;
    uint64_t frame;
    Vector2 player_position;
    float player_angle;

    bool spawned_bullet;
    BulletSpawnMessage new_bullet;
} Message;

int init_net(Net* net);
void deinit_net(Net* net);

int send_message(Net* net, Message* message);
Message* check_messages(Net* net);
