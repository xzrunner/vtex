#pragma once

#include "vtex/Page.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace vtex
{

struct VirtualTextureInfo;

class PageIndexer : boost::noncopyable
{
public:
	PageIndexer(const VirtualTextureInfo& info);

	int CalcPageIdx(const Page& page) const;
	const Page& QueryPageByIdx(int idx) const;

	int GetPageCount() const { return m_page_count; }

private:
	int m_page_count;
	int m_mip_count;

	std::vector<int> m_offsets;
	std::vector<int> m_sizes;

	std::vector<Page> m_pages;

}; // PageIndexer

}