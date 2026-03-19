#pragma once
#include "pch.h"
#pragma warning(push)
#pragma warning(disable: 4068) // unknown pragma
#include <FileWatch.hpp>
#pragma warning(pop)

namespace AG {
    class FileWatcher : public Pattern::ISingleton<FileWatcher> {
    public:
        void watch(const std::string& path, std::function<void(const std::string&, filewatch::Event)> callback);
        ~FileWatcher();
    private:
        std::vector<std::unique_ptr<filewatch::FileWatch<std::string>>> watchers;
    };
}
#define FILEWATCHER AG::FileWatcher::GetInstance()
