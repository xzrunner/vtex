#pragma once

#include "vtex/FeedbackBuffer.h"
#include "vtex/Page.h"
#include "vtex/TextureAtlas.h"
#include "vtex/PageIndexer.h"
#include "vtex/PageCache.h"
#include "vtex/PageTable.h"
#include "vtex/PageLoader.h"

#include <vector>
#include <functional>

namespace ur { class Shader; }

namespace vtex
{

class VirtualTexture : private boost::noncopyable
{
public:
	VirtualTexture(const std::string& filepath, const VirtualTextureInfo& info,
		int atlas_channel, int feedback_size, int virtual_tex_size);

	void Draw(std::function<void()> draw_cb);

	void ClearCache() { m_cache.Clear(); }
	PageLoader& GetPageLoader() { return m_loader; }

	void DecreaseMipBias();

private:
	void InitShaders();

	void Update(const std::vector<int>& requests);

private:
	struct PageWithCount
	{
		PageWithCount(const Page& page, int count)
			: page(page), count(count) {}

		Page page;
		int count = 0;
	};

private:
	int m_feedback_size;
	int m_virtual_tex_size;

	VirtualTextureInfo m_info;

	std::shared_ptr<ur::Shader> m_feedback_shader = nullptr;
	std::shared_ptr<ur::Shader> m_final_shader = nullptr;

	TextureAtlas m_atlas;

	PageIndexer  m_indexer;

	PageLoader   m_loader;

	PageTable    m_table;

	PageCache    m_cache;

	FeedbackBuffer m_feedback;

	std::vector<PageWithCount> m_toload;

	int m_mip_bias;

}; // VirtualTexture

}