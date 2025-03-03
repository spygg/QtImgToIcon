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

/*!
    \class QtIcoHandler
    \since 4.4
    \brief The QtIcoHandler class provides support for the ICO image format.
    \internal
*/

#include "qicohandler.h"

// Data readers and writers that takes care of alignment and endian stuff.
static bool readIconDirEntry(QIODevice *iodev, ICONDIRENTRY *iconDirEntry)
{
    if (iodev) {
        uchar tmp[ICONDIRENTRY_SIZE];
        if (iodev->read((char*)tmp, ICONDIRENTRY_SIZE) == ICONDIRENTRY_SIZE) {
            iconDirEntry->bWidth = tmp[0];
            iconDirEntry->bHeight = tmp[1];
            iconDirEntry->bColorCount = tmp[2];
            iconDirEntry->bReserved = tmp[3];

            iconDirEntry->wPlanes = qFromLittleEndian<quint16>(&tmp[4]);
            iconDirEntry->wBitCount = qFromLittleEndian<quint16>(&tmp[6]);
            iconDirEntry->dwBytesInRes = qFromLittleEndian<quint32>(&tmp[8]);
            iconDirEntry->dwImageOffset = qFromLittleEndian<quint32>(&tmp[12]);
            return true;
        }
    }
    return false;
}

static bool writeIconDirEntry(QIODevice *iodev, const ICONDIRENTRY &iconEntry)
{
    if (iodev) {
        uchar tmp[ICONDIRENTRY_SIZE];
        tmp[0] = iconEntry.bWidth;
        tmp[1] = iconEntry.bHeight;
        tmp[2] = iconEntry.bColorCount;
        tmp[3] = iconEntry.bReserved;
        qToLittleEndian<quint16>(iconEntry.wPlanes, &tmp[4]);
        qToLittleEndian<quint16>(iconEntry.wBitCount, &tmp[6]);
        qToLittleEndian<quint32>(iconEntry.dwBytesInRes, &tmp[8]);
        qToLittleEndian<quint32>(iconEntry.dwImageOffset, &tmp[12]);
        return iodev->write((char*)tmp, ICONDIRENTRY_SIZE) == ICONDIRENTRY_SIZE;
    }

    return false;
}

static bool readIconDir(QIODevice *iodev, ICONDIR *iconDir)
{
    if (iodev) {
        uchar tmp[ICONDIR_SIZE];
        if (iodev->read((char*)tmp, ICONDIR_SIZE) == ICONDIR_SIZE) {
            iconDir->idReserved = qFromLittleEndian<quint16>(&tmp[0]);
            iconDir->idType = qFromLittleEndian<quint16>(&tmp[2]);
            iconDir->idCount = qFromLittleEndian<quint16>(&tmp[4]);
            return true;
        }
    }
    return false;
}

static bool writeIconDir(QIODevice *iodev, const ICONDIR &iconDir)
{
    if (iodev) {
        uchar tmp[6];
        qToLittleEndian(iconDir.idReserved, tmp);
        qToLittleEndian(iconDir.idType, &tmp[2]);
        qToLittleEndian(iconDir.idCount, &tmp[4]);
        return iodev->write((char*)tmp, 6) == 6;
    }
    return false;
}

static bool readBMPInfoHeader(QIODevice *iodev, BMP_INFOHDR *pHeader)
{
    if (iodev) {
        uchar header[BMP_INFOHDR_SIZE];
        if (iodev->read((char*)header, BMP_INFOHDR_SIZE) == BMP_INFOHDR_SIZE) {
            pHeader->biSize = qFromLittleEndian<quint32>(&header[0]);
            pHeader->biWidth = qFromLittleEndian<quint32>(&header[4]);
            pHeader->biHeight = qFromLittleEndian<quint32>(&header[8]);
            pHeader->biPlanes = qFromLittleEndian<quint16>(&header[12]);
            pHeader->biBitCount = qFromLittleEndian<quint16>(&header[14]);
            pHeader->biCompression = qFromLittleEndian<quint32>(&header[16]);
            pHeader->biSizeImage = qFromLittleEndian<quint32>(&header[20]);
            pHeader->biXPelsPerMeter = qFromLittleEndian<quint32>(&header[24]);
            pHeader->biYPelsPerMeter = qFromLittleEndian<quint32>(&header[28]);
            pHeader->biClrUsed = qFromLittleEndian<quint32>(&header[32]);
            pHeader->biClrImportant = qFromLittleEndian<quint32>(&header[36]);
            return true;
        }
    }
    return false;
}

