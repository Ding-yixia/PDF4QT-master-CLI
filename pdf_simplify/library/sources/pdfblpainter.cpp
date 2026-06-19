// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pdfblpainter.h"

#include <QPaintEngine>

namespace pdf
{

PDFBLPaintDevice::PDFBLPaintDevice(QImage& offscreenBuffer, bool /*isMultithreaded*/) :
    m_offscreenBuffer(offscreenBuffer)
{
}

PDFBLPaintDevice::~PDFBLPaintDevice()
{
}

int PDFBLPaintDevice::devType() const
{
    return QInternal::Image;
}

QPaintEngine* PDFBLPaintDevice::paintEngine() const
{
    return m_offscreenBuffer.paintEngine();
}

uint32_t PDFBLPaintDevice::getVersion()
{
    return 0;
}

int PDFBLPaintDevice::metric(PaintDeviceMetric metric) const
{
    switch (metric)
    {
    case QPaintDevice::PdmWidth:
        return m_offscreenBuffer.width();
    case QPaintDevice::PdmHeight:
        return m_offscreenBuffer.height();
    case QPaintDevice::PdmWidthMM:
        return m_offscreenBuffer.widthMM();
    case QPaintDevice::PdmHeightMM:
        return m_offscreenBuffer.heightMM();
    case QPaintDevice::PdmNumColors:
        return m_offscreenBuffer.colorCount();
    case QPaintDevice::PdmDepth:
        return m_offscreenBuffer.depth();
    case QPaintDevice::PdmDpiX:
        return m_offscreenBuffer.logicalDpiX();
    case QPaintDevice::PdmDpiY:
        return m_offscreenBuffer.logicalDpiY();
    case QPaintDevice::PdmPhysicalDpiX:
        return m_offscreenBuffer.physicalDpiX();
    case QPaintDevice::PdmPhysicalDpiY:
        return m_offscreenBuffer.physicalDpiY();
    case QPaintDevice::PdmDevicePixelRatio:
        return m_offscreenBuffer.devicePixelRatio();
    case QPaintDevice::PdmDevicePixelRatioScaled:
        return m_offscreenBuffer.devicePixelRatioFScale();
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    case QPaintDevice::PdmDevicePixelRatioF_EncodedA:
    case QPaintDevice::PdmDevicePixelRatioF_EncodedB:
        return QPaintDevice::encodeMetricF(metric, m_offscreenBuffer.devicePixelRatio());
#endif
    default:
        Q_ASSERT(false);
        break;
    }

    return 0;
}

}   // namespace pdf