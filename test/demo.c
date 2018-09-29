#include "../src/websocket.h"

int main () {
    struct websocket *ws = WS("127.0.0.1", 8080, "ws://localhost:8080/", NULL, NULL);
    if(ws == NULL) {
        printf("Failed to connect to the websocket server!\n");
        return 1;
    }
    char *d = "This is a demo!!!\n";
    WS_send(ws, d, strlen(d), TEXT);
    char buffer[128] = { 0 };
    int len = 0;
    while (1) {
        if((len = WS_receive(ws, buffer)) > 0) {
            printf("Data from server: %s\n", buffer);
        }
        else {
            printf("Connection was unexpectly closed!\n");
            return 1;
        }
    }
    return 0;
}