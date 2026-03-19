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
		void CommandManager::ExecuteCommand(std::unique_ptr<ICommand> cmd) {
			cmd->Execute();
			m_undoStack.push(std::move(cmd));
			// Clear redo stack on new command
			while (!m_redoStack.empty()) m_redoStack.pop();
		}

		void CommandManager::Undo() {
			if (!m_undoStack.empty()) {
				auto& cmd = m_undoStack.top();
				cmd->Undo();
				m_redoStack.push(std::move(cmd));
				m_undoStack.pop();
			}
		}

		void CommandManager::Redo() {
			if (!m_redoStack.empty()) {
				auto& cmd = m_redoStack.top();
				cmd->Execute();
				m_undoStack.push(std::move(cmd));
				m_redoStack.pop();
			}
		}
	}
}