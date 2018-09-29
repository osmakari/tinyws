#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

typedef enum { CONTINUE = 0x00, TEXT = 0x01, BINARY = 0x02, CLOSE = 0x08, PING = 0x09, PONG = 0x0A } WS_FRAME_TYPE;

struct websocket {
    int socket;
    uint8_t state;
    char *url;
    struct sockaddr_in seraddr;
    char *rx_buffer;
    char *tx_buffer;
    char *origin;
    char *host;
};

struct websocket *WS (char *ip, uint16_t port, char *url, char *origin, char *host);
uint8_t WS_close (struct websocket *w);
int WS_receive (struct websocket *w, char *o_buffer);
int WS_send (struct websocket *w, char *data, uint8_t length, WS_FRAME_TYPE datatype);

#endif