#include "handlerwindow.h"
#include "addbinarydialog.h"
#include "ui_handlerwindow.h"
#include <QDir>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QShortcut>

enum
{
  COL_GAMES,
  COL_SCHEMA,
  COL_BINARY,
  COL_ARGUMENTS
};

HandlerWindow::HandlerWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::HandlerWindow)
{
  ui->setupUi(this);

  connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(addBinaryDialog()));
  connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(removeBinary()));

  new QShortcut(QKeySequence(Qt::Key_Delete), this, SLOT(removeBinary()));
}

HandlerWindow::~HandlerWindow()
{
  delete ui;
}

void HandlerWindow::setNXMHandler(const QString& handlerPath)
{
  if (handlerPath == QCoreApplication::applicationFilePath()) {
    ui->registerNXMButton->setEnabled(false);
    ui->nxmHandlerView->setText(tr("<Current>"));
  } else {
    ui->nxmHandlerView->setText(handlerPath);
  }
}

void HandlerWindow::setMODLHandler(const QString& handlerPath)
{
  if (handlerPath == QCoreApplication::applicationFilePath()) {
    ui->registerMODLButton->setEnabled(false);
    ui->modlHandlerView->setText(tr("<Current>"));
  } else {
    ui->modlHandlerView->setText(handlerPath);
  }
}

void HandlerWindow::setHandlerStorage(HandlerStorage* storage)
{
  m_Storage = storage;

  ui->handlersWidget->clear();
  auto list = storage->handlers();
  for (auto iter = list.begin(); iter != list.end(); ++iter) {
    QTreeWidgetItem* newItem = new QTreeWidgetItem(
        QStringList() << iter->games.join(",") << iter->schema
                      << QDir::toNativeSeparators(iter->executable) << iter->arguments);

    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
    ui->handlersWidget->addTopLevelItem(newItem);
  }
  ui->handlersWidget->resizeColumnToContents(COL_BINARY);
}

void HandlerWindow::closeEvent(QCloseEvent* event)
{
  m_Storage->clear();
  for (int i = 0; i < ui->handlersWidget->topLevelItemCount(); ++i) {
    QTreeWidgetItem* item = ui->handlersWidget->topLevelItem(i);
    m_Storage->registerHandler(item->text(0).split(","), item->text(1), item->text(2),
                               item->text(3), false, false);
  }
  QMainWindow::closeEvent(event);
}

void HandlerWindow::addBinaryDialog()
{
  AddBinaryDialog dialog(m_Storage->knownGames(), m_Storage->availableSchemas());
  if (dialog.exec() == QDialog::Accepted) {
    bool executableKnown = false;
    for (int i = 0; i < ui->handlersWidget->topLevelItemCount(); ++i) {
      QTreeWidgetItem* iterItem = ui->handlersWidget->topLevelItem(i);
      if (QFileInfo(iterItem->text(COL_BINARY)) == QFileInfo(dialog.executable())) {
        QStringList games = iterItem->text(COL_GAMES).split(",");
        games.append(dialog.gameIDs());
        games.removeDuplicates();
        iterItem->setText(COL_GAMES, games.join(","));
        iterItem->setText(COL_SCHEMA, dialog.schema());
        if (iterItem->text(COL_ARGUMENTS)
                .compare(dialog.arguments(), Qt::CaseInsensitive) != 0) {
          iterItem->setText(COL_ARGUMENTS, dialog.arguments());
        }
        executableKnown = true;
      }
    }

    if (!executableKnown) {
      QTreeWidgetItem* newItem = new QTreeWidgetItem(
          QStringList() << dialog.gameIDs().join(",") << dialog.schema()
                        << dialog.executable() << dialog.arguments());
      newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
      ui->handlersWidget->insertTopLevelItem(0, newItem);
    }
    ui->handlersWidget->resizeColumnToContents(COL_BINARY);
  }
}

void HandlerWindow::removeBinary()
{
  ui->handlersWidget->takeTopLevelItem(ui->handlersWidget->currentIndex().row());
  ui->handlersWidget->resizeColumnToContents(COL_BINARY);
}

void HandlerWindow::on_handlersWidget_customContextMenuRequested(const QPoint& pos)
{
  QMenu contextMenu;

  QModelIndex idx = ui->handlersWidget->indexAt(pos);
  if (idx.isValid()) {
    contextMenu.addAction(ui->actionRemove);
  } else {
    contextMenu.addAction(ui->actionAdd);
  }

  contextMenu.move(ui->handlersWidget->mapToGlobal(pos));
  contextMenu.exec();
}

void HandlerWindow::on_registerNXMButton_clicked()
{
  if (QMessageBox::question(
          this, tr("Change handler registration?"),
          tr("This will make the nxmhandler.exe you called the NXM handler "
             "registered in the system.\n"
             "That has no immediate impact on how links are handled.\nUse this "
             "if you moved Mod Organizer "
             "or if you uninstalled the Mod Organizer installation that was "
             "previously registered. Continue?"),
          QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
    ui->nxmHandlerView->setText(tr("<Current>"));
    ui->registerNXMButton->setEnabled(false);

    m_Storage->registerNxmProxy(QCoreApplication::applicationFilePath());
  }
}

void HandlerWindow::on_registerMODLButton_clicked()
{
  if (QMessageBox::question(
          this, tr("Change handler registration?"),
          tr("This will make the nxmhandler.exe you called the MODL handler "
             "registered in the system.\n"
             "That has no immediate impact on how links are handled.\nUse this "
             "if you moved Mod Organizer "
             "or if you uninstalled the Mod Organizer installation that was "
             "previously registered. Continue?"),
          QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
    ui->modlHandlerView->setText(tr("<Current>"));
    ui->registerMODLButton->setEnabled(false);

    m_Storage->registerModlProxy(QCoreApplication::applicationFilePath());
  }
}
