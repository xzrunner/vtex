#include "vtex/VirtualTexture.h"
#include "vtex/feedback.frag"
#include "vtex/final.frag"

#include <unirender/ShaderProgram.h>
#include <unirender/Device.h>
#include <unirender/Context.h>
#include <unirender/Uniform.h>
#include <unirender/Texture.h>
#include <shadertrans/ShaderTrans.h>
#include <painting2/DebugDraw.h>
#include <painting3/Blackboard.h>
#include <painting3/WindowContext.h>

#include <algorithm>

namespace
{

const int ATLAS_TEX_SIZE = 4096;
const int UPLOADS_PER_FRAME = 5;

const int MIP_SAMPLE_BIAS = 3;

const char* default_vs = R"(

attribute vec4 position;
attribute vec3 normal;
attribute vec2 texcoord;

uniform mat4 u_projection;
uniform mat4 u_modelview;

varying vec2 v_texcoord;

void main()
{
	gl_Position = u_projection * u_modelview * position;
	v_texcoord = texcoord;
}

)";

}

namespace vtex
{

VirtualTexture::VirtualTexture(const ur::Device& dev,
                               const std::string& filepath,
	                           const textile::VTexInfo& info,
	                           int atlas_channel,
	                           int feedback_size)
	: m_feedback_size(feedback_size)
	, m_vtex_w(info.vtex_width)
    , m_vtex_h(info.vtex_height)
	, m_info(info)
	, m_atlas(dev, ATLAS_TEX_SIZE, m_info.PageSize(), atlas_channel)
	, m_indexer(m_info)
	, m_loader(filepath, m_indexer)
	, m_table(dev, m_info.PageTableWidth(), m_info.PageTableHeight())
	, m_cache(m_atlas, m_loader, m_table, m_indexer)
	, m_feedback(dev, feedback_size, m_info.PageTableWidth(), m_info.PageTableHeight(), m_indexer)
	, m_mip_bias(MIP_SAMPLE_BIAS)
{
	InitShaders(dev);
}

void VirtualTexture::Draw(const ur::Device& dev, ur::Context& ctx, std::function<void()> draw_cb)
{
	// pass 1

	//pt3::EffectsManager::Instance()->SetUserEffect(m_feedback_shader);

	m_feedback.BindRT();

	//m_feedback_shader->Use();

	//auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	//auto screen_sz = wc->GetScreenSize();

	//rc.SetViewport(0, 0, m_feedback_size, m_feedback_size);

	//rc.SetZTest(ur::DEPTH_LESS_EQUAL);
	//rc.SetClearFlag(ur::MASKC | ur::MASKD);
 //   rc.SetClearColor(0);
	//rc.Clear();

	draw_cb();

	m_feedback.Download(dev);
	Update(dev, m_feedback.GetRequests());
	m_feedback.Clear();

	m_feedback.UnbindRT();

//	rc.SetViewport(0, 0, screen_sz.x, screen_sz.y);

	// pass 2
//	rc.Clear();

//	pt3::EffectsManager::Instance()->SetUserEffect(m_final_shader);
    ctx.SetTexture(m_final_shader->QueryTexSlot("u_page_table_tex"), m_table.GetTexture());
    ctx.SetTexture(m_final_shader->QueryTexSlot("u_texture_atlas_tex"), m_atlas.GetTexture());
	draw_cb();

	// debug
	pt2::DebugDraw::Draw(dev, ctx, m_atlas.GetTexture()->GetTexID(), 4);
	pt2::DebugDraw::Draw(dev, ctx, m_feedback.GetTexture()->GetTexID(), 3);
	//pt2::DebugDraw::Draw(virt_tex->GetPageTableTexID(), 2);
}

void VirtualTexture::DecreaseMipBias()
{
	--m_mip_bias;
	if (m_mip_bias < 0) {
		m_mip_bias = 0;
	}

    auto u_mip_sample_bias = m_feedback_shader->QueryUniform("u_mip_sample_bias");
    assert(u_mip_sample_bias);
    float bias = static_cast<float>(m_mip_bias);
    u_mip_sample_bias->SetValue(&bias, 1);
}

void VirtualTexture::InitShaders(const ur::Device& dev)
{
	//CU_VEC<ur::VertexAttrib> layout;
	//layout.push_back(ur::VertexAttrib("position", 3, 4, 32, 0));
	//layout.push_back(ur::VertexAttrib("normal",   3, 4, 32, 12));
	//layout.push_back(ur::VertexAttrib("texcoord", 2, 4, 32, 24));

    const float page_table_sz[2] = {
        static_cast<float>(m_info.PageTableWidth()),
        static_cast<float>(m_info.PageTableHeight())
    };
    const float vtex_sz[2] = {
        static_cast<float>(m_vtex_w),
        static_cast<float>(m_vtex_h)
    };

	// feedback
	{
		std::vector<unsigned int> vs, fs;
		shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::VertexShader, default_vs, vs);
		shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::PixelShader, feedback_frag, fs);
        m_feedback_shader = dev.CreateShaderProgram(vs, fs);

