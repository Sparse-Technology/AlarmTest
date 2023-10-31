#include "alarmwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	AlarmWidget w;
	w.show();
	return a.exec();
}
