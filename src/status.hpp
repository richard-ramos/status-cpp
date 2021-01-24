#ifndef STATUS_H
#define STATUS_H

#include <QString>
#include <QObject>
#include <QJSValue>
#include <QVariant>

class Status : public QObject {
  Q_OBJECT

public:
  ~Status() {}
  Q_INVOKABLE QString initKeystore();

  static Status *instance();

  enum SignalType { 
    Unknown,
    Message,
    EnvelopeSent,
    EnvelopeExpired,
    WhisperFilterAdded,
    Wallet,
    NodeLogin,
    NodeReady,
    NodeStarted,
    NodeStopped,
    MailserverRequestCompleted,
    MailserverRequestExpired,
    DiscoveryStarted,
    DiscoveryStopped,
    DiscoverySummary,
    SubscriptionsData,
    SubscriptionsError
  };
  
  Q_ENUM(SignalType)


  //Q_INVOKABLE void multiAccountStoreDerivedAccounts(QString accountId, QString password, const QJSValue &callback);



  Q_INVOKABLE QString generateAlias(QString publicKey);
  Q_INVOKABLE QString generateIdenticon(QString publicKey);
  Q_INVOKABLE void logout();

signals:
  void signal(SignalType signal);
  void login();

private:

  static Status *theInstance;
  
  explicit Status(QObject * parent = nullptr);
  static std::map<QString, SignalType> signalMap;
  static void statusGoEventCallback(const char *event);
};

#endif // STATUS_H