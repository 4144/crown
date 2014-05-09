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

#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
	#include "Device.h"
	#include "ConsoleServer.h"
	#define CE_LOGI(msg, ...) crown::device()->console()->log_to_all(crown::LogSeverity::INFO, msg, ##__VA_ARGS__)
	#define CE_LOGD(msg, ...) crown::device()->console()->log_to_all(crown::LogSeverity::DEBUG, msg, ##__VA_ARGS__)
	#define CE_LOGE(msg, ...) crown::device()->console()->log_to_all(crown::LogSeverity::ERROR, msg, ##__VA_ARGS__)
	#define CE_LOGW(msg, ...) crown::device()->console()->log_to_all(crown::LogSeverity::WARN, msg, ##__VA_ARGS__)
#else
	#define CE_LOGI(msg, ...) ((void)0)
	#define CE_LOGD(msg, ...) ((void)0)
	#define CE_LOGE(msg, ...) ((void)0)
	#define CE_LOGW(msg, ...) ((void)0)
#endif
