# Copyright (c) 2013 Daniele Bartolini, Michele Rossi
# Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

require 'optparse'
require 'ostruct'
require 'fileutils'

$config_h = 
"
#define CROWN_VERSION_MAJOR 0
#define CROWN_VERSION_MINOR 1
#define CROWN_VERSION_MICRO 13

#define PRId64 \"lld\"
#define PRIu64 \"llu\"
#define PRIi64 \"lli\"
#define PRIx64 \"llx\"

#define CE_PIXELS_PER_METER					32

#define CE_MAX_TEXTURE_UNITS				8
#define CE_MAX_TEXTURES						32
#define CE_MAX_RENDER_TARGETS				32
#define CE_MAX_VERTEX_BUFFERS				1024
#define CE_MAX_INDEX_BUFFERS				1024
#define CE_MAX_SHADERS						512
#define CE_MAX_GPU_PROGRAMS					512
#define CE_MAX_UNIFORMS						128
#define CE_TRANSIENT_VERTEX_BUFFER_SIZE		6 * 1024 * 1024		// In bytes
#define CE_TRANSIENT_INDEX_BUFFER_SIZE		2 * 1024 * 1024		// In bytes
#define CE_MAX_UNIFORM_NAME_LENGTH			64					// Including NUL character

#define CE_MAX_WORLDS						1024
#define CE_MAX_UNITS						65000				// Per world
#define CE_MAX_CAMERAS						16					// Per world
#define CE_MAX_ACTORS						1024				// Per world
#define CE_MAX_CONTROLLERS					16					// Per world
#define CE_MAX_TRIGGERS						1024				// Per world
#define CE_MAX_JOINTS						512					// Per world
#define CE_MAX_SOUND_INSTANCES				64					// Per world
#define CE_MAX_RAYCASTS						8					// Per World
#define CE_MAX_RAY_INTERSECTIONS			16

#define CE_MAX_CAMERA_COMPONENTS			16					// Per unit
#define CE_MAX_MESH_COMPONENTS				16					// Per unit
#define CE_MAX_SPRITE_COMPONENTS			16					// Per unit
#define CE_MAX_ACTOR_COMPONENTS				16					// Per unit
#define CE_MAX_MATERIAL_COMPONENTS			16					// Per unit				

#define CE_MAX_CONSOLE_CLIENTS				32

#define CE_MAX_GUI_RECTS 					64					// Per Gui
#define CE_MAX_GUI_TRIANGLES 				64					// Per Gui
#define CE_MAX_GUI_IMAGES 					64					// Per Gui
#define CE_MAX_GUI_TEXTS 					64					// Per Gui

#define CE_MAX_DEBUG_LINES					2 * 1024			// Per DebugLine

#define CE_MAX_LUA_VECTOR2					4096
#define CE_MAX_LUA_VECTOR3					4096
#define CE_MAX_LUA_MATRIX4X4				4096
#define CE_MAX_LUA_QUATERNION				4096
"

$application_mk =
"
APP_PLATFORM := android-10
APP_STL := gnustl_static
APP_ABI := armeabi-v7a
"

# Commands
$android_create 	= "android create project"
$android_update 	= "android update project"
$activity			= "CrownActivity"
$package			= "crown.android"

# Paths
$engine_src 		= "../engine/."
$android_src		= "../engine/os/android/java/."
$android_manifest	= "../engine/os/android/AndroidManifest.xml"
$luajit				= "../third/ARMv7/luajit"
$oggvorbis			= "../third/ARMv7/oggvorbis"
$physx				= "../third/ARMv7/physx"

#------------------------------------------------------------------------------
def validate_command_line(args)

	if args.length < 8
		return false 
	end
	if args[0] != "--build"
		return false
	end
	if args[2] != "--target"
		return false
	end
	if args[4] != "--name"
		return false
	end
	if args[6] != "--path"
		return false
	end

	return true
end

#------------------------------------------------------------------------------
def parse_command_line(args)

	banner = "Usage: crown-android.rb --build BUILD --target TARGET --name NAME --path PATH [--res RES]\n"

	if not validate_command_line(args)
		print banner
		exit
	end

	options = OpenStruct.new

	OptionParser.new do |opts|
		opts.banner = banner

		opts.on("-b", "--build BUILD", "Crown build") do |b|
			options.build = b
		end

		opts.on("-t", "--target TARGET", "Android target") do |t|
			options.target = t
		end

		opts.on("-n", "--name NAME", "Android project name") do |n|
			options.name = n
		end

		opts.on("-p", "--path PATH", "Android project path") do |p|
			options.path = p
		end

		opts.on("-r", "--res RES", "Android project compiled resources") do |r|
			options.res = r
		end

	    opts.on_tail("-h", "--help", "Show this message") do
	    	puts opts
	      	exit
	    end
	end.parse!(args)

	return options
end

#------------------------------------------------------------------------------
def generate_config_h(build, dest)
	if build == "debug"
		$config_h << "#define CROWN_DEBUG"
	elsif build == "development"
		$config_h << "#define CROWN_DEVELOPMENT"
	elsif build == "release"
		$config_h << "#define CROWN_RELEASE"
	end

	f = File.new(dest, File::WRONLY|File::CREAT|File::TRUNC, 0644)
	f.write($config_h)
	f.close()
