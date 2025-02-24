#include "FileSystemTreeModel.h"
#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <QStack>

// ================= 模型实现 =================
FileSystemTreeModel::FileSystemTreeModel(QObject *parent)
    : QAbstractItemModel(parent),
    m_rootItem(new TreeItem("Root", "", true)) {}

FileSystemTreeModel::~FileSystemTreeModel() {
    delete m_rootItem;
}

QModelIndex FileSystemTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);
    TreeItem *childItem = parentItem->child(row);

    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex FileSystemTreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FileSystemTreeModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;
    return getItem(parent)->childCount();
}

int FileSystemTreeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant FileSystemTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    TreeItem *item = getItem(index);
    switch (role) {
    case Qt::DisplayRole:
        return item->name();
    case Qt::UserRole + 1: // PathRole
        return item->path();
    case Qt::UserRole + 2: // IsDirectoryRole
        return item->isDirectory();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FileSystemTreeModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    roles[Qt::UserRole + 1] = "path";
    roles[Qt::UserRole + 2] = "isDirectory";
    return roles;
}

TreeItem *FileSystemTreeModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_rootItem;
}

void FileSystemTreeModel::loadDirectory(const QString &path) {
    beginResetModel();
    delete m_rootItem;
    m_rootItem = new TreeItem(QFileInfo(path).fileName(), path, true);
    scanDirectory(m_rootItem);
    endResetModel();
}

void FileSystemTreeModel::scanDirectory(TreeItem *parent) {
    QDir dir(parent->path());
    const auto entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    for (const auto &entry : entries) {
        bool isDir = entry.isDir();
        if (isDir || entry.suffix().compare("ogpr", Qt::CaseInsensitive) == 0) {
            TreeItem *child = new TreeItem(
                entry.fileName(),
                entry.absoluteFilePath(),
                isDir,
                parent
                );
            parent->appendChild(child);
            if (isDir) {
                scanDirectory(child);
            }
        }
    }
}

bool FileSystemTreeModel::exportProject(const QString &savePath) {
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << file.errorString();
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(4);
    writer.writeStartDocument("1.0", true); // version 1.0, standalone yes

    writer.writeStartElement("Project");

    // 使用栈进行非递归遍历
    QStack<TreeItem*> stack;
    stack.push(m_rootItem);

    while (!stack.isEmpty()) {
        TreeItem* current = stack.pop();

        // 只处理目录节点（排除根节点和文件节点）
        if (current != m_rootItem && current->isDirectory()) {
            writer.writeStartElement("SwathGroup");
            writer.writeAttribute("name", current->name());
            writer.writeAttribute("visible", "1");

            writer.writeStartElement("Folder");
            writer.writeCharacters(current->path());
            writer.writeEndElement(); // Folder

            writer.writeEndElement(); // SwathGroup
        }

        // 逆序添加子节点以保持原顺序
        for (int i = current->childCount() - 1; i >= 0; --i) {
            stack.push(current->child(i));
        }
    }

    writer.writeEndElement(); // Project
    writer.writeEndDocument();

    file.close();
    return true;
}
