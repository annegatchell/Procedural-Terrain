

////////////////////////////////////////////////////////////////////////////////////////////////////

 

#define ONE_F1                 (1.0f)

#define ZERO_F1                (0.0f)

 

#define USE_IMAGES_FOR_RESULTS (0)  // NOTE: It may be faster to use buffers instead of images

 

const float4 ZERO_F4 = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

const float4 ONE_F4 = (float4)(1.0f, 1.0f, 1.0f, 1.0f);

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

__constant int P_MASK = 255;

__constant int P_SIZE = 256;

__constant int P[512] = {151,160,137,91,90,15,

  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,

  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,

  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,

  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,

  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,

  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,

  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,

  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,

  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,

  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,

  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,

  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

  151,160,137,91,90,15,

  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,

  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,

  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,

  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,

  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,

  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,

  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,

  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,

  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,

  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,

  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,

  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,  

  };

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

__constant int G_MASK = 15;

__constant int G_SIZE = 16;

__constant int G_VECSIZE = 4;

__constant float G[16*4] = {

      +ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 

      -ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 

      +ONE_F1,  -ONE_F1, +ZERO_F1, +ZERO_F1, 

      -ONE_F1,  -ONE_F1, +ZERO_F1, +ZERO_F1,

      +ONE_F1, +ZERO_F1,  +ONE_F1, +ZERO_F1, 

      -ONE_F1, +ZERO_F1,  +ONE_F1, +ZERO_F1, 

      +ONE_F1, +ZERO_F1,  -ONE_F1, +ZERO_F1, 

      -ONE_F1, +ZERO_F1,  -ONE_F1, +ZERO_F1,

     +ZERO_F1,  +ONE_F1,  +ONE_F1, +ZERO_F1, 

     +ZERO_F1,  -ONE_F1,  +ONE_F1, +ZERO_F1, 

     +ZERO_F1,  +ONE_F1,  -ONE_F1, +ZERO_F1, 

     +ZERO_F1,  -ONE_F1,  -ONE_F1, +ZERO_F1,

      +ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 

      -ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 

     +ZERO_F1,  -ONE_F1,  +ONE_F1, +ZERO_F1, 

     +ZERO_F1,  -ONE_F1,  -ONE_F1, +ZERO_F1

};  

  

////////////////////////////////////////////////////////////////////////////////////////////////////

 

int mod(int x, int a)

{

    int n = (x / a);

    int v = v - n * a;

    if ( v < 0 )

        v += a;

    return v;   

}

 

float smooth(float t)

{

    return t*t*t*(t*(t*6.0f-15.0f)+10.0f); 

}

 

float4 normalized(float4 v)

