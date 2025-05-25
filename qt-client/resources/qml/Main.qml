import QtQuick
import "view"

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("HX-PDF-APP")

    Component {
        id: pdfViewComponent
        PdfListView {
            url: "http://127.0.0.1:28205/files/Cpp-T.pdf"
        }
    }

    Loader {
        id: pdfListView
        anchors.fill: parent
        sourceComponent: pdfViewComponent
    }
}