#include "includes_base.h"
#include "r_core.h"

bool fontLoaded = false;
R_TexID fontTex;
char R_2D_tempTextBuffer[8191];

void R_2D_LoadDebugFont(const char *imgPath)
{
	fontTex = R_LoadTex(imgPath, false, false);
	fontLoaded = true;
}

void R_2D_ApplyProjection(const int left, const int right, const int bottom, const int top)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(left, right, bottom, top);
	glMatrixMode(GL_MODELVIEW);

	R_CheckError();
}


// disables GL_TEXTURE_2D and glBegin's quads
void R_2D_DrawColorRect(const int x1, const int y1, const int x2, const int y2, const R_ColorRGBA rgba)
{
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	glColor4ubv((unsigned char*)&rgba);
	glVertex2i(x1, y1);
	glVertex2i(x1, y2);
	glVertex2i(x2, y2);
	glVertex2i(x2, y1);
	glEnd();

	R_CheckError();
}

void R_2D_DrawColorRect_WH(const int x, const int y, const int w, const int h, const R_ColorRGBA rgba)
{
	R_2D_DrawColorRect(x, y, x + w, y + h, rgba);
}

// enables GL_TEXTURE_2D and glBegin's quads, uses whatever texture was bound beforehand
void R_2D_DrawTexRect(const int x1, const int y1, const int x2, const int y2, const R_ColorRGBA rgba, const R_TexID tex)
{
	glEnable(GL_TEXTURE_2D);
	R_BindTex(tex);

	glBegin(GL_QUADS);
	glColor4ubv((unsigned char*)&rgba);
	glTexCoord2i(0, 0);
	glVertex2i(x1, y1);
	glTexCoord2i(0, 1);
	glVertex2i(x1, y2);
	glTexCoord2i(1, 1);
	glVertex2i(x2, y2);
	glTexCoord2i(1, 0);
	glVertex2i(x2, y1);
	glEnd();

	R_CheckError();
}

void R_2D_DrawTexRect_WH(const int x, const int y, const int w, const int h, const R_ColorRGBA rgba, const R_TexID tex)
{
	R_2D_DrawTexRect(x, y, x + w, y + h, rgba, tex);
}

void R_2D_DrawDebugString(const int x, const int y, const R_ColorRGBA color, const char *str)
{
	if (!fontLoaded)
		return;

	float texScaleX = 1.0f / 128.0f;
	float texScaleY = 1.0f / 64.0f;

	//glBindTexture(GL_TEXTURE_2D, glFontTex);

	glEnable(GL_TEXTURE_2D);
	R_BindTex(fontTex);

	glColor4ubv((unsigned char*)&color);

	glBegin(GL_QUADS);

	int drawX = x;
	int drawY = y;

	int srcX = 0;
	int srcY = 0;

	int textIndex = 0;
	while (true)
	{
		char c = str[textIndex++];
		if (c == '\0')
			break;

		if (c == ' ')
		{
			drawX += 8;
			continue;
		}

		if (c == '\n')
		{
			drawX = x;
			drawY += 9;
			continue;
		}

		// replace unknown chars
		if (c < 32 || c >= 128)
			c = '?';

		srcX = (c % 16) * 8;
		srcY = (c / 16) * 8;

		/*if (fontStyle == R_FONTSTYLE_SHADOWED)
		{
			glColor3fv(textShadow);

			glTexCoord2f(srcX * texScaleX, srcY * texScaleY);
			glVertex2i(drawX + 1, drawY + 1);
			glTexCoord2f(srcX * texScaleX, (srcY + 8) * texScaleY);
			glVertex2i(drawX + 1, (drawY + 9));
			glTexCoord2f((srcX + 8) * texScaleX, (srcY + 8) * texScaleY);
			glVertex2i((drawX + 9), (drawY + 9));
			glTexCoord2f((srcX + 8) * texScaleX, srcY * texScaleY);
			glVertex2i((drawX + 9), drawY + 1);
		}*/

		glTexCoord2f(srcX * texScaleX, srcY * texScaleY);
		glVertex2i(drawX, drawY);
		glTexCoord2f(srcX * texScaleX, (srcY + 8) * texScaleY);
		glVertex2i(drawX, (drawY + 8));
		glTexCoord2f((srcX + 8) * texScaleX, (srcY + 8) * texScaleY);
		glVertex2i((drawX + 8), (drawY + 8));
		glTexCoord2f((srcX + 8) * texScaleX, srcY * texScaleY);
		glVertex2i((drawX + 8), drawY);

		drawX += 8;
	}

	glEnd();

	R_CheckError();
}