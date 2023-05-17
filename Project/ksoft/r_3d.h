#pragma once

typedef int R_3D_MeshID;

void R_3D_Init();

R_3D_MeshID R_3D_LoadMesh(const char *path, const float scale);
void R_3D_DrawMesh(float x, float y, float z, float pitch, float yaw, R_ColorRGBA color, R_3D_MeshID modelID, R_TexID texID);

void R_3D_ApplyProjection(const float fov, const float aspectRatio, const float nearClip, const float farClip);
void R_3D_SetViewpoint(float x, float y, float z, float pitch, float yaw);