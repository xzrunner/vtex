#pragma once

#include "vtex/Page.h"
#include "vtex/TextureAtlas.h"
#include "vtex/PageIndexer.h"
#include "vtex/PageCache.h"
#include "vtex/PageTable.h"
#include "vtex/PageLoader.h"

#include <vector>

namespace vtex
{

class VirtualTexture : private boost::noncopyable
{
public:
	VirtualTexture(const std::string& filepath,
		const VirtualTextureInfo& info, int atlas_channel);

	void Update(const std::vector<int>& requests);

	int GetAtlasSize() const { return m_atlas.GetSize(); }

	uint32_t GetAtlasTexID() const { return m_atlas.GetTexID(); }
	uint32_t GetPageTableTexID() const { return m_table.GetTexID(); }

	const PageIndexer& GetPageIndexer() const { return m_indexer; }

private:
	struct PageWithCount
	{
		PageWithCount(const Page& page, int count)
			: page(page), count(count) {}

		Page page;
		int count = 0;
	};

private:
	TextureAtlas m_atlas;

	PageIndexer  m_indexer;

	PageLoader   m_loader;

	PageTable    m_table;

	PageCache    m_cache;

	std::vector<PageWithCount> m_toload;

}; // VirtualTexture

}