#pragma once

#include <QThread>
#include <QObject>

class MailserverCycle : public QThread
{
	Q_OBJECT

public:
	MailserverCycle(QObject* parent = nullptr);
	~MailserverCycle();

	Q_INVOKABLE void work();

signals:
	void cycle();
    void loopStopped();


protected:
	void run() override;

};