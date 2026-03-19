#pragma once

#ifdef AG_PLATFORM_WINDOW
	#ifdef  AG_BUILD_DLL
		#define AG_API __declspec(dllexport)
	#else
		#define AG_API __declspec(dllimport)
	#endif //  AG_BUILD_DLL
#else
	#error AGEngine Only Supports WINDOWS!
#endif // AG_PLATFORM_WINDOW

#define UNUSED_PARAMETER(x) (void)x
