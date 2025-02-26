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

        TreeItem *parentItem;

        if (!parent.isValid())
            parentItem = m_rootItem;
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        TreeItem *childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        return QModelIndex();
    }

    QModelIndex FileSystemTreeModel::parent(const QModelIndex &index) const {
        if (!index.isValid())
            return QModelIndex();

        TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
        TreeItem *parentItem = childItem->parentItem();

        if (parentItem == m_rootItem)
            return QModelIndex();

        return createIndex(parentItem->row(), 0, parentItem);
    }

    int FileSystemTreeModel::rowCount(const QModelIndex &parent) const {

        TreeItem *parentItem;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid())
            parentItem = m_rootItem;
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        // qDebug() << "[DEBUG] rowCount 被调用 parent:" << parent << " 返回:" << parentItem->childCount();
        return parentItem->childCount();

    }

    int FileSystemTreeModel::columnCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant FileSystemTreeModel::data(const QModelIndex &index, int role) const {
        // qDebug() << "[DEBUG] data() 被调用，index:" << index << "role:" << role;
        if (!index.isValid())
            return QVariant();

        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (!item) return QVariant();

        switch (role) {
        case DisplayRole:
            return item->name();
        case IsDirectoryRole:
            return item->isDirectory();
        case HasChildrenRole:
            return item->hasChildren();
        case DepthRole:
            return item->depth();
        case NameRole:
            return item->name();  // 可以根据需要返回不同的数据
        case PathRole:
            return item->path();  // 可以根据需要返回不同的数据
        case RowRole:
            return index.row();
        case ColumnRole:
            return index.column();
        default:
            return QVariant();
        }
    }

    QHash<int, QByteArray> FileSystemTreeModel::roleNames() const {
        return {
            {DisplayRole, "display"},          // 必须对应QML的model.display
            {NameRole, "name"},
            {PathRole, "path"},
            {IsDirectoryRole, "isDirectory"},
            {HasChildrenRole, "hasChildren"},
            {DepthRole, "depth"},
            {RowRole, "row"},
            {ColumnRole, "column"}
        };
    }

    bool FileSystemTreeModel::hasChildren(const QModelIndex &parent) const
    {
        if (!parent.isValid()) return m_rootItem->childCount() > 0;

        TreeItem *item = static_cast<TreeItem*>(parent.internalPointer());
        return item && item->childCount() > 0;
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
        qDebug() << "Loading directory:" << path;
        // 创建新的文件夹节点
        TreeItem *newFolder = new TreeItem(QFileInfo(path).fileName(), path, true, m_rootItem);

        // 通知视图开始插入行
        int rowCount = m_rootItem->childCount();
        // qDebug() << "createIndex(rowCount, 0, m_rootItem): " << createIndex(rowCount, 0, m_rootItem);
        // qDebug() << "QModelIndex(): " << QModelIndex();
        beginInsertRows(QModelIndex(), rowCount, rowCount);
        m_rootItem->appendChild(newFolder);
        endInsertRows();
        qDebug() << "Row count after insert: " << m_rootItem->childCount();
        // 扫描文件夹内容
        scanDirectory(newFolder);
        // QModelIndex newFolderIndex = indexForItem(newFolder);
        // emit itemAdded(newFolderIndex);
    }

    void FileSystemTreeModel::scanDirectory(TreeItem *parent) {
        // qDebug() << "Scanning directory:" << parent->path();
        QDir dir(parent->path());
        const QFileInfoList entries = dir.entryInfoList(
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
            QDir::DirsFirst
            );

        for (const QFileInfo &entry : entries) {
            // qDebug() << "Entry found:" << entry.absoluteFilePath();
            const bool isDir = entry.isDir();
            const bool isTargetFile = entry.suffix().compare("ogpr", Qt::CaseInsensitive) == 0;

            if (isDir || isTargetFile) {
                // 通知视图插入新行
                const int newRow = parent->childCount();
                // qDebug() << "Inserting new row at:" << newRow;
                beginInsertRows(createIndex(parent->row(), 0, parent), newRow, newRow);

                TreeItem *child = new TreeItem(entry.fileName(),
                                               entry.absoluteFilePath(),
                                               isDir,
                                               parent);
                parent->appendChild(child);

                endInsertRows();
                // qDebug() << "Inserted child:" << child->name() << "at" << child->path();
                // 递归扫描子目录
                if (isDir) {
                    scanDirectory(child);
                }
            }
        }
    }

    void FileSystemTreeModel::saveProject(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "无法保存文件:" << file.errorString();
            return;
        }

        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("Project");

        // 遍历所有添加的文件夹
        for (int i = 0; i < m_rootItem->childCount(); ++i) {
            TreeItem *folder = m_rootItem->child(i);

            writer.writeStartElement("SwathGroup");
            writer.writeAttribute("name", folder->name());
            writer.writeAttribute("visible", "1");

            writer.writeStartElement("Folder");
            writer.writeCharacters(folder->path());
            writer.writeEndElement(); // Folder

            writer.writeEndElement(); // SwathGroup
        }

        writer.writeEndElement(); // Project
        writer.writeEndDocument();
    }


    void FileSystemTreeModel::loadProject(const QString &filePath) {
        beginResetModel();
        delete m_rootItem;
        m_rootItem = new TreeItem("Root", "", true);
        qDebug() << "filePath:" << filePath;
        QFile file(filePath);

        // if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //     qWarning() << "无法打开文件:" << filePath
        //                << ", 错误:" << file.errorString()
        //                << ", 文件是否存在:" << QFile::exists(filePath)
        //                << ", 权限:" << file.permissions();

        //     // 关键修复：必须结束重置
        //     endResetModel();
        //     return;
        // }

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "打开文件:";
            QXmlStreamReader xml(&file);
            while (!xml.atEnd()) {
                if (xml.isStartElement() && xml.name().toString() == "SwathGroup") {
                    qDebug() << "找到SwathGroup";
                    QString folderPath;
                    while (!(xml.isEndElement() && xml.name().toString() == "SwathGroup")) {
                        xml.readNext();
                        if (xml.isStartElement() && xml.name().toString() == "Folder") {
                            folderPath = xml.readElementText();
                        }
                    }
                    if (!folderPath.isEmpty()) {
                        loadDirectory(folderPath);
                    }
                }

                xml.readNext();
            }
        }

        endResetModel();
    }

