#include <exec/types.h>
#include <stdlib.h>
#include <sys/syslimits.h>
#include <proto/dos.h>
#include <dos/dos.h>

extern "C" void Fail(unsigned char *)
{
	// Libjpeg fail message...
	// TODO
}

extern "C" char *realpath(const char *file_name, char *resolved_name)
{
	BPTR l = Lock(file_name, ACCESS_READ);
	if (l)
	{
		if (NameFromLock(l, resolved_name, PATH_MAX))
		{
			UnLock(l);
			return resolved_name;
		}
		UnLock(l);
	}

	return nullptr;
}
