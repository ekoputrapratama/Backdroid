import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtCore

// import Qt.labs.settings 1.0

Item {
  property Window window
  property string windowName: ""
  property int defaultX: 100
  property int defaultY: 100

  Settings {
    id: s
    category: windowName
    property int x
    property int y
    property int width
    property int height
    property int visibility
  }

  Component.onCompleted: {
    if (s.width && s.height) {
      window.x = s.x || defaultX;
      window.y = s.y || defaultY;
      window.width = s.width;
      window.height = s.height;
      window.visibility = s.visibility;
    }
  }

  Connections {
    target: window
    function onVisibilityChanged(visible) {
      saveSettingsTimer.restart();
    }
    function onHeightChanged(height) {
      saveSettingsTimer.restart();
    }
    function onWidthChanged(width) {
      saveSettingsTimer.restart();
    }
    function onYChanged(y) {
      saveSettingsTimer.restart();
    }
    function onXChanged(x) {
      saveSettingsTimer.restart();
    }
  }

  Timer {
    id: saveSettingsTimer
    interval: 100
    repeat: false
    onTriggered: saveSettings()
  }

  function saveSettings() {
    switch (window.visibility) {
    case ApplicationWindow.Windowed:
      s.x = window.x;
      s.y = window.y;
      s.width = window.width;
      s.height = window.height;
      s.visibility = window.visibility;
      break;
    case ApplicationWindow.FullScreen:
      s.visibility = window.visibility;
      break;
    case ApplicationWindow.Maximized:
      s.visibility = window.visibility;
      break;
    }
  }
}