        auto u_page_table_size = m_feedback_shader->QueryUniform("u_page_table_size");
        assert(u_page_table_size);
        u_page_table_size->SetValue(page_table_sz, 2);

        auto u_virt_tex_size = m_feedback_shader->QueryUniform("u_virt_tex_size");
        assert(u_virt_tex_size);
        u_virt_tex_size->SetValue(vtex_sz, 2);

        auto u_mip_sample_bias = m_feedback_shader->QueryUniform("u_mip_sample_bias");
        assert(u_mip_sample_bias);
        float bias = static_cast<float>(m_mip_bias);
        u_mip_sample_bias->SetValue(&bias, 1);
	}
	// final
	{
		//std::vector<std::string> textures;
		//textures.push_back("u_page_table_tex");
		//textures.push_back("u_texture_atlas_tex");

		std::vector<unsigned int> vs, fs;
		shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::VertexShader, default_vs, vs);
		shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::PixelShader, final_frag, fs);
        m_final_shader = dev.CreateShaderProgram(vs, fs);

        auto u_page_table_size = m_final_shader->QueryUniform("u_page_table_size");
        assert(u_page_table_size);
        u_page_table_size->SetValue(page_table_sz, 2);

        auto u_virt_tex_size = m_final_shader->QueryUniform("u_virt_tex_size");
        assert(u_virt_tex_size);
        u_virt_tex_size->SetValue(vtex_sz, 2);

        auto u_atlas_scale = m_final_shader->QueryUniform("u_atlas_scale");
        assert(u_atlas_scale);
        float atlas_scale = static_cast<float>(m_info.PageSize()) / m_atlas.GetSize();
        u_atlas_scale->SetValue(&atlas_scale, 1);

		float page_size = static_cast<float>(m_info.PageSize());

        auto u_border_scale = m_final_shader->QueryUniform("u_border_scale");
        assert(u_border_scale);
        float border_scale = (page_size - 2 * m_info.border_size) / page_size;
        u_border_scale->SetValue(&border_scale, 1);

        auto u_border_offset = m_final_shader->QueryUniform("u_border_offset");
        assert(u_border_offset);
        float border_offset = m_info.border_size / page_size;
        u_border_offset->SetValue(&border_offset, 1);
	}
}

void VirtualTexture::Update(const ur::Device& dev,
                            const std::vector<int>& requests)
{
	m_toload.clear();

	int touched = 0;
	for (int i = 0, n = requests.size(); i < n; ++i)
	{
		if (requests[i] == 0) {
			continue;
		}

		auto& page = m_indexer.QueryPageByIdx(i);
		if (!m_cache.Touch(page)) {
			m_toload.push_back(PageWithCount(page, requests[i]));
		} else {
			++touched;
		}
	}

	int page_n = m_atlas.GetPageCount();
	if (touched < page_n * page_n)
	{
		std::sort(m_toload.begin(), m_toload.end(), [](const PageWithCount& p0, const PageWithCount& p1)->bool {
			if (p0.page.mip != p1.page.mip) {
				return p0.page.mip < p1.page.mip;
			} else {
				return p0.count > p1.count;
			}
		});

		int load_n = std::min((int)m_toload.size(), UPLOADS_PER_FRAME);
		for (int i = 0; i < load_n; ++i) {
			m_cache.Request(dev, m_toload[i].page);
		}
	}
	else
	{
		DecreaseMipBias();
	}

	m_loader.Update(dev);

	m_table.Update();
}

}