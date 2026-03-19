#include "pch.h"
#include "IAsset.h"

bool CompressBuffer(const std::vector<unsigned char>& input, std::vector<unsigned char>& output)
{
	uLongf outSize = compressBound(static_cast<uLong>(input.size()));
	output.resize(outSize);

	int res = compress(
		reinterpret_cast<Bytef*>(output.data()),
		&outSize, // <-- FIX: pass pointer, not cast
		reinterpret_cast<const Bytef*>(input.data()),
		static_cast<uLong>(input.size())
	);

	if (res != Z_OK)
		return false;

	output.resize(outSize); // shrink to actual size
	return true;
}

// ------------------------
// Decompress helper
// ------------------------
bool DecompressBuffer(const std::vector<unsigned char>& input, std::vector<unsigned char>& output, size_t originalSize)
{
	output.resize(originalSize);
	uLongf outSize = static_cast<uLongf>(originalSize);

	int res = uncompress(
		reinterpret_cast<Bytef*>(output.data()),
		&outSize, // <-- FIX: pass pointer, not cast
		reinterpret_cast<const Bytef*>(input.data()),
		static_cast<uLong>(input.size())
	);

	return (res == Z_OK);
}