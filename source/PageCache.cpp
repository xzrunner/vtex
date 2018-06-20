#include "vtex/PageCache.h"
#include "vtex/TextureAtlas.h"
#include "vtex/PageLoader.h"
#include "vtex/PageTable.h"

namespace vtex
{

PageCache::PageCache(TextureAtlas& atlas, PageLoader& loader, PageTable& table)
	: m_atlas(atlas)
	, m_loader(loader)
	, m_table(table)
{
}

void PageCache::LoadComplete(const Page& page, const uint8_t* data)
{
	int x, y;

	int page_n = m_atlas.GetPageCount();
	if (m_lru.size() == page_n * page_n)
	{
		x = m_lru.back().x;
		y = m_lru.back().y;
		m_lru.pop_back();
	}
	else
	{
		x = m_lru.size() % page_n;
		y = m_lru.size() / page_n;
	}
	m_lru.push_back(PageWithPos(page, x, y));

	m_atlas.UploadPage(data, x, y);

	m_table.AddPage(page, x, y);
}

bool PageCache::Touch(const Page& page)
{
	for (auto& pp : m_lru) {
		if (pp.page == page) {
			return true;
		}
	}
	return false;
}

bool PageCache::Request(const Page& page)
{
	for (auto& pp : m_lru) {
		if (pp.page == page) {
			return false;
		}
	}

	auto data = m_loader.LoadPage(page);
	LoadComplete(page, data);

	return true;
}

}