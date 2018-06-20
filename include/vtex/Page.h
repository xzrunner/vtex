#pragma once

namespace vtex
{

struct Page
{
	Page() {}
	Page(int x, int y, int mip)
		: x(x), y(y), mip(mip) {}

	bool operator == (const Page& p) const {
		return x == p.x && y == p.y && mip == p.mip;
	}

	int x = 0;
	int y = 0;
	int mip = 0;
};

}