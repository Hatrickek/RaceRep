// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "accmem.h"
#pragma optimize("",off)
using namespace std;


template <typename T, unsigned S>
inline unsigned arraysize(const T(&v)[S])
{
	return S;
}
SMElement m_graphics;
SMElement m_physics;
SMElement m_static;


void accInitPhysics()
{
	TCHAR szName[] = TEXT("Local\\acpmf_physics");
	m_physics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFilePhysics), szName);
	if (!m_physics.hMapFile)
	{
		MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "ACCS", MB_OK);
	}
	m_physics.mapFileBuffer = (unsigned char*)MapViewOfFile(m_physics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFilePhysics));
	if (!m_physics.mapFileBuffer)
	{
		MessageBoxA(GetActiveWindow(), "MapViewOfFile failed", "ACCS", MB_OK);
	}
}

void accInitGraphics()
{
	TCHAR szName[] = TEXT("Local\\acpmf_graphics");
	m_graphics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileGraphic), szName);
	if (!m_graphics.hMapFile)
	{
		MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "ACCS", MB_OK);
	}
	m_graphics.mapFileBuffer = (unsigned char*)MapViewOfFile(m_graphics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileGraphic));
	if (!m_graphics.mapFileBuffer)
	{
		MessageBoxA(GetActiveWindow(), "MapViewOfFile failed", "ACCS", MB_OK);
	}
}

void accInitStatic()
{
	TCHAR szName[] = TEXT("Local\\acpmf_static");
	m_static.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileStatic), szName);
	if (!m_static.hMapFile)
	{
		MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "ACCS", MB_OK);
	}
	m_static.mapFileBuffer = (unsigned char*)MapViewOfFile(m_static.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileStatic));
	if (!m_static.mapFileBuffer)
	{
		MessageBoxA(GetActiveWindow(), "MapViewOfFile failed", "ACCS", MB_OK);
	}
}

void dismiss(SMElement element)
{
	UnmapViewOfFile(element.mapFileBuffer);
	CloseHandle(element.hMapFile);
}

void printData(string name, float value)
{
	wcout << name.c_str() << " : " << value << endl;
}

template <typename T, unsigned S>
inline void printData(const string name, const T(&v)[S])
{
	wcout << name.c_str() << " : ";

	for (int i = 0; i < S; i++)
	{
		wcout << v[i];
		if (i < S - 1)
		{
			wcout << " , ";
		}

	}
	wcout << endl;
}

template <typename T, unsigned S, unsigned S2>
inline void printData2(const string name, const T(&v)[S][S2])
{
	wcout << name.c_str() << " : ";

	for (int i = 0; i < S; i++)
	{
		wcout << i << " : ";
		for (int j = 0; j < S2; j++)
		{
			wcout << v[i][j];
			if (j < S2 - 1)
			{
				wcout << " , ";
			}
		}

		wcout << ";" << endl;

	}

}

