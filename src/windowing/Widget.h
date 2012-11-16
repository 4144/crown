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
#include "Rect.h"
#include "List.h"
#include "GarbageBin.h"
#include "WindowingEventArgs.h"
#include "WithProperties.h"
#include "Point32_t2.h"
#include "SignalSlot.h"
#include "Property.h"

namespace Crown
{

class Window;
class Bind;
class XMLNode;

enum WidgetSizeAcceptedEnum
{
	WSA_ACCEPTED,		//< The size is allowed
	WSA_MIN_SIZE,		//< The size is too small, min size would be used
	WSA_MAX_SIZE,		//< The size is too big, max size would be used
	WSA_FIXED				//< The size can't be accepted because a preferred size is set
};

enum WidgetVisibilityEnum
{
	WV_VISIBLE,		//Normal visibility
	WV_HIDDEN,		//The widget is hidden but occupies the space
	WV_COLLAPSED	//The widget is collapsed to a point32_t
};

enum FitToChildrenEnum
{
	FTC_NONE = 0,						//! Don't fit to children, occupy available space
	FTC_VERTICALLY = 1,			//! Fit to children vertically
	FTC_HORIZONTALLY = 2,		//! Fit to children horizontally
	FTC_BOTH = 3						//! Fit to children in both directions
};

struct DrawingClipInfo
{
	int32_t screenX, screenY;	//Save the screen coordinates of the current widget
	int32_t sx, sy, sw, sh;		//Scissor box
};

class Widget: public WithProperties, public IGarbageable
{
public:
	Widget(Widget* parent);
	virtual ~Widget();

	//Desired size and position
	inline const Point32_t2& GetDesiredPosition() const
	{
		return mDesiredPosition;
	}
	inline const Point32_t2& GetDesiredSize() const
	{
		return mDesiredSize;
	}
	inline const Margins& GetMargins() const
	{
		return mMargins;
	}

	void SetDesiredPosition(int32_t x, int32_t y);
	inline void SetDesiredPosition(const Point32_t2& value)
	{
		SetDesiredPosition(value.x, value.y);
	}
	void SetDesiredSize(int32_t x, int32_t y);
	inline void SetDesiredSize(const Point32_t2& value)
	{
		SetDesiredSize(value.x, value.y);
	}
	void SetMinimumSize(int32_t x, int32_t y);
	inline void SetMinimumSize(const Point32_t2& value)
	{
		SetMinimumSize(value.x, value.y);
	}
	void SetMaximumSize(int32_t x, int32_t y);
	inline void SetMaximumSize(const Point32_t2& value)
	{
		SetMaximumSize(value.x, value.y);
	}
	inline const Point32_t2& GetMinimumSize() const
	{
		return mMinimumSize;
	}
	inline const Point32_t2& GetMaximumSize() const
	{
		return mMaximumSize;
	}
	
	inline void SetMeasuredSize(int32_t x, int32_t y)
	{
		mMeasuredSize.x = x;
		mMeasuredSize.y = y;
	}
	inline const Point32_t2& GetMeasuredSize() const
	{
		return mMeasuredSize;
	}

	inline void SetTranslation(int32_t x, int32_t y)
	{
		mTranslation.x = x;
		mTranslation.y = y;
	}
	inline const Point32_t2& GetTranslation() const
	{
		return mTranslation;
	}

	inline void SetName(const Str& name)
	{
		mName = name;
	}
	inline const Str& GetName() const
	{
		return mName;
	}

	void SetMargins(int32_t left, int32_t top, int32_t right, int32_t bottom);

	inline Point32_t2 GetPosition()
	{
		return mPosition;
	}
	Point32_t2 GetScreenPosition();
	Point32_t2 GetRelativePosition(Widget* w);
	inline Point32_t2 GetSize()
	{
		return mSize;
	}
	inline Rect GetBounds()
	{
		//TODO: Use a Rect with Point32_t2 coordinates
		return Rect((mPosition + mTranslation).ToVec2(), (mPosition + mTranslation + mSize).ToVec2());
	}
	Window* GetWindow();

