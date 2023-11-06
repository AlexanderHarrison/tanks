#include "net_uv.h"

#define PORT 49069

void on_send(uv_udp_send_t* req, int status) {
    (void)req;

    printf("sent\n");

    if (status) {
        fprintf(stderr, "Send error: %s\n", uv_strerror(status));
        return;
    }
}

Message* message_queue = NULL;
Message* message_tail = NULL;
void on_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
    (void)flags;
    (void)addr;

    printf("received\n");

    if (nread == sizeof(Message)) {
        if (message_tail != NULL) {
            Message* new_tail = (Message*) buf->base;
            message_tail->next_message = new_tail;
            message_tail = new_tail;
        } else {
            message_queue = (Message*) buf->base;
            message_tail = message_queue;
        }
    } else {
        fprintf(stderr, "Recv error: %s\n", uv_strerror(nread));
        uv_close((uv_handle_t*) handle, NULL);
    }
    uv_udp_recv_stop(handle);
}

char* trim(char* s) {
    char* out = s;
    int i = 0;

    while (1) { 
        char c = s[i];
        if (c == ' ' || c == '\n' || c == '\t') { ++out; } else { break; }
        ++i;
    }

    while (1) {
        char c = s[i];
        if (c == ' ' || c == '\n' || c == '\t') {
            s[i] = '\0';
            break;
        } else if (c == '\0') { break; }
        ++i;
    }

    return out;
}

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    (void)handle;

    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

// returns a pointer to a linked list of messages in the order they were captured.
// or null if there are no messages;
// you are now in charge of freeing them.
Message* check_messages(Net* net) {
    uv_udp_recv_start(&net->recv_socket, alloc_buffer, on_recv);
    uv_run(&net->loop, UV_RUN_NOWAIT);
    Message* queue = message_queue;
    message_queue = NULL;
    message_tail = NULL;
    return queue;
}

uv_udp_send_t send_req;
int send_message(Net* net, Message* message) {
    uv_buf_t send_buf = uv_buf_init((char*)message, sizeof(Message));
    int ret = uv_udp_send(&send_req, &net->recv_socket, &send_buf, 1, NULL, on_send);
    if (ret < 0) {
        fprintf(stderr, "Send error: %s\n", uv_strerror(ret));
        return 1;
    }

    //int ret = uv_udp_try_send(&net->send_socket, &send_buf, 1, (const struct sockaddr*)net->ip_opponent);
    //int ret = uv_udp_try_send(&net->send_socket, &send_buf, 1, NULL);
    //int ret = uv_udp_try_send(&net->recv_socket, &send_buf, 1, NULL);
    //if (ret != sizeof(Message)) {
    //    fprintf(stderr, "Send error: %s\n", uv_strerror(ret));
    //    return 1;
    //}

    return 0;
}


int init_net(Net* net) {
    int ret;

    uv_loop_init(&net->loop);

    // init ips ----------------------------------------------------------
    
    int count;
    uv_interface_address_t* interfaces;
    uv_interface_addresses(&interfaces, &count);
        
    int interface_idx = 0;
    char ip[128];
    for (int i = 0; i < count; ++i) {
        if (!interfaces[i].is_internal) {
            if (strncmp(interfaces[i].name, "wlan", 4) != 0) {
                uv_ip4_name(&interfaces[i].address.address4, ip, 128);
                if (strlen(ip) < 12) { continue; }
                interface_idx = if_nametoindex(interfaces[i].name);
                net->ip_mine = &interfaces[i].address.address4;
                break;
            }
        }
    }
    uv_free_interface_addresses(interfaces, count);
    if (!interface_idx) {
        fprintf(stderr, "no interface\n");
        return 1;
    }

    printf("ip: %s\n", ip);
    net->ip_mine = malloc(sizeof(struct sockaddr_in));
    //ret = uv_ip4_addr(ip, PORT, net->ip_mine);
    ret = uv_ip4_addr("0.0.0.0", PORT, net->ip_mine);
    if (ret) {
        fprintf(stderr, "Ip error 1: %s\n", uv_strerror(ret));
        return 1;
    }

    char opp_ip[128];
    printf("enter opponent ip: ");
    fgets(opp_ip, 128, stdin);
    char* ipptr = trim(opp_ip);
    if (ipptr[0]) {
        net->ip_opponent = malloc(sizeof(struct sockaddr_in));
        ret = uv_ip4_addr(ipptr, PORT, net->ip_opponent);
    } else {
        printf("local address\n");
        //net->ip_opponent = net->ip_mine;
        //net->ip_mine = malloc(sizeof(struct sockaddr_in6));
        //uv_ip6_addr("0:0:0:0:0:0:0:1", PORT, net->ip_mine);
        
        net->ip_opponent = malloc(sizeof(struct sockaddr_in));
        ret = uv_ip4_addr(ip, PORT, net->ip_opponent);
    }
    if (ret) {
        fprintf(stderr, "Ip error 2: %s\n", uv_strerror(ret));
        return 1;
    }

    // open connection --------------------------------------------------------

    ret = uv_udp_init(&net->loop, &net->recv_socket);
    if (ret) { fprintf(stderr, "init error 1: %s\n", uv_strerror(ret)); return 1; }

    ret = uv_udp_bind(&net->recv_socket, (const struct sockaddr*)net->ip_mine, UV_UDP_REUSEADDR);
    if (ret < 0) { fprintf(stderr, "bind error 1: %s\n", uv_strerror(ret)); return 1; }

    ret = uv_udp_connect(&net->recv_socket, (const struct sockaddr*)net->ip_opponent);
    if (ret < 0) { fprintf(stderr, "connect error 1: %s\n", uv_strerror(ret)); return 1; }

    // syncronize programs -------------------------------------------------
    
    printf("syncronising\n");

    // first message is just timing.
    Message null;
    send_message(net, &null);
    printf("sent sync message\n");
    struct timespec t = {
        .tv_sec = 0,
        .tv_nsec = 10000000L
    };
    while (check_messages(net) == NULL) {
        nanosleep(&t, NULL);
    }
    printf("received sync message\n");

    return 0;
}

