#pragma once

#include "vtex/TileDataFile.h"
#include "vtex/Page.h"

#include <multitask/Task.h>

#include <boost/noncopyable.hpp>

#include <string>
#include <set>

namespace vtex
{

class PageIndexer;
class PageCache;

class PageLoader : boost::noncopyable
{
public:
	PageLoader(const std::string& filepath, const PageIndexer& indexer);

	void LoadPage(const Page& page, PageCache& cache);

	void Update();

	void ChangeShowBorder() { m_show_borders = !m_show_borders; }
	void ChangeShowMip() { m_show_mip = !m_show_mip; }

private:
	void LoadPageFromFile(const Page& page, uint8_t* pixels) const;

	void CopyBorder(uint8_t* pixels) const;
	void CopyColor(uint8_t* pixels, int mip) const;

private:
	class LoadPageTask : public mt::Task
	{
	public:
		LoadPageTask(PageLoader& loader, PageCache& cache, const Page& page);
		virtual ~LoadPageTask();

		virtual void Run() override;

		void Flush();

		void Initialize(const Page& page);
		void Terminate();

	private:
		PageLoader& m_loader;
		PageCache&  m_cache;

		Page m_page;

		uint8_t* m_pixels;

	}; // LoadPageTask

	class LoadPageTaskMgr
	{
	public:
		LoadPageTaskMgr() {}

		LoadPageTask* Fetch(PageLoader& loader,
			PageCache& cache, const Page& page);

		void AddResult(LoadPageTask* task) {
			m_result.Push(task);
		}

		bool IsEmpty() { return m_count == 0; }

		void Flush();

	private:
		int m_count = 0;

		mt::TaskQueue m_freelist;
		mt::SafeTaskQueue m_result;

	}; // LoadPageTaskMgr

private:
	void AddResult(LoadPageTask* task) {
		m_tasks.AddResult(task);
	}

private:
	const PageIndexer& m_indexer;

	TileDataFile m_file;

	bool m_show_borders;
	bool m_show_mip;

	LoadPageTaskMgr m_tasks;

	std::set<int> m_loading;

}; // PageLoader

}