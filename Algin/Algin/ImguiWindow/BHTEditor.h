#pragma once
#include "../Header/pch.h"
#define MAX_NODE 500

namespace AG {
	namespace AGImGui {

		class BHTEditor : public AGImGui::IImguiWindow {
		public:
			BHTEditor() { Reset(); }
			~BHTEditor() override {
				// Clear out all shared pointers explicitly to prevent memory leaks on shutdown
				m_root.reset();
				m_orphan_nodes.clear();
				m_nodeLinks.clear();
				m_exportVector.clear();
				t_exportVector.clear();
			}
			void Render() override;

		public:

			/*
			struct NodeRep {
				NodeType nodeType;
				int depth;
				std::string name;
				};
			*/

			struct NodeData
			{
				AI::NodeType m_type = AI::NodeType::UNKNOWN;
				int id;
				int output_attrib;
				int input_attrib;
				int depth;
				std::string script_name;

				std::vector<std::shared_ptr<NodeData>> child;

				ImVec2 editor_pos = ImVec2(0.0f, 0.0f);
				bool   position_initialized = false;
				NodeData() :
					id(0),
					output_attrib(0),
					input_attrib(0),
					depth(0){
				}
				NodeData(int identifier, int out_attrb, int in_attrib) :
					id(identifier),
					output_attrib(out_attrb),
					input_attrib(in_attrib),
					depth(0){
				}
			};

			struct NodeLink
			{
				int id;
				int parent;
				int child;

				NodeLink(int identifier, int p, int c) :
					id(identifier),
					parent(p),
					child(c) {
				}
			};

			static void Enable() { enable_editor = true; }

		private:

			inline static bool enable_editor = false;

			std::shared_ptr<NodeData>		m_root = std::make_shared<NodeData>(node_id_tracker++, node_out_tracker++, node_in_tracker++);
			std::vector<std::shared_ptr<NodeData>> m_orphan_nodes;
			std::vector<NodeLink>			m_nodeLinks;

			// identifier tracker
			int node_id_tracker = 0;
			int node_out_tracker = MAX_NODE;
			int node_in_tracker = MAX_NODE * 2;
			int link_id = 0;


			// combo
			std::vector<std::string> s_type;
			std::vector<const char*> c_type;

			// read/saver
			std::vector<AG::AI::NodeRep> m_exportVector;
			std::vector<AG::AI::NodeRep> t_exportVector;

			void TreeToGUI(const std::vector<AG::AI::NodeRep>& new_tree);
			void AddNodeFromVector(int index);
			void GuiToTree(std::vector<AG::AI::NodeRep>& export_tree);
			void SaveNodeToVector(std::weak_ptr<NodeData> node);
			void Reset();
			void RenderNode(std::weak_ptr<NodeData> node);
			void RenderNodes(std::weak_ptr<NodeData> node);
			void RenderOrphans();
			std::shared_ptr<NodeData> CreateNode();
			std::shared_ptr<NodeData> CreateChildNode();

			void CaptureNodePositions();
			void SortChildrenByY();

			/* Parent Child Management */
			std::shared_ptr<NodeData> FindNodeByOutputAttrib(int attr);
			std::shared_ptr<NodeData> FindNodeByInputAttrib(int attr);

			std::shared_ptr<NodeData> FindNodeByOutputAttribRecursive(const std::shared_ptr<NodeData>& node, int attr);
			std::shared_ptr<NodeData> FindNodeByInputAttribRecursive(const std::shared_ptr<NodeData>& node, int attr);

			std::shared_ptr<NodeData> FindParentOf(const std::shared_ptr<NodeData>& target, const std::shared_ptr<NodeData>& currentRoot);

			void RemoveFromOrphans(const std::shared_ptr<NodeData>& node);
			void AddToOrphansIfNotPresent(const std::shared_ptr<NodeData>& node);

			void AttachChild(const std::shared_ptr<NodeData>& parent, const std::shared_ptr<NodeData>& child);
			void DetachChild(const std::shared_ptr<NodeData>& parent, const std::shared_ptr<NodeData>& child);
			std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> FindNodeById(int nodeId);
			std::shared_ptr<AG::AGImGui::BHTEditor::NodeData> FindParentAnywhere(const std::shared_ptr<NodeData>& target);
			void DeleteNodeAndOrphanChildren(int nodeId);
			void RenderHierarchyNode(const std::shared_ptr<NodeData>& node);
		};
	}
}