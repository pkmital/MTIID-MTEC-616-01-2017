#version 120

uniform sampler2DRect tex;

varying vec2 texCoord;

void main() {
    vec4 tex_color = texture2DRect(tex, texCoord.xy);
    gl_FragColor = tex_color;
}
