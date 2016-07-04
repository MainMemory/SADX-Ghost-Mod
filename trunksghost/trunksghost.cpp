// trunksghost.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <list>
#include <vector>
#include <fstream>
#include "SADXModLoader.h"
using namespace std;

struct GhostData
{
	char segment;
	char padding;
	short animation;
	float animationframe;
	NJS_VECTOR position;
	Rotation rotation;
};

NJS_OBJECT **___SONIC_OBJECTS;

NJS_TEXLIST *const TextureLists[] = {
	(NJS_TEXLIST *)0x91CB58,
	(NJS_TEXLIST *)0x892A20,
	(NJS_TEXLIST *)0x91A9C8,
	(NJS_TEXLIST *)0x91BD20,
	(NJS_TEXLIST *)0x8CB470,
	(NJS_TEXLIST *)0x91C800,
	(NJS_TEXLIST *)0x91C560,
	(NJS_TEXLIST *)0x91C910,
	(NJS_TEXLIST *)0x91CBC0
};

AnimData *const AnimDataList[] = {
	(AnimData *)0x3C56210,
	(AnimData *)0x38F3CE0,
	(AnimData *)0x3C49D90,
	(AnimData *)0x3C532A0,
	(AnimData *)0x38F38F8,
	(AnimData *)0x3C54880,
	(AnimData *)0x3C53FA8,
	(AnimData *)0x3C556A0,
	(AnimData *)0x3C56210
};

string savepath = "savedata\\ghost";

const unsigned int magic = 'TSOG';
const char *const previoustimefmt = "Best Time:   %02d:%02d.%02d";
const char *const yourtimefmt =     "Your Time:   %02d:%02d.%02d";
const char *const differencefmt =   "Difference: %c%02d:%02d.%02d";

const char centiseconds[] = {
	0, 2, 3, 5, 7, 8, 10, 12, 13, 15, 17, 18, 20, 22, 23, 25, 27, 28, 30, 32,
	33, 35, 37, 38, 40, 42, 43, 45, 47, 48, 50, 52, 53, 55, 57, 58, 60, 62, 63, 65,
	67, 68, 70, 72, 73, 75, 77, 78, 80, 82, 83, 85, 87, 88, 90, 92, 93, 95, 97, 99
};

unsigned int oldghostlength = -1;
unsigned int newghostlength = 0;
unsigned int currentframe = 0;
unsigned int savedframe = 0;
list<vector<GhostData>> ghosts;
vector<GhostData> newghost;
int numghosts;
bool levelstarted = false;
bool levelcomplete = false;

void FramesToTime(unsigned int frames, unsigned char &min, unsigned char &sec, unsigned char &fr)
{
	fr = frames % 60;
	frames /= 60;
	sec = frames % 60;
	min = frames / 60;
}

