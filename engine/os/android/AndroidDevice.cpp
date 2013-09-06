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

#include <jni.h>
#include "Device.h"
#include "Renderer.h"

namespace crown
{

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_initDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	const char* argv[] = { "crown-android" };

	device()->init(1, (char**)argv);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_stopDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->stop();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdownDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->shutdown();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pauseDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->pause();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_unpauseDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->unpause();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isDeviceInit(JNIEnv* /*env*/, jobject /*obj*/)
{
	return device()->is_init();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isDeviceRunning(JNIEnv* /*env*/, jobject /*obj*/)
{
	return device()->is_running();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isDevicePaused(JNIEnv* /*env*/, jobject /*obj*/)
{
	return device()->is_paused();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->frame();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_initRenderer(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->renderer()->init();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdownRenderer(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->renderer()->shutdown();
}

} // namespace crown
