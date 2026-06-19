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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QLocale>
#include <QTextStream>
#include <QImage>

#include "pdfglobal.h"
#include "pdfdocument.h"
#include "pdfdocumentreader.h"
#include "pdfdocumentwriter.h"
#include "pdfoptimizer.h"
#include "pdfimageoptimizer.h"
#include "pdfdocumentsanitizer.h"
#include "pdfobjectutils.h"
#include "pdfobject.h"
#include "pdfcatalog.h"
#include "pdfpage.h"
#include "pdfconstants.h"
#include "pdfexception.h"
#include "pdfstreamfilters.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <locale>

using namespace pdf;

// ============================================================================
//  Helpers
// ============================================================================

static int error(const QString &msg)
{
    QTextStream err(stderr);
    err << "Error: " << msg << "\n";
    return 1;
}

static int info(const QString &msg)
{
    QTextStream out(stdout);
    out << msg << "\n";
    return 0;
}

static PDFDocument readDocument(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        throw PDFException(PDFTranslationContext::tr("Cannot open file '%1'.").arg(path));
    }

    QByteArray data = file.readAll();
    file.close();

    PDFDocumentReader reader(nullptr, std::function<QString(bool*)>(), true, false);
    PDFDocument document = reader.readFromBuffer(data);
    if (!document.getStorage().getObjects().empty())
    {
        return document;
    }

    throw PDFException(PDFTranslationContext::tr("Failed to read document '%1'.").arg(path));
}

static bool writeDocument(const QString &path, const PDFDocument &document)
{
    PDFDocumentWriter writer(nullptr);
    PDFOperationResult result = writer.write(path, &document, true);
    return static_cast<bool>(result);
}

// ============================================================================
//  Command: optimize  (Document Optimization)
// ============================================================================

static int commandOptimize(const QString &input, const QString &output,
                           PDFOptimizer::OptimizationFlags flags)
{
    try
    {
        PDFDocument document = readDocument(input);

        PDFOptimizer optimizer(flags, nullptr);
        optimizer.setDocument(&document);
        optimizer.optimize();

        document = optimizer.takeOptimizedDocument();

        if (!writeDocument(output, document))
        {
            return error("Failed to write optimized document.");
        }

        return info("Document optimized successfully: " + output);
    }
    catch (const PDFException &e)
    {
        return error(e.getMessage());
    }
}

// ============================================================================
//  Command: image-optimize  (Image Optimization)
// ============================================================================

static int commandImageOptimize(const QString &input, const QString &output,
                                const PDFImageOptimizer::Settings &settings,
                                const PDFImageOptimizer::ImageOverrides &overrides)
{
    try
    {
        PDFDocument document = readDocument(input);

        PDFImageOptimizer imageOptimizer;
        document = imageOptimizer.optimize(&document, settings, overrides);

        if (!writeDocument(output, document))
        {
            return error("Failed to write image-optimized document.");
        }

        return info("Images optimized successfully: " + output);
    }
    catch (const PDFException &e)
    {
        return error(e.getMessage());
    }
}

// ============================================================================
//  Command: sanitize  (Document Sanitization)
// ============================================================================

static int commandSanitize(const QString &input, const QString &output,
                           PDFDocumentSanitizer::SanitizationFlags flags)
{
    try
    {
        PDFDocument document = readDocument(input);

        PDFDocumentSanitizer sanitizer(PDFDocumentSanitizer::None, nullptr);
        sanitizer.setFlags(flags);
        sanitizer.setDocument(&document);
        sanitizer.sanitize();

        document = sanitizer.takeSanitizedDocument();

        if (!writeDocument(output, document))
        {
            return error("Failed to write sanitized document.");
        }

        return info("Document sanitized successfully: " + output);
    }
    catch (const PDFException &e)
    {
        return error(e.getMessage());
    }
}

// ============================================================================
//  Command: statistics  (Object Statistics)
// ============================================================================

