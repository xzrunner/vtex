#include "vtex/TextureAtlas.h"

#include <unirender/Device.h>
#include <unirender/TextureDescription.h>
#include <unirender/Texture.h>

namespace vtex
{

TextureAtlas::TextureAtlas(const ur::Device& dev, size_t atlas_size,
                           size_t page_size, int tex_channel)
	: m_atlas_size(atlas_size)
	, m_page_size(page_size)
	, m_tex_channel(tex_channel)
{
	m_page_count = m_atlas_size / m_page_size;

    uint8_t* pixels = new uint8_t[atlas_size * atlas_size * 4];
    memset(pixels, 0xff, atlas_size * atlas_size * 4);

    ur::TextureDescription desc;
    desc.target = ur::TextureTarget::Texture2D;
    desc.width  = atlas_size;
    desc.height = atlas_size;
    desc.format = ur::TextureFormat::RGBA8;
    m_tex = dev.CreateTexture(desc, pixels);

	delete[] pixels;
}

void TextureAtlas::Bind()
{
    m_tex->Bind();
}

void TextureAtlas::UploadPage(const uint8_t* pixels, int x, int y)
{
    m_tex->Upload(pixels, x * m_page_size, y * m_page_size, m_page_size, m_page_size);
}

}