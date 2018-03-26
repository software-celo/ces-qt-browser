import QtQuick 2.9 //2.1
import QtQuick.Window 2.2 //2.1
import QtWebEngine 1.5 // 1.3
import QtQuick.Controls 2.2 //1.0
import QtQuick.Controls 1.2 //1.0
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Styles 2.2
import QtQuick.VirtualKeyboard.Settings 2.2
import "webset.js" as Webset



Window {
	id: window
	visible: true
	height: 480		//480
	width: 800		//800
	function load(url) {
		webEngineView.url = url 
	}

	function blockDialogs(bool_block){
		console.log("Disable Block/Alert dialogs: " + bool_block);
		Webset.blockDialogs(bool_block)
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


	Timer {
		id: keypr
        interval: 100
		running: false
		repeat: false
		onTriggered:{
            /*Code is used, to shift hidden context input elements to view over keyboard*/
            InputContext.sendKeyClick(0x01000006," ",0);    /* Emulate Endkeycode with space */
            InputContext.sendKeyClick(0x01000003,"",0);     /* Deletes space */
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

	InputPanel {
		id: keyboardPanel
		y: Qt.inputMethod.visible ? parent.height - keyboardPanel.height : parent.height
		anchors.left: parent.left
		anchors.right: parent.right
	}

	Component.onCompleted: {
		Qt.inputMethod.visibleChanged.connect(showKeyboardStats);
		InputContext.inputEngine.activeKeyChanged.connect(pressedKey);
		Webset.webEngineConfig(WebEngine,webEngineView);
	}


	function pressedKey(k){
        /*Ask for Enterkey*/
        if(k === 16777220){
			//console.log("State Shift:"+ InputContext.shift)
			if(InputContext.shift === false){
				//console.log("Enter without shift")
				InputContext.sendKeyClick(16777217,"",0)
                InputContext.sendKeyClick(0x01000014,"",0)
			}
			else{
				//console.log("Enter with shift")
                InputContext.sendKeyClick(0x01000006,"\n",0)
			}
		}
	}

	function showKeyboardStats(){
		if(Qt.inputMethod.visible === true){
            /*  Keyboard visible */
			webEngineView.height = webEngineView.height-Qt.inputMethod.keyboardRectangle.height
            /*Triggers timer to emulate a keypress after keyboard is fully shown */
            keypr.running = true
		}else{
            /*  Keyboard invisble  */
			webEngineView.height = webEngineView.height+Qt.inputMethod.keyboardRectangle.height
		}
	}
}