int _tmain(int argc, _TCHAR* argv[])
{
	accInitPhysics();
	accInitGraphics();
	accInitStatic();

	wcout << "Press 1 for physics, 2 for graphics, 3 for static" << endl;

	while (true)
	{
		if (GetAsyncKeyState(0x31) != 0) // user pressed 1
		{
			wcout << "---------------PHYSICS INFO---------------" << endl;
			SPageFilePhysics* pf_phy = (SPageFilePhysics*)m_physics.mapFileBuffer;
			printData("acc G", pf_phy->accG);
			printData("brake", pf_phy->brake);
			printData("camber rad", pf_phy->camberRAD);
			printData("damage", pf_phy->carDamage);
			printData("car height", pf_phy->cgHeight);
			printData("drs", pf_phy->drs);
			printData("tc", pf_phy->tc);
			printData("fuel", pf_phy->fuel);
			printData("gas", pf_phy->gas);
			printData("gear", pf_phy->gear);
			printData("number of tyres out", pf_phy->numberOfTyresOut);
			printData("packet id", pf_phy->packetId);
			printData("heading", pf_phy->heading);
			printData("pitch", pf_phy->pitch);
			printData("roll", pf_phy->roll);
			printData("rpms", pf_phy->rpms);
			printData("speed kmh", pf_phy->speedKmh);
			printData2("contact point", pf_phy->tyreContactPoint);
			printData2("contact normal", pf_phy->tyreContactNormal);
			printData2("contact heading", pf_phy->tyreContactHeading);
			printData("steer ", pf_phy->steerAngle);
			printData("suspension travel", pf_phy->suspensionTravel);
			printData("tyre core temp", pf_phy->tyreCoreTemperature);
			printData("tyre dirty level", pf_phy->tyreDirtyLevel);
			printData("tyre wear", pf_phy->tyreWear);
			printData("velocity", pf_phy->velocity);
			printData("wheel angular speed", pf_phy->wheelAngularSpeed);
			printData("wheel load", pf_phy->wheelLoad);
			printData("wheel slip", pf_phy->wheelSlip);
			printData("wheel pressure", pf_phy->wheelsPressure);
		}

		if (GetAsyncKeyState(0x32) != 0) // user pressed 2
		{
			wcout << "---------------GRAPHICS INFO---------------" << endl;
			SPageFileGraphic* pf_phy = (SPageFileGraphic*)m_graphics.mapFileBuffer;
			printData("packetID ", pf_phy->packetId);
			printData("STATUS ", pf_phy->status);
			printData("session", pf_phy->session);
			printData("completed laps", pf_phy->completedLaps);
			printData("position", pf_phy->position);
			printData("current time s", pf_phy->currentTime);
			printData("current time", pf_phy->iCurrentTime);
			printData("last time s", pf_phy->lastTime);
			printData("last time ", pf_phy->iLastTime);
			printData("best time s", pf_phy->bestTime);
			printData("best time", pf_phy->iBestTime);
			printData("sessionTimeLeft", pf_phy->sessionTimeLeft);
			printData("distanceTraveled", pf_phy->distanceTraveled);
			printData("isInPit", pf_phy->isInPit);
			printData("currentSectorIndex", pf_phy->currentSectorIndex);
			printData("lastSectorTime", pf_phy->lastSectorTime);
			printData("numberOfLaps", pf_phy->numberOfLaps);
			wcout << "TYRE COMPOUND : " << pf_phy->tyreCompound << endl;
			printData("replayMult", pf_phy->replayTimeMultiplier);
			printData("normalizedCarPosition", pf_phy->normalizedCarPosition);
			printData2("carCoordinates", pf_phy->carCoordinates);
		}


		if (GetAsyncKeyState(0x33) != 0) // user pressed 3
		{
			wcout << "---------------STATIC INFO---------------" << endl;
			SPageFileStatic* pf_phy = (SPageFileStatic*)m_static.mapFileBuffer;
			wcout << "SM VERSION " << pf_phy->smVersion << endl;
			wcout << "AC VERSION " << pf_phy->acVersion << endl;

			printData("number of sessions ", pf_phy->numberOfSessions);
			printData("numCars", pf_phy->numCars);
			wcout << "Car model " << pf_phy->carModel << endl;
			wcout << "Car track " << pf_phy->track << endl;
			wcout << "Player Name " << pf_phy->playerName << endl;
			printData("sectorCount", pf_phy->sectorCount);

			printData("maxTorque", pf_phy->maxTorque);
			printData("maxPower", pf_phy->maxPower);
			printData("maxRpm", pf_phy->maxRpm);
			printData("maxFuel", pf_phy->maxFuel);
			printData("suspensionMaxTravel", pf_phy->suspensionMaxTravel);
			printData("tyreRadius", pf_phy->tyreRadius);

		}
	}

	dismiss(m_graphics);
	dismiss(m_physics);
	dismiss(m_static);

	return 0;
}