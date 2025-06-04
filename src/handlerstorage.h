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
  QString arguments;
};

class HandlerStorage : public QObject
{
  Q_OBJECT
public:
  HandlerStorage(const QString &storagePath, QObject *parent = nullptr);
  ~HandlerStorage();

  void clear();
  /// register the primary proxy handler
  void registerProxy(const QString &proxyPath);
  void registerModlProxy(const QString& proxyPath);
  /// register handler (for all games)
  void registerHandler(const QString &executable, const QString &arguments, bool prepend);
  /// register handler for specified games
  void registerHandler(const QStringList &games, const QString &executable, const QString &arguments, bool prepend, bool rereg);
  QStringList getHandler(const QString &game) const;
  std::vector<std::tuple<QString, QString, QString>> knownGames() const;
  std::list<HandlerInfo> handlers() const { return m_Handlers; }

  static QStringList stripCall(const QString &call);
private:
  void loadStore();
  void saveStore();
private:
  QString m_SettingsPath;
  std::list<HandlerInfo> m_Handlers;
};

#endif // HANDLERSTORAGE_H
