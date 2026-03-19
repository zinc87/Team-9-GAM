#include "pch.h"
#include "FontAsset.h"



void FontAsset::Load(std::filesystem::path path)
{
    m_original_name = path.filename().generic_string();

    FT_Face face;

    FT_Error error = FT_New_Face(AssetManager::GetInstance().GetFreetypeInit(), path.string().c_str(), 0, &face);
    if (error) {
        AG_CORE_WARN("Failed to load font: {}, error code: {}", path.string(), error);
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 128);

    m_height = m_width = 1024;

    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glClearTexImage(m_textureID, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int x_offset = 0;
    int y_offset = 0;
    int row_height = 0;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 32; c < 128; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load Glyph: " << c << std::endl;
            continue;
        }

        FT_GlyphSlot g = face->glyph;

        if (x_offset + static_cast<int>(g->bitmap.width) >= m_width) {
            x_offset = 0;
            y_offset += row_height;
            row_height = 0;
        }

        row_height = std::max(row_height, static_cast<int>(g->bitmap.rows));

        glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, y_offset, g->bitmap.width, g->bitmap.rows,
            GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        Glyphs glyph;    // stats are divided 100.f to make all wrt to hlyphs size of 1.f
        glyph.advanceX = static_cast<float>(g->advance.x >> 6) / 100.f;
        glyph.advanceY = static_cast<float>(g->advance.y >> 6) / 100.f;
        glyph.width = static_cast<float>(g->bitmap.width) / 100.f;
        glyph.height = static_cast<float>(g->bitmap.rows) / 100.f;
        glyph.bearingX = static_cast<float>(g->bitmap_left) / 100.f;
        glyph.bearingY = static_cast<float>(g->bitmap_top) / 100.f;

        glyph.texCoord[0][0] = static_cast<float>(x_offset) / m_width;
        glyph.texCoord[0][1] = static_cast<float>(y_offset) / m_height;

        glyph.texCoord[1][0] = static_cast<float>(x_offset + g->bitmap.width) / m_width;
        glyph.texCoord[1][1] = static_cast<float>(y_offset) / m_height;

        glyph.texCoord[2][0] = static_cast<float>(x_offset + g->bitmap.width) / m_width;
        glyph.texCoord[2][1] = static_cast<float>(y_offset + g->bitmap.rows) / m_height;

        glyph.texCoord[3][0] = static_cast<float>(x_offset) / m_width;
        glyph.texCoord[3][1] = static_cast<float>(y_offset + g->bitmap.rows) / m_height;


        m_glyphs_data[c] = glyph;

        x_offset += g->bitmap.width;
    }

}

void FontAsset::Unload()
{
    glDeleteTextures(1, &m_textureID); 
    m_glyphs_data.clear();
}

void FontAsset::WriteToBinary(std::string new_file)
{
    if (!m_textureID || m_width <= 0 || m_height <= 0) {
        AG_CORE_WARN("[ASSET][FONT] WriteToBinary: invalid texture/size '{}'", m_original_name);
        return;
    }

    std::ofstream out(new_file, std::ios::binary);
    if (!out) {
        AG_CORE_WARN("[ASSET][FONT] WriteToBinary: cannot open '{}'", new_file);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, m_textureID);

    // IMPORTANT: avoid row padding so dump is tightly packed
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Read back pixels: 1 byte per pixel
    const size_t pixelCount = static_cast<size_t>(m_width) * static_cast<size_t>(m_height);
    std::vector<std::uint8_t> pixels(pixelCount);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());

    // --- header (same as before) ---
    const char magic[8] = { 'A','G','F','N','T','v','1','\0' };
    const std::uint32_t version = 1u;
    const std::uint32_t width = static_cast<std::uint32_t>(m_width);
    const std::uint32_t height = static_cast<std::uint32_t>(m_height);
    const std::uint32_t channels = 1u;
    const std::uint32_t nameLen = static_cast<std::uint32_t>(m_original_name.size());
    const std::uint32_t glyphCount = static_cast<std::uint32_t>(m_glyphs_data.size());

    out.write(magic, sizeof(magic));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&width), sizeof(width));
    out.write(reinterpret_cast<const char*>(&height), sizeof(height));
    out.write(reinterpret_cast<const char*>(&channels), sizeof(channels));
    out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
    out.write(reinterpret_cast<const char*>(&glyphCount), sizeof(glyphCount));

    if (nameLen) out.write(m_original_name.data(), static_cast<std::streamsize>(nameLen));
    if (!pixels.empty())
        out.write(reinterpret_cast<const char*>(pixels.data()),
            static_cast<std::streamsize>(pixels.size()));

    for (const auto& kv : m_glyphs_data) {
        std::uint32_t code = static_cast<std::uint8_t>(kv.first);
        const Glyphs& g = kv.second;

        out.write(reinterpret_cast<const char*>(&code), sizeof(code));
        out.write(reinterpret_cast<const char*>(&g.advanceX), sizeof(g.advanceX));
        out.write(reinterpret_cast<const char*>(&g.advanceY), sizeof(g.advanceY));
        out.write(reinterpret_cast<const char*>(&g.width), sizeof(g.width));
        out.write(reinterpret_cast<const char*>(&g.height), sizeof(g.height));
        out.write(reinterpret_cast<const char*>(&g.bearingX), sizeof(g.bearingX));
        out.write(reinterpret_cast<const char*>(&g.bearingY), sizeof(g.bearingY));
        out.write(reinterpret_cast<const char*>(&g.atlasX), sizeof(g.atlasX));
        out.write(reinterpret_cast<const char*>(&g.atlasY), sizeof(g.atlasY));
        out.write(reinterpret_cast<const char*>(&g.texCoord[0][0]), sizeof(float) * 8);
    }

    if (!out.good()) {
        AG_CORE_WARN("[ASSET][FONT] WriteToBinary: I/O error '{}'", new_file);
    }
}

