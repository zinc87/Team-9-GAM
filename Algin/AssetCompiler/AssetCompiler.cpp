// AssetCompiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "AssetCommon.h"


/*

enum ASSET_TYPE
{
	UNKNOWN,
	TEXTURE,
	MODEL,
	AUDIO,
	SCRIPT,
	AGCUBEMAP,
	FONT,
	DDS,
	AGSKINNEDMESH,
	AGTEXTURE,
	AGSTATICMESH,
	AGDYNAMICMESH,
	AGAUDIO,
	AGFONT,
};

*/
std::vector<int> params;
int main(int argc, char* argv[])
{
	size_t idx = 0;
	int type = std::stoi(std::string(argv[1]), &idx, 10);
	std::string input = argv[2];
	std::string output = argv[3];
	for (int i = 4; i < argc; i++)
	{
		idx = 0;
		int param = std::stoi(std::string(argv[i]), &idx, 10);
		params.push_back(param);
	}
	/* Example Tester */

	std::erase(input, '"');
	std::erase(output, '"');

	std::cout << "Type: " << type << std::endl;
	std::cout << "Input: " << input << std::endl;
	std::cout << "Output: " << output << std::endl;
	
	for (int i = 0; i < params.size(); i++)
		std::cout << "Param " << i << ": " << params[i] << std::endl;
	/* Example Tester */
	switch (type)
	{
	case 1 : // static mesh
	{
		StaticMeshAsset temp;
		temp.Load(std::filesystem::path(input));
		temp.WriteToBinary(output);
		break;
	}
	case 2 :
	{
		break;
	}
	}
    return 0;
}