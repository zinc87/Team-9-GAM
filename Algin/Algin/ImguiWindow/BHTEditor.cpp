#include "pch.h"
#include "BHTEditor.h"

void AG::AGImGui::BHTEditor::Render()
{
	static std::string file_path;
	static std::string file_root;
	static std::string file_name;
	static bool saveas = false;
	static bool new_confirmation = false;
	static std::string dnd_source;

	if (!enable_editor)
		return;

	ImGui::Begin("BHT Editor", &enable_editor);

	ImNodes::BeginNodeEditor();

	if (ImGui::Button("Add Nodes"))
	{
		CreateNode();
	}
	// node_type_list
	s_type.clear();

	for (int i = 0; i < AI::node_string_vec.size(); i++)
	{
		s_type.push_back(AI::node_string_vec[i].second);
	}

	c_type.clear();
	for (auto& type : s_type)
	{
		c_type.push_back(type.c_str());
	}

	RenderNodes(m_root);
	RenderOrphans();

	for (auto& link : m_nodeLinks)
	{
		ImNodes::Link(link.id, link.parent, link.child);
	}


	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImNodes::IsEditorHovered())
	{
		ImGui::OpenPopup("CanvasContextMenu");
	}

	// render pop up
	if (ImGui::BeginPopup("CanvasContextMenu"))
	{

		if (ImGui::MenuItem("Create Node"))
		{
			CreateNode();
		}

		ImGui::EndPopup();
	}

	// start of tool

	static ImVec2 tool_pos = ImVec2(20, 20);
	ImVec2 tool_size = ImVec2(250, 300);

	ImGui::SetCursorPos(tool_pos);

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.8f)); 
	static bool render_tool;
	if (ImGui::Button("Tools", ImVec2(tool_size.x, 20.f)))
	{
		if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			render_tool = !render_tool;
	}

	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		ImVec2 delta = ImGui::GetIO().MouseDelta;
		tool_pos.x += delta.x;
		tool_pos.y += delta.y;

		ImVec2 container_size = ImGui::GetWindowSize(); // Or GetColumnWidth()

		if (tool_pos.x < 0) tool_pos.x = 0;
		if (tool_pos.y < 0) tool_pos.y = 0;

		if (tool_pos.x + tool_size.x > container_size.x) tool_pos.x = container_size.x - tool_size.x;
		if (tool_pos.y + 20.f> container_size.y) tool_pos.y = container_size.y - 20.f;
	}

	if (render_tool)
	{
		ImGui::SetCursorPos(tool_pos + ImVec2(0.f,25.f));
		ImGui::BeginChild("#bhted", tool_size);

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

		float wid = ImGui::GetContentRegionAvail().x;

		ImGui::SetNextItemWidth(wid);

		if (ImGui::Button("New##bht", ImVec2(wid, 25.f)))
		{
			new_confirmation = true;
		}

		if (ImGui::Button("Open", ImVec2(wid, 25.f)))
		{
			std::string file_p = OpenFileDialog();
			if (std::filesystem::path(file_p).extension() == ".bht")
			{
				Reset();
				// load new tree
				t_exportVector = AI::TreeBuilder::GetInstance().getTree(file_p);
				TreeToGUI(t_exportVector);
				// set order for new nodes (else all 0)

				file_root = std::filesystem::path(file_p).parent_path().generic_string();
				file_name = std::filesystem::path(file_p).filename().replace_extension("").generic_string();

			}
		}

		ImGui::SetNextItemWidth(wid);
		if (ImGui::Button("Save", ImVec2(wid, 25.f)))
		{
			if (file_path.empty())
			{
				saveas = true;
			}
			else
			{
				GuiToTree(t_exportVector);
				AI::TreeBuilder::GetInstance().buildTree(std::move(t_exportVector), file_path); //later change to select file name
				AG_CORE_INFO(file_path.c_str());
			}
		}

		ImGui::SetNextItemWidth(wid);
		if (ImGui::Button("Save As", ImVec2(wid, 25.f)))
		{
			saveas = true;
		}



		ImGui::SeparatorText("Hierachy Viewer");
		if (m_root)
		{
			RenderHierarchyNode(m_root);
		}
		if (!t_exportVector.empty())
		{
			ImGui::SeparatorText("Hierachy Viewer");

			for (auto& node : t_exportVector)
			{
				ImGui::Text("Depth: %d", node.depth);
				ImGui::Text("Script Name: %s", node.name.c_str());

				std::string type;
				for (auto& n : AI::node_string_vec)
				{
					if (node.nodeType == n.first)
					{
						type = n.second;
						break;
					}
				}
				ImGui::Text("NodeType: %s", type.c_str());
				ImGui::Separator();
			}
		}

		ImGui::PopStyleColor(2);
		ImGui::EndChild();
	}



	ImGui::PopStyleColor(1);
	ImGui::PopStyleVar(1);

	// end of tool

	ImNodes::EndNodeEditor();

	CaptureNodePositions();
	SortChildrenByY();

	// link creation
	{
		int parent_sock = 0;
		int child_sock = 0;
		if (ImNodes::IsLinkCreated(&parent_sock, &child_sock))
		{
			// parent_sock should be an output attribute
			// child_sock should be an input attribute

			auto parentNode = FindNodeByOutputAttrib(parent_sock);
			auto childNode = FindNodeByInputAttrib(child_sock);

			if (parentNode && childNode)
			{
				// Don't allow children on LEAF nodes if that matters
				if (parentNode->m_type != AI::NodeType::LEAF)
				{
					// If this child already has a parent, detach it from the old parent
					if (auto oldParent = FindParentOf(childNode, m_root))
					{
						DetachChild(oldParent, childNode);
					}

					// Attach to the new parent
					AttachChild(parentNode, childNode);

					// Remove from orphan list
					RemoveFromOrphans(childNode);

					// Create / store link
					m_nodeLinks.emplace_back(link_id++, parent_sock, child_sock);
				}
			}
		}
	}

	// link deletion
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{

		const int selectedNodeCount = ImNodes::NumSelectedNodes();
		if (selectedNodeCount > 0)
		{
			std::vector<int> selectedNodes(selectedNodeCount);
			ImNodes::GetSelectedNodes(selectedNodes.data());

			for (int nodeId : selectedNodes)
			{
				DeleteNodeAndOrphanChildren(nodeId);
			}
		}

		const int selectedLinksCount = ImNodes::NumSelectedLinks();
		if (selectedLinksCount > 0)
		{
			std::vector<int> selectedLinks(selectedLinksCount);
			ImNodes::GetSelectedLinks(selectedLinks.data());

			for (int selId : selectedLinks)
			{
				auto it = std::find_if(
					m_nodeLinks.begin(), m_nodeLinks.end(),
					[selId](const NodeLink& l) { return l.id == selId; });

				if (it == m_nodeLinks.end())
					continue;

				int parent_attr = it->parent; // output pin
				int child_attr = it->child;  // input pin

				auto parentNode = FindNodeByOutputAttrib(parent_attr);
				auto childNode = FindNodeByInputAttrib(child_attr);

				if (parentNode && childNode)
				{
					DetachChild(parentNode, childNode);

					AddToOrphansIfNotPresent(childNode);
				}

				m_nodeLinks.erase(it);
			}
		}
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BHT_CONTENT"))
		{
			dnd_source.assign((const char*)payload->Data, payload->DataSize);
			if (!dnd_source.empty() && dnd_source.back() == '\0')
			{
				dnd_source.pop_back();
			}
			new_confirmation = true;
			AG_CORE_INFO("dnd_source: {}", dnd_source.c_str());
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::End();


	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
	if (saveas)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();

		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		ImGui::SetNextWindowSize(ImVec2(800.f, 100.f));
		ImGui::Begin("Save", &saveas, window_flags);
		ImGui::Text("Directory:"); ImGui::SameLine();

		ImGui::InputText("##dir", &file_root); ImGui::SameLine();
		if (ImGui::Button("Pick Directory##bht"))
		{
			file_root = OpenFolderDialog();
		}
		ImGui::Text("File Name:"); ImGui::SameLine();
		ImGui::InputText("##inputtext", &file_name);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::Button("Save"))
		{
			file_path = file_root + "\\" + file_name + ".bht";
			GuiToTree(t_exportVector);
			AI::TreeBuilder::GetInstance().buildTree(std::move(t_exportVector), file_path);
			AG_CORE_INFO(file_path.c_str());
			saveas = false;
		}
		ImGui::End();
	}

	if (new_confirmation)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		ImGui::SetNextWindowSize(ImVec2(200.f, 100.f));

		ImGui::Begin("Warning", &new_confirmation, window_flags);

		ImGui::TextWrapped("Any unsaved progress will not be saved, are you sure?");

		if (ImGui::Button("Yes"))
		{
			file_path.clear();
			Reset();
			t_exportVector.clear();
			m_exportVector.clear();

			if (!dnd_source.empty())
			{
				t_exportVector = AI::TreeBuilder::GetInstance().getTree(dnd_source);
				TreeToGUI(t_exportVector);
				// set order for new nodes (else all 0)

				file_root = std::filesystem::path(dnd_source).parent_path().generic_string();
				file_name = std::filesystem::path(dnd_source).filename().replace_extension("").generic_string();
				dnd_source.clear();
			}

			new_confirmation = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("No"))
		{
			new_confirmation = false;
		}

		ImGui::End();
	}
}