static int commandStatistics(const QString &input, bool byClass, bool byType)
{
    try
    {
        PDFDocument document = readDocument(input);

        PDFObjectClassifier classifier;
        classifier.classify(&document);

        PDFObjectClassifier::Statistics stats = classifier.calculateStatistics(&document);
        QLocale locale;
        QTextStream out(stdout);

        if (byClass)
        {
            qint64 totalBytes = 0;
            for (const auto &item : stats.statistics)
            {
                totalBytes += item.second.bytes;
            }

            out << "\n=== Statistics by Object Class ===\n\n";
            out << QString("%1 | %2 | %3 | %4\n")
                       .arg("Class", -20)
                       .arg("Percentage", 12)
                       .arg("Count", 10)
                       .arg("Space Usage", 15);
            out << QString(60, '-') << "\n";

            auto printRow = [&](PDFObjectClassifier::Type type, const QString &label) {
                auto it = stats.statistics.find(type);
                if (it == stats.statistics.end())
                    return;
                const auto &item = it->second;
                double pct = totalBytes > 0 ? 100.0 * double(item.bytes) / double(totalBytes) : 0.0;
                out << QString("%1 | %2% | %3 | %4 bytes\n")
                           .arg(label, -20)
                           .arg(locale.toString(pct, 'f', 2), 10)
                           .arg(locale.toString(item.count), 10)
                           .arg(locale.toString(item.bytes), 15);
            };

            printRow(PDFObjectClassifier::Page, "Page");
            printRow(PDFObjectClassifier::ContentStream, "ContentStream");
            printRow(PDFObjectClassifier::GraphicState, "GraphicState");
            printRow(PDFObjectClassifier::ColorSpace, "ColorSpace");
            printRow(PDFObjectClassifier::Pattern, "Pattern");
            printRow(PDFObjectClassifier::Shading, "Shading");
            printRow(PDFObjectClassifier::Image, "Image");
            printRow(PDFObjectClassifier::Form, "Form");
            printRow(PDFObjectClassifier::Font, "Font");
            printRow(PDFObjectClassifier::Action, "Action");
            printRow(PDFObjectClassifier::Annotation, "Annotation");
            printRow(PDFObjectClassifier::None, "Other");

            out << "\n";
        }

        if (byType)
        {
            qint64 totalObjects = 0;
            for (PDFObject::Type type : PDFObject::getTypes())
            {
                totalObjects += stats.objectCountByType[size_t(type)];
            }

            out << "=== Statistics by Object Type ===\n\n";
            out << QString("%1 | %2 | %3\n")
                       .arg("Type", -20)
                       .arg("Percentage", 12)
                       .arg("Count", 10);
            out << QString(45, '-') << "\n";

            for (PDFObject::Type type : PDFObject::getTypes())
            {
                qint64 count = stats.objectCountByType[size_t(type)];
                if (count == 0)
                    continue;
                double pct = totalObjects > 0 ? 100.0 * double(count) / double(totalObjects) : 0.0;
                out << QString("%1 | %2% | %3\n")
                           .arg(PDFObjectUtils::getObjectTypeName(type), -20)
                           .arg(locale.toString(pct, 'f', 2), 10)
                           .arg(locale.toString(count), 10);
            }
            out << "\n";
        }

        return 0;
    }
    catch (const PDFException &e)
    {
        return error(e.getMessage());
    }
}

// ============================================================================
//  Command: inspect  (Object Inspector - dump object tree)
// ============================================================================

