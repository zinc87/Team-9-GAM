#pragma once
#include "pch.h"

namespace AG {
	namespace AI {
		enum class NodeType {
			UNKNOWN,
			CONTROL_FLOW,
			DECORATOR,
			LEAF,
			ROOT
		};

		static std::vector<std::pair<NodeType, std::string>> node_string_vec{
			{NodeType::UNKNOWN, "Pick a Node Type"},
			{NodeType::CONTROL_FLOW, "Control Flow"},
			{NodeType::DECORATOR, "Decorator"},
			{NodeType::LEAF, "Leaf"}

		};

		//The represent struct used in the editor, can change to whatever format neccessary
		struct NodeRep {
			NodeType nodeType;
			int depth;
			std::string name;
			
		};

		//Used for interface to edit/ build tree template.
		class TreeBuilder : public Pattern::ISingleton<TreeBuilder> {
		public :
			void init();
			void reload();
			void terminate();

			const std::filesystem::path& getCurrentTreePath() const;
			void setCurrentTreePath(const std::filesystem::path newPath);

			const std::unordered_map<std::string, NodeType>& getAllNode();

			std::vector<NodeRep> getTree(std::filesystem::path treePath);
			void buildTree(std::vector<NodeRep>&& editorTreeRep, const std::filesystem::path newPath);
		private:
			void loadAllnode();
			void saveNodeList();
		private:
			std::unordered_map<NodeType, std::vector<std::string>> typeToNameVector; //This is not updated, dont use it.
			std::unordered_map<std::string, NodeType> nameToType;

			std::filesystem::path currentTreePath;
		};
	}
}