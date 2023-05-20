#pragma once

void Input_Init();
void Input_HandleScancodeEvent(SDLKey key, bool pressed);

bool Input_IsKeyDown(SDLKey key);
bool Input_IsKeyJustDown(SDLKey key);
bool Input_IsKeyUp(SDLKey key);
bool Input_IsKeyJustUp(SDLKey key);