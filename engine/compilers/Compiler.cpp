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

#include "Compiler.h"
#include "Filesystem.h"
#include "DiskFilesystem.h"
#include "File.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
bool Compiler::compile(const char* root_path, const char* dest_path, const char* name_in, const char* name_out)
{
	DiskFilesystem root_fs(root_path);
	DiskFilesystem dest_fs(dest_path);

	// The compilation fails when returned size is zero
	size_t resource_size = 0;
	if ((resource_size = compile_impl(root_fs, name_in)) == 0)
	{
		Log::e("Compilation failed");
		return false;
	}

	// Open destination file
	File* out_file = dest_fs.open(name_out, FOM_WRITE);

	if (out_file)
	{
		// Write data
		write_impl(out_file);
		dest_fs.close(out_file);
		cleanup();
		return true;
	}

	Log::e("Unable to write compiled file.");
	return false;
}

//-----------------------------------------------------------------------------
void Compiler::cleanup()
{
}

} // namespace crown

