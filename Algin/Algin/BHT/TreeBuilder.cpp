#include "pch.h"
#include "TreeBuilder.h"

namespace AG {
    namespace AI {
        void TreeBuilder::init()
        {
            //load all the node into list
            loadAllnode();
        }
        void TreeBuilder::reload()
        {
            saveNodeList();
            loadAllnode();
        }
        void TreeBuilder::terminate()
        {
            //save the node list into a file (node.def)
            saveNodeList();
        }
        const std::filesystem::path& TreeBuilder::getCurrentTreePath() const
        {
            return currentTreePath;
        }
        void TreeBuilder::setCurrentTreePath(const std::filesystem::path newPath)
        {
            currentTreePath = newPath;
        }
        const std::unordered_map<std::string, NodeType>& TreeBuilder::getAllNode()
        {
            return nameToType;
        }
        std::vector<NodeRep> TreeBuilder::getTree(std::filesystem::path treePath)
        {
            //Deserialize the .bht file
            BehaviorTreeTemplate newTemplate{};
            SERIALIZATIONSYSTEM.loadBHT(treePath, newTemplate);
            newTemplate.setTemplatePath(treePath);
            
            return newTemplate.getNodesVector();
        }
        void TreeBuilder::buildTree(std::vector<NodeRep>&& editorTreeRep, const std::filesystem::path newPath)
        {
            //Create new template and save to the assets folder
            BehaviorTreeTemplate newTemplate{};
            newTemplate.setTemplateID(std::move(Data::GenerateGUID()));
            newTemplate.setTemplatePath(newPath);
            newTemplate.setNodesVector(std::move(editorTreeRep));
            SERIALIZATIONSYSTEM.saveBHT(newPath, newTemplate);
        }
        void TreeBuilder::loadAllnode()
        {
            //Get all the node from ScriptEngine
            std::vector<const char*> allNodeFromCSharp{};
            SCRIPTENGINE.getNodeClassName(allNodeFromCSharp);
            std::unordered_set<std::string> csharpNodes;
            csharpNodes.reserve(allNodeFromCSharp.size());
            for (auto* name : allNodeFromCSharp) {
                csharpNodes.insert(name);
            }

            //deserizlize the list. Sort them into the three types of nodes
            SERIALIZATIONSYSTEM.loadNodeList(nameToType);

            // Remove any node from nameToType that is NOT in C# ScriptEngine
            for (auto it = nameToType.begin(); it != nameToType.end(); ) {
                if (csharpNodes.count(it->first) == 0) {
                    // Node does not exist in C# -> remove it
                    it = nameToType.erase(it);
                }
                else {
                    ++it;
                }
            }

            //Add the new unsorted node from C# engine to the nameToType map as Unknown type
            for (const auto& name : csharpNodes) {
                if (nameToType.find(name) == nameToType.end()) {
                    // Insert new node as UNKNOWN type
                    nameToType[name] = AI::NodeType::UNKNOWN;

                    AG_CORE_INFO("New behavior node detected: {} (assigned as Unknown type)", name);
                }
            }
            
        }
        void TreeBuilder::saveNodeList()
        {
            //Serialize the list into NodeList.json in editor asset
            SERIALIZATIONSYSTEM.saveNodeList(nameToType);

        }
    }
}