void AG::AGImGui::BHTEditor::TreeToGUI(const std::vector<AG::AI::NodeRep>& new_tree)
{
	Reset();

	if (new_tree.empty())
		return;

	const auto& rootRep = new_tree[0];

	m_root->m_type = rootRep.nodeType;
	m_root->script_name = rootRep.name;
	m_root->depth = rootRep.depth;

	std::vector<std::shared_ptr<NodeData>> lastAtDepth;
	lastAtDepth.resize(rootRep.depth + 1);
	lastAtDepth[rootRep.depth] = m_root;

	for (size_t i = 1; i < new_tree.size(); ++i)
	{
		const auto& rep = new_tree[i];
		int d = rep.depth;

		if (d < 0)
			continue;

		if (d >= (int)lastAtDepth.size())
			lastAtDepth.resize(d + 1);

		std::shared_ptr<NodeData> parent = nullptr;

		if (d == 0)
		{
			parent = m_root;
		}
		else
		{
			parent = lastAtDepth[d - 1];
		}

		if (!parent)
			parent = m_root;

		auto guiNode = std::make_shared<NodeData>(
			node_id_tracker++,
			node_out_tracker++,
			node_in_tracker++
		);

		guiNode->m_type = rep.nodeType;
		guiNode->script_name = rep.name;
		guiNode->depth = parent->depth + 1;
		guiNode->position_initialized = false; // will be set on first render

		parent->child.push_back(guiNode);

		if (guiNode->depth >= (int)lastAtDepth.size())
			lastAtDepth.resize(guiNode->depth + 1);

		lastAtDepth[guiNode->depth] = guiNode;
	}

	// ---- Simple tree layout: depth on X, sibling index on Y ----
	std::vector<float> nextYPerDepth;
	std::function<void(const std::shared_ptr<NodeData>&, int)> layoutNode;
	layoutNode = [&](const std::shared_ptr<NodeData>& node, int depth)
		{
			if (!node) return;

			if (depth >= (int)nextYPerDepth.size())
				nextYPerDepth.resize(depth + 1, 0.0f);

			float x = depth * 250.0f;        // horizontal spacing between levels
			float y = nextYPerDepth[depth];  // current vertical position in this level

			node->editor_pos = ImVec2(x, y);
			node->position_initialized = false;  // force SetNodeEditorSpacePos next frame

			nextYPerDepth[depth] += 100.0f; // vertical spacing between siblings

			for (auto& child : node->child)
			{
				layoutNode(child, depth + 1);
			}
		};

	nextYPerDepth.clear();
	layoutNode(m_root, 0);

	// ---- Rebuild links for ImNodes ----
	m_nodeLinks.clear();
	link_id = 0;

	std::function<void(const std::shared_ptr<NodeData>&)> addLinks;
	addLinks = [&](const std::shared_ptr<NodeData>& node)
		{
			if (!node) return;

			for (auto& child : node->child)
			{
				if (!child) continue;

				m_nodeLinks.emplace_back(
					link_id++,
					node->output_attrib,
					child->input_attrib
				);

				addLinks(child);
			}
		};

	addLinks(m_root);
}

