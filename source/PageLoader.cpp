#include "vtex/PageLoader.h"
#include "vtex/PageIndexer.h"

namespace vtex
{

PageLoader::PageLoader(const std::string& filepath, const PageIndexer& indexer)
	: m_indexer(indexer)
	, m_file(filepath)
{
}

const uint8_t* PageLoader::LoadPage(const Page& page)
{
	auto& info = m_file.GetVTexInfo();
	int size = info.PageSize() * info.PageSize() * TileDataFile::CHANNEL_COUNT;

	uint8_t* data = new uint8_t[size];
	if (!data) {
		return nullptr;
	}

	int page_table_size_log2 = static_cast<int>(std::log2(info.PageTableSize()));
	int count = page_table_size_log2 - page.mip + 1;
	int y = std::pow(2, count - 1) - 1 - page.y;


	m_file.ReadPage(m_indexer.CalcPageIdx(Page(page.x, y, page.mip)), data);

	// revert y
	int line_sz = info.PageSize() * TileDataFile::CHANNEL_COUNT;
	uint8_t* buf = new uint8_t[line_sz];
	int bpos = 0, epos = line_sz * (info.PageSize() - 1);
	for (int i = 0, n = (int)(floorf(info.PageSize() / 2.0f)); i < n; ++i)
	{
		memcpy(buf, &data[bpos], line_sz);
		memcpy(&data[bpos], &data[epos], line_sz);
		memcpy(&data[epos], buf, line_sz);
		bpos += line_sz;
		epos -= line_sz;
	}

	return data;
}

}