static void inspectObject(const PDFDocument &document,
                          const PDFObjectStorage &storage,
                          const PDFObject &object,
                          QTextStream &out,
                          int indent = 0,
                          std::set<PDFObjectReference> *visited = nullptr,
                          bool showStreams = false,
                          bool showSizes = false)
{
    if (indent > 8)
        return;

    QString pad(indent * 2, ' ');

    switch (object.getType())
    {
        case PDFObject::Type::Null:
            out << pad << "null\n";
            break;

        case PDFObject::Type::Bool:
            out << pad << (object.getBool() ? "true" : "false") << "\n";
            break;

        case PDFObject::Type::Int:
            out << pad << object.getInteger() << "\n";
            break;

        case PDFObject::Type::Real:
            out << pad << QString::number(object.getReal(), 'f', 4) << "\n";
            break;

        case PDFObject::Type::String:
            out << pad << "(" << object.getString().left(80) << ")\n";
            break;

        case PDFObject::Type::Name:
            out << pad << "/" << object.getString() << "\n";
            break;

        case PDFObject::Type::Reference:
        {
            PDFObjectReference ref = object.getReference();
            out << pad << ref.objectNumber << " " << ref.generation << " R";
            if (showSizes)
            {
                qint64 size = PDFDocumentWriter::getObjectSize(&document, ref);
                out << "  [" << size << " bytes]";
            }
            out << "\n";

            if (visited)
            {
                if (visited->count(ref))
                {
                    out << pad << "  (circular reference)\n";
                    return;
                }
                visited->insert(ref);

                // Follow the reference
                const PDFObject &resolved = storage.getObjectByReference(ref);
                if (!resolved.isNull())
                {
                    inspectObject(document, storage, resolved, out, indent + 1, visited, showStreams, showSizes);
                }
            }
            break;
        }

        case PDFObject::Type::Array:
        {
            const PDFArray *array = object.getArray();
            out << pad << "Array [" << array->getCount() << " items]\n";
            for (size_t i = 0; i < array->getCount(); ++i)
            {
                out << pad << "  [" << i << "]: ";
                inspectObject(document, storage, storage.getObject(array->getItem(i)), out,
                              indent + 1, visited, showStreams, showSizes);
            }
            break;
        }

        case PDFObject::Type::Dictionary:
        {
            const PDFDictionary *dict = object.getDictionary();
            out << pad << "Dictionary [" << dict->getCount() << " entries]\n";
            for (size_t i = 0; i < dict->getCount(); ++i)
            {
                QByteArray key = dict->getKey(i).getString();
                PDFObject value = storage.getObject(dict->getValue(i));
                out << pad << "  /" << key << ": ";
                inspectObject(document, storage, value, out, indent + 1, visited, showStreams, showSizes);
            }
            break;
        }

        case PDFObject::Type::Stream:
        {
            const PDFStream *stream = object.getStream();
            const PDFDictionary *dict = stream->getDictionary();
            qint64 length = 0;
            if (dict->hasKey("Length"))
            {
                PDFObject lenObj = storage.getObject(dict->get("Length"));
                if (lenObj.isInt())
                    length = lenObj.getInteger();
            }

            QString subtype;
            if (dict->hasKey("Subtype"))
            {
                PDFObject stObj = storage.getObject(dict->get("Subtype"));
                if (stObj.isName())
                    subtype = QString::fromLatin1(stObj.getString());
            }

            QString type;
            if (dict->hasKey("Type"))
            {
                PDFObject tObj = storage.getObject(dict->get("Type"));
                if (tObj.isName())
                    type = QString::fromLatin1(tObj.getString());
            }

            out << pad << "Stream";
            if (!type.isEmpty())
                out << "  Type=" << type;
            if (!subtype.isEmpty())
                out << "  Subtype=" << subtype;
            out << "  " << length << " bytes\n";

            // Show dictionary
            if (dict->getCount() > 0)
            {
                out << pad << "  Dict:\n";
                inspectObject(document, storage, PDFObject::createDictionary(
                                          std::make_shared<PDFDictionary>(*dict)),
                              out, indent + 2, visited, showStreams, showSizes);
            }

            if (showStreams && length > 0 && length < 1024 * 1024)
            {
                QByteArray decoded = document.getDecodedStream(stream);
                if (!decoded.isEmpty())
                {
                    out << pad << "  Decoded content (" << decoded.size() << " bytes):\n";
                    QString contentText = QString::fromLatin1(decoded.left(4096));
                    for (const QString &line : contentText.split('\n'))
                    {
                        out << pad << "    " << line << "\n";
                    }
                }
            }
            break;
        }

        default:
            break;
    }
}