static bool writeBMPInfoHeader(QIODevice *iodev, const BMP_INFOHDR &header)
{
    if (iodev) {
        uchar tmp[BMP_INFOHDR_SIZE];
        qToLittleEndian<quint32>(header.biSize, &tmp[0]);
        qToLittleEndian<quint32>(header.biWidth, &tmp[4]);
        qToLittleEndian<quint32>(header.biHeight, &tmp[8]);
        qToLittleEndian<quint16>(header.biPlanes, &tmp[12]);
        qToLittleEndian<quint16>(header.biBitCount, &tmp[14]);
        qToLittleEndian<quint32>(header.biCompression, &tmp[16]);
        qToLittleEndian<quint32>(header.biSizeImage, &tmp[20]);
        qToLittleEndian<quint32>(header.biXPelsPerMeter, &tmp[24]);
        qToLittleEndian<quint32>(header.biYPelsPerMeter, &tmp[28]);
        qToLittleEndian<quint32>(header.biClrUsed, &tmp[32]);
        qToLittleEndian<quint32>(header.biClrImportant, &tmp[36]);

        return iodev->write((char*)tmp, BMP_INFOHDR_SIZE) == BMP_INFOHDR_SIZE;
    }
    return false;
}

ICOWriter::ICOWriter(QIODevice *iodevice)
    : iod(iodevice)
    , startpos(0)
    , headerRead(false)
{
}

int ICOWriter::count()
{
    if (readHeader())
        return iconDir.idCount;
    return 0;
}

bool ICOWriter::canRead(QIODevice *iodev)
{
    bool isProbablyICO = false;
    if (iodev) {
        qint64 oldPos = iodev->pos();

        ICONDIR ikonDir;
        if (readIconDir(iodev, &ikonDir)) {
            qint64 readBytes = ICONDIR_SIZE;
            if (readIconDirEntry(iodev, &ikonDir.idEntries[0])) {
                readBytes += ICONDIRENTRY_SIZE;
                // ICO format does not have a magic identifier, so we read 6 different values, which will hopefully be enough to identify the file.
                if (   ikonDir.idReserved == 0
                    && (ikonDir.idType == 1 || ikonDir.idType == 2)
                    && ikonDir.idEntries[0].bReserved == 0
                    && (ikonDir.idEntries[0].wPlanes <= 1 || ikonDir.idType == 2)
                    && (ikonDir.idEntries[0].wBitCount <= 32 || ikonDir.idType == 2)     // Bits per pixel
                    && ikonDir.idEntries[0].dwBytesInRes >= 40  // Must be over 40, since sizeof (infoheader) == 40
                    ) {
                    isProbablyICO = true;
                }

                if (iodev->isSequential()) {
                    // Our structs might be padded due to alignment, so we need to fetch each member before we ungetChar() !
                    quint32 tmp = ikonDir.idEntries[0].dwImageOffset;
                    iodev->ungetChar((tmp >> 24) & 0xff);
                    iodev->ungetChar((tmp >> 16) & 0xff);
                    iodev->ungetChar((tmp >>  8) & 0xff);
                    iodev->ungetChar(tmp & 0xff);

                    tmp = ikonDir.idEntries[0].dwBytesInRes;
                    iodev->ungetChar((tmp >> 24) & 0xff);
                    iodev->ungetChar((tmp >> 16) & 0xff);
                    iodev->ungetChar((tmp >>  8) & 0xff);
                    iodev->ungetChar(tmp & 0xff);

                    tmp = ikonDir.idEntries[0].wBitCount;
                    iodev->ungetChar((tmp >>  8) & 0xff);
                    iodev->ungetChar(tmp & 0xff);

                    tmp = ikonDir.idEntries[0].wPlanes;
                    iodev->ungetChar((tmp >>  8) & 0xff);
                    iodev->ungetChar(tmp & 0xff);

                    iodev->ungetChar(ikonDir.idEntries[0].bReserved);
                    iodev->ungetChar(ikonDir.idEntries[0].bColorCount);
                    iodev->ungetChar(ikonDir.idEntries[0].bHeight);
                    iodev->ungetChar(ikonDir.idEntries[0].bWidth);
                }
            }

            if (iodev->isSequential()) {
                // Our structs might be padded due to alignment, so we need to fetch each member before we ungetChar() !
                quint32 tmp = ikonDir.idCount;
                iodev->ungetChar((tmp >>  8) & 0xff);
                iodev->ungetChar(tmp & 0xff);

                tmp = ikonDir.idType;
                iodev->ungetChar((tmp >>  8) & 0xff);
                iodev->ungetChar(tmp & 0xff);

                tmp = ikonDir.idReserved;
                iodev->ungetChar((tmp >>  8) & 0xff);
                iodev->ungetChar(tmp & 0xff);
            }
        }
        if (!iodev->isSequential()) iodev->seek(oldPos);
    }

    return isProbablyICO;
}

