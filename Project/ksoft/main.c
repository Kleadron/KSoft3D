#include "includes_base.h"
#include "globals_base.h"
#include "defines_base.h"
#include "epicfail.h"
#include "r_core.h"
#include "r_2d.h"
#include "r_3d.h"
#include "map.h"
#include "input.h"
#include <sys/stat.h>

// private variabl;es....
bool running = true;
int screenWidth = 720, screenHeight = 480;
SDL_Window *window;
float updateFrequency = 1.0f / 30.0f;
int maxFrameUpdates = 2;
bool fullscreen = false;
int frameCounterTracker = 0;
float frameCounterTimer = 0;
int displayFPS = 0;

float	cpuUpdateTimeMS = 0,
		cpuDrawTimeMS = 0,
		totalDrawTimeMS = 0,
		totalLoopTimeMS = 0;

bool showdebug = false;

// Some globals
bool g_newIni = false;
float g_updateDelta, g_updateTotal, g_frameDelta, g_frameTotal;
float g_interp; // interpolation value 0-1 from previous to current update
int g_inputTick;

// functions
bool file_exists(const char *filename)
{
	struct stat   buffer;
	return (stat(filename, &buffer) == 0);
}

//float lerp(float a, float b, float f)
//{
//	return a * (1.0 - f) + (b * f);
//}

int sampleRate = 44100;

void InitAudio()
{
	if (g_newIni)
	{
		WritePrivateProfileStringA("Audio", "sample_rate", "44100", ENGINE_INI_PATH);
	}

	sampleRate = GetPrivateProfileIntA("Audio", "sample_rate", 44100, ENGINE_INI_PATH);

	int result = Mix_OpenAudio(sampleRate, AUDIO_S16SYS, 2, 512);
	if (result < 0)
		CheckFail(result, 0, "InitAudio: Mix_OpenAudio failed", Mix_GetError(), 0);

	result = Mix_AllocateChannels(16);
	if (result < 0)
		CheckFail(result, 0, "InitAudio: Mix_AllocateChannels failed", Mix_GetError(), 0);
}

void HandleInputKey(SDL_KeyCode key, bool pressed)
{
	if (pressed)
	{
		switch (key)
		{
		case SDLK_F3:
			showdebug = !showdebug;
			break;
		case SDLK_F11:
			fullscreen = !fullscreen;
			if (fullscreen)
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
			else
				SDL_SetWindowFullscreen(window, 0);
			break;
		}
	}


}

void UpdateEvents()
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		case SDL_WINDOWEVENT:
			if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				screenWidth = ev.window.data1;
				screenHeight = ev.window.data2;
			}
			break;
		case SDL_KEYDOWN:
			HandleInputKey(ev.key.keysym.sym, true);
			Input_HandleScancodeEvent(ev.key.keysym.scancode, true);
			break;
		case SDL_KEYUP:
			HandleInputKey(ev.key.keysym.sym, false);
			Input_HandleScancodeEvent(ev.key.keysym.scancode, false);
			break;
		}
	}
}

Uint64 startFrameCounter, lastFrameCounter;
Uint64 startUpdateCounter, lastUpdateCounter;

float squarepos, lastsquarepos;
float cubeTime, lastCubeTime;
R_TexID tex_test, tex_crosshair;
R_3D_MeshID mesh_test;

vec3 camViewpoint = {0.5f, 2.0f, 0.5f};
vec3 lastCamViewpoint = { 0.5f, 2.0f, 0.5f };
float camPitch = -15;
float lastCamPitch = -15;
float camYaw = 225;
float lastCamYaw = 225;

void GameLoad()
{
	tex_test = R_LoadTex("content/images/kyo_test.png", true, true);
	tex_crosshair = R_LoadTex("content/images/crosshair_default.png", false, false);
	mesh_test = R_3D_LoadMesh("content/meshes/funkybox.obj", 1.0f);

	//Mix_Music *music = Mix_LoadMUS("music/storm_6_alt_2.mp3");
	//Mix_PlayMusic(music, -1);

	for (int x = 0; x < Map_GetWidth(); x++)
	{
		for (int z = 0; z < Map_GetLength(); z++)
		{
			Map_Set(x, 0, z, 1);
		}
	}
}

