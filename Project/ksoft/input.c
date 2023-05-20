#include "includes_base.h"
#include "defines_base.h"
#include "globals_base.h"

typedef struct Input_KeyInfo
{
	int pressTick, releaseTick;
	bool pressed;
} Input_KeyInfo;

Input_KeyInfo keyinfos[SDLK_LAST];

void Input_Init()
{
	// set all ticks to -1 so they aren't automatically pressed on the first update
	for (int i = 0; i < SDLK_LAST; i++)
	{
		keyinfos[i].pressTick = -1;
		keyinfos[i].releaseTick = -1;
	}
}

void Input_HandleScancodeEvent(SDLKey key, bool pressed)
{
	keyinfos[key].pressed = pressed;

	// only need to update the input tick when the key is pressed.
	// Input tick is stored so if you press and release a key in the same update, the key's tick time will be checked first to determine if it should be pressed for at least one update.
	if (pressed)
	{
		keyinfos[key].pressTick = g_inputTick;
	}
	else
	{
		keyinfos[key].releaseTick = g_inputTick;
	}
}

bool Input_IsKeyDown(SDLKey key)
{
	return keyinfos[key].pressTick == g_inputTick || keyinfos[key].pressed;
}

bool Input_IsKeyJustDown(SDLKey key)
{
	return keyinfos[key].pressTick == g_inputTick && keyinfos[key].pressed;
}

bool Input_IsKeyUp(SDLKey key)
{
	return keyinfos[key].releaseTick == g_inputTick || !keyinfos[key].pressed;
}

bool Input_IsKeyJustUp(SDLKey key)
{
	return keyinfos[key].releaseTick == g_inputTick && !keyinfos[key].pressed;
}