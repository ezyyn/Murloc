#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;

layout(set = 0, binding = 0) uniform MVP {
    mat4 Model;
    mat4 View;
    mat4 Projection;
} u_MVP;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;
layout (location = 2) out flat float o_TexIndex;
void main()
{
    Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	o_TexIndex = a_TexIndex;

    gl_Position = u_MVP.Projection * u_MVP.View * u_MVP.Model * vec4(a_Position, 1.0);
}

#type fragment

#version 450 core

struct VertexInput
{
	vec4 Color;
	vec2 TexCoord;
};

layout(location = 0) in VertexInput Input;
layout(location = 2) in flat float in_TexIndex;

layout(location = 0) out vec4 o_Color;
layout(binding = 1) uniform sampler2D u_Textures;

void main()
{
    o_Color = Input.Color; //;* texture(texSampler, in_TexCoords);
    o_Color = vec4(Input.TexCoord,0,1.0);
}