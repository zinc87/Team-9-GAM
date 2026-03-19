/*!*****************************************************************************
\file ICommand.h
\author Zulfami Ashrafi Bin Wakif
\date 22/4/2025 (MM/DD/YYYY)

\brief Command interface to use the pattern
*******************************************************************************/
#pragma once

namespace AG {
	namespace Pattern {

		class ICommand {
		public:
			virtual ~ICommand() = default;
			virtual void Execute() = 0;
			virtual void Undo() = 0;
		};
	}
}