/*
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

#include "Config.h"
#include "Device.h"
#include "Filesystem.h"
#include "InputManager.h"
#include "Log.h"
#include "OS.h"
#include "Renderer.h"
#include "Types.h"
#include "String.h"
#include "Args.h"
#include <cstdlib>

#include "renderers/gl/GLRenderer.h"
//#include "renderers/gles/GLESRenderer.h"

namespace crown
{

const uint16_t Device::CROWN_MAJOR = 0;
const uint16_t Device::CROWN_MINOR = 1;
const uint16_t Device::CROWN_MICRO = 0;

//-----------------------------------------------------------------------------
Device::Device() :
	m_preferred_window_width(1000),
	m_preferred_window_height(625),
	m_preferred_window_fullscreen(false),

	m_is_init(false),
	m_is_running(false),

	m_renderer(NULL)
{
	string::strcpy(m_preferred_root_path, string::EMPTY);
	string::strcpy(m_preferred_user_path, string::EMPTY);
}

//-----------------------------------------------------------------------------
Device::~Device()
{
}

//-----------------------------------------------------------------------------
bool Device::init(int argc, char** argv)
{
	if (parse_command_line(argc, argv) == false)
	{
		return false;
	}

	if (is_init())
	{
		Log::E("Crown Engine is already initialized.");
		return false;
	}

	// Initialize
	Log::I("Initializing Crown Engine %d.%d.%d...", CROWN_MAJOR, CROWN_MINOR, CROWN_MICRO);

	// Set the root path
	// GetFilesystem()->Init(m_preferred_root_path.c_str(), m_preferred_user_path.c_str());

	// Create the renderer
	if (m_renderer == NULL)
	{
		// FIXME FIXME FIXME
		// #ifdef CROWN_BUILD_OPENGL
		 	m_renderer = new GLRenderer();
			Log::I("Using GLRenderer.");
		// #elif defined CROWN_BUILD_OPENGLES
		//	m_renderer = new GLESRenderer();
		// #endif
	}

	m_is_init = true;

	start();

	Log::I("Crown Engine initialized.");

	return true;
}

//-----------------------------------------------------------------------------
void Device::shutdown()
{
	if (is_init() == false)
	{
		Log::E("Crown Engine is not initialized.");	
		return;
	}

	Log::I("Releasing Renderer...");

	if (m_renderer)
	{
		delete m_renderer;
	}

	m_is_init = false;
}

//-----------------------------------------------------------------------------
bool Device::is_init() const
{
	return m_is_init;
}

//-----------------------------------------------------------------------------
Renderer* Device::renderer()
{
	return m_renderer;
}

//-----------------------------------------------------------------------------
void Device::start()
{
	if (is_init() == false)
	{
		Log::E("Cannot start uninitialized engine.");
		return;
	}

	m_is_running = true;
}

//-----------------------------------------------------------------------------
void Device::stop()
{
	if (is_init() == false)
	{
		Log::E("Cannot stop uninitialized engine.");
		return;
	}

	m_is_running = false;
}

//-----------------------------------------------------------------------------
bool Device::is_running() const
{
	return m_is_running;
}

//-----------------------------------------------------------------------------
void Device::frame()
{
	get_input_manager()->event_loop();

	m_renderer->begin_frame();
	m_renderer->end_frame();
}

//-----------------------------------------------------------------------------
bool Device::parse_command_line(int argc, char** argv)
{
	int32_t fullscreen = 0;

	ArgsOption options[] = 
	{
		"help",       AOA_NO_ARGUMENT,       NULL,        'i',
		"root-path",  AOA_REQUIRED_ARGUMENT, NULL,        'r',
		"user-path",  AOA_REQUIRED_ARGUMENT, NULL,        'u',
		"width",      AOA_REQUIRED_ARGUMENT, NULL,        'w',
		"height",     AOA_REQUIRED_ARGUMENT, NULL,        'h',
		"fullscreen", AOA_NO_ARGUMENT,       &fullscreen,  1,
		NULL, 0, NULL, 0
	};

	Args args(argc, argv, "", options);

	while (1)
	{
		int32_t ret = args.next_option();

		switch (ret)
		{
			case -1:
			{
				return true;
			}
			case 0:
			{
				m_preferred_window_fullscreen = fullscreen;

				break;
			}
			// Help
			case 'i':
			{
				print_help_message();
				return false;
			}
			// Root path
			case 'r':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing absolute path after `-root-path`\n", argv[0]);
					return false;
				}

				string::strcpy(m_preferred_root_path, args.option_argument());

				break;
			}
			// User path
			case 'u':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing absolute path after `--user-path`\n", argv[0]);
					return false;
				}

				string::strcpy(m_preferred_user_path, args.option_argument());

				break;
			}
			// Window width
			case 'w':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing width value after `--width`\n", argv[0]);
					return false;
				}

				m_preferred_window_width = atoi(args.option_argument());
				break;
			}
			// Window height
			case 'h':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing height value after `--height`\n", argv[0]);
					return false;
				}

				m_preferred_window_height = atoi(args.option_argument());
				break;
			}
			default:
			{
				break;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void Device::print_help_message()
{
	os::printf("Usage: crown [options]\n");
	os::printf("Options:\n\n");

	os::printf("All of the following options take precedence over\n");
	os::printf("environment variables and configuration files.\n\n");

	os::printf("  --help                Show this help.\n");
	os::printf("  --root-path <path>    Use <path> as the filesystem root path.\n");
	os::printf("  --user-path <path>    Use <path> as the filesystem user path.\n");
	os::printf("  --width <width>       Set the <width> of the render window.\n");
	os::printf("  --height <width>      Set the <height> of the render window.\n");
	os::printf("  --fullscreen          Start in fullscreen.\n");
}

Device device;
Device* GetDevice()
{
	return &device;
}

} // namespace crown

