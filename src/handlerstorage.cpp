#include "handlerstorage.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

static const QRegularExpression invalid_arguments("\"?%[0-9]+\"?");

HandlerStorage::HandlerStorage(const QString &storagePath, QObject *parent)
  : QObject(parent)
  , m_SettingsPath(storagePath + "/nxmhandler.ini")
{
  loadStore();
}

HandlerStorage::~HandlerStorage()
{
  saveStore();
}

void HandlerStorage::clear()
{
  m_Handlers.clear();
}

void HandlerStorage::registerProxy(const QString &proxyPath)
{
  QSettings settings("HKEY_CURRENT_USER\\Software\\Classes\\nxm\\", QSettings::NativeFormat);
  QString myExe = QString("\"%1\" ").arg(QDir::toNativeSeparators(proxyPath)).append("\"%1\"");
  settings.setValue("Default", "URL:NXM Protocol");
  settings.setValue("URL Protocol", "");
  settings.setValue("shell/open/command/Default", myExe);
  settings.sync();

  registerModlProxy(proxyPath);
}

void HandlerStorage::registerModlProxy(const QString& proxyPath)
{
  QSettings settings("HKEY_CURRENT_USER\\Software\\Classes\\modl\\", QSettings::NativeFormat);
  QString myExe = QString("\"%1\" ").arg(QDir::toNativeSeparators(proxyPath)).append("\"%1\"");
  settings.setValue("Default", "URL:MODL Protocol");
  settings.setValue("URL Protocol", "");
  settings.setValue("shell/open/command/Default", myExe);
  settings.sync();
}

void HandlerStorage::registerHandler(const QString &executable, const QString &arguments, bool prepend)
{
  QStringList games;
  for (const auto &game : this->knownGames()) {
    games.append(std::get<1>(game));
  }
  registerHandler(games, executable, arguments, prepend, false);
}

void HandlerStorage::registerHandler(const QStringList &games, const QString &executable, const QString &arguments, bool prepend, bool rereg)
{
  QStringList gamesLower;
  for (const QString &game : games) {
    gamesLower.append(game.toLower());
  }
  for (auto iter = m_Handlers.begin(); iter != m_Handlers.end(); ++iter) {
    if (iter->executable.compare(executable, Qt::CaseInsensitive) == 0) {
      // executable already registered, update supported games and move it to top if requested
      if (rereg) {
        HandlerInfo info = *iter;
        info.games = gamesLower;
        m_Handlers.erase(iter);
        if (prepend) {
          m_Handlers.push_front(info);
        } else {
          m_Handlers.push_back(info);
        }
      } else {
        iter->games.append(gamesLower);
        iter->games.removeDuplicates();
      }
      return; // important: in the rereg-case we changed the list thus screwing up the iterator
    }
  }

  // executable not yet registered
  HandlerInfo info;
  info.ID = static_cast<int>(m_Handlers.size());
  info.games = gamesLower;
  info.executable = executable;
  info.arguments = arguments;
  if (prepend) {
    m_Handlers.push_front(info);
  } else {
    m_Handlers.push_back(info);
  }
}

QStringList HandlerStorage::getHandler(const QString &game) const
{
  QString gameKey;
  QStringList results;

  auto games = knownGames();
  for (auto known : games) {
      if (game.compare(std::get<1>(known), Qt::CaseInsensitive) == 0 ||
          game.compare(std::get<2>(known), Qt::CaseInsensitive) == 0) {
          gameKey = std::get<1>(known);
      }
  }
  // look for an explictly registered handler
  for (const HandlerInfo &info : m_Handlers) {
    for (auto handler : info.games) {
        if (game.compare(handler, Qt::CaseInsensitive) == 0 ||
            gameKey.compare(handler, Qt::CaseInsensitive) == 0) {
            results << info.executable;
            results << info.arguments;
            return results;
        }
    }
  }

  // if no registered handler, look for the first "other" entry
  if (results.length() == 0) {
    for (const HandlerInfo &info : m_Handlers) {
      if (info.games.contains("other", Qt::CaseInsensitive)) {
        results << info.executable;
        results << info.arguments;
        return results;
      }
    }
  }

  // provide empty results if no handler was found
  while (results.length() < 2) {
    results << "";
  }

  return results;
}

