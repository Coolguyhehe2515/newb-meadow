#ifndef CLOUDS_H
#define CLOUDS_H

#include "detection.h"
#include "noise.h"
#include "sky.h"

// simple clouds 2D noise
float cloudNoise2D(vec2 p, highp float t, float rain) {
  t *= NL_CLOUD1_SPEED;
  p += t;
  p.y += 3.0*sin(0.3*p.x + 0.1*t);

  vec2 p0 = floor(p);
  vec2 u = p-p0;
  u *= u*(3.0-2.0*u);
  vec2 v = 1.0-u;

  float n = mix(
    mix(rand(p0),rand(p0+vec2(1.0,0.0)), u.x),
    mix(rand(p0+vec2(0.0,1.0)),rand(p0+vec2(1.0,1.0)), u.x),
    u.y
  );
  n *= 0.5 + 0.5*sin(p.x*0.6 - 0.5*t)*sin(p.y*0.6 + 0.8*t);
  n = min(n*(1.0+rain), 1.0);
  return n*n;
}

// simple clouds
vec4 renderCloudsSimple(nl_skycolor skycol, vec3 pos, highp float t, float rain) {
  pos.xz *= NL_CLOUD1_SCALE;
  float d = cloudNoise2D(pos.xz, t, rain);
  vec4 col = vec4(skycol.horizonEdge + skycol.zenith, smoothstep(0.1,0.6,d));
  col.rgb += 1.5*dot(col.rgb, vec3(0.3,0.4,0.3))*smoothstep(0.6,0.2,d)*col.a;
  col.rgb *= 1.0 - 0.8*rain;
  return col;
}

// rounded clouds

// rounded clouds 3D density map
float cloudDf(vec3 pos, float rain, vec2 boxiness) {
  boxiness *= 0.999;
  vec2 p0 = floor(pos.xz);
  vec2 u = max((pos.xz-p0-boxiness.x)/(1.0-boxiness.x), 0.0);
  u *= u*(3.0 - 2.0*u);

  vec4 r = vec4(rand(p0), rand(p0+vec2(1.0,0.0)), rand(p0+vec2(1.0,1.0)), rand(p0+vec2(0.0,1.0)));
  r = smoothstep(0.1001+0.2*rain, 0.1+0.2*rain*rain, r); // rain transition

  float n = mix(mix(r.x,r.y,u.x), mix(r.w,r.z,u.x), u.y);

  // round y
  n *= 1.0 - 1.5*smoothstep(boxiness.y, 2.0 - boxiness.y, 2.0*abs(pos.y-0.5));

  n = max(1.25*(n-0.2), 0.0); // smoothstep(0.2, 1.0, n)
  n *= n*(3.0 - 2.0*n);
  return n;
}

vec4 renderCloudsRounded(
    vec3 vDir, vec3 vPos, float rain, float time, vec3 horizonCol, vec3 zenithCol,
    const int steps, const float thickness, const float thickness_rain, const float speed,
    const vec2 scale, const float density, const vec2 boxiness
) {
  float height = 7.0*mix(thickness, thickness_rain, rain);
  float stepsf = float(steps);

  // scaled ray offset
  vec3 deltaP;
  deltaP.y = 1.0;
  deltaP.xz = height*scale*vDir.xz/(0.02+0.98*abs(vDir.y));

  // local cloud pos
  vec3 pos;
  pos.y = 0.0;
  pos.xz = scale*(vPos.xz + vec2(1.0,0.5)*(time*speed));
  pos += deltaP;

  deltaP /= -stepsf;

  // alpha, gradient
  vec2 d = vec2(0.0,1.0);
  for (int i=1; i<=steps; i++) {
    float m = cloudDf(pos, rain, boxiness);
    d.x += m;
    d.y = mix(d.y, pos.y, m);
    pos += deltaP;
  }
  d.x *= smoothstep(0.03, 0.1, d.x);
  d.x /= (stepsf/density) + d.x;

  if (vPos.y < 0.0) { // view from top
    d.y = 1.0 - d.y;
  }

  vec4 col = vec4(zenithCol + horizonCol, d.x);
  col.rgb += dot(col.rgb, vec3(0.3,0.4,0.3))*d.y*d.y;
  col.rgb *= 1.0 - 0.8*rain;
  return col;
}

float cloudsNoiseVr(vec2 p, float t) {
  float n = fastVoronoi2(p + t, 1.8);
  n *= fastVoronoi2(3.0*p + t, 1.5);
  n *= fastVoronoi2(9.0*p + t, 0.4);
  n *= fastVoronoi2(27.0*p + t, 0.1);
  //n *= fastVoronoi2(82.0*pos + t, 0.02); // more quality
  return n*n;
}