//int init_net6(Net* net) {
//    int ret;
//
//    uv_loop_init(&net->loop);
//
//    // init ips ----------------------------------------------------------
//    
//    int count;
//    uv_interface_address_t* interfaces;
//    uv_interface_addresses(&interfaces, &count);
//        
//    int interface_idx = 0;
//    char ip[128];
//    for (int i = 0; i < count; ++i) {
//        if (!interfaces[i].is_internal) {
//            if (strncmp(interfaces[i].name, "wlan", 4) != 0) {
//                uv_ip6_name(&interfaces[i].address.address6, ip, 128);
//                if (strlen(ip) < 20) { continue; }
//                interface_idx = if_nametoindex(interfaces[i].name);
//
//                net->ip_mine = &interfaces[i].address.address6;
//            }
//        }
//    }
//    uv_free_interface_addresses(interfaces, count);
//    if (!interface_idx) {
//        fprintf(stderr, "no interface\n");
//        return 1;
//    }
//
//    int ip_end_idx = 0;
//    while (ip[ip_end_idx]) { ++ip_end_idx; }
//    ip[ip_end_idx] = '%';
//    ++ip_end_idx;
//    char ip_iid[64];
//    size_t len = sizeof(ip_iid);
//    uv_if_indextoiid(interface_idx, ip_iid, &len);
//    strcpy(&ip[ip_end_idx], ip_iid);
//    printf("ip: %s\n", ip);
//    net->ip_mine = malloc(sizeof(struct sockaddr_in6));
//    ret = uv_ip6_addr(ip, PORT, net->ip_mine);
//    //ret = uv_ip6_addr("::", PORT, net->ip_mine);
//    if (ret) {
//        fprintf(stderr, "Ip error 1: %s\n", uv_strerror(ret));
//        return 1;
//    }
//
//    char opp_ip[128];
//    printf("enter opponent ip: ");
//    fgets(opp_ip, 128, stdin);
//    char* ipptr = trim(opp_ip);
//    if (ipptr[0]) {
//        net->ip_opponent = malloc(sizeof(struct sockaddr_in6));
//        ret = uv_ip6_addr(ipptr, PORT, net->ip_opponent);
//    } else {
//        printf("local address\n");
//        //net->ip_opponent = net->ip_mine;
//        //net->ip_mine = malloc(sizeof(struct sockaddr_in6));
//        //uv_ip6_addr("0:0:0:0:0:0:0:1", PORT, net->ip_mine);
//        
//        net->ip_opponent = malloc(sizeof(struct sockaddr_in6));
//        ret = uv_ip6_addr(ip, PORT, net->ip_opponent);
//    }
//    if (ret) {
//        fprintf(stderr, "Ip error 2: %s\n", uv_strerror(ret));
//        return 1;
//    }
//
//    // open connection --------------------------------------------------------
//
//    ret = uv_udp_init(&net->loop, &net->recv_socket);
//    if (ret) { fprintf(stderr, "init error 1: %s\n", uv_strerror(ret)); return 1; }
//
//    ret = uv_udp_bind(&net->recv_socket, (const struct sockaddr*)net->ip_mine, UV_UDP_IPV6ONLY | UV_UDP_REUSEADDR);
//    if (ret < 0) { fprintf(stderr, "bind error 1: %s\n", uv_strerror(ret)); return 1; }
//
//    ret = uv_udp_connect(&net->recv_socket, (const struct sockaddr*)net->ip_opponent);
//    if (ret < 0) { fprintf(stderr, "connect error 1: %s\n", uv_strerror(ret)); return 1; }
//
//    return 0;
//}

void close_handles(uv_handle_t *handle, void *arg) {
    (void)arg;
    uv_close(handle, 0);
}

void deinit_net(Net* net) {
    uv_udp_recv_stop(&net->recv_socket);
    uv_walk(&net->loop, close_handles, 0);

    uv_run(&net->loop, UV_RUN_DEFAULT);
    if (net->ip_opponent) free(net->ip_opponent);
    if (net->ip_mine) free(net->ip_mine);

    while (message_queue != NULL) {
        Message *message = message_queue;
        message_queue = message_queue->next_message;
        free(message);
    }

    uv_loop_close(&net->loop);
}