bool ICOWriter::readHeader()
{
    if (iod && !headerRead) {
        startpos = iod->pos();
        if (readIconDir(iod, &iconDir)) {
            if (iconDir.idReserved == 0 && (iconDir.idType == 1 || iconDir.idType == 2))
            headerRead = true;
        }
    }

    return headerRead;
}

bool ICOWriter::readIconEntry(int index, ICONDIRENTRY *iconEntry)
{
    if (readHeader()) {
        if (iod->seek(startpos + ICONDIR_SIZE + (index * ICONDIRENTRY_SIZE))) {
            return readIconDirEntry(iod, iconEntry);
        }
    }
    return false;
}

bool ICOWriter::readBMPHeader(quint32 imageOffset, BMP_INFOHDR *header)
{
    if (iod) {
        if (iod->seek(startpos + imageOffset)) {
            if (readBMPInfoHeader(iod, header)) {
                return true;
            }
        }
    }
    return false;
}

void ICOWriter::findColorInfo(QImage &image)
{
    if (icoAttrib.ncolors > 0) {                // set color table
        readColorTable(image);
    } else if (icoAttrib.nbits == 16) { // don't support RGB values for 15/16 bpp
        image = QImage();
    }
}

void ICOWriter::readColorTable(QImage &image)
{
    if (iod) {
        image.setColorCount(icoAttrib.ncolors);
        uchar rgb[4];
        for (int i=0; i<icoAttrib.ncolors; i++) {
            if (iod->read((char*)rgb, 4) != 4) {
            image = QImage();
            break;
            }
            image.setColor(i, qRgb(rgb[2],rgb[1],rgb[0]));
        }
    } else {
        image = QImage();
    }
}

void ICOWriter::readBMP(QImage &image)
{
    if (icoAttrib.nbits == 1) {                // 1 bit BMP image
        read1BitBMP(image);
    } else if (icoAttrib.nbits == 4) {            // 4 bit BMP image
        read4BitBMP(image);
    } else if (icoAttrib.nbits == 8) {
        read8BitBMP(image);
    } else if (icoAttrib.nbits == 16 || icoAttrib.nbits == 24 || icoAttrib.nbits == 32 ) { // 16,24,32 bit BMP image
        read16_24_32BMP(image);
    }
}