void FontAsset::LoadFromBinary(std::filesystem::path path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        AG_CORE_WARN("[ASSET][FONT] LoadFromBinary: cannot open '{}'", path.string());
        return;
    }

    char magic[8] = {};
    in.read(magic, sizeof(magic));
    const char expected[8] = { 'A','G','F','N','T','v','1','\0' };
    if (std::memcmp(magic, expected, sizeof(expected)) != 0) {
        AG_CORE_WARN("[ASSET][FONT] LoadFromBinary: bad magic '{}'", path.string());
        return;
    }

    std::uint32_t version = 0, width = 0, height = 0, channels = 0, nameLen = 0, glyphCount = 0;
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    in.read(reinterpret_cast<char*>(&width), sizeof(width));
    in.read(reinterpret_cast<char*>(&height), sizeof(height));
    in.read(reinterpret_cast<char*>(&channels), sizeof(channels));
    in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
    in.read(reinterpret_cast<char*>(&glyphCount), sizeof(glyphCount));

    if (!in.good() || version != 1u || channels != 1u) {
        AG_CORE_WARN("[ASSET][FONT] LoadFromBinary: unsupported header ver={}, ch={} '{}'",
            version, channels, path.string());
        return;
    }

    m_original_name.clear();
    if (nameLen) {
        m_original_name.resize(nameLen);
        in.read(m_original_name.data(), static_cast<std::streamsize>(nameLen));
    }
    else {
        m_original_name = path.filename().generic_string();
    }
    asset_name = path.filename().generic_string();
    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
    std::vector<std::uint8_t> pixels(pixelCount);
    if (pixelCount) {
        in.read(reinterpret_cast<char*>(pixels.data()),
            static_cast<std::streamsize>(pixels.size()));
    }
    if (!in.good()) {
        AG_CORE_WARN("[ASSET][FONT] LoadFromBinary: truncated pixel data '{}'", path.string());
        return;
    }

    // Recreate texture exactly like original Load(), but ensure alignment and (optionally) swizzle
    if (m_textureID) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }

    m_width = static_cast<int>(width);
    m_height = static_cast<int>(height);
    m_channel = 1;

    if (m_textureID) glDeleteTextures(1, &m_textureID);
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);                 // <—
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Make alpha come from RED so shaders using .a work identically:
    GLint swizzle[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };  // <—
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);


    // If your shader samples .a for coverage, uncomment this swizzle:
    // GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED }; // RGB=1, A=R
    // glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    // Glyphs
    m_glyphs_data.clear();
    m_glyphs_data.reserve(glyphCount);

    for (std::uint32_t i = 0; i < glyphCount; ++i) {
        std::uint32_t code = 0;
        Glyphs g{};
        in.read(reinterpret_cast<char*>(&code), sizeof(code));
        in.read(reinterpret_cast<char*>(&g.advanceX), sizeof(g.advanceX));
        in.read(reinterpret_cast<char*>(&g.advanceY), sizeof(g.advanceY));
        in.read(reinterpret_cast<char*>(&g.width), sizeof(g.width));
        in.read(reinterpret_cast<char*>(&g.height), sizeof(g.height));
        in.read(reinterpret_cast<char*>(&g.bearingX), sizeof(g.bearingX));
        in.read(reinterpret_cast<char*>(&g.bearingY), sizeof(g.bearingY));
        in.read(reinterpret_cast<char*>(&g.atlasX), sizeof(g.atlasX));
        in.read(reinterpret_cast<char*>(&g.atlasY), sizeof(g.atlasY));
        in.read(reinterpret_cast<char*>(&g.texCoord[0][0]), sizeof(float) * 8);
        if (!in.good()) {
            AG_CORE_WARN("[ASSET][FONT] LoadFromBinary: truncated glyph table '{}'", path.string());
            m_glyphs_data.clear();
            return;
        }
        m_glyphs_data[static_cast<char>(static_cast<std::uint8_t>(code & 0xFFu))] = g;
    }
}

