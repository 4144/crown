#include "Crown.h"
#include "Terrain.h"
#include "FPSSystem.h"

using namespace crown;

class WndCtrl: public KeyboardListener
{
public:

	WndCtrl()
	{
		device()->input_manager()->register_keyboard_listener(this);
	}

	void key_released(const KeyboardEvent& event)
	{
		if (event.key == KC_ESCAPE)
		{
			device()->stop();
		}
	}
};

void DrawCircle(const Vec3& pos, float radius);

class MainScene: public KeyboardListener, public MouseListener
{

public:

	MainScene() :
		optShowSkybox(true),
		optShowCrate(true),
		optShowTerrain(true)
	{
		device()->input_manager()->register_keyboard_listener(this);
		device()->input_manager()->register_mouse_listener(this);
		mouseRightPressed = false;
		mouseLeftPressed = false;
	}

	~MainScene()
	{
	}

	void key_released(const KeyboardEvent& event)
	{
		if (event.key == '1')
		{
			terrain.PlotCircle(2, 2, 2, 2);
		}

		if (event.key == '2')
		{
			terrain.PlotCircle(4, 4, 4, 2);
		}

		if (event.key == '3')
		{		
			terrain.PlotCircle(8, 8, 8, 2);
		}

		if (event.key == KC_SPACE)
		{
			if (cam)
			{
				cam->SetActive(!cam->IsActive());
			}
		}
	}

	void button_pressed(const MouseEvent& event)
	{
		if (event.button == MB_LEFT)
		{
			mouseLeftPressed = true;

			//GLint view[4];
			//GLdouble proj[16], model[16];

			//glGetDoublev(GL_MODELVIEW_MATRIX, model);
			//glGetDoublev(GL_PROJECTION_MATRIX, proj);
			//glGetIntegerv(GL_VIEWPORT, view);

			//int x = event.x;
			//int y = event.y;

			// Adjust y wndCoord
			//y = (625 - y);

			//double sX, sY, sZ;
			//double eX, eY, eZ;

			//gluUnProject(x, y, 0.0f, model, proj, view, &sX, &sY, &sZ);
			//gluUnProject(x, y, 1.0f, model, proj, view, &eX, &eY, &eZ);

			//Vec3 dir = Vec3(eX, eY, eZ) - Vec3(sX, sY, sZ);

			//dir.normalize();

			//ray.direction = dir;
		}
		else if (event.button == MB_RIGHT)
		{
			mouseRightPressed = true;
		}
		wheel += event.wheel * 0.25;
	}

	void button_released(const MouseEvent& event)
	{
		if (event.button == MB_LEFT)
		{
			mouseLeftPressed = false;
		}
		else if (event.button == MB_RIGHT)
		{
			mouseRightPressed = false;
		}
		wheel -= event.wheel * 0.25;
	}
		
	void OnLoad()
	{
		crown::Renderer* renderer = crown::device()->renderer();

		renderer->set_clear_color(Color4::LIGHTBLUE);
		
		Vec3 start = Vec3(0.0f, 10.0f, 0.0f);

		// Add a movable camera
		cam = new MovableCamera(/*Vec3::ZERO*/start, true, 90.0f, 1.6f, true, 0.1, 2.5);

		if (cam)
		{
			cam->SetActive(true);
			cam->SetSpeed(0.1);
			cam->SetFarClipDistance(1000.0f);
		}

		system = new FPSSystem(cam);

		// Add a skybox
		skybox = new Skybox(Vec3::ZERO, true);

		//if (skybox)
		//{
		//	skybox->SetFace(SF_NORTH,	GetTextureManager()->Load("res/red_north.tga"));
		//	skybox->SetFace(SF_SOUTH,	GetTextureManager()->Load("res/red_south.tga"));
		//	skybox->SetFace(SF_EAST,	GetTextureManager()->Load("res/red_east.tga"));
		//	skybox->SetFace(SF_WEST,	GetTextureManager()->Load("res/red_west.tga"));
		//	skybox->SetFace(SF_UP,		GetTextureManager()->Load("res/red_up.tga"));
		//	skybox->SetFace(SF_DOWN,	GetTextureManager()->Load("res/red_down.tga"));
		//}

		terrain.CreateTerrain(64, 64, 1, 0.0f);

		//grass = GetTextureManager()->Load("res/grass.tga");
		//grass->SetFilter(TF_TRILINEAR);

		terrain.PlotCircle(4, 4, 4, 2);

		//terrain.ApplyBrush(32, 32, 1.25f);
		terrain.UpdateVertexBuffer(true);
	}

	void RenderScene()
	{
		Renderer* renderer = device()->renderer();
		
		system->set_view_by_cursor();
		system->camera_render();

		renderer->set_lighting(false);
		renderer->set_texturing(0, false);

		if (skybox)
		{
			skybox->Render();
		}

		if (cam->IsActive())
		{
			ray.origin = cam->GetPosition();
			ray.direction = cam->GetLookAt();
		}

		/* Render the terrain */
		renderer->set_ambient_light(Color4(0.5f, 0.5f, 0.5f, 1.0f));

		renderer->set_lighting(true);
		renderer->set_light(0, true);
		renderer->set_light_params(0, LT_DIRECTION, Vec3(0.6, 0.5f, -2.0f));
		renderer->set_light_color(0, Color4::WHITE, Color4::WHITE, Color4(0.6f, 0.6f, 0.6f));
		renderer->set_light_attenuation(0, 1, 0, 0);

		renderer->set_material_params(Color4(0.3f, 0.3f, 0.3f), Color4(0.8f, 0.8f, 0.8f), Color4::BLACK, Color4::BLACK, 0);

		renderer->set_matrix(MT_MODEL, Mat4::IDENTITY);
		// Texture disabled because of last updates not in sync... :(
		//renderer->set_texturing(0, true);
		//renderer->set_texture(0, grass);
		renderer->set_lighting(true);
		
		//glColor3f(1, 1, 1);

		terrain.Render();

		/* Test for intersection */
		Triangle tri, tri2;
		real dist;
		if (terrain.TraceRay(ray, tri, tri2, dist))
		{
			renderer->set_depth_test(false);
			Vec3 intersectionPoint = ray.origin + (ray.direction * dist);
			if (mouseLeftPressed)
			{
				terrain.ApplyBrush(intersectionPoint, 0.09f);
				terrain.UpdateVertexBuffer(true);
			}
			if (mouseRightPressed)
			{
				terrain.ApplyBrush(intersectionPoint, -0.09f);
				terrain.UpdateVertexBuffer(true);
			}
			renderer->set_depth_test(true);
		}
	}

private:

	FPSSystem* system;
	MovableCamera* cam;
	Skybox* skybox;
	Mat4 ortho;
	Terrain terrain;

	// Resources
	ResourceId grass;

	bool optShowSkybox;
	bool optShowCrate;
	bool optShowTerrain;
	bool mouseLeftPressed;
	bool mouseRightPressed;
	float wheel;
	Ray ray;
};

int main(int argc, char** argv)
{
	os::init_os();
	os::create_render_window(0, 0, 1000, 625, false);
	os::init_input();

	Device* engine = device();

	if (!engine->init(argc, argv))
	{
		return 0;
	}

	WndCtrl ctrl;
	MainScene mainScene;
	mainScene.OnLoad();

	while (engine->is_running())
	{
		os::event_loop();

		device()->input_manager()->event_loop();

		engine->renderer()->begin_frame();
			mainScene.RenderScene();
		engine->renderer()->end_frame();

		os::swap_buffers();
	}

	engine->shutdown();

	os::destroy_render_window();

	return 0;
}

