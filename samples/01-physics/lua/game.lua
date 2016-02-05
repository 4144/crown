require "lua/camera"

local wd = wd or nil
local pw = pw or nil
local rw = rw or nil
local sg = sg or nil
local camera = camera or nil
local physics_debug = physics_debug or false
local graphics_debug = graphics_debug or false
local fpscamera = fpscamera or nil
local move = false

function init()
	-- Set the title of the main window
	Window.set_title("01-physics")

	Device.enable_resource_autoload(true)

	-- Create World
	wd = Device.create_world()
	pw = World.physics_world(wd)
	rw = World.render_world(wd)
	sg = World.scene_graph(wd)

	-- Spawn camera
	camera_unit = World.spawn_unit(wd, "core/units/camera")
	local camera_tr = SceneGraph.transform_instances(sg, camera_unit)
	SceneGraph.set_local_position(sg, camera_tr, Vector3(0, 1.5, -20))

	-- Spawn light
	World.spawn_unit(wd, "core/units/light", Vector3(-25, 25, -25))

	-- Spawn skydome
	skydome = World.spawn_unit(wd, "skydome")
	local skydome_transform = SceneGraph.transform_instances(sg, skydome)
	SceneGraph.set_local_scale(sg, skydome_transform, Vector3(500, 500, 500))

	-- Spawn ground
	World.spawn_unit(wd, "plane", Vector3(0, 0, 0))

	-- Load test level
	World.load_level(wd, "test", Vector3(0, 0.1, 0))

	-- Spawn additional stuff
	World.spawn_unit(wd, "ramp", Vector3(0, 0, 25), Quaternion.look(Vector3(1, 0, 0)))

	-- Debug stuff
	PhysicsWorld.enable_debug_drawing(pw, physics_debug)
	RenderWorld.enable_debug_drawing(rw, graphics_debug)

	-- Spawn FPS camera
	fpscamera = FPSCamera(wd, camera_unit)
end

function update(dt)
	-- Update wd
	World.update(wd, dt)

	-- Stop the engine when the 'ESC' key is released
	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end

	if Keyboard.released(Keyboard.button_id("z")) then
		physics_debug = not physics_debug
		PhysicsWorld.enable_debug_drawing(pw, physics_debug)
	end

	if Keyboard.released(Keyboard.button_id("x")) then
		graphics_debug = not graphics_debug
		RenderWorld.enable_debug_drawing(rw, graphics_debug)
	end

	-- Spawn a sphere when left mouse button is pressed
	if Mouse.pressed(Mouse.button_id("left")) then
		local camera_transform = SceneGraph.transform_instances(sg, camera_unit)
		local pos = SceneGraph.local_position(sg, camera_transform)
		local dir = Matrix4x4.z(SceneGraph.local_pose(sg, camera_transform))
		local u1 = World.spawn_unit(wd, "sphere", pos)
		Vector3.normalize(dir)
		local a1 = PhysicsWorld.actor_instances(pw, u1)
		PhysicsWorld.add_actor_impulse(pw, a1, dir * 500.0)
	end

	-- Perform a raycast when middle mouse button is pressed
	if Mouse.pressed(Mouse.button_id("middle")) then
		local camera_transform = SceneGraph.transform_instances(sg, camera_unit)
		local pos = SceneGraph.local_position(sg, camera_transform)
		local dir = Matrix4x4.z(SceneGraph.local_pose(sg, camera_transform))
		local hits = PhysicsWorld.raycast(pw, pos, dir, 100, "closest")
		if #hits > 0 then
			PhysicsWorld.add_actor_impulse(pw, hits[1], dir * 400.0)
		end
	end

	-- Update camera
	local delta = Vector3.zero()
	if Mouse.pressed(Mouse.button_id("right")) then move = true end
	if Mouse.released(Mouse.button_id("right")) then move = false end
	if move then delta = Mouse.axis(Mouse.axis_id("cursor_delta")) end
	fpscamera:update(-delta.x, -delta.y)
end

function render(dt)
	local win_w, win_h = Device.resolution()
	World.set_camera_aspect(wd, camera, win_w/win_h)

	Device.render_world(wd, fpscamera:camera())
end

function shutdown()
	Device.destroy_world(wd)
end
