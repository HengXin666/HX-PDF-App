import QtQuick
import QtQuick.Controls
import QmlPdfPageModel 1.0

FocusScope {
    id: root
    visible: true
    property string url
    
    function onUpdateImage(index: int) {
        const item = listView.itemAtIndex(index);
        if (item) {
            item.source = `image://pdf/${index}?t=${Date.now()}`;
        }
    }

    function onUpdateZoom() {
        console.log("刷新");
        for (let i = 0; i < listView.count; ++i) {
            const item = listView.itemAtIndex(i);
            if (item) {
                item.width = QmlPdfPageModel.getPageWidth(i);
                item.height = QmlPdfPageModel.getPageHeight(i);
                item.source = `image://pdf/${i}?f=${Date.now()}`;
            }
        }
    }

    Rectangle {
        id: pdfTopBar
        z: 2
        anchors.top: parent.top; width: parent.width; height: 40
        color: "#820cd7"
        Text {
            anchors.centerIn: parent
            text: "总页数: " + QmlPdfPageModel._totalPages
        }
    }

    ListView {
        id: listView
        z: 1
        anchors.top: pdfTopBar.bottom
        anchors.topMargin: 10
        anchors.fill: parent
        model: QmlPdfPageModel._totalPages
        clip: true
        
        orientation: ListView.Vertical
        delegate: Image {
            required property int index
            width: QmlPdfPageModel.getPageWidth(index)
            height: QmlPdfPageModel.getPageHeight(index)
            x: (ListView.view.width - width) / 2
            source: `image://pdf/${index}?t=${Date.now()}`
            fillMode: Image.PreserveAspectFit
            smooth: true
            asynchronous: true
            cache: false
            visible: status === Image.Ready
        }

        // 滚动速度, 默认是 1000
        flickDeceleration: 2400

        // 滚动条
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
            minimumSize: 0.1 // 滚动条最小占比
        }
    }

    // 按键监听
    Keys.onShortcutOverride: (event) => {
        console.log("按键!");
        if ((event.modifiers & Qt.ControlModifier) 
            && (event.key === Qt.Key_Plus || event.key === Qt.Key_Equal)
        ) {
            // Ctrl + '+'
            event.accepted = true;
            QmlPdfPageModel.setZoom(Math.min(3.0, QmlPdfPageModel.getZoom() + 0.1));
            console.log("Ctrl + '+' 被按下");
        } else if ((event.modifiers & Qt.ControlModifier) 
            && event.key === Qt.Key_Minus
        ) {
            // Ctrl + '-'
            event.accepted = true;
            QmlPdfPageModel.setZoom(Math.max(0.1, QmlPdfPageModel.getZoom() - 0.1));
            console.log("Ctrl + '-' 被按下");
        }
    }

    Component.onCompleted: {
        QmlPdfPageModel.setDocument(url);
        console.log("页数:", QmlPdfPageModel.getTotalPages(), QmlPdfPageModel._totalPages);

        // 绑定C++信号到QT槽
        QmlPdfPageModel.updateImage.connect(onUpdateImage);
        QmlPdfPageModel.updateZoom.connect(onUpdateZoom);

        // 强行获取焦点
        root.forceActiveFocus();
    }
}