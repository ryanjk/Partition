#pragma once

#include <Graphics\Window.h>

namespace pn {

namespace app {

#define PARTITION_CONFIGURATION_FILE_NAME "./partition.json"

// -------- VARIABLES -----------

extern pn::application_window_desc window_desc;

extern double FPS;
extern double FIXED_DT;
extern double dt;
extern double time_since_application_start;

// ------ FUNCTIONS ------------

void LoadEngineConfiguration();

void Exit();
bool ShouldExit();

} // namespace app

} // namespace pn