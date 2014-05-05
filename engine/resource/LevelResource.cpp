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

#include "LevelResource.h"
#include "Array.h"
#include "Memory.h"
#include "JSONParser.h"
#include "Filesystem.h"

namespace crown
{
namespace level_resource
{

void parse_units(JSONElement root, Array<LevelUnit>& units)
{
	JSONElement units_arr = root.key("units");
	const uint32_t size = units_arr.size();

	for (uint32_t i = 0; i < size; i++)
	{
		JSONElement e = units_arr[i];

		LevelUnit lu;

		DynamicString name;
		e.key("name").to_string(name);
		name += ".unit";

		lu.name = ResourceId(name.c_str());
		lu.position = e.key("position").to_vector3();
		lu.rotation = e.key("rotation").to_quaternion();

		array::push_back(units, lu);
	}
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	Array<LevelUnit> units(default_allocator());

	parse_units(root, units);

	fs.close(file);
	default_allocator().deallocate(buf);

	LevelHeader lh;
	lh.num_units = array::size(units);
	uint32_t offt = sizeof(LevelHeader);
	lh.units_offset = offt;

	out_file->write((char*) &lh, sizeof(LevelHeader));

	if (lh.num_units)
	{
		out_file->write((char*) array::begin(units), sizeof(LevelUnit) * lh.num_units);
	}
}

} // namespace level_resource
} // namespace crown
