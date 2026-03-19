#include "pch.h"
#include "TextureAsset.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// --- NEW INCLUDES ---
#include <DirectXTex.h>
#include <wrl/client.h> // For CoInitializeEx
#include <fstream>      // For std::ifstream/ofstream
#include <vector>       // For std::vector
// --------------------


// Texture::Load is unchanged from your original
void TextureAsset::Load(std::filesystem::path file)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(false);

    stbi_uc* data = stbi_load(file.generic_string().c_str(), &size.first, &size.second, &channel, 0);
    if (!data)
    {
        std::cout << "Failed to load texture: " << file << std::endl;
        return;
    }
    GLenum format = (channel == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, size.first, size.second, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // --- FIX 2: Add this line ---
    stbi_set_flip_vertically_on_load(false); // Reset global state

    //AG_CORE_INFO("Loaded: {}", file.string());
}

// Unused functions
void TextureAsset::LoadEx(std::filesystem::path /*file*/,
     int /*_colorSpace*/,
     int /*_wrapMode*/,
    int /*_filterMode*/,
     bool /*_generateMipMap*/) {}
void TextureAsset::WriteToBinary( std::string /*new_file*/) {}
void TextureAsset::LoadFromBinary( std::filesystem::path /*path*/) {}

// --- NEW HELPER FUNCTION ---
// Helper struct and function to map DXGI formats to OpenGL formats
// You may need to expand this for other formats (BC1, BC4, BC5, etc.)
namespace {
    struct GLFormatInfo {
        GLenum internalFormat = 0;
        GLenum externalFormat = 0;
        GLenum type = 0;
        bool compressed = false;
    };

    GLFormatInfo GetGLFormatFromDXGI(DXGI_FORMAT format) {
        switch (format) {
            // --- BC1 (DXT1) --- (Your existing code)
        case DXGI_FORMAT_BC1_UNORM:
            return { GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0, true };
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            return { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0, 0, true };

            // --- BC3 (DXT5) --- (Your existing code)
        case DXGI_FORMAT_BC3_UNORM:
            return { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0, true };
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            return { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0, 0, true };

            // --- NEW: BC5 (ATI2/3Dc) ---
            // Used for Normal Maps (RG channels)
        case DXGI_FORMAT_BC5_UNORM:
            return { GL_COMPRESSED_RG_RGTC2, 0, 0, true };
        case DXGI_FORMAT_BC5_SNORM:
            return { GL_COMPRESSED_SIGNED_RG_RGTC2, 0, 0, true };

            // --- NEW: BC7 (BPTC) ---
            // High quality RGBA compression
        case DXGI_FORMAT_BC7_UNORM:
            return { GL_COMPRESSED_RGBA_BPTC_UNORM_ARB, 0, 0, true };
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return { GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB, 0, 0, true };

            // --- Uncompressed RGBA --- (Your existing code)
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, false };
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return { GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, false };

        default:
            return { 0, 0, 0, false }; // Unsupported
        }
    }
} // anonymous namespace


