/* nuklear - v1.17 - public domain */
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include <cairo-gl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_CAIRO_IMPLEMENTATION
#include "../../nuklear.h"
#include "nuklear_cairo.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define BUF_MAX 128

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the include
 * and the corresponding function. */
/*#include "../style.c"*/
/*#include "../calculator.c"*/
/*#include "../overview.c"*/
/*#include "../node_editor.c"*/

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    enum {EASY, HARD};
    int op = EASY;
    int property = 20;
    struct nk_color background = nk_rgba(0x33, 0x33, 0x33, 0xff);
    struct nk_image logo = nk_cairo_png_load("../../example/icon/tools.png");
    char buf [BUF_MAX];
    int buf_len;

    int width = WINDOW_WIDTH;
    int height = WINDOW_HEIGHT;

    Display *dpy = XOpenDisplay(NULL);
    const Window root = DefaultRootWindow(dpy);

    GLint att [] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE,
        24,
        GLX_DOUBLEBUFFER,
        None
    };
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    GLXContext glc;
    Visual *visual = vi->visual;
    const int depth = vi->depth;
    cairo_device_t *dev;
    XSetWindowAttributes swa;
    Window win;
    cairo_surface_t *surf;
    cairo_t *cr;
    struct nk_context _ctx;
    struct nk_context *ctx = &_ctx;
    struct nk_user_font font;

    swa.colormap = XCreateColormap(dpy, root, visual, AllocNone);
    swa.event_mask =
        ExposureMask | StructureNotifyMask |
        KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask;
    win = XCreateWindow(dpy, root, 0, 0, width, height,
        0, depth, InputOutput, visual, CWColormap | CWEventMask, &swa);

    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Cairo OpenGL");

    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    dev = cairo_glx_device_create(dpy, glc);
    surf = cairo_gl_surface_create_for_window(dev, win, width, height);
    cr = cairo_create(surf);

    nk_cairo_init(ctx, &font, cr);
    /*set_style(ctx, THEME_DARK);*/

    while(1)
    {
        {
            XEvent xev;

            nk_input_begin(ctx);
            XPeekEvent(dpy, &xev);
            while(XPending(dpy) > 0)
            {
                XNextEvent(dpy, &xev);
                {
                    if(XFilterEvent(&xev, win))
                        continue;
                    nk_cairo_handle_event(ctx, dpy, &xev);
                }
            }
            nk_input_end(ctx);
        }

        {
            XWindowAttributes gwa;

            XGetWindowAttributes(dpy, win, &gwa);
            if( (gwa.width != width) || (gwa.height != height) )
            {
                width = gwa.width;
                height = gwa.height;
                cairo_gl_surface_set_size(surf, width, height);
            }
        }

        {
            if(nk_begin(ctx, "Demo", nk_rect(10, 10, width-20, height-20),
                    NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                    NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_static(ctx, 30, 160, 1);
                if(nk_button_image_label(ctx, logo, "button", NK_TEXT_ALIGN_RIGHT))
                    fprintf(stdout, "button pressed\n");

                nk_layout_row_dynamic(ctx, 30, 2);
                if(nk_option_label(ctx, "easy", op == EASY))
                    op = EASY;
                if(nk_option_label(ctx, "hard", op == HARD))
                    op = HARD;

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, buf, &buf_len, BUF_MAX, 0);
                
                {
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label(ctx, "background:", NK_TEXT_LEFT);
                    nk_layout_row_dynamic(ctx, 25, 1);
                    if(nk_combo_begin_color(ctx, background, nk_vec2(nk_widget_width(ctx),400)))
                    {
                        nk_layout_row_dynamic(ctx, 120, 1);
                        background = nk_color_picker(ctx, background, NK_RGBA);
                        nk_layout_row_dynamic(ctx, 25, 1);
                        background.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, background.r, 255, 1,1);
                        background.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, background.g, 255, 1,1);
                        background.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, background.b, 255, 1,1);
                        background.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, background.a, 255, 1,1);
                        nk_combo_end(ctx);
                    }
                }
            }
            nk_end(ctx);
            if(nk_window_is_closed(ctx, "Demo"))
                break;

            /* -------------- EXAMPLES ---------------- */
            /*calculator(ctx);*/
            /*overview(ctx);*/
            /*node_editor(ctx);*/
            /* ----------------------------------------- */
        }

        nk_cairo_clear(cr, width, height, background);
        nk_cairo_render(ctx, cr);
        cairo_gl_surface_swapbuffers(surf);
    }

    nk_cairo_shutdown(ctx);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    cairo_device_destroy(dev);
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XFree(vi);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    nk_cairo_png_unload(logo);

    return 0;
}
