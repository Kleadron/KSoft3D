#include "includes_base.h"
#include "defines_base.h"
#include "globals_base.h"
#include "r_core.h"
#include "r_3d.h"
#include "epicfail.h"
#include "map.h"





R_3D_MeshID mesh_block;
R_TexID tex_block;


int mapWidth, mapHeight, mapLength, mapVolume;

Map_TileID *mapIDs;

int GetIndex(const int x, const int y, const int z)
{
	return (z * mapWidth * mapHeight) + (y * mapWidth) + x;
}

bool InBounds(const int x, const int y, const int z)
{
	return x >= 0 && y >= 0 && z >= 0 && x < mapWidth && y < mapHeight && z < mapLength;
}

void Map_Init()
{
	mesh_block = R_3D_LoadMesh("content/meshes/block.obj", 1);
	tex_block = R_LoadTex("content/images/rock.png", false, false, true);
}

void Map_Create(const int width, const int height, const int length)
{
	mapWidth = width;
	mapHeight = height;
	mapLength = length;
	mapVolume = mapWidth * mapHeight * mapLength;

	// deallocate existing map data
	if (mapIDs != NULL)
	{
		free(mapIDs);
	}

	// allocate map data
	mapIDs = calloc(mapVolume, sizeof(Map_TileID));
}

int Map_GetWidth() { return mapWidth; }
int Map_GetHeight() { return mapHeight; }
int Map_GetLength() { return mapLength; }

void Map_Set(const int x, const int y, const int z, const Map_TileID id)
{
	if (InBounds(x, y, z))
	{
		mapIDs[GetIndex(x, y, z)] = id;
	}
}

Map_TileID Map_Get(const int x, const int y, const int z)
{
	if (InBounds(x, y, z))
	{
		return mapIDs[GetIndex(x, y, z)];
	}
	else
	{
		return 0;
	}
}

// if the specificed map tile is surrounded by solid tiles
// TODO: consider creating obscuration map so all the calls don't need to be done
// this is also very slow in debug builds
bool Map_Obscured(const int x, const int y, const int z)
{
	if (InBounds(x, y, z))
	{
		bool obscured =
			Map_Get(x+1, y, z) > 0 &&
			Map_Get(x-1, y, z) > 0 &&
			Map_Get(x, y+1, z) > 0 &&
			Map_Get(x, y-1, z) > 0 &&
			Map_Get(x, y, z+1) > 0 &&
			Map_Get(x, y, z-1) > 0;

		return obscured;
	}
	return false;
}

void Map_Draw()
{
	// TODO: replace with proper tile mesh generation
	for (int x = 0; x < mapWidth; x++)
	{
		for (int y = 0; y < mapHeight; y++)
		{
			for (int z = 0; z < mapLength; z++)
			{
				if (!Map_Obscured(x, y, z) && Map_Get(x, y, z) > 0)
				{
					R_3D_DrawMesh(x + 0.5f, y + 0.5f, z + 0.5f, 0, 0, 0, COLOR_WHITE, mesh_block, tex_block);
				}
			}
		}
	}
}