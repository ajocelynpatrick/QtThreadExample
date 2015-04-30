#pragma once

#include <QThread>
#include <QApplication>
#include <QMetaType>

#include <vector>
#include <memory>
#include <iostream>

struct Work
{
	int m_work;
};

struct Result
{
	int m_result;
	int m_workerIndex;
};

Q_DECLARE_METATYPE(Work);
Q_DECLARE_METATYPE(Result);

class Worker : public QThread
{
	Q_OBJECT

public:
	Worker(int workerIndex) : m_workerIndex(workerIndex)
	{
		moveToThread(this);
		connect(this, SIGNAL(WorkReceived(Work)), SLOT(DoWork(Work)));
	}

	void DispatchWork(Work work)
	{
		emit WorkReceived(work);
	}

public slots:
	void DoWork(Work work)
	{
		std::cout << "[" << reinterpret_cast<int>(currentThreadId()) << "]: Worker " << m_workerIndex << " received work " << work.m_work << std::endl;
		msleep(500);
		Result result = { work.m_work * 2, m_workerIndex };
		emit WorkDone(result);
	}

signals:
	void WorkReceived(Work work);
	void WorkDone(Result result);

private:
	int m_workerIndex;
};

class Master : public QThread
{
	Q_OBJECT

public:
	Master(int nWorkerCount)
	{
		for (int nWorkerIndex = 0; nWorkerIndex < nWorkerCount; ++nWorkerIndex)
		{
			auto pWorker = new Worker(nWorkerIndex);
			m_workers.push_back(std::move(std::unique_ptr<Worker>(pWorker)));
			connect(pWorker, SIGNAL(WorkDone(Result)), SLOT(WorkDone(Result)));
			pWorker->start();
		}
	}
	void run()
	{

		int activeWorker = 0;
		while (true)
		{
			Work work = { activeWorker };
			std::cout << "[" << reinterpret_cast<int>(currentThreadId()) << "]: Dispatching work to worker " << activeWorker << std::endl;
			m_workers[activeWorker]->DoWork(work);
			activeWorker = ++activeWorker % m_workers.size();
		}

	}

public slots:
	void WorkDone(Result result)
	{
		std::cout << "[" << reinterpret_cast<int>(currentThreadId()) <<"]: Received result " << result.m_result << " from worker " << result.m_workerIndex << std::endl;
	}

private:
	std::vector<std::unique_ptr<Worker>> m_workers;
};
