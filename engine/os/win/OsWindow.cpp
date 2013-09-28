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

#include "OsWindow.h"

namespace crown
{

//-----------------------------------------------------------------------------
OsWindow::OsWindow(uint32_t , uint32_t , uint32_t )
	// m_x(0),
	// m_y(0),
	// m_width(width),
	// m_height(height),
	// m_resizable(true),
{
}

//-----------------------------------------------------------------------------
OsWindow::~OsWindow()
{
}

//-----------------------------------------------------------------------------
void OsWindow::show()
{
}

//-----------------------------------------------------------------------------
void OsWindow::hide()
{
}

//-----------------------------------------------------------------------------
void OsWindow::get_size(uint32_t& width, uint32_t& height)
{
	width = m_width;
	height = m_height;
}

//-----------------------------------------------------------------------------
void OsWindow::get_position(uint32_t& x, uint32_t& y)
{
	x = m_x;
	y = m_y;
}

//-----------------------------------------------------------------------------
void OsWindow::resize(uint32_t , uint32_t )
{
}

//-----------------------------------------------------------------------------
void OsWindow::move(uint32_t , uint32_t )
{
}

//-----------------------------------------------------------------------------
void OsWindow::minimize()
{
}

//-----------------------------------------------------------------------------
void OsWindow::restore()
{
}

//-----------------------------------------------------------------------------
bool OsWindow::is_resizable() const
{
	return m_resizable;
}

//-----------------------------------------------------------------------------
void OsWindow::set_resizable(bool )
{
}

//-----------------------------------------------------------------------------
void OsWindow::show_cursor(bool )
{
}

//-----------------------------------------------------------------------------
void OsWindow::get_cursor_xy(int32_t& , int32_t& )
{
}

//-----------------------------------------------------------------------------
void OsWindow::set_cursor_xy(int32_t , int32_t )
{
}

//-----------------------------------------------------------------------------
char* OsWindow::title()
{
	return NULL;
}

//-----------------------------------------------------------------------------
void OsWindow::set_title(const char* )
{
}

} // namespace crown