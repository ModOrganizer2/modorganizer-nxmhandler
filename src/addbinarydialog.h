#ifndef ADDBINARYDIALOG_H
#define ADDBINARYDIALOG_H

#include <QDialog>
#include "handlerstorage.h"

namespace Ui {
class AddBinaryDialog;
}

class AddBinaryDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit AddBinaryDialog(const std::vector<std::pair<QString, QString> > &handlers, QWidget *parent = 0);
  ~AddBinaryDialog();
  QStringList gameIDs();
  QString executable();
  QString arguments();
private slots:
  void on_browseButton_clicked();
private:
  void addGame(const QString &name, const QString &id);
private:
  Ui::AddBinaryDialog *ui;
};

#endif // ADDBINARYDIALOG_H