bool TextureAsset::ConvertToDDS(std::filesystem::path input,
    std::string output,
    int _colorSpace,
    int _wrapMode,
    int _filterMode,
    bool _generateMipMap,
    TextureCompressionFormat compression_Format, int maxTextureSizeIndex, bool generateNormalMap ,
    float normalMapStrength )
{
    (void)compression_Format;
    // Initialize COM for WIC (Windows Imaging Component)
    // This is necessary for LoadFromWICFile
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    // Ignore RPC_E_CHANGED_MODE; it just means COM was already inited with a different model.
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        AG_CORE_WARN("❌ CoInitializeEx failed: HRESULT={:X}", static_cast<unsigned int>(hr));
        return false;
    }

    DirectX::ScratchImage scratchImage;
    DirectX::TexMetadata metadata;

    try
    {
        // Before LoadFromWICFile
        DirectX::WIC_FLAGS wicFlags = (_colorSpace == 1)
            ? DirectX::WIC_FLAGS_FORCE_SRGB     // treat source as sRGB
            : DirectX::WIC_FLAGS_IGNORE_SRGB;   // treat source as linear

        // 1. Load Image (JPG, PNG, etc.) using WIC
        hr = DirectX::LoadFromWICFile(
            input.c_str(),
            wicFlags,
            &metadata,
            scratchImage
        );

        if (FAILED(hr))
        {
            // Fallback for formats WIC doesn't support (like .tga)
            // Try stb_image
            AG_CORE_WARN("⚠️ WIC failed (HRESULT={:X}). Trying stb_image...", static_cast<unsigned int>(hr));
            int w, h, ch;

            stbi_set_flip_vertically_on_load(false);

            unsigned char* data = stbi_load(input.string().c_str(), &w, &h, &ch, 4); // Force 4 channels
            if (!data)
            {
                AG_CORE_WARN("❌ WIC and stb_image failed to load: {}", input.string());
                CoUninitialize();
                return false;
            }
            hr = scratchImage.Initialize2D(
                DXGI_FORMAT_R8G8B8A8_UNORM, // Use standard 8-bit RGBA
                w, h, 1, 1
            );
            if (FAILED(hr)) {
                AG_CORE_WARN("❌ Failed to initialize ScratchImage from stb_data");
                stbi_image_free(data);
                CoUninitialize();
                return false;
            }
            memcpy(scratchImage.GetPixels(), data, w * h * 4);
            stbi_image_free(data);
            metadata = scratchImage.GetMetadata(); // Get metadata from the new image
        }

        // --- START: ADDED RESIZE LOGIC ---

        // Map index to actual size (0: Original, 1: 1024, 2: 2048, 3: 4096)
        int actualMaxSize = 0;
        if (maxTextureSizeIndex == 1) actualMaxSize = 1024;
        else if (maxTextureSizeIndex == 2) actualMaxSize = 2048;
        else if (maxTextureSizeIndex == 3) actualMaxSize = 4096;

        // Check if resize is needed
        if (actualMaxSize > 0 && (metadata.width > actualMaxSize || metadata.height > actualMaxSize))
        {
            /*AG_CORE_WARN("ℹ️ Resizing texture from {}x{} to max {}}px", metadata.width, metadata.height, actualMaxSize);*/

            // Remove the extra '}'
            AG_CORE_WARN("ℹ️ Resizing texture from {}x{} to max {}px", metadata.width, metadata.height, actualMaxSize);


            // Calculate new dimensions while maintaining aspect ratio
            size_t newWidth, newHeight;
            if (metadata.width > metadata.height) {
                newWidth = actualMaxSize;
                newHeight = static_cast<size_t>(static_cast<float>(actualMaxSize) / metadata.width * metadata.height);
            }
            else {
                newHeight = actualMaxSize;
                newWidth = static_cast<size_t>(static_cast<float>(actualMaxSize) / metadata.height * metadata.width);
            }

            DirectX::ScratchImage resizedImage; // Temporary holder
            hr = DirectX::Resize(
                *scratchImage.GetImage(0, 0, 0), // Get the base image
                newWidth, newHeight,
                DirectX::TEX_FILTER_DEFAULT, // You could link this to _filterMode
                resizedImage
            );

            if (SUCCEEDED(hr)) {
                // Success: The resized image is now our new source
                scratchImage = std::move(resizedImage);
                metadata = scratchImage.GetMetadata(); // CRITICAL: Update metadata
            }
            else {
                AG_CORE_WARN("❌ Failed to resize image. Continuing with original.");
            }
        }
        // --- END: ADDED RESIZE LOGIC ---


        // --- START: FLIP IMAGE BEFORE PROCESSING ---
        // We flip here so the saved DDS contains Bottom-Up data (GL Style).
        // Note: This causes the file to look upside-down in Windows Explorer, 
        // but it will render correctly in OpenGL without UV hacks.
        DirectX::ScratchImage flippedSource;
        hr = DirectX::FlipRotate(
            *scratchImage.GetImage(0, 0, 0),
            DirectX::TEX_FR_FLIP_VERTICAL,
            flippedSource
        );

        if (SUCCEEDED(hr))
        {
            scratchImage = std::move(flippedSource);
            metadata = scratchImage.GetMetadata();
            AG_CORE_WARN("ℹ️ Image flipped vertically for OpenGL compliance.");
        }
        else
        {
            AG_CORE_WARN("❌ Failed to flip image source. Texture may appear inverted.");
        }
        // --- END: FLIP IMAGE BEFORE PROCESSING ---


        // --- START: ADDED NORMAL MAP GENERATION ---
        if (generateNormalMap)
        {
            AG_CORE_WARN("ℹ️ Generating Normal Map from Grayscale (Strength: {})", normalMapStrength);

            DirectX::ScratchImage normalMapImage;
            hr = DirectX::ComputeNormalMap(
                *scratchImage.GetImage(0, 0, 0),
                DirectX::CNMAP_CHANNEL_LUMINANCE, // Use brightness of image as height
                normalMapStrength,
                DXGI_FORMAT_R8G8B8A8_UNORM, // Output format
                normalMapImage
            );

            if (SUCCEEDED(hr))
            {
                // Overwrite the original image with our new Normal Map
                scratchImage = std::move(normalMapImage);
                metadata = scratchImage.GetMetadata(); // Update metadata
            }
            else
            {
                AG_CORE_WARN("❌ Failed to generate Normal Map.");
            }
        }
        // --- END: ADDED NORMAL MAP GENERATION ---


        DirectX::ScratchImage mipmappedImage;
        if (_generateMipMap)
        {
            // Use TEX_FILTER_FLAGS, not DWORD
            DirectX::TEX_FILTER_FLAGS mipFlags = DirectX::TEX_FILTER_DEFAULT;

            // sRGB-aware downsampling so colors average correctly when _colorSpace == sRGB
            if (_colorSpace == 1) mipFlags = static_cast<DirectX::TEX_FILTER_FLAGS>(mipFlags | DirectX::TEX_FILTER_SRGB);

            // Tie your UI’s "nearest/linear" to the mip kernel used to build the chain
            mipFlags = static_cast<DirectX::TEX_FILTER_FLAGS>(mipFlags |
                (_filterMode == 0 ? DirectX::TEX_FILTER_POINT : DirectX::TEX_FILTER_LINEAR));

            // Now PASS mipFlags here (this was the missing piece)
            hr = DirectX::GenerateMipMaps(
                *scratchImage.GetImage(0, 0, 0),
                mipFlags,
                0,  // all levels
                mipmappedImage
            );
            if (FAILED(hr))
            {
                AG_CORE_WARN("❌ Failed to generate mipmaps: {}", input.string());
                CoUninitialize();
                return false;
            }
        }
        else
        {
            mipmappedImage = std::move(scratchImage);
        }

        bool hasAlpha = DirectX::HasAlpha(mipmappedImage.GetMetadata().format)
            || /* your own heuristic: scan alpha or carry UI flag */ false;

		(void)hasAlpha; // Suppress unused variable warning if not used

        DXGI_FORMAT targetFormat =
            (_colorSpace == 1)
            ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB  // sRGB, 8-bits per channel
            : DXGI_FORMAT_R8G8B8A8_UNORM;     // Linear, 8-bits per channel

        AG_CORE_WARN("ℹ️ Converting to uncompressed {}...",
            (_colorSpace == 1) ? "R8G8B8A8_SRGB" : "R8G8B8A8_LINEAR");

        DirectX::ScratchImage finalImage; // We'll store the result here

        

        // 3. Determine Target Format based on user choice
        switch (compressionFormat)
        {
        case TextureCompressionFormat::BC1:
            targetFormat = (_colorSpace == 1) ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
            break;
        case TextureCompressionFormat::BC3:
            targetFormat = (_colorSpace == 1) ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;
            break;
        case TextureCompressionFormat::BC5:
            targetFormat = DXGI_FORMAT_BC5_UNORM; // BC5 is typically for linear data (like normal maps)
            if (_colorSpace == 1) {
                AG_CORE_WARN("⚠️ sRGB specified for BC5. BC5 is usually for linear data (normal maps). Compressing as linear.");
            }
            break;
        case TextureCompressionFormat::BC7:
            targetFormat = (_colorSpace == 1) ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
            break;
        case TextureCompressionFormat::None:
        default:
            targetFormat = (_colorSpace == 1) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        }

        // 4. Compress or Convert
        if (compressionFormat == TextureCompressionFormat::None)
        {
            //AG_CORE_WARN("ℹ️ Converting to uncompressed {}...",
            //    (_colorSpace == 1) ? "R8G8B8A8_SRGB" : "R8G8B8A8_LINEAR");

            //// This is your original code path
            //hr = DirectX::Convert(
            //    mipmappedImage.GetImages(),
            //    mipmappedImage.GetImageCount(),
            //    mipmappedImage.GetMetadata(),
            //    targetFormat,
            //    DirectX::TEX_FILTER_DEFAULT,
            //    DirectX::TEX_THRESHOLD_DEFAULT,
            //    finalImage
            //);

            AG_CORE_WARN("ℹ️ Processing uncompressed {}...",
                (_colorSpace == 1) ? "R8G8B8A8_SRGB" : "R8G8B8A8_LINEAR");

            // --- FIX START: Optimization & Safety ---
            // Check if the image is already in the target format.
            if (mipmappedImage.GetMetadata().format == targetFormat)
            {
                AG_CORE_WARN("ℹ️ Format matches target. Skipping unnecessary Convert.");
                // Just move the data over. Zero allocation, zero processing time.
                finalImage = std::move(mipmappedImage);
                hr = S_OK;
            }
            else
            {
                // Only convert if strictly necessary (e.g. BGR -> RGB)
                hr = DirectX::Convert(
                    mipmappedImage.GetImages(),
                    mipmappedImage.GetImageCount(),
                    mipmappedImage.GetMetadata(),
                    targetFormat,
                    DirectX::TEX_FILTER_DEFAULT,
                    DirectX::TEX_THRESHOLD_DEFAULT,
                    finalImage
                );
            }
            // --- FIX END ---

        }
        else
        {
            /*AG_CORE_WARN("ℹ️ Compressing to target format: {} (DXGI_FORMAT={})",
                magic_enum::enum_name(compressionFormat), static_cast<int>(targetFormat));*/
            AG_CORE_WARN("ℹ️ Compressing to format (Enum={}) (DXGI_FORMAT={})",
                static_cast<int>(compressionFormat), static_cast<int>(targetFormat));
            DirectX::TEX_COMPRESS_FLAGS compressFlags = DirectX::TEX_COMPRESS_DEFAULT | DirectX::TEX_COMPRESS_PARALLEL;

            // IMPORTANT: If the *source* is sRGB, we must tell the compressor
            // so it performs compression in the correct color space.
            if (_colorSpace == 1)
            {
                compressFlags = static_cast<DirectX::TEX_COMPRESS_FLAGS>(compressFlags | DirectX::TEX_COMPRESS_SRGB_IN);
            }

            hr = DirectX::Compress(
                mipmappedImage.GetImages(),
                mipmappedImage.GetImageCount(),
                mipmappedImage.GetMetadata(),
                targetFormat,
                compressFlags,
                DirectX::TEX_THRESHOLD_DEFAULT, // Alpha threshold for BC1
                finalImage
            );
        }

        if (FAILED(hr))
        {
            AG_CORE_WARN("❌ Failed to convert/compress texture: {}", input.string());
            CoUninitialize();
            return false;
        }
       

        // 5. Save to DDS file
    // We now save the 'finalImage'
        hr = DirectX::SaveToDDSFile(
            finalImage.GetImages(),
            finalImage.GetImageCount(),
            finalImage.GetMetadata(),
            DirectX::DDS_FLAGS_NONE,
            std::filesystem::path(output).c_str()
        );
        if (FAILED(hr))
        {
            AG_CORE_WARN("❌ Failed to save DDS: {}", output);
            CoUninitialize();
            return false;
        }

        // 6. Append custom footer (same logic as your original)
        DDSMetadataFooter footer{};
        footer.magic = 0xDEADBEEF;
        footer.colorSpace = _colorSpace;
        footer.wrapMode = _wrapMode;
        footer.filterMode = _filterMode;
        footer.generateMipMap = _generateMipMap;
        footer.channel = (compressionFormat == TextureCompressionFormat::BC5) ? 2 : 4;

        footer.compressionFormat = compressionFormat;

        // --- KEY CHANGE: Get width/height from 'finalImage' ---
        footer.width = static_cast<int>(finalImage.GetMetadata().width);
        footer.height = static_cast<int>(finalImage.GetMetadata().height);

        uint32_t footerPos = static_cast<uint32_t>(std::filesystem::file_size(output));
        std::ofstream ofs(output, std::ios::binary | std::ios::app);
        if (!ofs)
        {
            AG_CORE_WARN("❌ Could not append footer: {}", output);
            CoUninitialize();
            return false;
        }

        // Write footer, then write the position of the footer
        ofs.write(reinterpret_cast<const char*>(&footer), sizeof(footer));
        ofs.write(reinterpret_cast<const char*>(&footerPos), sizeof(uint32_t));
        ofs.close();

        AG_CORE_WARN("✅ DDS saved (DXTex, BC3, embedded footer): {} ({}×{})",
            output, footer.width, footer.height);

        CoUninitialize();
        return true;
    }
    catch (const std::exception& e)
    {
        AG_CORE_WARN("❌ ConvertToDDS (DXTex) Exception: {}", e.what());
        CoUninitialize();
        return false;
    }
}

