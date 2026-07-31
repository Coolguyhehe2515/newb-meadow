#ifndef INSTANCING
$input v_texcoord0, v_posTime
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include <newb/main.sh>

  SAMPLER2D_AUTOREG(s_SkyTexture);
#endif

void main() {
  #ifndef INSTANCING
    vec3 skyDir = normalize(v_posTime.xyz);
    float ax = abs(skyDir.x);
    float ay = abs(skyDir.y);
    float az = abs(skyDir.z);
    float col;
    float row;
    float u;
    float v;
    if (ax >= ay && ax >= az) {
      if (skyDir.x > 0.0) {
        u = -skyDir.z/ax; v = -skyDir.y/ax; col = 2.0; row = 1.0;
      } else {
        u = skyDir.z/ax; v = -skyDir.y/ax; col = 0.0; row = 1.0;
      }
    } else if (ay >= ax && ay >= az) {
      if (skyDir.y > 0.0) {
        u = skyDir.x/ay; v = skyDir.z/ay; col = 1.0; row = 0.0;
      } else {
        u = skyDir.x/ay; v = -skyDir.z/ay; col = 1.0; row = 2.0;
      }
    } else {
      if (skyDir.z > 0.0) {
        u = -skyDir.x/az; v = -skyDir.y/az; col = 3.0; row = 1.0;
      } else {
        u = skyDir.x/az; v = -skyDir.y/az; col = 1.0; row = 1.0;
      }
    }
    u = u*0.5+0.5;
    v = v*0.5+0.5;
    vec2 atlasUV = vec2((col+u)/4.0, (row+v)/3.0);
    vec4 diffuse = texture2D(s_SkyTexture, atlasUV);

    vec3 color = renderEndSky(getEndHorizonCol(), getEndZenithCol(), normalize(v_posTime.xyz), v_posTime.w);
    color += 2.8*diffuse.rgb; // stars

    color = colorCorrection(color);

    gl_FragColor = vec4(color, 1.0);
  #else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
