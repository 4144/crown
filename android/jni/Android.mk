# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := crown
LOCAL_SRC_FILES :=\
	core/bv/Circle.cpp\
	core/bv/Frustum.cpp\
	core/bv/Rect.cpp\
	core/compressors/ZipCompressor.cpp\
	core/containers/Generic.cpp\
	core/filesystem/BinaryReader.cpp\
	core/filesystem/BinaryWriter.cpp\
	core/filesystem/DiskFile.cpp\
	core/filesystem/File.cpp\
	core/filesystem/Filesystem.cpp\
	core/filesystem/MemoryFile.cpp\
	core/filesystem/TextReader.cpp\
	core/filesystem/TextWriter.cpp\
	core/math/Color4.cpp\
	core/math/Mat3.cpp\
	core/math/Mat4.cpp\
	core/math/MathUtils.cpp\
	core/math/Plane.cpp\
	core/math/Quat.cpp\
	core/math/Shape.cpp\
	core/math/Vec2.cpp\
	core/math/Vec3.cpp\
	core/math/Vec4.cpp\
	core/mem/Allocator.cpp\
	core/mem/HeapAllocator.cpp\
	core/mem/LinearAllocator.cpp\
	core/mem/ProxyAllocator.cpp\
	core/mem/StackAllocator.cpp\
	core/mem/Memory.cpp\
	core/settings/FloatSetting.cpp\
	core/settings/IntSetting.cpp\
	core/settings/StringSetting.cpp\
	core/Args.cpp\
	core/Log.cpp\
\
	input/Accelerometer.cpp\
	input/Keyboard.cpp\
	input/Mouse.cpp\
	input/Touch.cpp\
	input/EventDispatcher.cpp\
	input/InputManager.cpp\
\
	lua/LuaStack.cpp\
	lua/LuaEnvironment.cpp\
\
	network/BitMessage.cpp\
\
	os/OS.cpp\
	os/android/AndroidOS.cpp\
	os/android/AndroidDevice.cpp\
	os/android/OsWindow.cpp\
	os/android/OsFile.cpp\
	os/posix/Thread.cpp\
	os/posix/Mutex.cpp\
	os/posix/Cond.cpp\
	os/posix/TCPSocket.cpp\
	os/posix/UDPSocket.cpp\
\
	renderers/gles/GLESRenderer.cpp\
	renderers/gles/GLESUtils.cpp\
	renderers/gles/egl/GLContext.cpp\
	renderers/DebugRenderer.cpp\
	renderers/PixelFormat.cpp\
	renderers/VertexFormat.cpp\
\
	ArchiveBundle.cpp\
	Camera.cpp\
	Device.cpp\
	FileBundle.cpp\
	FontResource.cpp\
	FPSSystem.cpp\
	JSONParser.cpp\
	MaterialResource.cpp\
	PixelShaderResource.cpp\
	ResourceManager.cpp\
	TextResource.cpp\
	TextureResource.cpp\
	VertexShaderResource.cpp\
	Game.cpp\
\

LOCAL_C_INCLUDES	:=\
	$(LOCAL_PATH)/\
	$(LOCAL_PATH)/core\
	$(LOCAL_PATH)/core/bv\
	$(LOCAL_PATH)/core/compressors\
	$(LOCAL_PATH)/core/containers\
	$(LOCAL_PATH)/core/filesystem\
	$(LOCAL_PATH)/core/math\
	$(LOCAL_PATH)/core/mem\
	$(LOCAL_PATH)/core/filesystem\
	$(LOCAL_PATH)/core/settings\
	$(LOCAL_PATH)/core/strings\
	$(LOCAL_PATH)/input\
	$(LOCAL_PATH)/lua\
	$(LOCAL_PATH)/network\
	$(LOCAL_PATH)/os\
	$(LOCAL_PATH)/os/android\
	$(LOCAL_PATH)/renderers\
	$(LOCAL_PATH)/renderers/gles\
	$(LOCAL_PATH)/renderers/gles/egl\
	$(LOCAL_PATH)/samples\
#	include third/luajit/include
\

LOCAL_CPPFLAGS	:= -g -fexceptions
LOCAL_LDLIBS	:= -llog -landroid -lEGL -lGLESv2 -lz
LOCAL_LDLIBS 	+= $(LOCAL_PATH)/libluajit-5.1.so -lluajit-5.1
LOCAL_SHARED_LIBRARIES := crown luajit-5.1
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := crownlua

LOCAL_SRC_FILES :=\
	lua/LuaStack.cpp\
	lua/LuaEnvironment.cpp\
	lua/AccelerometerBinds.cpp\
	lua/CameraBinds.cpp\
	lua/DeviceBinds.cpp\
	lua/KeyboardBinds.cpp\
	lua/Mat4Binds.cpp\
	lua/MathBinds.cpp\
	lua/MouseBinds.cpp\
	lua/QuatBinds.cpp\
	lua/TouchBinds.cpp\
	lua/Vec2Binds.cpp\
	lua/Vec3Binds.cpp\
	Game.cpp\

\

LOCAL_C_INCLUDES	:=\
	$(LOCAL_PATH)\
	$(LOCAL_PATH)/lua\
#	include third/luajit/include
\

LOCAL_CPPFLAGS	:= -g -fexceptions -I
LOCAL_LDLIBS	:= -llog -landroid
LOCAL_LDLIBS 	+= $(LOCAL_PATH)/libluajit-5.1.so -lluajit-5.1
LOCAL_SHARED_LIBRARIES := crown luajit-5.1
include $(BUILD_SHARED_LIBRARY)