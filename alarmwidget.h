#ifndef ALARMWIDGET_H
#define ALARMWIDGET_H

#include <QWidget>
#include <QJsonObject>
#include <QNetworkAccessManager>

#include <thread>

QT_BEGIN_NAMESPACE
namespace Ui { class AlarmWidget; }
QT_END_NAMESPACE

class QLabel;

class AlarmWidget : public QWidget
{
	Q_OBJECT

public:
	AlarmWidget(QWidget *parent = nullptr);
	~AlarmWidget();

protected:
	void updateActiveEvents();
	void handleNewEvent(const QJsonObject &obj);
	void handleEventFinish(const QJsonObject &obj);
signals:
	void newPostData(const QString &data);

protected slots:
	void postRecved(const QString &data);

private slots:
	void on_listEvents_currentRowChanged(int currentRow);

	void on_comboCameras_currentIndexChanged(int index);

	void on_pushSubscribe_clicked();

	void on_tabWidget_currentChanged(int index);

protected:
	void clearAlarm();

private:
	Ui::AlarmWidget *ui;
	std::thread listenThread;
	QList<QLabel *> alarmLabels;
	QNetworkAccessManager nm;
	std::unordered_map<QString, std::unordered_map<QString, QJsonObject>> trackingObjects;
};
#endif // ALARMWIDGET_H
