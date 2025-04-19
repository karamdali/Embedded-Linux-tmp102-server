 #include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mongoose.h"

#define TEMP_PATH "/sys/class/hwmon/hwmon0/temp1_input"
static const char *s_http_addr = "http://192.168.0.100:8000"; //BBB IP Address from u-boot

static float read_temperature() {
    FILE *f = fopen(TEMP_PATH, "r");
    if (!f) return -99.9f;
    int millideg;
    fscanf(f, "%d", &millideg);
    fclose(f);
    return millideg / 1000.0f;
}


static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        struct mg_str *uri = &hm->uri;
        if (uri->len >= 3 && memcmp(uri->buf, "/ws", 3) == 0) {
            mg_ws_upgrade(c, hm, NULL);
        } else {
        //HTML page
            mg_http_reply(c, 200, "Content-Type: text/html\r\n",
                "<html><script>const ws=new WebSocket('ws://'+location.host+'/ws');"
                "ws.onmessage=e=>document.getElementById('temp').innerText=e.data+' Degree celsius';</script>"
                "<body><h1>TMP102 Temperature</h1><div id='temp'>--.- °C</div></body></html>"
            );
        }
    } else if (ev == MG_EV_WS_OPEN) {
        char msg[32];
        snprintf(msg, sizeof(msg), "%.2f", read_temperature());
        mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
    }
}

//Broadcast temperature to all clients
static void broadcast_temperature(struct mg_mgr *mgr) {
    char msg[32];
    snprintf(msg, sizeof(msg), "%.2f", read_temperature());
    for (struct mg_connection *c = mgr->conns; c; c = c->next) {
        if (c->is_websocket) mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
    }
}

int main() {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, s_http_addr, event_handler, NULL);
    while (1) {
        mg_mgr_poll(&mgr, 1000);
        broadcast_temperature(&mgr);
        sleep(1);
    }
    mg_mgr_free(&mgr);
    return 0;
}
