#pragma once

#include <memory>

class OneItem;
class ItemContainer {
 public:
  ItemContainer();
  ~ItemContainer();
  std::unique_ptr<OneItem> item_;
};

