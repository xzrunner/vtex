#pragma once

#include <boost/noncopyable.hpp>

namespace vtex
{

struct VirtualTextureInfo : private boost::noncopyable
{
	int virtual_texture_size = 0;
	int tile_size = 0;
	int border_size = 0;

	VirtualTextureInfo() {}
	VirtualTextureInfo(const VirtualTextureInfo& info)
		: virtual_texture_size(info.virtual_texture_size), tile_size(info.tile_size), border_size(info.border_size) {}
	VirtualTextureInfo(int virtual_texture_size, int tile_size, int border_size)
		: virtual_texture_size(virtual_texture_size), tile_size(tile_size), border_size(border_size) {}

	int PageSize() const { return tile_size + 2 * border_size; }
	int PageTableSize() const { return virtual_texture_size / tile_size; }

}; // VirtualTextureInfo

}