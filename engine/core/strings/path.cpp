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

#include "path.h"

namespace crown
{
namespace path
{
	bool is_valid_segment(const char* segment)
	{
		CE_ASSERT(segment != NULL, "Segment must be != NULL");

		size_t segment_len = strlen(segment);

		// Empty segment is not valid
		if (segment_len == 0)
		{
			return false;
		}

		// Segments containing only '.' are non valid
		if (segment_len == 1 && segment[0] == '.')
		{
			return false;
		}

		// Segments containing only ".." are not valid
		if (segment_len == 2 && segment[0] == '.' && segment[1] == '.')
		{
			return false;
		}

		// The segment does not have to contain any forward slashes ('/')
		// nor back slashes ('\'), nor colon signs (':')
		for (size_t i = 0; i < segment_len; i++)
		{
			if (segment[i] == '/' ||
				segment[i] == '\\' ||
				segment[i] == ':')
			{
				return false;
			}
		}

		return true;
	}

	/// Returns whether the path is valid.
	/// @note
	/// The rules for valid paths are as follows:
	/// a) The empty string is not valid.
	/// b) If the path is absolute, it mustn't contain any leading character.
	bool is_valid_path(const char* path)
	{
		(void)path;
	//	size_t path_len = strlen(path);

	//	if (pathLen == 0)
	//	{
	//		return false;
	//	}

	//	if (is_root_path(path))
	//	{
	//		return true;
	//	}

	//	Array<Str> segmentList;
	//	if (!get_segments(Str(path), segmentList))
	//	{
	//		return false;
	//	}

	//	size_t i = 0;
	//	if (IsAbsolutePath(path) && path[0] != '/')
	//	{
	//		i = 1;
	//	}

	//	for (; i < segmentList.GetSize(); i++)
	//	{
	//		if (!IsValidSegment(segmentList[i].c_str()))
	//		{
	//			return false;
	//		}
	//	}

		return true;
	}

	/// Returns the pathname of the path.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "/home/project"
	/// e.g. "/home/project" -> "/home"
	/// e.g. "/home" -> "/"
	/// e.g. "home" -> ""
	///
	/// The @a path must be valid.
	void pathname(const char* path, char* str, size_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_separator = find_last(path, '/');

		if (last_separator == end(path))
		{
			strncpy(str, "", len);
		}
		else
		{
			substring(begin(path), last_separator, str, len);
		}
	}

	/// Returns the filename of the path.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "texture.tga"
	/// e.g. "/home/project/texture" -> "texture"
	/// e.g. "/home -> "home"
	/// e.g. "/" -> ""
	///
	/// The @a path must be valid.
	void filename(const char* path, char* str, size_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_separator = find_last(path, '/');

		if (last_separator == end(path))
		{
			strncpy(str, "", len);
		}
		else
		{
			substring(last_separator + 1, end(path), str, len);
		}
	}

	/// Returns the basename of the path.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "texture"
	/// e.g. "/home/project" -> "project"
	/// e.g. "/" -> ""
	///
	/// The @a path must be valid.
	void basename(const char* path, char* str, size_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_separator = find_last(path, '/');
		const char* last_dot = find_last(path, '.');

		if (last_separator == end(path) && last_dot != end(path))
		{
			substring(begin(path), last_dot, str, len);
		}
		else if (last_separator != end(path) && last_dot == end(path))
		{
			substring(last_separator + 1, end(path), str, len);
		}
		else if (last_separator == end(path) && last_dot == end(path))
		{
			strncpy(str, path, len);
		}
		else
		{
			substring(last_separator + 1, last_dot, str, len);
		}
	}

	/// Returns the extension of the path.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "tga"
	/// e.g. "/home/project.x/texture" -> ""
	///
	/// The @a path must be valid.
	void extension(const char* path, char* str, size_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_dot = find_last(path, '.');

		if (last_dot == end(path))
		{
			strncpy(str, "", len);
		}
		else
		{
			substring(last_dot + 1, end(path), str, len);
		}
	}

	/// Returns the filename without the extension.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "/home/project/texture"
	/// e.g. "/home/project/texture" -> "/home/project/texture"
	///
	/// The @a path must be valid.
	void filename_without_extension(const char* path, char* str, size_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_dot = find_last(path, '.');

		substring(begin(path), last_dot, str, len);
	}

	/// Returns the segments contained in path.
	//bool segments(const char* path, Array<Str>& ret)
	//{
	//	path.Split(os::PATH_SEPARATOR, ret);

	//	if (ret.GetSize() > 0)
	//	{
	//		return true;
	//	}

	//	return false;
	//}

	/// Fills 'ret' with the same path but without the trailing directory separator.
	/// @note
	/// e.g. "/home/project/texture.tga/" -> "/home/project/texture.tga"
	/// e.g. "/home/project/texture.tga" -> "/home/project/texture.tga"
	///
	/// The @a path must be valid.
	void strip_trailing_separator(const char* path, char* str, size_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		size_t path_len = strlen(path);

		if (path[path_len - 1] == '/')
		{
			substring(begin(path), end(path) - 2, str, len);
		}
		else
		{
			substring(begin(path), end(path), str, len);
		}
	}
} // namespace path
} // namespace crown
