#pragma once

typedef int R_TexID;
typedef unsigned int R_ColorRGBA;

void R_Config();
void R_PostConfig();
void R_CheckError();

void R_SetViewport(const int x, const int y, const int width, const int height);

void R_SetClearColor(const float r, const float g, const float b);
void R_Clear(const bool color, const bool depth);

void R_Present();

void R_ResetView();

R_TexID R_LoadTex(const char *path, const bool filter, const bool clamp, const bool mipmapped);
void R_ApplyTexFilter(const R_TexID texID, const bool filter);
void R_ApplyTexClamp(const R_TexID texID, const bool clamp);
void R_BindTex(const R_TexID texID);
void R_SetTexEnabled(bool enabled);

void R_GetTexSize(const R_TexID texID, unsigned int *w, unsigned int *h);

void R_EnableAlphaTest();
void R_DisableAlphaTest();

void R_EnableDepthTest();
void R_DisableDepthTest();

void R_EnableBackfaceCulling();
void R_DisableBackfaceCulling();

void R_EnableLighting();
void R_DisableLighting();

void R_ToggleLight(const int index, const bool enabled);

void R_SetLightPos(const int index, const vec4 position);
void R_SetLightDiffuse(const int index, const vec4 vec);
void R_SetLightSpecular(const int index, const vec4 vec);
void R_SetLightAmbient(const int index, const vec4 vec);
