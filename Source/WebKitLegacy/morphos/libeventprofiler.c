#include "libeventprofiler.h"

#if defined(EP_PROFILING) && EP_PROFILING

#include <hardware/atomic.h>
#include <proto/exec.h>
#include <exec/memory.h>
#include <exec/system.h>
#include <dos/dosextens.h>

#define EP_NAME_MAX 64
#define EP_THREAD_MAX 512

struct ep_event {
        char ep_name[EP_NAME_MAX];        //
        char ep_func[EP_NAME_MAX];        // 64
        UQUAD ep_ts;                      // 128
        UQUAD ep_tsEnd;                   // 136
        ULONG ep_line;                    // 140
        ULONG ep_uid;                     // 144
        ULONG ep_isEvent : 1;             // 148
        ULONG ep_pad : 31;                // 148
};

struct ep_thread {
        char ep_name[EP_NAME_MAX];
        ULONG ep_uid;
        ULONG ep_pad;
};

struct ep_port {
        struct MsgPort ep_port;

        LONG  ep_users;
        LONG  ep_event;
        LONG  ep_maxEvent;
        LONG  ep_thread;
        ULONG ep_pad;

        struct ep_thread ep_threads[EP_THREAD_MAX];

        struct ep_event ep_data[0];
};

#define EP_PORTNAME "epProfiler"

struct __tls_variables
{
	struct ep_port *port;
	UQUAD  ticksNextCheck;      // time of next check
	ULONG  waitTicksCount;  // # of ticks between checks
	ULONG  uid;
	ULONG  startedEvents;
};

#define TLS_THREAD_DESTRUCTOR_EXTRA \
	if (__tls->port) ATOMIC_SUB(&__tls->port->ep_users, 1);
#define TLS_CONSTRUCTOR int ep_construct(void)
#define TLS_DESTRUCTOR void ep_destruct(void)

#include <gentls.h>
#include <constructor.h>

CONSTRUCTOR_P(ep_construct, 100)
{
	return ep_construct();
}

DESTRUCTOR_P(ep_destruct, 100)
{
	ep_destruct();
}

static inline void ep_stccpy(char *to, const char *from, int length)
{
	for (int i = 1; *from != '\0' && i < length; i++)
		*(to++) = *(from++);
	*to = '\0';
}

#if 0
static inline void ep_taskcopy(struct ep_event *e)
{
	struct Task *me = FindTask(0);
	struct Process *process = (struct Process *)me;
	if (me->tc_Node.ln_Type == NT_PROCESS && process->pr_TaskNum > 0 && process->pr_CLI != 0)
		ep_stccpy(e->ep_task, ((char *)BADDR(((struct CommandLineInterface *)(BADDR(process->pr_CLI)))->cli_CommandName))+1, EP_NAME_MAX);
	else
		ep_stccpy(e->ep_task, me->tc_Node.ln_Name, EP_NAME_MAX);
	e->ep_uid = me->tc_ETask->UniqueID;
}
#endif

static void ep_bind(struct __tls_variables *epData)
{
	UQUAD tb = __builtin_ppc_get_timebase();

	if (epData->ticksNextCheck < tb)
	{
		Forbid();
		if (epData->ticksNextCheck < tb)
		{
			struct ep_port *port = (struct ep_port *)FindPort(EP_PORTNAME);
			if (epData->port != port)
			{
				if (0 == epData->waitTicksCount)
				{
					ULONG freq; NewGetSystemAttrsA(&freq, sizeof(freq), SYSTEMINFOTYPE_TBCLOCKFREQUENCY, NULL);
					epData->waitTicksCount = freq;
					epData->waitTicksCount *= 5; // check every 5s
				}

				if (port && 0 != epData->startedEvents)
				{
					// orphan old port but don't attempt to use new one until we have pending events
					port = NULL;
				}

				if (port)
				{
					ATOMIC_ADD(&port->ep_users, 1);
				}

				if (epData->port)
				{
					ATOMIC_SUB(&epData->port->ep_users, 1);
				}

				epData->port = port;

				if (epData->port)
				{
					// TODO: register the thread!
					struct Task *me = FindTask(0);
					epData->uid = me->tc_ETask->UniqueID;

					LONG thread = ATOMIC_ADD(&epData->port->ep_thread, 1);
					if (thread >= 0 && thread < EP_THREAD_MAX)
					{
						struct Process *process = (struct Process *)me;

						if (me->tc_Node.ln_Type == NT_PROCESS && process->pr_TaskNum > 0 && process->pr_CLI != 0)
							ep_stccpy(epData->port->ep_threads[thread].ep_name, ((char *)BADDR(((struct CommandLineInterface *)(BADDR(process->pr_CLI)))->cli_CommandName))+1, EP_NAME_MAX);
						else
							ep_stccpy(epData->port->ep_threads[thread].ep_name, me->tc_Node.ln_Name, EP_NAME_MAX);
						epData->port->ep_threads[thread].ep_uid = epData->uid;
					}
				}
			}

			epData->ticksNextCheck = tb + epData->waitTicksCount;
		}
		Permit();
	}
}

void ep_event(const char *name, const char *func, ULONG line)
{
	struct __tls_variables *epData = __gettls();
	if (epData != &__tls_fallback)
	{
		ep_bind(epData);
		
		if (epData->port)
		{
			LONG event = ATOMIC_ADD(&epData->port->ep_event, 1);
			if (event >= 0 && event < epData->port->ep_maxEvent)
			{
				struct ep_event *e = &epData->port->ep_data[event];
				ep_stccpy(e->ep_name, name, sizeof(e->ep_name));
				ep_stccpy(e->ep_func, func, sizeof(e->ep_func));
				e->ep_line = line;
				e->ep_isEvent = 0;
				e->ep_pad = 0;
				e->ep_ts = e->ep_tsEnd = __builtin_ppc_get_timebase();
				e->ep_uid = epData->uid;
			}
		}
	}
}

LONG ep_start(const char *name, const char *func, ULONG line)
{
	struct __tls_variables *epData = __gettls();
	if (epData != &__tls_fallback)
	{
		ep_bind(epData);
		
		if (epData->port)
		{
			LONG event = ATOMIC_ADD(&epData->port->ep_event, 1);
			if (event >= 0 && event < epData->port->ep_maxEvent)
			{
				struct ep_event *e = &epData->port->ep_data[event];
				ep_stccpy(e->ep_name, name, sizeof(e->ep_name));
				ep_stccpy(e->ep_func, func, sizeof(e->ep_func));
				e->ep_line = line;
				e->ep_isEvent = 0;
				e->ep_pad = 0;
				e->ep_ts = e->ep_tsEnd = __builtin_ppc_get_timebase();
				e->ep_uid = epData->uid;
				epData->startedEvents ++;
				return event;
			}
		}
	}
	
	return -1;
}

void ep_stop(LONG event)
{
	struct __tls_variables *epData = __gettls();
	if (epData != &__tls_fallback)
	{
		if (epData->port)
		{
			struct ep_event *e = &epData->port->ep_data[event];
			if (event >= 0 && event < epData->port->ep_maxEvent)
			{
				e->ep_tsEnd = __builtin_ppc_get_timebase();
			}
		}
		
		epData->startedEvents --;
	}
}
#endif
