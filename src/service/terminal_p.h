#ifndef QTSERVICE_TERMINAL_P_H
#define QTSERVICE_TERMINAL_P_H

#include "qtservice_global.h"
#include "terminal.h"

#include <QtCore/QDataStream>
#include <QtCore/QLoggingCategory>

#include <QtNetwork/QLocalSocket>

namespace QtService {

// exported as generally terminals are created from their private component
class Q_SERVICE_EXPORT TerminalPrivate : public QObject
{
	Q_OBJECT
	friend class QtService::Terminal;

public:
	enum RequestType {
		InvalidRequest = 0,

		CharRequest = 1,
		MultiCharRequest = 2,
		LineRequest = 3
	};
	Q_ENUM(RequestType)

	TerminalPrivate(QLocalSocket *socket, QObject *parent = nullptr);

Q_SIGNALS:
	void terminalReady(TerminalPrivate *terminal, bool successful);

private Q_SLOTS:
	void disconnected();
	void error();
	void readyRead();

private:
	QLocalSocket *socket;

	Service::TerminalMode terminalMode = Service::TerminalMode::ReadWriteActive;
	QStringList command;
	bool autoDelete = true;

	bool isLoading = true;
	QDataStream commandStream;
};

class TerminalAwaitablePrivate
{
	Q_DISABLE_COPY(TerminalAwaitablePrivate)
public:
	TerminalAwaitablePrivate(Terminal *terminal, qint64 readCnt);

	Terminal * const terminal;
	const qint64 readCnt;
	QMetaObject::Connection connection;
	QByteArray result;
};

Q_DECLARE_LOGGING_CATEGORY(logTerm)

}

#endif // QTSERVICE_TERMINAL_P_H
