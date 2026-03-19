/*!*****************************************************************************
\file ImGuiSystem.h
\author Zulfami Ashrafi Bin Wakif
\date 19/2/2025 (MM/DD/YYYY)

\brief Manager imgui
*******************************************************************************/
#pragma once
#include "../Header/pch.h"


namespace AG {
	namespace System {
		class ImGuiSystem : public Pattern::ISingleton<ImGuiSystem> {
		public:
			using ImguiWindowPtr = std::unique_ptr<AGImGui::IImguiWindow>;
			using WVector = std::vector<ImguiWindowPtr>;

		public:
			// This construct should nvr be called let singleton handle it
			ImGuiSystem();
			~ImGuiSystem();

			void PreRender();
			void Render();
			void PostRender();

			void AddWindow(ImguiWindowPtr component);

			template<typename T>
			T* GetWindow();

			//=== Imgui Style ===//
			void RipeIGuiStyle();
			void UnityLightStyle();
			void UnityDarkStyle();
			void VSStudioStyle();

			std::weak_ptr<System::IObject> GetSelectedObj() { return m_selectedObject; }
            void SetSelectedObj(const std::shared_ptr<System::IObject>& object);

			void Free();
		private:
			void Init();
			void ImguiWindowList();
			void ShowMenuBar();
            void ShowExportSettings();
			bool m_showExportSettings{ false };
			WVector m_windows;
			std::weak_ptr<System::IObject> m_selectedObject;
		};

		template<typename T>
		T* ImGuiSystem::GetWindow() {
			for (size_t i{ 0 }; i < m_windows.size(); i++)
			{
				T* component = dynamic_cast<T*>(m_windows[i]);

				if (component != nullptr) {
					return component;
				}
			}

			return nullptr;
		}



	}
#define IMGUISYSTEM AG::System::ImGuiSystem::GetInstance()
}

#include <windows.h>
#include <shobjidl.h>   // for IFileOpenDialog
#include <string>

static std::string WideToUtf8(const wchar_t* w)
{
    if (!w) return {};
    // Query required size (includes the null terminator).
    const int size = ::WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) return {}; // empty or error

    std::string out(static_cast<size_t>(size - 1), '\0'); // exclude null
    ::WideCharToMultiByte(CP_UTF8, 0, w, -1, out.data(), size, nullptr, nullptr);
    return out;
}

inline std::string OpenFileDialog()
{
    // Init COM (S_FALSE is OK; it means “already initialized on this thread”)
    const HRESULT init = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool needUninit = SUCCEEDED(init); // we’ll balance with CoUninitialize only if this call succeeded

    IFileOpenDialog* dlg = nullptr;
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
        IID_PPV_ARGS(&dlg));
    if (FAILED(hr) || !dlg) {
        if (needUninit) ::CoUninitialize();
        return {};
    }

    DWORD flags = 0;
    if (SUCCEEDED(dlg->GetOptions(&flags))) {
        dlg->SetOptions(flags | FOS_FORCEFILESYSTEM);
    }

    // Basic “All files” filter
    static const COMDLG_FILTERSPEC kFilter[] = {
        { L"All Files (*.*)", L"*.*" }
    };
    dlg->SetFileTypes(1, kFilter);

    hr = dlg->Show(nullptr);
    if (FAILED(hr)) {
        // If the user cancels, hr is often HRESULT_FROM_WIN32(ERROR_CANCELLED) — treat as benign.
        dlg->Release();
        if (needUninit) ::CoUninitialize();
        return {};
    }

    IShellItem* item = nullptr;
    hr = dlg->GetResult(&item);
    if (FAILED(hr) || !item) {
        if (dlg) dlg->Release();
        if (needUninit) ::CoUninitialize();
        return {};
    }

    PWSTR pathW = nullptr;
    std::string result;
    hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pathW);
    if (SUCCEEDED(hr) && pathW) {
        result = WideToUtf8(pathW);      // ← proper UTF-8 conversion
        ::CoTaskMemFree(pathW);
    }

    item->Release();
    dlg->Release();
    if (needUninit) ::CoUninitialize();
    return result;
}


inline std::string OpenFolderDialog()
{
    std::string folderPath;

    IFileDialog* pFileDialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
        IID_PPV_ARGS(&pFileDialog));

    if (SUCCEEDED(hr))
    {
        DWORD options;
        pFileDialog->GetOptions(&options);
        pFileDialog->SetOptions(options | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);

        if (SUCCEEDED(pFileDialog->Show(nullptr)))
        {
            IShellItem* pItem;
            if (SUCCEEDED(pFileDialog->GetResult(&pItem)))
            {
                PWSTR pszFilePath = nullptr;
                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                {
                    char buffer[MAX_PATH];
                    WideCharToMultiByte(CP_ACP, 0, pszFilePath, -1, buffer, MAX_PATH, NULL, NULL);
                    folderPath = buffer;
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileDialog->Release();
    }
    return folderPath;
}


