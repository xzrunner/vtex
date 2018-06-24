#pragma once

namespace mt { class Task; }

namespace vtex
{

class Callback
{
public:
	struct Funs
	{
		void (*submit_task)(mt::Task*);
	};

	static void RegisterCallback(const Funs& funs);

	//////////////////////////////////////////////////////////////////////////

	static void SubmitTask(mt::Task*);

}; // Callback

}