/**
 * NOTE: A 1 bit BMP is only flipped vertically, and not horizontally like all other color depths!
 * (This is the same with the bitmask)
 *
 */
void ICOWriter::read1BitBMP(QImage &image)
{
    if (iod) {

        int h = image.height();
        int bpl = image.bytesPerLine();

        while (--h >= 0) {
            if (iod->read((char*)image.scanLine(h),bpl) != bpl) {
                image = QImage();
                break;
            }
        }
    } else {
        image = QImage();
    }
}

void ICOWriter::read4BitBMP(QImage &image)
{
    if (iod) {

        int h = icoAttrib.h;
        int buflen = ((icoAttrib.w+7)/8)*4;
        uchar *buf = new uchar[buflen];
        Q_CHECK_PTR(buf);

        while (--h >= 0) {
            if (iod->read((char*)buf,buflen) != buflen) {
                image = QImage();
                break;
            }
            uchar *p = image.scanLine(h);
            uchar *b = buf;
            for (int i=0; i<icoAttrib.w/2; i++) {   // convert nibbles to bytes
                *p++ = *b >> 4;
                *p++ = *b++ & 0x0f;
            }
            if (icoAttrib.w & 1)                    // the last nibble
                *p = *b >> 4;
        }

        delete [] buf;

    } else {
        image = QImage();
    }
}

void ICOWriter::read8BitBMP(QImage &image)
{
    if (iod) {

        int h = icoAttrib.h;
        int bpl = image.bytesPerLine();

        while (--h >= 0) {
            if (iod->read((char *)image.scanLine(h), bpl) != bpl) {
                image = QImage();
                break;
            }
        }
    } else {
        image = QImage();
    }
}

void ICOWriter::read16_24_32BMP(QImage &image)
{
    if (iod) {
        int h = icoAttrib.h;
        QRgb *p;
        QRgb  *end;
        uchar *buf = new uchar[image.bytesPerLine()];
        int    bpl = ((icoAttrib.w*icoAttrib.nbits+31)/32)*4;
        uchar *b;

        while (--h >= 0) {
            p = (QRgb *)image.scanLine(h);
            end = p + icoAttrib.w;
            if (iod->read((char *)buf, bpl) != bpl) {
                image = QImage();
                break;
            }
            b = buf;
            while (p < end) {
                if (icoAttrib.nbits == 24)
                    *p++ = qRgb(*(b+2), *(b+1), *b);
                else if (icoAttrib.nbits == 32)
                    *p++ = qRgba(*(b+2), *(b+1), *b, *(b+3));
                b += icoAttrib.nbits/8;
            }
        }

        delete[] buf;

    } else {
        image = QImage();
    }
}

static const char icoOrigDepthKey[] = "_q_icoOrigDepth";