vec4 renderClouds(vec2 p, float t, float rain, vec3 horizonCol, vec3 zenithCol, const vec2 scale, const float velocity, const float shadow) {
  p *= scale;
  t *= velocity;

  // layer 1
  float a = cloudsNoiseVr(p, t);
  float b = cloudsNoiseVr(p + NL_CLOUD3_SHADOW_OFFSET*scale, t);

  // layer 2
  p = 1.4 * p.yx + vec2(7.8, 9.2);
  t *= 0.5;
  float c = cloudsNoiseVr(p, t);
  float d = cloudsNoiseVr(p + NL_CLOUD3_SHADOW_OFFSET*scale, t);

  // higher = less clouds thickness
  // lower separation betwen x & y = sharper
  vec2 tr = vec2(0.35, 0.85) - 0.12*rain;
  a = smoothstep(tr.x, tr.y, a);
  c = smoothstep(tr.x, tr.y, c);

  // shadow
  b *= smoothstep(0.2, 0.8, b);
  d *= smoothstep(0.2, 0.8, d);

  vec4 col;
  col.a = a + c*(1.0-a);
  col.rgb = horizonCol + horizonCol.ggg;
  col.rgb = mix(col.rgb, 0.5*(zenithCol + zenithCol.ggg), shadow*mix(b, d, c));
  col.rgb *= 1.0-0.7*rain;

  return col;
}

