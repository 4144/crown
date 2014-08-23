
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

#include "config.h"
#include "device.h"
#include "args.h"
#include "disk_file.h"
#include "disk_filesystem.h"
#include "json_parser.h"
#include "keyboard.h"
#include "log.h"
#include "lua_environment.h"
#include "memory.h"
#include "mouse.h"
#include "os.h"
#include "os_window.h"
#include "resource_manager.h"
#include "string_setting.h"
#include "string_utils.h"
#include "touch.h"
#include "types.h"
#include "bundle.h"
#include "temp_allocator.h"
#include "resource_package.h"
#include "console_server.h"
#include "world.h"
#include "lua_stack.h"
#include "world_manager.h"
#include "network_filesystem.h"
#include "lua_system.h"
#include "debug_line.h"
#include "material_manager.h"
#include "gui.h"

#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS
	#include "bundle_compiler.h"
#endif

#if CROWN_PLATFORM_ANDROID
	#include "apk_filesystem.h"
#endif

#define MAX_SUBSYSTEMS_HEAP 8 * 1024 * 1024

namespace crown
{
//-----------------------------------------------------------------------------
Device::Device()
	: m_allocator(default_allocator(), MAX_SUBSYSTEMS_HEAP)

	, m_argc(0)
	, m_argv(NULL)

	, m_fileserver(0)
	, m_console_port(10001)

	, m_is_init(false)
	, m_is_running(false)
	, m_is_paused(false)

	, m_frame_count(0)

	, m_last_time(0)
	, m_current_time(0)
	, m_last_delta_time(0.0f)
	, m_time_since_start(0.0)

	, m_filesystem(NULL)
	, m_lua_environment(NULL)
	, m_renderer(NULL)

	, m_bundle_compiler(NULL)
	, m_console(NULL)
	, m_resource_manager(NULL)
	, m_resource_bundle(NULL)