DataPointer(int, dword_3ABD9CC, 0x3ABD9CC);
DataPointer(float, dword_3ABD9C0, 0x3ABD9C0);
FunctionPointer(void, sub_407040, (NJS_OBJECT *, NJS_MOTION *, float), 0x407040);
FunctionPointer(void, sub_407020, (NJS_ACTION *, float), 0x407020);
FunctionPointer(void, sub_405470, (NJS_ACTION *, float, int), 0x405470);
FunctionPointer(void, sub_412420, (int), 0x412420);
FunctionPointer(void, sub_4128A0, (float, float, float, float), 0x4128A0);
VoidFunc(sub_439520, 0x439520);
VoidFunc(sub_4128F0, 0x4128F0);
VoidFunc(sub_439540, 0x439540);
FunctionPointer(void, sub_439560, (int, int), 0x439560);
FunctionPointer(void, sub_4030D0, (int, int), 0x4030D0);
void __cdecl Ghost_Display(ObjectMaster *obj)
{
	if (levelcomplete)
	{
		unsigned char min, sec, fr;
		SetDebugFontColor(-1);
		if (oldghostlength != -1)
		{
			FramesToTime(oldghostlength, min, sec, fr);
			DisplayDebugStringFormatted(0, previoustimefmt, min, sec, centiseconds[fr]);
		}
		FramesToTime(newghostlength, min, sec, fr);
		DisplayDebugStringFormatted(oldghostlength != -1 ? 1 : 0, yourtimefmt, min, sec, centiseconds[fr]);
		if (oldghostlength != -1)
		{
			unsigned int dif;
			char sign;
			if (oldghostlength < newghostlength)
			{
				SetDebugFontColor(0xFFFF0000);
				sign = '+';
				dif = newghostlength - oldghostlength;
			}
			else
			{
				SetDebugFontColor(0xFF00FF00);
				sign = '-';
				dif = oldghostlength - newghostlength;
			}
			FramesToTime(dif, min, sec, fr);
			DisplayDebugStringFormatted(2, differencefmt, sign, min, sec, centiseconds[fr]);
			SetDebugFontColor(-1);
		}
	}
#ifdef verbose
	else
	{
		unsigned char min, sec, fr;
		SetDebugFontColor(-1);
		FramesToTime(currentframe, min, sec, fr);
		DisplayDebugStringFormatted(0, yourtimefmt, min, sec, centiseconds[fr]);
	}
#endif
	if (!GetCharacterObject(0))
		return;
	for (auto curghost = ghosts.cbegin(); curghost != ghosts.cend(); curghost++)
	{
		int i = currentframe;
		if (currentframe >= curghost->size())
			i = curghost->size() - 1;
		const GhostData *ghost = &(*curghost)[i];
		if (ghost->segment != CurrentAct)
			continue;
		int character = CurrentCharacter;
		if (character == Characters_Sonic && MetalSonicFlag)
			character = Characters_MetalSonic;
		NJS_TEXLIST *tex = TextureLists[character];
		bool super = CurrentCharacter == Characters_Sonic && (ghost->animation >= 134 && ghost->animation <= 145);
		if (super)
			tex = (NJS_TEXLIST *)0x142272C;
		njSetTexture(tex);
		NJS_ACTION *anim = AnimDataList[character][ghost->animation].Animation;
		sub_439520();
		sub_439560(0, 1048576);
		sub_4030D0(0, 8);
		sub_4030D0(1, 10);
		sub_4128A0(1.0, 1.0, 1.0, 1.0);
		sub_412420(super ? 4 : 2);
		njPushMatrix(nullptr);
		njTranslateV(nullptr, (NJS_VECTOR *)&ghost->position);
		if (ghost->rotation.z)
			njRotateZ(nullptr, (unsigned __int16)ghost->rotation.z);
		if (ghost->rotation.x)
			njRotateX(nullptr, (unsigned __int16)ghost->rotation.x);
		if (ghost->rotation.y != 0x8000)
			njRotateY(nullptr, (unsigned __int16)(-32768 - LOWORD(ghost->rotation.y)));
		njPushMatrix(nullptr);
		njTranslate(nullptr, 0, GetCharObj2(0)->PhysicsData.YOff, 0);
		if (character == Characters_MetalSonic)
		{
			if (anim->object == *___SONIC_OBJECTS)
				sub_407040(___SONIC_OBJECTS[68], anim->motion, ghost->animationframe);
			else if (anim->object == ___SONIC_OBJECTS[66])
				sub_407040(___SONIC_OBJECTS[69], anim->motion, ghost->animationframe);
			else if (anim->object == ___SONIC_OBJECTS[67])
				sub_407040(___SONIC_OBJECTS[70], anim->motion, ghost->animationframe);
			else
				sub_407020(anim, ghost->animationframe);
		}
		else if (dword_3ABD9CC)
		{
			dword_3ABD9C0 = -5952.0f;
			sub_405470(anim, ghost->animationframe, 1);
			dword_3ABD9C0 = 0;
		}
		else
			sub_407020(anim, ghost->animationframe);
		njPopMatrix(2);
		sub_412420(0);
		sub_4030D0(0, 8);
		sub_4030D0(1, 6);
		sub_4128F0();
		sub_439540();
	}
	if (!IsGamePaused())
		currentframe++;
}

void __cdecl Ghost_Main(ObjectMaster *obj)
{
	ObjectMaster *charobj = GetCharacterObject(0);
	if (!charobj || IsGamePaused())
		return;
	if (!levelcomplete)
	{
		EntityData1 *obj1 = charobj->Data1;
		CharObj2 *obj2 = GetCharObj2(0);
		GhostData frame;
		frame.segment = (char)CurrentAct;
		frame.padding = 0;
		frame.animation = obj2->AnimThing.Animation;
		frame.animationframe = obj2->AnimThing.AnimationFrame;
		frame.position = obj1->Position;
		frame.rotation = obj1->Rotation;
		newghost.push_back(frame);
	}
	Ghost_Display(obj);
}

void ResetGhost()
{
#ifdef verbose
	PrintDebug("ResetGhost()\n");
#endif
	newghost.clear();
	ghosts.clear();
	oldghostlength = -1;
	currentframe = 0;
	savedframe = 0;
	levelstarted = false;
	levelcomplete = false;
}

