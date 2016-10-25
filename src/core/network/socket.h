/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "ip_address.h"
#include "platform.h"
#include "types.h"

#if CROWN_PLATFORM_POSIX
	#include <errno.h>
	#include <fcntl.h>      // fcntl
	#include <netinet/in.h> // htons, htonl, ...
	#include <sys/socket.h>
	#include <unistd.h>     // close
	typedef int SOCKET;
#elif CROWN_PLATFORM_WINDOWS
	#include <winsock2.h>
	#pragma comment(lib, "Ws2_32.lib")
#endif

namespace crown
{
struct ConnectResult
{
	enum
	{
		SUCCESS,
		BAD_SOCKET,
		REFUSED,
		TIMEOUT,
		UNKNOWN
	} error;
};

struct BindResult
{
	enum
	{
		SUCCESS,
		BAD_SOCKET,
		ADDRESS_IN_USE,
		UNKNOWN
	} error;
};

struct AcceptResult
{
	enum
	{
		SUCCESS,
		BAD_SOCKET,
		NO_CONNECTION,
		UNKNOWN
	} error;
};

struct ReadResult
{
	enum
	{
		SUCCESS,
		WOULDBLOCK,
		BAD_SOCKET,
		REMOTE_CLOSED,
		TIMEOUT,
		UNKNOWN
	} error;
	u32 bytes_read;
};

struct WriteResult
{
	enum
	{
		SUCCESS,
		WOULDBLOCK,
		BAD_SOCKET,
		REMOTE_CLOSED,
		TIMEOUT,
		UNKNOWN
	} error;
	u32 bytes_wrote;
};

/// TCP socket
///
/// @ingroup Network
struct TCPSocket
{
	SOCKET _socket;

	TCPSocket()
#if CROWN_PLATFORM_POSIX
		: _socket(0)
#elif CROWN_PLATFORM_WINDOWS
		: _socket(INVALID_SOCKET)
#endif
	{
	}

	void open()
	{
		_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if CROWN_PLATFORM_POSIX
		CE_ASSERT(_socket >= 0, "socket: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		CE_ASSERT(_socket >= 0, "socket: WSAGetLastError = %d", WSAGetLastError());
#endif
	}

	void close()
	{
#if CROWN_PLATFORM_POSIX
		if (_socket != 0)
		{
			::close(_socket);
			_socket = 0;
		}
#elif CROWN_PLATFORM_WINDOWS
		if (_socket != INVALID_SOCKET)
		{
			::closesocket(_socket);
			_socket = INVALID_SOCKET;
		}
#endif
	}

	/// Connects to the @a ip address and @a port and returns the result.
	ConnectResult connect(const IPAddress& ip, u16 port)
	{
		close();
		open();

		sockaddr_in addr_in;
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.s_addr = htonl(ip.address());
		addr_in.sin_port = htons(port);

		int err = ::connect(_socket, (const sockaddr*)&addr_in, sizeof(sockaddr_in));

		ConnectResult cr;
		cr.error = ConnectResult::SUCCESS;

		if (err == 0)
			return cr;
#if CROWN_PLATFORM_POSIX
		if (errno == ECONNREFUSED)
			cr.error = ConnectResult::REFUSED;
		else if (errno == ETIMEDOUT)
			cr.error = ConnectResult::TIMEOUT;
		else
			cr.error = ConnectResult::UNKNOWN;
#elif CROWN_PLATFORM_WINDOWS
		int wsaerr = WSAGetLastError();
		if (wsaerr == WSAECONNREFUSED)
			cr.error = ConnectResult::REFUSED;
		else if (wsaerr == WSAETIMEDOUT)
			cr.error = ConnectResult::TIMEOUT;
		else
			cr.error = ConnectResult::UNKNOWN;
#endif
		return cr;
	}

	/// Binds the socket to @a port and returns the result.
	BindResult bind(u16 port)
	{
		close();
		open();
		set_reuse_address(true);

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);

		int err = ::bind(_socket, (const sockaddr*)&address, sizeof(sockaddr_in));

		BindResult br;
		br.error = BindResult::SUCCESS;

		if (err == 0)
			return br;
#if CROWN_PLATFORM_POSIX
		if (errno == EBADF)
			br.error = BindResult::BAD_SOCKET;
		else if (errno == EADDRINUSE)
			br.error = BindResult::ADDRESS_IN_USE;
		else
			br.error = BindResult::UNKNOWN;
#elif CROWN_PLATFORM_WINDOWS
		int wsaerr = WSAGetLastError();
		if (wsaerr == WSAEADDRINUSE)
			br.error = BindResult::ADDRESS_IN_USE;
		else
			br.error = BindResult::UNKNOWN;
#endif
		return br;
	}

