--
-- Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

function crown_project(_name, _kind, _defines)

	project ("crown" .. _name)
		kind (_kind)

		includedirs {
			CROWN_DIR .. "src",
			CROWN_DIR .. "src/audio",
			CROWN_DIR .. "src/compilers",
			CROWN_DIR .. "src/core",
			CROWN_DIR .. "src/core/containers",
			CROWN_DIR .. "src/core/error",
			CROWN_DIR .. "src/core/filesystem",
			CROWN_DIR .. "src/core/json",
			CROWN_DIR .. "src/core/math",
			CROWN_DIR .. "src/core/memory",
			CROWN_DIR .. "src/core/network",
			CROWN_DIR .. "src/core/strings",
			CROWN_DIR .. "src/core/thread",
			CROWN_DIR .. "src/input",
			CROWN_DIR .. "src/lua",
			CROWN_DIR .. "src/main",
			CROWN_DIR .. "src/physics",
			CROWN_DIR .. "src/renderers",
			CROWN_DIR .. "src/resource",
			CROWN_DIR .. "src/world",
			CROWN_DIR .. "third/bgfx/include",
			CROWN_DIR .. "third/bgfx/src",
			CROWN_DIR .. "third/bx/include",
			CROWN_DIR .. "third/freetype",
			CROWN_DIR .. "third/stb_image",
			CROWN_DIR .. "third/stb_vorbis",
		}

		if _OPTIONS["with-bullet"] then
			includedirs {
				CROWN_DIR .. "third/bullet3/src",
				CROWN_DIR .. "third/bullet3/src/Bullet3Collision",
				CROWN_DIR .. "third/bullet3/src/Bullet3Collision/BroadPhaseCollision",
				CROWN_DIR .. "third/bullet3/src/Bullet3Collision/NarrowPhaseCollision",
				CROWN_DIR .. "third/bullet3/src/Bullet3Common",
				CROWN_DIR .. "third/bullet3/src/Bullet3Dynamics",
				CROWN_DIR .. "third/bullet3/src/Bullet3Dynamics/ConstraintSover",
				CROWN_DIR .. "third/bullet3/src/Bullet3Geometry",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/BroadphaseCollision",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/initialize",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/NarrowPhaseCollision",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/ParallelPrimitives",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/Raycast",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/RigidBody",
				CROWN_DIR .. "third/bullet3/src/Bullet3Serialize",
				CROWN_DIR .. "third/bullet3/src/Bullet3Serialize/Bullet2FileLoader",
				CROWN_DIR .. "third/bullet3/src/BulletCollision",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/BroadphaseCollision",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/CollisionDispatch",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/CollisionShapes",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/Gimpact",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/NarrowPhaseCollision",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/Character",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/ConstraintSolver",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/Dynamics",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/Featherstone",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/MLCPSolvers",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/Vehicle",
				CROWN_DIR .. "third/bullet3/src/BulletSoftBody",
				CROWN_DIR .. "third/bullet3/src/LinearMath",
				CROWN_DIR .. "third/bullet3/src/clew",
			}

			links {
				"bullet",
			}
		end

		defines {
			_defines,
		}

		links {
			"bgfx",
		}

		if _OPTIONS["with-luajit"] then
			includedirs {
				CROWN_DIR .. "third/luajit/src",
			}
			configuration { "android-arm" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/android_arm"
				}
			configuration { "linux-* or android-arm" }
				links {
					"luajit"
				}
			configuration { "x32", "linux-*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/linux_x86"
				}
			configuration { "x64", "linux-*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/linux_x64"
				}

			configuration { "vs*"}
				links {
					"lua51"
				}
			configuration { "x32", "vs*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/win_x86"
				}
			configuration { "x64", "vs*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/win_x64"
				}

				configuration {}
		end

		if _OPTIONS["with-openal"] then
			includedirs {
				CROWN_DIR .. "third/openal/include"
			}

			links { "openal" }

			configuration {}
		end

		configuration { "debug or development" }
			flags {
				"Symbols"
			}
			defines {
				"_DEBUG",
				"CROWN_DEBUG=1"
			}

		configuration { "release" }
			defines {
				"NDEBUG"
			}

		configuration { "linux*" }
			includedirs {
				"$(PHYSX_SDK_LINUX)/Include",
				"$(PHYSX_SDK_LINUX)/Include/common",
				"$(PHYSX_SDK_LINUX)/Include/characterkinematic",
				"$(PHYSX_SDK_LINUX)/Include/cloth",
				"$(PHYSX_SDK_LINUX)/Include/common",
				"$(PHYSX_SDK_LINUX)/Include/cooking",
				"$(PHYSX_SDK_LINUX)/Include/extensions",
				"$(PHYSX_SDK_LINUX)/Include/foundation",
				"$(PHYSX_SDK_LINUX)/Include/geometry",
				"$(PHYSX_SDK_LINUX)/Include/particles",
				"$(PHYSX_SDK_LINUX)/Include/physxprofilesdk",
				"$(PHYSX_SDK_LINUX)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_LINUX)/Include/pvd",
				"$(PHYSX_SDK_LINUX)/Include/pxtask",
				"$(PHYSX_SDK_LINUX)/Include/RepX",
				"$(PHYSX_SDK_LINUX)/Include/RepXUpgrader",
				"$(PHYSX_SDK_LINUX)/Include/vehicle",
			}

		configuration { "android*" }
			includedirs {
				"$(PHYSX_SDK_ANDROID)/Include",
				"$(PHYSX_SDK_ANDROID)/Include/common",
				"$(PHYSX_SDK_ANDROID)/Include/characterkinematic",
				"$(PHYSX_SDK_ANDROID)/Include/cloth",
				"$(PHYSX_SDK_ANDROID)/Include/common",
				"$(PHYSX_SDK_ANDROID)/Include/cooking",
				"$(PHYSX_SDK_ANDROID)/Include/extensions",
				"$(PHYSX_SDK_ANDROID)/Include/foundation",
				"$(PHYSX_SDK_ANDROID)/Include/geometry",
				"$(PHYSX_SDK_ANDROID)/Include/particles",
				"$(PHYSX_SDK_ANDROID)/Include/physxprofilesdk",
				"$(PHYSX_SDK_ANDROID)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_ANDROID)/Include/pvd",
				"$(PHYSX_SDK_ANDROID)/Include/pxtask",
				"$(PHYSX_SDK_ANDROID)/Include/RepX",
				"$(PHYSX_SDK_ANDROID)/Include/RepXUpgrader",
				"$(PHYSX_SDK_ANDROID)/Include/vehicle",
			}

		configuration { "vs*" }
			includedirs {
				"$(PHYSX_SDK_WINDOWS)/Include",
				"$(PHYSX_SDK_WINDOWS)/Include/common",
				"$(PHYSX_SDK_WINDOWS)/Include/characterkinematic",
				"$(PHYSX_SDK_WINDOWS)/Include/cloth",
				"$(PHYSX_SDK_WINDOWS)/Include/common",
				"$(PHYSX_SDK_WINDOWS)/Include/cooking",
				"$(PHYSX_SDK_WINDOWS)/Include/extensions",
				"$(PHYSX_SDK_WINDOWS)/Include/foundation",
				"$(PHYSX_SDK_WINDOWS)/Include/geometry",
				"$(PHYSX_SDK_WINDOWS)/Include/particles",
				"$(PHYSX_SDK_WINDOWS)/Include/physxprofilesdk",
				"$(PHYSX_SDK_WINDOWS)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_WINDOWS)/Include/pvd",
				"$(PHYSX_SDK_WINDOWS)/Include/pxtask",
				"$(PHYSX_SDK_WINDOWS)/Include/RepX",
				"$(PHYSX_SDK_WINDOWS)/Include/RepXUpgrader",
				"$(PHYSX_SDK_WINDOWS)/Include/vehicle",
				"$(DXSDK_DIR)/Include",
			}

		configuration { "linux-*" }
			links {
				"X11",
				"Xrandr",
				"pthread",
				"GL",
				"dl",
			}

		configuration { "x32", "debug", "linux-*" }
			linkoptions {
				"-rdynamic",
				"-Wl,--start-group $(addprefix -l," ..
				"	PhysX3CHECKED_x86" ..
				"	PhysX3CommonCHECKED_x86" ..
				"	PhysX3CookingCHECKED_x86" ..
				"	PhysX3CharacterKinematicCHECKED_x86" ..
				"	PhysX3ExtensionsCHECKED" ..
				"	PhysX3VehicleCHECKED" ..
				"	PhysXProfileSDKCHECKED" ..
				"	PhysXVisualDebuggerSDKCHECKED" ..
				"	PxTaskCHECKED" ..
				") -Wl,--end-group"
			}

		configuration { "x64", "debug", "linux-*" }
			linkoptions {
				"-rdynamic",
				"-Wl,--start-group $(addprefix -l," ..
				"	PhysX3CHECKED_x64" ..
				"	PhysX3CommonCHECKED_x64" ..
				"	PhysX3CookingCHECKED_x64" ..
				"	PhysX3CharacterKinematicCHECKED_x64" ..
				"	PhysX3ExtensionsCHECKED" ..
				"	PhysX3VehicleCHECKED" ..
				"	PhysXProfileSDKCHECKED" ..
				"	PhysXVisualDebuggerSDKCHECKED" ..
				"	PxTaskCHECKED" ..
				") -Wl,--end-group"
			}

		configuration { "x32", "development", "linux-*" }
			linkoptions
			{
				"-rdynamic",
				"-Wl,--start-group $(addprefix -l," ..
				"	PhysX3PROFILE_x86" ..
				"	PhysX3CommonPROFILE_x86" ..
				"	PhysX3CookingPROFILE_x86" ..
				"	PhysX3CharacterKinematicPROFILE_x86" ..
				"	PhysX3ExtensionsPROFILE" ..
				"	PhysX3VehiclePROFILE" ..
				"	PhysXProfileSDKPROFILE" ..
				"	PhysXVisualDebuggerSDKPROFILE" ..
				"	PxTaskPROFILE" ..
				") -Wl,--end-group"
			}

		configuration { "x64", "development", "linux-*" }
			linkoptions
			{
				"-rdynamic",
				"-Wl,--start-group $(addprefix -l," ..
				"	PhysX3PROFILE_x64" ..
				"	PhysX3CommonPROFILE_x64" ..
				"	PhysX3CookingPROFILE_x64" ..
				"	PhysX3CharacterKinematicPROFILE_x64" ..
				"	PhysX3ExtensionsPROFILE" ..
				"	PhysX3VehiclePROFILE" ..
				"	PhysXProfileSDKPROFILE" ..
				"	PhysXVisualDebuggerSDKPROFILE" ..
				"	PxTaskPROFILE" ..
				") -Wl,--end-group"
			}

		configuration { "x32", "release", "linux-*" }
			linkoptions {
				"-Wl,--start-group $(addprefix -l," ..
				"	PhysX3_x86" ..
				"	PhysX3Common_x86" ..
				"	PhysX3Cooking_x86" ..
				"	PhysX3CharacterKinematic_x86" ..
				"	PhysX3Extensions" ..
				"	PhysX3Vehicle" ..
				"	PhysXProfileSDK" ..
				"	PhysXVisualDebuggerSDK" ..
				"	PxTask" ..
				") -Wl,--end-group"
			}

		configuration { "x64", "release", "linux-*" }
			linkoptions {
				"-Wl,--start-group $(addprefix -l," ..
				"	PhysX3_x64" ..
				"	PhysX3Common_x64" ..
				"	PhysX3Cooking_x64" ..
				"	PhysX3CharacterKinematic_x64" ..
				"	PhysX3Extensions" ..
				"	PhysX3Vehicle" ..
				"	PhysXProfileSDK" ..
				"	PhysXVisualDebuggerSDK" ..
				"	PxTask" ..
				") -Wl,--end-group"
			}

		configuration { "android*" }
			kind "ConsoleApp"
			targetextension ".so"
			linkoptions {
				"-shared"
			}
			links {
				"EGL",
				"GLESv2",
				"OpenSLES",
			}

		configuration { "debug", "android-arm" }
			linkoptions {
				"-Wl,--start-group $(addprefix -l," ..
				"	LowLevelCloth" ..
				"	PhysX3 " ..
				"	PhysX3Common" ..
				"	PxTask" ..
				"	LowLevel" ..
				"	PhysX3CharacterKinematic" ..
				"	PhysX3Cooking" ..
				"	PhysX3Extensions" ..
				"	PhysX3Vehicle" ..
				"	PhysXProfileSDK" ..
				"	PhysXVisualDebuggerSDK" ..
				"	PvdRuntime" ..
				"	SceneQuery" ..
				"	SimulationController" ..
				") -Wl,--end-group"
			}

		configuration { "development", "android-arm" }
			linkoptions {
				"-Wl,--start-group $(addprefix -l," ..
				"	LowLevelCloth" ..
				"	PhysX3 " ..
				"	PhysX3Common" ..
				"	PxTask" ..
				"	LowLevel" ..
				"	PhysX3CharacterKinematic" ..
				"	PhysX3Cooking" ..
				"	PhysX3Extensions" ..
				"	PhysX3Vehicle" ..
				"	PhysXProfileSDK" ..
				"	PhysXVisualDebuggerSDK" ..
				"	PvdRuntime" ..
				"	SceneQuery" ..
				"	SimulationController" ..
				") -Wl,--end-group"
			}

		configuration { "release", "android-arm" }
			linkoptions {
				"-Wl,--start-group $(addprefix -l," ..
				"	LowLevelCloth" ..
				"	PhysX3 " ..
				"	PhysX3Common" ..
				"	PxTask" ..
				"	LowLevel" ..
				"	PhysX3CharacterKinematic" ..
				"	PhysX3Cooking" ..
				"	PhysX3Extensions" ..
				"	PhysX3Vehicle" ..
				"	PhysXProfileSDK" ..
				"	PhysXVisualDebuggerSDK" ..
				"	PvdRuntime" ..
				"	SceneQuery" ..
				"	SimulationController" ..
				") -Wl,--end-group"
			}

		configuration { "vs*" }
			links {
				"OpenGL32",
				"dbghelp",
				"xinput",
			}

		configuration { "debug", "x32", "vs*"}
			links {
				"PhysX3CharacterKinematicCHECKED_x86",
				"PhysX3CHECKED_x86",
				"PhysX3CommonCHECKED_x86",
				"PhysX3CookingCHECKED_x86",
				"PhysX3ExtensionsCHECKED",
			}

		configuration { "debug", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematicCHECKED_x64",
				"PhysX3CHECKED_x64",
				"PhysX3CommonCHECKED_x64",
				"PhysX3CookingCHECKED_x64",
				"PhysX3ExtensionsCHECKED",
			}

		configuration { "development", "x32", "vs*" }
			links {
				"PhysX3CharacterKinematicPROFILE_x86",
				"PhysX3PROFILE_x86",
				"PhysX3CommonPROFILE_x86",
				"PhysX3CookingPROFILE_x86",
				"PhysX3ExtensionsPROFILE",
			}

		configuration { "development", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematicPROFILE_x64",
				"PhysX3PROFILE_x64",
				"PhysX3CommonPROFILE_x64",
				"PhysX3CookingPROFILE_x64",
				"PhysX3ExtensionsPROFILE",
			}

		configuration { "release", "x32", "vs*" }
			links {
				"PhysX3CharacterKinematic_x86",
				"PhysX3_x86",
				"PhysX3Common_x86",
				"PhysX3Cooking_x86",
				"PhysX3Extensions",
			}

		configuration { "release", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematic_x64",
				"PhysX3_x64",
				"PhysX3Common_x64",
				"PhysX3Cooking_x64",
				"PhysX3Extensions",
			}

		configuration {}

		files {
			CROWN_DIR .. "src/**.h",
			CROWN_DIR .. "src/**.cpp"
		}

		strip()

		configuration {} -- reset configuration
end
