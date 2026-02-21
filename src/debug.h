#pragma once

#if defined(_MSC_VER)
	#define DEBUG_BREAK __debugbreak()
#elif defined(__GNUC__)
	#define DEBUG_BREAK asm("int3")
#else
	#error "Platform not supported for programmatic breakpoints"
#endif

