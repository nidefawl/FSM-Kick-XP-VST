#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined _WIN32 && defined DEBUG
#define DEBUG_CONSOLE 
#define DISPATCHER_DEBUG_TRACE
#define dprintf(...) printf (__VA_ARGS__)
#else
#define dprintf(...) {}
#endif

#endif // __DEBUG_H__
