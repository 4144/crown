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

#include <cstdio>
#include "FileResourceArchive.h"
#include "Filesystem.h"
#include "Resource.h"
#include "FileStream.h"
#include "Log.h"
#include "String.h"

namespace crown
{

//-----------------------------------------------------------------------------
FileResourceArchive::FileResourceArchive(Filesystem& fs) :
	m_filesystem(fs)
{
}

//-----------------------------------------------------------------------------
FileResourceArchive::~FileResourceArchive()
{
}

//-----------------------------------------------------------------------------
FileStream* FileResourceArchive::find(ResourceId name)
{
	// Convert name/type into strings
	char name_string[512];
	char type_string[512];

	// FIXME
	snprintf(name_string, 512, "%X", name.name);
	snprintf(type_string, 512, "%X", name.type);

	string::strncat(name_string, type_string, 512);

	// Search the resource in the filesystem
	if (m_filesystem.exists(name_string) == false)
	{
		return NULL;
	}

	FileStream* file = (FileStream*)m_filesystem.open(name_string, SOM_READ);

	/// FIXME harcoded!!!
	file->skip(sizeof(uint32_t) * 3);

	return file;

	// FIXME NEED TO RELEASE MEMORY FOR file
}

} // namespace crown
