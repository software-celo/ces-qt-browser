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
    height: Screen.height
    width: Screen.width
    visibility: Window.FullScreen

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

    Timer {
        id: delayedReload
        interval: 2000
        running: false
        repeat: false
        onTriggered: {
            webEngineView.reload()
        }
    }

    Rectangle{
        id: webEngineViewRect
        anchors.centerIn: parent
        height: parent.height
        width:  parent.width
        rotation: _configBackend.rotationAngle

        /*  true = keyboard activated, false= keyboard deactivated
         *  Do not set this value manual here!
         */
        property bool keyboardEnable: _configBackend.keyboardEnable

        /*  true = dialogs used, false = dialogs blocked
         *  Do not set this value manual here!
         */
        property bool dialogsEnable: _configBackend.dialogsEnable

        /* true = insert <meta  name="viewport" content="initial-scale=1">
         * to default HEAD of html page after successful load of page
         */
        property bool initialScaleEnable: _configBackend.initialScaleEnable

        property bool maxScaleEnable: _configBackend.maxScaleEnable

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

            /* RenderProcess Termination Handling
             * Trigger reload after timer
             */
            onRenderProcessTerminated: {
                if (terminationStatus != WebEngineView.NormalTerminationStatus)
                    delayedReload.running = true
            }

            /* Error Page Handling
             *
             * In case of a load failure, the handler is called
             * which changes the url of the current view to the path
             * of the standard error page path.
             *
             * This error page will called instead.
             */
            onLoadingChanged: {
                webEngineView.LoadStartedStatus
                switch(loadRequest.status){
                    case WebEngineView.LoadStartedStatus:  {
                        break;
                    }
                    case WebEngineView.LoadStoppedStatus: {
                        disableTextSelection();
                        break;
                    }
                    case WebEngineView.LoadSucceededStatus: {
                        disableTextSelection();
                        if(true === webEngineViewRect.initialScaleEnable){
                            initialScaleHead(_configBackend.maxScaleEnable);
                        }
                        break;
                    }
                    default:{
                    }
                    case WebEngineView.LoadFailedStatus: {
                        /* For more information about the error code refer to this page
                         *   https://cs.chromium.org/chromium/src/net/base/net_error_list.h
                         */
                        webEngineView.url = webEngineView.erlPath + "?edomain=" + convertErrorDomainToString(loadRequest.errorDomain) + "&ecode=" + (-1)*loadRequest.errorCode ;
                        break;
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
        initKeyboard();
        initDialogs();
        rotateView();

    }

    /* Function converts the error domain code to an string representation
     * Return is a string
     */
    function convertErrorDomainToString(edomaincode){
        var errordesc;
        switch(edomaincode){
            default:{
            }
            case WebEngineView.NoErrorDomain:{
                errordesc ="UNKNOWN ERROR";
                break;
            }
            case WebEngineView.InternalErrorDomain:{
                errordesc ="QT INTERNAL ERROR";
                break;
            }
            case WebEngineView.ConnectionErrorDomain:{
                errordesc ="NETWORK CONNECTION ERROR";
                break;
            }
            case WebEngineView.CertificateErrorDomain:{
                errordesc ="SSL/TLS CERTIFICATE ERROR";
                break;
            }
            case WebEngineView.HttpErrorDomain:{
                errordesc ="HTTP CONNECTION ERROR";
                break;
            }
            case WebEngineView.FtpErrorDomain:{
                errordesc ="FTP CONNECTION ERROR";
                break;
            }
            case WebEngineView.DnsErrorDomain:{
                errordesc ="DNS CONNECTION ERROR";
                break;
            }
        }
        return errordesc;
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
     *
     *
     * Important notes:
     *
     *      Shift toggle on uppercase letters does not work or is not supported on following tested html standard input types:
     *          <input type="password" ><br>
     *          <input type="search" ><br>
     *          <input type="url" ><br>
     *
     *      The expected behaviour of this types is:
     *           NO_SHIFT --> CAPS_LOCK                                       (only caps lock and no shift)
     *
     *      A possible solution is included and commented out in the function below. The behaviour for the restricted types would be then:
     *           NO_SHIFT --> CAPS_LOCK --> (character input) --> NO_SHIFT    (CAPS_LOCK toggling )
     *
     *
     *      The expected behaviour for the other text based types is:
     *          NO_SHIFT --> SHIFT_KEY --> SHIFT_KEY --> CAPS_LOCK            (activate caps lock)
     *          NO_SHIFT --> SHIFT_KEY --> (character input) --> NO_SHIFT     (Toggle shift key)
     *
     *      A redefinition (javascript,typescript) of html input types could change the shift toggle behaviour of the qtvirtualkeyboard too.
     *
     *      For more information about the input types and the behaviour refer to this pages:
     *              http://doc.qt.io/qt-5/qt.html#InputMethodHint-enum
     *              http://doc.qt.io/qt-5/qml-qtquick-textinput.html#inputMethodHints-prop
     *              http://doc.qt.io/qt-5/qml-qtquick-virtualkeyboard-inputcontext.html#inputMethodHints-prop
     *
     *
     */
    function toggleShiftKeyHandler(key,text,modifiers){
        /* console.log("input method hints:" + InputContext.inputMethodHints.toString(16)); */
        if(key !== Qt.Key_Shift && key !== Qt.Key_unknown){
            if(InputContext.uppercase){
                if(!InputContext.capsLock){
                  InputContext.shift = false;
                }
            }
            /* Toggles CAPS_LOCK state to NO_SHIFT
                if((InputContext.inputMethodHints & Qt.ImhNoAutoUppercase) === Qt.ImhNoAutoUppercase && InputContext.capsLock === true){
                    InputContext.shift = false;
                    InputContext.capsLock = false;
                }
            */
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

        if( WebEngine.defaultProfile.offTheRecord){
            /* console.log("Cache in RAM: Activated") */
        }else{
            /* console.log("Cache in RAM: Deactivated"); */
        }

        WebEngine.defaultProfile.clearHttpCache();
        WebEngine.defaultProfile.httpCacheMaximumSize = 4000000;
        WebEngine.settings.errorPageEnabled = false;
        WebEngine.settings.showScrollBars = Qt.binding(function() { return _configBackend.scrollbarsEnable})
    }

    /* By default, this functions injects a smart piece of code to a html dom of a successful loaded html page,
     * and insert a tag <meta name="viewport" content="initial-scale=1"> to fix the browser resolution on startup
     * to the maximum display screen size on eglfs framebuffer
     * To prevent pinch-to-zoom-function, maximum-scale=1 is set in viewport on demand
     * BUG: Codesys 3.5.14 --> wrong scaling on startup
     */
    function initialScaleHead(maxscale){

        var scale;

        if(true === maxscale){
            scale = "node.setAttribute('content','initial-scale=1,maximum-scale=1');"
        }else{
            scale = "node.setAttribute('content','initial-scale=1');"
        }
        webEngineView.runJavaScript(
            "setTimeout( () => { " +
            "var head = document.head;" +
            "var node = document.createElement('meta');" +
            "node.setAttribute('name','viewport');" +
            scale +
            "head.appendChild(node); },250);"
            ,function(){})
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
     *
     * Supported angles:                    0,90,180,270
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
            request.dialogReject();
        }
    }

    /* Call of this function enables qtvirtualkeyboard, set property "keyboardEnable" to true
     * and connect a  keyhandler for customized key actions to keyboard
     */
    function initKeyboard(){
        /* Disable popup list for language key on keyboard */
        keyboardPanel.keyboard.style.languagePopupListEnabled = false;
        lockscreen.lockedScreen.connect(Qt.inputMethod.hide);
        Qt.inputMethod.visibleChanged.connect(qtkeyboardHandler);
        InputContext.inputEngine.activeKeyChanged.connect(pressedKeyHandler);
        InputContext.inputEngine.virtualKeyClicked.connect(toggleShiftKeyHandler);
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
        if(k === Qt.Key_Return){
            /* If shift is pressed, a newline symbol will be sent*/
            if(InputContext.shift){
                InputContext.sendKeyClick(Qt.Key_Insert, "\n", 0)
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
        if(!webEngineViewRect.keyboardEnable)
            return;

        if(Qt.inputMethod.visible){
            webEngineView.height = webEngineView.height - Qt.inputMethod.keyboardRectangle.height
            shiftInputToViewTimer.running = true
        }
        if(!Qt.inputMethod.visible){
            webEngineView.height = webEngineView.height + Qt.inputMethod.keyboardRectangle.height
        }
    }
}
