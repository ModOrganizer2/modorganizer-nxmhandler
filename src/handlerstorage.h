#ifndef HANDLERSTORAGE_H
#define HANDLERSTORAGE_H

#include <QSettings>
#include <list>
#include <vector>
#include <QStringList>


struct HandlerInfo
{
  int ID;
  QStringList games;
  QString executable;
};

class HandlerStorage : public QObject
{
  Q_OBJECT
public:
  HandlerStorage(const QString &storagePath, QObject *parent = NULL);
  ~HandlerStorage();

  void clear();
  /// register the primary proxy handler
  void registerProxy(const QString &proxyPath);
  /// register handler (for all games)
  void registerHandler(const QString &executable, bool prepend);
  /// register handler for specified games
  void registerHandler(const QStringList &games, const QString &executable, bool prepend, bool rereg);
  QString getHandler(const QString &game) const;
  std::vector<std::pair<QString, QString> > knownGames() const;
  std::list<HandlerInfo> handlers() const { return m_Handlers; }

  /// strip a call as found as url handler of the call parameter(s) and quotes
  static QString stripCall(const QString &call);
private:
  void loadStore();
  void saveStore();
private:
  QString m_SettingsPath;
  std::list<HandlerInfo> m_Handlers;
};

#endif // HANDLERSTORAGE_H
