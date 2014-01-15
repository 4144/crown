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

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "Types.h"
#include "NetAddress.h"
#include "Assert.h"
#include "OS.h"

namespace crown
{

struct ReadResult
{
	enum { NO_ERROR, UNKNOWN, REMOTE_CLOSED } error;
	size_t bytes_read;
};

struct WriteResult
{
	enum { NO_ERROR, UNKNOWN, REMOTE_CLOSED } error;
	size_t bytes_wrote;
};

struct AcceptResult
{
	enum { NO_ERROR, NO_CONNECTION, UNKNOWN } error;
};

class TCPSocket
{
public:

	//-----------------------------------------------------------------------------
	TCPSocket()
		: m_socket(0)
	{
	}

	//-----------------------------------------------------------------------------
	TCPSocket(int socket)
		: m_socket(socket)
	{
	}

	//-----------------------------------------------------------------------------
	bool connect(const NetAddress& destination, uint16_t port)
	{
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CE_ASSERT(m_socket > 0, "Failed to create socket");

		sockaddr_in addr_in;
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.s_addr = htonl(destination.address());
		addr_in.sin_port = htons(port);

		if (::connect(m_socket, (const sockaddr*)&addr_in, sizeof(sockaddr_in)) < 0)
		{
			os::printf("Failed to connect socket\n");
			close();

			return false;
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	void close()
	{
		if (m_socket != 0)
		{
			::close(m_socket);
			m_socket = 0;
		}
	}

	//-----------------------------------------------------------------------------
	ReadResult read_nonblock(void* data, size_t size)
	{
		set_blocking(false);
		ssize_t read_bytes = ::read(m_socket, (char*) data, size);

		ReadResult result;
		if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			result.error = ReadResult::NO_ERROR;
			result.bytes_read = 0;
		}
		else if (read_bytes == 0)
		{
			result.error = ReadResult::REMOTE_CLOSED;
		}
		else
		{
			result.error = ReadResult::NO_ERROR;
			result.bytes_read = read_bytes;
		}

		return result;		
	}

	//-----------------------------------------------------------------------------
	ReadResult read(void* data, size_t size)
	{
		set_blocking(true);

		// Ensure all data is read
		char* buf = (char*) data;
		size_t to_read = size;
		ReadResult result;
		result.bytes_read = 0;
		result.error = ReadResult::NO_ERROR;

		while (to_read > 0)
		{
			ssize_t read_bytes = ::read(m_socket, buf, to_read);
			
			if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
			else if (read_bytes == 0)
			{
				result.error = ReadResult::REMOTE_CLOSED;
				return result;
			}

			buf += read_bytes;
			to_read -= read_bytes;
			result.bytes_read += read_bytes;
		}

		result.error = ReadResult::NO_ERROR;
		return result;
	}

	//-----------------------------------------------------------------------------
	WriteResult write_nonblock(const void* data, size_t size)
	{
		set_blocking(false);
		ssize_t bytes_wrote = ::send(m_socket, data, size, 0);

		WriteResult result;
		if (bytes_wrote == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			result.error = WriteResult::NO_ERROR;
			result.bytes_wrote = 0;
		}
		else if (bytes_wrote == 0)
		{
			result.error = WriteResult::REMOTE_CLOSED;
		}
		else
		{
			result.error = WriteResult::UNKNOWN;
		}

		return result;
	}

	//-----------------------------------------------------------------------------
	WriteResult write(const void* data, size_t size)
	{
		set_blocking(true);

		const char* buf = (const char*) data;
		size_t to_send = size;
		WriteResult result;
		result.bytes_wrote = 0;
		result.error = WriteResult::NO_ERROR;

		// Ensure all data is sent
		while (to_send > 0)
		{
			ssize_t bytes_wrote = ::send(m_socket, (const char*) buf, to_send, 0);

			// Check for errors
			if (bytes_wrote == -1)
			{
				switch (errno)
				{
					case EAGAIN:
					{
						continue;
					}
					default:
					{
						result.error = WriteResult::UNKNOWN;
						return result;
					}
				}
			}

			buf += bytes_wrote;
			to_send -= bytes_wrote;
			result.bytes_wrote += bytes_wrote;
		}

		result.error = WriteResult::NO_ERROR;
		return result;
	}

	//-----------------------------------------------------------------------------
	void set_blocking(bool blocking)
	{
		int flags = fcntl(m_socket, F_GETFL, 0);
		fcntl(m_socket, F_SETFL, blocking ? (flags & ~O_NONBLOCK) : O_NONBLOCK);
	}

	//-----------------------------------------------------------------------------
	void set_resuse_address(bool reuse)
	{
		int optval = (int) reuse;
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	}

public:

	int m_socket;
};

class TCPServer
{
public:

	//-----------------------------------------------------------------------------
	bool open(uint16_t port)
	{
		m_server.m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CE_ASSERT(m_server.m_socket > 0, "Failed to create socket");

		m_server.set_resuse_address(true);

		// Bind socket
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);

