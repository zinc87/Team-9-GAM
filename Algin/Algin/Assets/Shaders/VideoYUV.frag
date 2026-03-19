#version 450 core

// VideoYUV Fragment Shader
// Converts YCbCr (YUV) color space to RGB for video playback.
// 
// pl_mpeg outputs frames in YCbCr 4:2:0 format with separate planes:
// - Y (luminance): full resolution, represents brightness
// - Cb (blue-difference chroma): half resolution
// - Cr (red-difference chroma): half resolution
//
// The GPU handles chroma upsampling automatically via texture filtering.
// This approach is more efficient than CPU-side RGB conversion.

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

// Y, Cb, Cr planes as separate single-channel (R8) textures
// Bound to texture units 0, 1, 2 respectively by VideoTexture::BindYUV()
layout (binding = 0) uniform sampler2D uTexY;
layout (binding = 1) uniform sampler2D uTexCb;
layout (binding = 2) uniform sampler2D uTexCr;

void main()
{
    // Sample luminance and chrominance values
    // Y is in range [0, 1], representing brightness
    float y = texture(uTexY, TexCoords).r;
    
    // Cb and Cr are centered at 0.5 in the texture (stored as [0,1])
    // Subtract 0.5 to get the actual chroma offset [-0.5, 0.5]
    float cb = texture(uTexCb, TexCoords).r - 0.5;
    float cr = texture(uTexCr, TexCoords).r - 0.5;

    // BT.601 YCbCr → RGB conversion (standard for MPEG-1)
    // This is the inverse of:
    //   Y  =  0.299*R + 0.587*G + 0.114*B
    //   Cb = -0.169*R - 0.331*G + 0.500*B
    //   Cr =  0.500*R - 0.419*G - 0.081*B
    vec3 rgb;
    rgb.r = y + 1.402 * cr;
    rgb.g = y - 0.344136 * cb - 0.714136 * cr;
    rgb.b = y + 1.772 * cb;

    // Clamp to valid range (decoding artifacts can produce out-of-range values)
    rgb = clamp(rgb, 0.0, 1.0);

    // Output linear RGB (gamma correction can be applied in post-processing)
    FragColor = vec4(rgb, 1.0);
}
