#version 430

out vec4 daColor;
in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;

uniform bool normalMapping_flag;

uniform sampler2D myTextureSampler0;
uniform sampler2D myTextureSampler1;
uniform vec4 shininess;
uniform float darken_scene;
uniform int realColor;

uniform vec4 emissionLight;
uniform vec4 ambientLight;
uniform vec3 eyePositionWorld;

uniform vec3 lightPositionWorld;		//Sun
uniform vec4 sunColor;

uniform vec3 lightPositionWorldMoon;	//Moon
uniform vec4 moonColor;


struct MultiLight
{
    vec3 position;
    vec3 color;
};
#define LIGHT_NUM 12
uniform MultiLight littleLight[LIGHT_NUM];


void main()
{
	vec3 normal = normalize(normalWorld);
	if (normalMapping_flag)
	{
		//Obtain normal from normal map in range [0,1]
		normal = texture(myTextureSampler1, UV).rgb;
		//Transform normal vector to range [-1,1]
		normal = normalize(normal *	2.0 - 1.0);
	}

	vec4 MaterialAmbientColor = 1.5f * texture(myTextureSampler0, UV).rgba;	//Increase Contrasts
	vec4 MaterialDiffuseColor = 1.5f * texture(myTextureSampler0, UV).rgba;
	vec4 MaterialSpecularColor = shininess;
	
	//Sun

	//Diffuse
	vec3 lightVectorWorld = normalize(lightPositionWorld + vec3(-1.1f, 0.7f, 0.0f));									//For sun, no need to minus vertexPositionWorld
	float brightness = dot(lightVectorWorld, normalize(normalWorld));
	vec4 diffuseLight = vec4(brightness, brightness, brightness, 1.0);

	//Specular
	vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld, normalWorld);
	vec3 eyeVectorWorld = normalize(eyePositionWorld - vertexPositionWorld);
	float s = clamp(dot(reflectedLightVectorWorld, eyeVectorWorld), 0 , 1);
	s = pow(s, 50);
	vec4 specularLight = vec4(s, s, s, 1);



	//Moon

	//Diffuse
	vec3 lightVectorWorldMoon = normalize(lightPositionWorldMoon + vec3(-1.7f, 0.7f, 0.0f));							//For moon, no need to minus vertexPositionWorld
	float brightnessMoon = dot(lightVectorWorldMoon, normalize(normalWorld));
	vec4 diffuseLightMoon = vec4(brightnessMoon, brightnessMoon, brightnessMoon, 1.0);

	//Specular
	vec3 reflectedLightVectorWorldMoon = reflect(-lightVectorWorldMoon, normalWorld);
	float s2 =clamp(dot(reflectedLightVectorWorldMoon, eyeVectorWorld), 0, 1);
	s2 = pow(s2, 50);
	vec4 specularLightMoon = vec4(s2, s2, s2, 1);

	
	//Sample
	/*
	//Diffuse
	vec3 lightVectorWorldMoon = normalize(lightPositionWorldMoon - vertexPositionWorld);
	float brightnessMoon = dot(lightVectorWorldMoon, normalize(normalWorld));
	vec4 diffuseLightMoon = vec4(brightnessMoon, brightnessMoon, brightnessMoon, 1.0);

	//Specular
	vec3 reflectedLightVectorWorldMoon = reflect(-lightVectorWorldMoon, normalWorld);
	float s2 =clamp(dot(reflectedLightVectorWorldMoon, eyeVectorWorld), 0, 1);
	s2 = pow(s2, 50);
	vec4 specularLightMoon = vec4(s2, s2, s2, 1);
	*/

		/*
			vec4 multi_spot_light;
			for(int i = 0; i < SPOTLIGHT_NUM; i++)
			{
				vec3 lightVectorWorldSpotLight = normalize(spotLight[i].position - vertexPositionWorld);
				float brightness2 = dot(lightVectorWorld2, normalize(normalWorld));
				vec4 diffuseLight2 = vec4(brightness2, brightness2, brightness2, 1.0) * vec4(littleLight[i].color, 1.0f);

				vec3 reflectedLightVectorWorld2 = reflect(-lightVectorWorld2, normalWorld);
				vec3 eyeVectorWorld2 = normalize(eyePositionWorld - vertexPositionWorld);
				float s2 =clamp(dot(reflectedLightVectorWorld2, eyeVectorWorld2), 0, 1);
				s2 = pow(s2, 50);
				vec4 specularLight2 = vec4(s2, s2, s2, 1)  * vec4(littleLight[i].color, 1.0f);
				multi_result += diffuseLight2 * 0.6f + specularLight2 * 0.3f;
			}
		*/






		/*
			daColor =
				MaterialDiffuseColor * clamp(diffuseLight, 0, 1) * 1.5f+
				specularLight +
				MaterialAmbientColor * 0.2f +
				MaterialAmbientColor * ambientLight + 
				MaterialDiffuseColor * clamp(diffuseLight, 0, 1) * 0.5f+
				specularLight * MaterialAmbientColor *0.5f;	// + multi_spot_light
		*/

	if (realColor == 1)
		daColor = texture(myTextureSampler0, UV).rgba * 0.8f;
	else if (realColor == 2)
		daColor = texture(myTextureSampler0, UV).rgba * 0.7f * darken_scene;
	else {
		daColor =	emissionLight * 3.2f																														//The Light itself
					+ ambientLight * MaterialAmbientColor * clamp(sunColor + vec4(0.3f, 0.3f, 0.3f, 0.0f), vec4(0.3f, 0.3f, 0.3f, 1), vec4(1, 1, 1, 1)) * 0.22f	//Ambient strengthened when daytime
					+ MaterialDiffuseColor * clamp(diffuseLight, 0 , 1) * sunColor * 1.2f + specularLight * MaterialSpecularColor * sunColor					//Sunlight
					//+ MaterialDiffuseColor * clamp(diffuseLightMoon, 0 , 1) * moonColor * 1.2f + specularLightMoon * MaterialSpecularColor * moonColor			//Moonlight
					;
		daColor *= darken_scene;
	}
}
