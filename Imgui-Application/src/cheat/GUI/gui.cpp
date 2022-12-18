#include "gui.h"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_dx9.h"
#include "../../imgui/imgui_impl_win32.h"
#include <fstream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wide_param, LPARAM long_param);

long __stdcall window_process(HWND window, UINT message, WPARAM wide_param, LPARAM long_param) {

	if (ImGui_ImplWin32_WndProcHandler(window, message, wide_param, long_param))
		return true;

	switch (message) 
	{
		case WM_SIZE: {
			if (gui::m_device && wide_param != SIZE_MINIMIZED) {

				gui::m_d3d_params.BackBufferWidth = LOWORD(long_param);
				gui::m_d3d_params.BackBufferHeight = LOWORD(long_param);
				gui::reset_device();

			}
		} return 0;

		case WM_SYSCOMMAND: {
			if ((wide_param & 0xff0) == SC_KEYMENU) /* disabled alt application menu*/
				return 0;

		} break;

		case WM_DESTROY: {
			PostQuitMessage(0);
		} return 0;

		case WM_LBUTTONDOWN: {
			gui::m_position = MAKEPOINTS(long_param); /*set click points*/
		} return 0;

		case WM_MOUSEMOVE: {
			if (wide_param == MK_LBUTTON) {
				const auto points = MAKEPOINTS(long_param);
				auto rect = RECT{};

				GetWindowRect(gui::m_window, &rect);

				rect.left += points.x - gui::m_position.x;
				rect.top += points.y - gui::m_position.y;

				if (gui::m_position.x >= 0 &&
					gui::m_position.x <= gui::m_width &&
					gui::m_position.y >= 0 && gui::m_position.y <= 19) {
					SetWindowPos(gui::m_window, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
		} return 0;
	}

	return DefWindowProcW(window, message, wide_param, long_param);
}

namespace gui {
	void create_window(const char* window_name, const char* class_name) noexcept {
		m_window_class.cbSize = sizeof(WNDCLASSEXA);
		m_window_class.style = CS_CLASSDC;
		m_window_class.lpfnWndProc = window_process;
		m_window_class.cbClsExtra = 0;
		m_window_class.cbWndExtra = 0;
		m_window_class.hInstance = GetModuleHandleA(0);
		m_window_class.hIcon = 0;
		m_window_class.hCursor = 0;
		m_window_class.hbrBackground = 0;
		m_window_class.lpszMenuName = 0;
		m_window_class.lpszClassName = class_name;
		m_window_class.hIconSm = 0;

		RegisterClassExA(&m_window_class);

		m_window = CreateWindowA(
			class_name,
			window_name,
			WS_POPUP,
			100, 100,
			m_width,
			m_height,
			0, 0,
			m_window_class.hInstance,
			0
		);

		ShowWindow(m_window, SW_SHOWDEFAULT);
		UpdateWindow(m_window);
	}
	void destroy_window() noexcept {
		DestroyWindow(m_window);
		UnregisterClass(m_window_class.lpszClassName, m_window_class.hInstance);
	}

	bool create_device() noexcept {
		m_d3d = Direct3DCreate9(D3D_SDK_VERSION);

		if (!m_d3d)
			return false;

		ZeroMemory(&m_d3d_params, sizeof(m_d3d_params));

		m_d3d_params.Windowed = TRUE;
		m_d3d_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		m_d3d_params.BackBufferFormat = D3DFMT_UNKNOWN;
		m_d3d_params.EnableAutoDepthStencil = TRUE;
		m_d3d_params.AutoDepthStencilFormat = D3DFMT_D16;
		m_d3d_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		if (m_d3d->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			m_window,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&m_d3d_params,
			&m_device
		) < 0) return false;

		return true;
	}

	void reset_device() noexcept {
		ImGui_ImplDX9_InvalidateDeviceObjects();

		const auto result = m_device->Reset(&m_d3d_params);

		if (result == D3DERR_INVALIDCALL)
			IM_ASSERT(0);

		ImGui_ImplDX9_CreateDeviceObjects();
	}

	void destroy_device() noexcept {
		if (m_device) {
			m_device->Release();
			m_device = nullptr;
		}

		if (m_d3d) {
			m_d3d->Release();
			m_d3d = nullptr;
		}
	}

	void create_imgui() noexcept {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		io.IniFilename = NULL;

		ImGui::StyleColorsDark();

		std::ifstream f("C:\\Windows\\Fonts\\segoeui.tff");
		if(f.good()) 
			io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.tff", 15.0f);

		ImGui_ImplWin32_Init(m_window);
		ImGui_ImplDX9_Init(m_device);
	}

	void destroy_gui() noexcept {
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void begin_render() noexcept {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		/*start imgui frame*/
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void end_render() noexcept {
		ImGui::EndFrame();

		m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

		m_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

		if (m_device->BeginScene() >= 0) {
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			m_device->EndScene();
		}

		const auto result = m_device->Present(0, 0, 0, 0);

		if (result == D3DERR_DEVICELOST && m_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			reset_device();
	}

	void render() noexcept {
		ImGui::SetNextWindowPos({ 0, 0 });
		ImGui::SetNextWindowSize({ m_width, m_height });
		ImGui::Begin("Imgui Application", &m_exit,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoMove
		);

		ImGui::Button("Shit yourself");

		ImGui::End();
	}
}