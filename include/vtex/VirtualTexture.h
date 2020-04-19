#pragma once

#include "vtex/FeedbackBuffer.h"
#include "vtex/TextureAtlas.h"
#include "vtex/PageCache.h"
#include "vtex/PageTable.h"

#include <textile/Page.h>
#include <textile/VTexInfo.h>
#include <textile/PageIndexer.h>
#include <textile/PageLoader.h>

#include <vector>
#include <functional>

namespace ur2 { class Device; class Context; class ShaderProgram; }

namespace vtex
{

class VirtualTexture : private boost::noncopyable
{
public:
	VirtualTexture(const ur2::Device& dev, const std::string& filepath,
        const textile::VTexInfo& info, int atlas_channel, int feedback_size);

	void Draw(const ur2::Device& dev, ur2::Context& ctx,
        std::function<void()> draw_cb);

	void ClearCache() { m_cache.Clear(); }
	textile::PageLoader& GetPageLoader() { return m_loader; }

	void DecreaseMipBias();

    auto Width() const { return m_vtex_w; }
    auto Height() const { return m_vtex_h; }

private:
	void InitShaders(const ur2::Device& dev);

	void Update(const ur2::Device& dev,
        const std::vector<int>& requests);

private:
	struct PageWithCount
	{
		PageWithCount(const textile::Page& page, int count)
			: page(page), count(count) {}

		textile::Page page;
		int count = 0;
	};

private:
	int m_feedback_size;
	int m_vtex_w, m_vtex_h;

	textile::VTexInfo m_info;

	std::shared_ptr<ur2::ShaderProgram> m_feedback_shader = nullptr;
	std::shared_ptr<ur2::ShaderProgram> m_final_shader = nullptr;

	TextureAtlas m_atlas;

	textile::PageIndexer m_indexer;
	textile::PageLoader  m_loader;

	PageTable m_table;
	PageCache m_cache;

	FeedbackBuffer m_feedback;

	std::vector<PageWithCount> m_toload;

	int m_mip_bias;

}; // VirtualTexture

}