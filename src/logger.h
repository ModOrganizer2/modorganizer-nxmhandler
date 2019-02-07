#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

namespace NxmHandler {

void LoggerInit(const QString &fileName);
void LoggerDeinit();

}; //namespace NxmHandler

#endif
