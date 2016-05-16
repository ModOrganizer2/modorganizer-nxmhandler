#ifndef HANDLERWINDOW_H
#define HANDLERWINDOW_H

#include <QMainWindow>
#include <QPersistentModelIndex>
#include "handlerstorage.h"

namespace Ui {
class HandlerWindow;
}

class HandlerWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit HandlerWindow(QWidget *parent = 0);
  ~HandlerWindow();

  void setPrimaryHandler(const QString &handlerPath);
  void setHandlerStorage(HandlerStorage *storage);
protected:
  virtual void closeEvent(QCloseEvent *event);
private slots:
  void on_handlersWidget_customContextMenuRequested(const QPoint &pos);
  void addBinaryDialog();
  void removeBinary();
  void on_registerButton_clicked();

private:
  Ui::HandlerWindow *ui;
  HandlerStorage *m_Storage;

};



#endif // HANDLERWINDOW_H
