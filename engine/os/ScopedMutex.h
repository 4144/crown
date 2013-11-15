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

#pragma once

#include "Mutex.h"

namespace crown
{

/// Automatically locks a mutex when created and unlocks when destroyed.
class ScopedMutex
{
public:

	/// Locks the given @a m mutex.
	ScopedMutex(Mutex& m)
		: m_mutex(m)
	{
		m_mutex.lock();
	}

	/// Unlocks the mutex passed to ScopedMutex::ScopedMutex()
	~ScopedMutex()
	{
		m_mutex.unlock();
	}

private:

	Mutex& m_mutex;

private:

	// Disable copying
	ScopedMutex(const ScopedMutex&);
	ScopedMutex& operator=(const ScopedMutex&);
};

} // namespace crown
