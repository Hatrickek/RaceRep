#include <Windows.h>
#include <TlHelp32.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <vector>
#include <tchar.h>

#include <SDL.h>
#include <SDL_syswm.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_sdl2.h>

struct WindowItem {
  HWND current_window = {};
  char current_window_title[125] = {};
  char current_process_name[125] = {};
};

static ID3D11Device* d3d_device = nullptr;
static ID3D11DeviceContext* d3d_device_context = nullptr;
static IDXGISwapChain* dxgi_swapchain = nullptr;
static ID3D11RenderTargetView* d3d_main_render_target_view = nullptr;
static HWND target_window = nullptr;
static DWORD target_pid = 0;

static const char* window_name = "RaceRep";
static bool is_running = true;
static bool target_set = false;
static bool toggle_input = true;
static HHOOK keyboard_hook = nullptr;
static SDL_Joystick* selected_joystick = nullptr;

SDL_Joystick* open_joystick(int device_index) {
  SDL_Joystick* joystick = ::SDL_JoystickOpen(device_index);
  if (!joystick) {
    ::SDL_Log("Could not open joystick %d: %s", device_index, SDL_GetError());
    return nullptr;
  }
  return joystick;
}

void close_joystick(SDL_Joystick* joystick) { //
  ::SDL_JoystickClose(joystick);
}

float calculate_steering(SDL_Joystick* joystick, float steering_width, float dash_steering_size_mult) {
  float value = 0.0f;

  int axes_count = ::SDL_JoystickNumAxes(joystick);
  if (axes_count < 1) {
    ::SDL_Log("Selected joystick has no axes!");
    ::SDL_JoystickClose(joystick);
    return (float)steering_width / 2 * dash_steering_size_mult;
  }

  // (normalized to -1.0 to 1.0)
  Sint16 axis_raw = ::SDL_JoystickGetAxis(joystick, 0); // Axis 0 = steering
  float axis_normalized = axis_raw / 32768.0f;          // 32768 = max value for int16

  if (axis_normalized == 0.0f) {
    value = (float)steering_width / 2 * dash_steering_size_mult;
  } else {
    value = axis_normalized * (float)steering_width / 2 * dash_steering_size_mult;
  }

  return value;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
    bool is_key_down = !(kb->flags & LLKHF_UP);

    // Toggle on Ctrl+Shift+T
    if (is_key_down && ::GetAsyncKeyState(VK_CONTROL) & 0x8000 && ::GetAsyncKeyState(VK_SHIFT) & 0x8000 &&
        kb->vkCode == 'T') {
      toggle_input = !toggle_input;
    }
    if (is_key_down && ::GetAsyncKeyState(VK_CONTROL) & 0x8000 && ::GetAsyncKeyState(VK_SHIFT) & 0x8000 &&
        kb->vkCode == 'Q') {
      is_running = false;
    }
  }
  return ::CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
}

void create_render_target() {
  ID3D11Texture2D* pBackBuffer;
  dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  if (pBackBuffer != nullptr) {
    d3d_device->CreateRenderTargetView(pBackBuffer, nullptr, &d3d_main_render_target_view);
    pBackBuffer->Release();
  }
}

void cleanup_render_target() {
  if (d3d_main_render_target_view != nullptr) {
    d3d_main_render_target_view->Release();
    d3d_main_render_target_view = nullptr;
  }
}

bool create_device_d3d(HWND hwnd) {
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  const UINT create_device_flags = 0;
  D3D_FEATURE_LEVEL feature_level;
  const D3D_FEATURE_LEVEL feature_level_array[2] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_0,
  };
  HRESULT res = ::D3D11CreateDeviceAndSwapChain(nullptr,
                                                D3D_DRIVER_TYPE_HARDWARE,
                                                nullptr,
                                                create_device_flags,
                                                feature_level_array,
                                                2,
                                                D3D11_SDK_VERSION,
                                                &sd,
                                                &dxgi_swapchain,
                                                &d3d_device,
                                                &feature_level,
                                                &d3d_device_context);
  if (res == DXGI_ERROR_UNSUPPORTED)
    res = ::D3D11CreateDeviceAndSwapChain(nullptr,
                                          D3D_DRIVER_TYPE_WARP,
                                          nullptr,
                                          create_device_flags,
                                          feature_level_array,
                                          2,
                                          D3D11_SDK_VERSION,
                                          &sd,
                                          &dxgi_swapchain,
                                          &d3d_device,
                                          &feature_level,
                                          &d3d_device_context);
  if (res != S_OK)
    return false;

  create_render_target();
  return true;
}

void cleanup_device_d3d() {
  cleanup_render_target();

  if (dxgi_swapchain) {
    dxgi_swapchain->Release();
    dxgi_swapchain = nullptr;
  }

  if (d3d_device_context) {
    d3d_device_context->Release();
    d3d_device_context = nullptr;
  }

  if (d3d_device) {
    d3d_device->Release();
    d3d_device = nullptr;
  }
}

