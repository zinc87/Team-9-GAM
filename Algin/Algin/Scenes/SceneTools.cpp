#include "pch.h"
#include "SceneTools.h"
#include <filesystem>
namespace fs = std::filesystem;

constexpr ImVec2 windowSize = ImVec2(500, 200);

void SceneTools::RenderUI()
{
    if (!m_isOpen)
        return;

    std::string title = "Scene Manager";
    switch (st_State)
    {
    case NEW:   title += " - New Scene"; break;
    case OPEN:  title += " - Open Scene"; break;
    case SAVE:  title += " - Save Scene"; break;
    case SAVEAS:title += " - Save Scene As"; break;
    }

    // Get main viewport (the whole application window)
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 center = viewport->GetCenter();

    // Calculate top-left corner for centered placement
    ImVec2 pos = ImVec2(center.x - windowSize.x * 0.5f,
        center.y - windowSize.y * 0.5f);

    // Apply position & size before Begin
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin(title.c_str(), &m_isOpen,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoMove
    );

    // Scale UI to fit window
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float labelWidth = avail.x * 0.3f;
    float inputWidth = avail.x * 0.6f;

    switch (st_State)
    {
    case NEW:
    {
        ImGui::SetWindowSize(windowSize, ImGuiCond_Always);

        if (ImGui::Button("Create New Scene", ImVec2(avail.x, 0)))
        {
            auto& objMgr = OBJECTMANAGER;
            auto& compMgr = COMPONENTMANAGER;

            // Clear runtime scene (objects & components)
            objMgr.ClearScene();
            compMgr.ClearAllComponents();

            // Clear file location since we haven't saved anything yet
            m_fileLoc.clear();
            m_sceneName.clear();

            AG_CORE_INFO("New scene created (not saved to disk yet)");

            // Close the dialog
            m_isOpen = false;
        }
    }
    break;

    case SAVE:
    {
        ImGui::SetWindowSize(windowSize, ImGuiCond_Always);

        auto& objMgr = OBJECTMANAGER;
        auto& compMgr = COMPONENTMANAGER;

        if (m_fileLoc.empty())
        {
            // No file yet: switch to Save As
            st_State = SAVEAS;
        }
        else
        {
            //Save directly to the existing file
            fs::path fullPath{ m_fileLoc };
            fullPath.replace_extension(".json");
            m_fileLoc = fullPath.string();
            if (!AG::System::SerializationSystem::GetInstance().SaveScene(m_fileLoc, objMgr, compMgr))
            {
                AG_CORE_ERROR("Failed to save scene to: {0}", m_fileLoc);
            }
            else
            {
                AG_CORE_INFO("Scene saved to: {0}", m_fileLoc);
                m_isOpen = false;
            }

            //Temp Block
            std::filesystem::path jsonFullPath{ m_fileLoc };
            jsonFullPath.replace_extension(".scene");
            if (!AG::System::SerializationSystem::GetInstance().SaveSceneJson(jsonFullPath))
            {
                AG_CORE_ERROR("Failed to save scene to: {0}", m_fileLoc);
            }
            else
            {
                AG_CORE_INFO("Scene saved to: {0}", m_fileLoc);
                m_isOpen = false;
            }
        }
    }
    break;

    case SAVEAS:
    {
        ImGui::SetWindowSize(windowSize, ImGuiCond_Always);

        // Show selected directory
        ImGui::Text("Save Location:");
        if (m_selectedFolder.empty())
        {
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "(No folder selected)");
        }
        else
        {
            ImGui::TextWrapped("%s", m_selectedFolder.c_str());
        }

        // Button to open folder selector
        if (ImGui::Button("Select Folder", ImVec2(avail.x, 0)))
        {
            m_selectedFolder = OpenFolderDialog();
        }

        ImGui::Spacing();

        // Scene name input
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, labelWidth);
        ImGui::Text("Scene Name:");
        ImGui::NextColumn();
        ImGui::PushItemWidth(inputWidth);
        ImGui::InputText("##SceneName", &m_sceneName);
        ImGui::PopItemWidth();
        ImGui::Columns(1);

        ImGui::Spacing();

        // Save button
        if (ImGui::Button("Save", ImVec2(avail.x, 0)))
        {
            if (m_selectedFolder.empty())
            {
                AG_CORE_ERROR("Please select a folder first");
            }
            else if (m_sceneName.empty())
            {
                AG_CORE_ERROR("Please enter a scene name");
            }
            else
            {
                auto& objMgr = OBJECTMANAGER;
                auto& compMgr = COMPONENTMANAGER;

                // Build the full path using the selected folder
                fs::path folderPath(m_selectedFolder);
                fs::path fileName = m_sceneName;

                // Add .bin extension if not present
                if (fileName.extension().empty())
                {
                    fileName.replace_extension(".json");
                }

                fs::path fullPath = folderPath / fileName;

                AG_CORE_INFO("Saving scene to: {0}", fullPath.string());

                // If we're changing the file name/location, remove the old file
                if (!m_fileLoc.empty() && fs::path(m_fileLoc) != fullPath)
                {
                    try {
                        if (fs::exists(m_fileLoc))
                        {
                            fs::remove(m_fileLoc);
                            AG_CORE_INFO("Removed old scene file: {0}", m_fileLoc);
                        }
                    }
                    catch (const std::exception& e) {
                        AG_CORE_WARN("Failed to remove old scene file {0}: {1}", m_fileLoc, e.what());
                    }
                }

                // Update current file location
                fullPath.replace_extension(".json");
                m_fileLoc = fullPath.string();

                // Save the scene
                if (!AG::System::SerializationSystem::GetInstance().SaveScene(m_fileLoc, objMgr, compMgr))
                {
                    AG_CORE_ERROR("Failed to save scene to: {0}", m_fileLoc);
                }
                else
                {
                    AG_CORE_INFO("Scene saved successfully to: {0}", m_fileLoc);
                    m_isOpen = false;
                    st_State = SAVE; // next time, Save will overwrite this file
                }

                //Temp Block
                std::filesystem::path jsonFullPath{ m_fileLoc };
                jsonFullPath.replace_extension(".scene");
                if (!AG::System::SerializationSystem::GetInstance().SaveSceneJson(jsonFullPath))
                {
                    AG_CORE_ERROR("Failed to save scene to: {0}", m_fileLoc);
                }
                else
                {
                    AG_CORE_INFO("Scene saved to: {0}", m_fileLoc);
                    m_isOpen = false;
                    st_State = SAVE; // next time, Save will overwrite this file
                }
            }
        }
    }
    break;

    case OPEN:
    {
        // File Location row with "O" button
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, labelWidth);
        ImGui::Text("File Location:");
        ImGui::NextColumn();
        ImGui::PushItemWidth(inputWidth - 30.0f);
        ImGui::InputText("##FileLocation", &m_fileLoc);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("O"))
        {
            m_fileLoc = OpenFileDialog();
        }
        ImGui::Columns(1);

        if (ImGui::Button("Open", ImVec2(avail.x, 0)))
        {
            auto& objMgr = OBJECTMANAGER;
            auto& compMgr = COMPONENTMANAGER;

            if (m_fileLoc.empty()) {
                AG_CORE_ERROR("No file selected to open");
            }
            else {
                // Check if file exists first
                if (!fs::exists(m_fileLoc))
                {
                    AG_CORE_ERROR("File does not exist: {0}", m_fileLoc);
                    m_isOpen = false;
                }
                else if (fs::path(m_fileLoc).extension().string() == ".scene") { //Open .scene file. the newer json file
                    // Clear runtime scene (objects & components)
                    objMgr.ClearScene();
                    compMgr.ClearAllComponents();

                    AG::System::SerializationSystem::GetInstance().loadSceneJson(fs::path(m_fileLoc));
                    
                    AG_CORE_INFO("Opened scene: {0}", m_fileLoc);
                    // Extract scene name from file path
                    fs::path tempPath = fs::path(m_fileLoc).replace_extension(".json");
                    m_sceneName = fs::path(m_fileLoc).stem().string();
                    m_isOpen = false;
                }
                else if (!AG::System::SerializationSystem::GetInstance().LoadScene(m_fileLoc, objMgr, compMgr))
                {
                    AG_CORE_ERROR("Failed to open scene: {0}", m_fileLoc);
                    m_fileLoc.clear();
                    m_sceneName.clear();
                    m_isOpen = false;
                }
                else
                {
                    AG_CORE_INFO("Opened scene: {0}", m_fileLoc);
                    // Extract scene name from file path
                    m_sceneName = fs::path(m_fileLoc).stem().string();
                    m_isOpen = false;
                }
            }
        }
    }
    break;

    default:
        break;
    }

    ImGui::End();

    if (m_fileExplorerOpen)
    {
        FileExplorer(fileExplorerMode);
    }
}

