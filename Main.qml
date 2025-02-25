import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform 1.1
import SkyProj 1.0

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "SkyProj Manager"

    FileSystemTreeModel {
        id: fsModel
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Button {
                text: "打开文件夹"
                onClicked: folderDialog.open()
            }
            Button {
                text: "打开项目"
                onClicked: projectOpenDialog.open()
            }
            Button {
                text: "保存项目"
                onClicked: projectSaveDialog.open()
            }
        }

        TreeView {
            id: treeView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            // The model needs to be a QAbstractItemModel
            model: fsModel
            selectionModel: ItemSelectionModel {
                model: treeView.model
            }

            delegate: Item {
                implicitWidth: padding + label.x + label.implicitWidth + padding
                implicitHeight: label.implicitHeight * 1.5

                readonly property real indentation: 20
                readonly property real padding: 5

                // Assigned to by TreeView:
                required property TreeView treeView
                required property bool isTreeNode
                required property bool expanded
                required property bool hasChildren
                required property int depth
                required property int row
                required property int column
                required property bool current

                Text {
                    text: model.name ? model.name : "No data" // 显示模型的名称
                }

                // Rotate indicator when expanded by the user
                // (requires TreeView to have a selectionModel)
                // 展开/折叠动画
                property Animation indicatorAnimation: NumberAnimation {
                    target: indicator
                    property: "rotation"
                    from: expanded ? 0 : 90
                    to: expanded ? 90 : 0
                    duration: 100
                    easing.type: Easing.OutQuart
                }
                TableView.onPooled: indicatorAnimation.complete()
                TableView.onReused: if (current)
                                        indicatorAnimation.start()
                onExpandedChanged: indicator.rotation = expanded ? 90 : 0
                // 背景高亮
                Rectangle {
                    id: background
                    anchors.fill: parent
                    color: row === treeView.currentRow ? palette.highlight : "black"
                    opacity: (treeView.alternatingRows
                              && row % 2 !== 0) ? 0.3 : 0.1
                }
                // 展开/折叠指示箭头
                Label {
                    id: indicator
                    x: padding + (depth * indentation)
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isTreeNode && hasChildren
                    text: "▶"

                    TapHandler {
                        onSingleTapped: {
                            let index = treeView.index(row, column)
                            treeView.selectionModel.setCurrentIndex(
                                        index, ItemSelectionModel.NoUpdate)
                            treeView.toggleExpanded(row)
                        }
                    }
                }

                Label {
                    id: label
                    x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - padding - x
                    clip: true
                    text: model.name
                }
            }
        }
    }

    FolderDialog {
        id: folderDialog
        onAccepted: fsModel.loadDirectory(getLocalPath(currentFolder))
    }

    FileDialog {
        id: projectOpenDialog
        nameFilters: ["SkyProj Files (*.skyproj)"]
        onAccepted: fsModel.loadProject(selectedFile)
    }

    FileDialog {
        id: projectSaveDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["SkyProj Files (*.skyproj)"]
        defaultSuffix: "skyproj"
        onAccepted: fsModel.saveProject(selectedFile)
    }

    function getLocalPath(url) {
        // console.log(url.toString().replace(/^file:\/{3}/, ""))
        return url.toString().replace(/^file:\/{3}/, "")
    }
}
