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

#include "Assert.h"
#include "Types.h"
#include "ContainerTypes.h"

namespace crown
{

/// Packed array of objects with lookup table.
///
/// @ingroup Containers
template <uint32_t MAX_NUM_ID, typename T>
class IdArray
{
public:

	/// Creates the table for tracking exactly @a MAX_NUM_ID - 1 unique Ids.
					IdArray();

	/// Random access by index.
	T&				operator[](uint32_t i);
	/// Random access by index.
	const T&		operator[](uint32_t i) const;

	/// Returns a new Id.
	Id				create(const T& object);

	/// Destroys the object with the given @a id.
	void			destroy(Id id);

	/// Returns whether the table has the object with the given @a id
	bool			has(Id id) const;

	/// Returns the number of objects in the array.
	uint32_t		size() const;

	/// Returns the object with the given @a id.
	T&				lookup(const Id& id);

	T*				begin();
	const T*		begin() const;
	T*				end();
	const T*		end() const;

private:

	// Returns the next available unique id.
	uint16_t		next_id();

public:

	// The index of the first unused id
	uint16_t		m_freelist;

	// The index of the last id in the id table
	uint16_t		m_last_index;

	// Next available unique id
	uint16_t		m_next_id;
	uint16_t		m_num_objects;


	// The last valid id is reserved and cannot be used to
	// refer to Ids from the outside
	Id				m_sparse[MAX_NUM_ID];
	uint16_t		m_sparse_to_dense[MAX_NUM_ID];
	uint16_t		m_dense_to_sparse[MAX_NUM_ID];
	T				m_objects[MAX_NUM_ID];
};

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline IdArray<MAX_NUM_ID, T>::IdArray()
	: m_freelist(INVALID_ID)
	, m_last_index(0)
	, m_next_id(0)
	, m_num_objects(0)
{
	for (uint32_t i = 0; i < MAX_NUM_ID; i++)
	{
		m_sparse[i].id = INVALID_ID;
	}
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline T& IdArray<MAX_NUM_ID, T>::operator[](uint32_t i)
{
	CE_ASSERT(i < m_num_objects, "Index out of bounds");
	return m_objects[i];
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline const T& IdArray<MAX_NUM_ID, T>::operator[](uint32_t i) const
{
	CE_ASSERT(i < m_num_objects, "Index out of bounds");
	return m_objects[i];
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline Id IdArray<MAX_NUM_ID, T>::create(const T& object)
{
	CE_ASSERT(m_num_objects < MAX_NUM_ID, "Object list full");

	// Obtain a new id
	Id id;
	id.id = next_id();

	// Recycle slot if there are any
	if (m_freelist != INVALID_ID)
	{
		id.index = m_freelist;
		m_freelist = m_sparse[m_freelist].index;
	}
	else
	{
		id.index = m_last_index++;
	}

	m_sparse[id.index] = id;
	m_sparse_to_dense[id.index] = m_num_objects;
	m_dense_to_sparse[m_num_objects] = id.index;
	m_objects[m_num_objects] = object;
	m_num_objects++;

	return id;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline void IdArray<MAX_NUM_ID, T>::destroy(Id id)
{
	CE_ASSERT(has(id), "IdArray does not have ID: %d,%d", id.id, id.index);

	m_sparse[id.index].id = INVALID_ID;
	m_sparse[id.index].index = m_freelist;
	m_freelist = id.index;

	// Swap with last element
	const uint32_t last = m_num_objects - 1;
	CE_ASSERT(last >= m_sparse_to_dense[id.index], "Swapping with previous item");
	m_objects[m_sparse_to_dense[id.index]] = m_objects[last];

	// Update tables
	uint16_t std = m_sparse_to_dense[id.index];
	uint16_t dts = m_dense_to_sparse[last];
	m_sparse_to_dense[dts] = std;
	m_dense_to_sparse[std] = dts;
	m_num_objects--;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline T& IdArray<MAX_NUM_ID, T>::lookup(const Id& id)
{
	CE_ASSERT(has(id), "IdArray does not have ID: %d,%d", id.id, id.index);

	return m_objects[m_sparse_to_dense[id.index]];
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline bool IdArray<MAX_NUM_ID, T>::has(Id id) const
{
	return id.index < MAX_NUM_ID && m_sparse[id.index].id == id.id;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline uint32_t IdArray<MAX_NUM_ID, T>::size() const
{
	return m_num_objects;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline uint16_t IdArray<MAX_NUM_ID, T>::next_id()
{
	return m_next_id++;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline T* IdArray<MAX_NUM_ID, T>::begin()
{
	return m_objects;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline const T* IdArray<MAX_NUM_ID, T>::begin() const
{
	return m_objects;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline T* IdArray<MAX_NUM_ID, T>::end()
{
	return m_objects + m_num_objects;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline const T* IdArray<MAX_NUM_ID, T>::end() const
{
	return m_objects + m_num_objects;
}

} // namespace crown
