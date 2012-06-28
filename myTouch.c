#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <X11/Xlib.h>

void mouseClick(Display *display, Window root, int button);
void mouseMove (Display *display, Window root, int x, int y);
void getInput(Display *display, Window root, char* devName);

int main(int argc, char** argv) {
    printf("Init\n");

    //Open X Display
    Display *display = XOpenDisplay(NULL);
    assert(display != NULL);
    Window root = XDefaultRootWindow(display);
    char* eventName = "/dev/input/event2";

    mouseMove(display, root, 0, 0);
    mouseClick(display, root, Button1);
    getInput(display, root, eventName);

    //Close the display
    XCloseDisplay(display);

    printf("You are awesome :) \n");
    return EXIT_SUCCESS;
}

void mouseClick(Display *display, Window root, int button) {
    XEvent clickEvent;
    int click;

    memset (&clickEvent, 0, sizeof (clickEvent));
    clickEvent.xbutton.button = button;
    clickEvent.xbutton.same_screen = True;
    clickEvent.xbutton.subwindow = root;
    while (clickEvent.xbutton.subwindow)
    {
        clickEvent.xbutton.window = clickEvent.xbutton.subwindow;
        XQueryPointer (display, clickEvent.xbutton.window,
                       &clickEvent.xbutton.root, &clickEvent.xbutton.subwindow,
                       &clickEvent.xbutton.x_root, &clickEvent.xbutton.y_root,
                       &clickEvent.xbutton.x, &clickEvent.xbutton.y,
                       &clickEvent.xbutton.state);
    }

    //Press
    clickEvent.type = ButtonPress;
    click = XSendEvent (display, PointerWindow, True, ButtonPressMask, &clickEvent);
    if (click == 0) {
        printf("Click Failed\n");
    }
    XFlush (display);
    usleep (1);

    // Release
    clickEvent.type = ButtonRelease;
    click = XSendEvent (display, PointerWindow, True, ButtonPressMask, &clickEvent);
    if (click == 0) {
        printf("Click Failed\n");
    }
    XFlush (display);
    usleep (1);
}

void mouseMove(Display *display, Window root, int x, int y) {

    XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
    XFlush(display);
    usleep(1);
}

void getInput(Display *display, Window root, char* devName) {
    int fd, rd, i, root_x, root_y;
    struct input_event ev[64];
    Window window_returned;
    int win_x, win_y;
    unsigned int mask_return;
    int prev_x, prev_y;

    int slots = 0;

    fd = open(devName, O_RDONLY);

    if(fd < 0) {
        printf("Error Opening device. Are you root?\n");
    }

    while(1) {
        rd = read(fd, ev, sizeof(struct input_event) * 64);

        if(rd < (int) sizeof(struct input_event)) {
            printf("Error Reading Input");
        }

        for(i = 0; i < rd / sizeof(struct input_event); i++) {
            if(ev[i].code >= 47 && ev[i].code <= 60) {
                //Get current pointer information
                XQueryPointer(display, root, &window_returned,
                              &window_returned, &root_x, &root_y, &win_x, &win_y,
                              &mask_return);

                int ii;

                //Fill the slots
                if(ev[i].code == ABS_MT_SLOT && (ev[i].value - slots) == 1){
                    slots++;
                    printf("ADD SLOTS: %d\n",slots);
                }

                //X-Axis
                if(ev[i].code == ABS_MT_POSITION_X) {
                    mouseMove(display, root, ev[i].value, root_y);
                    prev_x = ev[i].value;
                }
                //Y-Axis
                if(ev[i].code == ABS_MT_POSITION_Y) {
                    mouseMove(display, root, root_x, ev[i].value);
                    prev_y = ev[i].value;
                }

                if(ev[i].code == ABS_MT_TRACKING_ID && ev[i].value == -1){
                    
                    slots--;
                    printf("REMOVE SLOTS: %d\n",slots);
                    
                    //We can click with one slot ;)
                    /*if(slots == 0){
                        mouseClick(display, root, Button1);
                    }*/
                    mouseClick(display, root, Button1);
                }
            }
        }

    }

    close(fd);

}
