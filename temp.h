#pragma once

#include <iostream>

template <typename T>
class TypeInfo {
};

void PrintIntTypeAddress();

class aContainer {
 public:
  template <typename T>
  void PrintTypeAddress(T t) {
    static TypeInfo<T> info;
    std::cout << &info << std::endl;
  }
};
