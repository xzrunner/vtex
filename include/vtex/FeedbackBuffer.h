#pragma once

#include <unirender2/typedef.h>

#include <boost/noncopyable.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace textile { class PageIndexer; }
namespace ur2 { class Device; class Framebuffer; }

namespace vtex
{

class FeedbackBuffer : private boost::noncopyable
{
public:
	FeedbackBuffer(const ur2::Device& dev, int size, int page_table_w,
        int page_table_h, const textile::PageIndexer& indexer);
	~FeedbackBuffer();

	void BindRT();
	void UnbindRT();

	void Download(const ur2::Device& dev);

	const std::vector<int>& GetRequests() const { return m_requests; }

	void Clear();

    auto GetTexture() const { return m_fbo_col_tex; }

private:
	const textile::PageIndexer& m_indexer;

	int m_size;
	int m_page_table_w, m_page_table_h;

    ur2::TexturePtr m_fbo_col_tex = nullptr;
    ur2::TexturePtr m_fbo_depth_tex = nullptr;
    std::shared_ptr<ur2::Framebuffer> m_fbo = nullptr;
	uint8_t* m_data;

	std::vector<int> m_requests;

}; // FeedbackBuffer

}