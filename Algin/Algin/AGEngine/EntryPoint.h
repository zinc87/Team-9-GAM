#pragma once
#include <crtdbg.h>				// To check for memory leaks
#include "../Header/pch.h"

#include "../System/Event.h"

#ifdef  AG_PLATFORM_WINDOW
	#define UNREFERENCE_PARAM(arg_) (void)arg_

	extern AG::Application* AG::CreateApplication();

	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(416960);
#endif

		UNREFERENCE_PARAM(hInstance);
		UNREFERENCE_PARAM(hPrevInstance);
		UNREFERENCE_PARAM(pCmdLine);
		UNREFERENCE_PARAM(nCmdShow);

		AG::Application* app = AG::CreateApplication();
		app->Init();
		app->Update();
		app->Free();
		delete app;

		return 0;
	}
#endif //  AG_PLATFORM_WINDOW