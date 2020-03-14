#include "vtex/PageCache.h"
#include "vtex/TextureAtlas.h"
#include "vtex/PageTable.h"

#include <assert.h>

namespace vtex
{

PageCache::PageCache(TextureAtlas& atlas, textile::PageLoader& loader,
	                 PageTable& table, const textile::PageIndexer& indexer)
	: textile::PageCache(loader, indexer)
    , m_atlas(atlas)
	, m_table(table)
{
}

void PageCache::LoadComplete(const textile::Page& page, const uint8_t* data)
{
	int x, y;

	int page_n = m_atlas.GetPageCount();
	if (m_lru.Size() == page_n * page_n)
	{
		auto end = m_lru.GetListEnd();
		assert(end);

		m_table.RemovePage(end->page);

		x = end->x;
		y = end->y;
		m_lru.RemoveBack();
	}
	else
	{
		x = m_lru.Size() % page_n;
		y = m_lru.Size() / page_n;
	}
	m_lru.AddFront(page, x, y);

	m_atlas.UploadPage(data, x, y);

	m_table.AddPage(page, x, y);
}

}