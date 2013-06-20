#ifndef SORTABLETREEWIDGET_H
#define SORTABLETREEWIDGET_H

#include <QTreeWidget>

class SortableTreeWidget : public QTreeWidget
{
public:
  SortableTreeWidget(QWidget *parent = NULL);
protected:
  virtual void dropEvent(QDropEvent *event);
  virtual bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);
  virtual Qt::DropActions supportedDropActions() const;
private:
  bool moveSelection(QTreeWidgetItem *parent, int index);

};

#endif // SORTABLETREEWIDGET_H
