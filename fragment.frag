#version 130
 
precision highp float;

in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in vec3 vertexAmbientColor_out;
in vec3 vertexDiffuseColor_out;
in vec3 vertexSpecularColor_out;
in float opacity_out;

out vec4 color;

uniform vec3 LightPosition_worldspace;
uniform sampler2D texture_sample;
uniform int hasTexture;

void main(){
	vec3 LightColor = vec3(1,1,1);
	float LightPower = 9000.0f;
	vec3 MaterialDiffuseColor;

	if(hasTexture==1)
		MaterialDiffuseColor = texture(texture_sample,UV).rgb*vertexDiffuseColor_out;
	else
		MaterialDiffuseColor = vertexDiffuseColor_out;

	vec3 MaterialAmbientColor = MaterialDiffuseColor*vertexAmbientColor_out;
	vec3 MaterialSpecularColor = MaterialDiffuseColor*vertexSpecularColor_out;

	float distance = length( LightPosition_worldspace - Position_worldspace );
	vec3 n = normalize( Normal_cameraspace );
	vec3 l = normalize( LightDirection_cameraspace );
	vec3 Idiff = MaterialDiffuseColor * max(dot(n,l), 0.0);
    Idiff = clamp(Idiff, 0.0, 1.0);

	float cosTheta = clamp( dot( n,l ), 0,1 );
	vec3 E = normalize(EyeDirection_cameraspace);
	vec3 R = reflect(-l,n);
	float cosAlpha = clamp( dot( E,R ), 0.0,1.0 );

	vec3 base_Color =
		MaterialAmbientColor +
		MaterialDiffuseColor * LightColor * LightPower * (cosTheta/ (distance*distance)) +
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
	color = vec4(base_Color.xyz,opacity_out);
	//color.a = 0.3f;
}