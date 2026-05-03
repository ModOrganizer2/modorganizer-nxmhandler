#include "addbinarydialog.h"
#include "ui_addbinarydialog.h"
#include <QFileDialog>

AddBinaryDialog::AddBinaryDialog(
    const std::vector<std::tuple<QString, QString, QString>>& games,
    const QStringList schemas, QWidget* parent)
    : QDialog(parent), ui(new Ui::AddBinaryDialog)
{
  ui->setupUi(this);

  for (auto iter = games.begin(); iter != games.end(); ++iter) {
    addGame(std::get<0>(*iter), std::get<1>(*iter));
  }

  for (auto schema : schemas) {
    this->ui->schemaSelector->addItem(schema);
  }
}

AddBinaryDialog::~AddBinaryDialog()
{
  delete ui;
}

void AddBinaryDialog::addGame(const QString& name, const QString& id)
{
  QListWidgetItem* item = new QListWidgetItem(name);
  item->setData(Qt::UserRole, id);

  ui->gamesList->addItem(item);
}

QStringList AddBinaryDialog::gameIDs()
{
  QStringList result;
  Q_FOREACH (QListWidgetItem* item, ui->gamesList->selectedItems()) {
    result.append(item->data(Qt::UserRole).toString());
  }
  return result;
}

QString AddBinaryDialog::executable()
{
  return QDir::toNativeSeparators(ui->binaryEdit->text());
}

QString AddBinaryDialog::arguments()
{
  return ui->argumentsEdit->text();
}

QString AddBinaryDialog::schema()
{
  return ui->schemaSelector->currentText();
}

void AddBinaryDialog::on_browseButton_clicked()
{
  ui->binaryEdit->setText(QFileDialog::getOpenFileName(
      this, tr("Select Executable"), QString(), tr("Executable (*.exe)")));
}
