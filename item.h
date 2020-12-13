#pragma once

#include <iostream>
#include <string>

class OneItem {
 public:
  OneItem() {
    std::cout << "construct one item" << std::endl;
  }
  OneItem(int p, int s): priority(p), sequence(s) {
    std::cout << "construct one item" << std::endl;
  }
  OneItem(const OneItem& another) {
    std::cout << "copy construct one item" << std::endl;
    data = std::string(another.data);
    priority = another.priority;
    sequence = another.sequence;
  }
  OneItem& operator=(const OneItem& rhs) {
    std::cout << "copy assignment one item" << std::endl;
    data = std::string(rhs.data);
    priority = rhs.priority;
    sequence = rhs.sequence;
    return *this;
  }
  OneItem(OneItem&& another) {
    std::cout << "move construct one item" << std::endl;
    data = std::move(another.data);
    priority = std::move(another.priority);
    sequence = std::move(another.sequence);
  }
  OneItem& operator=(OneItem&& rhs) {
    std::cout << "move assignment one item" << std::endl;
    data = std::move(rhs.data);
    priority = std::move(rhs.priority);
    sequence = std::move(rhs.sequence);
    return *this;
  }
  ~OneItem() {
    std::cout << "destruct one item" << std::endl;
  };

  friend bool operator<(const OneItem& lhs, const OneItem& rhs) {
    std::cout << "compare two item\n";
    return lhs.priority < rhs.priority;
  }

  friend std::ostream& operator<<(std::ostream& os, const OneItem& item) {
    os << item.data;
    return os;
  }

  std::string data;
  int priority;
  int sequence;
};
