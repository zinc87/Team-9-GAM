#pragma once
#include "../Header/pch.h"
#include "../AGEngine/Application.h"


class objectPicking : public AG::Pattern::ISingleton<objectPicking> {

public:
	
	void init();
	void renderPickPass(ImVec2 mappedMousePos, ImVec2 imageSize);

	std::unordered_map<uint64_t, std::string> id64_to_guid;

private:
	int pickFBOwidth = 1920;
	int pickFBOheight = 1080;

	GLuint pickFBO = 0;
	GLuint pickColorTex = 0;
	GLuint depthTex = 0;
};