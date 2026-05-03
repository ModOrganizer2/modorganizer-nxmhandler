#include "handlerstorage.h"
#include "handlerwindow.h"
#include "logger.h"
#include <QAbstractButton>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrlQuery>
#include <uibase/nxmurl.h>
#include <uibase/utility.h>

#pragma comment(                                                                       \
    linker,                                                                            \
    "/manifestDependency:\"name='dlls' processorArchitecture='x86' version='1.0.0.0' type='win32' \"")

using MOBase::ToWString;

static QString g_LogFileName = "";

void logHandler(QtMsgType type, const QMessageLogContext& context,
                const QString& message)
{
  if (g_LogFileName.isEmpty()) {
    return;
  }

  QFile file(g_LogFileName);

  // Prevent the log from growing infinitely
  if (file.size() > 10 * 1024 * 1024) {
    if (!file.open(QIODevice::WriteOnly)) {
      return;
    }
  } else if (!file.open(QIODevice::Append)) {
    return;
  }

  file.write(qUtf8Printable(
      QString("[%1] %2\n").arg(QDateTime::currentDateTime().toString()).arg(message)));
}

void handleNxmLink(const QString& executable, const QString& arguments,
                   const QString& link)
{
  QString quotedExecutable(executable);
  if (!quotedExecutable.contains(QRegularExpression("^\".*\"$"))) {
    quotedExecutable = '"' + quotedExecutable + '"';
  }

  QString quotedLink(link);
  if (!quotedLink.contains(QRegularExpression("^\".*\"$"))) {
    quotedLink = '"' + quotedLink + '"';
  }

  ::ShellExecute(nullptr, TEXT("open"), ToWString(quotedExecutable).c_str(),
                 ToWString(arguments + " " + quotedLink).c_str(),
                 ToWString(QFileInfo(quotedExecutable).absolutePath()).c_str(),
                 SW_SHOWNORMAL);
}

void handleModlLink(const QString& executable, const QString& arguments,
                    const QString& link)
{
  QString quotedExecutable(executable);
  if (!quotedExecutable.contains(QRegularExpression("^\".*\"$"))) {
    quotedExecutable = '"' + quotedExecutable + '"';
  }

  QString quotedLink(link);
  if (!quotedLink.contains(QRegularExpression("^\".*\"$"))) {
    quotedLink = '"' + quotedLink + '"';
  }

  ::ShellExecute(nullptr, TEXT("open"), ToWString(quotedExecutable).c_str(),
                 ToWString("download " + arguments + " " + quotedLink).c_str(),
                 ToWString(QFileInfo(quotedExecutable).absolutePath()).c_str(),
                 SW_SHOWNORMAL);
}

HandlerStorage* registerNxmExecutable(const QDir& storagePath,
                                      const QString& handlerPath,
                                      const QString& handlerArgs)
{
  HandlerStorage* storage = nullptr;
  if (!handlerPath.isEmpty() &&
      !handlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive)) {
    // a foreign or global nxm handler, register ourself and use that handler as
    // an option - if this is another nxmhandler we could run into problems so
    // skip it
    storage = new HandlerStorage(storagePath.path());
    storage->registerHandler("nxm", handlerPath, handlerArgs, false);
    storage->registerNxmProxy(QCoreApplication::applicationFilePath());
  } else {
    // no handler registered yet or the existing handler is invalid -> overwrite
    storage = new HandlerStorage(storagePath.path());
    storage->registerNxmProxy(QCoreApplication::applicationFilePath());
  }
  return storage;
}

HandlerStorage* registerModlExecutable(const QDir& storagePath,
                                       const QString& handlerPath,
                                       const QString& handlerArgs)
{
  HandlerStorage* storage = nullptr;
  if (!handlerPath.isEmpty() &&
      !handlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive)) {
    // a foreign or global nxm handler, register ourself and use that handler as
    // an option - if this is another nxmhandler we could run into problems so skip it
    storage = new HandlerStorage(storagePath.path());
    storage->registerHandler("modl", handlerPath, handlerArgs, false);
    storage->registerModlProxy(QCoreApplication::applicationFilePath());
  } else {
    // no handler registered yet or the existing handler is invalid -> overwrite
    storage = new HandlerStorage(storagePath.path());
    storage->registerModlProxy(QCoreApplication::applicationFilePath());
  }
  return storage;
}

