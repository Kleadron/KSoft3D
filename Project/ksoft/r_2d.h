#pragma once

void R_2D_LoadDebugFont(const char *imgPath);

void R_2D_ApplyProjection(const int left, const int right, const int bottom, const int top);

void R_2D_DrawColorRect(const int x1, const int y1, const int x2, const int y2, const R_ColorRGBA rgba);
void R_2D_DrawColorRect_WH(const int x, const int y, const int w, const int h, const R_ColorRGBA rgba);
void R_2D_DrawTexRect(const int x1, const int y1, const int x2, const int y2, const R_ColorRGBA rgba, const R_TexID tex);
void R_2D_DrawTexRect_WH(const int x, const int y, const int w, const int h, const R_ColorRGBA rgba, const R_TexID tex);

void R_2D_DrawDebugString(const int x, const int y, const R_ColorRGBA color, const char *str);

extern char R_2D_tempTextBuffer[8191];
// wish I didn't have to do this but oh well. :D
#define R_2D_DrawDebugStringF(x,y,color,text,...) {sprintf_s((char*)&R_2D_tempTextBuffer, sizeof(R_2D_tempTextBuffer), text, __VA_ARGS__);	R_2D_DrawDebugString(x, y, color, R_2D_tempTextBuffer);}