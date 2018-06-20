#include "vtex/PageIndexer.h"
#include "vtex/VirtualTextureInfo.h"

#include <assert.h>

namespace vtex
{

PageIndexer::PageIndexer(const VirtualTextureInfo& info)
{
	m_mip_count = static_cast<int>(std::log2(info.PageTableSize())) + 1;

	m_sizes.resize(m_mip_count);
	for (int i = 0; i < m_mip_count; ++i) {
		m_sizes[i] = (info.virtual_texture_size / info.tile_size) >> i;
	}

	m_offsets.resize(m_mip_count);
	m_page_count = 0;
	for (int i = 0; i < m_mip_count; ++i) {
		m_offsets[i] = m_page_count;
		m_page_count += m_sizes[i] * m_sizes[i];
	}

	m_pages.resize(m_page_count);
	for (int i = 0; i < m_mip_count; ++i)
	{
		int size = m_sizes[i];
		for (int y = 0; y < size; ++y) {
			for (int x = 0; x < size; ++x) {
				Page p(x, y, i);
				m_pages[CalcPageIdx(p)] = p;
			}
		}
	}
}

int PageIndexer::CalcPageIdx(const Page& page) const
{
	assert(page.mip >= 0 && page.mip < m_mip_count);
	int offset = m_offsets[page.mip];
	int stride = m_sizes[page.mip];
	return offset + page.y * stride + page.x;
}

const Page& PageIndexer::QueryPageByIdx(int idx) const
{
	assert(idx >= 0 && idx < static_cast<int>(m_pages.size()));
	return m_pages[idx];
}

}