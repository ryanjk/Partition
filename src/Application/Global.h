#pragma once

#include <Graphics\Window.h>

namespace pn {

namespace app {

// -------- VARIABLES -----------

extern pn::application_window_desc window_desc;

extern double FPS;
extern double FIXED_DT;
extern double dt;
extern double time_since_application_start;

// ------ FUNCTIONS ------------

void Exit();
bool ShouldExit();

} // namespace app

} // namespace pn