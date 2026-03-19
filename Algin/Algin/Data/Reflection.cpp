#include "pch.h"
#include "Reflection.h"

namespace AG {

    void RenderFieldsInspector(void* componentThis, const std::vector<FieldInfo>& fields) {

        if (!componentThis) return;

        // group fields by category (empty => default/unnamed)
        std::map<std::string, std::vector<FieldInfo>> grouped;
        for (const auto& f : fields) {
            grouped[f.category].push_back(f); // category may be ""
        }

        // prepare render order: "" (default) first, then sorted other categories
        std::vector<std::string> categories;
        bool hasDefault = (grouped.find("") != grouped.end());
        if (hasDefault) categories.push_back("");
        for (const auto& kv : grouped) {
            if (!kv.first.empty()) categories.push_back(kv.first);
        }
        std::sort(categories.begin() + (hasDefault ? 1 : 0), categories.end());

        float start_x = ImGui::GetCursorPosX();
        float offsets = ImGui::GetContentRegionAvail().x * 0.35f;
        float new_start_x = start_x + offsets;

        // Render default category first (if any)
        if (hasDefault) {
            const auto& fieldsInCat = grouped[""];
            for (const auto& f : fieldsInCat) {
                if (f.hiddenInInspector)
                    continue;

                ImGui::Text("%s", f.name.c_str());
                ImGui::SameLine();
                ImGui::SetCursorPosX(new_start_x);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

                void* ptr = f.getPtr(componentThis);
                if (!ptr) { ImGui::TextDisabled("null"); continue; }

                // --- Combo fields ---
                if (f.isCombo && f.getComboItems) {
                    auto items = f.getComboItems();
                    if (f.type == typeid(std::string)) {
                        std::string* str = reinterpret_cast<std::string*>(ptr);
                        const char* preview = str->empty() ? "<None>" : str->c_str();
                        static ImGuiTextFilter filter;
                        if (ImGui::BeginCombo((std::string("##") + f.name).c_str(), preview)) {

                            filter.Draw("##ComboFilter", -1.0f);

                            ImGui::Separator();

                            for (int i = 0; i < (int)items.size(); i++) {

                                if (filter.PassFilter(items[i]))
                                {
                                    bool isSelected = (*str == items[i]);
                                    if (ImGui::Selectable(items[i], isSelected)) {
                                        *str = items[i];
                                    }
                                    if (isSelected) ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }
                    }
                    else {
                        int* idx = reinterpret_cast<int*>(ptr);
                        if (idx) ImGui::Combo((std::string("##") + f.name).c_str(), idx, items.data(), (int)items.size());
                    }
                    continue;
                }

                // === INT ===
                if (f.type == std::type_index(typeid(int))) {
                    int* v = reinterpret_cast<int*>(ptr);
                    if (v) ImGui::DragInt((std::string("##") + f.name).c_str(), v, 1.0f);
                    continue;
                }

                // === size_t ===
                if (f.type == std::type_index(typeid(size_t))) {
                    size_t* sval = reinterpret_cast<size_t*>(ptr);
                    if (sval) {
                        unsigned long long val = static_cast<unsigned long long>(*sval);

                        if (f.hasRange) {
                            unsigned long long minv = static_cast<unsigned long long>(std::max<float>(0.0f, f.min));
                            unsigned long long maxv = static_cast<unsigned long long>(std::max<float>(0.0f, f.max));
                            ImGui::SliderScalar((std::string("##") + f.name).c_str(),
                                ImGuiDataType_U64, &val, &minv, &maxv);
                        }
                        else {
                            if (!ImGui::InputScalar((std::string("##") + f.name).c_str(),
                                ImGuiDataType_U64, &val))
                            {
                                if (ImGui::IsItemHovered())
                                {
                                    auto asset = ASSET_MANAGER.GetAsset(static_cast<size_t>(val));
                                    if (!asset.expired()) {
                                        ImGui::BeginTooltip();
                                        ImGui::Text(asset.lock()->asset_name.c_str());
                                        ImGui::EndTooltip();
                                    }
                                }

                                if (ImGui::BeginDragDropTarget())
                                {
                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMGUI_CONTENT"))
                                    {
                                        val = HASH(std::string((const char*)payload->Data, payload->DataSize));
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                            }
                            
                        }

                        *sval = static_cast<size_t>(val);
                    }
                    continue;
                }

                // === FLOAT ===
                if (f.type == std::type_index(typeid(float))) {
                    float* v = reinterpret_cast<float*>(ptr);
                    if (v) {
                        if (f.hasRange)
                            ImGui::SliderFloat((std::string("##") + f.name).c_str(),
                                v, f.min, f.max, "%.3f");
                        else
                            ImGui::DragFloat((std::string("##") + f.name).c_str(),
                                v, 0.1f);
                    }
                    continue;
                }

                // === glm::vec3 ===
                if (f.type == std::type_index(typeid(glm::vec3))) {
                    glm::vec3* vec = reinterpret_cast<glm::vec3*>(ptr);
                    if (vec)
                        ImGui::DragFloat3((std::string("##") + f.name).c_str(),
                            glm::value_ptr(*vec), 0.1f);
                    continue;
                }

                // === glm::vec2 ===
                if (f.type == std::type_index(typeid(glm::vec2))) {
                    glm::vec2* vec = reinterpret_cast<glm::vec2*>(ptr);
                    if (vec)
                        ImGui::DragFloat2((std::string("##") + f.name).c_str(),
                            glm::value_ptr(*vec), 0.1f);
                    continue;
                }

                // === BOOL ===
                if (f.type == std::type_index(typeid(bool))) {
                    bool* b = reinterpret_cast<bool*>(ptr);
                    if (b) ImGui::Checkbox((std::string("##") + f.name).c_str(), b);
                    continue;
                }

                // === OBJECT ===
                if (f.type == std::type_index(typeid(System::IObject))) {
                    System::IObject* obj = reinterpret_cast<System::IObject*>(ptr);
                    std::string displayName = obj ? obj->GetName() : "<None>";

                    ImGui::Button(displayName.c_str(), ImVec2(150, 0));

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload =
                            ImGui::AcceptDragDropPayload("DND_OBJECT"))
                        {
                            const char* draggedID =
                                static_cast<const char*>(payload->Data);

                            auto draggedObj =
                                System::ObjectManager::GetInstance()
                                .GetObjectByID(
                                    static_cast<System::IObject::ID>(
                                        std::string{ draggedID }));

                            *obj = *draggedObj.get();
                        }
                        ImGui::EndDragDropTarget();
                    }
                    continue;
                }

                // === DOUBLE ===
                if (f.type == std::type_index(typeid(double))) {
                    double* v = reinterpret_cast<double*>(ptr);
                    if (v)
                        ImGui::DragScalar((std::string("##") + f.name).c_str(),
                            ImGuiDataType_Double, v, 0.1f);
                    continue;
                }

                // === UNSIGNED INT ===
                if (f.type == std::type_index(typeid(unsigned int))) {
                    unsigned int* v = reinterpret_cast<unsigned int*>(ptr);
                    if (v) {
                        unsigned int temp = *v;
                        ImGui::DragScalar((std::string("##") + f.name).c_str(),
                            ImGuiDataType_U32, &temp, 1.0f);
                        *v = temp;
                    }
                    continue;
                }

                // === GUID STRING FIELD (always stored as std::string in C++) ===
                if (f.name.find("GUID") != std::string::npos)
                {
                    std::string* guid = (std::string*)ptr;

                    // Display non-editable text
                    ImGui::TextDisabled(guid->c_str());

                    continue;
                }

                // === STRING (std::string) ===
                if (f.type == std::type_index(typeid(std::string)) &&
                    f.name.find("GUID") == std::string::npos) {
                    std::string* s = reinterpret_cast<std::string*>(ptr);

                    size_t buf_size = std::max<size_t>(256, s->size() + 1);
                    std::vector<char> buffer(buf_size);

                    memcpy(buffer.data(), s->c_str(), s->size());
                    buffer[s->size()] = '\0';

                    if (ImGui::InputText(
                        (std::string("##") + f.name).c_str(),
                        buffer.data(), buffer.size()))
                    {
                        *s = std::string(buffer.data());
                    }
                    continue;
                }

                if (f.type == std::type_index(typeid(std::string))) {
                    std::string* s = reinterpret_cast<std::string*>(ptr);
                    if (s) {
                        size_t buf_size = std::max<size_t>(256, s->size() + 1);
                        std::vector<char> buf(buf_size);

                        if (!s->empty()) {
                            std::memcpy(buf.data(), s->c_str(), s->size());
                        }
                        buf[s->size()] = '\0';

                        if (ImGui::InputText((std::string("##") + f.name).c_str(), buf.data(), buf.size())) {
                            *s = std::string(buf.data());
                        }
                    }
                    continue;
                }

                ImGui::TextDisabled("Unsupported Type");
            }
        }

        // If there are non-default categories, add exactly one separator
        int startIdx = hasDefault ? 1 : 0;
        for (size_t ci = startIdx; ci < categories.size(); ++ci) {

            const std::string& category = categories[ci];
            const auto& fieldsInCat = grouped[category];

            if (hasDefault && ci == 1)
                ImGui::Separator();
            else if (!hasDefault)
                ImGui::Separator();

            ImGui::Text("%s", category.c_str());

            for (const auto& f : fieldsInCat) {

                ImGui::Text("%s", f.name.c_str());
                ImGui::SameLine();
                ImGui::SetCursorPosX(new_start_x);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

                void* ptr = f.getPtr(componentThis);
                if (!ptr) {
                    ImGui::TextDisabled("null");
                    continue;
                }

                // ONLY provide category-level draw for string (other types already above)
                if (f.type == std::type_index(typeid(std::string))) {
                    std::string* s = reinterpret_cast<std::string*>(ptr);

                    size_t buf_size = std::max<size_t>(256, s->size() + 1);
                    std::vector<char> buffer(buf_size);

                    memcpy(buffer.data(), s->c_str(), s->size());
                    buffer[s->size()] = '\0';

                    if (ImGui::InputText(
                        (std::string("##") + f.name).c_str(),
                        buffer.data(), buffer.size()))
                    {
                        *s = std::string(buffer.data());
                    }
                    continue;
                }

                ImGui::TextDisabled("Unsupported Type");
            }
        }
    }
}
