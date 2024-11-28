/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QTICOHANDLER_H
#define QTICOHANDLER_H

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <qendian.h>
#include <qvariant.h>

// These next two structs represent how the icon information is stored
// in an ICO file.
typedef struct
{
    quint8 bWidth;         // Width of the image
    quint8 bHeight;        // Height of the image (actual height, not times 2)
    quint8 bColorCount;    // Number of colors in image (0 if >=8bpp) [ not ture ]
    quint8 bReserved;      // Reserved
    quint16 wPlanes;       // Color Planes
    quint16 wBitCount;     // Bits per pixel
    quint32 dwBytesInRes;  // how many bytes in this resource?
    quint32 dwImageOffset; // where in the file is this image
} ICONDIRENTRY, *LPICONDIRENTRY;
#define ICONDIRENTRY_SIZE 16

typedef struct
{
    quint16 idReserved;        // Reserved
    quint16 idType;            // resource type (1 for icons, 2 for cursors)
    quint16 idCount;           // how many images?
    ICONDIRENTRY idEntries[1]; // the entries for each image
} ICONDIR, *LPICONDIR;
#define ICONDIR_SIZE 6 // Exclude the idEntries field

typedef struct
{                     // BMP information header
    quint32 biSize;   // size of this struct
    quint32 biWidth;  // pixmap width
    quint32 biHeight; // pixmap height     (specifies the combined height of the XOR and AND masks)
    quint16 biPlanes; // should be 1
    quint16 biBitCount;      // number of bits per pixel
    quint32 biCompression;   // compression method
    quint32 biSizeImage;     // size of image
    quint32 biXPelsPerMeter; // horizontal resolution
    quint32 biYPelsPerMeter; // vertical resolution
    quint32 biClrUsed;       // number of colors used
    quint32 biClrImportant;  // number of important colors
} BMP_INFOHDR, *LPBMP_INFOHDR;
#define BMP_INFOHDR_SIZE 40

class ICOWriter
{
public:
    ICOWriter(QIODevice *iodevice);
    int count();
    QImage iconAt(int index);
    static bool canRead(QIODevice *iodev);

    static QVector<QImage> read(QIODevice *device);

    static bool write(QIODevice *device, const QVector<QImage> &images);

    bool readIconEntry(int index, ICONDIRENTRY *iconEntry);

private:
    bool readHeader();

    bool readBMPHeader(quint32 imageOffset, BMP_INFOHDR *header);
    void findColorInfo(QImage &image);
    void readColorTable(QImage &image);

    void readBMP(QImage &image);
    void read1BitBMP(QImage &image);
    void read4BitBMP(QImage &image);
    void read8BitBMP(QImage &image);
    void read16_24_32BMP(QImage &image);

    struct IcoAttrib
    {
        int nbits;
        int ncolors;
        int h;
        int w;
        int depth;
    } icoAttrib;

    QIODevice *iod;
    qint64 startpos;
    bool headerRead;
    ICONDIR iconDir;
};

#endif /* QTICOHANDLER_H */