void AG::AGImGui::BHTEditor::GuiToTree(std::vector<AG::AI::NodeRep>& export_tree)
{
	m_exportVector.clear();
	SaveNodeToVector(m_root);
	//m_exportVector.erase(m_exportVector.begin()); // remove root
	export_tree = m_exportVector;
}

void AG::AGImGui::BHTEditor::SaveNodeToVector(std::weak_ptr<NodeData> node)
{
	auto node_shr = node.lock();
	if (!node_shr) return;

	AG::AI::NodeRep tempNode;
	tempNode.depth = node_shr->depth;
	tempNode.nodeType = node_shr->m_type;
	tempNode.name = node_shr->script_name;
	m_exportVector.push_back(tempNode);

	for (auto& child : node_shr->child)
	{
		SaveNodeToVector(child);
	}
}

void AG::AGImGui::BHTEditor::RenderNode(std::weak_ptr<NodeData> node)
{
	auto node_shr = node.lock();
	if (!node_shr) return;

	// Apply initial layout position once
	if (!node_shr->position_initialized)
	{
		ImNodes::SetNodeEditorSpacePos(node_shr->id, node_shr->editor_pos);
		node_shr->position_initialized = true;
	}

	ImNodes::BeginNode(node_shr->id);
	std::string hidden_id = "##" + std::to_string(node_shr->id);
	ImGui::Dummy(ImVec2(100.f, 5.f));
	if (node_shr->m_type != AI::NodeType::ROOT)
	{
		// for type combo
		{
			int index = 0;
			for (int i = 0; i < AI::node_string_vec.size(); i++)
			{
				if (node_shr->m_type == AI::node_string_vec[i].first)
				{
					index = i;
					break;
				}
			}
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::Combo(hidden_id.c_str(), &index, c_type.data(), (int)c_type.size()))
			{
				node_shr->m_type = AI::node_string_vec[index].first;
			}
		}

		// for script combo
		{
			int scriptIndex{};
			auto& allNodes = AI::TreeBuilder::GetInstance().getAllNode();
			std::vector<const char*> nodesName{ "" };
			for (auto& node_ : allNodes) {
				nodesName.emplace_back(node_.first.c_str());
			}
			for (int i{}; i < nodesName.size(); ++i) {
				if (node_shr->script_name == nodesName[i]) {
					scriptIndex = i;
					break;
				}
			}
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::Combo("##Nodes", &scriptIndex, nodesName.data(), (int)nodesName.size())) {
				node_shr->script_name = nodesName[scriptIndex];
			}
		}

		ImNodes::BeginInputAttribute(node_shr->input_attrib);
		ImGui::Text("Parent");
		ImNodes::EndInputAttribute();
	}
	else
	{
		ImGui::Text("ROOT");
	}

	if (node_shr->m_type != AI::NodeType::LEAF)
	{
		ImNodes::BeginOutputAttribute(node_shr->output_attrib);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 100.f - ImGui::CalcTextSize("Child").x);
		ImGui::Text("Child");
		ImNodes::EndOutputAttribute();
	}

	ImNodes::EndNode();
}

