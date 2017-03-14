#version 120

uniform sampler2DRect tex;

void main() {
    vec4 tex_color = texture2DRect(tex, gl_FragCoord.xy);
    gl_FragColor = tex_color;
}