// --- UPDATED LoadFromDDS ---
bool TextureAsset::LoadFromDDS(std::filesystem::path input)
{
    try
    {
        //double start = glfwGetTime();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (!std::filesystem::exists(input))
        {
            AG_CORE_WARN("❌ File not found: {}", input.string());
            return false;
        }

        // 1. Locate footer (SAME LOGIC AS BEFORE, IT'S GOOD)
        DDSMetadataFooter footer{};
        bool hasFooter = false;
        size_t fileSize = std::filesystem::file_size(input);
        size_t ddsPayloadSize = fileSize; // Default to full size

        if (fileSize > sizeof(DDSMetadataFooter) + sizeof(uint32_t))
        {
            std::ifstream ifs_footer(input, std::ios::binary);
            ifs_footer.seekg(-static_cast<int>(sizeof(uint32_t)), std::ios::end);
            uint32_t footerPos = 0;
            ifs_footer.read(reinterpret_cast<char*>(&footerPos), sizeof(uint32_t));

            // Check if footer position is sane
            if (footerPos > 0 && (footerPos + sizeof(DDSMetadataFooter) + sizeof(uint32_t)) == fileSize)
            {
                ifs_footer.seekg(footerPos, std::ios::beg);
                ifs_footer.read(reinterpret_cast<char*>(&footer), sizeof(DDSMetadataFooter));
                hasFooter = (footer.magic == 0xDEADBEEF);
                if (hasFooter) {
                    ddsPayloadSize = footerPos; // The DDS data ends where the footer begins
                }
            }
            ifs_footer.close();
        }

        if (!hasFooter)
        {
            //AG_CORE_WARN("⚠️ No custom footer found in {}. Using defaults.", input.string());
            // Set defaults based on lead's message
            footer = {};
            footer.colorSpace = 0; // Linear
            footer.wrapMode = 0;   // Repeat
            footer.filterMode = 0; // Nearest
            footer.generateMipMap = true; // Assume mips were generated

            footer.compressionFormat = TextureCompressionFormat::None;

        }

        // 2. Load DDS payload into memory
        // We must load from memory because the file contains our custom footer,
        // which makes LoadFromDDSFile fail.
        std::vector<uint8_t> ddsData(ddsPayloadSize);
        std::ifstream ifs(input, std::ios::binary);
        if (!ifs) {
            AG_CORE_WARN("❌ Failed to open file for reading: {}", input.string());
            return false;
        }
        ifs.read(reinterpret_cast<char*>(ddsData.data()), ddsPayloadSize);
        ifs.close();

        // 3. Load DDS from memory using DirectXTex
        DirectX::TexMetadata metadata;
        DirectX::ScratchImage scratchImage;
        HRESULT hr = DirectX::LoadFromDDSMemory(
            ddsData.data(),
            ddsData.size(),
            DirectX::DDS_FLAGS_NONE,
            &metadata,
            scratchImage
        );

        if (FAILED(hr))
        {
            //AG_CORE_WARN("❌ DirectXTex::LoadFromDDSMemory failed: {}", input.string());
            return false;
        }

        //// Flip for OpenGL here
        //DirectX::ScratchImage flipped;
        //hr = DirectX::FlipRotate(
        //    scratchImage.GetImages(),
        //    scratchImage.GetImageCount(),
        //    metadata,
        //    DirectX::TEX_FR_FLIP_VERTICAL,
        //    flipped
        //);
        //if (SUCCEEDED(hr)) {
        //    scratchImage = std::move(flipped);
        //    metadata = scratchImage.GetMetadata();
        //}


        // 4. Translate DXGI format to OpenGL format
        GLFormatInfo glFormat = GetGLFormatFromDXGI(metadata.format);
        if (glFormat.internalFormat == 0)
        {
            //AG_CORE_WARN("❌ Unsupported DXGI format in DDS: {} (DXGI_FORMAT={})",
            //    input.string(), static_cast<int>(metadata.format));
            return false;
        }


        // --- START: ADD THIS BLOCK ---
// This will hold our flipped image IF we are on the uncompressed path
        DirectX::ScratchImage flippedImage;

        if (!glFormat.compressed)
        {
            // UNCOMPRESSED path (R8G8B8A8, etc.)
            // This is the one that's flipped. We must flip it "back" to bottom-up,
            // which is what 'glTexImage2D' expects.
            HRESULT flip_hr = DirectX::FlipRotate(
                scratchImage.GetImages(),
                scratchImage.GetImageCount(),
                metadata,
                DirectX::TEX_FR_FLIP_VERTICAL,
                flippedImage // Store the result here
            );

            if (FAILED(flip_hr))
            {
                //AG_CORE_WARN("❌ Failed to flip uncompressed DDS for OpenGL");
                // The image will still load, but be flipped.
            }
        }
        // For COMPRESSED path, we do nothing. 
        // 'scratchImage' (top-down) is exactly what 'glCompressedTexImage2D' wants.
        // --- END: ADD THIS BLOCK ---



        // 5. Upload to GPU
        GLuint texID = 0;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        for (size_t level = 0; level < metadata.mipLevels; ++level)
        {
            // Use the image directly from scratchImage (no flippedImage logic needed)
            const DirectX::Image* img = scratchImage.GetImage(level, 0, 0);

            if (!img) {
                //AG_CORE_WARN("❌ Failed to get image for mip level {}", (int)level);
                continue;
            }

            if (glFormat.compressed)
            {
                GLsizei imageSize = static_cast<GLsizei>(img->slicePitch);
                glCompressedTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level),
                    glFormat.internalFormat,
                    static_cast<GLsizei>(img->width),
                    static_cast<GLsizei>(img->height),
                    0, imageSize, img->pixels);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level),
                    glFormat.internalFormat,
                    static_cast<GLsizei>(img->width),
                    static_cast<GLsizei>(img->height),
                    0,
                    glFormat.externalFormat, glFormat.type, img->pixels);
            }

           // GLenum err = glGetError();
            //if (err != GL_NO_ERROR)
            //    AG_CORE_WARN("❌ gl[Compressed]TexImage2D(level {}): 0x{:X}", (int)level, err);
        }

        // 6. Generate Mipmaps IF requested and not already present
        //    We must do this *after* uploading mips, but *before* setting filters.
        size_t finalMipCount = metadata.mipLevels; // Start with what the file had

        if (footer.generateMipMap && metadata.mipLevels == 1)
        {
            // Fix the typo here!
            glGenerateMipmap(GL_TEXTURE_2D);

            // Now that we've generated them, we need to find out how many we have
            // so the filter logic below is correct. We can calculate it.
            GLint maxDim = std::max(static_cast<GLint>(metadata.width), static_cast<GLint>(metadata.height));
            finalMipCount = 1 + static_cast<size_t>(std::floor(std::log2(maxDim)));

            //AG_CORE_WARN("ℹ️ Generated {} mipmaps for {}", finalMipCount, input.string());
        }


        // 7. Texture parameters (using footer data)
        //    NOW we set the filters, using the *final* mip count.

        // wrapMode: 0 = Repeat, 1 = Clamp, 2 = Mirror
        GLenum wrap = footer.wrapMode == 1 ? GL_CLAMP_TO_EDGE :
            footer.wrapMode == 2 ? GL_MIRRORED_REPEAT : GL_REPEAT;

        // filterMode: 0 = Nearest, 1 = Linear
        GLenum magFilter = footer.filterMode == 0 ? GL_NEAREST : GL_LINEAR;
        GLenum minFilter;

        // --- START DEBUG STATEMENTS ---
        //AG_CORE_WARN("🐞 DEBUG: Checking filter logic...");
        //AG_CORE_WARN("   > finalMipCount: {}", finalMipCount);
        //AG_CORE_WARN("   > footer.filterMode: {}", footer.filterMode);
        // --- END DEBUG STATEMENTS ---

        // Apply mipmap filtering only if mipmaps exist (finalMipCount > 1)
        if (finalMipCount > 1)
        {
            minFilter = (footer.filterMode == 0) ?
                GL_NEAREST_MIPMAP_NEAREST : // Nearest filter, nearest mip
                GL_LINEAR_MIPMAP_LINEAR;   // Linear filter, linear mip (trilinear)

            // --- START DEBUG STATEMENTS ---
            if (minFilter == GL_LINEAR_MIPMAP_LINEAR) {
                //AG_CORE_WARN("   > ✅ Mips exist. Selected BLURRY (GL_LINEAR_MIPMAP_LINEAR).");
            }
            else {
                //AG_CORE_WARN("   > ⚠️ Mips exist. Selected PIXELATED (GL_NEAREST_MIPMAP_NEAREST).");
            }

        }
        else
        {
            minFilter = (footer.filterMode == 0) ? GL_NEAREST : GL_LINEAR;

            // --- START DEBUG STATEMENTS ---
            //AG_CORE_WARN("   > ❌ NO Mips. (finalMipCount is 1). Falling back to non-mip filter.");
            // --- END DEBUG STATEMENTS ---
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

        // Tell GL to use the full mip chain
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint)finalMipCount - 1);

        // We do NOT call glGenerateMipmap here. 
        // The DDS is a pre-processed asset; it either has mips or it doesn't.
        glBindTexture(GL_TEXTURE_2D, 0);

        // 8. Verify upload
        GLint w = 0, h = 0;
        glBindTexture(GL_TEXTURE_2D, texID);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        glBindTexture(GL_TEXTURE_2D, 0);

        //AG_CORE_WARN("🧪 GL upload verified: {}×{}", w, h);
        if (w == 0 || h == 0)
        {
            AG_CORE_WARN("❌ GPU upload failed (blank): {}", input.string());
            glDeleteTextures(1, &texID); // Clean up failed texture
            return false;
        }

        // 9. Store metadata
        this->textureID = texID;
        this->size = { static_cast<int>(metadata.width), static_cast<int>(metadata.height) };
        this->channel = (metadata.format == DXGI_FORMAT_BC5_UNORM || metadata.format == DXGI_FORMAT_BC5_SNORM) ? 2 : 4;
        this->colorSpace = footer.colorSpace;
        this->wrapMode = footer.wrapMode;
        this->filterMode = footer.filterMode;
        this->generateMipMap = footer.generateMipMap;

        this->compressionFormat = footer.compressionFormat;

        //AG_CORE_WARN("✅ DDS Loaded (DXTex): {} | {}×{} | Mips:{} | sRGB:{} | Wrap:{} | Filter:{} | {:.3f}s",
        //    input.string(), metadata.width, metadata.height, finalMipCount,
        //    footer.colorSpace, footer.wrapMode, footer.filterMode,
        //    glfwGetTime() - start);

        return true;
    }
    catch (const std::exception& e)
    {
        AG_CORE_WARN("❌ LoadFromDDS (DXTex) Exception: {}", e.what());
        return false;
    }
}


// --- Unload is unchanged ---
void TextureAsset::Unload()
{
    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}