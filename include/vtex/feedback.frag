static const char* feedback_frag = R"(

uniform vec2 u_page_table_size;
uniform vec2 u_virt_tex_size;
uniform float u_mip_sample_bias;

varying vec2 v_texcoord;

float tex_mip_level(vec2 coord, vec2 tex_size)
{
   vec2 dx_scaled, dy_scaled;
   vec2 coord_scaled = coord * tex_size;

   dx_scaled = dFdx(coord_scaled);
   dy_scaled = dFdy(coord_scaled);

   vec2 dtex = dx_scaled*dx_scaled + dy_scaled*dy_scaled;
   float min_delta = max(dtex.x,dtex.y);
   float miplevel = max(0.5 * log2(min_delta), 0.0);

   return miplevel;
}

void main()
{
	float mip = floor(tex_mip_level(v_texcoord, u_virt_tex_size) - u_mip_sample_bias);
	mip = clamp(mip, 0, log2(u_page_table_size.x));

	float times = u_page_table_size.x / exp2(mip);
	vec2 offset = floor(v_texcoord * times);
	offset = clamp(offset, vec2(0, 0), vec2(times - 1, times - 1));
	gl_FragColor = vec4(vec3(offset / 255.0, mip / 255.0), 1.0);
}

)";