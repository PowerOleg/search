#pragma once
#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <syncstream>
#include <iostream>

//namespace asio = boost::asio;

class Thread_pool
{
public:
	Thread_pool(size_t pool_size) : prodWork{ boost::asio::make_work_guard(io_context) }
	{
		/*boost::asio::io_context workerIO;
		boost::thread workerThread;
		boost::asio::executor_work_guard
			<boost::asio::io_context::executor_type, void, void> prodWork = boost::asio::make_work_guard(workerIO);*/
		for (size_t i = 0; i < pool_size; i++)
		{
			threads.create_thread([this]() {io_context.run(); });
		}
	
	}

	~Thread_pool()
	{
		io_context.stop();
		threads.join_all();
	}

	template <typename Task>
	void Enqueue(Task task)
	{
		io_context.post(task);
	}

private:
	boost::asio::io_context io_context;

	boost::thread_group threads;
	//boost::thread workerThread;
	

	//boost::asio::io_context::work work;//deprecated
	boost::asio::executor_work_guard
		<boost::asio::io_context::executor_type, void, void> prodWork;
};
