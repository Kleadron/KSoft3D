#pragma once

#define FAILFLAG_CANCONTINUE 1	// if the user should be able to continue from this point, with a possible unstable game state
#define FAILFLAG_IGNORECODE 2	// if the function is being used to display a message regardless of return or expect code. Removes the associated text in the displayed message.

void CheckFail(const int retcode, const int expect, const char *errtype, const char *errmsg, const int flags);