// ensure a nxmhandler.exe is registered to handle nxm-links, then load the
// handler storage for that registered instance
// (even if it's different from the one actually being run)
HandlerStorage* loadStorage(bool forceReg)
{
  HandlerStorage* storage = nullptr;

  QDir globalStorage(
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  globalStorage.cd("../ModOrganizer");
  QDir baseDir;
  if (globalStorage.exists()) {
    baseDir = globalStorage;
  } else {
    baseDir = QDir(qApp->applicationDirPath());
  }
  NxmHandler::LoggerInit(baseDir.filePath("nxmhandler.log"));
  QSettings nxmHandlerReg("HKEY_CURRENT_USER\\Software\\Classes\\nxm\\",
                          QSettings::NativeFormat);
  QSettings modlHandlerReg("HKEY_CURRENT_USER\\Software\\Classes\\modl\\",
                           QSettings::NativeFormat);
  QStringList nxmHandlerVals = HandlerStorage::stripCall(
      nxmHandlerReg.value("shell/open/command/Default").toString());
  QStringList modlHandlerVals = HandlerStorage::stripCall(
      modlHandlerReg.value("shell/open/command/Default").toString());
  QString nxmHandlerPath  = nxmHandlerVals.front();
  QString modlHandlerPath = modlHandlerVals.front();
  nxmHandlerVals.pop_front();
  modlHandlerVals.pop_front();
  QString nxmHandlerArgs  = nxmHandlerVals.join(" ");
  QString modlHandlerArgs = modlHandlerVals.join(" ");

  QDir nxmHandlerBaseDir  = QFileInfo(nxmHandlerPath).absoluteDir();
  QDir modlHandlerBaseDir = QFileInfo(modlHandlerPath).absoluteDir();

  QSettings settings(baseDir.absoluteFilePath("nxmhandler.ini"), QSettings::IniFormat);
  bool noRegister = settings.value("noregister", false).toBool();
  if (globalStorage.exists("nxmhandler.ini") &&
      nxmHandlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive) &&
      modlHandlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive) &&
      QFile::exists(nxmHandlerPath)) {
    // global configuration avaible - use it
    storage = new HandlerStorage(globalStorage.path());
  } else if (nxmHandlerBaseDir.exists("nxmhandler.ini") &&
             nxmHandlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive) &&
             modlHandlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive) &&
             QFile::exists(nxmHandlerPath)) {
    // a portable installation is registered to handle links, use its
    // configuration
    storage = new HandlerStorage(QFileInfo(nxmHandlerPath).absolutePath());
    if (forceReg && (QString::compare(QDir::toNativeSeparators(
                                          QCoreApplication::applicationFilePath()),
                                      nxmHandlerPath, Qt::CaseInsensitive))) {
      if (QMessageBox::question(
              nullptr, QObject::tr("Change Handler?"),
              QObject::tr("A nxm handler from a different Mod Organizer "
                          "installation has been registered. Do you want to "
                          "replace it? This is usually not necessary unless "
                          "the other installation is defective."),
              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        storage->registerNxmProxy(QCoreApplication::applicationFilePath());
      }
    }
  } else if (!noRegister || forceReg) {
    // no nxm registration
    if (!nxmHandlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive)) {
      QMessageBox registerBox(
          QMessageBox::Question, QObject::tr("Register?"),
          QObject::tr("Mod Organizer is not set up to handle nxm links. "
                      "Associate it with nxm links?"),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Save);
      registerBox.button(QMessageBox::Save)
          ->setText(QObject::tr("No, don't ask again"));
      switch (registerBox.exec()) {
      case QMessageBox::Yes: {
        // base dir is either the global dir if it exists or the local application
        // dir
        storage = registerNxmExecutable(baseDir, nxmHandlerPath, nxmHandlerArgs);
      } break;
      case QMessageBox::Save: {
        settings.setValue("noregister", true);
      } break;
      case QMessageBox::No: {
        settings.setValue("noregister", false);
      } break;
      }
    }
    if (!modlHandlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive)) {
      QMessageBox registerBox(
          QMessageBox::Question, QObject::tr("Register?"),
          QObject::tr("Mod Organizer is not set up to handle modl links. "
                      "Associate it with modl links?"),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Save);
      registerBox.button(QMessageBox::Save)
          ->setText(QObject::tr("No, don't ask again"));
      switch (registerBox.exec()) {
      case QMessageBox::Yes: {
        // base dir is either the global dir if it exists or the local application
        // dir
        storage = registerModlExecutable(baseDir, nxmHandlerPath, nxmHandlerArgs);
      } break;
      case QMessageBox::Save: {
        settings.setValue("noregister", true);
      } break;
      case QMessageBox::No: {
        settings.setValue("noregister", false);
      } break;
      }
    }
  }
  return storage;
}

