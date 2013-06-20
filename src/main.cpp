#include "handlerwindow.h"
#include "handlerstorage.h"
#include <nxmurl.h>
#include <utility.h>
#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <Shlwapi.h>


#pragma comment(linker, "/manifestDependency:\"name='dlls' processorArchitecture='x86' version='1.0.0.0' type='win32' \"")


using MOBase::ToWString;


void handleLink(const QString &executable, const QString &link)
{
  ::ShellExecute(NULL, TEXT("open"), ToWString(executable).c_str(),
                 ToWString(link).c_str(),
                 ToWString(QFileInfo(executable).absolutePath()).c_str(),
                 SW_SHOWNORMAL);
}


// register self as nxm handler
void registerProxy(QSettings &settings, const QString &proxyPath)
{
  QString myExe = QString("\"%1\" ").arg(QDir::toNativeSeparators(proxyPath)).append("\"%1\"");
  settings.setValue("Default", "URL:NXM Protocol");
  settings.setValue("URL Protocol", "");
  settings.setValue("shell/open/command/Default", myExe);
  settings.sync();
}

// ensure a nxmhandler.exe is registered to handle nxm-links, then load the handler storage for that registered instance
// (even if it's different from the one actually being run)
HandlerStorage *loadStorage()
{
  HandlerStorage *storage = NULL;

  QSettings handlerReg("HKEY_CURRENT_USER\\Software\\Classes\\nxm\\", QSettings::NativeFormat);
  QString handlerPath = HandlerStorage::stripCall(handlerReg.value("shell/open/command/Default", QString()).toString());

  qDebug("%s", qPrintable(handlerPath));
  if (handlerPath.endsWith("nxmhandler.exe", Qt::CaseInsensitive) && QFile::exists(handlerPath)) {
    qDebug("a");
    // already a nxmhandler.exe registered, use its configuration
    storage = new HandlerStorage(QFileInfo(handlerPath).absolutePath());
  } else if (!handlerPath.isEmpty()) {
    qDebug("b");
    // a foreign nxm handler, register ourself and use that handler as an option
    storage = new HandlerStorage(QCoreApplication::applicationDirPath());
    storage->registerHandler(handlerPath, false);
    registerProxy(handlerReg, QCoreApplication::applicationFilePath());
  } else {
    qDebug("c");
    // no handler registered yet or the existing handler is invalid -> overwrite
    storage = new HandlerStorage(QCoreApplication::applicationDirPath());
    registerProxy(handlerReg, QCoreApplication::applicationFilePath());
  }
  return storage;
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  HandlerStorage *storage = loadStorage();
  Q_ASSERT(storage != NULL);

  QStringList args = app.arguments();
  if (args.count() > 1) {
    if (args.at(1) == "reg") {
      if (args.count() == 3) {
        storage->registerHandler(args.at(2).split(",", QString::SkipEmptyParts), args.at(3), true);
        return 0;
      }
    } else if (args.at(1).startsWith("nxm://")) {
      NXMUrl url(args.at(1));
      QString executable = storage->getHandler(url.game());
      if (!executable.isEmpty()) {
        handleLink(executable, args.at(1));
        return 0;
      } else {
        QMessageBox::warning(NULL, QObject::tr("No handler found"),
                             QObject::tr( "No application registered to handle this game.\n"
                                          "If you expected Mod Organizer to handle the link, "
                                          "you have to click the Browser button inside that Mod Organizer installation "
                                          "once to register it as a handler.\n"
                                          "If you have NMM installed, you can re-register it for nxm-links so it handles "
                                          "links MO doesn't."));
        return 1;
      }
    } else {
      QMessageBox::warning(NULL, QObject::tr("Invalid Arguments"), QObject::tr("Invalid number of parameters"));
      return 1;
    }
    return 0;
  } else {
    HandlerWindow win;
    win.setHandlerStorage(storage);
    win.show();

    return app.exec();
  }
}
