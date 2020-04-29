#pragma once

#include <textile/PageCache.h>

namespace vtex
{

class TextureAtlas;
class PageTable;

class PageCache : public textile::PageCache
{
public:
	PageCache(TextureAtlas& atlas, textile::PageLoader& loader,
		PageTable& table, const textile::PageIndexer& indexer);

	virtual void LoadComplete(const ur::Device& dev, const textile::Page& page, const uint8_t* data) override;

private:
	TextureAtlas& m_atlas;
	PageTable&    m_table;

}; // PageCache

}