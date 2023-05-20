#pragma once

typedef int R_3D_MeshID;

void R_3D_Init();

void R_3D_ApplyProjection(const float fov, const float aspectRatio, const float nearClip, const float farClip);
void R_3D_SetViewpoint(const float x, const float y, const float z, const float pitch, const float yaw);

void R_3D_DrawLine(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const R_ColorRGBA color);
void R_3D_DrawWireBox(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const R_ColorRGBA colorr);

R_3D_MeshID R_3D_LoadMesh(const char *path, const float scale);
void R_3D_DrawMesh(const float x, const float y, const float z, const float pitch, const float yaw, const float roll, const R_ColorRGBA color, const R_3D_MeshID modelID, const R_TexID texID);
