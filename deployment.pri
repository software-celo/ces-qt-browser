unix:!android {
    isEmpty(target.path) {
        qnx {
            target.path = /tmp/$${TARGET}/bin
        } else {
            target.path = /usr/sbin/
        }
        export(target.path)
    }
    INSTALLS += target
}

unix:!android {
    content.path = /usr/share/ces-qt-browser
    content.files = content/*.svg

    INSTALLS += content
}

export(INSTALLS)

