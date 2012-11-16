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

#include "Types.h"
#include "Point2.h"
#include "Sprite.h"

namespace Crown
{

enum ThemesFunctions
{
	TF_BUTTON_NORMAL = 0,
	TF_BUTTON_HIGHLIGHTED,
	TF_BUTTON_PRESSED,
	TF_WINDOW,
	TF_LISTVIEW
};

enum ThemesSprites
{
	TS_CLOSEBUTTON_X = 0,
	TS_BOOM
};

typedef void (*ThemeFunction)(const Point2& size);

class ITheme
{
public:

	virtual ~ITheme() {}

	virtual ThemeFunction GetFunction(ThemesFunctions functionId) = 0;
	virtual Sprite* GetSprite(ThemesSprites spriteId) = 0;
};

class Themes
{
public:
	~Themes();

	static void LoadTheme(ITheme* theme);
	static void Draw(ThemesFunctions functionId, const Point2& size);
	static void Draw(ThemesSprites spriteId, uint32_t frameNumber);
	static Sprite* GetSprite(ThemesSprites spriteId);

private:
	Themes();

	static ITheme* mCurrentTheme;
};

} //namespace Crown
