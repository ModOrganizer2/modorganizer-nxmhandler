#include "handlerwindow.h"
#include "ui_handlerwindow.h"
#include "addbinarydialog.h"
#include <QMenu>
#include <QDir>


HandlerWindow::HandlerWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::HandlerWindow)
{
  ui->setupUi(this);

  connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(addBinaryDialog()));
  connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(removeBinary()));
}

HandlerWindow::~HandlerWindow()
{
  delete ui;
}


void HandlerWindow::setHandlerStorage(HandlerStorage *storage)
{
  storage->setParent(this);
  m_Storage = storage;

  ui->handlersWidget->clear();
  auto list = storage->handlers();
  for (auto iter = list.begin(); iter != list.end(); ++iter) {
    QTreeWidgetItem *newItem = new QTreeWidgetItem(QStringList() << iter->games.join(",") << QDir::toNativeSeparators(iter->executable));

    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
    ui->handlersWidget->addTopLevelItem(newItem);
  }
}

void HandlerWindow::closeEvent(QCloseEvent *event)
{
  m_Storage->clear();
  for (int i = 0; i < ui->handlersWidget->topLevelItemCount(); ++i) {
    QTreeWidgetItem *item = ui->handlersWidget->topLevelItem(i);
    m_Storage->registerHandler(item->text(0).split(","), item->text(1), false);
  }
  QMainWindow::closeEvent(event);
}

void HandlerWindow::addBinaryDialog()
{
  AddBinaryDialog dialog(m_Storage->knownGames());
  if (dialog.exec() == QDialog::Accepted) {
    QTreeWidgetItem *newItem = new QTreeWidgetItem(QStringList() << dialog.gameIDs().join(",") << dialog.executable());
    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
    ui->handlersWidget->insertTopLevelItem(0, newItem);
  }
}

void HandlerWindow::removeBinary()
{
  ui->handlersWidget->takeTopLevelItem(m_ContextIndex.row());
}

void HandlerWindow::on_handlersWidget_customContextMenuRequested(const QPoint &pos)
{
  QMenu contextMenu;

  m_ContextIndex = ui->handlersWidget->indexAt(pos);
  if (m_ContextIndex.isValid() != NULL) {
    contextMenu.addAction(ui->actionRemove);
  } else {
    contextMenu.addAction(ui->actionAdd);
  }

  contextMenu.move(ui->handlersWidget->mapToGlobal(pos));
  contextMenu.exec();
}