		int bind_ret = bind(m_server.m_socket, (const sockaddr*) &address, sizeof(sockaddr_in));
		CE_ASSERT(bind_ret != -1, "Failed to bind socket: errno: %d", errno);

		return true;
	}

	//-----------------------------------------------------------------------------
	void close()
	{
		m_server.close();
	}

	//-----------------------------------------------------------------------------
	void listen(uint32_t max)
	{
		int listen_ret = ::listen(m_server.m_socket, max);
		CE_ASSERT(listen_ret != -1, "Failed to listen on socket: errno: %d", errno);
	}

	//-----------------------------------------------------------------------------
	AcceptResult accept_nonblock(TCPSocket& c)
	{
		m_server.set_blocking(false);

		sockaddr_in client;
		size_t client_size = sizeof(client);
		int sock = ::accept(m_server.m_socket, (sockaddr*) &client, (socklen_t*) &client_size);

		AcceptResult result;
		if (sock == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			result.error = AcceptResult::NO_CONNECTION;
		}
		else if (sock == -1)
		{
			result.error = AcceptResult::UNKNOWN;
		}
		else
		{
			result.error = AcceptResult::NO_ERROR;
			c.m_socket = sock;
		}

		return result;
	}

	//-----------------------------------------------------------------------------
	AcceptResult accept(TCPSocket& c)
	{
		m_server.set_blocking(true);

		sockaddr_in client;
		size_t client_size = sizeof(client);

		int sock = ::accept(m_server.m_socket, (sockaddr*) &client, (socklen_t*) &client_size);

		AcceptResult result;
		if (sock == -1)
		{
			result.error = AcceptResult::UNKNOWN;
		}
		else
		{
			result.error = AcceptResult::NO_ERROR;
			c.m_socket = sock;
		}

		return result;
	}

	//-----------------------------------------------------------------------------
	ReadResult read_nonblock(void* data, size_t size)
	{
		return m_server.read_nonblock(data, size);
	}

	//-----------------------------------------------------------------------------
	ReadResult read(void* data, size_t size)
	{
		return m_server.read(data, size);
	}

	//-----------------------------------------------------------------------------
	WriteResult write_nonblock(void* data, size_t size)
	{
		return m_server.write_nonblock(data, size);
	}

	//-----------------------------------------------------------------------------
	WriteResult write(const void* data, size_t size)
	{
		return m_server.write(data, size);
	}

private:

	TCPSocket m_server;
};

class UDPSocket
{
public:

	//-----------------------------------------------------------------------------
	UDPSocket()
		: m_socket(0)
	{
	}

	//-----------------------------------------------------------------------------
	bool open(uint16_t port)
	{
		CE_ASSERT(!is_open(), "Socket is already open");

		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (m_socket <= 0)
		{
			os::printf("Failed to create socket.\n");
			m_socket = 0;

			return false;
		}

		// Bind to port
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		if (bind(m_socket, (const sockaddr*) &address, sizeof(sockaddr_in)) < 0)
		{
			os::printf("Failed to bind socket\n");
			close();

			return false;
		}

		if (fcntl(m_socket, F_SETFL, O_NONBLOCK, 1) == -1)
		{
			os::printf("Failed to set non-blocking socket\n");
			close();

			return false;
		}
		
		return true;
	}

	//-----------------------------------------------------------------------------
	void close()
	{
		if (m_socket != 0)
		{
			::close(m_socket);
			m_socket = 0;
		}
	}

	//-----------------------------------------------------------------------------
	ReadResult read(NetAddress& sender, uint16_t& port, const void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		sockaddr_in from;
		socklen_t from_length = sizeof(from);

		ssize_t received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &from_length);

		ReadResult result;

		if (received_bytes == -1 && errno == EAGAIN)
		{
			result.error = ReadResult::NO_ERROR;
			result.bytes_read = 0;
		}
		else if (received_bytes == 0)
		{
			result.error = ReadResult::REMOTE_CLOSED;
		}
		else
		{
			result.error = ReadResult::NO_ERROR;
			result.bytes_read = received_bytes;
		}

		sender.set(ntohl(from.sin_addr.s_addr));
		port = ntohs(from.sin_port);

		return result;
	}

	//-----------------------------------------------------------------------------
	WriteResult write(const NetAddress& receiver, uint16_t port, void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(receiver.address());
		address.sin_port = htons(port);

		ssize_t bytes_wrote = sendto(m_socket, (const char*) data, size, 0, (sockaddr*) &address, sizeof(sockaddr_in));

		WriteResult result;

		if (bytes_wrote < 0)
		{
			result.error = WriteResult::UNKNOWN;
			return result;
		}

		result.error = WriteResult::NO_ERROR;
		result.bytes_wrote = bytes_wrote;
		return result;
	}

	//-----------------------------------------------------------------------------
	bool is_open()
	{
		return m_socket != 0;
	}

public:

	int m_socket;
};

} // namespace crown