{

    float d = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    d = d > 0.0f ? d : 1.0f;

    float4 result = (float4)(v.x, v.y, v.z, 0.0f) / d;

    result.w = 1.0f;

    return result;

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

float mix1d(float a, float b, float t)

{

    float ba = b - a;

    float tba = t * ba;

    float atba = a + tba;

    return atba;    

}

 

float2 mix2d(float2 a, float2 b, float t)

{

    float2 ba = b - a;

    float2 tba = t * ba;

    float2 atba = a + tba;

    return atba;    

}

 

float4 mix3d(float4 a, float4 b, float t)

{

    float4 ba = b - a;

    float4 tba = t * ba;

    float4 atba = a + tba;

    return atba;    

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

int lattice1d(int i)

{

    return P[i];

}

 

int lattice2d(int2 i)

{

    return P[i.x + P[i.y]];

}

 

int lattice3d(int4 i)

{

    return P[i.x + P[i.y + P[i.z]]];

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

float gradient1d(int i, float v)

{

    int index = (lattice1d(i) & G_MASK) * G_VECSIZE;

    float g = G[index + 0];

    return (v * g);

}

 

float gradient2d(int2 i, float2 v)

{

    int index = (lattice2d(i) & G_MASK) * G_VECSIZE;

    float2 g = (float2)(G[index + 0], G[index + 1]);

    return dot(v, g);

}

 

float gradient3d(int4 i, float4 v)

{

    int index = (lattice3d(i) & G_MASK) * G_VECSIZE;

    float4 g = (float4)(G[index + 0], G[index + 1], G[index + 2], 1.0f);

    return dot(v, g);

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

// Signed gradient noise 1d

float sgnoise1d(float position)

{

    float p = position;

    float pf = floor(p);

    int ip = (int)pf;

    float fp = p - pf;        

    ip &= P_MASK;

    

    float n0 = gradient1d(ip + 0, fp - 0.0f);

    float n1 = gradient1d(ip + 1, fp - 1.0f);

 

    float n = mix1d(n0, n1, smooth(fp));

    return n * (1.0f / 0.7f);

}

 

// Signed gradient noise 2d

float sgnoise2d(float2 position)

{

    float2 p = position;

    float2 pf = floor(p);

    int2 ip = (int2)((int)pf.x, (int)pf.y);

    float2 fp = p - pf;        

    ip &= P_MASK;

    

    const int2 I00 = (int2)(0, 0);

    const int2 I01 = (int2)(0, 1);

    const int2 I10 = (int2)(1, 0);

    const int2 I11 = (int2)(1, 1);

    

    const float2 F00 = (float2)(0.0f, 0.0f);

    const float2 F01 = (float2)(0.0f, 1.0f);

    const float2 F10 = (float2)(1.0f, 0.0f);

    const float2 F11 = (float2)(1.0f, 1.0f);

 

    float n00 = gradient2d(ip + I00, fp - F00);

    float n10 = gradient2d(ip + I10, fp - F10);

    float n01 = gradient2d(ip + I01, fp - F01);

    float n11 = gradient2d(ip + I11, fp - F11);

 

    const float2 n0001 = (float2)(n00, n01);

    const float2 n1011 = (float2)(n10, n11);

 

    float2 n2 = mix2d(n0001, n1011, smooth(fp.x));

    float n = mix1d(n2.x, n2.y, smooth(fp.y));

    return n * (1.0f / 0.7f);

}

 

// Signed gradient noise 3d

float sgnoise3d(float4 position)

{

 

    float4 p = position;

    float4 pf = floor(p);

    int4 ip = (int4)((int)pf.x, (int)pf.y, (int)pf.z, 0.0);

    float4 fp = p - pf;        

    ip &= P_MASK;

 

    int4 I000 = (int4)(0, 0, 0, 0);

    int4 I001 = (int4)(0, 0, 1, 0);  

    int4 I010 = (int4)(0, 1, 0, 0);

    int4 I011 = (int4)(0, 1, 1, 0);

    int4 I100 = (int4)(1, 0, 0, 0);

    int4 I101 = (int4)(1, 0, 1, 0);

    int4 I110 = (int4)(1, 1, 0, 0);

    int4 I111 = (int4)(1, 1, 1, 0);

    

    float4 F000 = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

    float4 F001 = (float4)(0.0f, 0.0f, 1.0f, 0.0f);

    float4 F010 = (float4)(0.0f, 1.0f, 0.0f, 0.0f);

    float4 F011 = (float4)(0.0f, 1.0f, 1.0f, 0.0f);

    float4 F100 = (float4)(1.0f, 0.0f, 0.0f, 0.0f);

    float4 F101 = (float4)(1.0f, 0.0f, 1.0f, 0.0f);

    float4 F110 = (float4)(1.0f, 1.0f, 0.0f, 0.0f);

    float4 F111 = (float4)(1.0f, 1.0f, 1.0f, 0.0f);

    

    float n000 = gradient3d(ip + I000, fp - F000);

    float n001 = gradient3d(ip + I001, fp - F001);

    

    float n010 = gradient3d(ip + I010, fp - F010);

    float n011 = gradient3d(ip + I011, fp - F011);

    

    float n100 = gradient3d(ip + I100, fp - F100);

    float n101 = gradient3d(ip + I101, fp - F101);

 

    float n110 = gradient3d(ip + I110, fp - F110);

    float n111 = gradient3d(ip + I111, fp - F111);

 

    float4 n40 = (float4)(n000, n001, n010, n011);

    float4 n41 = (float4)(n100, n101, n110, n111);

 

    float4 n4 = mix3d(n40, n41, smooth(fp.x));

    float2 n2 = mix2d(n4.xy, n4.zw, smooth(fp.y));

    float n = mix1d(n2.x, n2.y, smooth(fp.z));

    return n * (1.0f / 0.7f);

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

// Unsigned Gradient Noise 1d

float ugnoise1d(float position)

{

    return (0.5f - 0.5f * sgnoise1d(position));

}

 

// Unsigned Gradient Noise 2d

float ugnoise2d(float2 position)

{

    return (0.5f - 0.5f * sgnoise2d(position));

}

 

// Unsigned Gradient Noise 3d

float ugnoise3d(float4 position)

{

    return (0.5f - 0.5f * sgnoise3d(position));

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

uchar4

tonemap(float4 color)

{

    uchar4 result = convert_uchar4_sat_rte(color * 255.0f);

    return result;

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

float monofractal2d(

    float2 position, 

    float frequency,

    float lacunarity, 

    float increment, 

    float octaves)

{

    int i = 0;

    float fi = 0.0f;

    float remainder = 0.0f; 

    float sample = 0.0f;    

    float value = 0.0f;

    int iterations = (int)octaves;

    

    for (i = 0; i < iterations; i++)

    {

        fi = (float)i;

        sample = sgnoise2d(position * frequency);

        sample *= pow( lacunarity, -fi * increment );

        value += sample;

        frequency *= lacunarity;

    }

    

    remainder = octaves - (float)iterations;

    if ( remainder > 0.0f )

    {

        sample = remainder * sgnoise2d(position * frequency);

        sample *= pow( lacunarity, -fi * increment );

        value += sample;

    }

        

    return value;   

}

 

float multifractal2d(

    float2 position, 

    float frequency,

    float lacunarity, 

    float increment, 

    float octaves)

{

    int i = 0;

    float fi = 0.0f;

    float remainder = 0.0f; 

    float sample = 0.0f;    

    float value = 1.0f;

    int iterations = (int)octaves;

    

    for (i = 0; i < iterations; i++)

    {

        fi = (float)i;

        sample = sgnoise2d(position * frequency) + 1.0f;

        sample *= pow( lacunarity, -fi * increment );

        value *= sample;

        frequency *= lacunarity;

    }

    

    remainder = octaves - (float)iterations;

    if ( remainder > 0.0f )

    {

        sample = remainder * (sgnoise2d(position * frequency) + 1.0f);

        sample *= pow( lacunarity, -fi * increment );

        value *= sample;

    }

        

    return value;   

}

 

float turbulence2d(

    float2 position, 

    float frequency,

    float lacunarity, 

    float increment, 

    float octaves)

{

    int i = 0;

    float fi = 0.0f;

    float remainder = 0.0f; 

    float sample = 0.0f;    

    float value = 0.0f;

    int iterations = (int)octaves;

    

    for (i = 0; i < iterations; i++)

    {

        fi = (float)i;

        sample = sgnoise2d(position * frequency);

        sample *= pow( lacunarity, -fi * increment );

        value += fabs(sample);

        frequency *= lacunarity;

    }

    

    remainder = octaves - (float)iterations;

    if ( remainder > 0.0f )

    {

        sample = remainder * sgnoise2d(position * frequency);

        sample *= pow( lacunarity, -fi * increment );

        value += fabs(sample);

    }

        

    return value;   

}

 

float ridgedmultifractal2d(

    float2 position, 

    float frequency,

    float lacunarity, 

    float increment, 

    float octaves)

{

    int i = 0;

    float fi = 0.0f;

    float remainder = 0.0f; 

    float sample = 0.0f;    

    float value = 0.0f;

    int iterations = (int)octaves;

 

    float threshold = 0.5f;

    float offset = 1.0f;

    float weight = 1.0f;

 

    float signal = fabs( sgnoise2d(position * frequency) );

    signal = offset - signal;

    signal *= signal;

    value = signal;

 

    for ( i = 0; i < iterations; i++ )

    {

        frequency *= lacunarity;

        weight = clamp( signal * threshold, 0.0f, 1.0f );   

        signal = fabs( sgnoise2d(position * frequency) );

        signal = offset - signal;

        signal *= signal;

        signal *= weight;

        value += signal * pow( lacunarity, -fi * increment );

 

    }

    return value;

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

float multifractal3d(

    float4 position, 

    float frequency,

    float lacunarity, 

    float increment, 

    float octaves)

{

    int i = 0;

    float fi = 0.0f;

    float remainder = 0.0f; 

    float sample = 0.0f;    

    float value = 1.0f;

    int iterations = (int)octaves;

    

    for (i = 0; i < iterations; i++)

    {

        fi = (float)i;

        sample = (1.0f - 2.0f * sgnoise3d(position * frequency)) + 1.0f;

        sample *= pow( lacunarity, -fi * increment );

        value *= sample;

        frequency *= lacunarity;

    }

    

    remainder = octaves - (float)iterations;

    if ( remainder > 0.0f )

    {

        sample = remainder * (1.0f - 2.0f * sgnoise3d(position * frequency)) + 1.0f;

        sample *= pow( lacunarity, -fi * increment );

        value *= sample;

    }

        

    return value;   

}

 

float ridgedmultifractal3d(

    float4 position, 

    float frequency,

    float lacunarity, 

    float increment, 

    float octaves)

{

    int i = 0;

    float fi = 0.0f;

    float remainder = 0.0f; 

    float sample = 0.0f;    

    float value = 0.0f;

    int iterations = (int)octaves;

 

    float threshold = 0.5f;

    float offset = 1.0f;

    float weight = 1.0f;

 

    float signal = fabs( (1.0f - 2.0f * sgnoise3d(position * frequency)) );

    signal = offset - signal;

    signal *= signal;

    value = signal;

 

    for ( i = 0; i < iterations; i++ )

    {

        frequency *= lacunarity;

        weight = clamp( signal * threshold, 0.0f, 1.0f );   

        signal = fabs( (1.0f - 2.0f * sgnoise3d(position * frequency)) );

        signal = offset - signal;

        signal *= signal;

        signal *= weight;

        value += signal * pow( lacunarity, -fi * increment );

 

    }

    return value;

}

 

float turbulence3d(

    float4 position, 

    float frequency,

    float lacunarity, 

    float increment, 

    float octaves)

{

    int i = 0;

    float fi = 0.0f;

    float remainder = 0.0f; 

    float sample = 0.0f;    

    float value = 0.0f;

    int iterations = (int)octaves;

    

    for (i = 0; i < iterations; i++)

    {

        fi = (float)i;

        sample = (1.0f - 2.0f * sgnoise3d(position * frequency));

        sample *= pow( lacunarity, -fi * increment );

        value += fabs(sample);

        frequency *= lacunarity;

    }

    

    remainder = octaves - (float)iterations;

    if ( remainder > 0.0f )

    {

        sample = remainder * (1.0f - 2.0f * sgnoise3d(position * frequency));

        sample *= pow( lacunarity, -fi * increment );

        value += fabs(sample);

    }

        

    return value;   

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

#if USE_IMAGES_FOR_RESULTS

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

__kernel void 

GradientNoiseImage2d(   

    write_only image2d_t output,

    const float2 bias, 

    const float2 scale,

    const float amplitude) 

{

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

 

    int2 size = (int2)(get_global_size(0), get_global_size(1));

 

    float2 position = (float2)(coord.x / (float)size.x, 

                                  coord.y / (float)size.y);

        

    float2 sample = (position + bias) * scale;

   

    float value = ugnoise2d(sample);

    

    float4 color = (float4)(value, value, value, 1.0f) * amplitude;

    color.w = 1.0f;

    

    write_imagef(output, coord, color);

}

 

__kernel void 

MonoFractalImage2d(

    write_only image2d_t output,

    const float2 bias, 

    const float2 scale,

    const float lacunarity, 

    const float increment, 

    const float octaves,    

    const float amplitude)

{

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

 

    int2 size = (int2)(get_global_size(0), get_global_size(1));

 

    float2 position = (float2)(coord.x / (float)size.x, 

                                  coord.y / (float)size.y);

        

    float2 sample = (position + bias);

   

    float value = monofractal2d(sample, scale.x, lacunarity, increment, octaves);

 

    float4 color = (float4)(value, value, value, 1.0f) * amplitude;

    color.w = 1.0f;

    

    write_imagef(output, coord, color);

}

 

__kernel void 

TurbulenceImage2d(

    write_only image2d_t output,

    const float2 bias, 

    const float2 scale,

    const float lacunarity, 

    const float increment, 

    const float octaves,    

    const float amplitude) 

{

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

 

    int2 size = (int2)(get_global_size(0), get_global_size(1));

 

    float2 position = (float2)(coord.x / (float)size.x, 

                                  coord.y / (float)size.y);

    

    float2 sample = (position + bias);

 

    float value = turbulence2d(sample, scale.x, lacunarity, increment, octaves);

 

    float4 color = (float4)(value, value, value, 1.0f) * amplitude;

    color.w = 1.0f;

 

    write_imagef(output, coord, color);

}

 

__kernel void 

RidgedMultiFractalImage2d(  

    write_only image2d_t output,

    const float2 bias, 

    const float2 scale,

    const float lacunarity, 

    const float increment, 

    const float octaves,    

    const float amplitude) 

{

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

 

    int2 size = (int2)(get_global_size(0), get_global_size(1));

 

    float2 position = (float2)(coord.x / (float)size.x, 

                                  coord.y / (float)size.y);

        

    float2 sample = (position + bias);

 

    float value = ridgedmultifractal2d(sample, scale.x, lacunarity, increment, octaves);

 

    float4 color = (float4)(value, value, value, 1.0f) * amplitude;

    color.w = 1.0f;

 

    write_imagef(output, coord, color);

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

#endif // USE_IMAGES_FOR_RESULTS

 

////////////////////////////////////////////////////////////////////////////////////////////////////

 

__kernel void 

GradientNoiseArray2d(   

    __global uchar4 *output,

    const float2 bias, 

    const float2 scale,

    const float amplitude) 

{

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

 

    int2 size = (int2)(get_global_size(0), get_global_size(1));

 

    float2 position = (float2)(coord.x / (float)size.x, coord.y / (float)size.y);

    

    float2 sample = (position + bias) * scale;

   

    float value = ugnoise2d(sample);

    

    float4 result = (float4)(value, value, value, 1.0f) * amplitude;

 

    uint index = coord.y * size.x + coord.x;

    output[index] = tonemap(result);

}

 

__kernel void 

MonoFractalArray2d(

    __global uchar4 *output,

    const float2 bias, 

    const float2 scale,

    const float lacunarity, 

    const float increment, 

    const float octaves,    

    const float amplitude)

{

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

 

    int2 size = (int2)(get_global_size(0), get_global_size(1));

 

    float2 position = (float2)(coord.x / (float)size.x, 

                                  coord.y / (float)size.y);

    

    float2 sample = (position + bias);

   

    float value = monofractal2d(sample, scale.x, lacunarity, increment, octaves);

 

    float4 result = (float4)(value, value, value, 1.0f) * amplitude;

 

    uint index = coord.y * size.x + coord.x;

    output[index] = tonemap(result);

}

 

__kernel void 

TurbulenceArray2d(

    __global uchar4 *output,

    const float2 bias, 

    const float2 scale,

    const float lacunarity, 

    const float increment, 

    const float octaves,    

    const float amplitude) 

{

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

 

    int2 size = (int2)(get_global_size(0), get_global_size(1));

 

    float2 position = (float2)(coord.x / (float)size.x, coord.y / (float)size.y);

    

    float2 sample = (position + bias);

 

    float value = turbulence2d(sample, scale.x, lacunarity, increment, octaves);

 

    float4 result = (float4)(value, value, value, 1.0f) * amplitude;

 

    uint index = coord.y * size.x + coord.x;

    output[index] = tonemap(result);

}

 

__kernel void 

RidgedMultiFractalArray2d(  

    __global uchar4 *output,

    const float2 bias, 

    const float2 scale,

    const float lacunarity, 

    const float increment, 

    const float octaves,    

    const float amplitude) 

{

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

 

    int2 size = (int2)(get_global_size(0), get_global_size(1));

 

    float2 position = (float2)(coord.x / (float)size.x, 

                                  coord.y / (float)size.y);

        

    float2 sample = (position + bias);

 

    float value = ridgedmultifractal2d(sample, scale.x, lacunarity, increment, octaves);

 

    float4 result = (float4)(value, value, value, 1.0f) * amplitude;

 

    uint index = coord.y * size.x + coord.x;

    output[index] = tonemap(result);

}

 

////////////////////////////////////////////////////////////////////////////////////////////////////


__constant int edgeTable[256]={
0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0   };
__constant int triTable[256][16] =
{{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

typedef struct
{
	float x;
   	float y;
   	float z;
 } float3; 

 typedef struct  
 { 
    float4 coordAmb; 
    float4 normal; 
 } rockVertex; 

 typedef struct  
 { 
    rockVertex rv1; 
    rockVertex rv2; 
    rockVertex rv3; 
 } triangle; 

 typedef struct 
 { 
    float3 rv1; 
    float3 rv2; 
    float3 rv3;   
 } triangleVert; 

 typedef struct 
 { 
    float3 n1; 
    float3 n2; 
    float3 n3; 
 } triangleNorm; 

 float transformX(float xOnBlock, float xPos, float n) 
 { 
   return xPos+(xOnBlock*1/(n-1)); 
  } 

 float transformY(float yOnBlock, float yPos, float n) 
 { 
   return yPos+(yOnBlock*1/(n-1)); 
  } 

 float transformZ(float zOnBlock, float zPos, float n) 
 { 
   return zPos-(zOnBlock*1/(n-1)); 
  } 

 float4 transform(float4 onBlock, float4 Pos, float n) 
 { 
   float4 ret; 
   ret.x = Pos.x+(onBlock.x*1/(n-1)); 
   ret.y = Pos.y+(onBlock.y*1/(n-1)); 
   ret.z = Pos.z-(onBlock.z*1/(n-1)); 
   ret.w = 0.0f; 
   return ret; 
  } 

 int a3dto1d(int i, int j, int k, int n) 
 { 
   return (k+n*j+n*n*i); 
 } 

 float densityFunction(float x, float y, float z) 
 { 
 	float4 position= {x, y, z-1, 0};
 	//float multifractal3d(float4 position, float frequency,float lacunarity, float increment, float octaves)
 	float temp = (multifractal3d(position, 2, 0.1, 1, 5)/1000000000000)-0.5;
   return (float) temp; 
 } 

 float densityFunctionVec(float4 position) 
 { 
   float temp = densityFunction(position.x, position.y, position.z); 
   return temp; 
 } 

 float4 VertexInterp(float isolevel, float4 p1, float4 p2, float valp1, float valp2) 
 { 
   float mu; 
   float4 p; 
   float temp = isolevel - valp1; 
   if(temp < 0) temp *= -1; 
   if (temp < 0.0001) 
       return(p1); 

   temp = isolevel - valp2; 
   if(temp < 0) temp *= -1; 
   if (temp < 0.0001) 
       return(p2); 

   temp = valp1-valp2; 
   if(temp < 0) temp *= -1; 
   if (temp < 0.0001) 
       return(p1); 

   mu = (isolevel - valp1) / (valp2 - valp1); 
   p.x = p1.x + mu * (p2.x - p1.x); 
   p.y = p1.y + mu * (p2.y - p1.y); 
   p.z = p1.z + mu * (p2.z - p1.z); 
   p.w = -1; 

    return(p);  
 } 

 float4 getNormal(float4 position) 
 { 
    float d = 1.0; 
    float4 grad; 
    grad.x = densityFunctionVec(position + (float4) {d, 0, 0, 0}) -   
          densityFunctionVec(position + (float4){-d, 0, 0,0});   
    grad.y = densityFunctionVec(position + (float4){ 0, d, 0,0}) -   
          densityFunctionVec(position + (float4){0,-d, 0,0});   
    grad.z = densityFunctionVec(position + (float4){0, 0, d,0}) -   
          densityFunctionVec(position + (float4){0, 0,-d,0});  
    grad.w = 1; 
    return normalize(grad); 
 } 

 __kernel void densityCalc(__global float density[],  
                            const float x, const float y, const float z, const unsigned int n) 
 { 
   unsigned int i = get_global_id(0); 
   unsigned int j = get_global_id(1); 
   unsigned int k = get_global_id(2); 

   float actualX = transformX(i, x, n); 
   float actualY = transformY(j, y, n); 
   float actualZ = transformZ(k, z, n); 
   float4 actualPos = (float4) (x,y,z,0.0f); 

   density[k + j*n + i*n*n] = densityFunction(actualX, actualY, actualZ); 

    
 } 

 __kernel void triangles(__global float density[],  
                            const float x, const float y, const float z, const unsigned int nEdges, 
                            __global float numTriangles[],__global float triangles[],__global triangleVert rockNormals[], 
                            unsigned int n, __global float debug[], __global triangleVert rockTriangles[]) 
 { 
//nEdges = 4
//n = 3
   unsigned int i = get_global_id(0); 
   unsigned int j = get_global_id(1); 
   unsigned int k = get_global_id(2); 

   float actualX = transformX(i, x, nEdges); 
   float actualY = transformY(j, y, nEdges); 
   float actualZ = transformZ(k, z, nEdges); 
   float4 actualPos = (float4) (x,y,z,0.0f); 


   int i0,i1,i2,i3,i4,i5,i6,i7; 
   i0 = a3dto1d(i,j,k,nEdges); 
   i1 = a3dto1d(i,j,k+1,nEdges); 
   i2 = a3dto1d(i+1,j,k+1,nEdges); 
   i3 = a3dto1d(i+1,j,k,nEdges); 
   i4 = a3dto1d(i,j+1,k,nEdges); 
   i5 = a3dto1d(i,j+1,k+1,nEdges); 
   i6 = a3dto1d(i+1,j+1,k+1,nEdges); 
   i7 = a3dto1d(i+1,j+1,k,nEdges); 

   float4 p0,p1,p2,p3,p4,p5,p6,p7; 
   p0 = transform((float4)(i, j, k, 0.0f), actualPos, nEdges); 
   p1 = transform((float4)(i, j, k+1, 0.0f), actualPos, nEdges); 
   p2 = transform((float4)(i+1, j, k+1, 0.0f), actualPos, nEdges); 
   p3 = transform((float4)(i+1, j, k, 0.0f), actualPos, nEdges); 
   p4 = transform((float4)(i, j+1, k, 0.0f), actualPos, nEdges); 
   p5 = transform((float4)(i, j+1, k+1, 0.0f), actualPos, nEdges); 
   p6 = transform((float4)(i+1, j+1, k+1, 0.0f), actualPos, nEdges); 
   p7 = transform((float4)(i+1, j+1, k, 0.0f), actualPos, nEdges); 

   int numberOfTrianglesInThisWorkItem = 0; 
   float4 empty = (float4) {0.0f,0.0f,0.0f,0.0f}; 
   float4 vertlist[12]; 
   float4 triangleArrays[5][3]; 
   float4 normalArrays[5][3]; 
   unsigned int cubeindex = 0; 
   float isolevel = 0; 
   //Initialize triangles to -1
    for(int a = 0; a<5*3*4;a++) 
    { 
       triangles[5*3*4*(k+j*n+i*n*n)+a] = -1; 
    } 
//Initialize the triangles arrays to -99

//Initialize triangleArrays

//Initialize vertlist
    for(int c = 0; c<12;c++) 
       {vertlist[c] = empty;} 


//If the voxel isn't hanging off the edge, find the vertices
    if(i<=n && j<=n && k<=n) 
    { 
       if (density[i0] < isolevel) cubeindex |= 1; 
       if (density[i1] < isolevel) cubeindex |= 2; 
       if (density[i2] < isolevel) cubeindex |= 4; 
       if (density[i3] < isolevel) cubeindex |= 8; 
       if (density[i4] < isolevel) cubeindex |= 16; 
       if (density[i5] < isolevel) cubeindex |= 32; 
       if (density[i6] < isolevel) cubeindex |= 64; 
       if (density[i7] < isolevel) cubeindex |= 128; 
   // Cube is entirely in/out of the surface
       if (edgeTable[cubeindex] == 0)  
       { 
          numberOfTrianglesInThisWorkItem = 0;  
       } 
       else{  
         // Find the vertices where the surface intersects the cube  
          if (edgeTable[cubeindex] & 1) 
                vertlist[0] = /*(float4) {11, 11, 11, 11};*/VertexInterp(isolevel,p0,p1,density[i0],density[i1]) ; 
          if (edgeTable[cubeindex] & 2) 
                vertlist[1] = /*(float4) {22, 22, 22, 22};*/VertexInterp(isolevel,p1,p2,density[i1],density[i2]); 
          if (edgeTable[cubeindex] & 4) 
                vertlist[2] = /*(float4) {33, 33, 33, 33};*/VertexInterp(isolevel,p2,p3,density[i2],density[i3]); 
          if (edgeTable[cubeindex] & 8) 
                vertlist[3] = /*(float4) {44, 44, 44, 44};*/VertexInterp(isolevel,p3,p0,density[i3],density[i0]); 
          if (edgeTable[cubeindex] & 16) 
                vertlist[4] = /*(float4) {55, 55, 55, 55};*/VertexInterp(isolevel,p4,p5,density[i4],density[i5]); 
          if (edgeTable[cubeindex] & 32) 
                vertlist[5] = /*(float4) {66, 66, 66, 66};*/VertexInterp(isolevel,p5,p6,density[i5],density[i6]); 
          if (edgeTable[cubeindex] & 64) 
                vertlist[6] = /*(float4) {77, 77, 77, 77};*/VertexInterp(isolevel,p6,p7,density[i6],density[i7]); 
          if (edgeTable[cubeindex] & 128) 
                vertlist[7] = /*(float4) {88, 88, 88, 88};*/VertexInterp(isolevel,p7,p4,density[i7],density[i4]); 
          if (edgeTable[cubeindex] & 256) 
                vertlist[8] = /*(float4) {99, 99, 99, 99};*/VertexInterp(isolevel,p0,p4,density[i0],density[i4]); 
          if (edgeTable[cubeindex] & 512) 
                vertlist[9] = /*(float4) {1010, 1010, 1010, 1010};*/VertexInterp(isolevel,p1,p5,density[i1],density[i5]); 
          if (edgeTable[cubeindex] & 1024) 
                vertlist[10] = /*(float4) {1111, 1111, 1111, 1111};*/VertexInterp(isolevel,p2,p6,density[i2],density[i6]); 
          if (edgeTable[cubeindex] & 2048) 
                vertlist[11] = /*(float4) {1212, 1212, 1212, 1212};*/VertexInterp(isolevel,p3,p7,density[i3],density[i7]); 
//Create the triangle

          int ntriang = 0; 
          for (int w=0;triTable[cubeindex][w]!=-1;w+=3) 
         //for (int w=0;w<15;w+=3)
          {  
             triangleArrays[ntriang][0] = /*(float4) {1, 1, 1, -1};*/vertlist[triTable[cubeindex][w  ]]; 
             triangleArrays[ntriang][1] = /*(float4) {2, 2, 2, -1};*/vertlist[triTable[cubeindex][w+1]]; 
             triangleArrays[ntriang][2] = /*(float4) {3, 3, 3, -1};*/vertlist[triTable[cubeindex][w+2]]; 
             normalArrays[ntriang][0] = getNormal(triangleArrays[ntriang][0]); 
             normalArrays[ntriang][1] = getNormal(triangleArrays[ntriang][0]); 
             normalArrays[ntriang][2] = getNormal(triangleArrays[ntriang][0]); 
             ntriang++; 
             if(ntriang>4) break; 
          } 
          numberOfTrianglesInThisWorkItem = ntriang; 
       } 
    } 
 

//Set the number of triangles for the voxel that this thread is responsbile for 
   numTriangles[k + j*n + i*n*n] = numberOfTrianglesInThisWorkItem; 
//Set the triangles array to the values in the temp triangle arrays

    for(int a = 0; a < 5; a+=1) 
    { 
       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv1.x = triangleArrays[a][0].x; 
       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv1.y = triangleArrays[a][0].y; 
       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv1.z = triangleArrays[a][0].z; 

       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv2.x = triangleArrays[a][1].x; 
       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv2.y = triangleArrays[a][1].y; 
       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv2.z = triangleArrays[a][1].z; 
      
       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv3.x = triangleArrays[a][2].x; 
       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv3.y = triangleArrays[a][2].y; 
       rockTriangles[5*(a3dto1d(i,j,k,n))+a].rv3.z = triangleArrays[a][2].z; 

       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv1.x = normalArrays[a][0].x; 
       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv1.y = normalArrays[a][0].y; 
       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv1.z = normalArrays[a][0].z; 

       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv2.x = normalArrays[a][1].x; 
       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv2.y = normalArrays[a][1].y; 
       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv2.z = normalArrays[a][1].z; 
      
       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv3.x = normalArrays[a][2].x; 
       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv3.y = normalArrays[a][2].y; 
       rockNormals[5*(a3dto1d(i,j,k,n))+a].rv3.z = normalArrays[a][2].z; 
    } 
  } ; 