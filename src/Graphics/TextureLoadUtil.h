#pragma once

#include <Utilities\UtilityTypes.h>
#include <Graphics\DirectX.h>

namespace pn {

// ------- FUNCTIONS ---------

dx_resource_view LoadTexture2D(const string& filepath);
dx_resource_view LoadCubemap(const string& filepath);


} // namespace pn