	, m_world_manager(NULL)
{
	// Bundle dir is current dir by default.
	os::getcwd(m_bundle_dir, MAX_PATH_LENGTH);
	string::strncpy(m_source_dir, "", MAX_PATH_LENGTH);
	string::strncpy(m_boot_file, "lua/game", MAX_PATH_LENGTH);
}

//-----------------------------------------------------------------------------
Device::~Device()
{
}

//-----------------------------------------------------------------------------
void Device::init()
{
	// Initialize
	CE_LOGI("Initializing Crown Engine %d.%d.%d...", CROWN_VERSION_MAJOR, CROWN_VERSION_MINOR, CROWN_VERSION_MICRO);

	CE_LOGD("Creating filesystem...");
	// Default bundle filesystem
	#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS
		if (m_fileserver == 1)
		{
			m_filesystem = CE_NEW(m_allocator, NetworkFilesystem)(NetAddress(127, 0, 0, 1), 10001);
		}
		else
		{
			m_filesystem = CE_NEW(m_allocator, DiskFilesystem)(m_bundle_dir);
		}
	#elif CROWN_PLATFORM_ANDROID
		if (m_fileserver == 1)
		{
			m_filesystem = CE_NEW(m_allocator, NetworkFilesystem)(NetAddress(192, 168, 0, 7), 10001);
		}
		else
		{
			m_filesystem = CE_NEW(m_allocator, ApkFilesystem)();
		}
	#endif

	m_resource_bundle = Bundle::create(m_allocator, *m_filesystem);

	// Create resource manager
	CE_LOGD("Creating resource manager...");
	m_resource_manager = CE_NEW(m_allocator, ResourceManager)(*m_resource_bundle);

	// Create world manager
	CE_LOGD("Creating world manager...");
	m_world_manager = CE_NEW(m_allocator, WorldManager)();

	// Create window
	CE_LOGD("Creating main window...");
	m_window = CE_NEW(m_allocator, OsWindow);

	// Create input devices
	m_keyboard = CE_NEW(m_allocator, Keyboard);
	m_mouse = CE_NEW(m_allocator, Mouse);
	m_touch = CE_NEW(m_allocator, Touch);

	// Create renderer
	CE_LOGD("Creating renderer...");
	graphics_system::init();
	debug_line::init();
	Gui::init();

	CE_LOGD("Creating material manager...");
	material_manager::init();

	CE_LOGD("Creating lua system...");
	lua_system::init();
	m_lua_environment = CE_NEW(m_allocator, LuaEnvironment)(lua_system::state());

	CE_LOGD("Creating physics...");
	physics_system::init();

	CE_LOGD("Creating audio...");
	audio_system::init();

	CE_LOGD("Crown Engine initialized.");
	CE_LOGD("Initializing Game...");

	m_is_init = true;
	start();

	// Execute lua boot file
	m_lua_environment->load_and_execute(m_boot_file);
	m_lua_environment->call_global("init", 0);
}

//-----------------------------------------------------------------------------
void Device::shutdown()
{
	CE_ASSERT(is_init(), "Engine is not initialized");

	// Shutdowns the game
	m_lua_environment->call_global("shutdown", 0);

	CE_LOGD("Releasing audio...");
	audio_system::shutdown();

	CE_LOGD("Releasing physics...");
	physics_system::shutdown();

	CE_LOGD("Releasing lua system...");
	lua_system::shutdown();
	if (m_lua_environment)
	{
		CE_DELETE(m_allocator, m_lua_environment);
	}

	CE_LOGD("Releasing input devices...");
	CE_DELETE(m_allocator, m_touch);
	CE_DELETE(m_allocator, m_mouse);
	CE_DELETE(m_allocator, m_keyboard);

	CE_LOGD("Releasing material manager...");
	material_manager::shutdown();

	CE_LOGD("Releasing renderer...");
	debug_line::shutdown();
	graphics_system::shutdown();

	CE_LOGD("Releasing world manager...");
	CE_DELETE(m_allocator, m_world_manager);

	CE_LOGD("Releasing resource manager...");
	if (m_resource_manager)
	{
		CE_DELETE(m_allocator, m_resource_manager);
	}

	if (m_resource_bundle)
	{
		Bundle::destroy(m_allocator, m_resource_bundle);
	}

	CE_LOGD("Releasing filesystem...");
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
bool Device::is_paused() const
{
	return m_is_paused;
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
	return NULL;
}

//-----------------------------------------------------------------------------
Keyboard* Device::keyboard()
{
	return m_keyboard;
}

//-----------------------------------------------------------------------------
Mouse* Device::mouse()
{
	return m_mouse;
}

//-----------------------------------------------------------------------------
Touch* Device::touch()
{
	return m_touch;
}

//-----------------------------------------------------------------------------
Accelerometer* Device::accelerometer()
{
	return NULL;
}

//-----------------------------------------------------------------------------
void Device::start()
{
	CE_ASSERT(m_is_init, "Cannot start uninitialized engine.");

	m_is_running = true;
	m_last_time = os::clocktime();
}

//-----------------------------------------------------------------------------
void Device::stop()
{
	CE_ASSERT(m_is_init, "Cannot stop uninitialized engine.");

	m_is_running = false;
}

//-----------------------------------------------------------------------------
void Device::pause()
{
	m_is_paused = true;
	CE_LOGI("Engine paused.");
}

//-----------------------------------------------------------------------------
void Device::unpause()
{
	m_is_paused = false;
	CE_LOGI("Engine unpaused.");
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
double Device::time_since_start() const
{
	return m_time_since_start;
}

//-----------------------------------------------------------------------------
void Device::frame()
{
	m_current_time = os::clocktime();
	const int64_t time = m_current_time - m_last_time;
	m_last_time = m_current_time;
	const double freq = (double) os::clockfrequency();
	m_last_delta_time = time * (1.0 / freq);
	m_time_since_start += m_last_delta_time;

	if (!m_is_paused)
	{
		m_resource_manager->complete_requests();
		m_lua_environment->call_global("frame", 1, ARGUMENT_FLOAT, last_delta_time());
	}

	lua_system::clear_temporaries();
	m_frame_count++;
}

//-----------------------------------------------------------------------------
void Device::update_world(World* world, float dt)
{
	world->update(dt);
}

//-----------------------------------------------------------------------------
void Device::render_world(World* world, Camera* camera)
{
	world->render(camera);
}

//-----------------------------------------------------------------------------
WorldId Device::create_world()
{
	return m_world_manager->create_world();
}

//-----------------------------------------------------------------------------
void Device::destroy_world(WorldId world)
{
	m_world_manager->destroy_world(world);
}

//-----------------------------------------------------------------------------
ResourcePackage* Device::create_resource_package(const char* name)
{
	return CE_NEW(default_allocator(), ResourcePackage)(name, *m_resource_manager);
}

//-----------------------------------------------------------------------------
void Device::destroy_resource_package(ResourcePackage* package)
{
	CE_DELETE(default_allocator(), package);
}

//-----------------------------------------------------------------------------
void Device::reload(const char* , const char* )
{
/*
	#if defined(LINUX) || defined(WINDOWS)
		TempAllocator4096 temp;
		DynamicString filename(temp);
		filename += name;
		filename += '.';
		filename += type;


		if (!m_bundle_compiler->compile(m_bundle_dir, m_source_dir, filename.c_str()))
		{
			CE_LOGD("Compilation failed.");
			return;
		}

		ResourceId old_res_id(type, name);
		const void* old_res = m_resource_manager->get(old_res_id);
		m_resource_manager->unload(old_res_id, true);

		ResourceId res_id = m_resource_manager->load(type, name);
		m_resource_manager->flush();
		const void* new_res = m_resource_manager->get(res_id);

		uint32_t type_hash = string::murmur2_32(type, string::strlen(type), 0);

		switch (type_hash)
		{
			case UNIT_TYPE:
			{
				CE_LOGD("Reloading unit: %s", name);
				/// Reload unit in all worlds
				for (uint32_t i = 0; i < id_array::size(m_world_manager->worlds()); i++)
				{
					m_world_manager->worlds()[i]->reload_units((UnitResource*) old_res, (UnitResource*) new_res);
				}
				break;
			}
			case SOUND_TYPE:
			{
				CE_LOGD("Reloading sound: %s", name);
				for (uint32_t i = 0; i < id_array::size(m_world_manager->worlds()); i++)
				{
					m_world_manager->worlds()[i]->sound_world()->reload_sounds((SoundResource*) old_res, (SoundResource*) new_res);
				}
				break;
			}
			default:
			{
				CE_ASSERT(false, "Oops, unknown resource type: %s", type);
				break;
			}
		}
	#endif
*/
}

static Device* g_device;
void set_device(Device* device)
{
	g_device = device;
}

Device* device()
{
	return g_device;
}

} // namespace crown
