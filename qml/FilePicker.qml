import QtQuick 2.12
import QtQuick.Controls 2.12
// import Qt.labs.platform 1.1
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.3

import "/common/vars.mjs" as Vars

Item {
  id: control
  width: 350
  height: 40
  property string selected: filepath.text
  property string label: ""
  property bool selectFolder: false
  property int labelWidth: 50
  Material.elevation: 4
  FileDialog {
    id: selector
    selectFolder: control.selectFolder
    onAccepted: {
      if(selectFolder) {
        filepath.text = selector.folder.toString().replace("file://", "");
      } else {
        filepath.text = selector.fileUrl.toString().replace("file://", "");
      }
    }
  }
  Item {
    anchors.fill: parent
    Component.onCompleted: {
      if (label.length === 0) {
        textBoxContainer.anchors.leftMargin = 0;
        labelWidth = 0
      } else {
        textBoxContainer.anchors.leftMargin = 10;
      }
    }
    Label {
      id: filepickerLabel
      width: control.labelWidth
      text: control.label
      anchors.bottom: parent.bottom
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.leftMargin: 0
      verticalAlignment: Label.AlignVCenter
    }
    Item {
      id: textBoxContainer
      anchors.left: filepickerLabel.right
      anchors.bottom: parent.bottom
      anchors.top: parent.top
      anchors.leftMargin: 10
      anchors.right: selectFileBtn.left
      anchors.rightMargin: 0

      // anchors.top: parent.top
      // anchors.topMargin: 0
      // anchors.bottom: parent.bottom
      // anchors.bottomMargin: 0
      Rectangle {
        color: "#48484880"
        anchors.fill: parent
        // border.width: 2
        // border.color: "#eeeeee"
      }

      TextInput {
        id: filepath
        text: qsTr("")
        enabled: false
        rightPadding: 5
        leftPadding: 5
        verticalAlignment: Text.AlignVCenter
        anchors.fill: parent
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5
        font.pixelSize: 12
        clip: true
      }
    }

    Button {
      id: selectFileBtn
      anchors.top: parent.top
      anchors.topMargin: 0
      anchors.bottom: parent.bottom
      anchors.bottomMargin: 0
      anchors.right: parent.right
      anchors.rightMargin: 0
      topPadding: 0
      flat: true
      highlighted: true
      width: 80
      contentItem: Label {
        text: qsTr("Browse")
        color: "#ffffff"
        anchors.fill: parent
        verticalAlignment: Label.AlignVCenter
        horizontalAlignment: Label.AlignHCenter
      }
      background: Rectangle {
        anchors.fill: parent
        // opacity: enabled ? 1 : 0.3
        color: selectFileBtn.down ? "#C9CACA" : "#484848"
      }
      onClicked: {
        selector.open()
      }
    }
  }
}
