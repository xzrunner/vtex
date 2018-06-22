#pragma once

#include "vtex/TileDataFile.h"

#include <boost/noncopyable.hpp>

#include <string>

namespace vtex
{

class PageIndexer;
struct Page;

class PageLoader : boost::noncopyable
{
public:
	PageLoader(const std::string& filepath, const PageIndexer& indexer);

	const uint8_t* LoadPage(const Page& page);

	void ChangeShowBorder() { m_show_borders = !m_show_borders; }
	void ChangeShowMip() { m_show_mip = !m_show_mip; }

private:
	void LoadPageFromFile(const Page& page, uint8_t* pixels);

	void CopyBorder(uint8_t* pixels) const;
	void CopyColor(uint8_t* pixels, int mip) const;

private:
	const PageIndexer& m_indexer;

	TileDataFile m_file;

	bool m_show_borders;
	bool m_show_mip;

}; // PageLoader

}