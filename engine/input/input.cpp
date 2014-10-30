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

#include "input.h"
#include "keyboard.h"
#include "mouse.h"
#include "touch.h"
#include "memory.h"

namespace crown
{
namespace input_globals
{
	const size_t BUFFER_SIZE = sizeof(Keyboard) +
								sizeof(Mouse) + sizeof(Touch);
	char _buffer[BUFFER_SIZE];
	Keyboard* _keyboard = NULL;
	Mouse* _mouse = NULL;
	Touch* _touch = NULL;

	void init()
	{
		_keyboard = new (_buffer) Keyboard();
		_mouse = new (_buffer + sizeof(Keyboard)) Mouse();
		_touch = new (_buffer + sizeof(Keyboard) + sizeof(Mouse)) Touch();
	}

	void shutdown()
	{
		_keyboard->~Keyboard();
		_keyboard = NULL;
		_mouse->~Mouse();
		_mouse = NULL;
		_touch->~Touch();
		_touch = NULL;
	}

	void update()
	{
		_keyboard->update();
		_mouse->update();
		_touch->update();
	}

	Keyboard& keyboard()
	{
		return *_keyboard;
	}

	Mouse& mouse()
	{
		return *_mouse;
	}

	Touch& touch()
	{
		return *_touch;
	}
} // namespace input_globals
} // namespace crown
