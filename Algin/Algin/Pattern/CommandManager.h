/*!*****************************************************************************
\file CommandManager.h
\author Zulfami Ashrafi Bin Wakif
\date 22/4/2025 (MM/DD/YYYY)

\brief A manager to control the commands
*******************************************************************************/
#pragma once
#include "pch.h"

namespace AG {
	namespace Pattern {
		class CommandManager : public Pattern::ISingleton<CommandManager> {
		public:
			void ExecuteCommand(std::unique_ptr<ICommand> cmd);
			void Undo();
			void Redo();

		private:
			std::stack<std::unique_ptr<ICommand>> m_undoStack;
			std::stack<std::unique_ptr<ICommand>> m_redoStack;
		};
	}
}
