#pragma once

void Input_Init();
void Input_HandleScancodeEvent(SDL_Scancode scancode, bool pressed);

bool Input_IsKeyDown(SDL_Scancode scancode);
bool Input_IsKeyJustDown(SDL_Scancode scancode);
bool Input_IsKeyUp(SDL_Scancode scancode);
bool Input_IsKeyJustUp(SDL_Scancode scancode);