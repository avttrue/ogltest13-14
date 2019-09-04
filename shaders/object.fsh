const int COUNT_LIGHTS = 5; // максимально допустимое количество источников света

struct MaterialProperty
{
    vec3 AmbienceColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;
    float Shines;
};

struct LightProperty
{
    vec3 AmbienceColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;
    vec4 Position;
    vec4 Direction;
    float Cutoff;
    float Power;
    int Type; // Direct - 0, Point - 1, Spot - 2
};

uniform bool u_IsUseDiffuseMap;
uniform bool u_IsUseNormalMap;
uniform bool u_IsDrawShadow;
uniform MaterialProperty u_MaterialProperty;
uniform sampler2D u_DiffuseMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_ShadowMap;
uniform highp float u_ShadowMapSize;
uniform highp float u_ShadowPointCloudFilteringQuality;
uniform LightProperty u_LightProperty[COUNT_LIGHTS];
uniform int u_CountLights;          // реальное количество источников освещения (<= COUNT_LIGHTS)
uniform int u_IndexLightShadow;     // индекс источника освещения с тенью

varying highp vec4 v_position;
varying highp vec2 v_textcoord;
varying highp vec3 v_normal;
varying highp mat3 v_tbnMatrix;
varying highp mat4 v_viewMatrix;
varying highp vec4 v_positionLightMatrix;
LightProperty v_LightProperty[COUNT_LIGHTS];

float SampleShadowMap(sampler2D map, vec2 coords, float compare)
{
    vec4 v = texture2D(map, coords);
    float value = v.x * 255.0f + (v.y * 255.0f + (v.z * 255.0f + v.w) / 255.0f) / 255.0f;
    return step(compare, value);
}

float SampleShadowMapLinear(sampler2D map, vec2 coords, float compare, vec2 texelsize)
{
    vec2 pixsize = coords / texelsize + 0.5f;
    vec2 pixfractpart = fract(pixsize);
    vec2 starttexel = (pixsize - pixfractpart) * texelsize;
    float bltexel = SampleShadowMap(map, starttexel, compare);
    float brtexel = SampleShadowMap(map, starttexel + vec2(texelsize.x, 0.0f), compare);
    float tltexel = SampleShadowMap(map, starttexel + vec2(0.0f, texelsize.y), compare);
    float trtexel = SampleShadowMap(map, starttexel + texelsize, compare);

    float mixL = mix(bltexel, tltexel, pixfractpart.y);
    float mixR = mix(brtexel, trtexel, pixfractpart.y);

    return mix(mixL, mixR, pixfractpart.x);
}

// point cloud filtering
float SampleShadowMapPCF(sampler2D map, vec2 coords, float compare, vec2 texelsize)
{
    float result = 0.0f;
    float spcfq = u_ShadowPointCloudFilteringQuality;
    for(float y = -spcfq; y < spcfq; y += 1.0f)
        for(float x = -spcfq; x < spcfq; x += 1.0f)
        {
            vec2 offset = vec2(x, y) * texelsize;
            result += SampleShadowMapLinear(map, coords + offset, compare, texelsize);
        }
    return result / 9.0f;
}

float CalcShadowAmount(sampler2D map)
{
    vec3 value = v_positionLightMatrix.xyz / v_positionLightMatrix.w;
    value = value * vec3(0.5f) + vec3(0.5f);
    float offset = 2.0f;
    offset *= dot(v_normal, v_LightProperty[u_IndexLightShadow].Direction.xyz);
    return SampleShadowMapPCF(map, value.xy, value.z * 255.0f + offset, vec2(1.0f / u_ShadowMapSize));
}

