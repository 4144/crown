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

#include "Log.h"
#include "Types.h"
#include "OS.h"
#include <EGL/egl.h>
//#include <android_native_app_glue.h>
#include <jni.h>

namespace crown
{
namespace os
{

EGLint w;
EGLint h;
EGLDisplay display;
EGLSurface surface;
EGLContext context;
//android_app* application;

bool create_render_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool fullscreen)
{
//	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

//	assert(display != EGL_NO_DISPLAY);

//	const EGLint attribs[] =
//	{
//		EGL_BUFFER_SIZE, 24,
//		EGL_DEPTH_SIZE, 24,
//		EGL_SURFACE_TYPE, 
//		EGL_WINDOW_BIT,
//		EGL_RENDERABLE_TYPE, 
//		EGL_OPENGL_ES_BIT,
//		EGL_NONE
//	};
//	
//	EGLint major;
//	EGLint minor;

//	assert(eglInitialize(display, &major, &minor));

//	EGLConfig  config;
//	EGLint     num_config;
//	assert(eglChooseConfig(display, attribs, &config, 1, &num_config));

//	EGLint format;
//    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

//	// Reconfigure ANativeWindow buffer
//    ANativeWindow_setBuffersGeometry(application->window, 0, 0, format);

//	surface = eglCreateWindowSurface(display, config, application->window, NULL);
//	assert(surface != EGL_NO_SURFACE);


//	EGLint ctxattr[] =
//	{
//		EGL_CONTEXT_CLIENT_VERSION, 
//		1,
//		EGL_NONE
//	};

//	context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxattr);
//	assert(context != EGL_NO_CONTEXT);

//	assert(eglMakeCurrent(display, surface, surface, context) != EGL_NO_CONTEXT);

//	eglQuerySurface(display, surface, EGL_WIDTH, &w);
//    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

//	return true;
}

bool destroy_render_window()
{
//	if (display != EGL_NO_DISPLAY)
//	{
//		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

//		if (context != EGL_NO_CONTEXT)
//		{
//            eglDestroyContext(display, context);
//		}
//		
//		if (surface != EGL_NO_SURFACE)
//		{
//            eglDestroySurface(display, surface);
//		}

//		eglTerminate(display);
//	}
}

void get_render_window_metrics(uint32_t& width, uint32_t& height)
{
//	width = w;
//	height = h;
}

void swap_buffers()
{
//	eglSwapBuffers(display, surface);
}

void event_loop()
{
	// FIXME
}




/* 
tmp JNI definitions, just for testing
TODO: remove this methods
*/
//extern "C"
//{
//	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_create(JNIEnv* env, jobject obj);
//	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_destroy(JNIEnv* env, jobject obj);
//}

//JNIEXPORT bool JNICALL Java_crown_android_CrownLib_create(JNIEnv* env, jobject obj, jint x, jint y, jint width, jint height, jboolean fullscreen)
//{
//	create_render_window(x, y, width, height, fullscreen);
//}

//JNIEXPORT bool JNICALL Java_crown_android_CrownLib_destroy(JNIEnv* env, jobject obj)
//{
//	destroy_render_window();
//}


} // namespace os
} // namespace crown

