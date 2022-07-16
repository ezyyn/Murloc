#type vertex

#version 450 

layout(binding = 0) uniform UniformBufferData {
    mat4 Model;
    mat4 View;
    mat4 Projection;
} u_UniformBufferData;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec2 o_TexCoords;

void main()
{
    o_Color = a_Color;
    o_TexCoords = a_TexCoords;

    gl_Position = u_UniformBufferData.Projection * u_UniformBufferData.View * u_UniformBufferData.Model * vec4(a_Position, 1.0);
}

#type fragment

#version 450 

layout(location = 0) in vec4 in_Color;
layout(location = 1) in vec2 in_TexCoords;

layout(location = 0) out vec4 o_Color;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    o_Color = in_Color * texture(texSampler, in_TexCoords);
}