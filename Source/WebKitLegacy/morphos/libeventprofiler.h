#pragma once
#include <exec/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void ep_event(const char *name, const char *func, ULONG line);
LONG ep_start(const char *name, const char *func, ULONG line);
void ep_stop(LONG identifier);

#ifdef __cplusplus
} //end extern "C"
#endif
	
#if defined(EP_PROFILING) && EP_PROFILING

#define EP_QUOTE(str) #str
#define EP_EXPAND_AND_QUOTE(str) EP_QUOTE(str)

#define EP_EVENT(__name__) ep_event(EP_EXPAND_AND_QUOTE(__name__), __func__, __LINE__);
#define EP_EVENTSTR(__name__) ep_event(__name__, __func__, __LINE__);
#define EP_BEGIN(__name__) ULONG ep##__name__ = ep_start(EP_EXPAND_AND_QUOTE(__name__), __func__, __LINE__);
#define EP_END(__name__) ep_stop(ep##__name__);

#ifdef __cplusplus
class epScope
{
public:
	epScope(const char *name, const char *func, ULONG line) : _epEvent(ep_start(name, func, line)) { }
	~epScope() { ep_stop(_epEvent); }
private:
	ULONG _epEvent;
};

#define EP_SCOPE(__name__) epScope __epsc##__name__ = epScope(EP_EXPAND_AND_QUOTE(__name__), __func__, __LINE__);
#define EP_SCOPESTR(__name__) epScope __epsc __LINE__ = epScope(__name__, __func__, __LINE__);
#endif

#else

#define EP_EVENT(__name__)
#define EP_EVENTSTR(__name__)
#define EP_BEGIN(__name__)
#define EP_END(__name__)

#ifdef __cplusplus
#define EP_SCOPE(__name__)
#define EP_SCOPESTR(__name__)
#endif

#endif
