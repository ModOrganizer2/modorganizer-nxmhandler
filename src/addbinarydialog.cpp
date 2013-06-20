#include "addbinarydialog.h"
#include "ui_addbinarydialog.h"
#include <QFileDialog>


AddBinaryDialog::AddBinaryDialog(const std::vector<std::pair<QString, QString> > &games, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::AddBinaryDialog)
{
  ui->setupUi(this);

  for (auto iter = games.begin(); iter != games.end(); ++iter) {
    addGame(iter->first, iter->second);
  }
}

AddBinaryDialog::~AddBinaryDialog()
{
  delete ui;
}

void AddBinaryDialog::addGame(const QString &name, const QString &id)
{
  QListWidgetItem *item = new QListWidgetItem(name);
  item->setData(Qt::UserRole, id);

  ui->gamesList->addItem(item);
}

QStringList AddBinaryDialog::gameIDs()
{
  QStringList result;
  Q_FOREACH(QListWidgetItem *item, ui->gamesList->selectedItems()) {
    result.append(item->data(Qt::UserRole).toString());
  }
  return result;
}

QString AddBinaryDialog::executable()
{
  return QDir::toNativeSeparators(ui->binaryEdit->text());
}

void AddBinaryDialog::on_browseButton_clicked()
{
  ui->binaryEdit->setText(QFileDialog::getOpenFileName(this, tr("Select Executable"), QString(),
                                                       tr("Executable (*.exe)")));
}
