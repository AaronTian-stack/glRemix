cbuffer MVP : register(b0)
{
    row_major float4x4 model;
    row_major float4x4 view;
    row_major float4x4 proj;
};

struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    
    float4 p = float4(input.Position, 1.0f);
    output.Position = mul(mul(mul(p, model), view), proj);
    output.Color = input.Color;
    
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.Color;
}
