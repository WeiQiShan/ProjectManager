#ifndef FILESYSTEMTREEMODEL_H
#define FILESYSTEMTREEMODEL_H

#include <QAbstractItemModel>
#include <QDir>
#include <QXmlStreamWriter>
#include "treeitem.h"

class FileSystemTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit FileSystemTreeModel(QObject *parent = nullptr);
    ~FileSystemTreeModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadDirectory(const QString &path);
    Q_INVOKABLE bool exportProject(const QString &savePath);

private:
    void scanDirectory(TreeItem *parent);
    TreeItem *getItem(const QModelIndex &index) const;
    TreeItem *m_rootItem;
};

#endif // FILESYSTEMTREEMODEL_H
