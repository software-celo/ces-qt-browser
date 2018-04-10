import QtQuick 2.9
import QtQuick.Window 2.2
import QtWebEngine 1.5
//import QtQuick.Controls 2.2
import QtQuick.Controls 1.2
import "webset.js" as Webset

Window {
	id: window
	visible: true
    height: 480
    width: 800

    property bool key: false

    Connections {
        target: _idleHelper
        onDailyReset: {
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

	Action {
		shortcut: "Ctrl+Esc"
		onTriggered: {
			if (webEngineView)
				window.close()
		}
	}

    WebEngineView {
        id: webEngineView
        focus: true
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: window.height
    }

    Lockscreen {
        id: lockscreen
        anchors.bottom: parent.bottom
        x: 0
        windowHeight: parent.height
        windowWidth: parent.width
    }

	Component.onCompleted: {
		Webset.webEngineConfig(WebEngine,webEngineView);
	}

    function load(url) {
        webEngineView.url = url
    }

    function blockDialogs(bool_block) {
//		console.debug("Disable Block/Alert dialogs: " + bool_block);
        Webset.blockDialogs(bool_block)
    }
}
