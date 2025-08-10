#pragma once

#include "../base/tile.h"
#include "../../config/config.h"  // Need complete type for value member Item

namespace LongView {
namespace Tiles {

class ItemTile final : public Tile {
    Q_OBJECT
    Q_DISABLE_COPY(ItemTile)

public:
    explicit ItemTile(const LongView::Config::Item& item, QWidget* parent = nullptr);
    ~ItemTile() override = default;

    void refresh() override; // MVP: no-op for now

    const LongView::Config::Item& item() const { return m_item; }

private:
    void buildContent();
    void applyOptionalProperties();

    const LongView::Config::Item m_item;
};

} // namespace Tiles
} // namespace LongView
