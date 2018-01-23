#include <Application\Global.h>

namespace pn {

namespace app {

// -------- VARIABLES -----------

pn::application_window_desc window_desc;

double FPS;
double FIXED_DT;
double dt;
double time_since_application_start;

bool exit;

// ---------- FUNCTIONS ---------

void Exit() {
	exit = true;
}
bool ShouldExit() {
	return exit;
}

}

}