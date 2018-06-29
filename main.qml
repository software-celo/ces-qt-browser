/***************************************************************************************************************
** Author: Peter Fink
** Author: Steffen Kothe
** Description: QtWebEngine based Browser with integration of Qt Virtualkeyboard
** Company: Christ Electronic System GmbH
** Used License: LGPLv3
** Copyright (C) 2017-2018 Christ Electronic Systems GmbH
**
***************************************************************************************************************/

import QtQuick 2.9
import QtQuick.Window 2.2
import QtWebEngine 1.5
import QtQuick.Controls 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Styles 2.2

Window {
    id: window
    visible: true
    height: 480
    width: 800

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

    Rectangle{
        id: webEngineViewRect
        anchors.centerIn: parent
        height: parent.height
        width:  parent.width
        rotation: _configBackend.rotationAngle
        /*  true = keyboard activated, false= keyboard deactivated      | Default: false
         *  Do not set this value manual here!
         */
        property bool keyboardEnable: _configBackend.keyboardEnable

        /*  true = dialogs used, false = dialogs blocked                | Default: true
         *  Do not set this value manual here!
         */
        property bool dialogsEnable: _configBackend.dialogsEnable

        onRotationChanged: {
            rotateView();
        }

        WebEngineView {
            id: webEngineView
            focus: true
            implicitHeight: parent.height
            implicitWidth: parent.width
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            /* Path of error langing page */
            property string erlPath: "file:///usr/share/ces-qt-browser/erl/error.html"

            /* Context Menu Handling
             * Disable right click context menu
             */
            onContextMenuRequested: {
                request.accepted = true;
            }

            /* Error Page Handling
             *
             * In case of an load failure, the handler is called
             * which changes the url of the current view to the path
             * of the standard error page path.
             *
             * This error page will called instead.
             */
            onLoadingChanged: {
                switch(loadRequest.status){
                    case 0:{
                        /* console.log("Load started"); */
                        break;
                    }
                    case 1:{
                        /* console.log("Load stopped"); */
                         disableTextSelection();
                        break;
                    }
                    case 2:{
                        /* console.log("Load succeded"); */
                        disableTextSelection();
                        break;
                    }
                    case 3:{
                        console.log("Go to Errorpage:" + webEngineView.erlPath);
                        webEngineView.url = webEngineView.erlPath + "?edomain=" + loadRequest.errorDomain + "&ecode=" + loadRequest.errorCode ;
                        break;
                    }
                    default:{
                        //TODO: default -> error?
                    }
                }
            }
        }

        InputPanel {
            id: keyboardPanel
            y: (Qt.inputMethod.visible && webEngineViewRect.keyboardEnable) ? parent.height - keyboardPanel.height : parent.height
            anchors.left: parent.left
            anchors.right: parent.right
        }

        Lockscreen {
            id: lockscreen
            anchors.bottom: parent.bottom
            x: 0
            windowHeight: parent.height
            windowWidth: parent.width
        }
    }

    Component.onCompleted: {
        webBrowserConfig();
        /* Disable popup list for language key on keyboard */
        keyboardPanel.keyboard.style.languagePopupListEnabled = false;
        InputContext.inputEngine.virtualKeyClicked.connect(toggleShiftKeyHandler);
        initKeyboard();
        initDialogs();
        rotateView();
    }

    /* Function to reload/load an url to WebEngineView
     */
    function load(url) {
        webEngineView.url = url
    }

    /* Function shift and caps lock toggle
     *
     * After emitting an uppercase letter, function toggles from activated "Shiftkey"
     * to disabled "Shiftkey"
     */
    function toggleShiftKeyHandler(key,text,modifiers){
        if(key !== 16777248 && key !== 33554431 ){
            if(InputContext.uppercase === true){
                if(InputContext.capsLock === false){
                  InputContext.shift = false;
                }
            }
        }
    }

    /* Function for general settings of WebEngine
     *
     * - activates Cache in RAM
     * - disable cookies
     * - allows no persistent cache
     * - maximum size of cache is restricted to  ~400MB
     * - standard chromium error page is disabled
     * - cache is cleared
     */
    function webBrowserConfig(){
        WebEngine.defaultProfile.httpCacheType = 2;             /* No persistent cache */
        WebEngine.defaultProfile.persistentCookiesPolicy = 0;	/* No Persistent cookies */
        WebEngine.defaultProfile.offTheRecord = true;           /* Hold everything in RAM memory --> disable disk */

        if( WebEngine.defaultProfile.offTheRecord === true){
            /* console.log("Cache in RAM: Activated") */
        }else{
            /* console.log("Cache in RAM: Deactivated"); */
        }

        WebEngine.defaultProfile.clearHttpCache();
        WebEngine.defaultProfile.httpCacheMaximumSize = 4000000;
        WebEngine.settings.errorPageEnabled = false;
        WebEngine.settings.showScrollBars = false;
    }

    /* By default, this function injects a smart piece of code to a loaded html page (DOM tree),
     * and disable the text selection on this page/html document
     */
    function disableTextSelection(){
        webEngineView.runJavaScript(
            "var head = document.head;" +
            "var node= document.createElement('style');" +
            "var textnode = document.createTextNode('body {-webkit-user-select: none !important;}');" +
            "node.appendChild(textnode);head.appendChild(node);"
            ,function(){
             /*Empty function for callback*/
            }
        )
    }

    /* Function is used to set width/height of rectangle (WebEngineView)
     * to horizontal rotation parameters.
     * Function also set width of InputPanel (Keyboard width)
     *
     * Used in combination with rotation
     */
    function wevHorizontalAlign(){
        webEngineViewRect.height = window.height;
        webEngineViewRect.width = window.width;
        webEngineView.width = webEngineViewRect.width
        webEngineView.height = webEngineViewRect.height
        keyboardPanel.width = webEngineViewRect.width
    }

    /* Function is used to set width/height of rectangle (WebEngineView)
     * to vertical rotation parameters.
     * Function also sets width of InputPanel (Keyboard width)
     *
     * Used in combination with rotation
     */
    function wevVerticalAlign(){
        webEngineViewRect.height = window.width;
        webEngineViewRect.width = window.height;
        webEngineView.width = webEngineViewRect.width
        webEngineView.height = webEngineViewRect.height
        keyboardPanel.width = webEngineViewRect.height
    }

    /* Function is used for rotation of Rectangle "webEngineViewRect" and its content(Keyboard, WebEngineView)
     * On call, rotation is performed for a given angle (parameter), rotation and alignment of rectangle are set
     *
     * Before a rotation qtvirtualkeyboard is closed
     * Property is set to the current rotation
     * If rotation is already set, rotation angle is returned without any rotation action
     *
     * On call, function returns the current/setted rotation angle
     *
     * Supported angles:                    0,90,180,270,360
     * On default or not supported angles:  0
     */
    function rotateView(){
        var degree = webEngineViewRect.rotation

        /*Close keyboard before a rotation action*/
        if(keyboardPanel.active){
            Qt.inputMethod.hide();
        }

        switch(degree){
            case 0: {
                wevHorizontalAlign();
                break;
            }
            case 90: {
                wevVerticalAlign();
                break;
            }
            case 180: {
                 wevHorizontalAlign();
                 break;
            }
            case 270: {
                 wevVerticalAlign();
                 break;
            }
            default: {
                 /*Rotate back to default value*/
                 _configBackend.rotationAngle = 0;
                 wevHorizontalAlign();
              break;
            }
        }
    }

    /* On call, this function connects file, authentication, color and javascript dialogs
     * to a general handler which prevents opening of these dialogs.
     *
     * In general: function disables callable dialogs
     *
     * property  "dialogsEnable" is set to false if function was called
     */
    function initDialogs(){
        /*Does not work for ContextMenuRequest*/
        webEngineView.javaScriptDialogRequested.connect(dialogsHandler);
        webEngineView.fileDialogRequested.connect(dialogsHandler);
        webEngineView.authenticationDialogRequested.connect(dialogsHandler);
        webEngineView.colorDialogRequested.connect(dialogsHandler);
    }

    /* Handler for blocking/unblocking dialogs
     */
    function dialogsHandler(request){
        if(!webEngineViewRect.dialogsEnable){
            request.accepted = true;
            request.dialogReject();
        }
        else{
            console.log("Warning: Dialogs not handled yet!")
        }
    }

    /* Call of this function enables qtvirtualkeyboard, set property "keyboardEnable" to true
     * and connect a  keyhandler for customized key actions to keyboard
     */
    function initKeyboard(){
        Qt.inputMethod.visibleChanged.connect(qtkeyboardHandler);
        InputContext.inputEngine.activeKeyChanged.connect(pressedKeyHandler);
    }

    /* Handler for pressed key events
     *      No Shift + Enter = Tabulator
     *      Shift + Enter = Newline
     */
    function pressedKeyHandler(k){
        if(!webEngineViewRect.keyboardEnable){
            return;
        }

        /* Check if Enterkey is pressed */
        if(k === 16777220){
            /* If shift is false, emulate tabulator */
            if(InputContext.shift === false){
                InputContext.sendKeyClick(16777217, "", 0)
                InputContext.sendKeyClick(0x01000014, "", 0)
            }
            /* If shift is true, send newline character */
            else{
                InputContext.sendKeyClick(0x01000006, "\n", 0)
            }
        }
    }

    /* See "shiftInputToView" function and "qtvirtualkeyboard" handler function */
    Timer{
        id: shiftInputToViewTimer
        repeat: false
        interval: 100
        onTriggered: {
            shiftInputToView();
        }
    }

    /* Injection of javascript code, of qt virtualkeyboard is activated and has focus
     * on input element.
     *
     * In case, input element is out of the view of qt webengine, javascript code scrolls
     * input element to the bottom view of webengineview and top of qt virtualkeyboard
     */
    function shiftInputToView(){
        if(webEngineView.focus){
            webEngineView.runJavaScript(
               "var el= document.activeElement;"+
                "var bounding = el.getBoundingClientRect();"+
               "if ("+
                    "bounding.top >= 0 &&"+
                    "bounding.left >= 0 &&"+
                    "bounding.right <= (window.innerWidth || document.documentElement.clientWidth) &&"+
                    "bounding.bottom <= (window.innerHeight || document.documentElement.clientHeight)"+
                ") {console.log('In the viewport!');} else {"+
                   "console.log('Not in the viewport... whomp whomp');"+
                   "el.scrollIntoView(false);"+
                   "}"
           ,function(){
           /*Empty function for callback*/
           });
        }
    }

    /* Handler for the opening and closing of qt virtualkeyboard
     *
     * In case of visible:
     *              -   webEngineView height will reduced by qt virtualkeyboard
     *              -   Timer is triggerd, waits for keyboard opening/drawing and javascript code injection (shiftInputToView())
     *
     * In case of invisible:
     *              - webEngineView height is restored to fullscreen height (no keyboard visible)
     */
    function qtkeyboardHandler(){
        if(!webEngineViewRect.keyboardEnable){
            return;
        }

        if(Qt.inputMethod.visible){
            webEngineView.height = webEngineView.height - Qt.inputMethod.keyboardRectangle.height
            shiftInputToViewTimer.running = true
        }
        if(!Qt.inputMethod.visible){
            webEngineView.height = webEngineView.height + Qt.inputMethod.keyboardRectangle.height
        }
    }
}
