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

#include "Crown.h"
#include "FPSSystem.h"

using namespace crown;

Renderer* r;
IndexBufferId monkey_ib;
VertexBufferId monkey_vb;
IndexBufferId cube_ib;
VertexBufferId cube_vb;
IndexBufferId cone_ib;
VertexBufferId cone_vb;

GPUProgramId default_program;
GPUProgramId texture_program;
Camera* camera;
FPSSystem* fps;

TextureId grass_texture;
TextureId lightmap_texture;
IndexBufferId quad_ib;
VertexBufferId quad_vb;

UniformId u_albedo_0;
UniformId u_lightmap_0;
UniformId u_brightness;

static float quad_vertices[] =
{
	-1.0f, -1.0f, 0.0f, 
	 0.0f, 0.0f, 1.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,
	 0.0f, 0.0f,

	 1.0f, -1.0f, 0.0f,
	 0.0f, 0.0f, 1.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,
	 1.0f, 0.0f,

	 1.0f,  1.0f, 0.0f,
	 0.0f, 0.0f, 1.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,
	 1.0f, 1.0f,

	-1.0f,  1.0f, 0.0f,
	 0.0f, 0.0f, 1.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,
	 0.0f, 1.0f
};

static uint16_t quad_indices[] =
{
	0, 1, 3,
	1, 2, 3
};

static const char* default_vertex =
	"in vec4           a_position;"
	"in vec4           a_normal;"
	"in vec2           a_tex_coord0;"
	"in vec4           a_color;"

	"uniform mat4      u_model;"
	"uniform mat4      u_model_view_projection;"

	"varying out vec2  tex_coord0;"
	"varying out vec4  color;"

	"void main(void)"
	"{"
	"	tex_coord0 = a_tex_coord0;"
	"   color = a_color;"
	"	gl_Position = u_model_view_projection * a_position;"
	"}";

static const char* default_fragment = 
	"void main(void)"
	"{"
	"	gl_FragColor = vec4(1, 0, 0, 0);"
	"}";

static const char* texture_fragment = 
	"in vec2            tex_coord0;"
	"in vec4            color;"

	"uniform sampler2D  u_albedo_0;"
	"uniform sampler2D  u_lightmap_0;"
	"uniform float      u_brightness;"

	"void main(void)"
	"{"
	"	gl_FragColor = texture(u_albedo_0, tex_coord0);"
	"}";

void draw(float dt)
{
	Mat4 pose; pose.load_identity();

	fps->update(dt);
	fps->set_view_by_cursor();

	//-----------------------
	r->set_layer_view(0, camera->view_matrix());
	r->set_layer_projection(0, camera->projection_matrix());
	r->set_layer_viewport(0, 0, 0, 1000, 625);
	r->set_layer_clear(0, CLEAR_COLOR | CLEAR_DEPTH, Color4::LIGHTBLUE, 1.0f);

	r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_CULL_CCW);
	r->commit(0);

	static uint64_t prim = STATE_PRIMITIVE_TRIANGLES;
	if (device()->keyboard()->key_pressed(KC_z))
	{
		prim = STATE_PRIMITIVE_TRIANGLES;
	}
	else if (device()->keyboard()->key_pressed(KC_x))
	{
		prim = STATE_PRIMITIVE_POINTS;
	}
	else if (device()->keyboard()->key_pressed(KC_c))
	{
		prim = STATE_PRIMITIVE_LINES;
	}

	static uint32_t filter = TEXTURE_FILTER_NEAREST;
	if (device()->keyboard()->key_pressed(KC_1))
	{
		filter = TEXTURE_FILTER_NEAREST;
	}
	else if (device()->keyboard()->key_pressed(KC_2))
	{
		filter = TEXTURE_FILTER_LINEAR;
	}
	else if (device()->keyboard()->key_pressed(KC_3))
	{
		filter = TEXTURE_FILTER_BILINEAR;
	}
	else if (device()->keyboard()->key_pressed(KC_4))
	{
		filter = TEXTURE_FILTER_TRILINEAR;
	}
	static float brightness = 1.0f;
	if (device()->keyboard()->key_pressed(KC_UP))
	{
		brightness += 0.01f;
	}
	else if (device()->keyboard()->key_pressed(KC_DOWN))
	{
		brightness += -0.01f;
	}
	if (brightness > 1.0f)
		brightness = 1.0f;

	//-----------------------
	// r->set_state(prim | STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_CULL_CW);
	// r->set_vertex_buffer(monkey_vb);
	// r->set_index_buffer(monkey_ib);
	// r->set_program(default_program);
	// pose.set_translation(Vec3(-3, 0, -3));
	// r->set_pose(pose);
	// r->commit(0);

	//-----------------------
	r->set_state(prim | STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_CULL_CW);
	r->set_vertex_buffer(cube_vb);
	r->set_index_buffer(cube_ib);
	r->set_program(texture_program);
	r->set_texture(0, u_albedo_0, grass_texture, filter | TEXTURE_WRAP_CLAMP_EDGE);
	r->set_texture(1, u_lightmap_0, lightmap_texture, filter | TEXTURE_WRAP_CLAMP_EDGE);
	r->set_uniform(u_brightness, UNIFORM_FLOAT_1, &brightness, 1);

	pose.set_translation(Vec3(0, 0, -3));
	r->set_pose(pose);
	r->commit(0);

	//-----------------------
	// r->set_state(prim | STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW);
	// r->set_vertex_buffer(cone_vb);
	// r->set_index_buffer(cone_ib);
	// r->set_program(default_program);

	// pose.set_translation(Vec3(3, 0, -3));
	// r->set_pose(pose);
	// r->commit(0);

	//-----------------------
	// r->set_state(prim | STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW);
	// r->set_vertex_buffer(quad_vb);
	// r->set_index_buffer(quad_ib);
	// r->set_program(texture_program);
	// r->set_texture(0, u_albedo_0, grass_texture, filter | TEXTURE_WRAP_CLAMP_EDGE);
	// r->set_texture(1, u_lightmap_0, lightmap_texture, filter | TEXTURE_WRAP_CLAMP_EDGE);
	// r->set_uniform(u_brightness, UNIFORM_FLOAT_1, &brightness, 1);

	// pose.set_translation(Vec3(0, 0, -1));
	// r->set_pose(pose);
	// r->commit(0);
}

