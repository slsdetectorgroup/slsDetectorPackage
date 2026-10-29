// Minimal XTest driver: xclick <display> click <x> <y> | key <keysym>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int XTestFakeMotionEvent(Display *, int, int, int, unsigned long);
extern int XTestFakeButtonEvent(Display *, unsigned int, int, unsigned long);
extern int XTestFakeKeyEvent(Display *, unsigned int, int, unsigned long);
int main(int argc, char **argv) {
    Display *d = XOpenDisplay(argc > 1 ? argv[1] : NULL);
    if (!d) { fprintf(stderr, "no display\n"); return 1; }
    if (argc > 3 && strcmp(argv[2], "click") == 0) {
        int x = atoi(argv[3]), y = atoi(argv[4]);
        XTestFakeMotionEvent(d, -1, x, y, 0);
        XTestFakeButtonEvent(d, 1, True, 0);
        XTestFakeButtonEvent(d, 1, False, 0);
    } else if (argc > 3 && strcmp(argv[2], "press") == 0) {
        XTestFakeMotionEvent(d, -1, atoi(argv[3]), atoi(argv[4]), 0);
        XTestFakeButtonEvent(d, 1, True, 0);
    } else if (argc > 3 && strcmp(argv[2], "move") == 0) {
        XTestFakeMotionEvent(d, -1, atoi(argv[3]), atoi(argv[4]), 0);
    } else if (argc > 2 && strcmp(argv[2], "release") == 0) {
        XTestFakeButtonEvent(d, 1, False, 0);
    } else if (argc > 2 && strcmp(argv[2], "raise") == 0) {
        const char *want = argc > 3 ? argv[3] : "";
        Window root = DefaultRootWindow(d), r2, par, *kids = NULL;
        unsigned n = 0;
        if (XQueryTree(d, root, &r2, &par, &kids, &n)) {
            for (unsigned i = 0; i < n; i++) {
                char *nm = NULL;
                if (XFetchName(d, kids[i], &nm) && nm) {
                    if (strstr(nm, want)) {
                        XRaiseWindow(d, kids[i]);
                        XSetInputFocus(d, kids[i], RevertToParent, CurrentTime);
                        printf("raised: %s\n", nm);
                    }
                    XFree(nm);
                }
            }
            if (kids) XFree(kids);
        }
    } else if (argc > 4 && strcmp(argv[2], "movewin") == 0) {
        const char *want = argv[3];
        Window root = DefaultRootWindow(d), r2, par, *kids = NULL;
        unsigned n = 0;
        if (XQueryTree(d, root, &r2, &par, &kids, &n)) {
            for (unsigned i = 0; i < n; i++) {
                char *nm = NULL;
                if (XFetchName(d, kids[i], &nm) && nm) {
                    if (strstr(nm, want)) {
                        XMoveWindow(d, kids[i], atoi(argv[4]), atoi(argv[5]));
                        printf("moved: %s\n", nm);
                    }
                    XFree(nm);
                }
            }
            if (kids) XFree(kids);
        }
    } else if (argc > 2 && strcmp(argv[2], "key") == 0) {
        KeySym ks = XStringToKeysym(argc > 3 ? argv[3] : "Return");
        KeyCode kc = XKeysymToKeycode(d, ks);
        XTestFakeKeyEvent(d, kc, True, 0);
        XTestFakeKeyEvent(d, kc, False, 0);
    }
    XFlush(d);
    XCloseDisplay(d);
    return 0;
}
