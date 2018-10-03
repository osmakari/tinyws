#include "websocket.h"
#include "b64.h"
#include "sha1.h"

#ifdef USE_PTHREADS
void *_ws_runner (void *wx) {
    struct websocket *w = (struct websocket*)wx;
    char rec[MAX_MESSAGE_LENGTH] = { 0 };
    int mesl = 0;
    while(w != NULL && w->state == 0x01) {
        
        if((mesl = WS_receive(w, rec)) < 0) {
            printf("Recv fail!\n");
            WS_close(w);
            
            break;
        }
        if(w->onmessage != NULL) {
            rec[mesl] = 0;
            w->onmessage(rec, mesl);
        }
        memset(rec, 0, MAX_MESSAGE_LENGTH);
    }
    printf("Runner stopped\n");
}
#endif

void set_bit (uint8_t *a, uint8_t index, uint8_t state) {
    if(state == 0) {
        *a &= ~(1UL << index);
    }
    else {
        *a |= 1UL << index;
    }
}

// Private function, ignore this
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
        
        if((rcvd = read(w->socket, sm, 1024)) < 0) {
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

/*
    Function:   WS
    Returns:    websocket connection object, NULL on error
    Parameters:
                parameter       description                 example
                char *ip        IP address for the socket   127.0.0.1
                u16 port        Port of the server          8080
                char *url       URL in the GET field        ws://127.0.0.1:8080/
                char *origin    Origin, default if NULL     localhost
                char *host      Host, default if NULL       localhost

*/
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
    ws->rx_buffer = malloc(MAX_MESSAGE_LENGTH);
    ws->tx_buffer = malloc(MAX_MESSAGE_LENGTH);
    ws->last_ping = time(NULL);
    #ifdef USE_PTHREADS
    ws->onclose = NULL;
    ws->onmessage = NULL;
    if(pthread_create(&ws->handle, NULL, _ws_runner, ws) < 0) {
        WS_close(ws);
        return NULL;
    }
    #endif
    return ws;
}
/*
    Function:   WS_close
    Returns:    0 if WebSocket was closed, 1 if websocket object is null
    Parameters:
                parameter               description
                struct websocket *w     websocket object
*/
uint8_t WS_close (struct websocket *w) {
    printf("Closing WebSocket..\n");
    if(w == NULL) 
        return 0x01;
    #ifdef USE_PTHREADS
    pthread_detach(w->handle);
    #endif    
    close(w->socket);
    if(w->rx_buffer != NULL)
        free(w->rx_buffer);
    if(w->tx_buffer != NULL)
        free(w->tx_buffer);



    free(w);
    return 0;
}

/*
    Function:   WS_receive
    Returns:    Message length on success, < 0 on fail
    Parameters:
                parameter               description
                struct websocket *w     websocket object
                char *o_buffer          Allocated memory for output message.
*/
int WS_receive (struct websocket *w, char *o_buffer) {
    if(w->state != 0x01) {
        printf("Websocket state not ready!\n");
        return -2;
    }
    int rcvd = 0;
    uint8_t opcode;
    uint8_t len;
    cont: // Jump here if received a control frame
    
    while(1) {
        if((rcvd = read(w->socket, w->rx_buffer, MAX_MESSAGE_LENGTH)) < 0) {
            return -1; // receive failed
        }
        if(rcvd < 1) // prevent blank messages
            continue;

        break;   
    }
    opcode = w->rx_buffer[0]; // last 4 bits of first byte is the opcode
    set_bit(&opcode, 7, 0);
    set_bit(&opcode, 6, 0);
    set_bit(&opcode, 5, 0);
    set_bit(&opcode, 4, 0);
    len = w->rx_buffer[1];

    set_bit(&len, 7, 0);
    

    if(opcode == 0x09) { // PING message, return with PONG
        
        WS_send(w, w->rx_buffer + 2, len, PONG);
        
        goto cont;
    }
    else if(opcode == 0x08) { // CLOSE
        for(int x = 0; x < len; x++) {
            o_buffer[x] = w->rx_buffer[x + 2];
        }
        o_buffer[len] = 0;
        #ifdef USE_PTHREADS
        if(w->onclose != NULL) {
            w->onclose(o_buffer);
        }
        #endif
        WS_close(w);
        printf("Server sent a close request\n");
        return -1;
    }
    int x_offset = 2;
    
    uint16_t long_length = 0;
    if(len >= 126) {
        long_length = (uint8_t)w->rx_buffer[2] << 8 | (uint8_t)w->rx_buffer[3];
        x_offset = 4;
    }
    else {
        long_length = len;
    }

    for(int x = 0; x < long_length; x++) {
        o_buffer[x] = w->rx_buffer[x + x_offset];
    }
    if(opcode == 0x01) { // TEXT
        o_buffer[long_length + x_offset] = 0; 
    }
    return long_length;
}
/*
    Function:   WS_send
    Returns:    Sends character data
    Parameters:
                parameter               description
                struct websocket *w     websocket object
                char *data              input data
                u16 length              message length, use strlen(data)
                WS_FRAME_TYPE           datatype
*/
int WS_send (struct websocket *w, char *data, uint16_t length, WS_FRAME_TYPE datatype) {
    if(w->state != 0x01) {
        printf("Websocket state not ready!\n");
        return -2;
    }

    uint8_t frame_b_0 = 0;
    set_bit(&frame_b_0, 7, 1); // FIN bit to 1
    set_bit(&frame_b_0, 6, 0); // next 3 RSV bits to 0
    set_bit(&frame_b_0, 5, 0);
    set_bit(&frame_b_0, 4, 0);
    set_bit(&frame_b_0, 3, (datatype >> 3 & 1)); // Set 4 bit OP CODE
    set_bit(&frame_b_0, 2, (datatype >> 2 & 1));
    set_bit(&frame_b_0, 1, (datatype >> 1 & 1));
    set_bit(&frame_b_0, 0, (datatype >> 0 & 1));
    uint8_t frame_b_1 = 0;
    uint16_t data_len = length;
    uint16_t frame_b_2 = 0;
    int x_offset = 2; // default 2, > 126B = 4
    if(data_len < 126) {
        frame_b_1 = data_len; // set 7-bit length
    }
    else {
        frame_b_1 = 126; // set 7-bit length to max - 1, continue to next size frame
        frame_b_2 = length;
        x_offset = 4;
    }
    set_bit(&frame_b_1, 7, 0); // MASK bit to 0

    for(int x = 0; x < length; x++) {
        w->tx_buffer[x + x_offset] = data[x];
    }
    w->tx_buffer[0] = frame_b_0;
    w->tx_buffer[1] = frame_b_1;
    if(data_len >= 126) {
        w->tx_buffer[2] = frame_b_2 >> 8;
        w->tx_buffer[3] = frame_b_2 >> 0;
    }
    
    write(w->socket, w->tx_buffer, length + x_offset);
    return 0;
}