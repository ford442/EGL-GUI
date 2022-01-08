#version 300 es
in vec2 vertPos;out vec4 colour;const vec4 white=vec4(1.0);const vec4 funkyColour=vec4(0.33);void main(){colour=white;gl_Position=vec4(vertPos, 0.0, 1.0);}
