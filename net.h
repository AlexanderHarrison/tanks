#pragma once

#include <uv.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <raylib.h>

typedef struct {
    uv_loop_t loop;
    struct sockaddr_in6* ip_mine;
    struct sockaddr_in6* ip_opponent;

    uv_udp_t send_socket;
    uv_udp_t recv_socket;
} Net;

typedef struct Message {
    struct Message* next_message;
    uint64_t frame;
    Vector2 position;
    float angle;
} Message;

int init_net(Net* net);
void deinit_net(Net* net);

int send_message(Net* net, Message* message);
Message* check_messages(Net* net);
