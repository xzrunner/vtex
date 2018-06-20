#include "vtex/TextureAtlas.h"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>

namespace vtex
{

TextureAtlas::TextureAtlas(size_t atlas_size, size_t page_size, int tex_channel)
	: m_atlas_size(atlas_size)
	, m_page_size(page_size)
	, m_tex_channel(tex_channel)
{
	m_page_count = m_atlas_size / m_page_size;

	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	m_texid = rc.CreateTextureID(atlas_size, atlas_size, ur::TEXTURE_RGBA8);

	uint8_t* pixels = new uint8_t[atlas_size * atlas_size * 4];
	memset(pixels, 0xff, atlas_size * atlas_size * 4);
	rc.UpdateTexture(m_texid, pixels, atlas_size, atlas_size);
	delete[] pixels;
}

TextureAtlas::~TextureAtlas()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	rc.ReleaseTexture(m_texid);
}

void TextureAtlas::Bind()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	rc.BindTexture(m_texid, m_tex_channel);
}

void TextureAtlas::UploadPage(const uint8_t* pixels, int x, int y)
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	rc.UpdateSubTexture(pixels, x * m_page_size, y * m_page_size, m_page_size, m_page_size, m_texid);
}

}