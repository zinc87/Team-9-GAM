#include "pch.h"
#include "AssetCompiler.h"

void AG::AssetCompiler::OpenStaticMeshImport(const std::filesystem::path& path)
{
	// 1. Open the window
	enable = true;

	// 2. Switch state to Static Mesh
	compile_type = MESH;

	// 3. Set the path
	m_TargetMeshStr = path.string();

	// NEW: Reset scale to default
	m_BuildScale[0] = 1.0f;
	m_BuildScale[1] = 1.0f;
	m_BuildScale[2] = 1.0f;

	m_BuildRotation[0] = 0.0f; m_BuildRotation[1] = 0.0f; m_BuildRotation[2] = 0.0f; // <--- ADD THIS

	// 4. Trigger the internal logic to load metadata (vertices, materials, etc.)
	// We set the existing flag 'selection_changed' to true so the GUI updates next frame
	selection_changed = true;
	m_TriggerMeshLoad = true; // Helper flag to ensure logic runs
}

void AG::AssetCompiler::OpenSkinnedMeshImport(const std::filesystem::path& path)
{
	// 1. Open the window
	enable = true;

	// 2. Switch state to Skinned/Dynamic Mesh
	compile_type = SKINNED;

	// 3. Set the input path
	m_TargetMeshStr = path.string();

	// 4. Auto-fill Output Name and Location based on input
	std::filesystem::path inpath(path);
	m_OutFileStr = inpath.stem().string();
	m_OutLocStr = inpath.parent_path().string();

	// 5. Trigger internal logic (flags used in RenderGUI loops)
	selection_changed = true;
	m_TriggerMeshLoad = true;
}


