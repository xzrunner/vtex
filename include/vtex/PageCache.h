#pragma once

#include "vtex/Page.h"

#include <boost/noncopyable.hpp>

#include <cstdint>
#include <list>
#include <unordered_map>

namespace vtex
{

struct Page;
class TextureAtlas;
class PageLoader;
class PageTable;
class PageIndexer;

class PageCache : public boost::noncopyable
{
public:
	PageCache(TextureAtlas& atlas, PageLoader& loader,
		PageTable& table, const PageIndexer& indexer);

	void LoadComplete(const Page& page, const uint8_t* data);

	bool Touch(const Page& page);

	bool Request(const Page& page);

	void Clear() { m_lru.Clear(); }

private:
	struct CachePage
	{
		Page page;
		int x, y;

		CachePage *prev = nullptr, *next = nullptr;
	};

	class LruCollection
	{
	public:
		LruCollection(const PageIndexer& indexer);
		~LruCollection();

		bool RemoveBack();
		bool AddFront(const Page& page, int x, int y);

		CachePage* GetListEnd() { return m_list_end; }

		bool Find(const Page& page) const;

		void Touch(const Page& page);

		int Size() const { return m_map.size(); }

		void Clear();

	private:
		void Check();

	private:
		const PageIndexer& m_indexer;

		std::unordered_map<int, CachePage*> m_map;
		CachePage *m_list_begin, *m_list_end;

		CachePage* m_freelist;

	}; // LruCollection

private:
	TextureAtlas& m_atlas;
	PageLoader&   m_loader;
	PageTable&    m_table;

	LruCollection m_lru;

}; // PageCache

}