void LoadGhost()
{
#ifdef verbose
	PrintDebug("LoadGhost()\n");
#endif
	switch (CurrentLevel)
	{
	case LevelIDs_HedgehogHammer:
	case LevelIDs_StationSquare:
	case LevelIDs_EggCarrierOutside:
	case LevelIDs_EggCarrierInside:
	case LevelIDs_MysticRuins:
	case LevelIDs_Past:
	case LevelIDs_SkyChase1:
	case LevelIDs_SkyChase2:
	case LevelIDs_SSGarden:
	case LevelIDs_ECGarden:
	case LevelIDs_MRGarden:
	case LevelIDs_ChaoRace:
		return;
	}
	if (!levelstarted || levelcomplete)
	{
		numghosts = 0;
		char path[MAX_PATH];
		for (int i = 1; i <= 999; i++)
		{
			if (i == 1)
				_snprintf_s(path, MAX_PATH, (savepath + "\\%02d%s.gst").c_str(), CurrentLevel, CharIDStrings[CurrentCharacter]);
			else
				_snprintf_s(path, MAX_PATH, (savepath + "\\%02d%s%03d.gst").c_str(), CurrentLevel, CharIDStrings[CurrentCharacter], i);
			ifstream str(path, ifstream::binary);
			if (str.is_open())
			{
				numghosts++;
				unsigned int m;
				str.read((char *)&m, sizeof(m));
				if (m == magic)
				{
					unsigned int length;
					str.read((char *)&length, sizeof(length));
					oldghostlength = min(length, oldghostlength);
					vector<GhostData> ghost(length);
					str.read((char *)ghost.data(), sizeof(GhostData) * length);
					ghosts.push_back(ghost);
				}
				str.close();
			}
			else
				break;
		}
		currentframe = 0;
		levelstarted = true;
	}
	levelcomplete = false;
	ObjectMaster *obj = LoadObject((LoadObj)0, 1, Ghost_Main);
	if (!obj)
	{
		ghosts.clear();
		oldghostlength = -1;
		return;
	}
	obj->DisplaySub = Ghost_Display;
}

void SaveGhost(unsigned int i, const vector<GhostData> &ghost)
{
	char path[MAX_PATH];
	if (i == 0)
		_snprintf_s(path, MAX_PATH, (savepath + "\\%02d%s.gst").c_str(), CurrentLevel, CharIDStrings[CurrentCharacter]);
	else
		_snprintf_s(path, MAX_PATH, (savepath + "\\%02d%s%03d.gst").c_str(), CurrentLevel, CharIDStrings[CurrentCharacter], i + 1);
	ofstream str(path, ifstream::binary);
	if (str.is_open())
	{
		str.write((char *)&magic, sizeof(magic));
		unsigned int size = ghost.size();
		str.write((char *)&size, sizeof(size));
		str.write((char *)ghost.data(), sizeof(GhostData) * size);
		str.close();
	}
}

void __cdecl SaveGhosts()
{
#ifdef verbose
	PrintDebug("SaveGhosts() frame = 0x%08X\n", currentframe);
#endif
	if (newghost.size() < oldghostlength)
	{
		SaveGhost(0, newghost);
		if (oldghostlength != -1)
			SaveGhost(numghosts, *ghosts.cbegin());
	}
	else
		SaveGhost(numghosts, newghost);
	newghostlength = newghost.size();
	newghost.clear();
	levelcomplete = true;
}

FunctionPointer(void, sub_40EFA0, (unsigned __int8), 0x40EFA0);
void __cdecl EndLevel(unsigned __int8 a1)
{
	sub_40EFA0(a1);
	SaveGhosts();
}

FunctionPointer(int, sub_79E400, (int ID, int a2, void *a3), 0x79E400);
int __cdecl EndTwinkleCircuit(int ID, int a2, void *a3)
{
	SaveGhosts();
	return sub_79E400(ID, a2, a3);
}

void Checkpoint()
{
#ifdef verbose
	PrintDebug("Checkpoint() frame = 0x%08X\n", currentframe);
#endif
	savedframe = currentframe;
}

int Restart()
{
#ifdef verbose
	PrintDebug("Restart() frame = 0x%08X\n", savedframe);
#endif
	newghost.resize(savedframe);
	currentframe = savedframe;
	return CheckRestartLevel();
}

VoidFunc(sub_424830, 0x424830);
void ChangeAct()
{
#ifdef verbose
	PrintDebug("ChangeAct() frame = 0x%08X\n", currentframe);
#endif
	savedframe = currentframe;
	sub_424830();
}

const PointerInfo jumps[] = {
	ptrdecl(0x41597B, LoadGhost),
	ptrdecl(0x44EEE1, Checkpoint),
	ptrdecl(0x44ED60, ResetGhost)
};

const PointerInfo calls[] = {
	ptrdecl(0x41481A, Restart),
	ptrdecl(0x415545, EndLevel),
	ptrdecl(0x4DB735, EndTwinkleCircuit),
	ptrdecl(0x4146E0, ChangeAct)
};

extern "C"
{
	__declspec(dllexport) void Init(const char *path, const HelperFunctions &helperFunctions)
	{
		HMODULE handle = GetModuleHandle(L"CHRMODELS_orig");
		___SONIC_OBJECTS = (NJS_OBJECT **)GetProcAddress(handle, "___SONIC_OBJECTS");
		if (helperFunctions.Version >= 4)
			savepath = string(helperFunctions.GetMainSavePath()) + "\\ghost";
		CreateDirectoryA(savepath.c_str(), nullptr);
	}

	__declspec(dllexport) const PointerList Jumps = { arrayptrandlength(jumps) };

	__declspec(dllexport) const PointerList Calls = { arrayptrandlength(calls) };

	__declspec(dllexport) const int SADXModInfo = ModLoaderVer;
}