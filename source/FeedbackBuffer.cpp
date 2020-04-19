#include "vtex/FeedbackBuffer.h"

#include <unirender2/Device.h>
#include <unirender2/Framebuffer.h>
#include <unirender2/TextureDescription.h>
#include <painting2/RenderTarget.h>
#include <textile/PageIndexer.h>

#include <algorithm>

namespace vtex
{

FeedbackBuffer::FeedbackBuffer(const ur2::Device& dev, int size, int page_table_w,
                               int page_table_h, const textile::PageIndexer& indexer)
	: m_size(size)
	, m_page_table_w(page_table_w)
    , m_page_table_h(page_table_h)
	, m_indexer(indexer)
{
    ur2::TextureDescription desc;
    desc.target = ur2::TextureTarget::Texture2D;
    desc.width  = m_size;
    desc.height = m_size;
    desc.format = ur2::TextureFormat::RGBA8;
    m_fbo_col_tex = dev.CreateTexture(desc, nullptr);
    desc.format = ur2::TextureFormat::DEPTH;
    m_fbo_depth_tex = dev.CreateTexture(desc, nullptr);

    m_fbo = dev.CreateFramebuffer();
    m_fbo->SetAttachment(ur2::AttachmentType::Color0, ur2::TextureTarget::Texture2D, m_fbo_col_tex, nullptr);
    m_fbo->SetAttachment(ur2::AttachmentType::Depth, ur2::TextureTarget::Texture2D, m_fbo_depth_tex, nullptr);

	m_data = new uint8_t[m_size * m_size * 4];

	m_requests.resize(indexer.GetPageCount(), 0);
}

FeedbackBuffer::~FeedbackBuffer()
{
	delete[] m_data;
}

void FeedbackBuffer::BindRT()
{
    m_fbo->Bind();
}

void FeedbackBuffer::UnbindRT()
{
}

void FeedbackBuffer::Download(const ur2::Device& dev)
{
    dev.ReadPixels(m_data, ur2::TextureFormat::RGBA8, 0, 0, m_size, m_size);

    int page_table_size_log2 = static_cast<int>(std::log2(std::min(m_page_table_w, m_page_table_h)));
	for (int i = 0, n = m_size * m_size; i < n; ++i)
	{
		if (m_data[i * 4 + 3] == 255)
		{
			int x = static_cast<int>(m_data[i * 4]);
			int y = static_cast<int>(m_data[i * 4 + 1]);
			int mip = static_cast<int>(m_data[i * 4 + 2]);
			int count = page_table_size_log2 - mip + 1;
			for (int j = 0; j < count; ++j)
			{
				int _x = x >> j;
				int _y = y >> j;
				int _mip = mip + j;
				int idx = m_indexer.CalcPageIdx(textile::Page(_x, _y, _mip));
				++m_requests[idx];
			}

			// todo cache

		}
	}
}

void FeedbackBuffer::Clear()
{
	std::fill(m_requests.begin(), m_requests.end(), 0);
}

}