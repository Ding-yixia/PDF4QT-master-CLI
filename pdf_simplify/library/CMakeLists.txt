# MIT License
#
# Copyright (c) 2018-2025 Jakub Melka and Contributors
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# ─── Export header ──────────────────────────────────────────────────────────
# For a static library the export macros are no-ops.  We generate a minimal
# header so that #include "pdf4qtlibcore_export.h" (pulled in by pdfglobal.h)
# still compiles.
set(PDF4QT_EXPORT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/pdf4qtlibcore_export.h")
file(WRITE "${PDF4QT_EXPORT_HEADER}"
    "#ifndef PDF4QTLIBCORESHARED_EXPORT\n"
    "#define PDF4QTLIBCORESHARED_EXPORT\n"
    "#endif\n"
)

# ─── Library target (STATIC – no GUI dependency) ────────────────────────────
add_library(pdf_core STATIC
    sources/pdfaction.cpp
    sources/pdfalgorithmlcs.cpp
    sources/pdfannotation.cpp
    sources/pdfblendfunction.cpp
    sources/pdfccittfaxdecoder.cpp
    sources/pdfcertificatemanager.cpp
    sources/pdfcms.cpp
    sources/pdfdiff.cpp
    sources/pdfdocumentbuilder.cpp
    sources/pdfdocumentmanipulator.cpp
    sources/pdfpagegeometry.cpp
    sources/pdfdocumenttextflow.cpp
    sources/pdfdocumenttextfloweditormodel.cpp
    sources/pdfdocumentwriter.cpp
    sources/pdfexecutionpolicy.cpp
    sources/pdffile.cpp
    sources/pdfform.cpp
    sources/pdficontheme.cpp
    sources/pdfjavascriptscanner.cpp
    sources/pdfjbig2decoder.cpp
    sources/pdfmultimedia.cpp
    sources/pdfobject.cpp
    sources/pdfobjecteditormodel.cpp
    sources/pdfobjectutils.cpp
    sources/pdfoptimizer.cpp
    sources/pdfoptionalcontent.cpp
    sources/pdfoutline.cpp
    sources/pdfpagenavigation.cpp
    sources/pdfpagetransition.cpp
    sources/pdfpainterutils.cpp
    sources/pdfparser.cpp
    sources/pdfdocument.cpp
    sources/pdfdocumentreader.cpp
    sources/pdfpattern.cpp
    sources/pdfplugin.cpp
    sources/pdfprogress.cpp
    sources/pdfredact.cpp
    sources/pdfsecurityhandler.cpp
    sources/pdfsignaturehandler.cpp
    sources/pdfsnapper.cpp
    sources/pdfstructuretree.cpp
    sources/pdftextlayout.cpp
    sources/pdftransparencyrenderer.cpp
    sources/pdfutils.cpp
    sources/pdfxfaengine.cpp
    sources/pdfxreftable.cpp
    sources/pdfvisitor.cpp
    sources/pdfencoding.cpp
    sources/pdfcatalog.cpp
    sources/pdfpage.cpp
    sources/pdfstreamfilters.cpp
    sources/pdfcolorspaces.cpp
    sources/pdfrenderer.cpp
    sources/pdfpagecontentprocessor.cpp
    sources/pdfpainter.cpp
    sources/pdffunction.cpp
    sources/pdfnametounicode.cpp
    sources/pdffont.cpp
    sources/pdfimagecompressor.cpp
    sources/pdfimageoptimizer.cpp
    sources/pdfimage.cpp
    sources/pdfdocumentsanitizer.cpp
    sources/pdfimageconversion.cpp
    sources/pdfcolorconvertor.cpp
    sources/pdftextlayoutgenerator.cpp
    sources/pdfwidgetsnapshot.cpp
    sources/pdfcertificatestore.cpp
    sources/pdfblpainter.cpp
    sources/pdfapplicationtranslator.cpp
    sources/pdfsettings.cpp

    # Qt resource files

    # Qt resource files
    cmaps.qrc
    aatl.qrc
    fonts.qrc
)

# ─── Include directories ────────────────────────────────────────────────────
target_include_directories(pdf_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/sources"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"          # for config.h
        "${CMAKE_CURRENT_BINARY_DIR}"          # for generated pdf4qtlibcore_export.h
        "C:/Code/vcpkg/installed/x64-windows/include"   # for vcpkg headers (blend2d, etc.)
)

# ─── Link libraries ─────────────────────────────────────────────────────────
target_link_libraries(pdf_core
    PUBLIC
        Qt6::Core Qt6::Gui Qt6::Xml Qt6::Svg
        ZLIB::ZLIB
    PRIVATE
        ${LCMS2_LIBRARIES}
        OpenSSL::SSL OpenSSL::Crypto
        Freetype::Freetype
        openjp2
        JPEG::JPEG
        blend2d::blend2d
)

# ─── Platform-specific ──────────────────────────────────────────────────────
if(MINGW)
    target_link_libraries(pdf_core PRIVATE Secur32 Mscms Gdi32 User32 crypt32)
endif()

# ─── Properties ─────────────────────────────────────────────────────────────
set_target_properties(pdf_core PROPERTIES
    VERSION ${PDF4QT_VERSION}
    SOVERSION ${PDF4QT_VERSION}
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
