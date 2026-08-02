$input v_color0, v_color1, v_fog, v_refl, v_texcoord0, v_lightmapUV, v_extra, v_position

#include <bgfx_shader.sh>
#include <newb/main.sh>

uniform vec4 ViewPositionAndTime;
uniform vec4 FogAndDistanceControl;

SAMPLER2D_AUTOREG(s_MatTexture);
SAMPLER2D_AUTOREG(s_SeasonsTexture);
SAMPLER2D_AUTOREG(s_LightMapTexture);

// raindrop screen overlay, credit: michaellynx
#define DROP_NUMBER 3

float stickyRaindrop(vec2 uv, vec2 center, float baseSize, float stretch) {
  vec2 p = (uv - center);

  p.y /= baseSize * stretch;
  p.x /= baseSize;

  float d = length(p * vec2(1.0, 0.6));  // horizontal squash
  float y = p.y;

  float shape = smoothstep(0.5, 0.45, d) *
                smoothstep(0.0, 1.0, y + 0.5) *
                smoothstep(1.0, -0.3, y);

  return shape;
}

vec3 RainDrop(vec4 diffuse, float time, vec2 uv) {
  vec3 baseColor = diffuse.rgb;
  vec3 kol = baseColor;
  const int drops = DROP_NUMBER;

  for (int i = 0; i < drops; i++) {
    float fi = float(i);

    vec2 dropPos = vec2(
      fract(sin(fi * 12.9898) * 43758.5453),
      fract(sin(fi * 78.233) * 12345.678 + time * 0.1)
    );
    dropPos.y = mod(dropPos.y - time * 0.12, 1.0); // falling

    float baseSize = 0.05 + 0.02 * fract(sin(fi * 5.21) * 1000.0);

    float stretch = mix(1.0, 2.5, smoothstep(0.1, 0.9, dropPos.y));

    float dropMask = stickyRaindrop(uv, dropPos, baseSize, stretch);

    vec3 dropColor = vec3(0.4, 0.6, 0.9);

    kol = mix(kol, dropColor, dropMask * 0.6);
  }
  return kol;
}

void main() {
  #if defined(DEPTH_ONLY_OPAQUE) || defined(DEPTH_ONLY) || defined(INSTANCING)
    gl_FragColor = vec4(1.0,1.0,1.0,1.0);
    return;
  #endif

  vec4 diffuse = texture2D(s_MatTexture, v_texcoord0);
  vec4 color = v_color0;

  #ifdef ALPHA_TEST
    if (diffuse.a < 0.6) {
      discard;
    }
  #endif

  #if defined(SEASONS) && (defined(OPAQUE) || defined(ALPHA_TEST))
    diffuse.rgb *= mix(vec3(1.0,1.0,1.0), texture2D(s_SeasonsTexture, v_color1.xy).rgb * 2.0, v_color1.z);
  #endif

  vec3 glow = nlGlow(s_MatTexture, v_texcoord0, v_extra.a);

  diffuse.rgb *= diffuse.rgb;

  #if defined(TRANSPARENT) && !(defined(SEASONS) || defined(RENDER_AS_BILLBOARDS))
    if (v_extra.b > 0.9) {
      diffuse.rgb = vec3_splat(1.0 - NL_WATER_TEX_OPACITY*(1.0 - diffuse.b*1.8));
      diffuse.a = color.a;
    }
  #else
    diffuse.a = 1.0;
  #endif

  diffuse.rgb *= color.rgb;
  diffuse.rgb += glow;

  if (v_extra.b > 0.9) {
    diffuse.rgb += v_refl.rgb*v_refl.a;
  } else if (v_refl.a > 0.0) {
    // reflective effect - only on xz plane
    float dy = abs(dFdy(v_extra.g));
    if (dy < 0.0002) {
      float mask = v_refl.a*(clamp(v_extra.r*10.0,8.2,8.8)-7.8);
      diffuse.rgb *= 1.0 - 0.6*mask;
      diffuse.rgb += v_refl.rgb*mask;
    }
  }

  diffuse.rgb = mix(diffuse.rgb, v_fog.rgb, v_fog.a);

  float time = ViewPositionAndTime.w;
  float rain = mix(smoothstep(0.66, 0.3, FogAndDistanceControl.x), 0.0, step(FogAndDistanceControl.x, 0.0));
  float wet = max(rain, v_position.x);

  vec2 dropUV = gl_FragCoord.xy / vec2(720.0, 1650.0);
  dropUV.x -= 0.5;

  diffuse.rgb += (RainDrop(diffuse, time, dropUV) - diffuse.rgb) * wet;

  diffuse.rgb = colorCorrection(diffuse.rgb);

  gl_FragColor = diffuse;
}