void GameUpdate()
{
	lastsquarepos = squarepos;
	squarepos = (screenWidth / 2) + sinf(g_updateTotal) * (screenWidth / 2);
	
	lastCubeTime = cubeTime;
	cubeTime = g_updateTotal * 1.0f;

	lastCamPitch = camPitch;
	lastCamYaw = camYaw;

	if (Input_IsKeyDown(SDL_SCANCODE_UP))
	{
		camPitch += 90 * g_updateDelta;
	}
	if (Input_IsKeyDown(SDL_SCANCODE_DOWN))
	{
		camPitch -= 90 * g_updateDelta;
	}
	if (Input_IsKeyDown(SDL_SCANCODE_LEFT))
	{
		camYaw += 90 * g_updateDelta;
	}
	if (Input_IsKeyDown(SDL_SCANCODE_RIGHT))
	{
		camYaw -= 90 * g_updateDelta;
	}

	vec3 forward = { -sinf(glm_rad(camYaw)), 0, -cosf(glm_rad(camYaw))};
	vec3 left = { -sinf(glm_rad(camYaw) + CGLM_PI_2), 0, -cosf(glm_rad(camYaw) + CGLM_PI_2) };

	vec3 moveTarget = {0, 0, 0};

	if (Input_IsKeyDown(SDL_SCANCODE_W))
	{
		glm_vec3_add(moveTarget, forward, moveTarget);
	}
	if (Input_IsKeyDown(SDL_SCANCODE_S))
	{
		glm_vec3_sub(moveTarget, forward, moveTarget);
	}
	if (Input_IsKeyDown(SDL_SCANCODE_A))
	{
		glm_vec3_add(moveTarget, left, moveTarget);
	}
	if (Input_IsKeyDown(SDL_SCANCODE_D))
	{
		glm_vec3_sub(moveTarget, left, moveTarget);
	}

	glm_vec3_normalize(moveTarget);

	if (Input_IsKeyDown(SDL_SCANCODE_Q))
	{
		moveTarget[1] -= 1;
	}
	if (Input_IsKeyDown(SDL_SCANCODE_E))
	{
		moveTarget[1] += 1;
	}


	glm_vec3_scale(moveTarget, g_updateDelta * 4, moveTarget);

	glm_vec3_copy(camViewpoint, lastCamViewpoint);
	glm_vec3_add(camViewpoint, moveTarget, camViewpoint);
}

void DisplayGameTiming(int x, int y)
{
	R_2D_DrawDebugStringF(x, y, COLOR_WHITE, 
		"### GAME TIMING ###\nFrame Delta %1.3f\nFrame Total %1.3f\nUpdate Delta %1.3f\nUpdate Total %1.3f\nInterp %1.3f", 
		g_frameDelta, g_frameTotal, g_updateDelta, g_updateTotal, g_interp);
}

void DisplayPerformance(int x, int y)
{
	R_2D_DrawDebugStringF(x, y, COLOR_WHITE,
		"### PERFORMANCE ###\nApprox FPS: %i\nCPU Update: %1.3f ms\nCPU Draw: %1.3f ms\nTotal Draw: %1.3f ms\nGame Loop: %1.3f ms",
		displayFPS, cpuUpdateTimeMS, cpuDrawTimeMS, totalDrawTimeMS, totalLoopTimeMS);
}

