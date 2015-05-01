#pragma once

#include <QThread>
#include <QApplication>
#include <QMetaType>
#include <QTimer>

#include <vector>
#include <memory>
#include <cstdio>
#include <algorithm>

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
		printf("[%d]: worker %d initialized\n", reinterpret_cast<int>(currentThreadId()), workerIndex);
	}

	void DispatchWork(Work work)
	{
		emit WorkReceived(work);
	}

public slots:
	void DoWork(Work work)
	{
		printf("[%d]: worker %d received work %d\n", reinterpret_cast<int>(currentThreadId()), m_workerIndex, work.m_work);
		msleep(100);
		Result result = { work.m_work * 2, m_workerIndex };
		emit WorkDone(result);
	}

signals:
	void WorkReceived(Work work);
	void WorkDone(Result result);

private:
	int m_workerIndex;
};

class Master : public QObject
{
	Q_OBJECT

public:
	Master(int workerCount) : m_activeWorker(0), m_workerCount(workerCount)
	{
		printf("[%d]: creating master thread\n", reinterpret_cast<int>(QThread::currentThreadId()));
	}
	~Master()
	{
		std::for_each(m_workers.begin(), m_workers.end(), [](std::unique_ptr<Worker>& worker)
		{
			worker->quit();
			worker->wait();
		});
	}
	

public slots:
	void Initialize()
	{
		printf("[%d]: initializing master thread\n", reinterpret_cast<int>(QThread::currentThreadId()));
		for (int workerIndex = 0; workerIndex < m_workerCount; ++workerIndex)
		{
			auto worker = new Worker(workerIndex);
			m_workers.push_back(std::move(std::unique_ptr<Worker>(worker)));
			connect(worker, SIGNAL(WorkDone(Result)), SLOT(WorkDone(Result)));
			worker->start();
		}
		m_timer = new QTimer();
		m_timer->setInterval(500);
		connect(m_timer, SIGNAL(timeout()), SLOT(GenerateWork()));
		m_timer->start();
	}
	void GenerateWork()
	{
		Work work = { m_activeWorker };
		printf("[%d]: dispatching work %d to worker %d\n", reinterpret_cast<int>(QThread::currentThreadId()), work.m_work, m_activeWorker);
		m_workers[m_activeWorker]->DispatchWork(work);
		m_activeWorker = ++m_activeWorker % m_workers.size();
	}
	void WorkDone(Result result)
	{
		printf("[%d]: received result %d from worker %d\n", reinterpret_cast<int>(QThread::currentThreadId()), result.m_result, result.m_workerIndex);
	}
	void Terminate()
	{
		m_timer->stop();
		delete m_timer;
	}

private:
	int m_workerCount;
	std::vector<std::unique_ptr<Worker>> m_workers;
	int m_activeWorker;
	QTimer* m_timer;
};
