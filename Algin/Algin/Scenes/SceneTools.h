#pragma once
#include "pch.h"

class SceneTools : public AG::Pattern::ISingleton<SceneTools>
{
public:
	enum ST_STATE
	{
		NEW,
		OPEN,
		SAVE,
		SAVEAS,
	};

	void Enable(ST_STATE state)
	{
		m_isOpen = true;
		st_State = state;
	}
	void RenderUI();

private:
	bool m_isOpen = false;
	bool m_fileExplorerOpen = false;
	int fileExplorerMode = 0; // 0 = folder, 1 = file

	std::string m_sceneName = "";
	std::string m_fileLoc = "";
	std::string m_selectedFolder = "";
	ST_STATE st_State = NEW;
	bool FileExplorer(int mode); // 0 = folder, 1 = file
};