void main(void)
{
    int countLights = u_CountLights;
    if(countLights > COUNT_LIGHTS) countLights = COUNT_LIGHTS;

    for(int i = 0; i < countLights; i++)
    {
        v_LightProperty[i].AmbienceColor =  u_LightProperty[i].AmbienceColor;
        v_LightProperty[i].DiffuseColor =   u_LightProperty[i].DiffuseColor;
        v_LightProperty[i].SpecularColor =  u_LightProperty[i].SpecularColor;
        v_LightProperty[i].Cutoff =         u_LightProperty[i].Cutoff;
        v_LightProperty[i].Power =          u_LightProperty[i].Power;
        v_LightProperty[i].Type =           u_LightProperty[i].Type;
        v_LightProperty[i].Direction =      v_viewMatrix * u_LightProperty[i].Direction;
        v_LightProperty[i].Position =       v_viewMatrix * u_LightProperty[i].Position;
    }

    highp float shadowCoef = 1.0f;
    if(u_IsDrawShadow)
    {
        //if(v_LightProperty[u_IndexLightShadow].Type == 0) // Directional
        {
            shadowCoef = CalcShadowAmount(u_ShadowMap);
            shadowCoef += 0.1f; // избавляемся от абсолютной черноты тени
            if(shadowCoef > 1.0f) shadowCoef = 1.0f;
        }
    }

    vec4 eyePosition = vec4(0.0f, 0.0f, 0.0f, 1.0f); // позиция наблюдателя
    vec3 eyeVec = normalize(v_position.xyz - eyePosition.xyz); // направление взгляда
    vec3 usingNormal = v_normal; // используемая нормаль
    vec4 resultColor = vec4(0.0f, 0.0f, 0.0f, 0.0f); // результирующий цвет чёрный
    vec4 diffMatColor = texture2D(u_DiffuseMap, v_textcoord); // диффузный цвет

    if(u_IsUseNormalMap) usingNormal = normalize(texture2D(u_NormalMap, v_textcoord).rgb * 2.0f - 1.0f);
    if(u_IsUseNormalMap) eyeVec = normalize(v_tbnMatrix * eyeVec);

    for(int i = 0; i < countLights; i++)
    {
        vec3 lightVec = vec3(0.0f, 0.0f, 0.0f); // вектор освещения
        vec4 resultLightColor = vec4(0.0f, 0.0f, 0.0f, 0.0f); // результирующий цвет освещения

        if(v_LightProperty[i].Type == 0) // Directional
            lightVec = normalize(v_LightProperty[i].Direction.xyz);
        else // Point, Spot
        {
            lightVec = normalize(v_position - v_LightProperty[i].Position).xyz;
            if(v_LightProperty[i].Type == 2) // Spot
            {
                float angle = acos(dot(v_LightProperty[i].Direction.xyz, lightVec));
                if(angle > v_LightProperty[i].Cutoff) lightVec = vec3(0.0f, 0.0f, 0.0f);
            }
        }

        if(u_IsUseNormalMap) lightVec = normalize(v_tbnMatrix * lightVec);

        vec3 reflectLight = normalize(reflect(lightVec, usingNormal)); // отражённый свет
        float len = length(v_position.xyz - eyePosition.xyz); // расстояние от наблюдателя до точки
        float specularFactor = u_MaterialProperty.Shines; // размер пятна блика
        float ambientFactor = 0.1f; // светимость материала
        vec4 reflectionColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); //цвет блика белый

        if(u_IsUseDiffuseMap == false) diffMatColor = vec4(u_MaterialProperty.DiffuseColor, 1.0f);

        vec4 diffColor = diffMatColor * v_LightProperty[i].Power * max(0.0f, dot(usingNormal, -lightVec));
        resultLightColor += diffColor * vec4(v_LightProperty[i].DiffuseColor, 1.0f);

        vec4 ambientColor = ambientFactor * diffMatColor;
        resultLightColor += ambientColor * vec4(u_MaterialProperty.AmbienceColor, 1.0f) * vec4(v_LightProperty[i].AmbienceColor, 1.0f);

        vec4 specularColor = reflectionColor * v_LightProperty[i].Power * pow(max(0.0f, dot(reflectLight, -eyeVec)), specularFactor);
        resultLightColor += specularColor * vec4(u_MaterialProperty.SpecularColor, 1.0f) * vec4(v_LightProperty[i].SpecularColor, 1.0f);

        resultColor += resultLightColor;
    }

    gl_FragColor = resultColor * shadowCoef;
}
