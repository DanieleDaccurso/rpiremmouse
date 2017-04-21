#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <xdo.h>

struct xdo_t *xd;

struct coords {
    int x;
    int y;
};

struct sockaddr_in *init_server() {
    struct sockaddr_in *saddr;
    saddr = malloc(sizeof(saddr));
    saddr->sin_family = AF_INET;
    saddr->sin_addr.s_addr = htons(INADDR_ANY);
    saddr->sin_port = htons(6667);
    return saddr;
}

struct coords *read_xy_coords(char *msg) {
    /* create a new coordinate point */
    struct coords *c;
    c = malloc(sizeof(c));

    /* read substrings */
    char *x;
    char *y;
    char *needle = ";";
    strtok(msg, needle);
    x = strtok(NULL, needle);
    y = strtok(NULL, needle);

    /* parse coordinate point integers */
    c->x = atoi(x);
    c->y = atoi(y);
    return c;
}

void handle_command(char *msg) {
    if (strncmp(msg, "MVI", 3) == 0) {
        /* MOVE INCREMENTAL */
        struct coords *point = read_xy_coords(msg);
        xdo_move_mouse_relative(xd, point->x, point->y);
        free(point);
    } else if (strncmp(msg, "MVA", 3) == 0) {
        /* MOVE ABSOLUTE */
        struct coords *point = read_xy_coords(msg);
        xdo_move_mouse(xd, point->x, point->y, NULL);
        free(point);
    } else if (strncmp(msg, "LCK", 3) == 0) {
        /* LEFT CLICK */
        xdo_click_window(xd, CURRENTWINDOW, 1);
    } else if (strncmp(msg, "LCD", 3) == 0) {
        /* LEFT CLICK DOUBLE */
        xdo_click_window_multiple(xd, CURRENTWINDOW, 3, 2, 1000);
    } else if (strncmp(msg, "RCK", 3) == 0) {
        /* RIGHT CLICK */
        xdo_click_window(xd, CURRENTWINDOW, 1);
    } else if (strncmp(msg, "RCD", 3) == 0) {
        /* RIGHT CLICK DOUBLE */
        xdo_click_window_multiple(xd, CURRENTWINDOW, 3, 2, 1000);
    } else if (strncmp(msg, "MWU", 3) == 0) {
        /* MOUSE WHEEL UP */
        xdo_click_window(xd, CURRENTWINDOW, 4);
    } else if (strncmp(msg, "MWD", 3) == 0) {
        /* MOUSE WHEEL DOWN */
        xdo_click_window(xd, CURRENTWINDOW, 5);
    }else if (strncmp(msg, "MCH", 3) == 0) {
        /* MOUSE CLICK LEFT HOLD */
        xdo_mouse_down(xd, CURRENTWINDOW, 1);
    } else if (strncmp(msg, "MCR", 3) == 0) {
        /* MOUSE CLICK LEFT RELEASE */
        xdo_mouse_up(xd, CURRENTWINDOW, 1);
    }
}

int main() {
    /* initialize global xdo_t instance to save time and memory */
    xd = xdo_new(NULL);

    char buf[100];
    int listen_fd, comm_fd;
    struct sockaddr_in *servaddr = init_server();

    /*
     * Bind socket and start listening
     */
    listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(listen_fd, servaddr, sizeof(struct sockaddr_in));
    listen(listen_fd, 1);

    for (;;) {
        /*
         * Connections are synced. Only one client may control the mouse
         * pointer at one time.
         */
        comm_fd = accept(listen_fd, (struct sockaddr *) NULL, NULL);
        for (;;) {
            bzero(buf, 100);
            /*
             * read returns 0 on EOF and -1 on error. In those cases, the connection
             * has to be abandoned and the next waiting connection may be processed.
             */
            if (read(comm_fd, buf, 100) < 1) {
                break;
            }

            /*
             * if the received message is not empty, proceed with
             * handling the client input
             */
            if (buf[0] != '\0') {
                handle_command(buf);
            }
        }
    }
}
