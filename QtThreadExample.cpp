#include "QtThreadExample.hpp"

int main(int argc, char** argv)
{
	qRegisterMetaType<Work>("Work");
	qRegisterMetaType<Result>("Result");
	QApplication application(argc, argv);
	Master master(5);
	master.start();
	application.exec();
	return 0;
}