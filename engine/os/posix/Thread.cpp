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

#include <stdlib.h>
#include <string.h>

#include "Thread.h"
#include "Assert.h"

namespace crown
{

//-----------------------------------------------------------------------------
Thread::Thread(const char* name) :
	m_name(name),
	m_is_running(false),
	m_is_terminating(false),
	m_thread(0)
{
	memset(&m_thread, 0, sizeof(pthread_t));
}

//-----------------------------------------------------------------------------
Thread::~Thread()
{
}

//-----------------------------------------------------------------------------
const char* Thread::name() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
void Thread::start()
{
	m_is_terminating = false;

	// Make thread joinable
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// Create thread
	int rc = pthread_create(&m_thread, &attr, Thread::background_proc, (void*) this);
	CE_ASSERT(rc == 0, "Failed to create the thread '%s': errno: %d", m_name, rc);

	// Free attr memory
	pthread_attr_destroy(&attr);

	m_is_running = true;
}

//-----------------------------------------------------------------------------
bool Thread::is_running() const
{
	return m_is_running;
}

//-----------------------------------------------------------------------------
bool Thread::is_terminating() const
{
	return m_is_terminating;
}

//-----------------------------------------------------------------------------
void Thread::stop()
{
	m_is_terminating = true;
}

//-----------------------------------------------------------------------------
int32_t Thread::run()
{
	return 0;
}

//-----------------------------------------------------------------------------
void* Thread::background_proc(void* thiz)
{
	Thread* thread = ((Thread*) thiz);

	thread->run();

	thread->m_is_running = false;

	return NULL;
}

//-----------------------------------------------------------------------------
void Thread::join()
{
	pthread_join(m_thread, NULL);
}

//-----------------------------------------------------------------------------
void Thread::detach()
{
	pthread_detach(m_thread);
}

} // namespace crown
