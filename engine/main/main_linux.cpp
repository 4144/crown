/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "config.h"

#if CROWN_PLATFORM_LINUX

#include "device.h"
#include "memory.h"
#include "os_event_queue.h"
#include "os_window_linux.h"
#include "thread.h"
#include "main.h"
#include "command_line.h"
#include "disk_filesystem.h"
#include "crown.h"
#include "bundle_compiler.h"
#include "console_server.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/Xrandr.h>
#include <bgfxplatform.h>
#include <bgfx.h>

namespace crown
{

// void display_modes(Array<DisplayMode>& modes)
// {
// 	int num_rrsizes = 0;
// 	XRRScreenSize* rrsizes = XRRConfigSizes(m_screen_config, &num_rrsizes);

// 	for (int i = 0; i < num_rrsizes; i++)
// 	{
// 		DisplayMode dm;
// 		dm.id = (uint32_t) i;
// 		dm.width = rrsizes[i].width;
// 		dm.height = rrsizes[i].height;
// 		array::push_back(modes, dm);
// 	}
// }

// void set_display_mode(uint32_t id)
// {
// 	// Check if id is valid
// 	int num_rrsizes = 0;
// 	XRRScreenSize* rrsizes = XRRConfigSizes(m_screen_config, &num_rrsizes);
// 	(void) rrsizes;

// 	if ((int) id >= num_rrsizes)
// 		return;

// 	XRRSetScreenConfig(m_x11_display,
// 		m_screen_config,
// 		RootWindow(m_x11_display, DefaultScreen(m_x11_display)),
// 		(int) id,
// 		RR_Rotate_0,
// 		CurrentTime);
// }

// void set_fullscreen(bool full)
// {
// 	XEvent e;
// 	e.xclient.type = ClientMessage;
// 	e.xclient.window = m_x11_window;
// 	e.xclient.message_type = XInternAtom(m_x11_display, "_NET_WM_STATE", False );
// 	e.xclient.format = 32;
// 	e.xclient.data.l[0] = full ? 1 : 0;
// 	e.xclient.data.l[1] = XInternAtom(m_x11_display, "_NET_WM_STATE_FULLSCREEN", False);
// 	XSendEvent(m_x11_display, DefaultRootWindow(m_x11_display), False, SubstructureNotifyMask, &e);
// }

static KeyboardButton::Enum x11_translate_key(KeySym x11_key)
{
	switch (x11_key)
	{
		case XK_BackSpace: return KeyboardButton::BACKSPACE;
		case XK_Tab:       return KeyboardButton::TAB;
		case XK_space:     return KeyboardButton::SPACE;
		case XK_Escape:    return KeyboardButton::ESCAPE;
		case XK_Return:    return KeyboardButton::ENTER;
		case XK_F1:        return KeyboardButton::F1;
		case XK_F2:        return KeyboardButton::F2;
		case XK_F3:        return KeyboardButton::F3;
		case XK_F4:        return KeyboardButton::F4;
		case XK_F5:        return KeyboardButton::F5;
		case XK_F6:        return KeyboardButton::F6;
		case XK_F7:        return KeyboardButton::F7;
		case XK_F8:        return KeyboardButton::F8;
		case XK_F9:        return KeyboardButton::F9;
		case XK_F10:       return KeyboardButton::F10;
		case XK_F11:       return KeyboardButton::F11;
		case XK_F12:       return KeyboardButton::F12;
		case XK_Home:      return KeyboardButton::HOME;
		case XK_Left:      return KeyboardButton::LEFT;
		case XK_Up:        return KeyboardButton::UP;
		case XK_Right:     return KeyboardButton::RIGHT;
		case XK_Down:      return KeyboardButton::DOWN;
		case XK_Page_Up:   return KeyboardButton::PAGE_UP;
		case XK_Page_Down: return KeyboardButton::PAGE_DOWN;
		case XK_Shift_L:   return KeyboardButton::LSHIFT;
		case XK_Shift_R:   return KeyboardButton::RSHIFT;
		case XK_Control_L: return KeyboardButton::LCONTROL;
		case XK_Control_R: return KeyboardButton::RCONTROL;
		case XK_Caps_Lock: return KeyboardButton::CAPS_LOCK;
		case XK_Alt_L:     return KeyboardButton::LALT;
		case XK_Alt_R:     return KeyboardButton::RALT;
		case XK_Super_L:   return KeyboardButton::LSUPER;
		case XK_Super_R:   return KeyboardButton::RSUPER;
		case XK_KP_0:      return KeyboardButton::KP_0;
		case XK_KP_1:      return KeyboardButton::KP_1;
		case XK_KP_2:      return KeyboardButton::KP_2;
		case XK_KP_3:      return KeyboardButton::KP_3;
		case XK_KP_4:      return KeyboardButton::KP_4;
		case XK_KP_5:      return KeyboardButton::KP_5;
		case XK_KP_6:      return KeyboardButton::KP_6;
		case XK_KP_7:      return KeyboardButton::KP_7;
		case XK_KP_8:      return KeyboardButton::KP_8;
		case XK_KP_9:      return KeyboardButton::KP_9;
		case '0':          return KeyboardButton::NUM_0;
		case '1':          return KeyboardButton::NUM_1;
		case '2':          return KeyboardButton::NUM_2;
		case '3':          return KeyboardButton::NUM_3;
		case '4':          return KeyboardButton::NUM_4;
		case '5':          return KeyboardButton::NUM_5;
		case '6':          return KeyboardButton::NUM_6;
		case '7':          return KeyboardButton::NUM_7;
		case '8':          return KeyboardButton::NUM_8;
		case '9':          return KeyboardButton::NUM_9;
		case 'a':          return KeyboardButton::A;
		case 'b':          return KeyboardButton::B;
		case 'c':          return KeyboardButton::C;
		case 'd':          return KeyboardButton::D;
		case 'e':          return KeyboardButton::E;
		case 'f':          return KeyboardButton::F;
		case 'g':          return KeyboardButton::G;
		case 'h':          return KeyboardButton::H;
		case 'i':          return KeyboardButton::I;
		case 'j':          return KeyboardButton::J;
		case 'k':          return KeyboardButton::K;
		case 'l':          return KeyboardButton::L;
		case 'm':          return KeyboardButton::M;
		case 'n':          return KeyboardButton::N;
		case 'o':          return KeyboardButton::O;
		case 'p':          return KeyboardButton::P;
		case 'q':          return KeyboardButton::Q;
		case 'r':          return KeyboardButton::R;
		case 's':          return KeyboardButton::S;
		case 't':          return KeyboardButton::T;
		case 'u':          return KeyboardButton::U;
		case 'v':          return KeyboardButton::V;
		case 'w':          return KeyboardButton::W;
		case 'x':          return KeyboardButton::X;
		case 'y':          return KeyboardButton::Y;
		case 'z':          return KeyboardButton::Z;
		default:           return KeyboardButton::NONE;
	}
}

static int x11_error_handler(Display* dpy, XErrorEvent* ev)
{
	return 0;
}

static bool s_exit = false;

struct MainThreadArgs
{
	Filesystem* fs;
	ConfigSettings* cs;
};

int32_t func(void* data)
{
	MainThreadArgs* args = (MainThreadArgs*) data;
	crown::init(*args->fs, *args->cs);
	crown::update();
	crown::shutdown();
	s_exit = true;
	return EXIT_SUCCESS;
}

struct LinuxDevice
{
	LinuxDevice()
		: _x11_display(NULL)
		, _x11_window(None)
		, _x11_parent_window(None)
		, _x11_hidden_cursor(None)
		, _screen_config(NULL)
	{
	}