static void applyChromeFix()
{
  QString dataPath = QDir::fromNativeSeparators(
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  QString fileName =
      QDir(dataPath + "/../google/chrome/user data/local state").canonicalPath();

  QFile chromeLocalState(fileName);

  if (!chromeLocalState.exists()) {
    // probably simply means that chrome isn't installed
    return;
  }

  if (!chromeLocalState.open(QIODevice::ReadOnly)) {
    // don't know, still no reason to report an error
    return;
  }

  QJsonParseError parseError;
  QJsonDocument document =
      QJsonDocument::fromJson(chromeLocalState.readAll(), &parseError);
  chromeLocalState.close();
  if (parseError.error == QJsonParseError::NoError) {
    QJsonObject docMap   = document.object();
    QJsonObject handlers = docMap["protocol_handler"].toObject();
    // toObject returns empty object if the key doesn't exist. Therefore if
    // excluded_schemes exists, protocol_handler existed as well
    if (handlers.contains("excluded_schemes")) {
      QJsonObject schemes = handlers["excluded_schemes"].toObject();
      if (schemes["nxm"].toBool(true) || schemes["modl"].toBool(true)) {
        if (QMessageBox::question(nullptr, "Apply Chrome fix",
                                  "Chrome may not support nexus links even though the "
                                  "association is set up correctly. "
                                  "Do you want to apply a fix for that (You have to "
                                  "close chrome before pressing yes or "
                                  "this will have no effect!)?",
                                  QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::No) {
          return;
        }
        schemes["nxm"]               = false;
        schemes["modl"]              = false;
        handlers["excluded_schemes"] = schemes;
        docMap["protocol_handler"]   = handlers;
        QByteArray result            = QJsonDocument(docMap).toJson();
        chromeLocalState.open(QIODevice::WriteOnly | QIODevice::Truncate);
        chromeLocalState.write(result);
        chromeLocalState.close();
        qDebug("chrome fix applied");
      }
    }
  }
}

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  try {
    QStringList args = app.arguments();

    // No arguments probably means the user explictly ran this application
    // Set forcereg=True to allow them to register
    bool forceReg =
        (args.count() == 1) || ((args.count() > 1) && args.at(1) == "forcereg");

    std::unique_ptr<HandlerStorage> storage(loadStorage(forceReg));
    if (storage.get() == nullptr) {
      return 0;
    }

    // Log the arguments
    qDebug() << qUtf8Printable("\"" + args.join("\" \"") + "\"");

    // No other logs, close the log
    NxmHandler::LoggerDeinit();

    // Acceptable arguments
    //
    // nxmhandler.exe
    //    forces registration and spawns handler window
    //
    // nxmhandler.exe reg|forcereg schema game1,game2,game3 C:/path/to/binary
    // [arguments]
    //    reg:      register if noregister==false
    //    forcereg: force registration
    //
    // nxmhandler.exe nxm://link/to/mod
    //    forwards link to registered handler

    if (args.count() > 1) {
      if ((args.at(1) == "reg") || (args.at(1) == "forcereg")) {
        if (args.count() == 5 || args.count() == 6) {
          auto arguments = args.count() == 6 ? args.at(5) : "";
          storage->registerHandler(args.at(3).split(",", Qt::SkipEmptyParts),
                                   args.at(2), QDir::toNativeSeparators(args.at(4)),
                                   arguments, true, forceReg);
          if (forceReg) {
            applyChromeFix();
          }
          return 0;
        } else {
          QMessageBox::critical(nullptr, QObject::tr("Error"),
                                QObject::tr("Invalid number of parameters"));
        }
      } else if (args.at(1).startsWith("nxm://")) {
        NXMUrl url(args.at(1));
        QStringList handlerVals = storage->getHandler(url.game(), "nxm");
        QString executable      = handlerVals.front();
        handlerVals.pop_front();
        QString arguments = handlerVals.join(" ");
        if (!executable.isEmpty()) {
          handleNxmLink(executable, arguments, args.at(1));
          return 0;
        } else {
          QMessageBox::warning(
              nullptr, QObject::tr("No handler found"),
              QObject::tr("No application registered to handle this game (%1).\n"
                          "If you expected Mod Organizer to handle the link, "
                          "you have to go to Settings->Nexus and click the \"Associate "
                          "with ... links\"-button.\n"
                          "If you have NMM installed, you can re-register it for "
                          "nxm-links so it handles "
                          "the links that MO doesn't.")
                  .arg(url.game()));
          return 1;
        }
      } else if (args.at(1).startsWith("modl://")) {
        QUrl url(args.at(1));
        QUrlQuery params(url.query());
        QStringList handlerVals = storage->getHandler(url.host(), "modl");
        QString executable      = handlerVals.front();
        QString downloadUrl     = params.queryItemValue("url", QUrl::FullyDecoded);
        handlerVals.pop_front();
        auto argumentChunks = handlerVals.first().split(" ");
        auto name           = params.hasQueryItem("name")
                                  ? params.queryItemValue("name", QUrl::FullyDecoded)
                                  : "";
        auto modName        = params.hasQueryItem("modname")
                                  ? params.queryItemValue("modname", QUrl::FullyDecoded)
                                  : "";
        auto version        = params.hasQueryItem("version")
                                  ? params.queryItemValue("version", QUrl::FullyDecoded)
                                  : "";
        auto source         = params.hasQueryItem("source")
                                  ? params.queryItemValue("source", QUrl::FullyDecoded)
                                  : "";
        if (argumentChunks.contains("%name%")) {
          argumentChunks.replace(argumentChunks.indexOf("%name%"), "\"" + name + "\"");
        }
        if (argumentChunks.contains("%modname%")) {
          argumentChunks.replace(argumentChunks.indexOf("%modname%"),
                                 "\"" + modName + "\"");
        }
        if (argumentChunks.contains("%version%")) {
          argumentChunks.replace(argumentChunks.indexOf("%version%"),
                                 "\"" + version + "\"");
        }
        if (argumentChunks.contains("%source%")) {
          argumentChunks.replace(argumentChunks.indexOf("%source%"),
                                 "\"" + source + "\"");
        }
        QString arguments = argumentChunks.join(" ");
        if (!executable.isEmpty()) {
          handleModlLink(executable, arguments, downloadUrl);
          return 0;
        } else {
          QMessageBox::warning(
              nullptr, QObject::tr("No handler found"),
              QObject::tr("No application registered to handle this game (%1).\n"
                          "If you expected Mod Organizer to handle the link, "
                          "you have to go to Settings and click the \"Associate "
                          "with MODL links\"-button.\n")
                  .arg(url.host()));
          return 1;
        }
      } else {
        QMessageBox::warning(nullptr, QObject::tr("Invalid Arguments"),
                             QObject::tr("Invalid number of parameters"));
        return 1;
      }
      return 0;
    } else {
      HandlerWindow win;
      win.setHandlerStorage(storage.get());
      QSettings handlerReg("HKEY_CURRENT_USER\\Software\\Classes\\nxm\\",
                           QSettings::NativeFormat);
      QStringList handlerVals = HandlerStorage::stripCall(
          handlerReg.value("shell/open/command/Default").toString());
      QString handlerPath = handlerVals.front();
      handlerVals.pop_front();
      QString handerArgs = handlerVals.join(" ");
      win.setPrimaryHandler(handlerPath);
      win.show();

      return app.exec();
    }
  } catch (std::exception& e) {
    QMessageBox::critical(nullptr, QApplication::applicationName(),
                          QObject::tr("Uncaught exception:\n%1").arg(e.what()));
    throw;
  }
}