	inline Widget* GetParent()
	{
		return mParent;
	}
	inline Widget* GetLogicalParent()
	{
		return mLogicalParent;
	}

	void AttachToParent(Widget* value);
	void SetLogicalParent(Widget* value)
	{
		mLogicalParent = value;
	}

	inline bool IsPoint32_tInside(const Point32_t2& point32_t)
	{
		//TODO: Use a Rect with Point32_t2 coordinates
		return GetBounds().ContainsPoint32_t(point32_t.ToVec2());
	}
	inline bool IsMouseOver() const
	{
		return mIsMouseOver;
	}
	bool HasTextInputFocus();
	inline bool IsMouseSensible()
	{
		return mIsMouseSensible && mVisibility == WV_VISIBLE;
	}
	inline void SetMouseSensible(bool value)
	{
		mIsMouseSensible = value;
	}
	inline void SetFitToChildren(FitToChildrenEnum value)
	{
		if (value != mFitToChildren)
		{
			mFitToChildren = value;
			NotifyNeedsLayout();
		}
	}
	bool IsChildOf(const Widget* parent);
	virtual Widget* GetContentWidget();

	inline WidgetVisibilityEnum GetVisibility() const
	{
		return mVisibility;
	}
	inline void SetVisibility(WidgetVisibilityEnum visibility)
	{
	  bool needsLayout = (mVisibility == WV_COLLAPSED) || (visibility == WV_COLLAPSED);
		mVisibility = visibility;
		if (needsLayout)
			NotifyNeedsLayout();
	}

	IWithProperties* GetBindingContext() const;
	void SetBindingContext(IWithProperties* bindingContext);

	virtual Generic GetPropertyValue(const Str& name) const;
	virtual void SetPropertyValue(const Str& name, const Generic& value);

	void AddBind(Bind* bind);
	void ApplyBinds();

	void ApplyTemplate(XMLNode* templateNode);

	virtual void OnDraw(DrawingClipInfo& clipInfo);
	virtual void OnMeasure(bool fitChildrenX, bool fitChildrenY);
	virtual void OnMeasureFitX();
	virtual void OnMeasureFitY();
	virtual void OnArrange(Point32_t2 position, Point32_t2 size);

	bool MouseCapture();
	void MouseRelease();
	void GetFocus();
	void Destroy();

	void NotifyNeedsLayout();

	WidgetSizeAcceptedEnum GetSizeAcceptedX(int32_t sizeX);
	WidgetSizeAcceptedEnum GetSizeAcceptedY(int32_t sizeY);

	void ErodeMarginsX(Point32_t2& position, Point32_t2& size);
	void ErodeMarginsY(Point32_t2& position, Point32_t2& size);

	int32_t EnlargeMarginsX(Point32_t2& position, Point32_t2& size);
	int32_t EnlargeMarginsY(Point32_t2& position, Point32_t2& size);

	void FitMeasuredSizeX(Point32_t2& position, Point32_t2& size);
	void FitMeasuredSizeY(Point32_t2& position, Point32_t2& size);

	Widget* FindChildByName(const Str& name);
	void DestroyContent();

	//! Set the flag to draw a debug border on widgets
	inline static void SetDrawDebugBorder(bool value)
	 { mDrawDebugBorder = value; }

	virtual Str ToStr() const
	 { return "Widget"; }

	void Print32_tLoop(int32_t depth);

protected:
	Point32_t2 mDesiredPosition;
	Point32_t2 mDesiredSize;
	Point32_t2 mMeasuredSize;
	Point32_t2 mMaximumSize;
	Point32_t2 mMinimumSize;
	Point32_t2 mTranslation;

	static bool mDrawDebugBorder;