bool is_window_alive() {
  DWORD current_pid;

  if (target_window == nullptr)
    return false;

  if (!::IsWindow(target_window))
    return true;

  ::GetWindowThreadProcessId(target_window, &current_pid);

  if (current_pid != target_pid)
    return false;

  return true;
}

void move_window(const HWND current_process_window) {
  RECT rect;
  if (target_window == nullptr)
    return;

  ::GetWindowRect(target_window, &rect);

  int window_width = rect.right - rect.left;
  int window_height = rect.bottom - rect.top;

  window_width -= 5;
  window_height -= 29;

  ::SetWindowPos(current_process_window, nullptr, rect.left, rect.top, window_width, window_height, SWP_SHOWWINDOW);
}

void get_process_name(const LPSTR lpProcessName, const DWORD dPID) {
  char lpCurrentProcessName[125];

  PROCESSENTRY32 proc_list{};
  proc_list.dwSize = sizeof(proc_list);

  const HANDLE hProcList = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcList == INVALID_HANDLE_VALUE)
    return;

  if (!::Process32First(hProcList, &proc_list))
    return;

  if (proc_list.th32ProcessID == dPID) {
    wcstombs_s(nullptr, lpCurrentProcessName, proc_list.szExeFile, sizeof(lpCurrentProcessName));
    strncpy_s(lpProcessName, sizeof(lpCurrentProcessName), lpCurrentProcessName, sizeof(lpCurrentProcessName));
    return;
  }

  while (::Process32Next(hProcList, &proc_list)) {
    if (proc_list.th32ProcessID == dPID) {
      wcstombs_s(nullptr, lpCurrentProcessName, proc_list.szExeFile, sizeof(lpCurrentProcessName));
      strncpy_s(lpProcessName, sizeof(lpCurrentProcessName), lpCurrentProcessName, sizeof(lpCurrentProcessName));
      return;
    }
  }
}