QImage ICOWriter::iconAt(int index)
{
    QImage img;

    if (count() > index) { // forces header to be read

        ICONDIRENTRY iconEntry;
        if (readIconEntry(index, &iconEntry)) {

            static const uchar pngMagicData[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

            iod->seek(iconEntry.dwImageOffset);

            const QByteArray pngMagic = QByteArray::fromRawData((const char*)pngMagicData, sizeof(pngMagicData));
            const bool isPngImage = (iod->read(pngMagic.size()) == pngMagic);

            if (isPngImage) {
                iod->seek(iconEntry.dwImageOffset);
                QImage image = QImage::fromData(iod->read(iconEntry.dwBytesInRes), "png");
                image.setText(QLatin1String(icoOrigDepthKey), QString::number(iconEntry.wBitCount));
                return image;
            }

            BMP_INFOHDR header;
            if (readBMPHeader(iconEntry.dwImageOffset, &header)) {
                icoAttrib.nbits = header.biBitCount ? header.biBitCount : iconEntry.wBitCount;

                switch (icoAttrib.nbits) {
                case 32:
                case 24:
                case 16:
                    icoAttrib.depth = 32;
                    break;
                case 8:
                case 4:
                    icoAttrib.depth = 8;
                    break;
                default:
                    icoAttrib.depth = 1;
                }
                if (icoAttrib.depth == 32)                // there's no colormap
                    icoAttrib.ncolors = 0;
                else                    // # colors used
                    icoAttrib.ncolors = header.biClrUsed ? header.biClrUsed : 1 << icoAttrib.nbits;
                if (icoAttrib.ncolors > 256) //color table can't be more than 256
                    return img;
                icoAttrib.w = iconEntry.bWidth;
                if (icoAttrib.w == 0) // means 256 pixels
                    icoAttrib.w = header.biWidth;
                icoAttrib.h = iconEntry.bHeight;
                if (icoAttrib.h == 0) // means 256 pixels
                    icoAttrib.h = header.biHeight/2;

                QImage::Format format = QImage::Format_ARGB32;
                if (icoAttrib.nbits == 24)
                    format = QImage::Format_RGB32;
                else if (icoAttrib.ncolors == 2 && icoAttrib.depth == 1)
                    format = QImage::Format_Mono;
                else if (icoAttrib.ncolors > 0)
                    format = QImage::Format_Indexed8;

                QImage image(icoAttrib.w, icoAttrib.h, format);
                if (!image.isNull()) {
                    findColorInfo(image);
                    if (!image.isNull()) {
                        readBMP(image);
                        if (!image.isNull()) {
                            QImage mask(image.width(), image.height(), QImage::Format_Mono);
                            if (!mask.isNull()) {
                                mask.setColorCount(2);
                                mask.setColor(0, qRgba(255,255,255,0xff));
                                mask.setColor(1, qRgba(0  ,0  ,0  ,0xff));
                                read1BitBMP(mask);
                                if (!mask.isNull()) {
                                    img = image;
                                    img.setAlphaChannel(mask);
                                    // (Luckily, it seems that setAlphaChannel() does not ruin the alpha values
                                    // of partially transparent pixels in those icons that have that)
                                }
                            }
                        }
                    }
                }
                img.setText(QLatin1String(icoOrigDepthKey), QString::number(iconEntry.wBitCount));
            }
        }
    }

    return img;
}


/*!
    Reads all the icons from the given \a device, and returns them as
    a list of QImage objects.

    Each image has an alpha channel that represents the mask from the
    corresponding icon.

    \sa write()
*/
QVector<QImage> ICOWriter::read(QIODevice *device)
{
    QVector<QImage> images;

    ICOWriter reader(device);
    const int N = reader.count();
    images.reserve(N);
    for (int i = 0; i < N; i++)
        images += reader.iconAt(i);

    return images;
}


/*!
    Writes all the QImages in the \a images list to the given \a
    device. Returns \c true if the images are written successfully;
    otherwise returns \c false.

    The first image in the list is stored as the first icon in the
    device, and is therefore used as the default icon by applications.
    The alpha channel of each image is converted to a mask for each
    corresponding icon.

    \sa read()
*/
bool ICOWriter::write(QIODevice *device, const QVector<QImage> &images)
{
    bool retValue = false;

    if (images.count()) {

        qint64 origOffset = device->pos();

        ICONDIR id;
        id.idReserved = 0;
        id.idType = 1;
        id.idCount = images.count();

        ICONDIRENTRY * entries = new ICONDIRENTRY[id.idCount];
        BMP_INFOHDR * bmpHeaders = new BMP_INFOHDR[id.idCount];
        QByteArray * imageData = new QByteArray[id.idCount];

        for (int i=0; i<id.idCount; i++) {

            QImage image = images[i];
            // Scale down the image if it is larger than 256 pixels in either width or height
            // because this is a maximum size of image in the ICO file.
            if (image.width() > 256 || image.height() > 256)
            {
                image = image.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            QImage maskImage(image.width(), image.height(), QImage::Format_Mono);
            image = image.convertToFormat(QImage::Format_ARGB32);

            if (image.hasAlphaChannel()) {
                maskImage = image.createAlphaMask();
            } else {
                maskImage.fill(0xff);
            }
            maskImage = maskImage.convertToFormat(QImage::Format_Mono);

            int    nbits = 32;
            int    bpl_bmp = ((image.width()*nbits+31)/32)*4;

            entries[i].bColorCount = 0;
            entries[i].bReserved = 0;
            entries[i].wBitCount = nbits;
            entries[i].bHeight = image.height() < 256 ? image.height() : 0;  // 0 means 256
            entries[i].bWidth = image.width() < 256 ? image.width() : 0;     // 0 means 256
            entries[i].dwBytesInRes = BMP_INFOHDR_SIZE + (bpl_bmp * image.height())
                + (maskImage.bytesPerLine() * maskImage.height());
            entries[i].wPlanes = 1;
            if (i == 0)
                entries[i].dwImageOffset = origOffset + ICONDIR_SIZE
                + (id.idCount * ICONDIRENTRY_SIZE);
            else
                entries[i].dwImageOffset = entries[i-1].dwImageOffset + entries[i-1].dwBytesInRes;

            bmpHeaders[i].biBitCount = entries[i].wBitCount;
            bmpHeaders[i].biClrImportant = 0;
            bmpHeaders[i].biClrUsed = entries[i].bColorCount;
            bmpHeaders[i].biCompression = 0;
            bmpHeaders[i].biHeight = entries[i].bHeight ? entries[i].bHeight * 2 : 256 * 2; // 2 is for the mask
            bmpHeaders[i].biPlanes = entries[i].wPlanes;
            bmpHeaders[i].biSize = BMP_INFOHDR_SIZE;
            bmpHeaders[i].biSizeImage = entries[i].dwBytesInRes - BMP_INFOHDR_SIZE;
            bmpHeaders[i].biWidth = entries[i].bWidth ? entries[i].bWidth : 256;
            bmpHeaders[i].biXPelsPerMeter = 0;
            bmpHeaders[i].biYPelsPerMeter = 0;

            QBuffer buffer(&imageData[i]);
            buffer.open(QIODevice::WriteOnly);

            uchar *buf = new uchar[bpl_bmp];
            uchar *b;
            memset( buf, 0, bpl_bmp );
            int y;
            for (y = image.height() - 1; y >= 0; y--) {    // write the image bits
                // 32 bits
                QRgb *p   = (QRgb *)image.scanLine(y);
                QRgb *end = p + image.width();
                b = buf;
                int x = 0;
                while (p < end) {
                    *b++ = qBlue(*p);
                    *b++ = qGreen(*p);
                    *b++ = qRed(*p);
                    *b++ = qAlpha(*p);
                    if (qAlpha(*p) > 0)   // Even mostly transparent pixels must not be masked away
                        maskImage.setPixel(x, y, Qt::color1);  // (i.e. createAlphaMask() takes away too much)
                    p++;
                    x++;
                }
                buffer.write((char*)buf, bpl_bmp);
            }
            delete[] buf;

            maskImage.invertPixels();   // seems as though it needs this
            // NOTE! !! The mask is only flipped vertically - not horizontally !!
            for (y = maskImage.height() - 1; y >= 0; y--)
                buffer.write((char*)maskImage.scanLine(y), maskImage.bytesPerLine());
        }

        if (writeIconDir(device, id)) {
            int i;
            bool bOK = true;
            for (i = 0; i < id.idCount && bOK; i++) {
                bOK = writeIconDirEntry(device, entries[i]);
            }
            if (bOK) {
                for (i = 0; i < id.idCount && bOK; i++) {
                    bOK = writeBMPInfoHeader(device, bmpHeaders[i]);
                    bOK &= (device->write(imageData[i]) == (int) imageData[i].size());
                }
                retValue = bOK;
            }
        }

        delete [] entries;
        delete [] bmpHeaders;
        delete [] imageData;

    }
    return retValue;
}
