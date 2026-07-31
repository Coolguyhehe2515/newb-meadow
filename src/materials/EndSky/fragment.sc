#ifndef INSTANCING
$input v_texcoord0, v_posTime, v_camPos
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include <newb/main.sh>

  SAMPLER2D_AUTOREG(s_SkyTexture);
  SAMPLER2D_AUTOREG(s_NoiseTex);
#endif

void main() {
  #ifndef INSTANCING
    vec4 diffuse = texture2D(s_SkyTexture, v_texcoord0);

    vec3 color = renderEndSky(getEndHorizonCol(), getEndZenithCol(), normalize(v_posTime.xyz), v_posTime.w);
    color += 2.8*diffuse.rgb; // stars

    #ifdef NL_BLACKHOLE
      float islandDist = length(v_camPos.xz);
      float islandFade = 1.0 - smoothstep(NL_BLACKHOLE_ISLAND_RADIUS*0.6, NL_BLACKHOLE_ISLAND_RADIUS, islandDist);
      if (islandFade > 0.001) {
        color = mix(color, nlRenderBlackHole(color, normalize(v_posTime.xyz), v_posTime.w, s_NoiseTex), islandFade);
      }
    #endif

    color = colorCorrection(color);

    gl_FragColor = vec4(color, 1.0);
  #else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