void AG::AGImGui::BHTEditor::Reset()
{
	node_id_tracker = 0;
	node_out_tracker = MAX_NODE;
	node_in_tracker = MAX_NODE * 2;
	link_id = 0;

	m_orphan_nodes.clear();
	m_nodeLinks.clear();

	m_root = std::make_shared<NodeData>(node_id_tracker++, node_out_tracker++, node_in_tracker++);
	m_root->m_type = AI::NodeType::ROOT;
	m_root->depth = 0;
	m_root->editor_pos = ImVec2(0.0f, 0.0f);
	m_root->position_initialized = false;
}

void AG::AGImGui::BHTEditor::RenderNodes(std::weak_ptr<NodeData> node)
{
	auto node_shr = node.lock();
	if (!node_shr) return;

	RenderNode(node);

	for (auto& child : node_shr->child)
	{
		RenderNodes(child);
	}
}

void AG::AGImGui::BHTEditor::RenderOrphans()
{
	for (auto& orphans : m_orphan_nodes)
	{
		RenderNode(orphans);

		for (auto& orphans_child : orphans->child)
		{
			RenderNode(orphans_child);
		}
	}
}

#pragma region PARENT/CHILD_MANAGEMENT

std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> AG::AGImGui::BHTEditor::CreateNode()
{
	std::shared_ptr<NodeData> newNode = std::make_shared<NodeData>(node_id_tracker++, node_out_tracker++, node_in_tracker++);
	m_orphan_nodes.push_back(newNode);
	return newNode;
}

std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> AG::AGImGui::BHTEditor::CreateChildNode()
{
	return std::shared_ptr<NodeData>();
}

