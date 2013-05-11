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
#include "Stream.h"
#include "Allocator.h"
#include <cassert>

namespace crown
{

class MemoryBuffer
{
public:

						MemoryBuffer();
	virtual				~MemoryBuffer();

	virtual void		release() = 0;
	virtual void		allocate(size_t size) = 0;

	virtual bool		is_valid() = 0;
	virtual size_t		size() = 0;
	virtual uint8_t*	data() = 0;

	virtual void		write(uint8_t* src, size_t offset, size_t size) = 0;
};

class DynamicMemoryBuffer: public MemoryBuffer
{
public:

						DynamicMemoryBuffer(Allocator& allocator, size_t initial_capacity);
	virtual				~DynamicMemoryBuffer();

	void				release();
	void				allocate(size_t capacity);

	inline bool			is_valid() { return m_buffer != 0; }

	void				check_space(size_t offset, size_t space);
	void				write(uint8_t* src, size_t offset, size_t size);

	inline size_t		size() { return m_size; }
	inline size_t		capacity() { return m_capacity; }

	inline uint8_t*		data() { return m_buffer; }

protected:

	Allocator&			m_allocator;
	uint8_t*			m_buffer;
	size_t				m_capacity;
	size_t				m_size;
};

/// Memory stream.
/// Access memory buffers.
class MemoryStream: public Stream
{
public:

	/// @copydoc Stream::Stream()
						MemoryStream(MemoryBuffer* buffer, StreamOpenMode mode);

	/// @copydoc Stream::~Stream()
	virtual				~MemoryStream();

	/// @copydoc Stream::seek()
	void				seek(size_t position);

	/// @copydoc Stream::seek_to_end()
	void				seek_to_end();

	/// @copydoc Stream::skip()
	void				skip(size_t bytes);

	/// @copydoc Stream::read_byte()
	uint8_t				read_byte();

	/// @copydoc Stream::read()
	void				read(void* buffer, size_t size);

	/// @copydoc Stream::write_byte()
	void				write_byte(uint8_t val);

	/// @copydoc Stream::write()
	void				write(const void* buffer, size_t size);

	/// @copydoc Stream::copy_to()
	bool				copy_to(Stream* stream, size_t size = 0);

	/// @copydoc Stream::flush()
	void				flush();

	/// @copydoc Stream::end_of_stream()
	bool				end_of_stream() const { return size() == m_memory_offset; }

	/// @copydoc Stream::is_valid()
	bool				is_valid() const { assert(m_memory != NULL); return m_memory->is_valid(); }

	/// @copydoc Stream::size()
	size_t				size() const { assert(m_memory != NULL); return m_memory->size(); }

	/// @copydoc Stream::position()
	size_t				position() const { return m_memory_offset; }

	/// @copydoc Stream::can_read()
	bool				can_read() const { return true; }

	/// @copydoc Stream::can_write()
	bool				can_write() const { return true; }

	/// @copydoc Stream::can_seek()
	bool				can_seek() const { return true; }

	/// Dumps the data to the console.
	void				dump();

protected:

	inline void			check_valid() { assert(m_memory != NULL); }

	MemoryBuffer*		m_memory;
	size_t				m_memory_offset;
};

}
