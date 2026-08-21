/* Copyright (C) 2026 Shitty team. MIT licensed. */
#include "composer.h"

#include "brand.h"
#include "cell_extra_store.h"
#include "options.h"

#include <std/mem/small_obj_allocator.h>

using namespace stl;

Composer::Composer(ObjPool* pool_, ComposerProfile)
    : pool(pool_)
    , brand(Brand::generic())
{
    Options* const headlessOptions = pool->make<Options>();
    headlessOptions->saveLines = 1000;
    opts = headlessOptions;
    cellExtras = CellExtraStore::create(*this, 0);
    smallObjects = SmallObjAllocator::create(pool);
}