int main(int argc, char** argv)
{
	crown::init();

	Device* engine = device();
	engine->init(argc, argv);

	// Load resources
	ResourceManager* resman = engine->resource_manager();
	ResourceId texture = resman->load("texture", "ship");
	ResourceId lightmap = resman->load("texture", "lightmap");
	ResourceId monkey = resman->load("mesh", "monkey");
	ResourceId cube = resman->load("mesh", "ship");
	ResourceId cone = resman->load("mesh", "cone");
	resman->flush();

	TextureResource* texture_resource = (TextureResource*)resman->data(texture);
	TextureResource* lightmap_resource = (TextureResource*)resman->data(lightmap);
	MeshResource* monkey_resource = (MeshResource*)resman->data(monkey);
	MeshResource* cube_resource = (MeshResource*)resman->data(cube);
	MeshResource* cone_resource = (MeshResource*)resman->data(cone);

	Log::d("monkey format = %d", monkey_resource->vertex_format());
	Log::d("monkey vcount = %d", monkey_resource->num_vertices());
	Log::d("cube format   = %d", cube_resource->vertex_format());
	Log::d("cube vcount   = %d", cube_resource->num_vertices());
	Log::d("cone format = %d", cone_resource->vertex_format());
	Log::d("cone vcount = %d", cone_resource->num_vertices());

	// Create vb/ib
	r = engine->renderer();
	monkey_vb = r->create_vertex_buffer(monkey_resource->num_vertices(), monkey_resource->vertex_format(), monkey_resource->vertices());
	monkey_ib = r->create_index_buffer(monkey_resource->num_indices(), monkey_resource->indices());
	cube_vb = r->create_vertex_buffer(cube_resource->num_vertices(), cube_resource->vertex_format(), cube_resource->vertices());
	cube_ib = r->create_index_buffer(cube_resource->num_indices(), cube_resource->indices());
	cone_vb = r->create_vertex_buffer(cone_resource->num_vertices(), cone_resource->vertex_format(), cone_resource->vertices());
	cone_ib = r->create_index_buffer(cone_resource->num_indices(), cone_resource->indices());

	// Create texture
	grass_texture = r->create_texture(texture_resource->width(), texture_resource->height(), texture_resource->format(),
										texture_resource->data());
	lightmap_texture = r->create_texture(lightmap_resource->width(), lightmap_resource->height(), lightmap_resource->format(),
										lightmap_resource->data());

	quad_vb = r->create_vertex_buffer(4, VERTEX_P3_N3_C4_T2, quad_vertices);
	quad_ib = r->create_index_buffer(6, quad_indices);

	ShaderId default_vs = r->create_shader(SHADER_VERTEX, default_vertex);
	ShaderId default_fs = r->create_shader(SHADER_FRAGMENT, default_fragment);
	ShaderId texture_fs = r->create_shader(SHADER_FRAGMENT, texture_fragment);

	u_albedo_0 = r->create_uniform("u_albedo_0", UNIFORM_INTEGER_1, 1);
	u_lightmap_0 = r->create_uniform("u_lightmap_0", UNIFORM_INTEGER_1, 1);
	u_brightness = r->create_uniform("u_brightness", UNIFORM_FLOAT_1, 1);

	default_program = r->create_gpu_program(default_vs, default_fs);
	texture_program = r->create_gpu_program(default_vs, texture_fs);

	// Create camera
	TempAllocator2048 alloc;
	camera = CE_NEW(alloc, Camera)(Vec3(0, 0, 3), 90.0f, 16.0f/9.0f);
	fps = CE_NEW(alloc, FPSSystem)(camera, 3.0f, 2.5f);

	while (engine->is_running())
	{
		engine->frame(draw);
	}

	//resman->unload(mesh);
	resman->unload(texture);
	resman->unload(lightmap);
	resman->unload(monkey);
	resman->unload(cube);
	resman->unload(cone);
	r->destroy_index_buffer(monkey_ib);
	r->destroy_vertex_buffer(monkey_vb);
	r->destroy_index_buffer(cube_ib);
	r->destroy_vertex_buffer(cube_vb);
	r->destroy_index_buffer(cone_ib);
	r->destroy_vertex_buffer(cone_vb);

	r->destroy_shader(default_vs);
	r->destroy_shader(default_fs);
	r->destroy_shader(texture_fs);
	r->destroy_gpu_program(default_program);
	r->destroy_gpu_program(texture_program);
	r->destroy_vertex_buffer(quad_vb);
	r->destroy_index_buffer(quad_ib);
	r->destroy_uniform(u_albedo_0);
	r->destroy_uniform(u_lightmap_0);
	r->destroy_uniform(u_brightness);

	engine->shutdown();
	crown::shutdown();
}
