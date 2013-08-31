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
OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdlib>

#include "Config.h"
#include "Device.h"
#include "Accelerometer.h"
#include "Args.h"
#include "ConsoleServer.h"
#include "DebugRenderer.h"
#include "DiskFile.h"
#include "DiskFilesystem.h"
#include "InputManager.h"
#include "JSONParser.h"
#include "Keyboard.h"
#include "Log.h"
#include "LuaEnvironment.h"
#include "Memory.h"
#include "Mouse.h"
#include "OS.h"
#include "OsWindow.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "StringSetting.h"
#include "StringUtils.h"
#include "TextReader.h"
#include "Touch.h"
#include "Types.h"
#include "Bundle.h"

#if defined(LINUX) || defined(WINDOWS)
	#include "BundleCompiler.h"
#endif

#if defined(ANDROID)
	#include "ApkFilesystem.h"
#endif

namespace crown
{

//-----------------------------------------------------------------------------
Device::Device() : 
	m_allocator(m_subsystems_heap, MAX_SUBSYSTEMS_HEAP),

	m_preferred_window_width(1000),
	m_preferred_window_height(625),
	m_preferred_window_fullscreen(0),
	m_parent_window_handle(0),
	m_compile(0),
	m_continue(0),

	m_quit_after_init(0),

	m_is_init(false),
	m_is_running(false),

	m_frame_count(0),

	m_last_time(0),
	m_current_time(0),
	m_last_delta_time(0.0f),

	m_filesystem(NULL),
	m_input_manager(NULL),
	m_lua_environment(NULL),
	m_renderer(NULL),
	m_debug_renderer(NULL),

	m_resource_manager(NULL),
	m_resource_bundle(NULL),

