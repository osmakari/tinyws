#include "websocket.h"
#include "b64.h"
#include "sha1.h"
uint8_t _ws_handshake (struct websocket *w) {
    const char *buffer = "GET %s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: %s\r\nOrigin: %s\r\nSec-WebSocket-Version: 13\r\n\r\n";
    
    srand ( time(NULL) );
    char rmb[17];
    for(int x = 0; x < 16; x++) {
        rmb[x] = rand() % 256;
    }

    rmb[16] = 0;

    int flen;
    char *base = base64(rmb, 16, &flen);
    char end_buffer[1024] = { 0 };
    
    sprintf(end_buffer, buffer, w->url, w->host, base, w->origin);

    write(w->socket, end_buffer, strlen(end_buffer));
    char sm[1024] = { 0 };
    int rcvd = 0;
    while(1) {
        
        if((rcvd = recv(w->socket, sm, 1024, 0)) < 0) {
            return 0x01; // receive failed
        }
        if(rcvd < 1)
            continue;
        if(strstr(sm, "HTTP/1.1 101 Switching Protocols") != 0) {
            return 0;
        }
        else {
            return 0x02; // Handshake fail
        }
    }
    return 0x03; // ??????
}

struct websocket *WS (char *ip, uint16_t port, char *url, char *origin, char *host) {
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        perror("Could not create socket\n");
        return NULL;
    }
    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed\n");
        return NULL;
    }

    struct websocket *ws = (struct websocket *)malloc(sizeof(struct websocket));
    ws->socket = sock;
    ws->state = 0xFF; // error state for now
    ws->url = (url == NULL ? "/" : url);
    ws->host = (host == NULL ? "localhost" : host);
    ws->origin = (origin == NULL ? "localhost" : origin);
    uint8_t ws_err;
    if((ws_err = _ws_handshake(ws)) != 0) {
        WS_close(ws);
        printf("WebSocket Handshake failed with error %i\n", ws_err);
        return NULL;
    }
    ws->state = 0x01;
    ws->rx_buffer = malloc(128);
    ws->tx_buffer = malloc(128);
}

uint8_t WS_close (struct websocket *w) {
    if(w == NULL) 
        return 0x01;
    
    close(w->socket);
    if(w->rx_buffer != NULL)
        free(w->rx_buffer);
    if(w->tx_buffer != NULL)
        free(w->tx_buffer);

    free(w);
    return 0;
}

int WS_receive (struct websocket *w, char *o_buffer) {
    if(w->state != 0x01) {
        printf("Websocket state not ready!\n");
        return -2;
    } 
    int rcvd = 0;
    while(1) {
        if((rcvd = recv(w->socket, w->rx_buffer, 128, 0)) < 0) {
            return -1; // receive failed
        }
        if(rcvd < 1) // prevent blank messages
            continue;

        break;   
    }
    
    uint8_t len = w->rx_buffer[1];
    len |= 0UL << 7;
    for(int x = 0; x < len; x++) {
        o_buffer[x] = w->rx_buffer[x + 2];
    }
    o_buffer[len + 2] = 0; 
    return len;
}

int WS_send (struct websocket *w, char *data, uint8_t length, WS_FRAME_TYPE datatype) {
    if(w->state != 0x01) {
        printf("Websocket state not ready!\n");
        return -2;
    }

    uint8_t frame_b_0 = 0;
    frame_b_0 |= 1UL << 7; // FIN bit to 1
    frame_b_0 |= 0UL << 6; // next 3 RSV bits to 0
    frame_b_0 |= 0UL << 5;
    frame_b_0 |= 0UL << 4;
    frame_b_0 |= (datatype >> 3 & 1) << 3; // Set 4 bit OP CODE
    frame_b_0 |= (datatype >> 2 & 1) << 2;
    frame_b_0 |= (datatype >> 1 & 1) << 1;
    frame_b_0 |= (datatype >> 0 & 1) << 0;

    uint8_t frame_b_1 = 0;
    uint8_t data_len = strlen(data);
    frame_b_1 = data_len;
    frame_b_1 |= 0UL << 7; // MASK bit to 0
    /*
    frame_b_1 |= (data_len >> 6 & 1) << 6; // set 7-bit length
    frame_b_1 |= (data_len >> 5 & 1) << 5;
    frame_b_1 |= (data_len >> 4 & 1) << 4;
    frame_b_1 |= (data_len >> 3 & 1) << 3;
    frame_b_1 |= (data_len >> 2 & 1) << 2;
    frame_b_1 |= (data_len >> 1 & 1) << 1;
    frame_b_1 |= (data_len >> 0 & 1) << 0;
    */
    for(int x = 0; x < length; x++) {
        w->tx_buffer[x + 2] = data[x];
    }
    w->tx_buffer[0] = frame_b_0;
    w->tx_buffer[1] = frame_b_1;

    write(w->socket, w->tx_buffer, length + 2);
    return 0;
}