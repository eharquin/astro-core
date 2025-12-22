#define MAX_TEXTURES 256

// ==========================
// Uniform buffer
// ==========================

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

// ==========================
// Vertex stage
// ==========================

struct VSInput {
    float3 inPos      : POSITION;
    float3 inColor    : COLOR;
    float3 inNormal   : NORMAL;
    float2 inTexCoord : TEXCOORD0;
};

struct VSOutput {
    float4 pos          : SV_Position;
    float3 fragColor    : COLOR;
    float2 fragTexCoord : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPos, 1.0))));
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

// ==========================
// Push constants
// ==========================

struct PushConstants {
    uint textureIndex;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

// ==========================
// Combined image samplers
// ==========================

[[vk::binding(1, 0)]]
Texture2D<float4> uTextures[MAX_TEXTURES];

[[vk::binding(1, 0)]]
SamplerState uSampler;

// ==========================
// Fragment stage
// ==========================

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float4 texColor =
        uTextures[pc.textureIndex].Sample(
            uSampler,
            vertIn.fragTexCoord
        );

    return texColor;
}
