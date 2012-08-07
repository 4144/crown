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
#include "Widget.h"
#include "SignalSlot.h"
#include "Button.h"
#include "Label.h"
#include "DragArea.h"
#include "Str.h"

namespace Crown
{

class WindowsManager;
class WindowContext;

class Window : public Widget
{
public:
	Window(WindowsManager* wm, int width, int height, Str title);
	Window(WindowsManager* wm, int x, int y, int width, int height, Str title);
	virtual ~Window();

	inline WindowsManager* GetWindowsManager()
	{
		return mWindowsManager;
	}
	virtual Widget* GetContentWidget();
	inline WindowContext* GetWindowContext() const
	 { return mWindowContext; }

	virtual void OnDraw(DrawingClipInfo& clipInfo);

	void NotifyNeedsLayout();
	void Close();
	void DoModal(Window* childWindow);

	void _PerformLayout();

	MulticastEvent<Window, WindowCloseEventArgs*> OnCloseEvent;

	virtual Str ToStr() const
	 { return "Window"; }

protected:
	void CloseButton_OnClick(Button* widget, EventArgs* args);

private:
	WindowsManager* mWindowsManager;
	Window* mModalParent;
	Widget* mClientArea;
	Button* mCloseButton;
	Label* mTitle;
	DragArea* mDragArea;
	bool mNeedsLayout;
	bool mIsDoingModal;
	WindowContext* mWindowContext;

	static Point2 mDefaultPosition;

	void InitWindow(int x, int y, int width, int height, Str title);

	friend class WindowsManager;
	friend class WindowContext;
};

} //namespace Crown
