#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FAILFLAG_CANCONTINUE 1	// if the user should be able to continue from this point, with a possible unstable game state
#define FAILFLAG_IGNORECODE 2	// if the function is being used to display a message regardless of return or expect code. Removes the associated text in the displayed message.

#define FAILMSG_TITLE_QUITANDCONTINUE "The KSoft application has paused execution to prevent further problems.\nContinuing may cause instabilities.\n"
#define FAILMSG_TITLE_QUITONLY "The KSoft application has paused execution to prevent further problems.\n"

#define FAILMSG_DETAIL "Failure Type: %s\nMessage: %s\n"

#define FAILMSG_CODE "Code: %i\nExpected Code: %i\n"

char failmsgbuffer[4096];
char *msgbufptr;
void CheckFail(const int retcode, const int expect, const char *errtype, const char *errmsg, const int flags)
{
	memset(&failmsgbuffer, 0, sizeof(failmsgbuffer));
	msgbufptr = (char*)&failmsgbuffer;

	if (flags & FAILFLAG_CANCONTINUE)
	{
		msgbufptr += sprintf(msgbufptr, FAILMSG_TITLE_QUITANDCONTINUE);
	}
	else
	{
		msgbufptr += sprintf(msgbufptr, FAILMSG_TITLE_QUITONLY);
	}

	msgbufptr += sprintf(msgbufptr, FAILMSG_DETAIL, errtype, errmsg);

	if (!(flags & FAILFLAG_IGNORECODE))
	{
		msgbufptr += sprintf(msgbufptr, FAILMSG_CODE, retcode, expect);
	}


	if (retcode != expect || flags & FAILFLAG_IGNORECODE)
	{
		SDL_MessageBoxButtonData buttons[3];

		// order is reversed because the flag doesn't work and kills the icon

		if (flags & FAILFLAG_CANCONTINUE)
		{
			buttons[2].buttonid = 0;
			buttons[2].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
			buttons[2].text = "Continue";

			buttons[1].buttonid = 1;
			buttons[1].flags = 0;
			buttons[1].text = "Debug";

			buttons[0].buttonid = 2;
			buttons[0].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
			buttons[0].text = "Quit";
		}
		else
		{
			buttons[1].buttonid = 1;
			buttons[1].flags = 0;
			buttons[1].text = "Debug";

			buttons[0].buttonid = 2;
			buttons[0].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
			buttons[0].text = "Quit";
		}

		SDL_MessageBoxData data;
		int clickButton = -1;

		data.flags = SDL_MESSAGEBOX_ERROR;
		data.title = "Epic Fail";
		data.numbuttons = 2;
		if (flags & FAILFLAG_CANCONTINUE)
			data.numbuttons = 3;
		data.buttons = (SDL_MessageBoxButtonData*)&buttons;
		data.window = NULL;
		data.colorScheme = NULL;
		data.message = (char*)&failmsgbuffer;

		int showncode = SDL_ShowMessageBox(&data, &clickButton);

		if (showncode != 0)
		{
			printf("\n...epically failed and could not show message box.\n%s\n%s\n", errtype, errmsg);
			SDL_Quit();
			exit(EXIT_FAILURE);
		}

		if (clickButton == 2)
		{
			// quit
			SDL_Quit();
			exit(EXIT_FAILURE);
		}

		if (clickButton == 1)
		{
			// debug
			SDL_TriggerBreakpoint();
		}

		if (clickButton == 0)
		{
			// continue
		}

		return;
	}
}