end

#------------------------------------------------------------------------------
def generate_application_mk(target, dest)
	f = File.new(dest, File::WRONLY|File::CREAT|File::TRUNC, 0644)
	f.write($application_mk)
	f.write("APP_APPLICATION := " + target)
	f.close()
end

#------------------------------------------------------------------------------
def create_android_project(target, name, path)
	engine_dest 	= path + "/jni"
	android_dest	= path + "/src/crown/android"
	assets_dest		= path + "/assets"

	# Creates path if not exists
	if not Dir.exists?(path)
		print "Creating directory " + path + "..."
		FileUtils.mkdir_p(path)
		print "OK!\n"
	end

	# Project path is not empty
	if not Dir["#{path}/."].empty?
		print "Cleaning directory " + path + "..."
		FileUtils.rm_rf("#{path}/.", :secure => true)
		print "OK!\n"
	end

	# Creates android project
	print "Creating android project...\n"
	system($android_create + " --target " + target + " --name " + name + " --path " + path + " --activity " + $activity + " --package " + $package)
	print "OK!\n"
	
	# if jni dir does not exists, create it!
	if not Dir.exists?(engine_dest)
		print "Creating directory " + engine_dest + "..."
		FileUtils.mkdir_p(engine_dest)
		print "OK!\n"
	end

	# if assets dir does not exists, create it!
	if not Dir.exists?(assets_dest)
		print "Creating directory " + assets_dest + "..."
		FileUtils.mkdir_p(assets_dest)
		print "OK!\n"
	end
end

#------------------------------------------------------------------------------
def fill_android_project(build, target, res, path)

	engine_dest 	= path + "/jni"
	android_dest	= path + "/src/crown/android"
	resources_dest	= path + "/assets"

	print "Filling Android project..."
	# Copy Engine files
	FileUtils.cp_r($engine_src, engine_dest, :remove_destination => true)
	FileUtils.cp_r($engine_src + "/../third", engine_dest, :remove_destination => true)

	# Generate android Config.h
	generate_config_h(build, engine_dest + "/Config.h")

	# Generate Application.mk
	generate_application_mk(target, engine_dest + "/Application.mk")
	
	# Copy luajit lib
	FileUtils.cp($luajit + "/lib/libluajit-5.1.so.2.0.2", engine_dest + "/libluajit-5.1.so")

	# Copy oggvorbis lib
	FileUtils.cp($oggvorbis + "/lib/libogg.a", engine_dest + "/libogg.a")
	FileUtils.cp($oggvorbis + "/lib/libvorbis.a", engine_dest + "/libvorbis.a")

	# Copy physx lib
	FileUtils.cp($physx + "/lib/libPhysX3.a", engine_dest)
	FileUtils.cp($physx + "/lib/libSimulationController.a", engine_dest)
	FileUtils.cp($physx + "/lib/libLowLevel.a", engine_dest)
	FileUtils.cp($physx + "/lib/libLowLevelCloth.a", engine_dest)
	FileUtils.cp($physx + "/lib/libPxTask.a", engine_dest)
	FileUtils.cp($physx + "/lib/libPhysXProfileSDK.a", engine_dest)
	FileUtils.cp($physx + "/lib/libPhysX3Extensions.a", engine_dest)
	FileUtils.cp($physx + "/lib/libSceneQuery.a", engine_dest)
	FileUtils.cp($physx + "/lib/libPhysX3Common.a", engine_dest)
	FileUtils.cp($physx + "/lib/libPhysX3CharacterKinematic.a", engine_dest)
	FileUtils.cp($physx + "/lib/libPhysX3Vehicle.a", engine_dest)
	FileUtils.cp($physx + "/lib/libPhysX3Cooking.a", engine_dest)
	FileUtils.cp($physx + "/lib/libPvdRuntime.a", engine_dest)

	# Copy java files
	FileUtils.cp_r(Dir.glob($android_src), android_dest, :remove_destination => true)

	# Copy android manifest
	FileUtils.cp($android_manifest, path)

	if build == "release"
		FileUtils.cp_r(res + "/.", resources_dest)
	end

	print "OK!\n"
end

#------------------------------------------------------------------------------
def build_android_project(path)
	print "Building...\n"
	# Move to root directory of Android project
	Dir.chdir(path)
	# Build libraries
	if not system("ndk-build")
		print "Critical error: Unable to build crown libraries"
		return
	end

	# Build apk
	# FIXME: it's convenient to build apk in debug mode because we avoid apk signature.
	# In a near future, we'll manage the build process in release mode
	# N.B: you can build crown in release mode and run 'ant debug' for achieving the same result of a final product
	if not system("ant debug")
		print "Critical error: Unable to build crown project"
		return
	else
		print "Done!\n"
	end
end

#------------------------------------------------------------------------------
opts = parse_command_line(ARGV)

create_android_project(opts.target, opts.name, opts.path)
fill_android_project(opts.build, opts.target, opts.res, opts.path)
build_android_project(opts.path)