void AG::AGImGui::BHTEditor::CaptureNodePositions()
{
	auto capture = [&](auto&& self, const std::shared_ptr<NodeData>& node) -> void
		{
			if (!node) return;

			node->editor_pos = ImNodes::GetNodeEditorSpacePos(node->id);

			for (auto& c : node->child)
			{
				self(self, c);
			}
		};

	// Tree nodes
	capture(capture, m_root);

	// Orphans
	for (auto& o : m_orphan_nodes)
	{
		if (o)
			o->editor_pos = ImNodes::GetNodeEditorSpacePos(o->id);
	}
}

void AG::AGImGui::BHTEditor::SortChildrenByY()
{
	auto sortRec = [&](auto&& self, const std::shared_ptr<NodeData>& node) -> void
		{
			if (!node) return;

			std::sort(node->child.begin(), node->child.end(),
				[](const std::shared_ptr<NodeData>& a,
					const std::shared_ptr<NodeData>& b)
				{
					if (a->editor_pos.y == b->editor_pos.y)
						return a->editor_pos.x < b->editor_pos.x;   // tie-break on X
					return a->editor_pos.y < b->editor_pos.y;       // main sort on Y
				});

			for (auto& c : node->child)
			{
				self(self, c);
			}
		};

	sortRec(sortRec, m_root);
}

std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> AG::AGImGui::BHTEditor::FindNodeByOutputAttrib(int attr)
{
	if (!m_root) return nullptr;

	// Search in tree (root + children)
	if (auto found = FindNodeByOutputAttribRecursive(m_root, attr))
		return found;

	// Search in orphans
	for (auto& orphan : m_orphan_nodes)
	{
		if (orphan && orphan->output_attrib == attr)
			return orphan;
	}

	return nullptr;
}

std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> AG::AGImGui::BHTEditor::FindNodeByInputAttrib(int attr)
{
	if (!m_root) return nullptr;

	// Search in tree
	if (auto found = FindNodeByInputAttribRecursive(m_root, attr))
		return found;

	// Search in orphans
	for (auto& orphan : m_orphan_nodes)
	{
		if (orphan && orphan->input_attrib == attr)
			return orphan;
	}

	return nullptr;
}

std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> AG::AGImGui::BHTEditor::FindNodeByOutputAttribRecursive(
	const std::shared_ptr<NodeData>& node, int attr)
{
	if (!node) return nullptr;

	if (node->output_attrib == attr)
		return node;

	for (auto& c : node->child)
	{
		if (auto found = FindNodeByOutputAttribRecursive(c, attr))
			return found;
	}

	return nullptr;
}

std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> AG::AGImGui::BHTEditor::FindNodeByInputAttribRecursive(
	const std::shared_ptr<NodeData>& node, int attr)
{
	if (!node) return nullptr;

	if (node->input_attrib == attr)
		return node;

	for (auto& c : node->child)
	{
		if (auto found = FindNodeByInputAttribRecursive(c, attr))
			return found;
	}

	return nullptr;
}

std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> AG::AGImGui::BHTEditor::FindParentOf(
	const std::shared_ptr<NodeData>& target,
	const std::shared_ptr<NodeData>& currentRoot)
{
	if (!target || !currentRoot) return nullptr;

	for (auto& c : currentRoot->child)
	{
		if (c == target)
			return currentRoot;

		if (auto p = FindParentOf(target, c))
			return p;
	}

	return nullptr;
}

void AG::AGImGui::BHTEditor::RemoveFromOrphans(const std::shared_ptr<NodeData>& node)
{
	auto it = std::remove(m_orphan_nodes.begin(), m_orphan_nodes.end(), node);
	m_orphan_nodes.erase(it, m_orphan_nodes.end());
}

void AG::AGImGui::BHTEditor::AddToOrphansIfNotPresent(const std::shared_ptr<NodeData>& node)
{
	if (!node) return;

	auto it = std::find(m_orphan_nodes.begin(), m_orphan_nodes.end(), node);
	if (it == m_orphan_nodes.end())
	{
		node->depth = 0; // reset depth if you want
		m_orphan_nodes.push_back(node);
	}
}

void AG::AGImGui::BHTEditor::AttachChild(
	const std::shared_ptr<NodeData>& parent,
	const std::shared_ptr<NodeData>& child)
{
	if (!parent || !child) return;

	// Avoid duplicate
	auto it = std::find(parent->child.begin(), parent->child.end(), child);
	if (it == parent->child.end())
	{
		child->depth = parent->depth + 1;
		parent->child.push_back(child);
	}
}

