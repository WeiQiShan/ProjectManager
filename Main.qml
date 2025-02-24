import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
// import Qt.labs.folderlistview 1.0
import SkyProj 1.0

ApplicationWindow {
    id: rootWindow
    visible: true
    width: 800
    height: 600
    title: "SkyProj Manager - Qt 6.7"

    function getLocalPath(url) {
        let path = url.toString()
        if (Qt.platform.os === "windows") {
            return path.replace(/^file:\/{3}/, "")
        } else {
            return path.replace(/^file:\/{2}/, "")
        }
    }

    FileSystemTreeModel {
        id: fsModel
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Button {
                text: "选择文件夹"
                onClicked: folderDialog.open()
            }

            Button {
                text: "导出项目"
                onClicked: exportDialog.open()
            }
        }

        TreeView {
            id: treeView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: fsModel

            delegate: TreeViewDelegate {
                required property var model
                required property int row

                contentItem: Row {
                    spacing: 5
                    Image {
                        source: model.isDirectory ? "qrc:/folder.svg" : "qrc:/file.svg"
                        width: 20
                        height: 20
                    }
                    Text {
                        text: model.name
                        color: model.isDirectory ? "blue" : "black"
                    }
                }

                onDoubleClicked: {
                    if (model.isDirectory) {
                        if (treeView.isExpanded(row)) {
                            treeView.collapse(row)
                        } else {
                            treeView.expand(row)
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: folderDialog
        title: "选择包含OPGR文件的文件夹"
        fileMode: FileDialog.OpenFolder
        options: FileDialog.ShowDirsOnly
        currentFolder: StandardPath.standardLocations(StandardPath.Documents)[0]

        onAccepted: {
            const path = getLocalPath(currentFolder)
            console.log("Loading directory:", path)
            fsModel.loadDirectory(path)
        }
    }

    FileDialog {
        id: exportDialog
        title: "保存项目文件"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "skyproj"
        nameFilters: ["SkyProj Files (*.skyproj)"]
        currentFolder: folderDialog.currentFolder

        onAccepted: {
            const filePath = getLocalPath(selectedFile)
            console.log("Exporting to:", filePath)
            const success = fsModel.exportProject(filePath)
            showExportResult(success, filePath)
        }
    }

    Dialog {
        id: resultDialog
        title: "操作结果"
        anchors.centerIn: parent
        standardButtons: Dialog.Ok
        property string message: ""

        Label {
            text: resultDialog.message
            wrapMode: Text.Wrap
        }
    }

    function showExportResult(success, path) {
        resultDialog.message = success ? qsTr("导出成功！\n路径：%1").arg(path) : qsTr(
                                             "导出失败！请检查：\n1. 文件是否被占用\n2. 是否有写入权限")
        resultDialog.open()
    }
}
