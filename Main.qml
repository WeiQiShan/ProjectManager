import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform 1.1
import SkyProj 1.0
import QtQuick.Dialogs

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "SkyProj Manager"

    palette {
        text: "black"
        base: "white"
        highlight: "blue"
        highlightedText: "white"
    }

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

            model: fsModel
            selectionModel: ItemSelectionModel {
                model: treeView.model
            }

            delegate: Item {
                implicitWidth: padding + label.x + label.implicitWidth + padding
                implicitHeight: label.implicitHeight * 1.5

                readonly property real indentation: 20
                readonly property real padding: 5

                required property var model
                required property bool isTreeNode
                required property bool expanded
                required property bool hasChildren
                required property int depth
                required property bool current
                required property int row
                required property int column

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
                // onExpandedChanged:
                Rectangle {
                    id: background
                    anchors.fill: parent
                    // color: treeView.selectionModel.hasSelection
                    //        && model.path.startsWith(
                    //            treeView.selectionModel.currentIndex.data(
                    //                FileSystemTreeModel.PathRole)) ? "lightgray" : "transparent"
                    // opacity: 1.0
                    color: treeView.selectionModel.currentIndex.row
                           === row ? "lightgray" : "transparent"
                }

                Label {
                    id: indicator
                    x: padding + (depth * indentation)
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isTreeNode && hasChildren
                    text: "▶"
                    color: "black"
                    rotation: treeView.isExpanded(row) ? 90 : 0
                    TapHandler {
                        onSingleTapped: {
                            onSingleTapped: treeView.toggleExpanded(row)
                        }
                    }
                }

                Label {
                    id: label
                    x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - padding - x
                    clip: true
                    text: model.display
                    color: "black"
                }

                TapHandler {
                    onSingleTapped: {
                        let index = treeView.index(row, column)
                        treeView.selectionModel.setCurrentIndex(
                                    index, ItemSelectionModel.ClearAndSelect)
                        if (!model.isDirectory) {
                            console.log("选中文件路径: " + model.path)
                        }
                    }
                }
            }
        }
    }

    FolderDialog {
        id: folderDialog
        onAccepted: fsModel.loadDirectory(getLocalPath(selectedFolder))
    }

    FileDialog {
        id: projectOpenDialog
        nameFilters: ["SkyProj Files (*.skyproj)"]
        onAccepted: fsModel.loadProject(getLocalPath(selectedFile))
    }

    FileDialog {
        id: projectSaveDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["SkyProj Files (*.skyproj)"]
        defaultSuffix: "skyproj"
        onAccepted: fsModel.saveProject(getLocalPath(selectedFile))
    }

    function getLocalPath(url) {
        console.log(url.toString().replace(/^file:\/{3}/, ""))
        return url.toString().replace(/^file:\/{3}/, "")
    }
}
