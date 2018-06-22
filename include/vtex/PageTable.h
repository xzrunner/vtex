#pragma once

#include <boost/noncopyable.hpp>

#include <memory>
#include <vector>

namespace vtex
{

struct Page;

class PageTable : private boost::noncopyable
{
public:
	PageTable(int page_table_size);
	~PageTable();

	void AddPage(const Page& page, int mapping_x, int mapping_y);
	void RemovePage(const Page& page);

	void Update();

	uint32_t GetTexID() const { return m_texid; }

private:
	struct Image
	{
		~Image() {
			delete[] data;
		}

		int size = 0;
		uint8_t* data = nullptr;
	};

	struct Rect
	{
		Rect(int x, int y, int w, int h)
			: x(x), y(y), w(w), h(h) {}

		bool Contain(int px, int py) const {
			return px >= x && px < x + w
				&& py >= y && py < y + h;
		}

		int x, y;
		int w, h;
	};

	struct QuadNode : private boost::noncopyable
	{
		QuadNode(int level, const Rect& rect);

		Rect CalcChildRect(int idx) const;

		void Write(int size, uint8_t* data, int mip_level);

		int level;
		Rect rect;

		int mapping_x, mapping_y;

		std::unique_ptr<QuadNode> children[4];

	}; // QuadNode

private:
	QuadNode* FindPage(const Page& page, int& index) const;

private:
	int m_page_table_size;

	std::unique_ptr<QuadNode> m_root;

	std::vector<Image> m_data;

	uint32_t m_texid;

}; // PageTable

}