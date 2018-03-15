import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
    width: 0
    height: 0
    id: page
    color: "#000000"
    opacity: 0.4
    Component.onCompleted: {
        console.log("starting timer")
        timer.running = true
    }

    Timer {
         id: timer
         interval: 3000
         running: false
         repeat: false
         onTriggered: {
             console.log("calling unlock")
             _idleTimer.unlock()
         }
     }

    Connections {
        target: _idleTimer
        onLockTimeout: {
            page.width = 1280
            page.height = 800
            lockedLogo.opacity = 0.6
            page.opacity = 0.4
            _backlight.lock()
            console.log("lockTimeout")
        }
        onBlankTimeout: {
            _backlight.blank()
            console.log("blankTimeout")
        }
        onUnblank: {
            _backlight.unblank()
            console.log("unblank")
        }
    }

    ParallelAnimation{
        id: lockscreen
        NumberAnimation {
            target: page
            property: "opacity"
            duration: 600
            from: page.opacity
            to: 0.0
            easing.type: Easing.InOutQuint
        }

        NumberAnimation {
            target: lockedLogo
            property: "opacity"
            duration: 600
            from: lockedLogo.opacity
            to: 0.0
            easing.type: Easing.InOutQuint
        }

        NumberAnimation {
            target: page
            property: "height"
            duration: 600
            from: page.height
            to: 0
            easing.type: Easing.InOutQuint
        }
        running: false
    }

    Image{
        id: lockedLogo
        source: "web_locked.svg"
        sourceSize.height: 300
        sourceSize.width: 300
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: (300/1280) * parent.width
        height: (300/800) * parent.height
        opacity: 0.6
    }

    MouseArea{
        id: mouseLockArea
        anchors.fill: page
        onClicked: {
                    lockscreen.running = true
                    _backlight.unlock()
                    _idleTimer.unlock()
        }
    }
}
