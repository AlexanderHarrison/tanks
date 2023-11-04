#include "net.h"

void on_send(uv_udp_send_t* req, int status) {
    (void)req;

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

int init_net(Net* net) {
    int ret;

    uv_loop_init(&net->loop);

    // init ips ----------------------------------------------------------
    
    int count;
    uv_interface_address_t* interfaces;
    uv_interface_addresses(&interfaces, &count);
        
    int found_interface = 0;
    for (int i = 0; i < count; ++i) {
        printf("interface name: %s\n", interfaces[i].name);
        if (!interfaces[i].is_internal) {
            if (strncmp(interfaces[i].name, "wlan", 4) != 0) {
                printf("  valid ip\n");
                net->ip_mine = &interfaces[i].address.address6;
                found_interface = 1;
            }
        }
    }
    uv_free_interface_addresses(interfaces, count);
    if (!found_interface) {
        fprintf(stderr, "no interface\n");
        return 1;
    }
    char my_ip[64];
    uv_ip6_name(net->ip_mine, my_ip, 64);
    printf("user ip: %s\n", my_ip);

    net->ip_mine = malloc(sizeof(struct sockaddr_in));
    ret = uv_ip6_addr(my_ip, 12361, net->ip_mine);
    if (ret) {
        fprintf(stderr, "Ip error 1: %s\n", uv_strerror(ret));
        return 1;
    }

    char ip[64];
    net->ip_opponent = malloc(sizeof(struct sockaddr_in));
    printf("enter opponent ip: ");
    fgets(ip, 64, stdin);
    char* ipptr = trim(ip);
    if (ipptr[0]) {
        ret = uv_ip6_addr(ipptr, 12361, net->ip_opponent);
        if (ret) {
            fprintf(stderr, "Ip error 2: %s\n", uv_strerror(ret));
            return 1;
        }
    } else {
        uv_ip6_addr(my_ip, 12361, net->ip_opponent);
    }

    // open connection --------------------------------------------------------

    uv_udp_init_ex(&net->loop, &net->recv_socket, UV_UDP_RECVMMSG);
    uv_udp_init(&net->loop, &net->send_socket);
    ret = uv_udp_bind(&net->recv_socket, (const struct sockaddr*)net->ip_mine, UV_UDP_REUSEADDR);
    if (ret) {
        fprintf(stderr, "bind error: %s\n", uv_strerror(ret));
        return 1;
    }

    return 0;
}

int send_message(Net* net, Message* message) {
    uv_buf_t send_buf = uv_buf_init((char*)message, sizeof(Message));
    int ret = uv_udp_try_send(&net->send_socket, &send_buf, 1, (const struct sockaddr*)net->ip_opponent);
    if (ret < 0) {
        fprintf(stderr, "Send error: %s\n", uv_strerror(ret));
        return 1;
    }

    return 0;
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
