#pragma once
#include "pch.h"

namespace AG
{
	class Benchmarker : public Pattern::ISingleton<Benchmarker>
	{
	public:
		/* --- Time Marker --- */
		void BeginMarker(std::string session);
		void EndMarker(std::string session);
		std::map<std::string, std::pair<double, double>>& GetProfile() { return second_buffer; }
		std::vector<std::string> GetMarkers();
		std::pair<double, double> GetData(std::string session);

		/* --- Draw Call Stats --- */
		void IncDrawCall() { draw_calls.first++ ; }
		unsigned int GetDrawCalls() { return draw_calls.second; }

		void UpdateBenchmarker();
		double GetDeltaTime() { return delta_time; }

		void FrustumCheckInc() { frustumcheck++; }
		void FrustumCulledInc() { frustumculled++; }
		unsigned int GetFrustumCheck() { return frustumcheck; }
		unsigned int GetFrustumCulled() { return frustumculled; }

		void ResetFrustumChecker() { frustumcheck = frustumculled = 0; }

	private:
		unsigned int frustumcheck = 0;
		unsigned int frustumculled = 0;
		std::map<std::string, std::pair<double, double>> first_buffer;
		std::map<std::string, std::pair<double, double>> second_buffer;
		double delta_time=0.0;
		std::pair<unsigned int, unsigned int> draw_calls;
	};
}

#define Benchmarker_StartMarker(x) Benchmarker::GetInstance().BeginMarker(x)
#define Benchmarker_EndMarker(x) Benchmarker::GetInstance().EndMarker(x)
#define Benchmarker_Update Benchmarker::GetInstance().UpdateBenchmarker()
#define BENCHMARKER Benchmarker::GetInstance()