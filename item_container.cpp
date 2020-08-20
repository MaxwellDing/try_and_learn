#include "item_container.h"

#include "item.h"

ItemContainer::ItemContainer() {
  item_ = std::unique_ptr<OneItem>(new OneItem);
}

ItemContainer::~ItemContainer() {
}
