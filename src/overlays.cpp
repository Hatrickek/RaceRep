#include "Overlays.h"
#include "imgui.h"
#include "GLFW/glfw3.h"
#include "Assets.h"
#include "Settings.h"
void draw_steering(int deviceIndex);
void draw_pedalgraph();
void draw_timetable();
void draw_tacho();
void draw_pedals(int deviceIndex);
float CalcSteering(int deviceIndex);
float steeringWidth = (float)956;
float steeringHeight = (float)110;
ImGuiConfigFlags floatingFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
float dashSteeringSizeMult = 0.5;
float dashKnobSizeMult = 1;
bool steeringOn = true;
bool useDirectInput = false;
SPageFilePhysics* pf_phy = {};
SPageFileGraphic* pf_grp = {};
SPageFileStatic* pf_stc = {};
int dashPreset = 0;
void UpdatePhysicsPageFile()
{
	if (pf_phy == NULL)
	{
		pf_phy = (SPageFilePhysics*)m_physics.mapFileBuffer;
	}
}
void UpdateStaticPageFile()
{
	if (pf_stc == NULL)
	{
		pf_stc = (SPageFileStatic*)m_static.mapFileBuffer;
	}
}
void UpdateGraphicsPageFile()
{
	if (pf_grp == NULL)
	{
		pf_grp = (SPageFileGraphic*)m_graphics.mapFileBuffer;
	}
}
SPageFileGraphic* GetGraphicsPageFile()
{
	return pf_grp;
}
SPageFilePhysics* GetPhysicPageFile()
{
	return pf_phy;
}
SPageFileStatic* GetStaticPageFile()
{
	return pf_stc;
}
void DrawDashOverlay(bool cond, int deviceIndex)
{
	if (!cond) return;
	UpdatePhysicsPageFile();
	UpdateGraphicsPageFile();
	UpdateStaticPageFile();
	
	ImGuiWindowClass winclass;
	winclass.ViewportFlagsOverrideSet = ImGuiViewportFlags_TopMost | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowClass(&winclass);
	//ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::Begin("Dash Overlay", NULL, ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
	
	switch (dashPreset)
	{
	case 0 :
		draw_pedalgraph();
		ImGui::SameLine();
		draw_pedals(deviceIndex);
		ImGui::SameLine();
		draw_tacho();
		break;
	case 1:
		draw_steering(deviceIndex);
		draw_pedals(deviceIndex);
		draw_timetable();
		break;
	case 2:
		draw_steering(deviceIndex);
		break;
	default:
		dashPreset = 0;
		break;
	}
	ImGui::End();
}
void DrawDashSettings(bool cond)
{
	if (!cond) return;
	ImGui::Begin("Dash settings", NULL, floatingFlags);
	{
		const char* steeringSizes[] = { "0.25","0.5","0.75","1"};
		static int current_steersize = 1;
		const char* dashPresets[] = { "1","2","3"};
		static int current_dashpreset = 0;
		if (ImGui::Combo("Dash preset", &current_dashpreset, dashPresets, IM_ARRAYSIZE(dashPresets)))
			dashPreset = current_dashpreset;
		if(ImGui::Combo("Steering Size", &current_steersize, steeringSizes, IM_ARRAYSIZE(steeringSizes)))
			SetSteeringSize(current_steersize);
		ImGui::Checkbox("Steering on.", &steeringOn);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The steering indicator on the dash overlay.");
		ImGui::Checkbox("Use direct input.", &useDirectInput);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Get input from directly the controller instead of ACC.");
		if(ImGui::Button("Save", ImVec2(80, 30)))
		{
			sjson::save();
		}
		/*if (ImGui::Button("Load", ImVec2(80, 30)))
		{
			sjson::loadJsonFile();

		}*/
	}
	ImGui::End();
}
void DrawTireApp(bool cond)//TODO: Add tyre compound.
{
	if (!cond) return;
	UpdatePhysicsPageFile();
	static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
	ImGuiWindowClass winclass;
	winclass.ViewportFlagsOverrideSet = ImGuiViewportFlags_TopMost | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowClass(&winclass);	
	ImGui::Begin("Tire App", NULL, floatingFlags);
	{
		if (ImGui::BeginTable("Tires", 2, flags))
		{
			ImGui::TableSetupColumn("FL");
			ImGui::TableSetupColumn("FR");
			ImGui::TableHeadersRow();
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%.2f psi", pf_phy->wheelsPressure[0]);
			ImGui::Text("%.2f C", pf_phy->tyreCoreTemperature[0]);
			ImGui::Text("Brake %.2f C", pf_phy->brakeTemp[0]);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.2f psi", pf_phy->wheelsPressure[1]);
			ImGui::Text("%.2f C", pf_phy->tyreCoreTemperature[1]);
			ImGui::Text("Brake %.2f C", pf_phy->brakeTemp[1]);
			ImGui::EndTable();
		}
		if (ImGui::BeginTable("Tires", 2, flags))
		{
			ImGui::TableSetupColumn("RL");
			ImGui::TableSetupColumn("RR");
			ImGui::TableHeadersRow();
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%.2f psi", pf_phy->wheelsPressure[2]);
			ImGui::Text("%.2f C", pf_phy->tyreCoreTemperature[2]);
			ImGui::Text("Brake %.2f C", pf_phy->brakeTemp[2]);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.2f psi", pf_phy->wheelsPressure[3]);
			ImGui::Text("%.2f C", pf_phy->tyreCoreTemperature[3]);
			ImGui::Text("Brake %.2f C", pf_phy->brakeTemp[3]);
			ImGui::EndTable();
		}
		
	}
	ImGui::End();
}
void SetSteeringSize(int item_current)
{
	switch (item_current)
	{
	case 0:
		dashSteeringSizeMult = 0.25;
		break;
	case 1:
		dashSteeringSizeMult = 0.5;
		break;
	case 2:
		dashSteeringSizeMult = 0.75;
		break;
	case 3:
		dashSteeringSizeMult = 1;
		break;
	default:
		dashSteeringSizeMult = 0.5;
		break;
	}
}
float CalcSteering(int deviceIndex)
{
	float value;
	int axesCount;
	const float* axes = glfwGetJoystickAxes(deviceIndex, &axesCount); //axes = 0 steering, axes = 1 throttle, axes = 2 brake
	if (axes[0] == 0)
	{
		value = (float) steeringWidth/ 2 * dashSteeringSizeMult;
	}
	else
	{
		value = (float)(axes[0]/*useDirectInput ? axes[0] : pf_phy->steerAngle*/ * (float)steeringWidth / 2 * dashSteeringSizeMult);
	}
	return value;
}
void DrawFuelApp(bool cond)
{
	if (!cond)return;
	UpdateGraphicsPageFile();
	UpdatePhysicsPageFile();
	UpdateStaticPageFile();
	ImGui::Begin("Fuel App", NULL, floatingFlags);
	{		
		ImGui::Text("Left: %.2f", pf_phy->fuel);
		ImGui::Text("Avg. consumption per laps: %.2f", pf_grp->fuelXLap);
		ImGui::Text("Laps possible : %.2f", pf_grp->fuelEstimatedLaps);
		ImGui::Text("Recommended fuel level : %.2f", pf_stc->aidFuelRate / pf_grp->fuelXLap);
		if(ImGui::IsItemHovered()) ImGui::SetTooltip("Cars fuel consumption / fuel usage per lap.");
		ImGui::Text("Used fuel since refueling: %.2f", pf_grp->usedFuel);
	}
	ImGui::End();
}
void DrawSessionInfo(bool cond)
{
	if (!cond) return;
	ImGui::Begin("Info", NULL, floatingFlags);
	{
		ImGui::BeginTable("Leaderboard", 1, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("test");
		ImGui::EndTable();
	}
	ImGui::End();
}
//Draw functions
void draw_steering(int deviceIndex)
{
	if (steeringOn)
	{
		ImGui::Image((void*)(intptr_t)textures[0], ImVec2(steeringWidth * dashSteeringSizeMult, steeringHeight * dashSteeringSizeMult));
		ImGui::SameLine(CalcSteering(deviceIndex), (float)(steeringWidth / 2 * dashSteeringSizeMult) + 10);
		ImGui::Image((void*)(intptr_t)textures[1], ImVec2((float)15 * dashSteeringSizeMult, 80 * dashSteeringSizeMult));
	}
}
void draw_pedalgraph()
{
	static float gas_val[90] = {};
	static int gas_values_offset = 0;
	static double gas_refresh_time = 0.0;
	if (gas_refresh_time == 0.0)
		gas_refresh_time = ImGui::GetTime();
	while (gas_refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
	{
		gas_val[gas_values_offset] = pf_phy->gas;
		gas_values_offset = (gas_values_offset + 1) % IM_ARRAYSIZE(gas_val);
		gas_refresh_time += 1.0f / 60.0f;
	}
	ImGui::SetItemAllowOverlap();
	ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0, 1, 0, 1));
	ImGui::PlotLines("##", gas_val, IM_ARRAYSIZE(gas_val), gas_values_offset, "##", 0.0f, 1.0f, ImVec2(0, 80.0f));
	ImGui::PopStyleColor();
	ImGui::SameLine(-1, 16);
	static float br_val[90] = {};
	static int br_values_offset = 0;
	static double br_refresh_time = 0.0;
	if (br_refresh_time == 0.0)
		br_refresh_time = ImGui::GetTime();
	while (br_refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
	{
		br_val[br_values_offset] = pf_phy->brake;
		br_values_offset = (br_values_offset + 1) % IM_ARRAYSIZE(br_val);
		br_refresh_time += 1.0f / 60.0f;
	}
	ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1, 0, 0, 1));
	ImGui::PlotLines("##", br_val, IM_ARRAYSIZE(br_val), br_values_offset, "##", 0.0f, 1.0f, ImVec2(0, 80.0f));
	ImGui::PopStyleColor();
}
void draw_timetable()
{
	ImGui::BeginChild("Time table", ImVec2(205, 70), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::TextColored(pf_grp->isValidLap ? ImVec4(1, 1, 1, 1) : ImVec4(1, 0, 0, 1), "Current Time: %ls", pf_grp->currentTime);
	ImGui::SameLine(0, 5);
	ImGui::TextColored(pf_grp->isDeltaPositive ? ImVec4(1, 0, 0, 1) : ImVec4(0, 1, 0, 1), " %ls", pf_grp->deltaLapTime);
	ImGui::Text("Best Time: %ls", pf_grp->bestTime);
	ImGui::Text("Last Time: %ls", pf_grp->lastTime);
	ImGui::EndChild();
}
void draw_pedals(int deviceIndex)
{
	int axesCount;
	const float* axes = glfwGetJoystickAxes(deviceIndex, &axesCount); //axes = 0 steering, axes = 1 throttle, axes = 2 brake
	ImGui::BeginChild("Pedals", ImVec2(100, 110), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs);
	float throttle = useDirectInput ? axes[1] : pf_phy->gas;
	throttle *= 100;
	ImGui::VSliderFloat("##", ImVec2(30, 100), &throttle, 0.0f, 100, "%.0f");
	float brake = useDirectInput ? axes[2] : pf_phy->brake;
	brake *= 100;
	ImGui::SameLine();
	ImGui::VSliderFloat("##", ImVec2(30, 100), &brake, 0.0f, 100, "%.0f");
	ImGui::EndChild();
}
void draw_tacho()
{
	ImGui::BeginChild("Tacho", ImVec2(130,100), false,ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	float redline = pf_phy->currentMaxRpm - 300.0f;
	int rpms = pf_phy->rpms;
	ImGui::TextColored(rpms >= redline && rpms != 0 ? ImVec4(1, 0, 0, 1) : ImVec4(1, 1, 1, 1), "RPM: %d", rpms);
	ImGui::SameLine();
	ImGui::Text("Gear %d", pf_phy->gear - 1);
	//ImGui::SameLine();
	ImGui::Text("%.2f km/h", pf_phy->speedKmh);
	ImGui::SameLine();
	ImGui::Text("%.2f L", pf_phy->fuel);
	ImGui::EndChild();
}