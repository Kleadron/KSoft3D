#pragma once

#define ENGINE_INI_PATH ".\\engine.ini"
#define USER_INI_PATH ".\\user.ini"

#define MAKE_RGBA(r, g, b, a) (R_ColorRGBA)((r) | (g << 8) | (b << 16) | (a << 24))
#define COLOR_WHITE MAKE_RGBA(255, 255, 255, 255)
#define COLOR_BLACK MAKE_RGBA(0, 0, 0, 255)

#define GET_COLOR_R(color) (unsigned int)(color & 255)
#define GET_COLOR_G(color) (unsigned int)((color >> 8) & 255)
#define GET_COLOR_B(color) (unsigned int)((color >> 16) & 255)
#define GET_COLOR_A(color) (unsigned int)((color >> 24) & 255)

//#define LERP(a, b, f) (a * (1.0f - f) + (b * f))

#define POS_X 0
#define POS_Y 1
#define POS_Z 2

#define ANGLE_PITCH 0
#define ANGLE_YAW 1 
#define ANGLE_ROLL 2