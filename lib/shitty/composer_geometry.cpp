/* Copyright (C) 2026 Shitty team. MIT licensed. */
#include "composer.h"

#include "font_pack.h"
#include "listener.h"
#include "options.h"

#include <std/alg/minmax.h>
#include <std/dbg/assert.h>

using namespace stl;

void Composer::setContentScale(float scale) {
    STD_ASSERT(scale > 0.0f);
    if (contentScale == scale) return;
    contentScale = scale;
    for (IntrusiveNode* node = contentScaleChangedListeners.mutFront();
         node != contentScaleChangedListeners.mutEnd();) {
        Listener* const listener = static_cast<Listener*>(node);
        node = node->next;
        listener->onListen();
    }
}

void Composer::setGlyphSize(u16 width, u16 height) {
    STD_ASSERT(width != 0);
    STD_ASSERT(height != 0);
    glyphWidth = width;
    glyphHeight = height;
}

float Composer::boxDrawingStroke() const {
    if (fonts != nullptr) {
        const float measured = fonts->boxDrawingStroke();
        if (measured > 0.0f) return measured;
    }
    const u16 shortSide = glyphWidth < glyphHeight ? glyphWidth : glyphHeight;
    const float fallback = (float)(shortSide) / 12.0f;
    return fallback > 1.0f ? fallback : 1.0f;
}

void Composer::setCellExtras(CellExtraStore* extras) {
    if (cellExtras == extras) return;
    cellExtras = extras;
    for (IntrusiveNode* node = cellExtrasChangedListeners.mutFront();
         node != cellExtrasChangedListeners.mutEnd();) {
        Listener* const listener = static_cast<Listener*>(node);
        node = node->next;
        listener->onListen();
    }
}

u16 Composer::borderPixels() const {
    const float scaled = opts->border * contentScale;
    if (!(scaled > 0)) return 0;
    if (scaled >= 3000) return 3000;
    return (u16)(scaled + 0.5f);
}

void Composer::resize(u16 pixelWidth_, u16 pixelHeight_) {
    STD_ASSERT(glyphWidth != 0);
    STD_ASSERT(glyphHeight != 0);
    const u32 borders = 2u * borderPixels();
    const u32 contentWidth = pixelWidth_ > borders ? pixelWidth_ - borders : 0;
    const u32 contentHeight = pixelHeight_ > borders ? pixelHeight_ - borders : 0;
    const u16 columns_ = (u16)(max<u32>(1, contentWidth / glyphWidth));
    const u16 rows_ = (u16)(max<u32>(1, contentHeight / glyphHeight));
    if (columns == columns_ && rows == rows_ &&
        pixelWidth == pixelWidth_ && pixelHeight == pixelHeight_) return;
    columns = columns_;
    rows = rows_;
    pixelWidth = pixelWidth_;
    pixelHeight = pixelHeight_;
    for (IntrusiveNode* node = resizedListeners.mutFront(); node != resizedListeners.mutEnd();) {
        Listener* const listener = static_cast<Listener*>(node);
        node = node->next;
        listener->onListen();
    }
}