void RunSilent(const std::string& command)
{
	STARTUPINFOA si{};
	PROCESS_INFORMATION pi{};
	si.cb = sizeof(si);

	// Launch hidden (no console)
	if (CreateProcessA(
		nullptr,
		const_cast<LPSTR>(command.c_str()),
		nullptr,
		nullptr,
		FALSE,
		CREATE_NO_WINDOW,   // <-- key line
		nullptr,
		nullptr,
		&si,
		&pi))
	{
		// Wait for completion
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}
void AG::AssetCompiler::RenderGUI()
{
	// 1. Trigger the popup if the boolean is true
	if (enable)
	{
		ImGui::OpenPopup("Asset Compiler");
	}

	// 2. Center the Modal (Standard UX for modals)
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	// 3. Set Size (Use ImGuiCond_FirstUseEver so the user can resize it if needed)
	ImGui::SetNextWindowSize(ImVec2(800.f, 900.f), ImGuiCond_FirstUseEver);

	// 4. Begin Modal
	// Passing '&enable' allows the user to close it via the 'X' button or clicking outside (if configured)
	if (ImGui::BeginPopupModal("Asset Compiler", &enable, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
	{
		static std::vector<std::string> compStr =
		{
			"Texture", "Mesh", "Cube Map"
		};

		if (compile_type == NONE)
		{
			// --- Selection Menu ---

			ImGui::SetCursorPos(ImVec2(25.f, 40.f));
			if (ImGui::Button("Texture", ImVec2(200.f, 50.f))) compile_type = TEXTURE;

			ImGui::SetCursorPos(ImVec2(250.f, 40.f));
			ImGui::TextWrapped("Convert generic image format (eg. png/jpg) into GPU ready format (dds)");

			ImGui::SetCursorPos(ImVec2(25.f, 100.f));
			if (ImGui::Button("Static Mesh", ImVec2(200.f, 50.f))) compile_type = MESH;

			ImGui::SetCursorPos(ImVec2(250.f, 100.f));
			ImGui::TextWrapped("Convert FBX format into non-animated engine custom format (agstaticmesh) for faster reading.");

			ImGui::SetCursorPos(ImVec2(25.f, 160.f));
			if (ImGui::Button("Dynamic Mesh", ImVec2(200.f, 50.f))) compile_type = SKINNED;

			ImGui::SetCursorPos(ImVec2(250.f, 160.f));
			ImGui::TextWrapped("Convert FBX format into animated engine custom format (agskinnedmesh) for faster reading.");

			ImGui::SetCursorPos(ImVec2(25.f, 220.f));
			if (ImGui::Button("Cube Map", ImVec2(200.f, 50.f))) compile_type = CUBEMAP;

			ImGui::SetCursorPos(ImVec2(250.f, 220.f));
			ImGui::TextWrapped("Create cube map (agcubemap) for sky box using (6) different texture.");

			ImGui::SetCursorPos(ImVec2(25.f, 280.f));
			if (ImGui::Button("Font", ImVec2(200.f, 50.f))) compile_type = FONT;

			ImGui::SetCursorPos(ImVec2(250.f, 280.f));
			ImGui::TextWrapped("Convert TTF format font file into agfont");

			ImGui::SetCursorPos(ImVec2(25.f, 340.f));
			if (ImGui::Button("Material", ImVec2(200.f, 50.f))) compile_type = MATERIAL;

			ImGui::SetCursorPos(ImVec2(250.f, 340.f));
			ImGui::TextWrapped("Create Material");

			// Optional: Close Button at the bottom for UX
			ImGui::SetCursorPos(ImVec2(25.f, 800.f)); // Adjust Y as needed
			if (ImGui::Button("Close", ImVec2(100.f, 30.f)))
			{
				enable = false;
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			// --- Sub-Menus ---

			// Helpful addition: A "Back" button to return to the main selection
			if (ImGui::Button("<< Back"))
			{
				compile_type = NONE;
			}
			ImGui::Separator();

			switch (compile_type)
			{
			case TEXTURE: CompileTextureGUI(); break;
			case MESH: CompileStaticMeshGUI(); break;
			case SKINNED: CompileSkinnedGUI(); break;
			case CUBEMAP: CompileCubeMapGUI(); break;
			case FONT: CompileFontGUI(); break;
			case MATERIAL: CompileMaterialGUI(); break;
			default: break;
			}
		}

		// Must use EndPopup() for modals, not End()
		ImGui::EndPopup();
	}
}

void AG::AssetCompiler::CompileAsset()
{

}


void AG::AssetCompiler::CompileTextureGUI()
{
	if (ImGui::Button("<-"))
	{
		compile_type = NONE;
	}
	readytocompile = true;
	static std::string outfile;
	static std::string outloc;
	static std::string infile;

	static int compressionMode = 4; // Default to BC3 (DXT5)

	ImGui::SeparatorText("Import");
	float seperation = ImGui::GetContentRegionAvail().x * 0.3f;
	float item_width = ImGui::GetContentRegionAvail().x * 0.7f;
	float explore_width = ImGui::GetFrameHeight();
	ImGui::Text("Targeted Texture: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);
	ImGui::InputText("##infile", &infile); ImGui::SameLine();
	if (ImGui::Button("O##inf", { explore_width ,explore_width }))
	{
		infile = OpenFileDialog();
		std::filesystem::path inpath(infile);
		outfile = inpath.stem().string();
		outloc = inpath.parent_path().string();
	}

	if (selection_changed)
	{
		outfile = std::filesystem::path(infile).filename().replace_extension().string();
		std::filesystem::path inpath(infile);
		outfile = inpath.stem().string();
		outloc = inpath.parent_path().string();
		selection_changed = false;
	}

	if (ASSET_MANAGER.GetFileType(std::filesystem::path(infile)) != ASSET_TYPE::TEXTURE)
	{
		readytocompile = false;
		ImGui::TextColored({ 1,0,0,1 }, "(The selected asset is not a valid texture format)");
	}
	else ImGui::TextColored({ 0,1,0,1 }, "(Valid)");

	ImGui::SeparatorText("Color Space");
	RadioButtonRight("Linear##Color", &colorSpace, 0);
	RadioButtonRight("sRGB", &colorSpace, 1);

	ImGui::SeparatorText("Wrap Mode");
	RadioButtonRight("Repeat", &wrapMode, 0);
	RadioButtonRight("Clamp", &wrapMode, 1);
	RadioButtonRight("Mirror", &wrapMode, 2);

	ImGui::SeparatorText("Filter Mode");
	RadioButtonRight("Nearest", &filterMode, 0);
	RadioButtonRight("Linear", &filterMode, 1);

	ImGui::SeparatorText("MipMap");
	CheckBoxRight("Generate MipMap##gmm", &mipmap);

	// --- ADD THIS: MAX TEXTURE SIZE ---
	static int maxTextureSize = 0;
	ImGui::SeparatorText("Max Texture Size");
	const char* maxSizes[] = { "Original", "1024", "2048", "4096" };
	// Use a Combo box for selection
	if (ImGui::BeginCombo("##maxSz", maxSizes[maxTextureSize]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(maxSizes); n++)
		{
			bool is_selected = (maxTextureSize == n);
			if (ImGui::Selectable(maxSizes[n], is_selected))
				maxTextureSize = n;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	// ----------------------------------

	// --- ADD THIS ---
	static bool genNormalMap = false;
	static float normalStrength = 2.0f;
	ImGui::SeparatorText("Post-Processing");
	CheckBoxRight("Gen Normal Map##gnm", &genNormalMap);
	if (genNormalMap)
	{
		ImGui::Text("Bumpiness"); ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
		ImGui::DragFloat("##bmp", &normalStrength, 0.1f, 0.1f, 20.0f);
	}
	// ----------------


	// --- ADD THIS NEW SECTION ---
	ImGui::SeparatorText("Compression Format");
	RadioButtonRight("None (Uncompressed R8G8B8A8)", &compressionMode, 0);
	RadioButtonRight("BC1 (DXT1, RGB/1-bit Alpha)", &compressionMode, 1);
	RadioButtonRight("BC3 (DXT5, RGBA)", &compressionMode, 2);
	RadioButtonRight("BC5 (Normal Maps, RG channels)", &compressionMode, 3);
	RadioButtonRight("BC7 (High Quality RGBA)", &compressionMode, 4);
	// --- END OF NEW SECTION ---

	ImGui::SeparatorText("Output");
	ImGui::Text("Output File:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width);
	ImGui::InputText("##outfile", &outfile);
	if (outfile.empty())	ImGui::TextColored({ 1,0,0,1 }, "(Output file name cannot be empty)");
	else					ImGui::TextColored({ 0,1,0,1 }, "(Valid)");

	ImGui::Text("Output Location:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);

	ImGui::InputText("##outloc", &outloc); ImGui::SameLine();
	if (ImGui::Button("O##outl", { explore_width ,explore_width }))
	{
		outloc = OpenFolderDialog();
	}

	static std::string outfinal;
	outfinal = outloc + "\\" + outfile + ".dds";

	ImGui::TextWrapped("Exporting to : %s", outfinal.c_str());

	/* --- Button on bottom right --- */
	ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - ImGui::GetFrameHeightWithSpacing());
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Convert").x - ImGui::GetStyle().FramePadding.x * 2);
	if (ImGui::Button("Compile"))
	{
		if (!outfile.empty() && readytocompile)
		{
			/* --- load using STB -> write into binary -> unload STB -> load from binary --- */
			std::shared_ptr<TextureAsset> precomp = std::make_shared<TextureAsset>();

			TextureCompressionFormat format = TextureCompressionFormat::BC7; // Default
			switch (compressionMode)
			{
			case 0: format = TextureCompressionFormat::None; break;
			case 1: format = TextureCompressionFormat::BC1; break;
			case 2: format = TextureCompressionFormat::BC3; break;
			case 3: format = TextureCompressionFormat::BC5; break;
			case 4: format = TextureCompressionFormat::BC7; break;
			}

			precomp->ConvertToDDS(infile, outfinal, colorSpace, wrapMode, filterMode, mipmap, format,
				maxTextureSize,
				genNormalMap,   // <--- ADD THIS
				normalStrength  // <--- ADD THIS
			);
			precomp->asset_name = outfile + ".dds";
			precomp->LoadFromDDS(outfinal);
			AssetManager::GetInstance().RegisterAsset(DDS, AssetManager::GetInstance().hasher(outfile + ".dds"), precomp);
		}

	}

}

void AG::AssetCompiler::CompileStaticMeshGUI()
{
	if (ImGui::Button("<-"))
	{
		compile_type = NONE;
	}
	readytocompile = true;

	// REMOVED: static std::string target_mesh, outfile, outloc, etc.
	// We will use the class member variables (m_TargetMeshStr, etc.) exclusively.

	// --- 1. Consolidated Layout Definitions (Defines variables ONCE) ---
	float availWidth = ImGui::GetContentRegionAvail().x;
	float seperation = availWidth * 0.3f;
	float item_width = availWidth * 0.7f;
	float explore_width = ImGui::GetFrameHeight(); // Square button size

	// Calculate width for transform inputs
	float transformInputWidth = (availWidth - seperation) - explore_width - ImGui::GetStyle().ItemSpacing.x;

	ImGui::SeparatorText("Import");

	ImGui::SeparatorText("Transform (Bake)");


	// --- SCALE SECTION (FIXED UI) ---
	ImGui::Text("Scale:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation);

	ImGui::SetNextItemWidth(transformInputWidth); // <--- FIX: Explicit width
	ImGui::DragFloat3("##scale", m_BuildScale, 0.01f, 0.001f, 1000.0f, "%.3f");

	ImGui::SameLine();
	if (ImGui::Button("R##Scale", ImVec2(explore_width, explore_width))) // <--- FIX: Square button
	{
		m_BuildScale[0] = 1.0f; m_BuildScale[1] = 1.0f; m_BuildScale[2] = 1.0f;
	}


	// --- ROTATION SECTION (NEW) ---
	ImGui::Text("Rotation:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation);

	ImGui::SetNextItemWidth(transformInputWidth);
	ImGui::DragFloat3("##rot", m_BuildRotation, 0.1f, -360.0f, 360.0f, "%.1f deg");

	ImGui::SameLine();
	if (ImGui::Button("R##Rot", ImVec2(explore_width, explore_width)))
	{
		m_BuildRotation[0] = 0.0f; m_BuildRotation[1] = 0.0f; m_BuildRotation[2] = 0.0f;
	}

	// ---------------------------------------------------------



	ImGui::Text("Targeted Mesh: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);

	// 1. Use Member Variable for Input
	ImGui::InputText("##infile", &m_TargetMeshStr); ImGui::SameLine();

	if (ImGui::Button("O##inf", { explore_width ,explore_width }))
	{
		// 2. Update Member Variable directly from Dialog
		m_TargetMeshStr = OpenFileDialog();
		selection_changed = true;
	}

	static StaticMeshAsset meta_mesh;

	// 3. Logic to auto-fill data (triggered by Content Browser OR File Dialog)
	if (selection_changed || m_TriggerMeshLoad)
	{
		if (!m_TargetMeshStr.empty())
		{
			// Get Metadata
			meta_mesh.GetMetaData(m_TargetMeshStr, m_MetaVert, m_MetaSubmesh, m_MetaMaterials);

			// Set Output Names based on input filename
			std::filesystem::path inpath(m_TargetMeshStr);
			m_OutFileStr = inpath.stem().string();
			m_OutLocStr = inpath.parent_path().string();
		}

		// Reset triggers
		selection_changed = false;
		m_TriggerMeshLoad = false;
	}

	// 4. Validation using Member Variable
	if (ASSET_MANAGER.GetFileType(std::filesystem::path(m_TargetMeshStr)) != ASSET_TYPE::MODEL
		&& ASSET_MANAGER.GetFileType(std::filesystem::path(m_TargetMeshStr)) != ASSET_TYPE::AGSKINNEDMESH)
	{
		readytocompile = false;
		ImGui::TextColored({ 1,0,0,1 }, "(The selected asset is not a valid model format)");
	}
	else ImGui::TextColored({ 0,1,0,1 }, "(Valid)");

	ImGui::SeparatorText("Import");
	static int meshMode = 0;
	RadioButtonRight("Static Mesh", &meshMode, 0);

	ImGui::SeparatorText("Meta Data");
	ImGui::Text("Vertices: "); ImGui::SameLine(); ImGui::SetCursorPosX(seperation);
	ImGui::Text("%d", m_MetaVert);

	ImGui::Text("Sub Meshes: "); ImGui::SameLine(); ImGui::SetCursorPosX(seperation);
	ImGui::Text("%d", m_MetaSubmesh);

	ImGui::SeparatorText("Texture Maps");
	for (auto& mat : m_MetaMaterials)
	{
		ImGui::BulletText(mat.c_str());
	}

	ImGui::SeparatorText("Output");
	ImGui::Text("Output File:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width);

	// 5. Use Member Variable for Output File
	ImGui::InputText("##outfile", &m_OutFileStr);
	if (m_OutFileStr.empty())    ImGui::TextColored({ 1,0,0,1 }, "(Output file name cannot be empty)");
	else                    ImGui::TextColored({ 0,1,0,1 }, "(Valid)");

	ImGui::Text("Output Location:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);

	// 6. Use Member Variable for Output Location
	ImGui::InputText("##outloc", &m_OutLocStr); ImGui::SameLine();
	if (ImGui::Button("O##outl", { explore_width ,explore_width }))
	{
		m_OutLocStr = OpenFolderDialog();
	}

	static std::string outmeshfinal;
	std::string ext = meshMode == 0 ? ".agstaticmesh" : ".agdynamicmesh";
	outmeshfinal = m_OutLocStr + "\\" + m_OutFileStr + ext;

	ImGui::TextWrapped("Exporting to : %s", outmeshfinal.c_str());

	/* --- Compile Button --- */
	ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - ImGui::GetFrameHeightWithSpacing());
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Convert").x - ImGui::GetStyle().FramePadding.x * 2);

	if (ImGui::Button("Compile"))
	{
		if (!m_OutFileStr.empty() && readytocompile)
		{
			std::shared_ptr<StaticMeshAsset> precomp = std::make_shared<StaticMeshAsset>();

			// 1. Load the raw FBX data
			precomp->Load(m_TargetMeshStr);

			// =========================================================
		// NEW: RECENTER GEOMETRY FIX
		// =========================================================

		// A. Calculate the Bounding Box Center
			float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
			float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;
			bool hasVerts = false;

			for (const auto& sub : precomp->submeshes) {
				for (const auto& v : sub.vertices) {
					// v.position is a float[3], so we access indices manually
					if (v.position[0] < minX) minX = v.position[0];
					if (v.position[1] < minY) minY = v.position[1];
					if (v.position[2] < minZ) minZ = v.position[2];

					if (v.position[0] > maxX) maxX = v.position[0];
					if (v.position[1] > maxY) maxY = v.position[1];
					if (v.position[2] > maxZ) maxZ = v.position[2];
					hasVerts = true;
				}
			}

			if (hasVerts) {
				// Center = Midpoint of the Bounding Box
				float cX = (minX + maxX) * 0.5f;
				float cY = (minY + maxY) * 0.5f;
				float cZ = (minZ + maxZ) * 0.5f;

				// Log it so we know it happened
				// AG_CORE_WARN("Fixing Mesh Offset by subtracting: {}, {}, {}", cX, cY, cZ);

				// B. Shift all vertices by this center
				for (auto& sub : precomp->submeshes) {
					for (auto& v : sub.vertices) {
						v.position[0] -= cX;
						v.position[1] -= cY;
						v.position[2] -= cZ;
					}
				}
			}
			// =========================================================
			// END FIX
			// =========================================================


			// =========================================================
// 2. APPLY ROTATION (FIXED)
// =========================================================
			if (m_BuildRotation[0] != 0.0f || m_BuildRotation[1] != 0.0f || m_BuildRotation[2] != 0.0f)
			{
				// Helper lambda (same as before)
				auto RotateVector = [&](float* vec, float xDeg, float yDeg, float zDeg)
					{
						float rad = 3.14159265359f / 180.0f;
						float cX = cos(xDeg * rad), sX = sin(xDeg * rad);
						float cY = cos(yDeg * rad), sY = sin(yDeg * rad);
						float cZ = cos(zDeg * rad), sZ = sin(zDeg * rad);

						float x = vec[0], y = vec[1], z = vec[2];

						// Rotate X
						float y1 = y * cX - z * sX;
						float z1 = y * sX + z * cX;
						float x1 = x;

						// Rotate Y
						float x2 = x1 * cY + z1 * sY;
						float z2 = -x1 * sY + z1 * cY;
						float y2 = y1;

						// Rotate Z
						float x3 = x2 * cZ - y2 * sZ;
						float y3 = x2 * sZ + y2 * cZ;
						float z3 = z2;

						vec[0] = x3; vec[1] = y3; vec[2] = z3;
					};

				for (auto& sub : precomp->submeshes)
				{
					for (auto& v : sub.vertices)
					{
						// Rotate Position
						RotateVector(v.position, m_BuildRotation[0], m_BuildRotation[1], m_BuildRotation[2]);

						// Rotate Normal
						RotateVector(v.normal, m_BuildRotation[0], m_BuildRotation[1], m_BuildRotation[2]);

						// Rotate Tangent
						RotateVector(v.tangent, m_BuildRotation[0], m_BuildRotation[1], m_BuildRotation[2]);

						// Rotate Bitangent (CHANGED FROM v.binormal TO v.bitangent)
						RotateVector(v.bitangent, m_BuildRotation[0], m_BuildRotation[1], m_BuildRotation[2]);
					}
				}
			}


			// 2. APPLY SCALING HERE (Before writing to binary)
			// This modifies the vertex data permanently for this asset.
			precomp->ApplyScaling(m_BuildScale[0], m_BuildScale[1], m_BuildScale[2]);

			// 3. Write the modified data to your custom format
			precomp->WriteToBinary(outmeshfinal);

			// 4. Reload so the engine uses the new data immediately
			precomp->Unload();
			precomp->LoadFromBinary(outmeshfinal);

			precomp->asset_name = m_OutFileStr + ext;
			AssetManager::GetInstance().RegisterAsset(AGSTATICMESH, AssetManager::GetInstance().hasher(precomp->asset_name), precomp);
		}
	}
}

void AG::AssetCompiler::CompileCubeMapGUI()
{
	if (ImGui::Button("<-"))
	{
		compile_type = NONE;
	}

	float seperation = ImGui::GetContentRegionAvail().x * 0.3f;
	float item_width = ImGui::GetContentRegionAvail().x * 0.7f;
	float explore_width = ImGui::GetFrameHeight();

	ImGui::SeparatorText("Faces");

	// --- RIGHT ---
	ImGui::Text("Right: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);
	ImGui::InputText("##right", &m_CM_Right); ImGui::SameLine(); // CHANGED to member var
	if (ImGui::Button("O##right", { explore_width ,explore_width }))
	{
		m_CM_Right = OpenFileDialog();
	}

	// --- LEFT ---
	ImGui::Text("Left: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);
	ImGui::InputText("##left", &m_CM_Left); ImGui::SameLine(); // CHANGED
	if (ImGui::Button("O##left", { explore_width ,explore_width }))
	{
		m_CM_Left = OpenFileDialog();
	}

	// --- DOWN ---
	ImGui::Text("Down: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);
	ImGui::InputText("##down", &m_CM_Down); ImGui::SameLine(); // CHANGED
	if (ImGui::Button("O##down", { explore_width ,explore_width }))
	{
		m_CM_Down = OpenFileDialog();
	}

	// --- TOP ---
	ImGui::Text("Top: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);
	ImGui::InputText("##top", &m_CM_Top); ImGui::SameLine(); // CHANGED
	if (ImGui::Button("O##top", { explore_width ,explore_width }))
	{
		m_CM_Top = OpenFileDialog();
	}

	// --- FRONT ---
	ImGui::Text("Front: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);
	ImGui::InputText("##front", &m_CM_Front); ImGui::SameLine(); // CHANGED
	if (ImGui::Button("O##front", { explore_width ,explore_width }))
	{
		m_CM_Front = OpenFileDialog();
	}

	// --- BACK ---
	ImGui::Text("Back: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);
	ImGui::InputText("##back", &m_CM_Back); ImGui::SameLine(); // CHANGED
	if (ImGui::Button("O##back", { explore_width ,explore_width }))
	{
		m_CM_Back = OpenFileDialog();
	}

	ImGui::SeparatorText("Output");
	ImGui::Text("Output File:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width);

	ImGui::InputText("##outfile", &m_CM_OutFile); // CHANGED

	ImGui::Text("Output Location:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);

	ImGui::InputText("##outloc", &m_CM_OutLoc); ImGui::SameLine(); // CHANGED
	if (ImGui::Button("O##outl", { explore_width ,explore_width }))
	{
		m_CM_OutLoc = OpenFolderDialog();
	}

	static std::string outfinal;
	outfinal = m_CM_OutLoc + "\\" + m_CM_OutFile + ".agcubemap";

	ImGui::TextWrapped("Exporting to : %s", outfinal.c_str());

	// final confirmation for cubemap
	std::vector<std::string> precomp_face{ m_CM_Right, m_CM_Left, m_CM_Down, m_CM_Top, m_CM_Front, m_CM_Back };
	std::vector<std::string> face_id{ "Right", "Left","Down","Top","Front","Back" };

	// ... Validation Logic (remains mostly the same, just using local vector) ... 
	for (size_t i = 0; i < 6; i++)
	{
		bool valid = true;
		auto fs_p_fn = std::filesystem::path(precomp_face[i]).filename();
		std::string temp_ext = fs_p_fn.extension().string();
		if (temp_ext != ".agtex" && temp_ext != ".dds" && temp_ext != ".jpg" && temp_ext != ".png")
			valid = false;

		ImGui::Text((face_id[i] + ": ").c_str()); ImGui::SameLine(); ImGui::SetCursorPosX(seperation);
		if (precomp_face[i].empty())
			ImGui::TextColored({ 1,0,0,1 }, "pick a texture!");
		else
			if (valid) ImGui::Text(precomp_face[i].c_str());
			else ImGui::TextColored({ 1,0,0,1 }, (precomp_face[i] + " (pick a proper image file)").c_str());
	}

	/* --- Compile Button --- */
	ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - ImGui::GetFrameHeightWithSpacing());
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Convert").x - ImGui::GetStyle().FramePadding.x * 2);
	if (ImGui::Button("Compile"))
	{
		std::vector<std::size_t> precomp_hash(6);
		for (int i = 0; i < 6; ++i) precomp_hash[i] = HASH(std::filesystem::path(precomp_face[i]).filename().string());

		std::shared_ptr<CubeMapAsset> precomp = std::make_shared<CubeMapAsset>();
		precomp->GenerateCubeMap(precomp_hash, m_CM_OutFile + ".agcubemap");
		precomp->WriteToBinary(outfinal);
		AssetManager::GetInstance().RegisterAsset(AGCUBEMAP, AssetManager::GetInstance().hasher(m_CM_OutFile + ".agcubemap"), precomp);
	}
}

void AG::AssetCompiler::CompileFontGUI()
{
	ImGui::Text("Font file"); ImGui::SameLine();

	// REMOVED: static std::string in_font; 
	// CHANGED: Use m_FontInputPath
	ImGui::InputText("##infont", &m_FontInputPath); ImGui::SameLine();

	if (ImGui::Button("##pick_font", ImVec2(25.f, 25.f)))
	{
		m_FontInputPath = OpenFileDialog();
	}

	// CHANGED: Use m_FontInputPath
	bool valid = ASSET_MANAGER.GetFileType(m_FontInputPath) == ASSET_TYPE::FONT;
	if (!valid)
	{
		ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Select a font file (ttf)");
	}

	// REMOVED: static std::string out_name;
	// CHANGED: Use m_FontOutputName
	ImGui::Text("Output Name"); ImGui::SameLine();
	ImGui::InputText("##outname", &m_FontOutputName);

	// REMOVED: static std::string out_dir;
	// CHANGED: Use m_FontOutputDir
	ImGui::Text("Output Directory"); ImGui::SameLine();
	ImGui::InputText("##outdir", &m_FontOutputDir); ImGui::SameLine();

	if (ImGui::Button("##out_dir", ImVec2(25.f, 25.f)))
	{
		m_FontOutputDir = OpenFolderDialog();
	}

	std::string final_output(m_FontOutputDir + "\\" + m_FontOutputName + ".agfont");
	ImGui::Text("Output %s:", final_output.c_str());

	ImGui::BeginDisabled(!valid);
	if (ImGui::Button("Convert", ImVec2(ImGui::GetContentRegionAvail().x, 25.f)))
	{
		std::shared_ptr<FontAsset> precomp = std::make_shared<FontAsset>();

		// CHANGED: Use member variables
		precomp->Load(m_FontInputPath);
		precomp->WriteToBinary(final_output);
		precomp->Unload();
		precomp->LoadFromBinary(final_output);

		AssetManager::GetInstance().RegisterAsset(AGFONT, AssetManager::GetInstance().hasher(m_FontOutputName + ".agfont"), precomp);
	}
	ImGui::EndDisabled();
}

void AG::AssetCompiler::CompileSkinnedGUI()
{

	if (ImGui::Button("<-"))
	{
		compile_type = NONE;
	}
	ImGui::SameLine();
	ImGui::Text("Skinned Mesh Compiler");
	ImGui::Separator();

	readytocompile = true;

	// --- Layout Definitions ---
	float availWidth = ImGui::GetContentRegionAvail().x;
	float seperation = availWidth * 0.3f;
	float item_width = availWidth * 0.7f;
	float explore_width = ImGui::GetFrameHeight();

	// --- Import Section ---
	ImGui::SeparatorText("Import Settings");

	ImGui::Text("Target FBX: "); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation);
	ImGui::SetNextItemWidth(item_width - explore_width);

	// Using existing member variable m_TargetMeshStr
	ImGui::InputText("##sk_infile", &m_TargetMeshStr); ImGui::SameLine();

	if (ImGui::Button("O##sk_inf", { explore_width, explore_width }))
	{
		m_TargetMeshStr = OpenFileDialog(); // Should filter for .fbx, .obj, etc.
		selection_changed = true;
	}

	// Logic to auto-fill data (triggered by selection change)
	//static SkinnedMeshAsset meta_skinned;
	//if (selection_changed || m_TriggerMeshLoad)
	//{
	//	if (!m_TargetMeshStr.empty())
	//	{
	//		// Note: You might need a GetMetaData specifically for skinned meshes 
	//		// if it differs from static, but usually it populates bone counts.
	//		meta_skinned.GetMetaData(m_TargetMeshStr, m_MetaVert, m_MetaSubmesh, m_MetaMaterials);

	//		std::filesystem::path inpath(m_TargetMeshStr);
	//		m_OutFileStr = inpath.stem().string();
	//		m_OutLocStr = inpath.parent_path().string();
	//	}
	//	selection_changed = false;
	//	m_TriggerMeshLoad = false;
	//}

	// Validation
	if (ASSET_MANAGER.GetFileType(std::filesystem::path(m_TargetMeshStr)) != ASSET_TYPE::MODEL)
	{
		readytocompile = false;
		ImGui::TextColored({ 1,0,0,1 }, "(Please select a valid FBX/Model file)");
	}
	else ImGui::TextColored({ 0,1,0,1 }, "(FBX Loaded)");

	// --- Metadata Display ---
	ImGui::SeparatorText("Mesh Info");
	ImGui::Text("Vertices: "); ImGui::SameLine(); ImGui::SetCursorPosX(seperation);
	ImGui::Text("%d", m_MetaVert);

	// For Skinned Meshes, showing Bone count is helpful
	// ImGui::Text("Bones Found: "); ImGui::SameLine(); ImGui::SetCursorPosX(seperation);
	// ImGui::Text("%zu", meta_skinned.bones.size());

	// --- Output Section ---
	ImGui::SeparatorText("Output Settings");

	ImGui::Text("Output File:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width);
	ImGui::InputText("##sk_outfile", &m_OutFileStr);

	ImGui::Text("Output Location:"); ImGui::SameLine();
	ImGui::SetCursorPosX(seperation); ImGui::SetNextItemWidth(item_width - explore_width);
	ImGui::InputText("##sk_outloc", &m_OutLocStr); ImGui::SameLine();

	if (ImGui::Button("O##sk_outl", { explore_width, explore_width }))
	{
		m_OutLocStr = OpenFolderDialog();
	}

	std::string outmeshfinal = m_OutLocStr + "\\" + m_OutFileStr + ".agskinnedmesh";
	ImGui::TextWrapped("Full Path: %s", outmeshfinal.c_str());

	// --- Compile Button ---
	ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - ImGui::GetFrameHeightWithSpacing());
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Compile Skinned").x - ImGui::GetStyle().FramePadding.x * 4);

	if (ImGui::Button("Compile Skinned"))
	{
		if (!m_OutFileStr.empty() && readytocompile)
		{
			auto precomp = std::make_shared<SkinnedMeshAsset>();

			// 1. Load the FBX using Assimp logic
			precomp->Load(m_TargetMeshStr);

			// 2. Write to your custom binary format
			precomp->WriteToBinary(outmeshfinal);

			// 3. Hot-reload into the engine
			precomp->Unload();
			precomp->LoadFromBinary(outmeshfinal);

			precomp->asset_name = m_OutFileStr + ".agskinnedmesh";
			AssetManager::GetInstance().RegisterAsset(AGSKINNEDMESH,
				AssetManager::GetInstance().hasher(precomp->asset_name), precomp);
		}
	}


}


void AG::AssetCompiler::OpenCubeMapImport()
{
	// 1. Open the window
	enable = true;

	// 2. Switch tab
	compile_type = CUBEMAP;

	// 3. Clear all fields so they are blank (as requested)
	m_CM_Right.clear();
	m_CM_Left.clear();
	m_CM_Down.clear();
	m_CM_Top.clear();
	m_CM_Front.clear();
	m_CM_Back.clear();
	m_CM_OutFile.clear();
	m_CM_OutLoc.clear();
}
void AG::AssetCompiler::CompileMaterialGUI()
{
	static MaterialTemplate mat;
	static std::string directory = "Assets/Materials/";

	// --- Prepare Texture Lists ---
	std::vector<std::string> s_tex = { "None" }; // Ensure first item is "None" or empty
	for (auto& tex : AssetManager::GetInstance().GetAssets(ASSET_TYPE::DDS))
	{
		s_tex.push_back(tex.second.lock()->asset_name);
	}
	std::vector<const char*> c_tex;
	for (auto& str : s_tex)
	{
		c_tex.push_back(str.c_str());
	}

	float width = ImGui::GetContentRegionAvail().x;

	// --- Header Info ---
	ImGui::Text("Name: "); ImGui::SameLine();
	ImGui::SetCursorPosX(width * 0.25f);
	ImGui::InputText("##mat_inp_name", &mat.material_name);

	ImGui::Text("Directory (FIXED): %s", directory.c_str());

	// =========================================================
	// ALBEDO BOX (Searchable Fix)
	// =========================================================
	ImGui::SetCursorPosX((width - ImGui::CalcTextSize("Albedo").x) * 0.5f);
	ImGui::Text("Albedo");
	ImGui::BeginChild("##Albedo", ImVec2(width, 80.f), 1);

	// Image Preview Logic
	ImTextureID albedo_id = 0;
	std::string albedo_name;
	auto albedo_wk = AssetManager::GetInstance().GetAsset(mat.albedo_hash);
	if (albedo_wk.lock())
	{
		auto albedo_shr = std::dynamic_pointer_cast<TextureAsset>(albedo_wk.lock());
		if (albedo_shr)
		{
			albedo_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(albedo_shr->textureID));
			albedo_name = albedo_shr->asset_name;
		}
	}
	ImGui::Image(albedo_id, ImVec2(63.f, 63.f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, 1.f));
	ImGui::SameLine();

	// Calculate current index for display name
	int albedo_idx = 0;
	for (int i = 0; i < s_tex.size(); i++)
	{
		// Use HASH on the string to compare with stored hash
		// Note: Check if s_tex[i] is empty/None handle specifically if needed
		if (mat.albedo_hash == HASH(s_tex[i]))
		{
			albedo_idx = i;
			break;
		}
	}

	const char* current_albedo_preview = (albedo_idx >= 0 && albedo_idx < c_tex.size()) ? c_tex[albedo_idx] : "None";

	// --- SEARCHABLE COMBO START ---
	static ImGuiTextFilter albedoFilter;
	if (ImGui::BeginCombo("##pickAlbedo", current_albedo_preview))
	{
		// 1. Search Bar
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere(); // Focus on search bar immediately

		albedoFilter.Draw("##albedo_search", -1.0f);

		ImGui::Separator();

		// 2. Filtered List
		for (int i = 0; i < c_tex.size(); i++)
		{
			if (albedoFilter.PassFilter(c_tex[i]))
			{
				bool is_selected = (albedo_idx == i);
				if (ImGui::Selectable(c_tex[i], is_selected))
				{
					if (i == 0) mat.albedo_hash = 0;
					else mat.albedo_hash = HASH(s_tex[i]);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	// --- SEARCHABLE COMBO END ---

	ImGui::EndChild();

	// Drag Drop Logic
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMGUI_CONTENT"))
		{
			mat.albedo_hash = *(const size_t*)payload->Data;
		}
		ImGui::EndDragDropTarget();
	}


	// =========================================================
	// NORMAL MAP BOX (Searchable Fix)
	// =========================================================
	ImGui::SetCursorPosX((width - ImGui::CalcTextSize("Normal Map").x) * 0.5f);
	ImGui::Text("Normal Map");
	ImGui::BeginChild("##NormalMap", ImVec2(width, 80.f), 1);

	// Image Preview Logic
	ImTextureID normal_id = 0;
	auto normal_wk = AssetManager::GetInstance().GetAsset(mat.normal_hash);
	if (normal_wk.lock())
	{
		auto normal_shr = std::dynamic_pointer_cast<TextureAsset>(normal_wk.lock());
		if (normal_shr)
		{
			normal_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(normal_shr->textureID));
		}
	}
	ImGui::Image(normal_id, ImVec2(63.f, 63.f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, 1.f));
	ImGui::SameLine();

	// Calculate current index
	int normal_idx = 0;
	for (int i = 0; i < s_tex.size(); i++)
	{
		if (mat.normal_hash == HASH(s_tex[i]))
		{
			normal_idx = i;
			break;
		}
	}

	const char* current_normal_preview = (normal_idx >= 0 && normal_idx < c_tex.size()) ? c_tex[normal_idx] : "None";

	// --- SEARCHABLE COMBO START ---
	static ImGuiTextFilter normalFilter;
	if (ImGui::BeginCombo("##pickNormal", current_normal_preview))
	{
		// 1. Search Bar
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		normalFilter.Draw("##normal_search", -1.0f);

		ImGui::Separator();

		// 2. Filtered List
		for (int i = 0; i < c_tex.size(); i++)
		{
			if (normalFilter.PassFilter(c_tex[i]))
			{
				bool is_selected = (normal_idx == i);
				if (ImGui::Selectable(c_tex[i], is_selected))
				{
					if (i == 0) mat.normal_hash = 0;
					else mat.normal_hash = HASH(s_tex[i]);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	// --- SEARCHABLE COMBO END ---

	ImGui::EndChild();

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMGUI_CONTENT"))
		{
			mat.normal_hash = *(const size_t*)payload->Data;
		}
		ImGui::EndDragDropTarget();
	}

	// =========================================================
	// REMAINING UI (Base Color, Sliders, Save Button)
	// =========================================================

	// BASE COLOR
	ImGui::SetCursorPosX((width - ImGui::CalcTextSize("Base Color").x) * 0.5f);
	ImGui::Text("Base Color");
	ImGui::BeginChild("##BaseColor", ImVec2(width, 380.f), ImGuiChildFlags_Borders);
	ImGui::ColorPicker4("##color", &mat.baseColor.r);
	ImGui::EndChild();

	// Metallic / Roughness / etc
	ImGui::BeginChild("##etc");
	ImGui::Text("Metallic "); ImGui::SameLine();
	ImGui::SetCursorPosX(width * 0.25f);
	ImGui::SliderFloat("##metal", &mat.metallic, 0.f, 1.f);

	ImGui::Text("Roughness "); ImGui::SameLine();
	ImGui::SetCursorPosX(width * 0.25f);
	ImGui::SliderFloat("##rough", &mat.roughness, 0.f, 1.f);

	ImGui::Text("UV Scale "); ImGui::SameLine();
	ImGui::SetCursorPosX(width * 0.25f);
	ImGui::SliderFloat("##uv", &mat.uv_scale, 0.f, 1.f);

	ImGui::Text("Base Color Tint "); ImGui::SameLine();
	ImGui::SetCursorPosX(width * 0.25f);
	ImGui::SliderFloat("##tint", &mat.tint, 0.f, 1.f);
	ImGui::EndChild();

	// SAVE BUTTON
	if (ImGui::Button("Save Material"))
	{
		mat.SaveToJson(directory + "\\" + mat.material_name + ".agmat");
		std::shared_ptr<MaterialTemplate> mat_asset = std::make_shared<MaterialTemplate>();
		mat_asset->LoadFromJson(directory + "\\" + mat.material_name + ".agmat");
		mat_asset->material_name = mat.material_name + ".agmat";
		mat_asset->asset_name = mat.material_name + ".agmat";
		AssetManager::GetInstance().RegisterAsset(AGMATERIAL, AssetManager::GetInstance().hasher(mat.material_name + ".agmat"), mat_asset);
	}
}


// [NEW] Implement the setup function
void AG::AssetCompiler::OpenFontImport(const std::filesystem::path& path)
{
	// 1. Open the window
	enable = true;

	// 2. Switch state to Font
	compile_type = FONT;

	// 3. Auto-fill data based on the clicked file
	m_FontInputPath = path.string();
	m_FontOutputName = path.stem().string();
	m_FontOutputDir = path.parent_path().string();
}

