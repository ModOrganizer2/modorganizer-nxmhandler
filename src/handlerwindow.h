#ifndef HANDLERWINDOW_H
#define HANDLERWINDOW_H

#include "handlerstorage.h"
#include <QMainWindow>
#include <QPersistentModelIndex>

namespace Ui
{
class HandlerWindow;
}

class HandlerWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit HandlerWindow(QWidget* parent = 0);
  ~HandlerWindow();

  void setNXMHandler(const QString& handlerPath);
  void setMODLHandler(const QString& handlerPath);
  void setHandlerStorage(HandlerStorage* storage);

protected:
  virtual void closeEvent(QCloseEvent* event);
private slots:
  void on_handlersWidget_customContextMenuRequested(const QPoint& pos);
  void addBinaryDialog();
  void removeBinary();
  void on_registerNXMButton_clicked();
  void on_registerMODLButton_clicked();

private:
  Ui::HandlerWindow* ui;
  HandlerStorage* m_Storage;
};

#endif  // HANDLERWINDOW_H
