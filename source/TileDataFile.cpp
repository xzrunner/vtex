#include "vtex/TileDataFile.h"

namespace vtex
{

TileDataFile::TileDataFile(const std::string& filepath)
{
	m_file.open(filepath.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

	ReadInfo();
}

TileDataFile::~TileDataFile()
{
	m_file.close();
}

void TileDataFile::ReadInfo()
{
	m_file.seekg(0, std::ios_base::beg);
	m_file.read(reinterpret_cast<char*>(&m_vtex_info), sizeof(m_vtex_info));

	m_tile_size = m_vtex_info.PageSize() * m_vtex_info.PageSize() * CHANNEL_COUNT;
}

void TileDataFile::WriteInfo()
{
	m_file.seekp(0, std::ios_base::beg);
	m_file.write(reinterpret_cast<char*>(&m_vtex_info), sizeof(m_vtex_info));
}

void TileDataFile::ReadPage(int index, uint8_t* data)
{
	m_file.seekg(DATA_OFFSET + m_tile_size * index);
	m_file.read(reinterpret_cast<char*>(data), m_tile_size);
}

void TileDataFile::WritePage(int index, const uint8_t* data)
{
	m_file.seekp(DATA_OFFSET + m_tile_size * index);
	m_file.write(reinterpret_cast<const char*>(data), m_tile_size);
}

}