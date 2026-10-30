cbuffer Rotation : register(b1, space0){
  float time;
};

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoords    : TEXCOORD0;
};

Texture2D    rainbow : register(t0);
SamplerState smp     : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 uv = input.texcoords + float2(time * 0.2f, 0.0f);
    return rainbow.Sample(smp, uv);
}
