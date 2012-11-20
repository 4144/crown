#include <cassert>
#include <cstring>

#include "Message.h"

namespace crown
{
namespace network
{
  
Message::Message() : w_data(NULL), r_data(NULL), max_size(0), cur_size(0), write_bit(0), read_count(0), read_bit(0)
{
  
}

//---------------------------------------------------------------------------------------------

Message::~Message()
{
  
}

//---------------------------------------------------------------------------------------------

uint8_t* Message::get_byte_space(int32_t len)
{
	uint8_t *ptr;

	if (!w_data) 
	{
		printf( "idBitMsg::GetByteSpace: cannot write to message" );
	}

	// round up to the next byte
	write_byte_align();

	// check for overflow
	check_overflow(len << 3);

	// allocate space
	ptr = w_data + cur_size;
	cur_size += len;
	
	return ptr;  
}

//---------------------------------------------------------------------------------------------

bool Message::check_overflow(int32_t num_bits)
{
	assert( num_bits >= 0 );
	
	if (num_bits > get_remaining_write_bits()) 
	{
		if (num_bits > (max_size << 3)) 
		{
			printf(" %i bits is > full message size", num_bits );
		}
		printf("overflow\n");
		begin_writing();
		overflowed = true;
		return true;
	}
	return false;  
}

//---------------------------------------------------------------------------------------------

void Message::init(uint8_t *data, int32_t len)
{
	w_data = data;
	r_data = data;
	max_size = len;
}

//---------------------------------------------------------------------------------------------

void Message::init(const uint8_t *data, int32_t len)
{
	w_data = NULL;
	r_data = data;
	max_size = len;
}

//---------------------------------------------------------------------------------------------

uint8_t* Message::get_data()
{
	return w_data;
}

//---------------------------------------------------------------------------------------------

const uint8_t* Message::get_data() const
{
	return r_data;
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_max_size() const
{
	return max_size;
}

//---------------------------------------------------------------------------------------------

bool Message::is_overflowed()
{
	return overflowed;
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_size() const
{
	return cur_size;
}

//---------------------------------------------------------------------------------------------

void Message::set_size(int32_t size)
{
	if (size > max_size)
	{
		cur_size = max_size;
	}
	else
	{
		cur_size = size;
	}
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_write_bit() const
{
	return write_bit;
}

//---------------------------------------------------------------------------------------------

void Message::set_write_bit(int32_t bit)
{
	write_bit = bit & 7;
	if (write_bit) 
	{
		w_data[cur_size-1] &= (1 << write_bit) - 1;
	}
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_num_bits_written() const
{
	return ((cur_size << 3) - ((8 - write_bit) & 7));  
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_remaining_write_bits() const
{
	return (max_size << 3) - get_num_bits_written(); 
}

//---------------------------------------------------------------------------------------------

void Message::save_write_state(int32_t& s,int32_t& b) const
{
	s = cur_size;
	b = write_bit;
}

//---------------------------------------------------------------------------------------------

void Message::restore_write_state(int32_t s,int32_t b)
{
	cur_size = s;
	write_bit = b & 7;
	
	if (write_bit) 
	{
		w_data[cur_size-1] &= (1 << write_bit) - 1;
	}  
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_read_count() const
{
	return read_count;
}

//---------------------------------------------------------------------------------------------

void Message::set_read_count(int32_t bytes)
{
	read_count = bytes;
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_read_bit() const
{
	return read_bit;
}

//---------------------------------------------------------------------------------------------

void Message::set_read_bit(int32_t bit)
{
	read_bit = bit & 7;
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_num_bits_read() const
{
	return ((read_count << 3) - ((8 - read_bit) & 7));  
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_remaining_read_bits() const
{
	return (cur_size << 3) - get_num_bits_read();
}

//---------------------------------------------------------------------------------------------

void Message::save_read_state(int32_t& c, int32_t& b) const
{
	c = read_count;
	b = read_bit;
}

//---------------------------------------------------------------------------------------------

void Message::restore_read_state(int32_t c, int32_t b)
{
	read_count = c;
	read_bit = b & 7;
}

//---------------------------------------------------------------------------------------------

void Message::begin_writing()
{
	cur_size = 0;
	write_bit = 0;
	overflowed = false;
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_remaining_space() const
{
	return max_size - cur_size;
}

//---------------------------------------------------------------------------------------------

void Message::write_byte_align()
{
	write_bit = 0;
}

//---------------------------------------------------------------------------------------------

void Message::write_bits(int32_t value, int32_t num_bits)
{
	int32_t		put;
	int32_t		fraction;

	// check if w_data is void
	if (!w_data) 
	{
		printf( "cannot write to message" );
	}
	// check if the number of bits is valid
	if (num_bits == 0 || num_bits < -31 || num_bits > 32) 
	{
		printf( "bad numBits %i", num_bits);
	}

	// check for value overflows
	// this should be an error really, as it can go unnoticed and cause either bandwidth or corrupted data transmitted
	if (num_bits != 32) 
	{
		if (num_bits > 0) 
		{
			if (value > (1 << num_bits) - 1) 
			{
				printf( "value overflow %d %d", value, num_bits );
			} 
			else if (value < 0) 
			{
				printf( "value overflow %d %d", value, num_bits );
			}
		} 
		else 
		{
			int32_t r = 1 << (-1 - num_bits);
			if (value > r - 1) 
			{
				printf( "value overflow %d %d", value, num_bits );
			} 
			else if (value < -r) 
			{
				printf( "value overflow %d %d", value, num_bits );
			}
		}
	}

	// Change sign if it is negative
	if (num_bits < 0 ) 
	{
		num_bits = -num_bits;
	}

	// check for msg overflow
	if (check_overflow(num_bits)) 
	{
		return;	
	}

	// write the bits
	while(num_bits) 
	{
		if (write_bit == 0) 
		{
			w_data[cur_size] = 0;
			cur_size++;
		}
		
		put = 8 - write_bit;
		if (put > num_bits) 
		{
			put = num_bits;
		}
		
		fraction = value & ((1 << put) - 1);
		w_data[cur_size - 1] |= fraction << write_bit;
		num_bits -= put;
		value >>= put;
		write_bit = (write_bit + put) & 7;
	}
}

//---------------------------------------------------------------------------------------------

void Message::write_int8(int32_t c)
{
	write_bits(c, -8);
}

//---------------------------------------------------------------------------------------------

void Message::write_uint8(int32_t c)
{
	write_bits(c, 8);  
}

//---------------------------------------------------------------------------------------------

void Message::write_int16(int32_t c)
{
	write_bits(c, -16);  
}

//---------------------------------------------------------------------------------------------

void Message::write_uint16(int32_t c)
{
	write_bits(c, 16);
}

//---------------------------------------------------------------------------------------------

void Message::write_int32(int32_t c)
{
	write_bits(c, 32);
}

//---------------------------------------------------------------------------------------------

void Message::write_real(real f)
{
	write_bits(*reinterpret_cast<int32_t *>(&f), 32);  
}

//---------------------------------------------------------------------------------------------

void Message::write_vec3(const Vec3& v, int32_t num_bits)
{
	write_bits(vec3_to_bits(v, num_bits), num_bits);
}

//---------------------------------------------------------------------------------------------

void Message::write_string(const char* s, int32_t max_len, bool make_7_bit)
{
	if (!s) 
	{
		write_data("", 1);
	}
	else 
	{
		int32_t i;
		int32_t l;
		uint8_t* data_ptr;
		const uint8_t* byte_ptr;
		
		// calculate s length
		for (l = 0; s[l]; l++) {}
		
		if (max_len >= 0 && l >= max_len) 
		{
			l = max_len - 1;
		}
		
		data_ptr = get_byte_space(l + 1);
		byte_ptr = reinterpret_cast<const uint8_t*>(s);
		if (make_7_bit) 
		{
			for (i = 0; i < l; i++) 
			{
				if ( byte_ptr[i] > 127 ) 
				{
					data_ptr[i] = '.';
				} 
				else 
				{
					data_ptr[i] = byte_ptr[i];
				}
			}
		}
		else 
		{
			for (i = 0; i < l; i++) 
			{
				data_ptr[i] = byte_ptr[i];
			}
		}
		
		data_ptr[i] = '\0';
	}  
}

//---------------------------------------------------------------------------------------------

void Message::write_data(const void* data, int32_t length)
{
	memcpy(get_byte_space(length), data, length);
}

//---------------------------------------------------------------------------------------------

void Message::write_ipv4addr(const os::IPv4Address addr)
{
	uint8_t* ptr;
	
	ptr = get_byte_space(4);
	memcpy(ptr, addr.address, 4);
	write_uint16(addr.port);
}

//---------------------------------------------------------------------------------------------

void Message::begin_reading() const
{
	read_count = 0;
	read_bit = 0;
}

//---------------------------------------------------------------------------------------------

int32_t Message::get_remaing_data() const
{
	cur_size - read_count;
}

//---------------------------------------------------------------------------------------------

void Message::read_byte_align() const
{
	read_bit = 0;
}

//---------------------------------------------------------------------------------------------

int32_t Message::read_bits(int32_t num_bits) const
{
	int32_t		value;
	int32_t		value_bits;
	int32_t		get;
	int32_t		fraction;
	bool		sgn;

	if (!r_data) 
	{
		printf("cannot read from message");
	}

	// check if the number of bits is valid
	if ( num_bits == 0 || num_bits < -31 || num_bits > 32 ) 
	{
		printf("bad number of bits %i", num_bits );
	}

	value = 0;
	value_bits = 0;

	// change sign if it is negative
	if (num_bits < 0) 
	{
		num_bits = -num_bits;
		sgn = true;
	}
	else 
	{
		sgn = false;
	}

	// check for overflow
	if (num_bits > get_remaining_read_bits()) 
	{
		return -1;
	}

	while (value_bits < num_bits) 
	{
		if (read_bit == 0) 
		{
			read_count++;
		}
		
		get = 8 - read_bit;
		
		if (get > (num_bits - value_bits)) 
		{
			get = num_bits - value_bits;
		}
		
		fraction = r_data[read_count - 1];
		fraction >>= read_bit;
		fraction &= (1 << get) - 1;
		value |= fraction << value_bits;

		value_bits += get;
		read_bit = (read_bit + get) & 7;
	}

	if (sgn) 
	{
		if (value & (1 << (num_bits - 1))) 
		{
			value |= -1 ^ (( 1 << num_bits) - 1);
		}
	}

	return value;  
}

//---------------------------------------------------------------------------------------------

int32_t Message::read_int8() const
{
	return (int32_t)read_bits(-8);
}

//---------------------------------------------------------------------------------------------

int32_t Message::read_uint8() const
{
  	return (int32_t)read_bits(8);

}

//---------------------------------------------------------------------------------------------

int32_t Message::read_int16() const
{
	return (int32_t)read_bits(-16);  
}

//---------------------------------------------------------------------------------------------

int32_t Message::read_uint16() const
{
	return (int32_t)read_bits(16);  
}

//---------------------------------------------------------------------------------------------

int32_t Message::read_int32() const
{
	return (int32_t)read_bits(32);
}

//---------------------------------------------------------------------------------------------

real Message::read_real() const
{
	float value;
	*reinterpret_cast<int*>(&value) = read_bits(32);
	return value;  
}

//---------------------------------------------------------------------------------------------

Vec3 Message::read_vec3(int32_t num_bits) const
{
	return bits_to_vec3(read_bits(num_bits), num_bits);
}

//---------------------------------------------------------------------------------------------

int32_t Message::read_string(char* buffer, int32_t buffer_size) const
{
	int	l = 0;
	int c;

	read_byte_align();
	
	while(1) 
	{
		c = read_uint8();
		if (c <= 0 || c >= 255) 
		{
			break;
		}
		// translate all fmt spec to avoid crash bugs in string routines
		if ( c == '%' ) 
		{
			c = '.';
		}

		// we will read past any excessively long string, so
		// the following data can be read, but the string will
		// be truncated
		if (l < buffer_size - 1) 
		{
			buffer[l] = c;
			l++;
		}
	}
	
	buffer[l] = 0;
	return l;  
}

//---------------------------------------------------------------------------------------------

int32_t Message::read_data(void* data, int32_t length) const
{
	int count;

	read_byte_align();
	
	count = read_count;

	if (read_count + length > cur_size) 
	{
		if (data) 
		{
			memcpy(data, r_data + read_count, get_remaing_data());
		}
		read_count = cur_size;
	} 
	else 
	{
		if (data) 
		{
			memcpy(data, r_data + read_count, length);
		}
		read_count += length;
	}

	return (read_count - count);  
}

//---------------------------------------------------------------------------------------------

void Message::read_ipv4addr(os::IPv4Address* addr) const
{

	for (int i = 0; i < 4; i++) 
	{
		addr->address[i] = read_uint8();
	}
	
	addr->port = read_uint16();  
}

//---------------------------------------------------------------------------------------------

int32_t Message::vec3_to_bits(const Vec3& v, int32_t num_bits)
{
	assert(num_bits >= 6 && num_bits <= 32);
	assert(v.squared_length() - 1.0f < 0.01f);
  
	int32_t max; 
	int32_t bits;
	float bias;

	num_bits /= 3;
	max = (1 << (num_bits - 1)) - 1;
	bias = 0.5f / max;

	bits = FLOATSIGNBITSET(v.x) << (num_bits * 3 - 1);
	bits |= ((int32_t)((math::abs(v.x) + bias) * max)) << (num_bits * 2);
	bits |= FLOATSIGNBITSET(v.y) << (num_bits * 2 - 1);
	bits |= ((int32_t)((math::abs(v.y) + bias) * max)) << (num_bits * 1);
	bits |= FLOATSIGNBITSET(v.z) << (num_bits * 1 - 1);
	bits |= ((int32_t)((math::abs(v.z) + bias) * max)) << (num_bits * 0);
	
	return bits;  
}

//---------------------------------------------------------------------------------------------

Vec3 Message::bits_to_vec3(int32_t bits, int32_t num_bits)
{
	assert(num_bits >= 6 && num_bits <= 32);
  
	static float sign[2] = {1.0f, -1.0f};
	int max;
	float inv_max;
	Vec3 v;

	num_bits /= 3;
	max = (1 << (num_bits - 1)) - 1;
	inv_max = 1.0f / max;

	v.x = sign[(bits >> (num_bits * 3 - 1)) & 1] * ((bits >> (num_bits * 2)) & max) * inv_max;
	v.y = sign[(bits >> (num_bits * 2 - 1)) & 1] * ((bits >> (num_bits * 1)) & max) * inv_max;
	v.z = sign[(bits >> (num_bits * 1 - 1)) & 1] * ((bits >> (num_bits * 0)) & max) * inv_max;
	v.normalize();
	
	return v;
}

//---------------------------------------------------------------------------------------------

}	//namespace network
}	//namespace crown
