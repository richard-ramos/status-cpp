#include "mailserver-cycle.hpp"
#include "settings.hpp"
#include <QDebug>
#include <QThread>

MailserverCycle::MailserverCycle(QObject* parent)
	: QThread(parent)
{ }

MailserverCycle::~MailserverCycle()
{
	wait();
	qDebug() << "DELETING THREAD";
}

void MailserverCycle::work()
{
	if(!isRunning())
	{
		start(LowPriority);
	}
}

void MailserverCycle::run()
{
	// read all the settings at the start of the mailserver cycle
	QString pubKey = Settings::instance()->publicKey();
	QString currency = Settings::instance()->currency();

	qDebug() << "Executing mailserver thread...";
	qDebug() << currency;

	if(currency == "USD")
	{
		Settings::instance()->setCurrency("EUR");
	}
	else if(currency == "EUR")
	{
		Settings::instance()->setCurrency("DOP");
	}
	else
	{
		Settings::instance()->setCurrency("USD");
	}

	emit cycle();
    sleep(2000);
	qDebug() << "Finished execution";
}