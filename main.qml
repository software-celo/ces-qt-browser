import QtQuick 2.1
import QtQuick.Window 2.1
import QtWebEngine 1.0
import QtQuick.Controls 1.0

Window {
    id: window
    visible: true
    height: 800
    width: 1280
    function load(url) { webEngineView.url = url }

    Connections {
        target: _idleTimer
        onReloadBrowser: {
            webEngineView.reload()
        }
    }

    Action {
        shortcut: "Ctrl+R"
        onTriggered: {
            if (webEngineView)
                webEngineView.reload()
        }
    }

    WebEngineView {
        id: webEngineView
        focus: true
        anchors.fill: parent
    }

    Lockscreen{
        id: lockscreen
        anchors.bottom: parent.bottom
        x: 0
    }
}

