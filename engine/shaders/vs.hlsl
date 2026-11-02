cbuffer CameraShit : register(b0, space0){
  float4x4 mvp;
};

struct VSInput {
  float3 position : POSITION;
  float3 normals : NORMAL;
  float2 texcoords : TEXCOORD0;
};

struct VSOutput {
  float4 position : SV_POSITION;
  float2 texcoords : TEXCOORD0;
};

VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.position = mul(float4(input.position, 1.0f), mvp);
  output.texcoords = input.texcoords;
  return output;
}
