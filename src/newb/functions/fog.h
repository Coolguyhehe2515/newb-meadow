#ifndef FOG_H
#define FOG_H

float nlRenderFogFade(float relativeDist, vec3 FOG_COLOR, vec2 FOG_CONTROL) {
  #ifdef NL_FOG
    float fade = smoothstep(FOG_CONTROL.x, FOG_CONTROL.y, relativeDist);

    // misty effect
    float density = NL_MIST_DENSITY*(19.0 - 18.0*FOG_COLOR.g);
    fade += (1.0-fade)*(0.3-0.3*exp(-relativeDist*relativeDist*density));

    return NL_FOG * fade;
  #else
    return 0.0;
  #endif
}

float nlRenderGodRayIntensity(vec3 cPos, vec3 worldPos, float t, vec2 uv1, float relativeDist, nl_environment env) {
  vec3 offset = cPos - 16.0*fract(worldPos*0.0625);
  offset = abs(2.0*fract(offset*0.0625)-1.0);
  offset = offset*offset*(3.0-2.0*offset);

  float dayMix = smoothstep(-0.05, 0.05, env.dayFactor);
  vec3 lightDir = mix(env.moonDir, env.sunDir, dayMix);

  vec3 lightRight = normalize(cross(lightDir, vec3(0.0, 1.0, 0.0)));
  vec3 lightUp = cross(lightRight, lightDir);
  vec3 alignedPos = vec3(dot(worldPos, lightRight), dot(worldPos, lightUp), dot(worldPos, lightDir));
  vec3 nrmof = normalize(alignedPos);

  float u = nrmof.z/length(nrmof.zy);
  float diff = dot(offset,vec3(0.1,0.2,1.0)) + 0.07*t;
  float mask = nrmof.x*nrmof.x;

  float vol = sin(7.0*u + 1.5*diff)*sin(3.0*u + diff);
  vol *= vol*mask*uv1.y*(1.0-mask*mask);
  vol *= relativeDist*relativeDist;

  vol *= smoothstep(0.0, 0.12, lightDir.y);

  vol = smoothstep(0.0, 0.1, vol);
  return vol;
}

float nlRenderUnderwaterGodRay(vec3 cPos, vec3 worldPos, float t, vec2 uv1, vec3 sunDir) {
  vec3 offset = cPos - 16.0*fract(worldPos*0.0625);
  offset = abs(2.0*fract(offset*0.0625)-1.0);
  offset = offset*offset*(3.0-2.0*offset);

  vec3 sunRight = normalize(cross(sunDir, vec3(0.0, 1.0, 0.0)));
  vec3 sunUp = cross(sunRight, sunDir);
  vec3 alignedPos = vec3(dot(worldPos, sunRight), dot(worldPos, sunUp), dot(worldPos, sunDir));
  vec3 nrmof = normalize(alignedPos);

  float u = nrmof.z/length(nrmof.zy);
  float diff = dot(offset,vec3(0.1,0.2,1.0)) + 0.07*t;
  float mask = nrmof.x*nrmof.x;

  float vol = sin(7.0*u + 1.5*diff)*sin(3.0*u + diff);
  vol *= vol*mask*uv1.y*(1.0-mask*mask);

  vol = smoothstep(0.0, 0.1, vol);
  return vol;
}

#endif
