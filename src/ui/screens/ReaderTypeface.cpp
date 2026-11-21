#include "ui/screens/ReaderScreen.h"
#include "ui/screens/ReaderLayout.h"

namespace screens {
    void ReaderScreen::activateFace(const FontCatalog::Face& face) {
        text_.setFont(face.raster.get());
    }

    void ReaderScreen::refreshTypeface() {
        // FontCatalog keeps one mutable strike per family. Page browsing can
        // replace the object referenced by face_ without changing the word.
        if (pageFontActive_) {
            pageFontActive_ = false;
            pageState_ = {};
            browse_.layout = {};
            loadedWordIndex_ = SIZE_MAX;
            loadedFontSizeIndex_ = 0xFF;
        }
        if (loadedWordIndex_ == session.state.wordIndex && loadedFontSizeIndex_ == typography_.fontSizeIndex) {
            activateFace(face_);
            return;
        }
        loadedWordIndex_ = session.state.wordIndex;
        const size_t family = fontChoice(session.state.wordIndex);
        if (loadedFamilyIndex_ != family || loadedFontSizeIndex_ != typography_.fontSizeIndex) {
            loadedFamilyIndex_ = family;
            loadedFontSizeIndex_ = typography_.fontSizeIndex;
            ++fontRevision_;
        }
        face_ = readerTypeface(family);
        activateFace(face_);
    }

    FontCatalog::Face ReaderScreen::readerTypeface(size_t family) {
        if (typography_.fontSizeIndex != RFont4::kExtraLargeStrikeIndex)
            return fonts.loadFace(family, typography_.fontSizeIndex);
        constexpr std::array<size_t, 5> sizes{RFont4::kExtraLargeStrikeIndex, 0, 1, 2, RFont4::kCompactStrikeIndex};
        const auto area = readerLayout::readingArea(width_, height_, false);
        return fonts.loadFaceFittingHeight(family, sizes, area.h - 12);
    }

    FontCatalog::Face ReaderScreen::pageTypeface(size_t wordIndex) {
        pageFontActive_ = true;
        if (pagePreview_ && (typography_.fontSizeIndex == 0 || typography_.fontSizeIndex == RFont4::kExtraLargeStrikeIndex)) {
            const bool vertical = session.metadata.writingMode == WritingMode::verticalRl;
            const auto area = readerLayout::readingArea(width_, height_, vertical);
            constexpr std::array<size_t, 5> sizes{0, 1, 2, RFont4::kPreviewStrikeIndex, RFont4::kCompactStrikeIndex};
            return fonts.loadFaceFittingHeight(fontChoice(wordIndex), sizes, (area.h - 8) / readerLayout::kPreviewRows - 2, vertical);
        }
        // Smooth Small for the modern browser. The legacy playing page mode
        // keeps its compact strike; it is not exposed in this preview UI.
        const size_t size = pageBrowsing_ ? RFont4::sizeIndexForId("small") : readerLayout::pageStrikeIndex();
        return fonts.loadFace(fontChoice(wordIndex), size);
    }
} // namespace screens
