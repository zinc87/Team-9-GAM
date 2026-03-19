/*!*****************************************************************************
\file ISingleton.h
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Singleton interface to use the pattern
*******************************************************************************/
#pragma once

namespace AG {
	namespace Pattern {
		
		template <typename T>
		class ISingleton {
		public:
			// Ensure that there is only one instance
			ISingleton(const ISingleton&) = delete;
			ISingleton& operator=(const ISingleton&) = delete;
			ISingleton(ISingleton&&) = delete;
			ISingleton& operator=(ISingleton&&) = delete;

			static T& GetInstance() {
				static T instance;
				return instance;
			}

		protected:
			ISingleton() { }
			virtual ~ISingleton() { }
		};
	}
}