bool is_window_cloaked(const HWND current_window) {
  DWORD cloaked;
  const HRESULT hr = ::DwmGetWindowAttribute(current_window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
  return SUCCEEDED(hr) && cloaked;
}

bool is_window_valid(const HWND current_window) {
  DWORD styles, ex_styles;
  RECT rect;

  if (!::IsWindowVisible(current_window) || (::IsIconic(current_window) || is_window_cloaked(current_window)))
    return FALSE;

  ::GetClientRect(current_window, &rect);
  styles = (DWORD)GetWindowLongPtr(current_window, GWL_STYLE);
  ex_styles = (DWORD)GetWindowLongPtr(current_window, GWL_EXSTYLE);

  if (ex_styles & WS_EX_TOOLWINDOW)
    return FALSE;
  if (styles & WS_CHILD)
    return FALSE;
  if (rect.bottom == 0 || rect.right == 0)
    return FALSE;

  return TRUE;
}

BOOL CALLBACK enum_all_windows(const HWND window, const LPARAM lPrams) {
  if (!is_window_valid(window))
    return TRUE;

  WindowItem current_window_item = {window};
  DWORD procID;

  ::GetWindowTextA(window, current_window_item.current_window_title, sizeof(current_window_item.current_window_title));

  if (strlen(current_window_item.current_window_title) == 0)
    return TRUE;

  ::GetWindowThreadProcessId(window, &procID);
  get_process_name(current_window_item.current_process_name, procID);

  const auto vWindowList = (std::vector<WindowItem>*)lPrams;

  vWindowList->push_back(current_window_item);

  return TRUE;
}

void get_all_window(std::vector<WindowItem>* window_list) {
  window_list->clear();
  ::EnumWindows(enum_all_windows, (LPARAM)window_list);
}

void set_target_window(const HWND win) {
  target_window = win;
  ::SetForegroundWindow(target_window);
  ::GetWindowThreadProcessId(target_window, &target_pid);
  target_set = true;
}

void render() {
  // ImGui_ImplWin32_EnableDpiAwareness();
  ::SetProcessDPIAware();
  if (::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) != 0) {
    ::MessageBoxA(0, SDL_GetError(), "SDL Init Error", MB_ICONERROR);
    return;
  }

  // Setup window
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_POPUP_MENU |
                                                   SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS |
                                                   SDL_WINDOW_FULLSCREEN_DESKTOP);
  SDL_Window* window = ::SDL_CreateWindow(
      window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  if (window == nullptr) {
    ::MessageBoxA(0, SDL_GetError(), "SDL Create Window Error", MB_ICONERROR);
    return;
  }

  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  HWND hwnd = (HWND)wmInfo.info.win.window;

  keyboard_hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

  const auto hit_test_callback = [](SDL_Window*, const SDL_Point*, void* data) -> SDL_HitTestResult {
    HWND hWnd = static_cast<HWND>(data);
    if (toggle_input) {
      ::SetWindowLongPtr(hWnd, GWL_EXSTYLE, ::GetWindowLongPtr(hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
      return SDL_HITTEST_NORMAL; // Handle input normally
    }
    ::SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED);
    return SDL_HITTEST_NORMAL;
  };

  ::SDL_SetWindowHitTest(window, hit_test_callback, hwnd);

  ::SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

  ::SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
  const MARGINS margin = {-1, 0, 0, 0};
  ::DwmExtendFrameIntoClientArea(hwnd, &margin);

  if (!create_device_d3d(hwnd)) {
    cleanup_device_d3d();
    return;
  }

  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.IniFilename = nullptr;

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4* colors = style.Colors;

  // Primary background
  colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);  // #131318
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f); // #131318

  colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);

  // Headers
  colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);

  // Buttons
  colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.32f, 0.40f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.38f, 0.50f, 1.00f);

  // Frame BG
  colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);

  // Title
  colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

  // Borders
  colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // Text
  colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

  // Highlights
  colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.80f, 1.00f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 1.00f, 0.50f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.75f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.90f, 1.00f, 1.00f);

  // Scrollbar
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.00f);

  style.WindowMenuButtonPosition = ImGuiDir_None;
  style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
  style.AntiAliasedFill = true;
  style.AntiAliasedLines = true;
  style.AntiAliasedLinesUseTex = true;

  ::ImGui_ImplSDL2_InitForD3D(window);
  ::ImGui_ImplDX11_Init(d3d_device, d3d_device_context);

  style.FontSizeBase = 20.0f;
  io.Fonts->AddFontDefault();
  ImFont* nerd_font = io.Fonts->AddFontFromFileTTF("resources/fonts/AdwaitaMonoNerdFont-Regular.ttf");
  IM_ASSERT(nerd_font != nullptr);

  const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

  while (is_running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ::ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        is_running = false;
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
          is_running = false;
        if (event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(window)) {
          cleanup_render_target();
          dxgi_swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
          create_render_target();
        }
      }
    }

    if (!is_window_alive() && target_set)
      is_running = false;

    if (!is_running)
      break;

    if (target_window != nullptr && target_set)
      move_window(hwnd);

    ::ImGui_ImplDX11_NewFrame();
    ::ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    {

      // --- UI CODE BEGINS ---

      ImGui::PushFont(nerd_font);

      enum class UIState : uint8_t {
        ProcessSelection,
        DrawDashboard,
      };

      static UIState ui_state = UIState::ProcessSelection;

      switch (ui_state) {
        case UIState::ProcessSelection: {
          std::vector<WindowItem> window_list;
          get_all_window(&window_list);

          if (!window_list.empty()) {
            ImGui::SetNextWindowSize({400, 100}, ImGuiCond_Once);
            ImGui::SetNextWindowBgAlpha(1.0f);

            static WindowItem selected_window = {};

            ImGui::Begin("Overlay Target Chooser", &is_running, ImGuiWindowFlags_AlwaysAutoResize);
            if (ImGui::BeginCombo("##combo", selected_window.current_window_title)) {
              for (const auto& item : window_list) {
                const bool is_selected = (strcmp(selected_window.current_window_title, item.current_window_title) == 0);
                if (ImGui::Selectable(item.current_window_title, is_selected))
                  selected_window = item;
                if (is_selected)
                  ImGui::SetItemDefaultFocus();
              }
              ImGui::EndCombo();
            }
            ImGui::NewLine();
            if (ImGui::Button("Start Overlay")) {
              set_target_window(selected_window.current_window);

              selected_joystick = open_joystick(0); // just open the first joystick for now

              ui_state = UIState::DrawDashboard;
            }
            ImGui::End();
          }
          break;
        }
        case UIState::DrawDashboard: {
          ImGui::Begin("Dash");
          float steering = calculate_steering(selected_joystick, 1.0f, 1.0f);
          ImGui::Text("Steering: %f", steering);
          ImGui::End();
          break;
        }
      }

      ImGui::PopFont();

      // --- UI CODE ENDS ---
    }
    ImGui::EndFrame();

    ImGui::Render();
    const float clear_color_with_alpha[4] = {
        clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
    d3d_device_context->OMSetRenderTargets(1, &d3d_main_render_target_view, nullptr);
    d3d_device_context->ClearRenderTargetView(d3d_main_render_target_view, clear_color_with_alpha);
    ::ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    dxgi_swapchain->Present(1, 0);
  }

  close_joystick(selected_joystick);

  ::ImGui_ImplDX11_Shutdown();
  ::ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  cleanup_device_d3d();

  ::DestroyWindow(hwnd);

  ::TerminateProcess(GetModuleHandleA(nullptr), 0);
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  render();

  return 0;
}
