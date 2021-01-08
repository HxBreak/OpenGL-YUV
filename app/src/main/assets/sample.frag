#version 300 es

precision mediump float;
in vec4 v_Color;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_textureY;
uniform sampler2D s_textureUV;
void main()
{
    float y = texture(s_textureY, v_texCoord).r;
    float u = texture(s_textureUV, v_texCoord).a - 0.5;
    float v = texture(s_textureUV, v_texCoord).r - 0.5;

    outColor = vec4(
        y + 1.13983 * v,
        y - 0.39465 * u - 0.58060 * v,
        y + 2.03211 * u,
        1
    );
}