#include <System\Flycam.h>

#include <Component\transform_t.h>

#include <Input\Input.h>
#include <Application\Global.h>

namespace pn {

void UpdateFlycam(transform_t& camera, const float movement_speed, const float rotation_speed) {
	pn::vec3f camera_translation{ 0.0f, 0.0f, 0.0f };
	if (pn::input::GetKeyState(pn::input::W) == pn::input::key_state::PRESSED) {
		camera_translation += pn::vec3f(0.0f, 0.0f, 1);
	}
	if (pn::input::GetKeyState(pn::input::S) == pn::input::key_state::PRESSED) {
		camera_translation += pn::vec3f(0.0f, 0.0f, -1);
	}
	if (pn::input::GetKeyState(pn::input::A) == pn::input::key_state::PRESSED) {
		camera_translation += pn::vec3f(-1, 0.0f, 0.0f);
	}
	if (pn::input::GetKeyState(pn::input::D) == pn::input::key_state::PRESSED) {
		camera_translation += pn::vec3f(1, 0.0f, 0.0f);
	}

	auto dt = app::dt;
	auto mouse_pos_delta = pn::input::GetMousePosDelta();
	if (mouse_pos_delta.x != 0 || mouse_pos_delta.y != 0) {
		vec2f md(mouse_pos_delta.x, mouse_pos_delta.y);
		md = md * rotation_speed * dt;

		RotateLocal(camera, vec3f::UnitX, md.y);
		RotateWorld(camera, vec3f::UnitY, md.x);
	}

	const float SPEED = movement_speed * dt;
	TranslateLocal(camera, camera_translation * SPEED);
};


}
