#include "vtex/PageTable.h"

#include <unirender2/Device.h>
#include <unirender2/Texture.h>
#include <unirender2/TextureDescription.h>
#include <textile/PageIndexer.h>

#include <algorithm>

#include <assert.h>

namespace vtex
{

PageTable::PageTable(const ur2::Device& dev, int width, int height)
	: m_width(width)
    , m_height(height)
{
    const auto level = CalcMaxLevel();
	m_root = std::make_unique<QuadNode>(level, Rect(0, 0, m_width, m_height));

	m_data.resize(level + 1);
	for (size_t i = 0; i < level + 1; ++i)
	{
		int sw = width >> i;
        int sh = height >> i;

		auto& data = m_data[i];
		data.w = sw;
        data.h = sh;
		data.data = new uint8_t[sw * sh * 4];
		memset(data.data, 0, sw * sh * 4);
	}

    ur2::TextureDescription desc;
    desc.target = ur2::TextureTarget::Texture2D;
    desc.width  = m_width;
    desc.height = m_height;
    desc.format = ur2::TextureFormat::RGBA8;
    m_tex = dev.CreateTexture(desc, nullptr);
}

void PageTable::AddPage(const textile::Page& page, int mapping_x, int mapping_y)
{
	int scale = 1 << page.mip;
	int x = page.x * scale;
	int y = page.y * scale;

	QuadNode* node = m_root.get();
	while (page.mip < node->level)
	{
		for (int i = 0; i < 4; ++i)
		{
			Rect cr = node->CalcChildRect(i);
			if (!cr.Contain(x, y)) {
				continue;
			}
			if (node->children[i] == nullptr) {
				node->children[i] = std::make_unique<QuadNode>(node->level - 1, cr);
			}
			node = node->children[i].get();
			break;
		}
	}

	node->mapping_x = mapping_x;
	node->mapping_y = mapping_y;
}

void PageTable::RemovePage(const textile::Page& page)
{
	int index;
	auto node = FindPage(page, index);
	if (node != nullptr) {
		node->children[index] = nullptr;
	}
}

void PageTable::Update()
{
	const auto level = CalcMaxLevel();
	for (size_t i = 0; i < level + 1; ++i)
	{
		m_root->Write(m_data[i].w, m_data[i].h, m_data[i].data, i);
        m_tex->Upload(m_data[i].data, 0, 0, m_data[i].w, m_data[i].h, i);
	}
}

PageTable::QuadNode* PageTable::FindPage(const textile::Page& page, int& index) const
{
	QuadNode* node = m_root.get();

	int scale = 1 << page.mip;
	int x = page.x * scale;
	int y = page.y * scale;

	bool exitloop = false;
	while (!exitloop)
	{
		exitloop = true;
		for (int i = 0; i < 4; ++i)
		{
			if (node->children[i] != nullptr && node->children[i]->rect.Contain(x, y))
			{
				// We found it
				if (page.mip == node->level - 1)
				{
					index = i;
					return node;
				}

				// Check the children
				else
				{
					node = node->children[i].get();
					exitloop = false;
				}
			}
		}
	}

	// We couldn't find it so it must not exist anymore
	index = -1;
	return nullptr;
}

size_t PageTable::CalcMaxLevel() const
{
    return static_cast<size_t>(std::min(std::log2(m_width), std::log2(m_height)));
}

/************************************************************************/
/* class PageTable::QuadNode                                            */
/************************************************************************/

PageTable::QuadNode::QuadNode(int level, const Rect& rect)
	: level(level)
	, rect(rect)
	, mapping_x(0)
	, mapping_y(0)
{
	for (int i = 0; i < 4; ++i) {
		children[i] = nullptr;
	}
}

PageTable::Rect PageTable::QuadNode::CalcChildRect(int idx) const
{
	int x = rect.x;
	int y = rect.y;
	int w = rect.w / 2;
	int h = rect.h / 2;

	switch (idx)
	{
	case 0:
		return Rect(x, y, w, h);
	case 1:
		return Rect(x + w, y, w, h);
	case 2:
		return Rect(x + w, y + h, w, h);
	case 3:
		return Rect(x, y + h, w, h);
	default:
		assert(0);
		return rect;
	}
}

void PageTable::QuadNode::Write(int w, int h, uint8_t* data, int mip_level)
{
	if (level < mip_level) {
		return;
	}

	// fill
	int rx = rect.x >> mip_level;
	int ry = rect.y >> mip_level;
	int rw = rect.w >> mip_level;
	int rh = rect.h >> mip_level;
	for (int y = ry; y < ry + rh; ++y) {
		for (int x = rx; x < rx + rw; ++x) {
			int ptr = (y * w + x) * 4;
			data[ptr + 0] = mapping_x;
			data[ptr + 1] = mapping_y;
			data[ptr + 2] = level;
			data[ptr + 3] = 255;
		}
	}

	for (int i = 0; i < 4; ++i) {
		if (children[i] != nullptr) {
			children[i]->Write(w, h, data, mip_level);
		}
	}
}

}