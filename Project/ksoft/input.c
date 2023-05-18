#include "includes_base.h"
#include "defines_base.h"
#include "globals_base.h"

typedef struct Input_KeyInfo
{
	int pressTick, releaseTick;
	bool pressed;
} Input_KeyInfo;

Input_KeyInfo keyinfos[SDL_NUM_SCANCODES];

void Input_Init()
{
	// set all ticks to -1 so they aren't automatically pressed on the first update
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		keyinfos[i].pressTick = -1;
		keyinfos[i].releaseTick = -1;
	}
}

void Input_HandleScancodeEvent(SDL_Scancode scancode, bool pressed)
{
	keyinfos[scancode].pressed = pressed;

	// only need to update the input tick when the key is pressed.
	// Input tick is stored so if you press and release a key in the same update, the key's tick time will be checked first to determine if it should be pressed for at least one update.
	if (pressed)
	{
		keyinfos[scancode].pressTick = g_inputTick;
	}
	else
	{
		keyinfos[scancode].releaseTick = g_inputTick;
	}
}

bool Input_IsKeyDown(SDL_Scancode scancode)
{
	return keyinfos[scancode].pressTick == g_inputTick || keyinfos[scancode].pressed;
}

bool Input_IsKeyJustDown(SDL_Scancode scancode)
{
	return keyinfos[scancode].pressTick == g_inputTick && keyinfos[scancode].pressed;
}

bool Input_IsKeyUp(SDL_Scancode scancode)
{
	return keyinfos[scancode].releaseTick == g_inputTick || !keyinfos[scancode].pressed;
}

bool Input_IsKeyJustUp(SDL_Scancode scancode)
{
	return keyinfos[scancode].releaseTick == g_inputTick && !keyinfos[scancode].pressed;
}