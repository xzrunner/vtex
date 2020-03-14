#include "vtex/FeedbackBuffer.h"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <painting2/RenderTarget.h>
#include <textile/PageIndexer.h>

#include <algorithm>

namespace vtex
{

FeedbackBuffer::FeedbackBuffer(int size, int page_table_w, int page_table_h,
	                           const textile::PageIndexer& indexer)
	: m_size(size)
	, m_page_table_w(page_table_w)
    , m_page_table_h(page_table_h)
	, m_indexer(indexer)
{
	m_rt = std::make_unique<pt2::RenderTarget>(m_size, m_size, true);
	m_data = new uint8_t[m_size * m_size * 4];

	m_requests.resize(indexer.GetPageCount(), 0);
}

FeedbackBuffer::~FeedbackBuffer()
{
	delete[] m_data;
}

void FeedbackBuffer::BindRT()
{
	m_rt->Bind();
}

void FeedbackBuffer::UnbindRT()
{
	m_rt->Unbind();
}

void FeedbackBuffer::Download()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	rc.ReadPixels(m_data, 4, 0, 0, m_size, m_size);

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

uint32_t FeedbackBuffer::GetTexID() const
{
	return m_rt->GetTexID();
}

}