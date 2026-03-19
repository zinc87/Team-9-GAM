#include "pch.h"
#include <FileWatch.hpp>

namespace AG {
	void FileWatcher::watch(const std::string& path, std::function<void(const std::string&, filewatch::Event)> callback)
	{
		watchers.emplace_back(
			std::make_unique<filewatch::FileWatch<std::string>>(path, callback)
		);
	}
	FileWatcher::~FileWatcher()
	{
		for (auto& w : watchers) {
			w.reset();
		}
		watchers.clear();
	}
}