#include "launcher.h"
#include "imgui.h"
#include "GLFW/glfw3.h"
#include "Overlays.h"
#include "IconsFontAwesome5.h"
#include "DiscordRPC.h"
#include "assets.h"
#include "settings.h"

#define MAX_DEVICES 8
const char* devices[MAX_DEVICES];
int selectedDevice = 0;
void SelectDevice(int context);
void CheckForDevices();
bool isFuelOverlayOn = false;
static bool show_app_about = false;
bool isTireAppOn = false;
bool isDashOverlayOn = false;
bool isDashSettingsOn = false;
bool isSessionInfoOn = false;
bool isDiscordRpcOn = true;
bool lowPowerOn = false;
void DrawLauncherUI()
{
	CheckForDevices();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.5f;
	ImGuiIO& io = ImGui::GetIO();
	ImGuiConfigFlags flags = io.ConfigFlags;
	flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiViewportFlags_NoFocusOnClick | ImGuiViewportFlags_NoDecoration | ImGuiWindowFlags_NoBackground;
	static bool use_work_area = false;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 window_pos = ImGui::GetWindowPos();
	ImVec2 window_size = ImGui::GetWindowSize();
	ImVec2 window_center = ImVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
	ImGui::SetNextWindowPos(viewport->Pos, use_work_area, ImVec2( 0, 0 ));
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Background", NULL, flags);
	{
		//GetBackgroundDrawList()->AddImage((void*)(intptr_t)textures[2], ImVec2(1396, 598), ImVec2(600,400), ImVec2(0, 0), ImVec2(1, 1));
		ImGui::Image((void *)textures[2], ImVec2(700, 400), ImVec2(0, 0), ImVec2(-1, -1));
	}
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::SetNextWindowPos(viewport->Pos, use_work_area, ImVec2(0, 0));
	ImGui::SetNextWindowSize(viewport->Size, use_work_area);
	ImGui::Begin("Launcher", NULL, flags);
	{//Dash
		if (ImGui::Button("Dash Overlay", ImVec2(100, 40))){
			isDashOverlayOn = !isDashOverlayOn;
		}
		DrawDashOverlay(isDashOverlayOn, selectedDevice);
		ImGui::SameLine(0, 5);
		//Dash settings
		if (ImGui::Button(ICON_FA_PLUS, ImVec2(40, 40))){
			isDashSettingsOn = !isDashSettingsOn;
		}
		DrawDashSettings(isDashSettingsOn);
	}
	ImGui::SameLine(500);
	{//Settings
		if (ImGui::Button("Settings", ImVec2(80, 40)))
		{
			ImGui::OpenPopup("Settings");
		}
		if (ImGui::BeginPopupModal("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
		{
			float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
			ImGui::Text("Selected device : %s", devices[selectedDevice]);
			ImGui::SameLine(250, 50);
			if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { SelectDevice(0); }
			ImGui::SameLine(0.0f, spacing);
			if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { SelectDevice(1); }
			ImGui::Checkbox("Low power mode.", &lowPowerOn);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Progam will be less demanding on cpu but it will be laggy.");
			ImGui::Checkbox("Discord RPC on.", &isDiscordRpcOn);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Restart required.");
			if (ImGui::Button("OK", ImVec2(150, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::SameLine();
			if (ImGui::Button("Save", ImVec2(150, 0))) { sjson::save(); ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}
	{
		if (ImGui::Button("Tire App", ImVec2(100, 40))){
			isTireAppOn = !isTireAppOn;
		}
		DrawTireApp(isTireAppOn);
	}
	{//Info
		if (ImGui::Button("Session Info", ImVec2(100, 40))){
			isSessionInfoOn = !isSessionInfoOn;
		}
		if(ImGui::IsItemHovered())
			ImGui::SetTooltip("Work in progress...");
		//DrawSessionInfo(isSessionInfoOn);
	}
	{//Fuel
		if (ImGui::Button("Fuel App", ImVec2(100, 40))){
			isFuelOverlayOn = !isFuelOverlayOn;
		}
		DrawFuelApp(isFuelOverlayOn);
	}
	//About
	ImGui::SetCursorPos(ImVec2(500,350));
	if (ImGui::Button("About", ImVec2(80, 40)))
	{
		ImGui::OpenPopup("About");
	}
	if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
	{
		//ImGui::PushFont(paradigma);
		ImGui::Text("Made by Patrickek");
		ImGui::Text("Special thanks to KalyN");
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PopStyleVar();
		if (ImGui::Button("OK", ImVec2(150, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
	
	ImGui::End();
	if(isDiscordRpcOn)
		DiscordRPC::Update();
	if (lowPowerOn) { Sleep(25); }
}
void CheckForDevices()
{
	for (int i = 0; i < MAX_DEVICES; i++)
	{
		if (glfwJoystickPresent(i))
		{
			devices[i] = glfwGetJoystickName(i);
		}
	}
}
void SelectDevice(int context)
{
	if (context == 0)
	{
		selectedDevice--;
		if (selectedDevice < 0)
			selectedDevice = MAX_DEVICES - 1;
		if (devices[selectedDevice] == NULL)
		{
			selectedDevice = 0;
		}
	}
	if (context == 1)
	{
		selectedDevice++;
		if (selectedDevice >= MAX_DEVICES)
			selectedDevice = 0;
		if (devices[selectedDevice] == NULL)
		{
			selectedDevice = 0;
		}
	}
}