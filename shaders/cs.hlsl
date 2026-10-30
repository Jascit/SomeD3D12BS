RWTexture2D<float4> output : register(u0);

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= 0 || id.y >= 0)
        return;
    output[id.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}