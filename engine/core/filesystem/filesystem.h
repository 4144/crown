/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "file.h"
#include "container_types.h"
#include "dynamic_string.h"

namespace crown
{

/// @defgroup Filesystem Filesystem

/// Provides a platform-independent way to access files and directories
/// on the host filesystem.
///
/// Accessing files:
/// Every file and every directory must be accessed through the Filesystem.
/// Not a single C/C++ std file io call or other similar facilities
/// should be used in any other part of the engine in order to maintain
/// absolute platform independence.
///
/// Filesystem maintains a root path which acts as base directory for every
/// file operation; access to files outside the root path is not allowed. If
/// you really need it, instantiate another filesystem whith the appropriate
/// root path (e.g.)
///
/// Filesystem fs("/home/foo"); // fs will use "/home/foo" as root path
///
/// fs.is_file("bar.txt");      // file "bar.txt" is relative to the root path,
///                             // so it refers to "/home/foo/bar.txt"
///
/// The filesystem will take care of the necessary path conversions.
/// The root path must be an absolute path for the underlying operating system.
/// Examples of valid root paths:
///
/// 1) "/home/foo"
/// 2) "C:\Users\Phil"
///
/// The relative paths, used to access files, must follow some strict rules:
///
/// a) Only unix-like pathnames (i.e. case sensitive and using '/' as separator)
///    are allowed.
/// b) Only relative paths are allowed: the filesystem is responsible for
///    the creation of its absolute platform-specific counterpart.
/// c) Filesystem forbids pathnames containing '.' and '..' to prevent access to
///    files outside the filesystem's root path.
/// d) Platform specific characters like '/', '\\' and ':' are forbidden as well.
/// e) Symlinks, on platforms which support them, are _not_ resolved for the same
///    reason of c)
/// f) Although mixed-case pathnames are allowed, it is generally safer to use
///    only lower-case ones for maximum compatibility.
///
/// Examples of valid relative paths.
///
/// 1) data/textures/grass.texture
/// 2) grass.texture
/// 3) foo/bar
///
/// @ingroup Filesystem
class Filesystem
{
public:

	Filesystem() {};
	virtual ~Filesystem() {};

	/// Opens the file at the given @a path with the given @a mode.
	virtual File* open(const char* path, FileOpenMode mode) = 0;

	/// Closes the given @a file.
	virtual void close(File* file) = 0;

	/// Returns whether @a path exists.
	virtual bool exists(const char* path) = 0;

	/// Returns true if @a path is a directory.
	virtual bool is_directory(const char* path) = 0;

	/// Returns true if @a path is a regular file.
	virtual bool is_file(const char* path) = 0;

	/// Creates the directory at the given @a path.
	virtual void create_directory(const char* path) = 0;

	/// Deletes the directory at the given @a path.
	virtual void delete_directory(const char* path) = 0;

	/// Creates the file at the given @a path.
	virtual void create_file(const char* path) = 0;

	/// Deletes the file at the given @a path.
	virtual void delete_file(const char* path) = 0;

	/// Returns the relative file names in the given @a path.
	virtual void list_files(const char* path, Vector<DynamicString>& files) = 0;

	/// Returns the absolute path of the given @a path based on
	/// the root path of the file source. If @a path is absolute,
	/// the given path is returned.
	virtual void get_absolute_path(const char* path, DynamicString& os_path) = 0;

private:

	// Disable copying
	Filesystem(const Filesystem&);
	Filesystem& operator=(const Filesystem&);
};

} // namespace crown
