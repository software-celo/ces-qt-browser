import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
    width: 0
    height: 0
    id: page
    color: "#000000"
    opacity: 0.4
    visible: false

    property int windowWidth: 0
    property int windowHeight: 0
    property bool locked: true

    signal lockedScreen();

    Component.onCompleted: {
        startTimer.running = true
    }

    Timer {
         id: startTimer
         interval: 100
         running: false
         repeat: false
         onTriggered: {
             _idleHelper.unlock()
             _backlight.unlock()
             locked = false
         }
     }

    Connections {
        target: _idleHelper
        function onLockTimeout() {
            if (_idleHelper.lockEnable){
                page.width = windowWidth
                page.height = windowHeight
                page.visible=true
                lockedLogo.opacity = 0.6
                page.opacity = 0.4
                _backlight.lock()
                locked = true
                page.lockedScreen()
//                console.debug("lockTimeout")
            }
            else {
//                console.debug("ignoring lockTimeout")
            }
        }
        function onBlankTimeout() {
            if (_idleHelper.blankEnable){
                page.width = windowWidth
                page.height = windowHeight
                lockedLogo.opacity = 0.6
                page.opacity = 0.4
                page.visible = true
                _backlight.blank()
                locked = true
                page.lockedScreen()
//                console.debug("blankTimeout")
            }
            else {
//                console.debug("ignoring blankTimeout")
            }
        }
        function onUnblank() {
//            console.debug("unblank")
            if (_idleHelper.lockEnable){
                if(locked){
                    _backlight.unblank()
                    page.lockedScreen()
                }else{
                    page.width = 0
                    page.height = 0
                    page.visible = false
                    _backlight.unlock()
                }
            }
            else {
                page.width = 0
                page.height = 0
                page.visible = false
                _backlight.unlock()
//                console.debug("unlock (unblank)")
            }
        }
        function onInputEvent() {
//            console.debug("fast_unblank")
            if(_idleHelper.lockEnable && locked){
                    _backlight.unblank()
                page.lockedScreen()
            }
            else {
                page.width = 0
                page.height = 0
                page.visible = false
                locked = false
                _backlight.unlock()
                _idleHelper.unlock()
            }
        }
    }

    ParallelAnimation {
        id: lockscreen
        NumberAnimation {
            target: page
            property: "opacity"
            duration: 60
            from: page.opacity
            to: 0.0
            easing.type: Easing.InOutQuint
        }

        NumberAnimation {
            target: lockedLogo
            property: "opacity"
            duration: 60
            from: lockedLogo.opacity
            to: 0.0
            easing.type: Easing.InOutQuint
        }

        NumberAnimation {
            target: page
            property: "height"
            duration: 60
            from: page.height
            to: 0
            easing.type: Easing.InOutQuint
        }
        running: false
    }

    Image {
        id: lockedLogo
        source: "file:" + _configBackend.lockImagePath
        sourceSize.height: parent.height < 800 ? 450 : (450/800) * parent.height
        sourceSize.width: parent.width < 1280 ? 450 : (450/1280) * parent.width
        fillMode: Image.PreserveAspectFit
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: (450/1280) * parent.width
        height: (450/800) * parent.height
        opacity: 0.6
    }

    MouseArea {
        id: mouseLockArea
        anchors.fill: page
        onClicked: {
            console.log("Click locked=" + locked);
            if (_idleHelper.lockEnable){
                if (!locked){
                    page.width = 0
                    page.height = 0
                    page.visible = true
                    _backlight.unblank()
                    page.lockedScreen()
                }
                else {
                    page.width = 0
                    page.height = 0
                    page.visible = false
                    locked=false
                    _backlight.unlock()
                    _idleHelper.unlock()
                }
            }
            else {
                page.width = 0
                page.height = 0
                page.visible = false
                _backlight.unlock()
                _idleHelper.unlock()
            }
        }
    }
}
