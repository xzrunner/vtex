#pragma once

#include <boost/noncopyable.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace pt2 { class RenderTarget; }

namespace vtex
{

class PageIndexer;

class FeedbackBuffer : private boost::noncopyable
{
public:
	FeedbackBuffer(int size, int page_table_size,
		const PageIndexer& indexer);
	~FeedbackBuffer();

	void BindRT();
	void UnbindRT();

	void Download();

	const std::vector<int>& GetRequests() const { return m_requests; }

	void Clear();

	uint32_t GetTexID() const;

private:
	const PageIndexer& m_indexer;

	int m_size;
	int m_page_table_size;

	std::unique_ptr<pt2::RenderTarget> m_rt = nullptr;
	uint8_t* m_data;

	std::vector<int> m_requests;

}; // FeedbackBuffer

}