static int commandInspect(const QString &input,
                          const QString &modeStr,
                          bool showStreams,
                          bool showSizes,
                          const QString &outputFile)
{
    try
    {
        PDFDocument document = readDocument(input);
        const PDFObjectStorage &storage = document.getStorage();

        // Output target
        QFile outFile;
        QTextStream out(stdout);
        if (!outputFile.isEmpty())
        {
            outFile.setFileName(outputFile);
            if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                return error("Cannot open output file: " + outputFile);
            }
            out.setDevice(&outFile);
        }

        const auto &objects = storage.getObjects();

        if (modeStr == "list" || modeStr == "all")
        {
            out << "=== Indirect Object List ===\n\n";
            for (size_t i = 1; i < objects.size(); ++i)
            {
                const auto &entry = objects[i];
                if (entry.object.isNull())
                    continue;

                PDFObjectReference ref(PDFInteger(i), entry.generation);
                out << ref.objectNumber << " " << ref.generation << " R: ";

                QString typeName = PDFObjectUtils::getObjectTypeName(entry.object.getType());
                out << typeName;

                if (showSizes)
                {
                    qint64 size = PDFDocumentWriter::getObjectSize(nullptr, ref);
                    out << "  [" << size << " bytes]";
                }
                out << "\n";
            }
        }

        if (modeStr == "document" || modeStr == "all")
        {
            out << "\n=== Document Trailer / Root ===\n\n";
            std::set<PDFObjectReference> visited;
            inspectObject(document, storage, storage.getTrailerDictionary(), out, 0,
                          &visited, showStreams, showSizes);
        }

        if (modeStr == "pages" || modeStr == "all")
        {
            out << "\n=== Pages ===\n\n";
            const PDFCatalog *catalog = document.getCatalog();
            if (catalog)
            {
                for (size_t i = 0; i < catalog->getPageCount(); ++i)
                {
                    const PDFPage *page = catalog->getPage(i);
                    if (page)
                    {
                        out << "Page " << (i + 1) << " (ref: "
                            << page->getPageReference().objectNumber << " "
                            << page->getPageReference().generation << " R)\n";
                    }
                }
            }
        }

        if (modeStr == "images" || modeStr == "all")
        {
            out << "\n=== Images ===\n\n";
            PDFObjectClassifier classifier;
            classifier.classify(&document);
            auto refs = classifier.getObjectsByType(PDFObjectClassifier::Image);
            for (const auto &ref : refs)
            {
                const PDFObject &obj = storage.getObjectByReference(ref);
                if (obj.isStream())
                {
                    const PDFDictionary *dict = obj.getStream()->getDictionary();
                    int w = 0, h = 0, bpc = 8;
                    if (dict->hasKey("Width"))
                    {
                        PDFObject wObj = storage.getObject(dict->get("Width"));
                        if (wObj.isInt())
                            w = (int)wObj.getInteger();
                    }
                    if (dict->hasKey("Height"))
                    {
                        PDFObject hObj = storage.getObject(dict->get("Height"));
                        if (hObj.isInt())
                            h = (int)hObj.getInteger();
                    }
                    if (dict->hasKey("BitsPerComponent"))
                    {
                        PDFObject bObj = storage.getObject(dict->get("BitsPerComponent"));
                        if (bObj.isInt())
                            bpc = (int)bObj.getInteger();
                    }
                    out << "  " << ref.objectNumber << " " << ref.generation << " R"
                        << "  " << w << "x" << h << "  " << bpc << " bpc";
                    if (showSizes)
                    {
                        qint64 sz = PDFDocumentWriter::getObjectSize(&document, ref);
                        out << "  [" << sz << " bytes]";
                    }
                    out << "\n";
                }
            }
        }

        if (modeStr == "fonts" || modeStr == "all")
        {
            out << "\n=== Fonts ===\n\n";
            PDFObjectClassifier classifier;
            classifier.classify(&document);
            auto refs = classifier.getObjectsByType(PDFObjectClassifier::Font);
            for (const auto &ref : refs)
            {
                const PDFObject &obj = storage.getObjectByReference(ref);
                out << "  " << ref.objectNumber << " " << ref.generation << " R";
                if (obj.isStream() || obj.isDictionary())
                {
                    const PDFDictionary *dict = obj.isStream() ? obj.getStream()->getDictionary() : obj.getDictionary();
                    if (dict)
                    {
                        PDFObject bn = storage.getObject(dict->get("BaseFont"));
                        if (bn.isName())
                            out << "  " << QString::fromLatin1(bn.getString());
                    }
                }
                if (showSizes)
                {
                    qint64 sz = PDFDocumentWriter::getObjectSize(&document, ref);
                    out << "  [" << sz << " bytes]";
                }
                out << "\n";
            }
        }

        if (modeStr == "annotations" || modeStr == "all")
        {
            out << "\n=== Annotations ===\n\n";
            PDFObjectClassifier classifier;
            classifier.classify(&document);
            auto refs = classifier.getObjectsByType(PDFObjectClassifier::Annotation);
            for (const auto &ref : refs)
            {
                const PDFObject &obj = storage.getObjectByReference(ref);
                out << "  " << ref.objectNumber << " " << ref.generation << " R";
                if (showSizes)
                {
                    qint64 sz = PDFDocumentWriter::getObjectSize(&document, ref);
                    out << "  [" << sz << " bytes]";
                }
                out << "\n";
            }
        }

        return 0;
    }
    catch (const PDFException &e)
    {
        return error(e.getMessage());
    }
}