void GameRender()
{
	R_EnableBackfaceCulling();
	R_EnableDepthTest();
	R_ResetView();
	R_3D_ApplyProjection(80, (float)screenWidth / (float)screenHeight, 0.05f, 200);

	vec3 frameViewpoint = {0, 0, 0};

	glm_vec3_lerp(lastCamViewpoint, camViewpoint, g_interp, frameViewpoint);

	R_3D_SetViewpoint(frameViewpoint[0], frameViewpoint[1], frameViewpoint[2], 
		glm_lerp(lastCamPitch, camPitch, g_interp), glm_lerp(lastCamYaw, camYaw, g_interp));

	R_EnableLighting();
	
	R_ToggleLight(0, true);
	R_SetLightPos(0, (vec4) { 0, 1, 1, 0 });
	R_SetLightDiffuse(0, (vec4) { 1, 1, 1, 1 });
	R_SetLightAmbient(0, (vec4) { 0.25f, 0.25f, 0.25f, 1 });
	//R_SetLightSpecular(0, (vec4) { 0.5f, 0.5f, 0.5f, 1 });

	/*R_3D_DrawModel(0, 0, -3, (g_frameTotal * 90) - 4, (g_frameTotal * 75) - 4, MAKE_RGBA(128, 128, 128, 255), mdl_test);
	R_3D_DrawModel(0, 0, -3, (g_frameTotal * 90) - 2, (g_frameTotal * 75) - 2, MAKE_RGBA(192, 192, 192, 255), mdl_test);
	R_3D_DrawModel(0, 0, -3, g_frameTotal * 90, g_frameTotal * 75, COLOR_WHITE, mdl_test);*/

	float cubeInterp = glm_lerp(lastCubeTime, cubeTime, g_interp);
	R_3D_DrawMesh(3, 2, 3, cubeInterp * 90, cubeInterp * 75, 0, COLOR_WHITE, mesh_test, 0);

	

	R_DisableLighting();

	R_3D_DrawWireBox(1, 1.1f, 1, 5, 3, 5, MAKE_RGBA(0, 255, 255, 255));

	Map_Draw();


	R_DisableDepthTest();

	R_ResetView();
	R_2D_ApplyProjection(0, screenWidth, screenHeight, 0);
	/*R_2D_DrawColorRect_WH(10, 10, 10, 10, MAKE_RGBA(255, 100, 100, 255));
	R_2D_DrawColorRect_WH(30, 10, 10, 10, MAKE_RGBA(100, 255, 100, 255));
	R_2D_DrawColorRect_WH(50, 10, 10, 10, MAKE_RGBA(100, 100, 255, 255));
	R_2D_DrawTexRect_WH(10, 30, 256, 256, COLOR_WHITE, tex_test);*/

	//R_2D_DrawDebugString(10, 300, COLOR_WHITE, "Hi =]");

	int debugX = 10;
	int debugY = 10;

	if (showdebug)
	{
		R_2D_DrawColorRect_WH(debugX - 4, debugY - 4, 190, 130, COLOR_BLACK);
		DisplayPerformance(debugX, debugY);
		DisplayGameTiming(debugX, debugY + 70);
	}
	
	R_2D_DrawTexRect_WH(screenWidth / 2 - 16, screenHeight / 2 - 16, 32, 32, COLOR_WHITE, tex_crosshair);
}