	//Preview events
	virtual void OnPreviewMouseIn(MouseEventArgs* args);
	virtual void OnPreviewMouseOut(MouseEventArgs* args);
	virtual void OnPreviewMouseMove(MouseMoveEventArgs* args);
	virtual void OnPreviewMouseDown(MouseButtonEventArgs* args);
	virtual void OnPreviewMouseUp(MouseButtonEventArgs* args);
	virtual void OnPreviewKeyDown(KeyboardEventArgs* args);
	virtual void OnPreviewKeyUp(KeyboardEventArgs* args);
	virtual void OnPreviewTextInput(TextInputEventArgs* args);

	//Bubble events
	virtual void OnMouseIn(MouseEventArgs* args);
	virtual void OnMouseOut(MouseEventArgs* args);
	virtual void OnMouseMove(MouseMoveEventArgs* args);
	virtual void OnMouseDown(MouseButtonEventArgs* args);
	virtual void OnMouseUp(MouseButtonEventArgs* args);
	virtual void OnKeyDown(KeyboardEventArgs* args);
	virtual void OnKeyUp(KeyboardEventArgs* args);
	virtual void OnTextInput(TextInputEventArgs* args);
	virtual void OnGotFocus(WindowingEventArgs* args);
	virtual void OnLostFocus(WindowingEventArgs* args);
	virtual void OnSizeChanged(WindowingEventArgs* args);

	//OnEvent helpers
	void OnMouseInHelper(bool doPreview, MouseEventArgs* args);
	void OnMouseOutHelper(bool doPreview, MouseEventArgs* args);
	void OnMouseMoveHelper(bool doPreview, MouseMoveEventArgs* args);
	void OnMouseDownHelper(bool doPreview, MouseButtonEventArgs* args);
	void OnMouseUpHelper(bool doPreview, MouseButtonEventArgs* args);
	void OnKeyDownHelper(bool doPreview, KeyboardEventArgs* args);
	void OnKeyUpHelper(bool doPreview, KeyboardEventArgs* args);
	void OnTextInputHelper(bool doPreview, TextInputEventArgs* args);
	void OnGotFocusHelper(bool doPreview, WindowingEventArgs* args);
	void OnLostFocusHelper(bool doPreview, WindowingEventArgs* args);
	void OnSizeChangedHelper(bool doPreview, WindowingEventArgs* args);
	/*
	 * MouseIn
	 * MouseOut
	 * MouseUp
	 * MouseDown
	 * MouseMove
	 * MouseWheel
	 * KeyUp
	 * KeyDown
	 * TextInput
	 * GotFocus
	 * LostFocus
	 * Loaded
	 * Unloaded
	 * SizeChanged
	 *
	 */

	friend class WindowsManager;

	void DrawInit(DrawingClipInfo& clipInfo);
	void DrawChildren(const DrawingClipInfo& clipInfo);
	void DebugDrawBorder();

	virtual void AddChild(Widget* child);
	inline const List<Widget*>& GetChildren() const
	{
		return mChildren;
	}
	virtual void Resize(int32_t width, int32_t height);
	inline void Resize(Point32_t2 size)
	{
		Resize(size.x, size.y);
	}
	virtual void Move(int32_t x, int32_t y);
	inline void Move(Point32_t2 pos)
	{
		Move(pos.x, pos.y);
	}

	inline FitToChildrenEnum GetFitToChildren()
	{
	  return mFitToChildren;
	}

	void DestroyChildren();

	//void ApplyMargins(Point32_t2& position, Point32_t2& size);

private:
	Point32_t2 mPosition;
	Point32_t2 mSize;
	Margins mMargins;
	Widget* mParent;
	Widget* mLogicalParent;
	List<Widget*> mChildren;
	Str mName;
	WidgetVisibilityEnum mVisibility;
	Weak<IWithProperties> mBindingContext;
	List<Bind*> mBinds;

	bool mIsMouseOver;
	bool mIsMouseSensible;
	FitToChildrenEnum mFitToChildren;

	//! Updates all the descending Binds bound to the BindingContext to use the new one
	void UpdateBindsToNewBindingContext(IWithProperties* newContext);
};

} //namespace Crown
