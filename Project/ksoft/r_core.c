#include "includes_base.h"
#include "defines_base.h"
#include "globals_base.h"
#include "r_core.h"
#include "epicfail.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#define GL_CHECK_ERRORS _DEBUG

typedef struct R_TexInfo
{
	bool allocated;
	char *filePath;

	unsigned int glTexID;

	unsigned int width;
	unsigned int height;

	bool mipmapped;

} R_TexInfo;

SDL_GLContext glContext;
int colorBits = 24;
int depthBits = 16;
bool mipfilter = true;

#define MAX_TEXTURES 256
R_TexInfo textures[MAX_TEXTURES];
R_TexID nextTexSlot = 0;

float clearR, clearG, clearB;

void R_Init(SDL_Window *window)
{
	if (g_newIni)
	{
		WritePrivateProfileStringA("Renderer", "swap_interval", "0", ENGINE_INI_PATH);
		WritePrivateProfileStringA("Renderer", "color_bits", "24", ENGINE_INI_PATH);
		WritePrivateProfileStringA("Renderer", "depth_bits", "16", ENGINE_INI_PATH);
		WritePrivateProfileStringA("Renderer", "mip_filter", "1", ENGINE_INI_PATH);
	}

	memset(textures, 0, sizeof(textures));

	colorBits = GetPrivateProfileIntA("Renderer", "color_bits", 24, ENGINE_INI_PATH);
	depthBits = GetPrivateProfileIntA("Renderer", "depth_bits", 16, ENGINE_INI_PATH);
	mipfilter = GetPrivateProfileIntA("Renderer", "mip_filter", 1, ENGINE_INI_PATH);

	// Settings
	
	// request opengl 1.1
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	
	if (colorBits == 16)
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	}

	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthBits);

	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	glContext = SDL_GL_CreateContext(window);
	CheckFail(glContext == NULL, 0, "R_Init OpenGL context creation failed", NULL, 0);

	int swap_interval = GetPrivateProfileIntA("Renderer", "swap_interval", 0, ENGINE_INI_PATH);
	
	SDL_GL_SetSwapInterval(swap_interval);

	glShadeModel(GL_FLAT);
	//glEnable(GL_MULTISAMPLE);

	R_CheckError();

	printf("R_Init success!\n");
	printf("########################################\n");
	printf("GL Ver : %s\nVendor : %s\n", glGetString(GL_VERSION), glGetString(GL_VENDOR));
	printf("########################################\n");
}

__forceinline void R_CheckError()
{
#if GL_CHECK_ERRORS
	int glError = glGetError();
	if (glError != GL_NO_ERROR)
	{
		const char *errstr = gluErrorString(glError);
		CheckFail(glError, GL_NO_ERROR, "OpenGL error", errstr, FAILFLAG_CANCONTINUE);
	}
#endif
}

void R_SetViewport(const int x, const int y, const int width, const int height)
{
	glViewport(x, y, width, height);
	R_CheckError();
}

void R_SetClearColor(const float r, const float g, const float b)
{
	clearR = r;
	clearG = g;
	clearB = b;
}

void R_Clear(const bool color, const bool depth)
{
	int clearbits = (color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0);

	if (color)
		glClearColor(clearR, clearG, clearB, 1.0f);

	if (depth)
		glClearDepth(1);

	glClear(clearbits);

	R_CheckError();
}

void R_Present(SDL_Window *window)
{
	// display to screen
	SDL_GL_SwapWindow(window);
	R_CheckError();
}

