#include "QtThreadExample.hpp"
#include <QTimer>

int main(int argc, char** argv)
{
	qRegisterMetaType<Work>("Work");
	qRegisterMetaType<Result>("Result");
	QApplication application(argc, argv);
	QThread masterThread;
	Master master(5);
	master.moveToThread(&masterThread);
	master.connect(&masterThread, SIGNAL(started()), SLOT(Initialize()));
	master.connect(&masterThread, SIGNAL(terminated()), SLOT(Terminate()));
	masterThread.start();
	// Set a timer to terminate the program after 10 seconds
	QTimer::singleShot(10 * 1000, &application, SLOT(quit()));
	application.exec();
	masterThread.quit();
	masterThread.wait();
	printf("[%d]: master thread has finished\n", reinterpret_cast<int>(QThread::currentThreadId()));
	return 0;
}