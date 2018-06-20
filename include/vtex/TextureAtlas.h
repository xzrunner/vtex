#pragma once

#include <boost/noncopyable.hpp>

#include <cstdint>

namespace vtex
{

class TextureAtlas : private boost::noncopyable
{
public:
	TextureAtlas(size_t atlas_size, size_t page_size, int tex_channel);
	~TextureAtlas();

	int GetSize() const { return m_atlas_size; }

	size_t GetPageCount() const { return m_page_count; }

	void Bind();

	void UploadPage(const uint8_t* pixels, int x, int y);

	uint32_t GetTexID() const { return m_texid; }

private:
	size_t m_atlas_size;
	size_t m_page_size;
	size_t m_page_count;

	int m_tex_channel;

	uint32_t m_texid;

}; // TextureAtlas

}