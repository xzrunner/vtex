#pragma once

#include "vtex/VirtualTextureInfo.h"

#include <string>
#include <fstream>
#include <mutex>

namespace vtex
{

class TileDataFile : private boost::noncopyable
{
public:
	TileDataFile(const std::string& filepath);
	~TileDataFile();

	void ReadInfo() const;
	void WriteInfo();

	void ReadPage(int index, uint8_t* data) const;
	void WritePage(int index, const uint8_t* data);

	const VirtualTextureInfo& GetVTexInfo() const { return m_vtex_info; }

public:
	static const int CHANNEL_COUNT = 4;

private:
	static const int DATA_OFFSET = 16;

private:
	mutable VirtualTextureInfo m_vtex_info;

	mutable size_t m_tile_size;

	mutable std::fstream m_file;

	mutable std::mutex m_mutex;

}; // TileDataFile

}