std::vector<std::tuple<QString, QString, QString>> HandlerStorage::knownGames() const
{
  return {
    std::make_tuple<QString, QString, QString>("Morrowind", "morrowind", "morrowind"),
    std::make_tuple<QString, QString, QString>("Oblivion", "oblivion", "oblivion"),
    std::make_tuple<QString, QString, QString>("Fallout 3", "fallout3", "fallout3"),
    std::make_tuple<QString, QString, QString>("Fallout 4", "fallout4", "fallout4"),
    std::make_tuple<QString, QString, QString>("Fallout NV", "falloutnv", "newvegas"),
    std::make_tuple<QString, QString, QString>("Skyrim", "skyrim", "skyrim"),
    std::make_tuple<QString, QString, QString>("SkyrimSE", "skyrimse", "skyrimspecialedition"),
    std::make_tuple<QString, QString, QString>("Enderal", "enderal", "enderal"),
    std::make_tuple<QString, QString, QString>("EnderalSE", "enderalse", "enderalspecialedition"),
    std::make_tuple<QString, QString, QString>("Other", "other", "other")
  };
}

QStringList HandlerStorage::stripCall(const QString &call)
{
  // results[0] is binary, results[1..n] are optional arguments
  // guarenteed to return at least 2 items
  QStringList results;

  bool in_quote = false;
  QString word;
  for( QString::const_iterator iter = call.begin(); iter != call.end(); iter++ ){
    // Handle quotes
    if (*iter == '"') {
      if (!in_quote) {
        in_quote = true;
      }
      else {
        in_quote = false;
      }
    }

    // Space outside a quote means the end of a word
    if (*iter == ' ' && !in_quote) {
      // Don't process stuff like %1, %2, etc.
      if (!word.contains(invalid_arguments)) {
        results << word;
      }
      word = "";
      continue; //skip space
    }

    // Made it here? Add to the word
    word += *iter;
    }

  // Add the last word to the results if needed
  if (!word.isEmpty()) {
    if (!word.contains(invalid_arguments)) {
      results << word;
    }
    word = "";
  }

  // Remove quotes around first word for hacky reasons
  if (!results.isEmpty()) {
    results.replace(0, results.front().remove(QRegularExpression("(^\"|\"$)")));
  }

  while (results.length() < 2) {
    results.append(QString(""));
  }

  return results;
}

void HandlerStorage::loadStore()
{
  // register configured handlers
  QSettings settings(m_SettingsPath, QSettings::IniFormat);
  int size = settings.beginReadArray("handlers");
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    HandlerInfo info;
    info.ID = i;
    QString gameList = settings.value("games").toString();
    if (!gameList.isEmpty()) {
      info.games = gameList.split(",");
    }
    info.executable = settings.value("executable").toString();
    info.arguments = settings.value("arguments").toString();
    if (QFile::exists(info.executable)) {
      m_Handlers.push_back(info);
    }
  }
  settings.endArray();


  // also register the global handler
  HandlerInfo info;
  QSettings handlerReg("HKEY_CLASSES_ROOT\\nxm\\", QSettings::NativeFormat);
  QStringList handlerValues(stripCall(handlerReg.value("shell/open/command/Default").toString()));

  info.ID = static_cast<int>(m_Handlers.size());
  auto games = knownGames();
  QStringList ids;
  for (auto iter = games.begin(); iter != games.end(); ++iter) {
    ids.append(std::get<1>(*iter));
  }
  info.games = QStringList() << ids;
  info.executable = handlerValues.front();
  handlerValues.pop_front();
  info.arguments = handlerValues.join(" ");
  if (!info.executable.isEmpty() && !info.executable.endsWith("nxmhandler.exe", Qt::CaseInsensitive)) {
    bool known = false;
    for (auto iter = m_Handlers.begin(); iter != m_Handlers.end(); ++iter) {
      if ((iter->executable == info.executable) && (iter->arguments  == info.arguments)) {
        known = true;
      }
    }
    if (!known) {
      m_Handlers.push_back(info);
    }
  }
}

void HandlerStorage::saveStore()
{
  QSettings settings(m_SettingsPath, QSettings::IniFormat);
  settings.remove("handlers");
  settings.beginWriteArray("handlers", static_cast<int>(m_Handlers.size()));
  int i = 0;
  for (auto iter = m_Handlers.begin(); iter != m_Handlers.end(); ++iter) {
    settings.setArrayIndex(i++);
    settings.setValue("games", iter->games.join(","));
    settings.setValue("executable", iter->executable);
    settings.setValue("arguments", iter->arguments);
  }
  settings.endArray();
}
