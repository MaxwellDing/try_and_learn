#pragma once

#include <memory>

class Thing;
class ThingContainer {
 public:
  ThingContainer();
  ~ThingContainer();

 private:
  std::unique_ptr<Thing> thing_;
};

