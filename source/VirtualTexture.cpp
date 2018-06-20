#include "vtex/VirtualTexture.h"
#include "vtex/VirtualTextureInfo.h"

#include <algorithm>

namespace
{

const int ATLAS_TEX_SIZE = 4096;
const int UPLOADS_PER_FRAME = 5;

}

namespace vtex
{

VirtualTexture::VirtualTexture(const std::string& filepath,
	                           const VirtualTextureInfo& info,
	                           int atlas_channel)
	: m_atlas(ATLAS_TEX_SIZE, info.PageSize(), atlas_channel)
	, m_indexer(info)
	, m_loader(filepath, m_indexer)
	, m_table(info.PageTableSize())
	, m_cache(m_atlas, m_loader, m_table)
{
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
		// todo
//		--MipBias;
	}

	// loader update
	m_table.Update();
}

}