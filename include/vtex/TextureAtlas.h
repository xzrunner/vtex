#pragma once

#include <unirender/typedef.h>

#include <boost/noncopyable.hpp>

#include <cstdint>

namespace ur { class Device; }

namespace vtex
{

class TextureAtlas : private boost::noncopyable
{
public:
	TextureAtlas(const ur::Device& dev, size_t atlas_size,
        size_t page_size, int tex_channel);

	int GetSize() const { return m_atlas_size; }

	size_t GetPageCount() const { return m_page_count; }

	void Bind();

	void UploadPage(const uint8_t* pixels, int x, int y);

    auto GetTexture() const { return m_tex; }

private:
	size_t m_atlas_size;
	size_t m_page_size;
	size_t m_page_count;

	int m_tex_channel;

    ur::TexturePtr m_tex = nullptr;

}; // TextureAtlas

}