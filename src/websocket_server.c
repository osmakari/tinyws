#include "websocket_server.h"
#include <poll.h>

struct ws_client *_ws_server_accept (struct websocket_server *ws);

int WS_server_update (struct websocket_server *ws) {
    _ws_server_accept(ws); // Accept incoming connections
    int x;
    for(x = 0; x < MAX_CLIENTS; x++) {
        if(ws->clients[x] == NULL)
            continue;
        int read 
        while()
    }
}

struct ws_client *_ws_server_accept (struct websocket_server *ws) {
    int acc;
    struct sockaddr_in w_client;
    acc = accept(ws->sockfd, (struct sockaddr_in *)&w_client, (socklen_t*)sizeof(struct sockaddr_in));

    if(acc < 0) {
        
        return NULL;
    }

    struct ws_client *cli = (struct ws_client *)malloc(sizeof(struct ws_client));

    cli->addr = w_client;
    cli->auth = 0;
    cli->connection = ws;
    cli->socket = acc;
    int x;
    for(x = 0; x < MAX_CLIENTS; x++) {
        if(ws->clients[x] == NULL) {
            ws->clients[x] = cli;
            cli->_client_index = x;
            break;
        }
    }

    return cli;
}

struct websocket_server *WS_server (uint16_t port) {
    struct websocket_server *ws = (struct websocket_server*)malloc(sizeof(struct websocket_server));

    ws->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    if(ws->sockfd == -1) {
        free(ws);
        printf("Failed to open socket!\n");
        return NULL;
    }
    ws->addr.sin_family = AF_INET;
    ws->addr.sin_addr.s_addr = INADDR_ANY;
    ws->addr.sin_port = port;
    ws->rx = (char*)malloc(2048);
    int x;
    for(x = 0; x < MAX_CLIENTS; x++) {
        ws->clients[x] = NULL;
    }

    if( bind(ws->sockfd, (struct sockaddr *)&ws->addr , sizeof(ws->addr)) < 0)
    {
        print("Bind failed\n");
        return NULL;
    }
    listen(ws->sockfd, 3);
    return ws;
}