highp float nlCloud4Random(highp vec2 p) {
  highp vec3 p3 = fract(vec3(p.xyx) * 0.1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

highp float nlCloud4Noise(highp vec2 uv) {
  highp vec2 i = floor(uv);
  highp vec2 f = fract(uv);
  f = f * f * (3.0 - 2.0 * f);
  return mix(
    mix(nlCloud4Random(i),                nlCloud4Random(i + vec2(1.0, 0.0)), f.x),
    mix(nlCloud4Random(i + vec2(0.0,1.0)), nlCloud4Random(i + vec2(1.0, 1.0)), f.x),
    f.y
  );
}

highp float nlCloud4Fbm(highp vec2 p) {
  highp float v = 0.0;
  mat2 rot = mat2(0.8776, 0.4794, -0.4794, 0.8776);
  v += 0.570 * nlCloud4Noise(p);
  p = mul(rot, p) * 2.1 + vec2(100.0);
  v += 0.285 * nlCloud4Noise(p);
  return v * 1.17;
}

vec2 nlCloud4Sample(vec2 uv, float t) {
  vec2 offset = vec2(t * NL_CLOUD4_SPEED, t * NL_CLOUD4_SPEED * 0.6);
  float raw = nlCloud4Fbm(uv * NL_CLOUD4_SCALE + offset);
  float density = smoothstep(NL_CLOUD4_THRESHOLD, NL_CLOUD4_THRESHOLD + NL_CLOUD4_SOFTNESS, raw);
  return vec2(density, raw);
}

vec3 nlCloud4Shade(float lightFactor, float rawNoise) {
  float innerShadow = 1.0 - clamp((rawNoise - NL_CLOUD4_THRESHOLD) * 2.0, 0.0, 1.0) * NL_CLOUD4_SHADOW_STRENGTH;
  vec3 bottom = mix(NL_CLOUD4_SHADE_COLOR, NL_CLOUD4_AMBIENT_COLOR, 0.5);
  vec3 top = mix(NL_CLOUD4_AMBIENT_COLOR, NL_CLOUD4_SUN_COLOR, lightFactor);
  return mix(bottom, top, innerShadow);
}

vec4 renderCloudsLayered(vec3 vDir, float t, float rain) {
  float hfade = smoothstep(0.03, 0.28, vDir.y);
  if (hfade < 0.001) {
    return vec4_splat(0.0);
  }

  float safeY = max(vDir.y, 0.001);
  float totalAlpha = 0.0;
  vec3 totalColor = vec3(0.0, 0.0, 0.0);
  float transmit = 1.0;

  float layerAlpha, contrib;
  vec2 layerUV, s;
  vec3 cloudColor;

  layerUV = vDir.xz * (0.18 / safeY);
  s = nlCloud4Sample(layerUV, t);
  cloudColor = nlCloud4Shade(0.0, s.y);
  layerAlpha = clamp(s.x * NL_CLOUD4_DENSITY * hfade, 0.0, 1.0);
  contrib = layerAlpha * transmit;
  totalColor += cloudColor * contrib;
  totalAlpha += contrib;
  transmit *= (1.0 - layerAlpha * 0.85);

  if (transmit >= 0.02) {
    layerUV = vDir.xz * ((0.18 + NL_CLOUD4_LAYER_GAP) / safeY);
    s = nlCloud4Sample(layerUV, t);
    cloudColor = nlCloud4Shade(0.33, s.y);
    layerAlpha = clamp(s.x * NL_CLOUD4_DENSITY * hfade, 0.0, 1.0);
    contrib = layerAlpha * transmit;
    totalColor += cloudColor * contrib;
    totalAlpha += contrib;
    transmit *= (1.0 - layerAlpha * 0.85);
  }

  if (transmit >= 0.02) {
    layerUV = vDir.xz * ((0.18 + NL_CLOUD4_LAYER_GAP * 2.0) / safeY);
    s = nlCloud4Sample(layerUV, t);
    cloudColor = nlCloud4Shade(0.66, s.y);
    layerAlpha = clamp(s.x * NL_CLOUD4_DENSITY * hfade, 0.0, 1.0);
    contrib = layerAlpha * transmit;
    totalColor += cloudColor * contrib;
    totalAlpha += contrib;
    transmit *= (1.0 - layerAlpha * 0.85);
  }

  if (transmit >= 0.02) {
    layerUV = vDir.xz * ((0.18 + NL_CLOUD4_LAYER_GAP * 3.0) / safeY);
    s = nlCloud4Sample(layerUV, t);
    cloudColor = nlCloud4Shade(1.0, s.y);
    layerAlpha = clamp(s.x * NL_CLOUD4_DENSITY * hfade, 0.0, 1.0);
    contrib = layerAlpha * transmit;
    totalColor += cloudColor * contrib;
    totalAlpha += contrib;
  }

  totalAlpha = clamp(totalAlpha, 0.0, 1.0);
  vec3 finalColor = totalAlpha > 0.0 ? totalColor / totalAlpha : NL_CLOUD4_SUN_COLOR;
  finalColor *= 1.0 - 0.7*rain;
  return vec4(finalColor, totalAlpha);
}

// aurora is rendered on clouds layer
#ifdef NL_AURORA
vec4 renderAurora(vec3 p, float t, float rain, vec3 FOG_COLOR) {
  t *= NL_AURORA_VELOCITY;
  p.xz *= NL_AURORA_SCALE;
  p.xz += 0.05*sin(p.x*4.0 + 20.0*t);

  float d0 = sin(p.x*0.1 + t + sin(p.z*0.2));
  float d1 = sin(p.z*0.1 - t + sin(p.x*0.2));
  float d2 = sin(p.z*0.1 + 1.0*sin(d0 + d1*2.0) + d1*2.0 + d0*1.0);
  d0 *= d0; d1 *= d1; d2 *= d2;
  d2 = d0/(1.0 + d2/NL_AURORA_WIDTH);

  float mask = (1.0-0.8*rain)*max(1.0 - 4.0*max(FOG_COLOR.b, FOG_COLOR.g), 0.0);
  return vec4(NL_AURORA*mix(NL_AURORA_COL1,NL_AURORA_COL2,d1),1.0)*d2*mask;
}
#endif

vec4 nlCloudAuroraReflection(nl_skycolor skycol, nl_environment env, vec3 viewDir, vec3 wPos, vec3 CAMERA_POS, highp float t) {
  vec2 cloudPos = wPos.xz;
  cloudPos += (187.0-(wPos.y+CAMERA_POS.y))*viewDir.xz/viewDir.y;
  float fade = clamp(2.0 - 0.005*length(cloudPos), 0.0, 1.0);
  cloudPos += CAMERA_POS.xz;

  vec4 refl = vec4_splat(0.0);

  #ifdef NL_AURORA
    vec4 aurora = renderAurora(cloudPos.xyy, t, env.rainFactor, env.fogCol);
    aurora.a *= fade;
    refl = vec4(2.0*aurora.rgb*aurora.a, aurora.a);
  #endif

  #if NL_CLOUD_TYPE == 1
    vec4 clouds = renderCloudsSimple(skycol, cloudPos.xyy, t, env.rainFactor);
    clouds.a *= fade;
    refl = vec4(mix(refl.rgb, clouds.rgb, clouds.a), min(refl.a + clouds.a, 1.0));
  #endif

  return refl;
}

#endif
