#version 400

uniform vec3 planetColor;

uniform vec3 La = vec3(0.8);		//Ambient light intensity
uniform	vec3 Ld = vec3(0.8);		//Diffuse light intensity
uniform	vec3 Ls = vec3(0.8);		//Specular light intensity


uniform vec3 Ra = vec3(0.5);			//Ambient reflectivity
uniform	vec3 Rd = vec3(0.5);			//Diffuse reflectivity
uniform	vec3 Rs = vec3(0.5);			//Specular reflectivity
uniform	float Shininess = 0.5;	        //Specular shininess factor


in VS_OUT {
    vec3 norm_eye;
    vec3 light_eye;
    vec3 view_eye;
};

out vec4 FragColor;


void main()
{
    //Calculate color with Phong Model
	vec3 ambient, diffuse, spec;

	vec3 n = normalize( norm_eye );
	vec3 s = normalize( light_eye);
	vec3 v = normalize( -view_eye );
	vec3 r = reflect( -s, n );
	ambient = La * Ra;
	float sDotN = max( dot( s, n ), 0.0 );
	diffuse = Ld * Rd * sDotN;
	spec = Ls * Rs * pow( max( dot(r,v) , 0.0 ), Shininess );

	FragColor = vec4( vec3(ambient + diffuse), 1 ) * vec4(planetColor, 1) + vec4( spec, 1 );


}

