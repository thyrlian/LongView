#pragma once

#include "../base/tile.h"
#include "../../config/config.h"
#include <vector>
#include <QScrollArea>
#include <QFrame>

// Forward declarations
class QVBoxLayout;
class QLabel;

namespace LongView {
namespace Tiles {

class ItemTile;

/**
 * @brief GroupTile represents a group of items in the LongView application
 * 
 * Manages a collection of ItemTiles with group-level functionality.
 * Takes ownership of added ItemTiles and deletes them when removed.
 */
class GroupTile final : public Tile {
    Q_OBJECT

public:
    explicit GroupTile(const Config::Group& group, QWidget* parent = nullptr);
    ~GroupTile() override = default;

    // Group management methods
    bool addItemTile(ItemTile* itemTile);
    bool removeItemTile(ItemTile* itemTile);
    void clearItemTiles();
    void populateFromConfig();
    
    // New methods for recursive expansion control
    void expandAllItems();
    void collapseAllItems();
    
    const Config::Group& group() const { return m_group; }
    const std::vector<ItemTile*>& itemTiles() const { return m_itemTiles; }
    
    // Override Tile methods
    void refresh() override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void onItemTileExpandedChanged(bool expanded);
    void onItemTileCompletedChanged(bool completed);

private:
    void buildContent();
    void setupItemTileConnections(ItemTile* itemTile);
    void disconnectItemTile(ItemTile* itemTile);
    void updateExpandButtonState();
    void updateHeaderCount();
    void updateGroupCompletionState();

    const Config::Group m_group;
    std::vector<ItemTile*> m_itemTiles;
    QVBoxLayout* m_itemsLayout = nullptr;
    QLabel* m_headerInfo = nullptr;
    QLabel* m_itemsPlaceholder = nullptr;
    
    Q_DISABLE_COPY(GroupTile)
};

} // namespace Tiles
} // namespace LongView
