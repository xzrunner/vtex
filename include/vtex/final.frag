static const char* final_frag = R"(

uniform sampler2D u_page_table_tex;
uniform sampler2D u_texture_atlas_tex;

uniform vec2 u_page_table_size;
uniform vec2 u_virt_tex_size;

uniform float u_atlas_scale;		// This value is used to scale the uv to the texture atlas. It holds (PageSize/TextureAtlasSize)

uniform float u_border_scale;		// (PageSize-2*BorderSize)/PageSize
uniform float u_border_offset;		// BorderSize/PageSize

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

// This function samples the page table and returns the page's
// position and mip level.
vec3 sample_table(vec2 uv, float mip)
{
	vec2 offset = fract(uv * u_page_table_size) / u_page_table_size;
	return texture2D(u_page_table_tex, uv - offset, mip).xyz;
}

// This functions samples from the texture atlas and returns the final color
vec4 sample_atlas(vec3 page, vec2 uv)
{
	float mipsize = exp2(floor(page.z * 255.0 + 0.5));

	uv = fract(uv * u_page_table_size / mipsize);

	uv *= u_border_scale;
	uv += u_border_offset;
	vec2 offset = floor(page.xy * 255 + 0.5);

	return texture2D(u_texture_atlas_tex, (offset + uv) * u_atlas_scale);
}

vec4 bilinear_sample()
{
	float mip = floor(tex_mip_level(v_texcoord, u_virt_tex_size));
	mip = clamp(mip, 0, log2(u_page_table_size.x));

	vec3 page = sample_table(v_texcoord, mip);
	return sample_atlas(page, v_texcoord);
}

vec4 trilinear_sample()
{
	float miplevel = tex_mip_level(v_texcoord, u_virt_tex_size);
	miplevel = clamp(miplevel, 0, log2(u_page_table_size.x) - 1);

	float mip1    = floor(miplevel);
	float mip2	  = mip1 + 1;
	float mipfrac = miplevel - mip1;

	vec3 page1 = sample_table(v_texcoord, mip1);
	vec3 page2 = sample_table(v_texcoord, mip2);

	vec4 sample1 = sample_atlas(page1, v_texcoord);
	vec4 sample2 = sample_atlas(page2, v_texcoord);

	return mix(sample1, sample2, mipfrac);
}

void main()
{
	gl_FragColor = bilinear_sample();
}

)";