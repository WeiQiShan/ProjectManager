#include "treeitem.h"

// ================= TreeItem 实现 =================
TreeItem::TreeItem(const QString &name, const QString &path, bool isDir, TreeItem *parent)
    : m_parent(parent), m_name(name), m_path(path), m_isDir(isDir) {

    m_depth = parent ? parent->m_depth + 1 : 0;
}

TreeItem::~TreeItem() {
    qDeleteAll(m_children);
}

void TreeItem::appendChild(TreeItem *child) {
    m_children.append(child);
}

TreeItem *TreeItem::child(int row) {
    if (row < 0 || row >= m_children.size())
        return nullptr;
    return m_children.at(row);
}

int TreeItem::childCount() const {
    return m_children.size();
}

bool TreeItem::hasChildren() const
{
    return !m_children.isEmpty();
}

QString TreeItem::name() const {
    return m_name;
}

QString TreeItem::path() const {
    return m_path;
}

bool TreeItem::isDirectory() const {
    return m_isDir;
}

int TreeItem::row() const {
    if (m_parent)
        return m_parent->m_children.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

TreeItem *TreeItem::parentItem() {
    return m_parent;
}

int TreeItem::depth() const
{
    return m_depth;
}

void TreeItem::removeChild(int position) {
    if (position < 0 || position >= m_children.size()) {
        return;
    }
    // 删除子项
    delete m_children.takeAt(position);
}

