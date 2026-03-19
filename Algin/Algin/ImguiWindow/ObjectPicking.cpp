#include "pch.h"
#include "ObjectPicking.h"


void objectPicking::init() {
	glCreateFramebuffers(1, &pickFBO);

	glCreateTextures(GL_TEXTURE_2D, 1, &pickColorTex);
	glTextureStorage2D(pickColorTex, 1, GL_RG32UI, pickFBOwidth, pickFBOheight);

	glTextureParameteri(pickColorTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(pickColorTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(pickColorTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(pickColorTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


	glCreateTextures(GL_TEXTURE_2D, 1, &depthTex);
	glTextureStorage2D(depthTex, 1, GL_DEPTH_COMPONENT24, pickFBOwidth, pickFBOheight);

	glTextureParameteri(depthTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(depthTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depthTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(depthTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glNamedFramebufferTexture(pickFBO, GL_COLOR_ATTACHMENT0, pickColorTex, 0);
	glNamedFramebufferTexture(pickFBO, GL_DEPTH_ATTACHMENT, depthTex, 0);

	glNamedFramebufferDrawBuffer(pickFBO, GL_COLOR_ATTACHMENT0);
}

void objectPicking::renderPickPass(ImVec2 mappedMousePos, ImVec2 imageSize) {

	int px = (int)std::floor(mappedMousePos.x * (pickFBOwidth / imageSize.x));
	int py = (int)std::floor(mappedMousePos.y * (pickFBOheight / imageSize.y));

	glBindFramebuffer(GL_FRAMEBUFFER, pickFBO);
	glViewport(0, 0, pickFBOwidth, pickFBOheight);


	GLuint clearRG[2] = { 0u, 0u };
	glClearBufferuiv(GL_COLOR, 0, clearRG);


	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glDisable(GL_BLEND);
	glDisable(GL_MULTISAMPLE);                
	glDisable(GL_FRAMEBUFFER_SRGB);

	RenderPipeline::GetInstance().SetPipeline(RenderPipeline::OBJPICK);
	COMPONENTMANAGER.LateUpdate();

	GLuint rg[2] = { 0, 0 };
	glBindFramebuffer(GL_READ_FRAMEBUFFER, pickFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(px, py, 1, 1, GL_RG_INTEGER, GL_UNSIGNED_INT, rg);
	uint64_t picked64 = (uint64_t(rg[1]) << 32) | uint64_t(rg[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	AG::Data::GUID objID = id64_to_guid[picked64];

	auto objPtr = OBJECTMANAGER.GetObjectById(objID);
	if (!objPtr) {
		//AG_CORE_WARN("Incorrect Object ID"); <- is annoying af when i want check the prev debug console :(
	}
	else {
		IMGUISYSTEM.SetSelectedObj(objPtr);

		std::stringstream ss;
		ss << objPtr->GetName() << " Object Selected";
		AG_CORE_INFO(ss.str());
	}
}