static std::string selectedPath;

bool SceneTools::FileExplorer(int mode)
{
    static fs::path rootPath = fs::current_path().parent_path().parent_path().parent_path(); // Go up from bin/x64/Debug
    static fs::path currentPath = rootPath;

    bool finished = false;

    ImGui::Begin("File Explorer", &m_fileExplorerOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
    ImGui::SetWindowSize(ImVec2(1000, 500), ImGuiCond_Once);

    ImGui::Text("Current Path: %s", currentPath.string().c_str());
    ImGui::Separator();

    ImGui::Columns(2, "##fileexplorer", true);

    // --- Left: Directory tree ---
    ImGui::BeginChild("##dirTree", ImVec2(0, -50), true);

    std::function<void(const fs::path&)> DrawDirTree = [&](const fs::path& dir)
        {
            try {
                for (auto& entry : fs::directory_iterator(dir))
                {
                    if (entry.is_directory())
                    {
                        const auto& subdir = entry.path();
                        std::string name = subdir.filename().string();

                        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
                        bool open = ImGui::TreeNodeEx(name.c_str(), nodeFlags);

                        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                            currentPath = subdir;

                        if (open)
                        {
                            DrawDirTree(subdir);
                            ImGui::TreePop();
                        }
                    }
                }
            }
            catch (...) {}
        };

    if (ImGui::TreeNodeEx(rootPath.filename().string().c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
    {
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            currentPath = rootPath;

        DrawDirTree(rootPath);
        ImGui::TreePop();
    }

    ImGui::EndChild();
    ImGui::NextColumn();

    // --- Right: File/Folder list ---
    ImGui::BeginChild("##filelist", ImVec2(0, -50), true);

    try {
        for (auto& entry : fs::directory_iterator(currentPath))
        {
            const auto& path = entry.path();
            std::string name = path.filename().string();

            if (entry.is_directory())
            {
                if (ImGui::Selectable(("[DIR] " + name).c_str(), false))
                {
                    currentPath = path;
                }
            }
            else if (mode == 1) // file mode - only show files
            {
                // Only show .bin and .scene files
                auto ext = path.extension().string();
                if (ext == ".json" || ext == ".scene")
                {
                    if (ImGui::Selectable(name.c_str(), m_fileLoc == path.string()))
                    {
                        m_fileLoc = path.string();
                    }
                }
            }
        }
    }
    catch (...)
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error reading directory!");
    }

    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::Separator();

    if (mode == 0) // folder mode
    {
        ImGui::Text("Selected Folder: %s", currentPath.string().c_str());
    }
    else if (mode == 1) // file mode
    {
        ImGui::Text("Selected File: %s", m_fileLoc.c_str());
    }

    // --- Bottom buttons ---
    float footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::Dummy(ImVec2(0, footerHeight));

    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 180);

    if (mode == 0) // folder mode
    {
        if (ImGui::Button("Select", ImVec2(80, 0)))
        {
            m_selectedFolder = currentPath.string();
            AG_CORE_INFO("Selected folder: {0}", m_selectedFolder);
            finished = true;
            m_fileExplorerOpen = false;
        }
    }
    else // file mode
    {
        if (ImGui::Button("Open", ImVec2(80, 0)))
        {
            if (!m_fileLoc.empty())
            {
                finished = true;
                m_fileExplorerOpen = false;
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(80, 0)))
    {
        m_fileLoc.clear();
        m_fileExplorerOpen = false;
        finished = true;
    }

    ImGui::End();
    return finished;
}