void R_ResetView()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// A texture ID is returned to make the renderer slightly more abstract. not allowed to directly touch any graphics resources
R_TexID R_LoadTex(const char *path, const bool filter, const bool clamp, const bool mipmapped)
{
	CheckFail(path == NULL, 0, "R_LoadTex path is null", path, FAILFLAG_CANCONTINUE);

	CheckFail(file_exists(path), 1, "R_LoadTex file doesn't exist", path, 0);

	// check if this was already loaded
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		if (textures[i].allocated)
		{
			if (strcmp(path, textures[i].filePath) == 0)
			{
				// give the existing id
				return i;
			}
		}
	}

	SDL_Surface *surf = IMG_Load(path);
	CheckFail(surf == NULL, 0, "R_LoadTex surf is null", path, FAILFLAG_CANCONTINUE);

	char *pixels = malloc(surf->w * surf->h * 4);

	int convcode = SDL_ConvertPixels(
		surf->w, surf->h,
		surf->format->format,
		surf->pixels,
		surf->pitch,
		SDL_PIXELFORMAT_ABGR8888,
		pixels,
		surf->w * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ABGR8888));
	CheckFail(convcode, 0, "R_LoadTex pixel conversion failed", SDL_GetError(), FAILFLAG_CANCONTINUE);


	unsigned int glTexID;

	glGenTextures(1, &glTexID);
	R_CheckError();

	R_TexID slot = nextTexSlot++;
	textures[slot].glTexID = glTexID;
	textures[slot].width = surf->w;
	textures[slot].height = surf->h;
	textures[slot].allocated = true;
	textures[slot].mipmapped = mipmapped;
	
	// keep track of already loaded textures
	textures[slot].filePath = (char*)malloc(strlen(path) + 1);
	strcpy(textures[slot].filePath, path);

	glBindTexture(GL_TEXTURE_2D, glTexID);
	R_CheckError();

	//gluBuild2DMipmaps()

	int texformat = (colorBits == 24) ? GL_RGBA : GL_RGB5_A1;
	if (mipmapped)
	{
		gluBuild2DMipmaps(GL_TEXTURE_2D, texformat, surf->w, surf->h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, texformat, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}

	GLfloat color[4] = { 0,0,0,1 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	
	R_CheckError();

	R_ApplyTexFilter(slot, filter);
	R_ApplyTexClamp(slot, clamp);

	SDL_FreeSurface(surf);
	free(pixels);

	return slot;
}

void R_BindTex(const R_TexID texID)
{
	unsigned int glTexID = textures[texID].glTexID;
	glBindTexture(GL_TEXTURE_2D, glTexID);
	//boundTexID = texID;
	R_CheckError();
}

// Sets if filtering is enabled or disable for the current texture
void R_ApplyTexFilter(const R_TexID texID, const bool filter)
{
	R_BindTex(texID);

	//int filtermode;

	if (textures[texID].mipmapped)
	{
		if (filter)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipfilter ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}
	else
	{
		if (filter)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}

	R_CheckError();
}

// Sets if clamping is enabled for the current texture
void R_ApplyTexClamp(const R_TexID texID, const bool clamp)
{
	R_BindTex(texID);

	int wrapmode = GL_REPEAT;
	if (clamp)
		wrapmode = GL_CLAMP;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapmode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapmode);

	R_CheckError();
}

void R_GetTexSize(const R_TexID texID, unsigned int *w, unsigned int *h)
{
	*w = textures[texID].width;
	*h = textures[texID].height;
}

void R_EnableAlphaTest()
{
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5);
}

void R_DisableAlphaTest()
{
	glDisable(GL_ALPHA_TEST);
}

void R_EnableDepthTest()
{
	glEnable(GL_DEPTH_TEST);
}

void R_DisableDepthTest()
{
	glDisable(GL_DEPTH_TEST);
}

void R_EnableBackfaceCulling()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void R_DisableBackfaceCulling()
{
	glDisable(GL_CULL_FACE);
}

void R_EnableLighting()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
}

void R_DisableLighting()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
}

void R_ToggleLight(const int index, const bool enabled)
{
	int glLightIndex = GL_LIGHT0 + index;

	if (enabled)
	{
		glEnable(glLightIndex);
	}
	else
	{
		glDisable(glLightIndex);
	}

	R_CheckError();
}

void R_SetLightPos(const int index, const vec4 position)
{
	int glLightIndex = GL_LIGHT0 + index;
	glLightfv(glLightIndex, GL_POSITION, position);
	R_CheckError();
}

void R_SetLightDiffuse(const int index, const vec4 vec)
{
	int glLightIndex = GL_LIGHT0 + index;
	glLightfv(glLightIndex, GL_DIFFUSE, vec);
	R_CheckError();
}

void R_SetLightSpecular(const int index, const vec4 vec)
{
	int glLightIndex = GL_LIGHT0 + index;
	glLightfv(glLightIndex, GL_SPECULAR, vec);
	R_CheckError();
}

void R_SetLightAmbient(const int index, const vec4 vec)
{
	int glLightIndex = GL_LIGHT0 + index;
	glLightfv(glLightIndex, GL_AMBIENT, vec);
	R_CheckError();
}