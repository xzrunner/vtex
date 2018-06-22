#include "vtex/PageLoader.h"
#include "vtex/PageIndexer.h"

namespace vtex
{

PageLoader::PageLoader(const std::string& filepath, const PageIndexer& indexer)
	: m_indexer(indexer)
	, m_file(filepath)
	, m_show_borders(false)
	, m_show_mip(false)
{
}

const uint8_t* PageLoader::LoadPage(const Page& page)
{
	auto& info = m_file.GetVTexInfo();
	int size = info.PageSize() * info.PageSize() * TileDataFile::CHANNEL_COUNT;

	uint8_t* pixels = new uint8_t[size];
	if (!pixels) {
		return nullptr;
	}

	if (m_show_mip) {
		CopyColor(pixels, page.mip);
	} else {
		LoadPageFromFile(page, pixels);
	}

	if (m_show_borders) {
		CopyBorder(pixels);
	}

	return pixels;
}

void PageLoader::LoadPageFromFile(const Page& page, uint8_t* pixels)
{
	auto& info = m_file.GetVTexInfo();

	// revert y
	int page_table_size_log2 = static_cast<int>(std::log2(info.PageTableSize()));
	int count = page_table_size_log2 - page.mip + 1;
	int y = std::pow(2, count - 1) - 1 - page.y;

	m_file.ReadPage(m_indexer.CalcPageIdx(Page(page.x, y, page.mip)), pixels);

	// revert y
	int line_sz = info.PageSize() * TileDataFile::CHANNEL_COUNT;
	uint8_t* buf = new uint8_t[line_sz];
	int bpos = 0, epos = line_sz * (info.PageSize() - 1);
	for (int i = 0, n = (int)(floorf(info.PageSize() / 2.0f)); i < n; ++i)
	{
		memcpy(buf, &pixels[bpos], line_sz);
		memcpy(&pixels[bpos], &pixels[epos], line_sz);
		memcpy(&pixels[epos], buf, line_sz);
		bpos += line_sz;
		epos -= line_sz;
	}
}

void PageLoader::CopyBorder(uint8_t* pixels) const
{
	int channel = TileDataFile::CHANNEL_COUNT;

	int pagesize = m_file.GetVTexInfo().PageSize();
	int bordersize = m_file.GetVTexInfo().border_size;

	for (int i = 0; i < pagesize; ++i)
	{
		int xindex = bordersize * pagesize + i;
		pixels[xindex * channel + 0] = 0;
		pixels[xindex * channel + 1] = 255;
		pixels[xindex * channel + 2] = 0;
		pixels[xindex * channel + 3] = 255;

		int yindex = i * pagesize + bordersize;
		pixels[yindex * channel + 0] = 0;
		pixels[yindex * channel + 1] = 255;
		pixels[yindex * channel + 2] = 0;
		pixels[yindex * channel + 3] = 255;
	}
}

void PageLoader::CopyColor(uint8_t* pixels, int mip) const
{
	int channel = TileDataFile::CHANNEL_COUNT;

	static const uint8_t colors[][4] =
	{
		{   0,   0, 255, 255 },
		{   0, 255, 255, 255 },
		{ 255,   0,   0, 255 },
		{ 255,   0, 255, 255 },
		{ 255, 255,   0, 255 },
		{  64,  64, 192, 255 },

		{  64, 192,  64, 255 },
		{  64, 192, 192, 255 },
		{ 192,  64,  64, 255 },
		{ 192,  64, 192, 255 },
		{ 192, 192,  64, 255 },
		{   0, 255,   0, 255 }
	};

	int pagesize = m_file.GetVTexInfo().PageSize();

	for (int y = 0; y < pagesize; ++y) {
		for (int x = 0; x < pagesize; ++x) {
			pixels[(y * pagesize + x)*channel + 0] = colors[mip][0];
			pixels[(y * pagesize + x)*channel + 1] = colors[mip][1];
			pixels[(y * pagesize + x)*channel + 2] = colors[mip][2];
			pixels[(y * pagesize + x)*channel + 3] = colors[mip][3];
		}
	}
}

}