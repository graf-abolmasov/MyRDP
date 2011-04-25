/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/



#include "screenshot.h"

//! [0]
Screenshot::Screenshot(QTcpClientConnection *con)
{
    screenshotLabel = new QLabel;
    screenshotLabel->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Expanding);
    screenshotLabel->setAlignment(Qt::AlignCenter);
    screenshotLabel->setMinimumSize(240, 160);

    createButtonsLayout();

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(screenshotLabel);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    intervalSpinBox->setValue(1);

    setWindowTitle(tr("Screenshot"));

    myConnection = con;
    myTimer = new QTimer();
    connect(myTimer, SIGNAL(timeout()),
            this, SLOT(capture()));
    connect(con, SIGNAL(headerRecieved(QMap<QString,QVariant>)),
            this, SLOT(headerRecieved(QMap<QString,QVariant>)));
    connect(con, SIGNAL(packetRecieved(QMap<QString,QVariant>,QByteArray)),
            this, SLOT(screenRecieved(QMap<QString,QVariant>,QByteArray)));
    connect(con, SIGNAL(recieveProgress(qint64,qint64)),
            this, SLOT(recieveProgress(qint64,qint64)));
    resize(820, 660);
}

Screenshot::~Screenshot()
{
    delete screenshotLabel;
    delete mainLayout;
    delete buttonsLayout;
    delete myTimer;
    delete label;
    delete intervalSpinBox;
    delete startButton;
    delete stopButton;
    delete captureButton;
    delete closeButton;

    QProgressBar *prograssBar;
}

void Screenshot::closeEvent(QCloseEvent *)
{
    myTimer->stop();
}

void Screenshot::resizeEvent(QResizeEvent * /* event */)
{
    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(screenshotLabel->size(), Qt::KeepAspectRatio);
    if (!screenshotLabel->pixmap()
            || scaledSize != screenshotLabel->pixmap()->size())
        updateScreenshotLabel();
}

void Screenshot::createButtonsLayout()
{
    startButton = createButton(tr("Start"), this, SLOT(startCapture()));
    stopButton = createButton(tr("Stop"), this, SLOT(stopCapture()));
    captureButton = createButton(tr("Capture"), this, SLOT(capture()));

    intervalSpinBox = new QSpinBox;
    intervalSpinBox->setSuffix(tr(" s"));
    intervalSpinBox->setMinimum(1);

    label = new QLabel(tr("Refresh interval:"));

    prograssBar = new QProgressBar;
    prograssBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(label);
    buttonsLayout->addWidget(intervalSpinBox);
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addWidget(stopButton);
    buttonsLayout->addWidget(captureButton);
    buttonsLayout->addWidget(prograssBar);
    //buttonsLayout->addStretch();
}

QPushButton *Screenshot::createButton(const QString &text, QWidget *receiver,
                                      const char *member)
{
    QPushButton *button = new QPushButton(text);
    button->connect(button, SIGNAL(clicked()), receiver, member);
    return button;
}

void Screenshot::updateScreenshotLabel()
{
    screenshotLabel->setPixmap(originalPixmap.scaled(screenshotLabel->size(),
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation));
}

void Screenshot::startCapture()
{
    if (intervalSpinBox->value() > 0) {
        capture();
        myTimer->start(intervalSpinBox->value() * 1000);
        stopButton->setEnabled(true);
        startButton->setEnabled(false);
        intervalSpinBox->setEnabled(false);
    }
}

void Screenshot::capture()
{
    QMap<QString, QVariant> header;
    header[tr("Command")] = tr("Capture screen");
    header[tr("Format")] = tr("png");
    myConnection->sendPacket(header, QByteArray());
}

void Screenshot::stopCapture()
{
    myTimer->stop();
    stopButton->setEnabled(false);
    startButton->setEnabled(true);
    intervalSpinBox->setEnabled(true);
}

void Screenshot::screenRecieved(const QMap<QString, QVariant> &header, const QByteArray &data)
{
    if (header.contains(tr("Screenshot"))) {
        qint64 w = header[tr("Width")].toInt();
        qint64 h = header[tr("Height")].toInt();
        QString format = header[tr("Format")].toString();
        originalPixmap.loadFromData(data, format.toAscii());
        updateScreenshotLabel();
    }
}

void Screenshot::recieveProgress(const qint64 &numBytes, const qint64 &bytesTotal)
{
    if (!ignorePacket) {
        prograssBar->setMinimum(0);
        prograssBar->setMaximum(bytesTotal);
        prograssBar->setValue(numBytes);
    }
}

void Screenshot::headerRecieved(const QMap<QString, QVariant> &header)
{
    ignorePacket = !header.contains(tr("Screenshot"));
}
