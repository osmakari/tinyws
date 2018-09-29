#include "../src/websocket.h"

int main () {
    // create websocket object
    struct websocket *ws = WS("127.0.0.1", 8080, "ws://localhost:8080/", NULL, NULL);
    if(ws == NULL) {
        printf("Failed to connect to the websocket server!\n");
        return 1;
    }
    // Send important data
    char *d = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
    WS_send(ws, d, strlen(d), TEXT);
    // Allocate Buffer
    char buffer[4096] = { 0 };
    int len = 0;
    while (1) {
        // Receive data
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