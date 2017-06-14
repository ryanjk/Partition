#pragma once

#include <Windows.h>

#include <Utilities\UtilityTypes.h>

namespace pn {

// ------------- TYPEDEFS ------------------

// --------- CLASS DEFINITIONS -------------

struct application_window_desc {
	HINSTANCE	h_instance;
	int			width;
	int			height;
	bool		fullscreen;
};

// ------------- FUNCTIONS -----------------

WNDCLASS	CreateWindowClass(const pn::string& class_name, const HINSTANCE h_instance, const WNDPROC window_proc);
void		RegisterWindowClass(const WNDCLASS& window_class);

HWND		CreateWindowHandle(const HINSTANCE h_instance, const pn::string& window_class, const application_window_desc& awd);
HWND		CreateApplicationWindow(const application_window_desc& awd, const WNDPROC window_proc);

void		CreateConsole();

// TODO:
// LoadApplicationWindowDescFromFile


} // namespace pn