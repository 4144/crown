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

#pragma once

#include "Keyboard.h"

struct _XDisplay;
typedef struct _XDisplay Display;

namespace crown
{

/**
	Implementation of Keyboard interface for the X11 Windowing System.
*/
class X11Keyboard : public Keyboard
{

public:

	/** @copydoc Keyboard::Keyboard() */
	X11Keyboard(InputManager* creator);

	/** @copydoc Keyboard::~Keyboard() */
	~X11Keyboard();

	/** @copydoc Keyboard::IsModifierPressed() */
	virtual bool IsModifierPressed(ModifierKey modifier) const;

	/** @copydoc Keyboard::IsKeyPressed() */
	virtual bool IsKeyPressed(KeyCode key) const;

	/** @copydoc Keyboard::IsKeyReleased() */
	virtual bool IsKeyReleased(KeyCode key) const;

	/**
		Returns whether DetectableAutoRepeat is set.
	@return
		True if set, false otherwise
	*/
	inline bool HasDetectableAutoRepeat() { return mDetectableAutoRepeat; }

private:

	Key TranslateKey(int x11Key);

	uint mModifierMask;
	bool mKeyState[MAX_KEYCODES];

	// X11 related
	Display* mDisplay;
	unsigned long mXWindow;
	bool mDetectableAutoRepeat;
};

} // namespace crown

