#include "alarmwidget.h"
#include "./ui_alarmwidget.h"

#include "httplib.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

using namespace httplib;

AlarmWidget::AlarmWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::AlarmWidget)
{
	ui->setupUi(this);

	connect(this, &AlarmWidget::newPostData, this, &AlarmWidget::postRecved);

	listenThread = std::thread([this](){
		httplib::Server *svr = new httplib::Server;
		svr->Post("/my_test_ep", [&](const auto& req, Response& res, const ContentReader &content_reader) {
			std::string body;
			content_reader([&](const char *data, size_t data_length) {
				body.append(data, data_length);
				return true;
			});
			emit newPostData(QString::fromStdString(body));
		});
		svr->listen("0.0.0.0", 24567);
	});
}

AlarmWidget::~AlarmWidget()
{
	delete ui;
}

void AlarmWidget::handleNewEvent(const QJsonObject &obj)
{
	auto key = QString("%1").arg(obj["trackId"].toString());
	qDebug() << "new" << key;
	ui->listEvents->addItem(key);
	on_listEvents_currentRowChanged(ui->listEvents->count() - 1);
}

void AlarmWidget::handleEventFinish(const QJsonObject &obj)
{
	auto key = QString("%1").arg(obj["trackId"].toString());
	qDebug() << "finish" << key;
	for (int i = 0; i < ui->listEvents->count(); i++) {
		if (ui->listEvents->item(i)->text() == key) {
			//qDebug() << ui->listEvents->item(i)->text() << key;
			delete ui->listEvents->takeItem(i);
			//ui->listEvents->removeItemWidget(ui->listEvents->item(i));
			if (ui->listEvents->count())
				on_listEvents_currentRowChanged(0);
			else
				clearAlarm();
			return;
		}
	}
}

void AlarmWidget::postRecved(const QString &data)
{
	const auto &doc = QJsonDocument::fromJson(data.toUtf8());
	const auto &arr = doc.array();

	if (!arr.size()) {
		//qDebug() << "zero array size";
		return;
	}
	//qDebug() << "new one";

	for (const auto &value: arr) {
		const auto &obj = value.toObject();
		auto event = obj["eventType"].toString();
		auto trackId = obj["trackId"].toString();
		auto cam = obj["cameraName"].toString();
		if (ui->comboCameras->findText(cam) == -1)
			ui->comboCameras->addItem(cam);
		if (cam != ui->comboCameras->currentText())
			continue;
		auto object = obj["objectType"].toString();
		//if (object == "person")
			//object = "no safety vest";
		if (object == "person" || object == "no hardhat") {
			//qDebug() << event << trackId;
			continue;
		}
		if (event == "track") {
			trackingObjects[cam][trackId] = obj;
			handleNewEvent(obj);
		} else if (event == "disappeared") {
			if (!trackingObjects[cam].count(trackId)) {
				qDebug("no previous track event for '%s'", qPrintable(trackId));
			} else {
				handleEventFinish(obj);
				trackingObjects[cam].erase(trackId);
			}
		} else
			qDebug() << "unhandled event type" << event;
		/* event: track, dissappear */
	}
}

void AlarmWidget::on_listEvents_currentRowChanged(int currentRow)
{
	if (currentRow < 0)
		return;
	auto key = ui->listEvents->item(currentRow)->text();
	auto obj = trackingObjects[ui->comboCameras->currentText()][key];
	const auto &ba = QByteArray::fromBase64(obj["image"].toString().toUtf8());
	obj.remove("image");
	auto image = QImage::fromData(ba);
	ui->labelImage->setPixmap(QPixmap::fromImage(image));
	ui->plainEventJson->setPlainText(QString::fromUtf8(QJsonDocument(obj).toJson()));
}

void AlarmWidget::clearAlarm()
{
	ui->labelImage->clear();
	ui->plainEventJson->clear();
	ui->listEvents->clear();
}

void AlarmWidget::on_comboCameras_currentIndexChanged(int index)
{
	clearAlarm();
	trackingObjects.clear();
}

