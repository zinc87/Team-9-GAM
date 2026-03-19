#version 450 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D u_ScreenTex;

// Control the blur intensity here (or pass as a uniform)
const float Radius = 4.0; 

void main()
{
    // 1. Get the size of a single pixel (texel) for the current texture
    vec2 texelSize = 1.0 / textureSize(u_ScreenTex, 0);
    
    vec3 result = vec3(0.0);
    float count = 0.0;

    // 2. Loop through neighbors within the Radius
    // Note: A larger radius is more expensive (O(N^2) complexity)
    for(float x = -Radius; x <= Radius; ++x)
    {
        for(float y = -Radius; y <= Radius; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize;
            result += texture(u_ScreenTex, TexCoords + offset).rgb;
            count += 1.0;
        }
    }

    // 3. Average the color
    result = result / count;
    
    FragColor = vec4(result, 1.0);
}