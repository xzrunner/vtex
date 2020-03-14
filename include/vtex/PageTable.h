#pragma once

#include <boost/noncopyable.hpp>

#include <memory>
#include <vector>

namespace textile { struct Page; }

namespace vtex
{

class PageTable : private boost::noncopyable
{
public:
	PageTable(int width, int height);
	~PageTable();

	void AddPage(const textile::Page& page, int mapping_x, int mapping_y);
	void RemovePage(const textile::Page& page);

	void Update();

	uint32_t GetTexID() const { return m_texid; }

private:
	struct Image
	{
		~Image() {
			delete[] data;
		}

		int w = 0, h = 0;
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

		void Write(int w, int h, uint8_t* data, int mip_level);

		int level;
		Rect rect;

		int mapping_x, mapping_y;

		std::unique_ptr<QuadNode> children[4];

	}; // QuadNode

private:
	QuadNode* FindPage(const textile::Page& page, int& index) const;

    size_t CalcMaxLevel() const;

private:
	int m_width, m_height;

	std::unique_ptr<QuadNode> m_root;

	std::vector<Image> m_data;

	uint32_t m_texid;

}; // PageTable

}