	m_console_server(NULL)
{
	// Bundle dir is current dir by default.
	string::strncpy(m_bundle_dir, os::get_cwd(), MAX_PATH_LENGTH);
	string::strncpy(m_source_dir, "", MAX_PATH_LENGTH);
}

//-----------------------------------------------------------------------------
Device::~Device()
{
}

//-----------------------------------------------------------------------------
bool Device::init(int argc, char** argv)
{
	CE_ASSERT(!is_init(), "Engine already initialized");

	parse_command_line(argc, argv);
	check_preferred_settings();

	// Resource compilation only in debug or development mode and only on linux or windows builds
	#if (defined(LINUX) || defined(WINDOWS)) && (defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT))
		if (m_compile == 1)
		{
			m_bundle_compiler = CE_NEW(m_allocator, BundleCompiler);
			if (!m_bundle_compiler->compile(m_bundle_dir, m_source_dir))
			{
				CE_DELETE(m_allocator, m_bundle_compiler);
				Log::e("Exiting.");
				exit(EXIT_FAILURE);
			}

			if (!m_continue)
			{
				CE_DELETE(m_allocator, m_bundle_compiler);
				exit(EXIT_SUCCESS);
			}
		}
	#endif

	// Initialize
	Log::i("Initializing Crown Engine %d.%d.%d...", CROWN_VERSION_MAJOR, CROWN_VERSION_MINOR, CROWN_VERSION_MICRO);

	// Default bundle filesystem
	#if defined (LINUX) || defined(WINDOWS)
		m_filesystem = CE_NEW(m_allocator, DiskFilesystem)(m_bundle_dir);
	#elif defined(ANDROID)
		m_filesystem = CE_NEW(m_allocator, ApkFilesystem)();
	#endif
	Log::d("Filesystem created.");

	m_resource_bundle = Bundle::create(m_allocator, *m_filesystem);

	// // Read resource seed
	// DiskFile* seed_file = (DiskFile*)filesystem()->open(g_default_mountpoint.value(), "seed.ini", FOM_READ);
	// TextReader reader(*seed_file);

	// char tmp_buf[32];
	// reader.read_string(tmp_buf, 32);

	// filesystem()->close(seed_file);

	// uint32_t seed = string::parse_uint(tmp_buf);

	// Create resource manager
	m_resource_manager = CE_NEW(m_allocator, ResourceManager)(*m_resource_bundle, 0);
	Log::d("Resource manager created.");
	Log::d("Resource seed: %d", m_resource_manager->seed());

	// Create input manager
	m_input_manager = CE_NEW(m_allocator, InputManager)();
	Log::d("Input manager created.");

	m_window = CE_NEW(m_allocator, OsWindow)(m_preferred_window_width, m_preferred_window_height, m_parent_window_handle);

	CE_ASSERT(m_window != NULL, "Unable to create the window");

	m_window->set_title("Crown Game Engine");
	m_window->show();
	Log::d("Window created.");

	m_renderer = Renderer::create(m_allocator);
	m_renderer->init();
	Log::d("Renderer created.");

	// Create debug renderer
	m_debug_renderer = CE_NEW(m_allocator, DebugRenderer)(*m_renderer);
	Log::d("Debug renderer created.");

	m_lua_environment = CE_NEW(m_allocator, LuaEnvironment)();
	m_lua_environment->init();
	Log::d("Lua environment created.");

	Log::i("Crown Engine initialized.");
	Log::i("Initializing Game...");

	m_is_init = true;
	start();

	ResourceId luagame_id = m_resource_manager->load("lua", "game");
	m_resource_manager->flush();
	m_lua_environment->load((LuaResource*) m_resource_manager->data(luagame_id));
	m_lua_environment->call_global("init", 0);

	if (m_quit_after_init == 1)
	{
		stop();
		shutdown();
	}

	return true;
}

//-----------------------------------------------------------------------------
void Device::shutdown()
{
	if (is_init() == false)
	{
		Log::e("Crown Engine is not initialized.");	
		return;
	}

	// Shutdowns the game
	m_lua_environment->call_global("shutdown", 0);

	Log::i("Releasing ConsoleServer...");
	if (m_console_server)
	{
		//m_console_server->shutdown();

		CE_DELETE(m_allocator, m_console_server);
	}

	Log::i("Releasing LuaEnvironment...");
	if (m_lua_environment)
	{
		m_lua_environment->shutdown();
		
		CE_DELETE(m_allocator, m_lua_environment);
	}

	Log::i("Releasing InputManager...");
	if (m_input_manager)
	{
		CE_DELETE(m_allocator, m_input_manager);
	}

	Log::i("Releasing DebugRenderer...");
	if (m_debug_renderer)
	{
		CE_DELETE(m_allocator, m_debug_renderer);
	}

	Log::i("Releasing Renderer...");
	if (m_renderer)
	{
		m_renderer->shutdown();

		Renderer::destroy(m_allocator, m_renderer);
	}

	Log::i("Releasing Window...");
	if (m_window)
	{
		CE_DELETE(m_allocator, m_window);
	}

	Log::i("Releasing ResourceManager...");
	if (m_resource_manager)
	{
		CE_DELETE(m_allocator, m_resource_manager);
	}

	if (m_resource_bundle)
	{
		Bundle::destroy(m_allocator, m_resource_bundle);
	}

	Log::i("Releasing Filesystem...");
	if (m_filesystem)
	{
		CE_DELETE(m_allocator, m_filesystem);
	}

	m_allocator.clear();

	m_is_init = false;
}

//-----------------------------------------------------------------------------
bool Device::is_init() const
{
	return m_is_init;
}

//-----------------------------------------------------------------------------
Filesystem* Device::filesystem()
{
	return m_filesystem;
}

//-----------------------------------------------------------------------------
ResourceManager* Device::resource_manager()
{
	return m_resource_manager;
}

//-----------------------------------------------------------------------------
InputManager* Device::input_manager()
{
	return m_input_manager;
}

//-----------------------------------------------------------------------------
LuaEnvironment* Device::lua_environment()
{
	return m_lua_environment;
}

//-----------------------------------------------------------------------------
OsWindow* Device::window()
{
	return m_window;
}

//-----------------------------------------------------------------------------
Renderer* Device::renderer()
{
	return m_renderer;
}

//-----------------------------------------------------------------------------
DebugRenderer* Device::debug_renderer()
{
	return m_debug_renderer;
}

//-----------------------------------------------------------------------------
Keyboard* Device::keyboard()
{
	return m_input_manager->keyboard();
}

//-----------------------------------------------------------------------------
Mouse* Device::mouse()
{
	return m_input_manager->mouse();
}

//-----------------------------------------------------------------------------
Touch* Device::touch()
{
	return m_input_manager->touch();
}

//-----------------------------------------------------------------------------
Accelerometer* Device::accelerometer()
{
	return m_input_manager->accelerometer();
}

ConsoleServer* Device::console_server()
{
	return m_console_server;
}
//-----------------------------------------------------------------------------
void Device::start()
{
	CE_ASSERT(m_is_init, "Cannot start uninitialized engine.");

	m_is_running = true;
	m_last_time = os::milliseconds();
}

//-----------------------------------------------------------------------------
void Device::stop()
{
	CE_ASSERT(m_is_init, "Cannot stop uninitialized engine.");

	m_is_running = false;
}

//-----------------------------------------------------------------------------
bool Device::is_running() const
{
	return m_is_running;
}

//-----------------------------------------------------------------------------
uint64_t Device::frame_count() const
{
	return m_frame_count;
}

//-----------------------------------------------------------------------------
float Device::last_delta_time() const
{
	return m_last_delta_time;
}

//-----------------------------------------------------------------------------
void Device::frame()
{
	m_current_time = os::microseconds();
	m_last_delta_time = (m_current_time - m_last_time) / 1000000.0f;
	m_last_time = m_current_time;

	m_resource_manager->poll_resource_loader();

	m_window->frame();
	m_input_manager->frame(frame_count());
	m_lua_environment->call_global("frame", 1, ARGUMENT_FLOAT, last_delta_time());

	// m_console_server->execute();

	m_debug_renderer->draw_all();
	m_renderer->frame();

	m_frame_count++;
}

//-----------------------------------------------------------------------------
void Device::compile(const char* , const char* , const char* )
{
}

//-----------------------------------------------------------------------------
void Device::reload(ResourceId name)
{
	(void)name;
}

//-----------------------------------------------------------------------------
void Device::parse_command_line(int argc, char** argv)
{
	static ArgsOption options[] = 
	{
		{ "help",             AOA_NO_ARGUMENT,       NULL,        'i' },
		{ "source-dir",       AOA_REQUIRED_ARGUMENT, NULL,        's' },
		{ "bundle-dir",       AOA_REQUIRED_ARGUMENT, NULL,        'b' },
		{ "compile",          AOA_NO_ARGUMENT,       &m_compile,   1 },
		{ "continue",         AOA_NO_ARGUMENT,       &m_continue,  1 },
		{ "width",            AOA_REQUIRED_ARGUMENT, NULL,        'w' },
		{ "height",           AOA_REQUIRED_ARGUMENT, NULL,        'h' },
		{ "fullscreen",       AOA_NO_ARGUMENT,       &m_preferred_window_fullscreen, 1 },
		{ "parent-window",    AOA_REQUIRED_ARGUMENT, NULL,        'p' },
		{ "quit-after-init",  AOA_NO_ARGUMENT,       &m_quit_after_init, 1 },
		{ NULL, 0, NULL, 0 }
	};

	Args args(argc, argv, "", options);

	int32_t opt;

	while ((opt = args.getopt()) != -1)
	{
		switch (opt)
		{
			case 0:
			{
				break;
			}
			// Source directory
			case 's':
			{
				string::strncpy(m_source_dir, args.optarg(), MAX_PATH_LENGTH);
				break;
			}
			// Bundle directory
			case 'b':
			{
				string::strncpy(m_bundle_dir, args.optarg(), MAX_PATH_LENGTH);
				break;
			}
			// Window width
			case 'w':
			{
				m_preferred_window_width = atoi(args.optarg());
				break;
			}
			// Window height
			case 'h':
			{
				m_preferred_window_height = atoi(args.optarg());
				break;
			}
			// Parent window
			case 'p':
			{
				m_parent_window_handle = string::parse_uint(args.optarg());
				break;
			}
			case 'i':
			case '?':
			default:
			{
				print_help_message();
				exit(EXIT_FAILURE);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void Device::check_preferred_settings()
{
	if (!os::is_absolute_path(m_bundle_dir))
	{
		Log::e("The root path must be absolute.");
		exit(EXIT_FAILURE);
	}

	if (m_preferred_window_width == 0 || m_preferred_window_height == 0)
	{
		Log::e("Window width and height must be greater than zero.");
		exit(EXIT_FAILURE);
	}
}

//-----------------------------------------------------------------------------
void Device::read_engine_settings()
{
}

//-----------------------------------------------------------------------------
void Device::print_help_message()
{
	os::printf(
	"Usage: crown [options]\n"
	"Options:\n\n"

	"All of the following options take precedence over\n"
	"environment variables and configuration files.\n\n"

	"  --help                     Show this help.\n"
	"  --bundle-dir <path>        Use <path> as the source directory for compiled resources.\n"
	"  --width <width>            Set the <width> of the main window.\n"
	"  --height <width>           Set the <height> of the main window.\n"
	"  --fullscreen               Start in fullscreen.\n"
	"  --parent-window <handle>   Set the parent window <handle> of the main window.\n"
	"                             Used only by tools.\n"

	"\nAvailable only in debug and development builds:\n\n"

	"  --source-dir <path>        Use <path> as the source directory for resource compilation.\n"
	"  --compile                  Run the engine as resource compiler.\n"
	"  --continue                 Do a full compile of the resources and continue the execution.\n"
	"  --quit-after-init          Quit the engine immediately after the initialization.\n");
}

Device g_device;
Device* device()
{
	return &g_device;
}

} // namespace crown