// ============================================================================
//  Main
// ============================================================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("pdf_simplify");
    QCoreApplication::setApplicationVersion(PDF_LIBRARY_VERSION);

    // ── Top-level parser ───────────────────────────────────────────────
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "PDF4QT Simplify - PDF optimization, sanitization, statistics and inspection tool.\n"
        "Usage: pdf_simplify <command> [options] <input.pdf> [output.pdf]");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("command", "Command: optimize, image-optimize, sanitize, statistics, inspect");

    // ── Common options ─────────────────────────────────────────────────
    QCommandLineOption helpOption(QStringList() << "h" << "help", "Show help");
    // We use our own dispatch, so parse first manually
    parser.parse(QCoreApplication::arguments());

    QStringList args = parser.positionalArguments();
    if (args.isEmpty())
    {
        parser.showHelp();
        return 0;
    }

    QString command = args.first();

    // ────────────────────────────────────────────────────────────────────
    //  COMMAND: optimize
    // ────────────────────────────────────────────────────────────────────
    if (command == "optimize")
    {
        QCommandLineParser optParser;
        optParser.setApplicationDescription("Document optimization: remove unused/duplicate data and recompress streams.");
        optParser.addHelpOption();
        optParser.addPositionalArgument("input", "Input PDF file");
        optParser.addPositionalArgument("output", "Output PDF file");

        QCommandLineOption derefOpt("dereference-simple-objects",
                                    "Dereference (embed) simple objects (integers, bools, reals).");
        QCommandLineOption removeNullOpt("remove-null-objects",
                                         "Remove null objects from dictionary entries.");
        QCommandLineOption removeUnusedOpt("remove-unused-objects",
                                           "Remove objects not reachable from the document root.");
        QCommandLineOption mergeOpt("merge-identical-objects",
                                    "Merge identical objects.");
        QCommandLineOption shrinkOpt("shrink-object-storage",
                                     "Shrink object storage (compress free entries).");
        QCommandLineOption recompressOpt("recompress-flate-streams",
                                         "Recompress Flate streams with maximum compression.");

        optParser.addOption(derefOpt);
        optParser.addOption(removeNullOpt);
        optParser.addOption(removeUnusedOpt);
        optParser.addOption(mergeOpt);
        optParser.addOption(shrinkOpt);
        optParser.addOption(recompressOpt);

        optParser.process(app);

        QStringList posArgs = optParser.positionalArguments();
        if (posArgs.size() < 3)
            optParser.showHelp();

        PDFOptimizer::OptimizationFlags flags = PDFOptimizer::None;
        if (optParser.isSet(derefOpt))
            flags |= PDFOptimizer::DereferenceSimpleObjects;
        if (optParser.isSet(removeNullOpt))
            flags |= PDFOptimizer::RemoveNullObjects;
        if (optParser.isSet(removeUnusedOpt))
            flags |= PDFOptimizer::RemoveUnusedObjects;
        if (optParser.isSet(mergeOpt))
            flags |= PDFOptimizer::MergeIdenticalObjects;
        if (optParser.isSet(shrinkOpt))
            flags |= PDFOptimizer::ShrinkObjectStorage;
        if (optParser.isSet(recompressOpt))
            flags |= PDFOptimizer::RecompressFlateStreams;
        if (flags == PDFOptimizer::None)
            flags = PDFOptimizer::All;

        return commandOptimize(posArgs[1], posArgs[2], flags);
    }

    // ────────────────────────────────────────────────────────────────────
    //  COMMAND: image-optimize
    // ────────────────────────────────────────────────────────────────────
    else if (command == "image-optimize")
    {
        QCommandLineParser imgParser;
        imgParser.setApplicationDescription("Image optimization: resample, convert color mode, and recompress images.");
        imgParser.addHelpOption();
        imgParser.addPositionalArgument("input", "Input PDF file");
        imgParser.addPositionalArgument("output", "Output PDF file");

        // Global settings
        imgParser.addOption(QCommandLineOption(QStringList{"a", "auto-mode"}, "Enable auto mode (analyze image content)."));
        imgParser.addOption(QCommandLineOption(QStringList{"c", "color-mode"}, "Color mode: auto, preserve, color, grayscale, bitonal.", "mode"));
        imgParser.addOption(QCommandLineOption(QStringList{"g", "goal"}, "Optimization goal: prefer-quality, minimum-size.", "goal"));
        imgParser.addOption(QCommandLineOption(QStringList{"k", "keep-original"}, "Keep original image if re-encode is not smaller."));
        imgParser.addOption(QCommandLineOption(QStringList{"t", "preserve-transparency"}, "Preserve transparency via soft mask."));

        // Color profile
        imgParser.addOption(QCommandLineOption(QStringList{"color-algorithm"}, "Algorithm for color images: auto, flate, jpeg, jpeg2000.", "algo"));
        imgParser.addOption(QCommandLineOption(QStringList{"color-target-dpi"}, "Target DPI for color images (0 = keep original).", "dpi"));
        imgParser.addOption(QCommandLineOption(QStringList{"color-jpeg-quality"}, "JPEG quality for color images (0-100).", "q"));
        imgParser.addOption(QCommandLineOption(QStringList{"color-jpeg2000-rate"}, "JPEG2000 rate for color images.", "rate"));

        // Grayscale profile
        imgParser.addOption(QCommandLineOption(QStringList{"gray-algorithm"}, "Algorithm for grayscale images.", "algo"));
        imgParser.addOption(QCommandLineOption(QStringList{"gray-target-dpi"}, "Target DPI for grayscale images.", "dpi"));
        imgParser.addOption(QCommandLineOption(QStringList{"gray-jpeg-quality"}, "JPEG quality for grayscale images (0-100).", "q"));

        // Bitonal profile
        imgParser.addOption(QCommandLineOption(QStringList{"bitonal-algorithm"}, "Algorithm for bitonal images.", "algo"));
        imgParser.addOption(QCommandLineOption(QStringList{"bitonal-target-dpi"}, "Target DPI for bitonal images.", "dpi"));

        // Global
        imgParser.addOption(QCommandLineOption(QStringList{"png-predictor"}, "Enable PNG predictor for Flate compression."));
        imgParser.addOption(QCommandLineOption(QStringList{"resample-filter"}, "Resample filter: nearest, bilinear, bicubic, lanczos.", "filter"));

        imgParser.process(app);

        QStringList posArgs = imgParser.positionalArguments();
        if (posArgs.size() < 3)
            imgParser.showHelp();

        PDFImageOptimizer::Settings settings = PDFImageOptimizer::Settings::createDefault();
        settings.enabled = true;

        if (imgParser.isSet("auto-mode"))
            settings.autoMode = true;
        if (imgParser.isSet("color-mode"))
        {
            QString mode = imgParser.value("color-mode");
            if (mode == "auto")          settings.colorMode = PDFImageOptimizer::ColorMode::Auto;
            else if (mode == "preserve") settings.colorMode = PDFImageOptimizer::ColorMode::Preserve;
            else if (mode == "color")    settings.colorMode = PDFImageOptimizer::ColorMode::Color;
            else if (mode == "grayscale") settings.colorMode = PDFImageOptimizer::ColorMode::Grayscale;
            else if (mode == "bitonal")   settings.colorMode = PDFImageOptimizer::ColorMode::Bitonal;
            else return error("Invalid color-mode: " + mode);
        }
        if (imgParser.isSet("goal"))
        {
            QString goal = imgParser.value("goal");
            if (goal == "prefer-quality")
                settings.goal = PDFImageOptimizer::OptimizationGoal::PreferQuality;
            else if (goal == "minimum-size")
                settings.goal = PDFImageOptimizer::OptimizationGoal::MinimumSize;
            else
                return error("Invalid goal: " + goal);
        }
        if (imgParser.isSet("keep-original"))
            settings.keepOriginalIfLarger = true;
        if (imgParser.isSet("preserve-transparency"))
            settings.preserveTransparency = true;

        auto parseAlgorithm = [](const QString &s) -> PDFImageOptimizer::CompressionAlgorithm {
            if (s == "auto")    return PDFImageOptimizer::CompressionAlgorithm::Auto;
            if (s == "flate")   return PDFImageOptimizer::CompressionAlgorithm::Flate;
            if (s == "jpeg")    return PDFImageOptimizer::CompressionAlgorithm::JPEG;
            if (s == "jpeg2000") return PDFImageOptimizer::CompressionAlgorithm::JPEG2000;
            return PDFImageOptimizer::CompressionAlgorithm::Auto;
        };

        // Color profile
        if (imgParser.isSet("color-algorithm"))
            settings.colorProfile.algorithm = parseAlgorithm(imgParser.value("color-algorithm"));
        if (imgParser.isSet("color-target-dpi"))
            settings.colorProfile.targetDpi = imgParser.value("color-target-dpi").toInt();
        if (imgParser.isSet("color-jpeg-quality"))
            settings.colorProfile.jpegQuality = imgParser.value("color-jpeg-quality").toInt();
        if (imgParser.isSet("color-jpeg2000-rate"))
            settings.colorProfile.jpeg2000Rate = imgParser.value("color-jpeg2000-rate").toFloat();

        // Grayscale profile
        if (imgParser.isSet("gray-algorithm"))
            settings.grayProfile.algorithm = parseAlgorithm(imgParser.value("gray-algorithm"));
        if (imgParser.isSet("gray-target-dpi"))
            settings.grayProfile.targetDpi = imgParser.value("gray-target-dpi").toInt();
        if (imgParser.isSet("gray-jpeg-quality"))
            settings.grayProfile.jpegQuality = imgParser.value("gray-jpeg-quality").toInt();

        // Bitonal profile
        if (imgParser.isSet("bitonal-algorithm"))
            settings.bitonalProfile.algorithm = parseAlgorithm(imgParser.value("bitonal-algorithm"));
        if (imgParser.isSet("bitonal-target-dpi"))
            settings.bitonalProfile.targetDpi = imgParser.value("bitonal-target-dpi").toInt();

        // Global
        if (imgParser.isSet("png-predictor"))
        {
            settings.colorProfile.enablePngPredictor = true;
            settings.grayProfile.enablePngPredictor = true;
            settings.bitonalProfile.enablePngPredictor = true;
        }
        if (imgParser.isSet("resample-filter"))
        {
            QString filter = imgParser.value("resample-filter");
            PDFImage::ResampleFilter rf = PDFImage::ResampleFilter::Bicubic;
            if (filter == "nearest")    rf = PDFImage::ResampleFilter::Nearest;
            else if (filter == "bilinear") rf = PDFImage::ResampleFilter::Bilinear;
            else if (filter == "bicubic")  rf = PDFImage::ResampleFilter::Bicubic;
            else if (filter == "lanczos")  rf = PDFImage::ResampleFilter::Lanczos;
            settings.colorProfile.resampleFilter = rf;
            settings.grayProfile.resampleFilter = rf;
            settings.bitonalProfile.resampleFilter = rf;
        }

        return commandImageOptimize(posArgs[1], posArgs[2], settings, {});
    }

    // ────────────────────────────────────────────────────────────────────
    //  COMMAND: sanitize
    // ────────────────────────────────────────────────────────────────────
    else if (command == "sanitize")
    {
        QCommandLineParser sanParser;
        sanParser.setApplicationDescription(
            "Document sanitization: remove sensitive/private data from a PDF.");
        sanParser.addHelpOption();
        sanParser.addPositionalArgument("input", "Input PDF file");
        sanParser.addPositionalArgument("output", "Output PDF file");

        sanParser.addOption(QCommandLineOption(QStringList{"remove-document-info"}, "Remove document information dictionary."));
        sanParser.addOption(QCommandLineOption(QStringList{"remove-metadata"}, "Remove all metadata streams."));
        sanParser.addOption(QCommandLineOption(QStringList{"remove-outline"}, "Remove document outline (bookmarks)."));
        sanParser.addOption(QCommandLineOption(QStringList{"remove-file-attachments"}, "Remove file attachments."));
        sanParser.addOption(QCommandLineOption(QStringList{"remove-embedded-search-index"}, "Remove embedded search index."));
        sanParser.addOption(QCommandLineOption(QStringList{"remove-markup-annotations"}, "Remove markup annotations."));
        sanParser.addOption(QCommandLineOption(QStringList{"remove-page-thumbnails"}, "Remove page thumbnails."));
        sanParser.addOption(QCommandLineOption(QStringList{"remove-page-labels"}, "Remove page labels."));
        sanParser.addOption(QCommandLineOption(QStringList{"remove-invisible-text"}, "Remove invisible text (OCR layer)."));

        sanParser.process(app);

        QStringList posArgs = sanParser.positionalArguments();
        if (posArgs.size() < 3)
            sanParser.showHelp();

        PDFDocumentSanitizer::SanitizationFlags flags = PDFDocumentSanitizer::None;
        if (sanParser.isSet("remove-document-info"))
            flags |= PDFDocumentSanitizer::DocumentInfo;
        if (sanParser.isSet("remove-metadata"))
            flags |= PDFDocumentSanitizer::Metadata;
        if (sanParser.isSet("remove-outline"))
            flags |= PDFDocumentSanitizer::Outline;
        if (sanParser.isSet("remove-file-attachments"))
            flags |= PDFDocumentSanitizer::FileAttachments;
        if (sanParser.isSet("remove-embedded-search-index"))
            flags |= PDFDocumentSanitizer::EmbeddedSearchIndex;
        if (sanParser.isSet("remove-markup-annotations"))
            flags |= PDFDocumentSanitizer::MarkupAnnotations;
        if (sanParser.isSet("remove-page-thumbnails"))
            flags |= PDFDocumentSanitizer::PageThumbnails;
        if (sanParser.isSet("remove-page-labels"))
            flags |= PDFDocumentSanitizer::PageLabels;
        if (sanParser.isSet("remove-invisible-text"))
            flags |= PDFDocumentSanitizer::InvisibleText;
        if (flags == PDFDocumentSanitizer::None)
            flags = PDFDocumentSanitizer::All;

        return commandSanitize(posArgs[1], posArgs[2], flags);
    }

    // ────────────────────────────────────────────────────────────────────
    //  COMMAND: statistics
    // ────────────────────────────────────────────────────────────────────
    else if (command == "statistics")
    {
        QCommandLineParser statParser;
        statParser.setApplicationDescription(
            "Compute statistics of internal objects used in a document.");
        statParser.addHelpOption();
        statParser.addPositionalArgument("input", "Input PDF file");

        statParser.addOption(QCommandLineOption(QStringList{"by-class"}, "Show statistics by object class."));
        statParser.addOption(QCommandLineOption(QStringList{"by-type"}, "Show statistics by object type."));

        statParser.process(app);

        QStringList posArgs = statParser.positionalArguments();
        if (posArgs.size() < 2)
            statParser.showHelp();

        bool showClass = statParser.isSet("by-class");
        bool showType = statParser.isSet("by-type");
        if (!showClass && !showType)
        {
            showClass = true;
            showType = true;
        }

        return commandStatistics(posArgs[1], showClass, showType);
    }

    // ────────────────────────────────────────────────────────────────────
    //  COMMAND: inspect
    // ────────────────────────────────────────────────────────────────────
    else if (command == "inspect")
    {
        QCommandLineParser inspParser;
        inspParser.setApplicationDescription(
            "Inspect the internal object structure of a PDF document.");
        inspParser.addHelpOption();
        inspParser.addPositionalArgument("input", "Input PDF file");

        inspParser.addOption(QCommandLineOption(QStringList{"m", "mode"}, "Inspection mode: list, document, pages, images, fonts, annotations, all", "mode"));
        inspParser.addOption(QCommandLineOption(QStringList{"s", "show-streams"}, "Decode and display stream content."));
        inspParser.addOption(QCommandLineOption(QStringList{"z", "show-sizes"}, "Show object byte sizes."));
        inspParser.addOption(QCommandLineOption(QStringList{"o", "output"}, "Write output to file instead of stdout.", "file"));

        inspParser.process(app);

        QStringList posArgs = inspParser.positionalArguments();
        if (posArgs.size() < 2)
            inspParser.showHelp();

        QString mode = inspParser.value("mode");
        if (mode.isEmpty())
            mode = "list";

        return commandInspect(posArgs[1], mode,
                              inspParser.isSet("show-streams"),
                              inspParser.isSet("show-sizes"),
                              inspParser.value("output"));
    }

    // ────────────────────────────────────────────────────────────────────
    //  Unknown command
    // ────────────────────────────────────────────────────────────────────
    else
    {
        QTextStream err(stderr);
        err << "Unknown command: " << command << "\n\n";
        err << "Available commands:\n";
        err << "  optimize         Document optimization (dereference, remove unused, merge, recompress)\n";
        err << "  image-optimize   Image optimization (resample, color conversion, compression)\n";
        err << "  sanitize         Document sanitization (remove metadata, outlines, annotations, etc.)\n";
        err << "  statistics       Object statistics (by class and by type)\n";
        err << "  inspect          Object inspector (dump object tree, list objects)\n";
        err << "\n";
        err << "Use 'pdf_simplify <command> --help' for command-specific options.\n";
        return 1;
    }

    return 0;
}