// MAIN!!!! :D
int main(int argc, char **argv)
{
	printf("KSoft3D (c) Kleadron Software 2023\n");
	printf("This is IN-DEVELOPMENT software and may not work correctly.\nYOU HAVE BEEN WARNED!\n");
#if _DEBUG
	printf("Built with Debug configuration.\n");
#endif

	printf("\n");

	printf("starting game engine\n");

	// config file setup
	if (!file_exists(ENGINE_INI_PATH))
	{
		g_newIni = true;
		printf("creating new config file\n");
	}
	
	if (g_newIni)
	{
		char tempstr[127];
		WritePrivateProfileStringA("Window", "width", _itoa(screenWidth, tempstr, 10), ENGINE_INI_PATH);
		WritePrivateProfileStringA("Window", "height", _itoa(screenHeight, tempstr, 10), ENGINE_INI_PATH);
	}

	screenWidth = GetPrivateProfileIntA("Window", "width", screenWidth, ENGINE_INI_PATH);
	screenHeight = GetPrivateProfileIntA("Window", "height", screenHeight, ENGINE_INI_PATH);

	printf("initializing SDL\n");

	// SDL initialization
	int initcode = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	if (initcode != 0)
	{
		CheckFail(initcode, 0, "SDL init failed", SDL_GetError(), 0);
	}

	int windowflags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	window = SDL_CreateWindow("KSoft3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, windowflags);
	CheckFail(window == NULL, 0, "Window creation failed", NULL, 0);

	printf("done...\n");

	printf("initializing audio");
	InitAudio();
	printf("done...\n");

	printf("rasterizer init\n");

	// renderer init
	R_Init(window);
	R_3D_Init();

	printf("done...\n");

	printf("load default resources\n");

	// preload placeholders
	R_LoadTex("content/images/placeholder.png", false, false);
	R_3D_LoadMesh("content/meshes/placeholder.obj", 1);

	R_2D_LoadDebugFont("content/images/asciifont2.png");

	printf("done...\n");

	R_EnableAlphaTest();

	printf("map init\n");

	Map_Init();
	Map_Create(32, 16, 32);

	printf("done...\n");

	Input_Init();

	// done initializing
	printf("inited :)\n");

	printf("game load\n");

	GameLoad();

	printf("start main loop\n");

	// reset counters
	startUpdateCounter = startFrameCounter = lastUpdateCounter = lastFrameCounter = SDL_GetPerformanceCounter();
	Uint64 perfFreq = SDL_GetPerformanceFrequency();

	while (running)
	{
		Uint64 loopStartCounter = SDL_GetPerformanceCounter();

		UpdateEvents();

		Uint64 currentCounter = SDL_GetPerformanceCounter();

		g_frameDelta = (currentCounter - lastFrameCounter) / (float)perfFreq;
		g_frameTotal = (currentCounter - startFrameCounter) / (float)perfFreq;

		frameCounterTracker++;
		frameCounterTimer += g_frameDelta;

		if (frameCounterTimer >= 1.0f)
		{
			displayFPS = frameCounterTracker;
			frameCounterTracker = 0;
			frameCounterTimer -= 1.0f;
		}

		lastFrameCounter = currentCounter;

		g_updateDelta = updateFrequency;
		Uint64 addCounter = (Uint64)(updateFrequency * (double)perfFreq);

		Uint64 cpuUpdateStartCounter = SDL_GetPerformanceCounter();

		for (int i = 0; i < maxFrameUpdates; i++)
		{
			//float elapsedSinceLastUpdate = (currentCounter - lastUpdateCounter) / (float)SDL_GetPerformanceFrequency();
			if (currentCounter >= lastUpdateCounter + addCounter)
			{
				lastUpdateCounter += addCounter;
				g_updateTotal = (lastUpdateCounter - startUpdateCounter) / (float)perfFreq;

				// tick
				GameUpdate();
				g_inputTick++;
			}
		}

		while (currentCounter >= lastUpdateCounter + addCounter)
		{
			lastUpdateCounter += addCounter;
			startUpdateCounter += addCounter;
		}

		Uint64 cpuUpdateEndCounter = SDL_GetPerformanceCounter();

		g_interp = ((currentCounter - lastUpdateCounter) / (float)perfFreq) / updateFrequency;

		// clamp the interpolation value
		if (g_interp > 1.0f)
			g_interp = 1.0f;
		
		Uint64 drawStartCounter = SDL_GetPerformanceCounter();

		R_SetViewport(0, 0, screenWidth, screenHeight);
		R_SetClearColor(0.1f, 0.15f, 0.25f);
		R_Clear(true, true);

		GameRender();

		Uint64 cpuDrawEndCounter = SDL_GetPerformanceCounter();

		// frame is done, present all the contents to the window
		R_Present(window);

		Uint64 totalDrawEndCounter = SDL_GetPerformanceCounter();

		Uint64 loopEndCounter = SDL_GetPerformanceCounter();

		// count numbers
		cpuUpdateTimeMS = ((cpuUpdateEndCounter - cpuUpdateStartCounter) / (float)perfFreq) * 1000.0f;
		cpuDrawTimeMS = ((cpuDrawEndCounter - drawStartCounter) / (float)perfFreq) * 1000.0f;
		totalDrawTimeMS = ((totalDrawEndCounter - drawStartCounter) / (float)perfFreq) * 1000.0f;
		totalLoopTimeMS = ((loopEndCounter - loopStartCounter) / (float)perfFreq) * 1000.0f;
	}




	printf("exited :(\n");

	return EXIT_SUCCESS;
}
