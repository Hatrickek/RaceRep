#pragma once
#include "SharedFileOut.h"
#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <tchar.h>
#include <iostream>
void accInitPhysics(void);
void accInitGraphics(void);
void accInitStatic(void);

struct SMElement
{
	HANDLE hMapFile;
	unsigned char* mapFileBuffer;
};

extern SMElement m_graphics;
extern SMElement m_physics;
extern SMElement m_static;
