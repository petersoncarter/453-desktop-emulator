#include "render.h"

/* here are our X variables */
Display *dis;
int screen;
Window win;
GC gc;

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();
void create_colormap();
long resolve_pixel_color(Pixel p);
void render_frame_buffer();

int width, height, pixel_width, pixel_height;
Pixel pixel_buffers[2][256];
Pixel *read_buffer, *frame_buffer;

pthread_mutex_t frame_buffer_lock;
bool render_flag;

void init_display(void)
{  
  pthread_mutex_init(&frame_buffer_lock, NULL);
  render_flag = false;
  init_x();

  frame_buffer = pixel_buffers[0];
  read_buffer = pixel_buffers[1];
  
  int i = 0;
  for(; i < PIXELS_WIDTH * PIXELS_HEIGHT; i++) {
    frame_buffer[i].red = 0;
    frame_buffer[i].green = 0;
    frame_buffer[i].blue = 0;
  }
}

void init_x()
{
  /* get the colors black and white (see section for details) */        
  unsigned long black,white;

  dis=XOpenDisplay((char *)0);
  screen=DefaultScreen(dis);
  black=BlackPixel(dis,screen),
    white=WhitePixel(dis, screen);
  win=XCreateSimpleWindow(dis,DefaultRootWindow(dis),0,0,	
			  EMULATOR_WINDOW_WIDTH, EMULATOR_WINDOW_HEIGHT, 0, black, white);
  XSetStandardProperties(dis, win, "GameTable Emulator",
			 "GameTable Emulator", None, NULL, 0, NULL);
  XSelectInput(dis, win, ExposureMask|ButtonPressMask|ButtonReleaseMask|Button1MotionMask);
  XAutoRepeatOn(dis);
  gc=XCreateGC(dis, win, 0,0);
  XSetBackground(dis,gc,white);
  XSetForeground(dis,gc,black);

  XSizeHints hints;
  hints.flags = PMinSize | PMaxSize;
  hints.max_width = EMULATOR_WINDOW_WIDTH;
  hints.min_width = EMULATOR_WINDOW_WIDTH;
  hints.max_height = EMULATOR_WINDOW_HEIGHT;
  hints.min_height = EMULATOR_WINDOW_HEIGHT;
  XSetWMNormalHints(dis, win, &hints);
	
  XClearWindow(dis, win);
  XMapRaised(dis, win);

  XWindowAttributes win_attr;
  XGetWindowAttributes(dis, win, &win_attr);
  width = win_attr.width;
  height = win_attr.height;
  pixel_width = width / 32;
  pixel_height = height / 8;
}

void close_x()
{
  XFreeGC(dis, gc);
  XDestroyWindow(dis,win);
  XCloseDisplay(dis);	
  exit(1);				
}

/*
 *  X Window rendering related functions
 */
void redraw()
{
  XClearWindow(dis, win);
}

long resolve_pixel_color(Pixel p)
{
  return (long) (p.red << 16) + (p.green << 8) + p.blue;
}

void render_frame_buffer()
{
  int i = 0;
  for(; i < PIXELS_WIDTH * PIXELS_HEIGHT; i++) {
    XSetForeground(dis, gc, resolve_pixel_color(read_buffer[i]));
    XFillRectangle(dis, win, gc, (i % PIXELS_WIDTH) * pixel_width,
		   (i / PIXELS_WIDTH) * pixel_height, pixel_width, pixel_height);
  }
  
  XFlush(dis);
}

void set_pixel(int i, int j, Pixel p)
{
  frame_buffer[(j * PIXELS_WIDTH) + i] = p;
}

void render()
{
  Pixel *temp;

  pthread_mutex_lock(&frame_buffer_lock);
  temp = read_buffer;
  read_buffer = frame_buffer;
  frame_buffer = read_buffer;

  render_flag = true;
  pthread_mutex_unlock(&frame_buffer_lock);
}
