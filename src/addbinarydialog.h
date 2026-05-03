#ifndef ADDBINARYDIALOG_H
#define ADDBINARYDIALOG_H

#include "handlerstorage.h"
#include <QDialog>

namespace Ui
{
class AddBinaryDialog;
}

class AddBinaryDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AddBinaryDialog(
      const std::vector<std::tuple<QString, QString, QString>>& handlers,
      const QStringList schemas, QWidget* parent = 0);
  ~AddBinaryDialog();
  QStringList gameIDs();
  QString executable();
  QString arguments();
  QString schema();
private slots:
  void on_browseButton_clicked();

private:
  void addGame(const QString& name, const QString& id);

private:
  Ui::AddBinaryDialog* ui;
};

#endif  // ADDBINARYDIALOG_H
