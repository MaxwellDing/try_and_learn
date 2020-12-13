#include "thing_container.h"

#include "thing.h"

ThingContainer::ThingContainer() {
  thing_ = std::unique_ptr<Thing>(new Thing);
}

ThingContainer::~ThingContainer() {
}
