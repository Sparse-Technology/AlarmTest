#include "alarmwidget.h"
#include "./ui_alarmwidget.h"

#include "httplib.h"

#include <QDebug>
#include <QJsonArray>
#include <QMessageBox>
#include <QJsonObject>
#include <QInputDialog>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QNetworkAccessManager>

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

	QGridLayout *layout = new QGridLayout;
	for (int i = 0; i < 16; i++) {
		QLabel *label = new QLabel(this);
		layout->addWidget(label, i / 4, i % 4, Qt::AlignCenter);
		alarmLabels << label;
		label->hide();
	}
	ui->tabWidget->widget(1)->setLayout(layout);
}

AlarmWidget::~AlarmWidget()
{
	delete ui;
}

void AlarmWidget::updateActiveEvents()
{
	const auto &m = trackingObjects[ui->comboCameras->currentText()];
	for (int i = 0; i < 16; i++)
		alarmLabels[i]->hide();
	int count = 0;
	for (const auto &[key, value]: m) {
		const auto &ba = QByteArray::fromBase64(value["image"].toString().toUtf8());
		auto image = QImage::fromData(ba);
		alarmLabels[count]->setToolTip(value["colorDistance"].toString());
		alarmLabels[count]->setPixmap(QPixmap::fromImage(image));
		alarmLabels[count++]->show();

	}
}

void AlarmWidget::handleNewEvent(const QJsonObject &obj)
{
	/* do nothing if this is not the currently selected camera */
	auto cam = obj["cameraName"].toString();
	if (cam != ui->comboCameras->currentText())
		return;

	/* update event details list */
	auto key = QString("%1").arg(obj["trackId"].toString());
	qDebug() << "new" << key;
	ui->listEvents->addItem(key);
	on_listEvents_currentRowChanged(ui->listEvents->count() - 1);

	/* update active event grid */
	updateActiveEvents();
}

void AlarmWidget::handleEventFinish(const QJsonObject &obj)
{
	/* do nothing if this is not the currently selected camera */
	auto cam = obj["cameraName"].toString();
	if (cam != ui->comboCameras->currentText())
		return;

	/* delete event from details */
	auto key = QString("%1").arg(obj["trackId"].toString());
	qDebug() << "finish" << key;
	for (int i = 0; i < ui->listEvents->count(); i++) {
		if (ui->listEvents->item(i)->text() == key) {
			delete ui->listEvents->takeItem(i);
			if (ui->listEvents->count())
				on_listEvents_currentRowChanged(0);
			else
				clearAlarm();
			return;
		}
	}

	/* update active event grid */
	updateActiveEvents();
}

void AlarmWidget::postRecved(const QString &data)
{
	const auto &doc = QJsonDocument::fromJson(data.toUtf8());
	const auto &arr = doc.array();

	if (!arr.size()) {
		qDebug() << "zero array size";
		return;
	}

	for (const auto &value: arr) {
		const auto &obj = value.toObject();
		auto type = obj["type"].toString();
		auto trackId = obj["trackId"].toString();
		auto cam = obj["cameraName"].toString();

		/* add camera name to the camera filter combo */
		if (ui->comboCameras->findText(cam) == -1)
			ui->comboCameras->addItem(cam);

		/* update our local alarm data structure */
		if (type == "start") {
			trackingObjects[cam][trackId] = obj;
			handleNewEvent(obj);
		} else if (type == "end") {
			if (!trackingObjects[cam].count(trackId)) {
				qDebug("no previous track event for '%s'", qPrintable(trackId));
			} else {
				trackingObjects[cam].erase(trackId);
				handleEventFinish(obj);
			}
		} else
			qDebug() << "unhandled event type" << type << obj;
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
}

void AlarmWidget::on_pushSubscribe_clicked()
{
	auto text = QInputDialog::getText(this, "Subscription", "Please enter your target IP");
	if (text.isEmpty())
		return;

	QString myip;
	if (myip.isEmpty()) {
		auto addresses = QNetworkInterface::allAddresses();
		for (auto &addr: addresses) {
			if (addr.isLoopback() || addr.isBroadcast() || addr.isLinkLocal())
				continue;
			myip = addr.toString();
			break;
		}
	}
	/* register ourselves */
	QNetworkRequest req(QUrl(QString("http://%1:50043/EventSub/Subscribe").arg(text)));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	QJsonObject obj;
	obj["url"] = QString("http://%1:24567/my_test_ep").arg(myip);
	auto *reply = nm.post(req, QJsonDocument(obj).toJson());
	connect(reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError err) {
		auto *reply = (QNetworkReply *)sender();
		QMessageBox::warning(this, "Subscription error", QString("Error '%1' subscribing to alarm server").arg(err));
		reply->deleteLater();
	});

	connect(reply, &QNetworkReply::finished, this, [this]() {
		auto *reply = (QNetworkReply *)sender();
		if (reply->error() == QNetworkReply::NoError)
			QMessageBox::warning(this, "Subscription", QString("Successfully subscribed to alarm server"));
		else
			QMessageBox::warning(this, "Subscription error", QString("Error '%1' subscribing to alarm server").arg(reply->error()));
		reply->deleteLater();
	});
}

void AlarmWidget::on_tabWidget_currentChanged(int index)
{
	if (index == 1)
		updateActiveEvents();
}

