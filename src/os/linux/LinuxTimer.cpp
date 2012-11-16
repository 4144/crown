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

#include "Types.h"
#include "Timer.h"
#include <sys/time.h>

namespace crown
{

Timer::Timer()
{
	Reset();
}

Timer::~Timer()
{
}

void Timer::Reset()
{
	clock_gettime(CLOCK_MONOTONIC, &mCreationTime);
}

uint64_t Timer::GetMilliseconds() const
{
	timespec tmp;
	clock_gettime(CLOCK_MONOTONIC, &tmp);
	return (tmp.tv_sec - mCreationTime.tv_sec) * 1000 + (tmp.tv_nsec - mCreationTime.tv_nsec) / 1000000;
}

uint64_t Timer::GetMicroseconds() const
{
	timespec tmp;
	clock_gettime(CLOCK_MONOTONIC, &tmp);
	return (tmp.tv_sec - mCreationTime.tv_sec) * 1000000 + (tmp.tv_nsec - mCreationTime.tv_nsec) / 1000;
}


void Timer::StartMilliseconds()
{
	clock_gettime(CLOCK_MONOTONIC, &mStartTime);
}

uint64_t Timer::StopMilliseconds() const
{
	timespec tmp;
	clock_gettime(CLOCK_MONOTONIC, &tmp);
	return (tmp.tv_sec - mStartTime.tv_sec) * 1000 + (tmp.tv_nsec - mStartTime.tv_nsec) / 1000000;
}

void Timer::StartMicroseconds()
{
	clock_gettime(CLOCK_MONOTONIC, &mStartTime);
}

uint64_t Timer::StopMicroseconds() const
{
	timespec tmp;
	clock_gettime(CLOCK_MONOTONIC, &tmp);
	return (tmp.tv_sec - mStartTime.tv_sec) * 1000000 + (tmp.tv_nsec - mStartTime.tv_nsec) / 1000;
}

} // namespace crown

