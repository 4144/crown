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

#include "InputManager.h"
#include "OS.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
InputManager::InputManager() :
	m_cursor_visible(true)
{
}

//-----------------------------------------------------------------------------
InputManager::~InputManager()
{
}

//-----------------------------------------------------------------------------
void InputManager::event_loop()
{
	os::OSEvent event;

	while (1)
	{
		event = os::pop_event();

		switch (event.type)
		{
			case os::OSET_NONE:
			{
				return;
			}
			case os::OSET_BUTTON_PRESS:
			case os::OSET_BUTTON_RELEASE:
			{
				MouseEvent mouse_event;
				mouse_event.x = event.data_a.int_value;
				mouse_event.y = event.data_b.int_value;
				mouse_event.button = event.data_c.int_value == 0 ? MB_LEFT : event.data_c.int_value == 1 ? MB_MIDDLE : MB_RIGHT;
				mouse_event.wheel = 0.0f;

				if (event.type == os::OSET_BUTTON_PRESS)
				{
					m_event_dispatcher.button_pressed(mouse_event);
				}
				else
				{
					m_event_dispatcher.button_released(mouse_event);
				}

				break;
			}
			case os::OSET_KEY_PRESS:
			case os::OSET_KEY_RELEASE:
			{
				KeyboardEvent keyboard_event;
				keyboard_event.key = event.data_a.int_value;

				if (event.type == os::OSET_KEY_PRESS)
				{
					m_event_dispatcher.key_pressed(keyboard_event);
				}
				else
				{
					m_event_dispatcher.key_released(keyboard_event);
				}

				break;
			}
			case os::OSET_TOUCH_DOWN:
			case os::OSET_TOUCH_UP:
			{
				TouchEvent touch_event;
				touch_event.pointer_id = event.data_a.int_value;
				touch_event.x = event.data_b.int_value;
				touch_event.y = event.data_c.int_value;
				if (event.type == os::OSET_TOUCH_DOWN)
				{
					m_event_dispatcher.touch_down(touch_event);
				}
				else
				{
					m_event_dispatcher.touch_up(touch_event);
				}
				break;
			}
			case os::OSET_TOUCH_MOVE:
			{
				TouchEvent touch_event;
				touch_event.pointer_id = event.data_a.int_value;
				touch_event.x = event.data_b.int_value;
				touch_event.y = event.data_c.int_value;	
				m_event_dispatcher.touch_move(touch_event);			
				break;
			}
			case os::OSET_ACCELEROMETER:
			{
				AccelerometerEvent sensor_event;
				sensor_event.x = event.data_a.float_value;
				sensor_event.y = event.data_b.float_value;
				sensor_event.z = event.data_c.float_value;
				m_event_dispatcher.accelerometer_changed(sensor_event);
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool InputManager::is_cursor_visible() const
{
	return m_cursor_visible;
}

//-----------------------------------------------------------------------------
void InputManager::set_cursor_visible(bool visible)
{
	if (visible)
	{
		os::hide_cursor();
	}
	else
	{
		os::show_cursor();
	}

	m_cursor_visible = visible;
}

//-----------------------------------------------------------------------------
Point2 InputManager::get_cursor_xy() const
{
	Point2 xy;

	os::get_cursor_xy(xy.x, xy.y);

	return xy;
}

//-----------------------------------------------------------------------------
void InputManager::set_cursor_xy(const Point2& position)
{
	os::set_cursor_xy(position.x, position.y);
}

//-----------------------------------------------------------------------------
Vec2 InputManager::get_cursor_relative_xy() const
{
	uint32_t window_width;
	uint32_t window_height;

	os::get_render_window_metrics(window_width, window_height);

	Vec2 pos = get_cursor_xy().to_vec2();

	pos.x = pos.x / (float) window_width;
	pos.y = pos.y / (float) window_height;

	return pos;
}

//-----------------------------------------------------------------------------
void InputManager::set_cursor_relative_xy(const Vec2& position)
{
	uint32_t window_width;
	uint32_t window_height;

	os::get_render_window_metrics(window_width, window_height);

	set_cursor_xy(Point2((int32_t)(position.x * (float) window_width), (int32_t)(position.y * (float) window_height)));
}

} // namespace crown

