#version 400


layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 norm_eye;
    vec3 light_eye;
    vec3 view_eye;
} gs_in[];


out VS_OUT {
    vec3 norm_eye;
    vec3 light_eye;
    vec3 view_eye;
};



void main()
{

    for (int i = 0; i < 3; ++i) {
        norm_eye    = gs_in[i].norm_eye;
        light_eye   = gs_in[i].light_eye;
        view_eye    = gs_in[i].view_eye;

        gl_Position = gl_in[i].gl_Position;
        gl_PrimitiveID = gl_PrimitiveIDIn;
        EmitVertex();
    }


}



