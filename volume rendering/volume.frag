#version 330 core
in vec3 texCoord;
in vec3 worldPos;

out vec4 FragColor;

uniform sampler3D volumeData;
uniform sampler1D transferFunc;
uniform vec3 eyePos;
uniform float stepSize;

void main() {
    vec3 dir = normalize(worldPos - eyePos);
    vec3 pos = texCoord;

    vec4 colorAccum = vec4(0.0);
    float alphaAccum = 0.0;

    for (int i = 0; i < 512; ++i) {

        float val = texture(volumeData, pos).a;
        vec4 rgba = texture(transferFunc,val);

        //vec4 rgba = vec4(val, val, val, val); // 也可嘗試 vec4(val)

        // 前向累加（簡單光線合成）
        rgba.rgb *= rgba.a;
        colorAccum.rgb += (1.0 - alphaAccum) * rgba.rgb;
        alphaAccum += (1.0 - alphaAccum) * rgba.a;

        if (alphaAccum >= 1 ) break;

        pos += dir * stepSize;
    }
    if(alphaAccum<1.0) colorAccum.rgb+=vec3(0.1f, 0.1f, 0.9f)*(1-alphaAccum);
    FragColor = vec4(colorAccum.rgb, alphaAccum); 
}

/*void main() {
    vec3 dir = normalize(worldPos - eyePos);
    vec3 pos = texCoord;
    vec4 colorAccum = vec4(0.0);
    float alphaAccum = 0.0;
    pos = pos+ dir * 0.05;


    float val = texture(volumeData, pos).a;

    //FragColor = vec4(texCoord,1.0);
    FragColor = vec4(val); 
    //else FragColor = vec4(0.0f); 
} */                                                                   