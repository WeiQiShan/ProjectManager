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

    enum Role {
        DisplayRole = Qt::DisplayRole,
        NameRole = Qt::UserRole + 1,
        PathRole = Qt::UserRole + 2,
        IsDirectoryRole = Qt::UserRole + 3,
        HasChildrenRole = Qt::UserRole + 4,
        DepthRole = Qt::UserRole + 5,
        RowRole = Qt::UserRole + 6,
        ColumnRole = Qt::UserRole + 7,
        FileCountRole = Qt::UserRole + 8
    };

    // QAbstractItemModel 接口
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool hasChildren(const QModelIndex &parent) const override;


    // 自定义功能
    Q_INVOKABLE void loadDirectory(const QString &path);
    Q_INVOKABLE void loadProject(const QString &filePath);
    Q_INVOKABLE void saveProject(const QString &filePath);
    Q_INVOKABLE void removeItem(const QModelIndex &index);

private:

    void scanDirectory(TreeItem *parent);
    void parseXmlElement(const QXmlStreamReader &xml, TreeItem *parent);
    TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *m_rootItem;
};

#endif // FILESYSTEMTREEMODEL_H
