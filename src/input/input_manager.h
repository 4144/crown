/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "input_types.h"

namespace crown
{

class InputManager
{
public:

	InputManager();
	~InputManager();

	/// Creates a new input device and returns it.
	InputDevice* create_input_device(const char* name, uint8_t num_buttons, uint8_t num_axes);

	/// Returns the default keyboard input device.
	InputDevice* keyboard();

	/// Returns the default mouse input device.
	InputDevice* mouse();

	/// Returns the default touch input device.
	InputDevice* touch();

	/// Returns the default joypad input device.
	InputDevice* joypad();

	/// Updates the input devices
	void update();

private:

	InputDevice* _keyboard;
	InputDevice* _mouse;
	InputDevice* _touch;
	InputDevice* _joypad;
};

} // namespace crown
