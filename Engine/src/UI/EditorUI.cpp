#include <UI\EditorUI.h>

namespace pn {

namespace gui {

// ------- CONSTANTS ---------

const float MAIN_MENU_HEIGHT = 20.0f;

// ------- VARIABLES ----------

bool show_main_menu;

// --------- FUNCTIONS --------

void InitEditorUI() {
	show_main_menu = true;
}

// --------- MAIN MENU ---------

void SetMainMenuVisible(bool value) {
	show_main_menu = value;
}

void DrawMainMenu(const unsigned int screen_width) {
	if (!show_main_menu) return;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, { 0.0f });
	if (ImGui::Begin("Window", NULL,
					 ImGuiWindowFlags_MenuBar |
					 ImGuiWindowFlags_NoTitleBar |
					 ImGuiWindowFlags_NoMove |
					 ImGuiWindowFlags_NoResize)) {
		ImGui::SetWindowPos({ 0, 0 });
		ImGui::SetWindowSize({ static_cast<float>(screen_width), MAIN_MENU_HEIGHT });
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(1);
}

} // namespace gui

} // namespace pn