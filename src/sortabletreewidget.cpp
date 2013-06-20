#include "sortabletreewidget.h"
#include <QDropEvent>

SortableTreeWidget::SortableTreeWidget(QWidget *parent)
  : QTreeWidget(parent)
{
}

void SortableTreeWidget::dropEvent(QDropEvent *event)
{
  // only react on internal drag&drop
  if (event->source() == this) {
    QTreeWidget::dropEvent(event);
  }
}

Qt::DropActions SortableTreeWidget::supportedDropActions() const
{
  // we do our own drop handling, this ensures dropMimeData is called.
  return Qt::CopyAction;
}


bool SortableTreeWidget::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData*, Qt::DropAction)
{
  // TODO: ignores type of action set up in designer, always moves
  return this->moveSelection(parent, index);
}

bool SortableTreeWidget::moveSelection(QTreeWidgetItem *parent, int index)
{
  QModelIndex parentIndex = indexFromItem(parent);

  std::vector<QPersistentModelIndex> persistentIndices;
  Q_FOREACH(const QModelIndex &index, selectedIndexes()) {
    if (index == parentIndex) return false;
    if (index.column() != 0) continue;
    persistentIndices.push_back(index);
  }

  int targetRow = -1;
  if (itemsExpandable() || !parentIndex.isValid()) {
    targetRow = model()->index(index, 0, parentIndex).row();

    if (targetRow < 0) {
      targetRow = index;
    }
  } else {
    // if items aren't expandable, we can't be placing items on sublevels
    targetRow = parentIndex.row();
    parentIndex = QModelIndex();
  }

  // remove items from the list, store in temporary location
  QList<QTreeWidgetItem*> temp;
  for (auto iter = persistentIndices.rbegin(); iter != persistentIndices.rend(); ++iter) {
    QTreeWidgetItem *item = itemFromIndex(*iter);
    if (item == NULL) continue;
    if ((item == NULL) || (item->parent() == NULL)) {
      temp.append(takeTopLevelItem(iter->row()));
    } else {
      temp.append(item->parent()->takeChild(iter->row()));
    }
  }

  if (index == -1) {
    // append
    if (parentIndex.isValid()) {
      parent->addChildren(temp);
    } else {
      addTopLevelItems(temp);
    }
  } else {
    if (parentIndex.isValid()) {
      parent->insertChildren(std::min<int>(targetRow, parent->childCount()), temp);
    } else {
      insertTopLevelItems(std::min<int>(targetRow, topLevelItemCount()), temp);
    }
  }
  return true;
}