	/// Listens for @a max socket connections.
	void listen(u32 max)
	{
		int err = ::listen(_socket, max);
#if CROWN_PLATFORM_POSIX
		CE_ASSERT(err == 0, "listen: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		CE_ASSERT(err == 0, "listen: WSAGetLastError = %d", WSAGetLastError());
#endif
		CE_UNUSED(err);
	}

	AcceptResult accept_internal(TCPSocket& c)
	{
		SOCKET err = ::accept(_socket, NULL, NULL);

		AcceptResult ar;
		ar.error = AcceptResult::SUCCESS;

#if CROWN_PLATFORM_POSIX
		if (err >= 0)
			c._socket = err;
		else if (err == -1 && errno == EBADF)
			ar.error = AcceptResult::BAD_SOCKET;
		else if (err == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			ar.error = AcceptResult::NO_CONNECTION;
		else
			ar.error = AcceptResult::UNKNOWN;
#elif CROWN_PLATFORM_WINDOWS
		if (err != INVALID_SOCKET)
		{
			c._socket = err;
			return ar;
		}

		int wsaerr = WSAGetLastError();
		if (wsaerr == WSAEWOULDBLOCK)
			ar.error = AcceptResult::NO_CONNECTION;
		else
			ar.error = AcceptResult::UNKNOWN;
#endif
		return ar;
	}

	/// Accepts a new connection @a c.
	AcceptResult accept_nonblock(TCPSocket& c)
	{
		set_blocking(false);
		return accept_internal(c);
	}

	/// Accepts a new connection @a c.
	AcceptResult accept(TCPSocket& c)
	{
		set_blocking(true);
		return accept_internal(c);
	}

	ReadResult read_internal(void* data, u32 size)
	{
		ReadResult rr;
		rr.error = ReadResult::SUCCESS;
		rr.bytes_read = 0;

		char* buf = (char*)data;
		u32 to_read = size;

		while (to_read > 0)
		{
#if CROWN_PLATFORM_POSIX
			ssize_t read_bytes = ::recv(_socket, buf, to_read, 0);

			if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				rr.error = ReadResult::WOULDBLOCK;
				return rr;
			}
			else if (read_bytes == -1 && errno == ETIMEDOUT)
			{
				rr.error = ReadResult::TIMEOUT;
				return rr;
			}
			else if (read_bytes == 0)
			{
				rr.error = ReadResult::REMOTE_CLOSED;
				return rr;
			}
#elif CROWN_PLATFORM_WINDOWS
			int read_bytes = ::recv(_socket, buf, (int)to_read, 0);

			if (read_bytes == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
			{
				rr.error = ReadResult::WOULDBLOCK;
				return rr;
			}
			else if (read_bytes == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
			{
				rr.error = ReadResult::TIMEOUT;
				return rr;
			}
			else if (read_bytes == 0)
			{
				rr.error = ReadResult::REMOTE_CLOSED;
				return rr;
			}
#endif
			buf += read_bytes;
			to_read -= read_bytes;
			rr.bytes_read += read_bytes;
		}

		return rr;
	}

	/// Reads @a size bytes and returns the result.
	ReadResult read_nonblock(void* data, u32 size)
	{
		set_blocking(false);
		return read_internal(data, size);
	}

	/// Reads @a size bytes and returns the result.
	ReadResult read(void* data, u32 size)
	{
		set_blocking(true);
		return read_internal(data, size);
	}

	WriteResult write_internal(const void* data, u32 size)
	{
		WriteResult wr;
		wr.error = WriteResult::SUCCESS;
		wr.bytes_wrote = 0;

		const char* buf = (const char*)data;
		u32 to_send = size;

		while (to_send > 0)
		{
#if CROWN_PLATFORM_POSIX
			ssize_t bytes_wrote = ::send(_socket, buf, to_send, 0);

			if (bytes_wrote == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				wr.error = WriteResult::WOULDBLOCK;
				return wr;
			}
			else if (bytes_wrote == -1 && errno == ETIMEDOUT)
			{
				wr.error = WriteResult::TIMEOUT;
				return wr;
			}
			else if (bytes_wrote == 0)
			{
				wr.error = WriteResult::REMOTE_CLOSED;
				return wr;
			}
			else
			{
				wr.error = WriteResult::UNKNOWN;
				return wr;
			}
#elif CROWN_PLATFORM_WINDOWS
			int bytes_wrote = ::send(_socket, buf, (int)to_send, 0);

			if (bytes_wrote == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
			{
				wr.error = WriteResult::WOULDBLOCK;
				return wr;
			}
			else if (bytes_wrote == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
			{
				wr.error = WriteResult::TIMEOUT;
				return wr;
			}
			else if (bytes_wrote == 0)
			{
				wr.error = WriteResult::REMOTE_CLOSED;
				return wr;
			}
			else
			{
				wr.error = WriteResult::UNKNOWN;
				return wr;
			}
#endif
			buf += bytes_wrote;
			to_send -= bytes_wrote;
			wr.bytes_wrote += bytes_wrote;
		}
		return wr;
	}

	/// Writes @a size bytes and returns the result.
	WriteResult write_nonblock(const void* data, u32 size)
	{
		set_blocking(false);
		return write_internal(data, size);
	}

	/// Writes @a size bytes and returns the result.
	WriteResult write(const void* data, u32 size)
	{
		set_blocking(true);
		return write_internal(data, size);
	}

	/// Sets whether the socket is @a blocking.
	void set_blocking(bool blocking)
	{
#if CROWN_PLATFORM_POSIX
		int flags = fcntl(_socket, F_GETFL, 0);
		fcntl(_socket, F_SETFL, blocking ? (flags & ~O_NONBLOCK) : O_NONBLOCK);
#elif CROWN_PLATFORM_WINDOWS
		u_long non_blocking = blocking ? 0 : 1;
		ioctlsocket(_socket, FIONBIO, &non_blocking);
#endif
	}

	/// Sets whether the socket should @a reuse a busy port.
	void set_reuse_address(bool reuse)
	{
		int optval = (int)reuse;
		int err = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
#if CROWN_PLATFORM_POSIX
		CE_ASSERT(err == 0, "setsockopt: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		CE_ASSERT(err == 0, "setsockopt: WSAGetLastError = %d", WSAGetLastError());
#endif
		CE_UNUSED(err);
	}

	/// Sets the timeout to the given @a seconds.
	void set_timeout(u32 seconds)
	{
		struct timeval timeout;
		timeout.tv_sec = seconds;
		timeout.tv_usec = 0;
#if CROWN_PLATFORM_POSIX
		int err = setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		CE_ASSERT(err == 0, "setsockopt: errno: %d", errno);
		err = setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		CE_ASSERT(err == 0, "setsockopt: errno: %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		int err = setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		CE_ASSERT(err == 0, "setsockopt: WSAGetLastError: %d", WSAGetLastError());
		err = setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		CE_ASSERT(err == 0, "setsockopt: WSAGetLastError: %d", WSAGetLastError());
#endif
		CE_UNUSED(err);
	}
};

} // namespace crown
