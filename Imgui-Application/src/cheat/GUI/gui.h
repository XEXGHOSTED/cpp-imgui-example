#pragma once

#include <d3d9.h>

namespace gui {
	constexpr int m_width = 720;
	constexpr int m_height = 480;
	
	inline bool m_exit = true;

	inline HWND m_window = nullptr;
	inline WNDCLASSEXA m_window_class = {};

	inline POINTS m_position = {};

	inline PDIRECT3D9 m_d3d = nullptr;
	inline IDirect3DDevice9 *m_device = nullptr;
	inline D3DPRESENT_PARAMETERS m_d3d_params = {};

	void create_window(const char* window_name, const char* class_name) noexcept;
	void destroy_window() noexcept;

	bool create_device() noexcept;
	void reset_device() noexcept;
	void destroy_device() noexcept;

	void create_imgui() noexcept;
	void destroy_gui() noexcept;

	void begin_render() noexcept;
	void end_render() noexcept;
	void render() noexcept;
}