void AG::AGImGui::BHTEditor::DetachChild(
	const std::shared_ptr<NodeData>& parent,
	const std::shared_ptr<NodeData>& child)
{
	if (!parent || !child) return;

	auto& vec = parent->child;
	auto it = std::remove(vec.begin(), vec.end(), child);
	vec.erase(it, vec.end());
}


std::shared_ptr<AG::AGImGui::BHTEditor::NodeData>
AG::AGImGui::BHTEditor::FindNodeById(int nodeId)
{
	// DFS in tree (root + descendants)
	std::function<std::shared_ptr<NodeData>(const std::shared_ptr<NodeData>&)> dfsTree;
	dfsTree = [&](const std::shared_ptr<NodeData>& n) -> std::shared_ptr<NodeData>
		{
			if (!n) return nullptr;
			if (n->id == nodeId)
				return n;
			for (auto& c : n->child)
			{
				if (auto r = dfsTree(c)) return r;
			}
			return nullptr;
		};

	if (m_root)
	{
		if (auto r = dfsTree(m_root)) return r;
	}

	// Search orphans (and their subtrees)
	std::function<std::shared_ptr<NodeData>(const std::shared_ptr<NodeData>&)> dfsOrphan;
	dfsOrphan = [&](const std::shared_ptr<NodeData>& n) -> std::shared_ptr<NodeData>
		{
			if (!n) return nullptr;
			if (n->id == nodeId)
				return n;
			for (auto& c : n->child)
			{
				if (auto r = dfsOrphan(c)) return r;
			}
			return nullptr;
		};

	for (auto& o : m_orphan_nodes)
	{
		if (auto r = dfsOrphan(o)) return r;
	}

	return nullptr;
}

std::shared_ptr<AG::AGImGui::BHTEditor::NodeData>
AG::AGImGui::BHTEditor::FindParentAnywhere(const std::shared_ptr<NodeData>& target)
{
	if (!target) return nullptr;

	// Try tree rooted at m_root
	if (m_root && m_root != target)
	{
		if (auto p = FindParentOf(target, m_root))
			return p;
	}

	// Try each orphan root
	for (auto& o : m_orphan_nodes)
	{
		if (o && o != target)
		{
			if (auto p = FindParentOf(target, o))
				return p;
		}
	}

	return nullptr; // no parent → either root or orphan root
}


void AG::AGImGui::BHTEditor::DeleteNodeAndOrphanChildren(int nodeId)
{
	auto node = FindNodeById(nodeId);
	if (!node)
		return;

	// Don't allow deleting the special ROOT (optional safety)
	if (node == m_root)
	{
		// up to you: either ignore, or orphan its children + Reset()
		return;
	}

	// 1) Make all DIRECT children orphans (their subtrees remain attached to them)
	for (auto& child : node->child)
	{
		AddToOrphansIfNotPresent(child);
	}

	// 2) Detach this node from its parent, if any
	if (auto parent = FindParentAnywhere(node))
	{
		DetachChild(parent, node);
	}

	// 3) If this node itself is in the orphan list as a root, remove it
	RemoveFromOrphans(node);

	// 4) Remove all links that touch this node's pins
	const int outAttr = node->output_attrib;
	const int inAttr = node->input_attrib;

	m_nodeLinks.erase(
		std::remove_if(
			m_nodeLinks.begin(), m_nodeLinks.end(),
			[outAttr, inAttr](const NodeLink& l)
			{
				// Any link where this node was either the parent or the child
				return l.parent == outAttr || l.child == inAttr;
			}),
		m_nodeLinks.end()
	);

	// 5) Important: we DON'T touch node->child here.
	//    Once it's removed from its parent/orphan list, and we don't keep
	//    any shared_ptr to it, it'll be destroyed. The children stay alive
	//    via m_orphan_nodes (step 1).
}

#pragma endregion

void AG::AGImGui::BHTEditor::RenderHierarchyNode(const std::shared_ptr<NodeData>& node)
{
	if (!node) return;

	std::string label = "Unknown";
	for (auto& pair : AI::node_string_vec)
	{
		if (node->m_type == pair.first)
		{
			label = pair.second;
			break;
		}
	}

	label += "##hier_";
	label += std::to_string(node->id);

	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (node->child.empty())
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	bool open = ImGui::TreeNodeEx(label.c_str(), flags);

	// If leaf, no push happened because of NoTreePushOnOpen
	if (!node->child.empty() && open)
	{
		for (auto& c : node->child)
		{
			RenderHierarchyNode(c);
		}
		ImGui::TreePop();
	}
}