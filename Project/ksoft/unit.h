#pragma once

typedef struct Unit
{

	vec3 pos;


	// privately owned data by this unit
	void *class_data;
} Unit;

void UnitMgr_Init();