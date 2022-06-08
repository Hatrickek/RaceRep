#pragma once
#include "accmem.h"
void DrawDashOverlay(bool cond, int deviceIndex);
void DrawFuelApp(bool cond);
void DrawDashSettings(bool cond);
void DrawTireApp(bool cond);
void UpdatePhysicsPageFile(void);
void UpdateGraphicsPageFile(void);
void UpdateStaticPageFile(void);
void DrawSessionInfo(bool cond);

void SetSteeringSize(int item_current);

SPageFilePhysics* GetPhysicPageFile();
SPageFileGraphic* GetGraphicsPageFile();
SPageFileStatic* GetStaticPageFile();

extern bool steeringOn;
extern bool useDirectInput;
extern float dashSteeringSizeMult;

extern int dashPreset;