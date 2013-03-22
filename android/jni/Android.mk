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
	core/containers/Generic.cpp\
	core/containers/Str.cpp\
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
	core/streams/FileStream.cpp\
	core/streams/MemoryStream.cpp\
	core/streams/Stream.cpp\
\
	input/EventDispatcher.cpp\
	input/InputManager.cpp\
\
	loaders/BMPImageLoader.cpp\
	loaders/TGAImageLoader.cpp\
\
	Filesystem.cpp\
\
	renderers/gles/GLESIndexBuffer.cpp\
	renderers/gles/GLESRenderer.cpp\
	renderers/gles/GLESSupport.cpp\
	renderers/gles/GLESTextRenderer.cpp\
	renderers/gles/GLESTexture.cpp\
	renderers/gles/GLESTextureManager.cpp\
	renderers/gles/GLESVertexBuffer.cpp\
\
	App.cpp\
	Camera.cpp\
	Device.cpp\
	Entity.cpp\
	Font.cpp\
	FontManager.cpp\
	Frame.cpp\
	Image.cpp\
	ImageLoader.cpp\
	Light.cpp\
	Log.cpp\
	LogManager.cpp\
	Material.cpp\
	MaterialManager.cpp\
	MeshChunk.cpp\
	Mesh.cpp\
	MeshManager.cpp\
	MovableCamera.cpp\
	PhysicNode.cpp\
	PhysicsManager.cpp\
	Pixel.cpp\
	Renderer.cpp\
	RenderWindow.cpp\
	ResourceManager.cpp\
	Scene.cpp\
	SceneManager.cpp\
	SceneNode.cpp\
	Skybox.cpp\
	SpriteAnimator.cpp\
	Sprite.cpp\
	Timer.cpp\
\

LOCAL_C_INCLUDES	:=\
	$(LOCAL_PATH)/core\
	$(LOCAL_PATH)/core/math\
	$(LOCAL_PATH)/core/compressors\
	$(LOCAL_PATH)/core/containers\
	$(LOCAL_PATH)/core/bv\
	$(LOCAL_PATH)/core/mem\
	$(LOCAL_PATH)/core/streams\
	$(LOCAL_PATH)/loaders\
	$(LOCAL_PATH)/renderers\
	$(LOCAL_PATH)/things\
	$(LOCAL_PATH)/filesystem\
	$(LOCAL_PATH)/gui\
	$(LOCAL_PATH)/windowing\
	$(LOCAL_PATH)/windowing/themes\
	$(LOCAL_PATH)/windowing/layouts\
	$(LOCAL_PATH)/windowing/templates\
	$(LOCAL_PATH)/windowing/toolbox\
	$(LOCAL_PATH)/renderers/gl\
	$(LOCAL_PATH)/renderers/gl/glx\
	$(LOCAL_PATH)/renderers/gl/wgl\
	$(LOCAL_PATH)/renderers/gles\
	$(LOCAL_PATH)/renderers/gles/egl\
	$(LOCAL_PATH)/input\
	$(LOCAL_PATH)/os\
	$(LOCAL_PATH)/os/android\

LOCAL_CPPFLAGS	:= -g -fexceptions
LOCAL_LDLIBS	:= -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

#LOCAL_MODULE    := simple
#LOCAL_SRC_FILES :=	tests/chainsawbuffet/maain.cpp\
#					tests/chainsawbuffet/Globals.cpp\
#					tests/chainsawbuffet/entities/Pg.cpp\
#					tests/chainsawbuffet/entities/Zombie.cpp\
#					tests/chainsawbuffet/entities/SolidSceneNode.cpp\
#					tests/chainsawbuffet/entities/Joystick.cpp\
#					tests/chainsawbuffet/scenes/ArenaScene.cpp\
#					tests/chainsawbuffet/entities/Bullet.cpp
#LOCAL_SHARED_LIBRARIES := crown
#LOCAL_C_INCLUDES	:=\
#	$(LOCAL_PATH)/core\
#	$(LOCAL_PATH)/core/math\
#	$(LOCAL_PATH)/core/containers\
#	$(LOCAL_PATH)/core/bv\
#	$(LOCAL_PATH)/core/mem\
#	$(LOCAL_PATH)/core/streams\
#	$(LOCAL_PATH)/loaders\
#	$(LOCAL_PATH)/renderers\
#	$(LOCAL_PATH)/things\
#	$(LOCAL_PATH)/filesystem\
#	$(LOCAL_PATH)/gui\
#	$(LOCAL_PATH)/windowing\
#	$(LOCAL_PATH)/windowing/themes\
#	$(LOCAL_PATH)/windowing/layouts\
#	$(LOCAL_PATH)/windowing/templates\
#	$(LOCAL_PATH)/windowing/toolbox\
#	$(LOCAL_PATH)/renderers/gl\
#	$(LOCAL_PATH)/renderers/gl/glx\
#	$(LOCAL_PATH)/renderers/gl/wgl\
#	$(LOCAL_PATH)/renderers/gles\
#	$(LOCAL_PATH)/renderers/gles/egl\
#	$(LOCAL_PATH)/input\
#	$(LOCAL_PATH)/input/android\
#	$(LOCAL_PATH)/tests\
#	$(LOCAL_PATH)/tests/chainsawbuffet\
#	$(LOCAL_PATH)/tests/chainsawbuffet/entities\
#	$(LOCAL_PATH)/tests/chainsawbuffet/scenes\

#LOCAL_CPPFLAGS	:= -g -fexceptions
#LOCAL_LDLIBS	:= -llog -landroid -lEGL -lGLESv1_CM -lz
#LOCAL_STATIC_LIBRARIES := android_native_app_glue
#include $(BUILD_SHARED_LIBRARY)
#$(call import-module,android/native_app_glue)

