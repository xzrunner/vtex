#pragma once

#include "vtex/Page.h"

#include <boost/noncopyable.hpp>

#include <cstdint>
#include <vector>

namespace vtex
{

struct Page;
class TextureAtlas;
class PageLoader;
class PageTable;

class PageCache : public boost::noncopyable
{
public:
	PageCache(TextureAtlas& atlas, PageLoader& loader, PageTable& table);

	void LoadComplete(const Page& page, const uint8_t* data);

	bool Touch(const Page& page);

	bool Request(const Page& page);

	void Clear() { m_lru.clear(); }

private:
	struct PageWithPos
	{
		PageWithPos(const Page& page, int x, int y)
			: page(page), x(x), y(y) {}

		Page page;
		int x, y;
	};

private:
	TextureAtlas& m_atlas;
	PageLoader&   m_loader;
	PageTable&    m_table;

	std::vector<PageWithPos> m_lru;

}; // PageCache

}