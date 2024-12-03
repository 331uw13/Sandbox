#ifndef SANDBOX_SHADERS_H
#define SANDBOX_SHADERS_H


#define _STRL(s) #s
#define _MSTR(s) _STRL(s)


// SHADERS FOR PIXELS:
// 

static const char SANDBOX_PIXEL_VERTEX_SRC[] = 
GLSL_VERSION
"layout(location = 0) in vec2 i_pos;"
"layout(location = 1) in vec3 i_color;"
"out vec3 f_color;"

"void main() {"
    "gl_Position = vec4(i_pos.x, i_pos.y, 0.0, 1.0);"
    "f_color = i_color;"
"}"
;

static const char SANDBOX_PIXEL_FRAGMENT_SRC[] = 
GLSL_VERSION
"in vec3 f_color;"
"out vec4 out_color;"

"void main() {"
    "out_color = vec4(f_color, 1.0);"
"}"
;



// SHADERS FOR LIGHTS AND EFFECTS:
//

static const char SANDBOX_EFFECT_VERTEX_SRC[] = 
GLSL_VERSION
"layout(location = 0) in vec2 i_pos;"
"layout(location = 1) in vec2 i_texcoord;"
"out vec2 f_pos;"
"out vec2 f_texcoord;"

"void main() {"
    "gl_Position = vec4(i_pos.x, i_pos.y, 0.0, 1.0);"
    "f_pos = i_pos;"
    "f_texcoord = i_texcoord;"
"}"
;

// NOTE: this may need to be changed manually
//       if adding more variables for lights.
//       keep in mind the alignment
#define EFFECTSHADER_LIGHT_T_SIZEB 48
#define EFFECTSHADER_LIGHTS_BINDING_POINT 3

static const char SANDBOX_EFFECT_FRAGMENT_SRC[] = 
GLSL_VERSION
"in vec2 f_pos;"
"in vec2 f_texcoord;"
"out vec4 out_color;"

"uniform sampler2D tex;"

"struct light_t {"
    "vec4 pos;"
    "vec4 color;"
    "vec4 v;"
"};"

"\n"
"#define STRENGTH (lights[i].v.x)\n"
"#define RADIUS (lights[i].v.y)\n"
"#define EFFECT (lights[i].v.z)\n"
"\n"

"layout (std140, binding = " _MSTR(EFFECTSHADER_LIGHTS_BINDING_POINT) ")"
" uniform lights_uniform_block {"
    "light_t lights[" _MSTR(MAX_LIGHTS) "];"

"};"

"uniform int f_num_lights;"
"uniform float f_time;"


"\n#define PI2 6.28318\n"

"vec3 hash3(vec2 p) {"
    "vec3 q = vec3(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3)),dot(p,vec2(419.2,371.9)));"
    "return fract(sin(q)*43758.5453);"
"}"
"float voronoise(vec2 p) {"
    "vec2 n = floor(p);"
    "vec2 f = fract(p);"
    "vec3 md = vec3(16.0);"
    
    "for(int y = -1; y <= 1; y++) {" 
        "for(int x = -1; x <= 1; x++) {" 
            
            "vec2 g = vec2(x, y);"
            "vec2 o = hash3(n + g).xy;"

            "vec2 r = g + (0.5+0.5 * sin(f_time*1.635 + PI2 * o)) - f;"
            "float d = dot(r, r * 1.8);"
            "if(d < md.x) {"
                "md = vec3(d, o);"
            "}"
        
        "}"
    "}"
    "return md.x;"
"}"


"vec4 add_light(int i) {"
    "vec4 lcolor = vec4(0.0);"
    
    "float dist = length(lights[i].pos.xy - f_pos.xy);"
    "float l = dist * (100.0 - RADIUS);"

    "float strength = 12.0-(STRENGTH+10.0);"

    "l = pow(l, strength) * 0.3;"
    "l = 1.0 / l;"
    "lcolor.xyz = lights[i].color.xyz * l;"

    // this will be radius where the voronoise can take effect.
    // to be honest this is kind of cursed, but it works.
    "float d0 = 1.0-dist*(5.85/l);"
    "float r = clamp(d0, 0.0, 1.0);"

    "float vn = r*(voronoise((lights[i].pos.xy-f_pos.xy)*10.0));"
    "vn = pow(vn, 3.0) * EFFECT;"

    //"lcolor = vec3(r);"
    "lcolor.xyz += vec3(vn);"
    "lcolor.w += l;"

    "return lcolor;"
"}"

"void main() {"
    "out_color = texture(tex, f_texcoord);"
    "out_color.w = 0.0;"

    "for(int i = 0; i < f_num_lights; i++) {"
        "out_color += add_light(i);"
    "}"


    // gamma correction
    "float gamma = 2.0;"
    "out_color.xyz = pow(out_color.xyz, vec3(1.0/gamma));"
"}"
;




#endif
