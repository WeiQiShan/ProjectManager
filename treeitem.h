#ifndef TREEITEM_H
#define TREEITEM_H

#include <QString>
#include <QList>

class TreeItem {
public:
    explicit TreeItem(const QString &name, const QString &path, bool isDir, TreeItem *parent = nullptr);
    ~TreeItem();

    void appendChild(TreeItem *child);
    TreeItem *child(int row);
    int childCount() const;
    bool hasChildren() const;
    QString name() const;
    QString path() const;
    bool isDirectory() const;
    int row() const;
    TreeItem *parentItem();
    int depth() const;
private:
    QList<TreeItem*> m_children;
    TreeItem *m_parent;
    QString m_name;
    QString m_path;
    bool m_isDir;
    int m_depth;
};

#endif // TREEITEM_H
