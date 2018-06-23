#include "vtex/PageTable.h"
#include "vtex/Page.h"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>

#include <assert.h>

namespace vtex
{

PageTable::PageTable(int page_table_size)
	: m_page_table_size(page_table_size)
{
	int level = static_cast<int>(std::log2(m_page_table_size));
	m_root = std::make_unique<QuadNode>(level, Rect(0, 0, m_page_table_size, m_page_table_size));

	m_data.resize(level + 1);
	for (int i = 0; i < level + 1; ++i)
	{
		int size = page_table_size >> i;

		auto& data = m_data[i];
		data.size = size;
		data.data = new uint8_t[size * size * 4];
		memset(data.data, 0, size * size * 4);
	}

	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	m_texid = rc.CreateTextureID(m_page_table_size, m_page_table_size, ur::TEXTURE_RGBA8, 1);
}

PageTable::~PageTable()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	rc.ReleaseTexture(m_texid);
}

void PageTable::AddPage(const Page& page, int mapping_x, int mapping_y)
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

void PageTable::RemovePage(const Page& page)
{
	int index;
	auto node = FindPage(page, index);
	if (node != nullptr) {
		node->children[index] = nullptr;
	}
}

void PageTable::Update()
{
	int level = static_cast<int>(std::log2(m_page_table_size));
	for (int i = 0; i < level + 1; ++i)
	{
		m_root->Write(m_data[i].size, m_data[i].data, i);
		auto& rc = ur::Blackboard::Instance()->GetRenderContext();
		rc.UpdateTexture(m_texid, m_data[i].data, m_data[i].size, m_data[i].size, 0, i, 0);
	}
}

PageTable::QuadNode* PageTable::FindPage(const Page& page, int& index) const
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

void PageTable::QuadNode::Write(int size, uint8_t* data, int mip_level)
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
			int ptr = (y * size + x) * 4;
			data[ptr + 0] = mapping_x;
			data[ptr + 1] = mapping_y;
			data[ptr + 2] = level;
			data[ptr + 3] = 255;
		}
	}

	for (int i = 0; i < 4; ++i) {
		if (children[i] != nullptr) {
			children[i]->Write(size, data, mip_level);
		}
	}
}

}