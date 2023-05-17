#pragma once

typedef unsigned char Map_TileID;

void Map_Init();
void Map_Create(const int width, const int height, const int length);

int Map_GetWidth();
int Map_GetHeight();
int Map_GetLength();

void Map_Set(const int x, const int y, const int z, const Map_TileID id);
Map_TileID Map_Get(const int x, const int y, const int z);

void Map_Draw();