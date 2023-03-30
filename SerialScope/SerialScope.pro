QT += core gui serialport opengl concurrent network
QT += webenginewidgets

TARGET = vSailorProject
RC_ICONS = image\favicon.ico
#DEFINES += QT_NO_DEBUG_OUTPUT #不使用qdebug

CONFIG += c++11

#LIBS += -LD:/Qt/Qt5.14.2/5.14.2/msvc2017/lib -lfreeglut -lOpenGL32
#LIBS += -lOpengl32 -lglu32
LIBS += -lOpengl32 \
        -lglu32 \
#        -lglut

# 支持OpenGL
DEFINES += QCUSTOMPLOT_USE_OPENGL\
           QT_DEPRECATED_WARNINGS \
           VDEBUG_OPEN \
           HTML_PLAIN_TEXT
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.



# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    vAnsiBash/vansibash.cpp \
    vCsv/CSVHelper.cpp \
    vSeaskyPort/Protocol/bsp_protocol_class.cpp \
    vSeaskyPort/Protocol/crc/bsp_crc16.cpp \
    vSeaskyPort/Protocol/crc/bsp_crc8.cpp \
    vSerialPortCfg/vcodeconverter.cpp \
    vSerialPortCfg/vqserialrxthread.cpp \
    vSerialPortCfg/vqserialtxthread.cpp \
    vSerialPortCfg/vserialport.cpp \
    vCustomWidget/vcustomwidget.cpp \
    vOpenGL/model.cpp \
    vOpenGL/openglscene.cpp \
    vOpenGL/vopenglwidget.cpp \
    vPlainTextEdit/vplaintextedit.cpp \
    vQCustomplot/qcustomplot.cpp \
    vQCustomplot/vqcustomplot.cpp \
    vQCustomplot/vtracer.cpp \
    vQLineEditHex/vqlineedithex.cpp \
    vQTextBrowserHtml/vqtextbrowserhtml.cpp \
    vQTextEdit/vqtextedit.cpp \
    vQWebEngineView/vqwebengineview.cpp \
    vQtConfig/vCombox.cpp \
    vSeaskyPort/Protocol/bsp_protocol.cpp \
    vSeaskyPort/vseaskyport.cpp \
    vTcp/vserverthread.cpp \
    vTcp/vtcpctr.cpp \
    vcafesclient.cpp \
    vqtexteditline.cpp \
    vserialcom.cpp  \
    vDebug/vdebug.cpp \
    vTcp/vqtcpserver.cpp \
    vTcp/vqtcpsocket.cpp \

HEADERS += \
    mainwindow.h \
    vAnsiBash/vansibash.h \
    vCsv/CSVHelper.h \
    vSeaskyPort/Protocol/bsp_protocol_class.h \
    vSeaskyPort/Protocol/crc/bsp_crc16.h \
    vSeaskyPort/Protocol/crc/bsp_crc8.h \
    vSerialPortCfg/vcodeconverter.h \
    vSerialPortCfg/vqserialrxthread.h \
    vSerialPortCfg/vqserialtxthread.h \
    vSerialPortCfg/vserialport.h \
    vCustomWidget/vcustomwidget.h \
    vOpenGL/model.h \
    vOpenGL/openglscene.h \
    vOpenGL/point3d.h \
    vOpenGL/vopenglwidget.h \
    vPlainTextEdit/vplaintextedit.h \
    vQCustomplot/qcustomplot.h \
    vQCustomplot/vqcustomplot.h \
    vQCustomplot/vtracer.h \
    vQLineEditHex/vqlineedithex.h \
    vQTextBrowserHtml/vqtextbrowserhtml.h \
    vQTextEdit/vqtextedit.h \
    vQWebEngineView/vqwebengineview.h \
    vQtConfig/vCombox.h \
    vSeaskyPort/Protocol/bsp_protocol.h \
    vSeaskyPort/vseaskyport.h \
    vTcp/vserverthread.h \
    vTcp/vtcpctr.h \
    vcafesclient.h \
    vqtexteditline.h \
    vserialcom.h \
    vDebug/vdebug.h \
    vTcp/vqtcpserver.h \
    vTcp/vqtcpsocket.h \

INCLUDEPATH +=./vQtConfig \
              ./vPlainTextEdit \
              ./vQTextEdit  \
              ./vOpenGL     \
              ./vQCustomplot \
              ./OpenGLib    \
              ./vCustomWidget \
              ./vQWebEngineView\
              ./vQTextBrowserHtml\
              ./vSerialPortCfg  \
              ./vSeaskyPort \
              ./vTcp \
              ./vDebug\
              ./vAnsiBash\
              ./vCsv


FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    SerialScope_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# 支持QCustomPlot
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

RESOURCES += \
    img.qrc

DISTFILES += \
    readme.md



