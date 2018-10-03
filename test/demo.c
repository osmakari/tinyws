#include "../src/websocket.h"

#ifdef USE_PTHREADS
struct websocket *ws;
void onmessage (char *c, uint16_t length) {
    printf("Length: %i, Message: %s\n", length, c);
    char *msg = "abcdefg";
    //WS_send(ws, msg, strlen(msg), TEXT);
}

int main () {
    ws = WS("127.0.0.1", 8971, NULL, NULL, NULL);
    if(ws == NULL) {
        printf("Failed to connect to the websocket server!\n");
        return 1;
    }
    ws->onmessage = onmessage;
    while(1) {
        //sleep(5);
        //char *c = "Data: abcassdgkawgkqwghwp89ghqw498ghw30498ghw094jgw948ghw9g8hw94gh8we98hwerhg8wergwgj893q489gh3w49ghwe9r8ghwe89rg";
        //WS_send(ws, c, strlen(c), TEXT);
    }

    return 0;
}

#else
int main () {
    // create websocket object
    struct websocket *ws = WS("127.0.0.1", 8971, NULL, NULL, NULL);
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
#endif