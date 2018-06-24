#include "vtex/VirtualTexture.h"
#include "vtex/VirtualTextureInfo.h"
#include "vtex/feedback.frag"
#include "vtex/final.frag"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <unirender/Shader.h>
#include <painting2/DebugDraw.h>
#include <painting3/EffectsManager.h>
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

VirtualTexture::VirtualTexture(const std::string& filepath,
	                           const VirtualTextureInfo& info,
	                           int atlas_channel,
	                           int feedback_size)
	: m_feedback_size(feedback_size)
	, m_virtual_tex_size(info.virtual_texture_size)
	, m_info(info)
	, m_atlas(ATLAS_TEX_SIZE, m_info.PageSize(), atlas_channel)
	, m_indexer(m_info)
	, m_loader(filepath, m_indexer)
	, m_table(m_info.PageTableSize())
	, m_cache(m_atlas, m_loader, m_table, m_indexer)
	, m_feedback(feedback_size, m_info.PageTableSize(), m_indexer)
	, m_mip_bias(MIP_SAMPLE_BIAS)
{
	InitShaders();
}

void VirtualTexture::Draw(std::function<void()> draw_cb)
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();

	// pass 1

	pt3::EffectsManager::Instance()->SetUserEffect(m_feedback_shader);

	m_feedback.BindRT();

	m_feedback_shader->Use();

	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	auto screen_sz = wc->GetScreenSize();

	rc.SetViewport(0, 0, m_feedback_size, m_feedback_size);

	rc.SetDepthFormat(ur::DEPTH_LESS_EQUAL);
	rc.SetClearFlag(ur::MASKC | ur::MASKD);
	rc.Clear(0);

	draw_cb();

	m_feedback.Download();
	Update(m_feedback.GetRequests());
	m_feedback.Clear();

	m_feedback.UnbindRT();

	rc.SetViewport(0, 0, screen_sz.x, screen_sz.y);

	// pass 2
	rc.Clear(0);

	pt3::EffectsManager::Instance()->SetUserEffect(m_final_shader);
	rc.BindTexture(m_table.GetTexID(), 0);
	rc.BindTexture(m_atlas.GetTexID(), 1);
	draw_cb();

	// debug
	pt2::DebugDraw::Draw(m_atlas.GetTexID(), 4);
	pt2::DebugDraw::Draw(m_feedback.GetTexID(), 3);
	//pt2::DebugDraw::Draw(virt_tex->GetPageTableTexID(), 2);
}

void VirtualTexture::DecreaseMipBias()
{
	--m_mip_bias;
	if (m_mip_bias < 0) {
		m_mip_bias = 0;
	}
	m_feedback_shader->SetFloat("u_mip_sample_bias", static_cast<float>(m_mip_bias));
}

void VirtualTexture::InitShaders()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();

	CU_VEC<ur::VertexAttrib> layout;
	layout.push_back(ur::VertexAttrib("position", 3, 4));
	layout.push_back(ur::VertexAttrib("normal", 3, 4));
	layout.push_back(ur::VertexAttrib("texcoord", 2, 4));

	// feedback
	{
		std::vector<std::string> textures;
		m_feedback_shader = std::make_shared<ur::Shader>(&rc, default_vs, feedback_frag, textures, layout);

		m_feedback_shader->Use();

		m_feedback_shader->SetFloat("u_page_table_size", static_cast<float>(m_info.PageTableSize()));
		m_feedback_shader->SetFloat("u_virt_tex_size", static_cast<float>(m_virtual_tex_size));
		m_feedback_shader->SetFloat("u_mip_sample_bias", static_cast<float>(m_mip_bias));
	}
	// final
	{
		std::vector<std::string> textures;
		textures.push_back("u_page_table_tex");
		textures.push_back("u_texture_atlas_tex");

		m_final_shader = std::make_shared<ur::Shader>(&rc, default_vs, final_frag, textures, layout);

		m_final_shader->Use();

		m_final_shader->SetFloat("u_page_table_size", static_cast<float>(m_info.PageTableSize()));
		m_final_shader->SetFloat("u_virt_tex_size", static_cast<float>(m_virtual_tex_size));
		m_final_shader->SetFloat("u_atlas_scale", static_cast<float>(m_info.PageSize()) / m_atlas.GetSize());

		float page_size = static_cast<float>(m_info.PageSize());
		m_final_shader->SetFloat("u_border_scale", (page_size - 2 * m_info.border_size) / page_size);
		m_final_shader->SetFloat("u_border_offset", m_info.border_size / page_size);
	}
}

void VirtualTexture::Update(const std::vector<int>& requests)
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
			m_cache.Request(m_toload[i].page);
		}
	}
	else
	{
		DecreaseMipBias();
	}

	m_loader.Update();

	m_table.Update();
}

}