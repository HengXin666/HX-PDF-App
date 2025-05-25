import QtQuick
import QtQuick.Controls
import QmlPdfPageModel 1.0

Rectangle {
    id: root
    width: 640
    height: 480
    visible: true
    property string url
    
    function onUpdataImage(index: int) {
        const item = listView.itemAtIndex(index);
        console.log("刷新:", index, item);
        if (item) {
            item.source = `image://pdf/${index}?t=${Date.now()}`
        }
    }

    Rectangle {
        id: pdfTopBar
        anchors.top: parent.top
        Text {
            text: `总页数 ${QmlPdfPageModel._totalPages}`
        }
    }

    ListView {
        id: listView
        anchors.top: pdfTopBar.bottom
        anchors.topMargin: 10
        anchors.fill: parent
        model: QmlPdfPageModel._totalPages
        orientation: ListView.Vertical
        delegate: Image {
            required property int index
            width: QmlPdfPageModel.getPageWidth(index)
            height: QmlPdfPageModel.getPageHeight(index)
            source: `image://pdf/${index}?t=${Date.now()}`
            fillMode: Image.PreserveAspectFit
            smooth: true
            asynchronous: true
            onStatusChanged: {
                if (status === Image.Ready) {
                    console.log("第", index, "页加载完成");
                } else {
                    console.log("第", index, "页加载中...", width, height);
                }
            }
        }
    }

    Component.onCompleted: {
        // 绑定C++信号到QT槽
        QmlPdfPageModel.updataImage.connect(onUpdataImage);

        QmlPdfPageModel.setDocument(url);
        console.log("页数:", QmlPdfPageModel._totalPages);
    }
}