#include "pch.h"
#include "Benchmarker.h"

void AG::Benchmarker::BeginMarker(std::string session)
{
	first_buffer[session].first = glfwGetTime();
}

void AG::Benchmarker::EndMarker(std::string session)
{
	first_buffer[session].second = glfwGetTime();
}

void AG::Benchmarker::UpdateBenchmarker()
{
	delta_time = first_buffer["Update"].first - second_buffer["Update"].first;
	second_buffer = first_buffer;
	draw_calls.second = draw_calls.first;
	draw_calls.first = 0;
}

std::vector<std::string> AG::Benchmarker::GetMarkers()
{
	std::vector<std::string> temp;
	for (auto& sessions : first_buffer)
	{
		temp.push_back(sessions.first);
	}
	return temp;
}

std::pair<double, double> AG::Benchmarker::GetData(std::string session)
{
	return second_buffer[session];
}
