#include "item_tile.h"

#include <QLabel>
#include <QVBoxLayout>

namespace LongView {
namespace Tiles {

ItemTile::ItemTile(const LongView::Config::Item& item, QWidget* parent)
    : Tile(Tile::Kind::Item, parent)
    , m_item(item)
{
    // Title: use item.name if present, else a generic label
    const QString title = m_item.name.has_value()
        ? QString::fromStdString(m_item.name.value())
        : tr("Item");
    setTitle(title);

    // Helpful tooltip for debugging and inspection
    const auto val = QString::fromStdString(m_item.value);
    setToolTip(tr("Name: %1\nType: %2\nValue: %3")
               .arg(title)
               .arg(tr("n/a"))
               .arg(val.left(200) + (val.size() > 200 ? "..." : "")));

    buildContent();
    applyOptionalProperties();
}

void ItemTile::refresh()
{
    // MVP: no-op; future: delegate to inner view (e.g., WebView reload)
}

void ItemTile::buildContent()
{
    // MVP placeholder content; real content will come from ViewFactory later
    auto* content = new QWidget(this);
    auto* vbox = new QVBoxLayout(content);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(kSpacing);

    auto* info = new QLabel(content);
    info->setTextFormat(Qt::PlainText);
    const auto val = QString::fromStdString(m_item.value);
    info->setText(tr("Item placeholder\nValue: %1").arg(val.left(160)));
    info->setTextInteractionFlags(Qt::TextSelectableByMouse);
    info->setWordWrap(true);

    vbox->addWidget(info);
    vbox->addStretch();

    setContentWidget(content);
}

void ItemTile::applyOptionalProperties()
{
    // TODO: when ViewFactory is integrated, remove this minimum size here
    // and delegate size handling to the specific XxxView via applySize().
    // Apply optional size to the content widget (not the Tile itself)
    if (m_item.size.has_value()) {
        const auto s = m_item.size.value();
        if (auto* cw = contentWidget()) {
            cw->setMinimumSize(s.width, s.height);
        }
    }
    // refresh_frequency will be handled by concrete views in future steps
}

} // namespace Tiles
} // namespace LongView
