#include "vtex/PageCache.h"
#include "vtex/TextureAtlas.h"
#include "vtex/PageLoader.h"
#include "vtex/PageTable.h"
#include "vtex/PageIndexer.h"

#include <assert.h>

//#define CHECK_LRU

#ifdef CHECK_LRU
namespace
{
int tot_count = 0;
}
#endif // CHECK_LRU

namespace vtex
{

PageCache::PageCache(TextureAtlas& atlas, PageLoader& loader,
	                 PageTable& table, const PageIndexer& indexer)
	: m_atlas(atlas)
	, m_loader(loader)
	, m_table(table)
	, m_lru(indexer)
{
}

void PageCache::LoadComplete(const Page& page, const uint8_t* data)
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

bool PageCache::Touch(const Page& page)
{
	if (!m_lru.Find(page)) {
		return false;
	} else {
		m_lru.Touch(page);
		return true;
	}
}

bool PageCache::Request(const Page& page)
{
	if (m_lru.Find(page)) {
		return false;
	} else {
		m_loader.LoadPage(page, *this);
		return true;
	}
}

/************************************************************************/
/* PageCache::LruCollection                                             */
/************************************************************************/

PageCache::LruCollection::LruCollection(const PageIndexer& indexer)
	: m_indexer(indexer)
	, m_list_begin(nullptr)
	, m_list_end(nullptr)
	, m_freelist(nullptr)
{
}

PageCache::LruCollection::~LruCollection()
{
	for (auto& itr : m_map) {
		delete itr.second;
	}
}

bool PageCache::LruCollection::RemoveBack()
{
	if (m_map.empty()) {
		return false;
	}

	int key = m_indexer.CalcPageIdx(m_list_end->page);
	m_map.erase(key);

	assert(m_list_end);

	if (m_list_begin == m_list_end)
	{
		if (m_freelist) {
			m_freelist->prev = m_list_begin;
		}
		m_list_begin->next = m_freelist;

		m_freelist = m_list_begin;
		m_list_begin = nullptr;
		m_list_end = nullptr;
	}
	else
	{
		auto back = m_list_end;
		if (back->prev) {
			back->prev->next = nullptr;
		}
		m_list_end = back->prev;
		m_list_end->next = nullptr;

		back->next = m_freelist;
		if (m_freelist) {
			m_freelist->prev = back;
		}
		m_freelist = back;
	}

#ifdef CHECK_LRU
	Check();
#endif // CHECK_LRU

	return true;
}

bool PageCache::LruCollection::AddFront(const Page& page, int x, int y)
{
	int idx = m_indexer.CalcPageIdx(page);
	if (m_map.find(idx) != m_map.end()) {
		return false;
	}

	CachePage* cp;
	if (m_freelist) {
		cp = m_freelist;
		m_freelist = m_freelist->next;
		if (m_freelist) {
			m_freelist->prev = nullptr;
		}
	} else {
		cp = new CachePage();
#ifdef CHECK_LRU
		++tot_count;
#endif // CHECK_LRU
	}

	m_map.insert({ idx, cp });

	cp->page = page;
	cp->x = x;
	cp->y = y;
	cp->prev = nullptr;
	cp->next = m_list_begin;
	if (m_list_begin)
	{
		m_list_begin->prev = cp;
		m_list_begin = cp;
	}
	else
	{
		m_list_begin = cp;
		m_list_end = cp;
	}

#ifdef CHECK_LRU
	Check();
#endif // CHECK_LRU

	return true;
}

bool PageCache::LruCollection::Find(const Page& page) const
{
	return m_map.find(m_indexer.CalcPageIdx(page)) != m_map.end();
}

void PageCache::LruCollection::Touch(const Page& page)
{
	if (m_map.size() <= 1) {
		return;
	}

	auto itr = m_map.find(m_indexer.CalcPageIdx(page));
	if (itr == m_map.end()) {
		return;
	}

	CachePage* curr = itr->second;

	if (curr == m_list_begin) {
		return;
	}
	if (curr == m_list_end) {
		m_list_end = curr->prev;
	}

	auto prev = curr->prev;
	auto next = curr->next;
	if (prev) {
		prev->next = next;
	}
	if (next) {
		next->prev = prev;
	}

	curr->next = m_list_begin;
	if (m_list_begin) {
		m_list_begin->prev = curr;
	}
	curr->prev = nullptr;
	m_list_begin = curr;

#ifdef CHECK_LRU
	Check();
#endif // CHECK_LRU
}

void PageCache::LruCollection::Clear()
{
	m_map.clear();

	if (m_list_begin)
	{
		assert(m_list_end);
		m_list_end->next = m_freelist;
		if (m_freelist) {
			m_freelist->prev = m_list_end;
		}
		m_freelist = m_list_begin;

		m_list_begin = nullptr;
		m_list_end = nullptr;
	}

#ifdef CHECK_LRU
	Check();
#endif // CHECK_LRU
}

void PageCache::LruCollection::Check()
{
#ifdef CHECK_LRU
	assert(!m_list_begin->prev && !m_list_end->next);

	int count = 0;
	auto n = m_list_begin;
	auto last = n;
	while (n) {
		last = n;

		++count;
		assert(count <= m_map.size());

		if (n->next && n->next->prev != n) {
			assert(0);
		}
		if (n->prev && n->prev->next != n) {
			assert(0);
		}

		n = n->next;
	}

	assert(last == m_list_end);
	assert(count == m_map.size());

	int free_count = 0;
	n = m_freelist;
	while (n) {
		++free_count;
		n = n->next;
	}
	assert(free_count + count == tot_count);
#endif // CHECK_LRU
}

}