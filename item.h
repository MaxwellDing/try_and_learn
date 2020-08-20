#pragma once

#include <iostream>
#include <string>

class OneItem {
 public:
  OneItem() {
    std::cout << "construct one item" << std::endl;
  }
  OneItem(const OneItem& another) {
    std::cout << "copy construct one item" << std::endl;
    data = std::string(another.data);
  }
  OneItem& operator=(const OneItem& rhs) {
    std::cout << "copy assignment one item" << std::endl;
    data = std::string(rhs.data);
    return *this;
  }
  OneItem(OneItem&& another) {
    std::cout << "move construct one item" << std::endl;
    data = std::move(another.data);
  }
  OneItem& operator=(OneItem&& rhs) {
    std::cout << "move assignment one item" << std::endl;
    data = std::move(rhs.data);
    return *this;
  }
  ~OneItem() {
    std::cout << "destruct one item" << std::endl;
  };

  std::string data;
};
