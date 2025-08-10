#include "group_tile.h"
#include "../item/item_tile.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QString>
#include <algorithm>
#include <optional>
#include <QScrollArea>
#include <QFrame>

namespace LongView {
namespace Tiles {

namespace {
    constexpr int kItemSpacing = 8;
    constexpr int kGroupMargin = 12;
    constexpr int kPlaceholderPadding = 20;
    
    // Size calculation constants
    constexpr int kHeaderHeight = 20;
    constexpr int kPlaceholderHeight = 40;
    
    // Utility function to safely convert optional string to QString
    static inline QString optName(const std::optional<std::string>& n) {
        return n ? QString::fromStdString(*n) : QString();
    }
}

GroupTile::GroupTile(const Config::Group& group, QWidget* parent)
    : Tile(Kind::Group, parent)
    , m_group(group)
{
    const QString title = m_group.name ? QString::fromStdString(*m_group.name) : tr("Group");
    setTitle(title);
    
    // Set default expanded state silently BEFORE building content
    setExpanded(true, true);
    
    buildContent();
    
    // Populate items from config AFTER building content
    populateFromConfig();
    
    // NEW: Recursively expand all child ItemTiles by default
    // This should be called after populateFromConfig() to ensure all items are created
    expandAllItems();
    
    // Force UI update to ensure content is visible after setting expanded state
    updateUI();
    
    // Connect group completion change to sync with children
    // Use signal blocking to prevent re-entrant calls during synchronization
    connect(this, &Tile::completedChanged, this, [this](bool completed) {
        // Block signals temporarily to prevent re-entrant calls
        QSignalBlocker blocker(this);
        
        for (auto* itemTile : m_itemTiles) {
            // Block child signals to prevent re-entrant calls
            QSignalBlocker childBlocker(itemTile);
            // Only set if state is different to avoid unnecessary signals
            if (itemTile->isCompleted() != completed) {
                itemTile->setCompleted(completed);
            }
        }
    });
}

void GroupTile::buildContent()
{
    // Create scroll area for items container
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Create container widget for items
    auto* container = new QWidget(scrollArea);
    m_itemsLayout = new QVBoxLayout(container);
    m_itemsLayout->setContentsMargins(kGroupMargin, kGroupMargin, kGroupMargin, kGroupMargin);
    m_itemsLayout->setSpacing(kItemSpacing);
    
    // Add header info
    m_headerInfo = new QLabel;
    m_headerInfo->setTextFormat(Qt::PlainText);
    m_headerInfo->setText(tr("Group: %1\nItems: %2")
                       .arg(optName(m_group.name))
                       .arg(m_group.items.size()));
    m_headerInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    auto* headerLayout = new QHBoxLayout;
    headerLayout->setSpacing(Tile::kHeaderSpacing);
    headerLayout->addWidget(m_headerInfo);
    headerLayout->addStretch();
    
    m_itemsLayout->addLayout(headerLayout);
    
    // Add placeholder for items
    m_itemsPlaceholder = new QLabel(tr("No items added yet"));
    m_itemsPlaceholder->setAlignment(Qt::AlignCenter);
    m_itemsPlaceholder->setStyleSheet(QString("color: #888; padding: %1px;").arg(kPlaceholderPadding));
    m_itemsLayout->addWidget(m_itemsPlaceholder);
    
    // Set the container as the scroll area's widget
    scrollArea->setWidget(container);
    
    // Set the scroll area as the content widget
    setContentWidget(scrollArea);
    
    // Critical fix: ensure content container visibility is correctly set
    container->setVisible(true);
    
    // Set size policies for better height adaptation
    // Use MinimumExpanding for height to prevent compression
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    
    // Set tooltip with group information
    const auto tooltip = tr("Group: %1\nItems: %2\nType: %3")
                        .arg(optName(m_group.name))
                        .arg(m_group.items.size())
                        .arg(tr("n/a")); // Future: Config::toString(m_group.type)
    
    setToolTip(tooltip);
    
    // Set minimum size for group tile using Tile's protected constants
    setMinimumSize(Tile::kMinWidth, Tile::kMinHeight);
}



/**
 * @brief Add an ItemTile to this group
 * 
 * Takes ownership of the itemTile and sets it as a child of this GroupTile.
 * The itemTile will be automatically deleted when removed or when this GroupTile is destroyed.
 * 
 * @param itemTile The ItemTile to add (must not be nullptr)
 */
bool GroupTile::addItemTile(ItemTile* itemTile)
{
    if (!itemTile) return false;
    
    // Prevent duplicate addition
    if (std::find(m_itemTiles.begin(), m_itemTiles.end(), itemTile) != m_itemTiles.end()) {
        return false;
    }
    
    // Hide placeholder when adding first item
    if (m_itemTiles.empty()) {
        m_itemsPlaceholder->setVisible(false);
    }
    
    // Take ownership of the item tile
    m_itemTiles.push_back(itemTile);
    m_itemsLayout->addWidget(itemTile); // addWidget automatically sets parent
    
    // Align item tile visibility with group expansion state for better UX
    itemTile->setVisible(isExpanded());
    
    // Connect signals for state synchronization with Qt::UniqueConnection
    setupItemTileConnections(itemTile);
    
    // Update group completion state
    updateGroupCompletionState();
    
    // Update header info
    updateHeaderCount();
    
    return true;
}

/**
 * @brief Remove an ItemTile from this group
 * 
 * The itemTile will be automatically deleted via deleteLater().
 * 
 * @param itemTile The ItemTile to remove
 */
bool GroupTile::removeItemTile(ItemTile* itemTile)
{
    auto it = std::find(m_itemTiles.begin(), m_itemTiles.end(), itemTile);
    if (it == m_itemTiles.end()) return false;
    
    disconnectItemTile(itemTile);
    m_itemsLayout->removeWidget(itemTile);
    itemTile->deleteLater();
    m_itemTiles.erase(it);
    
    // Update group completion state
    updateGroupCompletionState();
    
    // Update header info
    updateHeaderCount();
    
    // Add placeholder back if no items
    if (m_itemTiles.empty()) {
        m_itemsPlaceholder->setVisible(true);
    }
    
    return true;
}

/**
 * @brief Remove all ItemTiles from this group
 * 
 * All ItemTiles will be automatically deleted via deleteLater().
 */
void GroupTile::clearItemTiles()
{
    // Disconnect all item tiles first
    for (auto* itemTile : m_itemTiles) {
        disconnectItemTile(itemTile);
        m_itemsLayout->removeWidget(itemTile);
        itemTile->deleteLater();
    }
    m_itemTiles.clear();
    
    // Update group completion state
    updateGroupCompletionState();
    
    // Update header info
    updateHeaderCount();
    
    // Show placeholder
    m_itemsPlaceholder->setVisible(true);
}

void GroupTile::populateFromConfig()
{
    clearItemTiles();
    m_itemTiles.reserve(m_group.items.size());
    for (const auto& item : m_group.items) {
        auto* tile = new ItemTile(item, this);
        addItemTile(tile);
    }
}

void GroupTile::refresh()
{
    // Refresh all child item tiles
    for (auto* itemTile : m_itemTiles) {
        itemTile->refresh();
    }
    
    // Update group completion state
    updateGroupCompletionState();
}

void GroupTile::setupItemTileConnections(ItemTile* itemTile)
{
    if (!itemTile) return;
    
    connect(itemTile, &ItemTile::expandedChanged, 
            this, &GroupTile::onItemTileExpandedChanged, Qt::UniqueConnection);
    connect(itemTile, &ItemTile::completedChanged, 
            this, &GroupTile::onItemTileCompletedChanged, Qt::UniqueConnection);
}

void GroupTile::disconnectItemTile(ItemTile* itemTile)
{
    if (!itemTile) return;
    // Disconnect all signals from itemTile to this
    QObject::disconnect(itemTile, nullptr, this, nullptr);
}

void GroupTile::onItemTileExpandedChanged(bool expanded)
{
    Q_UNUSED(expanded);
    // Update expand button state based on children
    updateExpandButtonState();
}

void GroupTile::onItemTileCompletedChanged(bool completed)
{
    Q_UNUSED(completed);
    // Update group completion state
    updateGroupCompletionState();
}

void GroupTile::updateGroupCompletionState()
{
    if (m_itemTiles.empty()) {
        // Only set if state is different to avoid unnecessary signals
        if (isCompleted() != false) {
            setCompleted(false);
        }
        return;
    }
    
    // Check if all items are completed
    bool allCompleted = true;
    for (const auto* itemTile : m_itemTiles) {
        if (!itemTile->isCompleted()) {
            allCompleted = false;
            break;
        }
    }
    
    // Only update if state is different to avoid unnecessary signals
    if (isCompleted() != allCompleted) {
        setCompleted(allCompleted);
    }
}

void GroupTile::updateExpandButtonState()
{
    // This could be used to show different expand button states
    // based on children's expanded states
    // For now, just a placeholder for future enhancement
}

void GroupTile::updateHeaderCount()
{
    if (!m_headerInfo) return;
    m_headerInfo->setText(tr("Group: %1\nItems: %2")
                          .arg(optName(m_group.name))
                          .arg(m_itemTiles.size()));
}

QSize GroupTile::sizeHint() const
{
    // With QScrollArea, we can use a fixed height for the group tile
    // The content will scroll internally if needed
    return QSize(Tile::kDefaultWidth, Tile::kDefaultHeight);
}

QSize GroupTile::minimumSizeHint() const
{
    // Use the base tile's minimum size
    return QSize(Tile::kMinWidth, Tile::kMinHeight);
}

void GroupTile::expandAllItems()
{
    // First expand the group itself
    setExpanded(true);
    
    // Then expand all child ItemTiles
    for (auto* itemTile : m_itemTiles) {
        itemTile->setExpanded(true);
    }
}

void GroupTile::collapseAllItems()
{
    // First collapse the group itself
    setExpanded(false);
    
    // Optionally collapse all child ItemTiles too
    // (though they won't be visible when group is collapsed)
    for (auto* itemTile : m_itemTiles) {
        itemTile->setExpanded(false);
    }
}

} // namespace Tiles
} // namespace LongView
