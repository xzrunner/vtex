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

private:
	const PageIndexer& m_indexer;

	TileDataFile m_file;

}; // PageLoader

}