	int32_t run(Filesystem* fs, ConfigSettings* cs, CommandLineSettings* cls)
	{
		// Create main window
		XInitThreads();
		XSetErrorHandler(x11_error_handler);
		_x11_display = XOpenDisplay(NULL);

		CE_ASSERT(_x11_display != NULL, "Unable to open X11 display");

		int screen = DefaultScreen(_x11_display);
		int depth = DefaultDepth(_x11_display, screen);
		Visual* visual = DefaultVisual(_x11_display, screen);

		_x11_parent_window = (cls->parent_window == 0) ? RootWindow(_x11_display, screen) :
			(Window) cls->parent_window;

		// Create main window
		XSetWindowAttributes win_attribs;
		win_attribs.background_pixmap = 0;
		win_attribs.border_pixel = 0;
		win_attribs.event_mask = FocusChangeMask
			| StructureNotifyMask 
			| KeyPressMask
			| KeyReleaseMask 
			| ButtonPressMask 
			| ButtonReleaseMask
			| PointerMotionMask;

		_x11_window = XCreateWindow(_x11_display,
			_x11_parent_window,
			0, 0,
			cs->window_width,
			cs->window_height,
			0,
			depth,
			InputOutput,
			visual,
			CWBorderPixel | CWEventMask,
			&win_attribs
		);
		CE_ASSERT(_x11_window != None, "Unable to create X window");

		// Do we have detectable autorepeat?
		Bool detectable;
		_x11_detectable_autorepeat = (bool) XkbSetDetectableAutoRepeat(_x11_display, true, &detectable);

		// Build hidden cursor
		Pixmap bm_no;
		XColor black, dummy;
		Colormap colormap;
		static char no_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		colormap = XDefaultColormap(_x11_display, screen);
		XAllocNamedColor(_x11_display, colormap, "black", &black, &dummy);
		bm_no = XCreateBitmapFromData(_x11_display, _x11_window, no_data, 8, 8);
		_x11_hidden_cursor = XCreatePixmapCursor(_x11_display, bm_no, bm_no, &black, &black, 0, 0);

		_wm_delete_message = XInternAtom(_x11_display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(_x11_display, _x11_window, &_wm_delete_message, 1);

		oswindow_set_window(_x11_display, _x11_window);
		bgfx::x11SetDisplayWindow(_x11_display, _x11_window);
		XMapRaised(_x11_display, _x11_window);

		// Get screen configuration
		_screen_config = XRRGetScreenInfo(_x11_display, RootWindow(_x11_display, screen));

		Rotation rr_old_rot;
		const SizeID rr_old_sizeid = XRRConfigCurrentConfiguration(_screen_config, &rr_old_rot);

		// Start main thread
		MainThreadArgs mta;
		mta.fs = fs;
		mta.cs = cs;

		Thread main_thread;
		main_thread.start(func, &mta);

		while (!s_exit)
		{
			pump_events();
		}

		main_thread.stop();

		// Restore previous screen configuration if changed
		Rotation rr_cur_rot;
		const SizeID rr_cur_sizeid = XRRConfigCurrentConfiguration(_screen_config, &rr_cur_rot);

		if (rr_cur_rot != rr_old_rot || rr_cur_sizeid != rr_old_sizeid)
		{
			XRRSetScreenConfig(_x11_display,
				_screen_config,
				RootWindow(_x11_display, screen),
				rr_old_sizeid,
				rr_old_rot,
				CurrentTime);
		}
		XRRFreeScreenConfigInfo(_screen_config);

		XDestroyWindow(_x11_display, _x11_window);
		XCloseDisplay(_x11_display);
		return EXIT_SUCCESS;
	}

	void pump_events()
	{
		while (XPending(_x11_display))
		{
			XEvent event;
			XNextEvent(_x11_display, &event);

			switch (event.type)
			{
				case ClientMessage:
				{
					if ((Atom)event.xclient.data.l[0] == _wm_delete_message)
					{
						_queue.push_exit_event(0);
					}
					break;
				}
				case ConfigureNotify:
				{
					_queue.push_metrics_event(event.xconfigure.x, event.xconfigure.y,
						event.xconfigure.width, event.xconfigure.height);
					break;
				}
				case ButtonPress:
				case ButtonRelease:
				{
					MouseButton::Enum mb;
					switch (event.xbutton.button)
					{
						case Button1: mb = MouseButton::LEFT; break;
						case Button2: mb = MouseButton::MIDDLE; break;
						case Button3: mb = MouseButton::RIGHT; break;
						default: mb = MouseButton::NONE; break;
					}

					if (mb != MouseButton::NONE)
					{
						_queue.push_mouse_event(event.xbutton.x, event.xbutton.y, mb, event.type == ButtonPress);
					}

					break;
				}
				case MotionNotify:
				{
					_queue.push_mouse_event(event.xmotion.x, event.xmotion.y);
					break;
				}
				case KeyPress:
				case KeyRelease:
				{
					KeySym keysym = XLookupKeysym(&event.xkey, 0);
					KeyboardButton::Enum kb = x11_translate_key(keysym);

					// Check if any modifier key is pressed or released
					int32_t modifier_mask = 0;

					if (kb == KeyboardButton::LSHIFT || kb == KeyboardButton::RSHIFT)
					{
						(event.type == KeyPress) ? modifier_mask |= ModifierButton::SHIFT : modifier_mask &= ~ModifierButton::SHIFT;
					}
					else if (kb == KeyboardButton::LCONTROL || kb == KeyboardButton::RCONTROL)
					{
						(event.type == KeyPress) ? modifier_mask |= ModifierButton::CTRL : modifier_mask &= ~ModifierButton::CTRL;
					}
					else if (kb == KeyboardButton::LALT || kb == KeyboardButton::RALT)
					{
						(event.type == KeyPress) ? modifier_mask |= ModifierButton::ALT : modifier_mask &= ~ModifierButton::ALT;
					}

					_queue.push_keyboard_event(modifier_mask, kb, event.type == KeyPress);
					break;
				}
				case KeymapNotify:
				{
					XRefreshKeyboardMapping(&event.xmapping);
					break;
				}
				default:
				{
					break;
				}
			}
		}
	}

public:

	Display* _x11_display;
	Window _x11_window;
	Window _x11_parent_window;
	Cursor _x11_hidden_cursor;
	Atom _wm_delete_message;
	XRRScreenConfiguration* _screen_config;
	bool _x11_detectable_autorepeat;
	OsEventQueue _queue;
};

static LinuxDevice s_ldvc;

bool next_event(OsEvent& ev)
{
	return s_ldvc._queue.pop_event(ev);
}

} // namespace crown

int main(int argc, char** argv)
{
	using namespace crown;

	CommandLineSettings cls = parse_command_line(argc, argv);

	memory_globals::init();
	DiskFilesystem src_fs(cls.source_dir);
	ConfigSettings cs = parse_config_file(src_fs);

	console_server_globals::init();
	console_server_globals::console().init(cs.console_port, cls.wait_console);

	bundle_compiler_globals::init(cls.source_dir, cls.bundle_dir);

	bool do_continue = true;
	int exitcode = EXIT_SUCCESS;

	do_continue = bundle_compiler::main(cls);

	if (do_continue)
	{
		DiskFilesystem dst_fs(cls.bundle_dir);
		exitcode = crown::s_ldvc.run(&dst_fs, &cs, &cls);
	}

	bundle_compiler_globals::shutdown();
	console_server_globals::shutdown();
	memory_globals::shutdown();
	return exitcode;
}

#endif // CROWN_PLATFORM_LINUX
