#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdbool.h>

#ifdef __APPLE__
#define DPRINT(...) do { fprintf( stderr, __VA_ARGS__ ); } while (false)
#define DMSG(...) do { fprintf( stderr, "%s:%d: ", __FILE__ , __LINE__ ); DPRINT(__VA_ARGS__); } while (false)
#else
#define DPRINT(...) do { ; } while (false);
#define DMSG(...) do { ; } while (false)
#endif

#endif
