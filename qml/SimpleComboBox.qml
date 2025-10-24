import QtQuick 2.12
import QtQuick.Controls 2.12


Item {
  id: control
  width: 350
  height: 40
  property var selected: cmb.currentValue
  property string label: "Test label"
  property var items: ["test1","test2"]
  property int labelWidth: 50
  property var onCurrentValueChanged: undefined
  Item {
    anchors.fill: parent
    Component.onCompleted: {
      if (label.length === 0) {
        cmbContainer.anchors.leftMargin = 0;
        labelWidth = 0
      } else {
        cmbContainer.anchors.leftMargin = 10;
      }
    }
    Label {
      id: cmbLabel
      width: control.labelWidth
      text: control.label
      anchors.bottom: parent.bottom
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.leftMargin: 0
      verticalAlignment: Label.AlignVCenter
    }
    Item {
      id: cmbContainer
      anchors.left: cmbLabel.right
      anchors.bottom: parent.bottom
      anchors.top: parent.top
      anchors.leftMargin: 10
      anchors.right: parent.right
      anchors.rightMargin: 0

      ComboBox {
        id: cmb
        rightPadding: 5
        leftPadding: 5
        anchors.fill: parent
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 0
        font.pixelSize: 12
        model: control.items
        onCurrentValueChanged: control.onCurrentValueChanged
      }
    }
  }
}
