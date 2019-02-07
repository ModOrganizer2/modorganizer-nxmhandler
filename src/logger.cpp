
#include "logger.h"

#include <QDateTime>
#include <QFile>

namespace NxmHandler {

static QFile g_File;

static void logHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
  if (!g_File.isOpen())
    return;

  g_File.write(qUtf8Printable(QString("[%1] %2\r\n").arg(QDateTime::currentDateTime().toString()).arg(message)));
}

void LoggerInit(const QString &fileName)
{
  if (g_File.isOpen())
    g_File.close();

  g_File.setFileName(fileName);

  // Prevent the log from growing infinitely
  if (g_File.size() > 10 * 1024 * 1024) {
    if (!g_File.open(QIODevice::WriteOnly)) {
      return;
    }
  } else if (!g_File.open(QIODevice::Append)) {
    return;
  }

  qInstallMessageHandler(logHandler);
}

void LoggerDeinit()
{
  if (g_File.isOpen())
    g_File.close();

  qInstallMessageHandler(NULL);
}

}; // namespace NxmHandler