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

HandlerStorage* registerSchemaExecutable(HandlerStorage* storage,
                                         const QDir& storagePath,
                                         const QString& handlerPath,
                                         const QString& schema,
                                         const QString& handlerArgs)
{
  if (!handlerPath.isEmpty() &&
      !handlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive)) {
    // a foreign or global nxm handler, register ourself and use that handler as
    // an option - if this is another nxmhandler we could run into problems so skip it
    if (storage == nullptr)
      storage = new HandlerStorage(storagePath.path());
    storage->registerHandler(schema, handlerPath, handlerArgs, false);
    storage->registerSchemaProxy(QCoreApplication::applicationFilePath(), schema);
  } else {
    // no handler registered yet or the existing handler is invalid -> overwrite
    if (storage == nullptr)
      storage = new HandlerStorage(storagePath.path());
    storage->registerSchemaProxy(QCoreApplication::applicationFilePath(), schema);
  }
  return storage;
}

QSettings resolveSettings(QDir baseDir)
{
  if (!baseDir.exists("downloadhandler.ini") && baseDir.exists("nxmhandler.ini")) {
    QFile oldSettings(baseDir.absoluteFilePath("nxmhandler.ini"));
    oldSettings.copy(baseDir.absoluteFilePath("downloadhandler.ini"));
  }
  return QSettings(baseDir.absoluteFilePath("downloadhandler.ini"),
                   QSettings::IniFormat);
}

HandlerStorage* registerHandler(HandlerStorage* storage, const QString& schema,
                                bool forceReg)
{
  QDir globalStorage(
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  globalStorage.cd("../ModOrganizer");
  QDir baseDir;
  if (globalStorage.exists()) {
    baseDir = globalStorage;
  } else {
    baseDir = QDir(qApp->applicationDirPath());
  }
  QSettings handlerReg("HKEY_CURRENT_USER\\Software\\Classes\\" + schema + "\\",
                       QSettings::NativeFormat);
  QStringList handlerVals = HandlerStorage::stripCall(
      handlerReg.value("shell/open/command/Default").toString());
  QString handlerPath = handlerVals.front();
  handlerVals.pop_front();
  QString handlerArgs = handlerVals.join(" ");

  QDir handlerBaseDir = QFileInfo(handlerPath).absoluteDir();

  QSettings settings = resolveSettings(baseDir);
  bool noRegister    = settings.value("noregister", false).toBool();
  if (globalStorage.exists("downloadhandler.ini") &&
      handlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive) &&
      QFile::exists(handlerPath)) {
    // global configuration available - use it
    if (storage == nullptr)
      storage = new HandlerStorage(globalStorage.path());
  } else if (handlerBaseDir.exists("downloadhandler.ini") &&
             handlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive) &&
             QFile::exists(handlerPath)) {
    // a portable installation is registered to handle links, use its
    // configuration
    if (storage == nullptr)
      storage = new HandlerStorage(handlerBaseDir.path());
    if (forceReg && (QString::compare(QDir::toNativeSeparators(
                                          QCoreApplication::applicationFilePath()),
                                      handlerPath, Qt::CaseInsensitive))) {
      if (QMessageBox::question(
              nullptr, QObject::tr("Change Handler?"),
              QObject::tr("A %1 handler from a different Mod Organizer "
                          "installation has been registered. Do you want to "
                          "replace it? This is usually not necessary unless "
                          "the other installation is defective.")
                  .arg(schema),
              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        storage->registerSchemaProxy(QCoreApplication::applicationFilePath(), schema);
      }
    }
  } else if (!noRegister || forceReg) {
    // no handler registration
    QMessageBox registerBox(
        QMessageBox::Question, QObject::tr("Register?"),
        QObject::tr("Mod Organizer is not set up to handle %1 links. "
                    "Associate it with %1 links?")
            .arg(schema),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Save);
    registerBox.button(QMessageBox::Save)->setText(QObject::tr("No, don't ask again"));
    switch (registerBox.exec()) {
    case QMessageBox::Yes: {
      // base dir is either the global dir if it exists or the local application
      // dir
      storage =
          registerSchemaExecutable(storage, baseDir, handlerPath, schema, handlerArgs);
    } break;
    case QMessageBox::Save: {
      settings.setValue("noregister", true);
    } break;
    case QMessageBox::No: {
      settings.setValue("noregister", false);
    } break;
    }
  }
  return storage;
}

// ensure a nxmhandler.exe is registered to handle links, then load the
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
  QStringList schemas = {"nxm", "modl"};
  for (auto schema : schemas) {
    storage = registerHandler(storage, schema, forceReg);
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
        auto argumentChunks                 = handlerVals.first().split(" ");
        QMap<QString, QString> targetParams = {
            {"name", ""}, {"modname", ""}, {"version", ""}, {"source", ""}};
        for (auto param : targetParams.asKeyValueRange()) {
          auto value = params.hasQueryItem(param.first)
                           ? params.queryItemValue(param.first, QUrl::FullyDecoded)
                           : "";
          targetParams[param.first] = value;
        }
        for (auto param : targetParams.asKeyValueRange()) {
          if (argumentChunks.contains("%" + param.first + "%")) {
            argumentChunks.replace(argumentChunks.indexOf("%" + param.first + "%"),
                                   "\"" + param.second.replace("\"", "\\\"") + "\"");
          }
        }
        argumentChunks.append("-g " + url.host());
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
      QSettings nxmHandlerReg("HKEY_CURRENT_USER\\Software\\Classes\\nxm\\",
                              QSettings::NativeFormat);
      QStringList nxmHandlerVals = HandlerStorage::stripCall(
          nxmHandlerReg.value("shell/open/command/Default").toString());
      QString nxmHandlerPath = nxmHandlerVals.front();
      win.setNXMHandler(nxmHandlerPath);
      QSettings modlHandlerReg("HKEY_CURRENT_USER\\Software\\Classes\\modl\\",
                               QSettings::NativeFormat);
      QStringList modlHandlerVals = HandlerStorage::stripCall(
          modlHandlerReg.value("shell/open/command/Default").toString());
      QString modlHandlerPath = modlHandlerVals.front();
      win.setMODLHandler(modlHandlerPath);
      win.show();

      return app.exec();
    }
  } catch (std::exception& e) {
    QMessageBox::critical(nullptr, QApplication::applicationName(),
                          QObject::tr("Uncaught exception:\n%1").arg(e.what()));
    throw;
  }
}
