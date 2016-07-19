/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "error.h"
#include "input_device.h"
#include "memory.h"
#include "string_id.h"
#include "string_utils.h"
#include <string.h> // strcpy, memset

namespace crown
{
const char* InputDevice::name() const
{
	return _name;
}

bool InputDevice::connected() const
{
	return _connected;
}

u8 InputDevice::num_buttons() const
{
	return _num_buttons;
}

u8 InputDevice::num_axes() const
{
	return _num_axes;
}

bool InputDevice::pressed(u8 id) const
{
	CE_ASSERT(id < _num_buttons, "Index out of bounds");
	return (~_last_state[id] & _state[id]) != 0;
}

bool InputDevice::released(u8 id) const
{
	CE_ASSERT(id < _num_buttons, "Index out of bounds");
	return (_last_state[id] & ~_state[id]) != 0;
}

bool InputDevice::any_pressed() const
{
	return pressed(_last_button);
}

bool InputDevice::any_released() const
{
	return released(_last_button);
}

Vector3 InputDevice::axis(u8 id) const
{
	CE_ASSERT(id < _num_axes, "Index out of bounds");
	return _axis[id];
}

const char* InputDevice::button_name(u8 id)
{
	CE_ASSERT(id < _num_buttons, "Index out of bounds");
	return _button_name[id];
}

const char* InputDevice::axis_name(u8 id)
{
	CE_ASSERT(id < _num_axes, "Index out of bounds");
	return _axis_name[id];
}

u8 InputDevice::button_id(StringId32 name)
{
	for (u32 i = 0; i < _num_buttons; ++i)
	{
		if (_button_hash[i] == name)
			return i;
	}

	CE_FATAL("Unknown button name");
	return 0;
}

u8 InputDevice::axis_id(StringId32 name)
{
	for (u32 i = 0; i < _num_axes; ++i)
	{
		if (_axis_hash[i] == name)
			return i;
	}

	CE_FATAL("Unknown axis name");
	return 0;
}

void InputDevice::set_connected(bool connected)
{
	_connected = connected;
}

void InputDevice::set_button_state(u8 i, bool state)
{
	CE_ASSERT(i < _num_buttons, "Index out of bounds");
	_last_button = i;
	_state[i] = state;
}

void InputDevice::set_axis(u8 i, const Vector3& value)
{
	CE_ASSERT(i < _num_axes, "Index out of bounds");
	_axis[i] = value;
}

void InputDevice::update()
{
	memcpy(_last_state, _state, sizeof(u8)*_num_buttons);
}

namespace input_device
{
	InputDevice* create(Allocator& a, const char* name, u8 num_buttons, u8 num_axes, const char** button_names, const char** axis_names)
	{
		const u32 size = 0
			+ sizeof(InputDevice) + alignof(InputDevice)
			+ sizeof(u8)*num_buttons*2 + alignof(u8)
			+ sizeof(Vector3)*num_axes + alignof(Vector3)
			+ sizeof(char*)*num_buttons + alignof(char*)
			+ sizeof(char*)*num_axes + alignof(char*)
			+ sizeof(StringId32)*num_buttons + alignof(StringId32)
			+ sizeof(StringId32)*num_axes + alignof(StringId32)
			+ strlen32(name) + 1 + alignof(char)
			;

		InputDevice* id = (InputDevice*)a.allocate(size);

		id->_connected   = false;
		id->_num_buttons = num_buttons;
		id->_num_axes    = num_axes;
		id->_last_button = 0;

		id->_last_state  = (u8*         )&id[1];
		id->_state       = (u8*         )memory::align_top(id->_last_state + num_buttons,  alignof(*id->_state      ));
		id->_axis        = (Vector3*    )memory::align_top(id->_state + num_buttons,       alignof(*id->_axis       ));
		id->_button_name = (const char**)memory::align_top(id->_axis + num_axes,           alignof(*id->_button_name));
		id->_axis_name   = (const char**)memory::align_top(id->_button_name + num_buttons, alignof(*id->_axis_name  ));
		id->_button_hash = (StringId32* )memory::align_top(id->_axis_name + num_axes,      alignof(*id->_button_hash));
		id->_axis_hash   = (StringId32* )memory::align_top(id->_button_hash + num_buttons, alignof(*id->_axis_hash  ));
		id->_name        = (char*       )memory::align_top(id->_axis_hash + num_axes,      alignof(*id->_name       ));

		memset(id->_last_state, 0, sizeof(u8)*num_buttons);
		memset(id->_state, 0, sizeof(u8)*num_buttons);
		memset(id->_axis, 0, sizeof(Vector3)*num_axes);
		memcpy(id->_button_name, button_names, sizeof(const char*)*num_buttons);
		memcpy(id->_axis_name, axis_names, sizeof(const char*)*num_axes);

		for (u32 i = 0; i < num_buttons; ++i)
			id->_button_hash[i] = StringId32(button_names[i]);

		for (u32 i = 0; i < num_axes; ++i)
			id->_axis_hash[i] = StringId32(axis_names[i]);

		strcpy(id->_name, name);

		return id;
	}

	void destroy(Allocator& a, InputDevice& id)
	{
		a.deallocate(&id);
	}
} // namespace input_device

} // namespace crown
