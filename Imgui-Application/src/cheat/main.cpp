#include "GUI/gui.h"

#include <thread>

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR args, int command_show) {

	/*create gui*/
	gui::create_window("GUI Application", "GUI Application");
	gui::create_device();
	gui::create_imgui();

	while (gui::m_exit) 
	{
		gui::begin_render();
		gui::render();
		gui::end_render();

		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}

	gui::destroy_gui();
	gui::destroy_device();
	gui::destroy_window();

	return EXIT_SUCCESS;
}