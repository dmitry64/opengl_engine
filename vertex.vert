#version 130

in vec3 vertexPosition_modelspace;
in vec2 vertexUV;
in vec3 vertexNormal_modelspace;
in vec3 vertexAmbientColor;
in vec3 vertexDiffuseColor;
in vec3 vertexSpecularColor;
in float opacity;

out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out vec3 vertexAmbientColor_out;
out vec3 vertexDiffuseColor_out;
out vec3 vertexSpecularColor_out;
out float opacity_out;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;

void main(){
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;
	vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
	Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz;
	vertexAmbientColor_out = vertexAmbientColor;
	vertexDiffuseColor_out = vertexDiffuseColor;
	vertexSpecularColor_out = vertexSpecularColor;
	UV = vertexUV;
	opacity_out = opacity;

}
