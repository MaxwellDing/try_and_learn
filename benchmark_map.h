#include <benchmark/benchmark.h>
#include <iostream>
#include <map>
#include <unordered_map>

static void bench_map_insert(benchmark::State& state) {
  for (auto _ : state) {
    std::map<std::string, int> s;
    int test_t = state.range(0);
    while (test_t--) {
      s.insert({std::to_string(test_t), test_t});
    }
  }
}

BENCHMARK(bench_map_insert)->RangeMultiplier(10)->Range(1, 100000);

static void bench_unordered_map_insert(benchmark::State& state) {
  for (auto _ : state) {
    std::unordered_map<std::string, int> s;
    int test_t = state.range(0);
    while (test_t--) {
      s.insert({std::to_string(test_t), test_t});
    }
  }
}

BENCHMARK(bench_unordered_map_insert)->RangeMultiplier(10)->Range(1, 100000);


static void bench_map_access(benchmark::State& state) {
  std::map<std::string, int> s;
  int test_t = state.range(0);
  while (test_t--) {
    s.insert({std::to_string(test_t), test_t});
  }
    
  for (auto _ : state) {
    test_t = state.range(0);
    while (test_t--) {
      (void)s[std::to_string(test_t)];
    }
  }
}

BENCHMARK(bench_map_access)->RangeMultiplier(10)->Range(1, 100000);

static void bench_unordered_map_access(benchmark::State& state) {
  std::unordered_map<std::string, int> s;
  int test_t = state.range(0);
  while (test_t--) {
    s.insert({std::to_string(test_t), test_t});
  }

  for (auto _ : state) {
    test_t = state.range(0);
    while (test_t--) {
      (void)s[std::to_string(test_t)];
    }
  }
}

BENCHMARK(bench_unordered_map_access)->RangeMultiplier(10)->Range(1, 100000);


static void bench_map_count(benchmark::State& state) {
  std::map<std::string, int> s;
  int test_t = state.range(0);
  while (test_t--) {
    s.insert({std::to_string(test_t), test_t});
  }
    
  for (auto _ : state) {
    test_t = state.range(0);
    while (test_t--) {
      s.count(std::to_string(test_t));
    }
  }
}

BENCHMARK(bench_map_count)->RangeMultiplier(10)->Range(1, 100000);

static void bench_unordered_map_count(benchmark::State& state) {
  std::unordered_map<std::string, int> s;
  int test_t = state.range(0);
  while (test_t--) {
    s.insert({std::to_string(test_t), test_t});
  }

  for (auto _ : state) {
    test_t = state.range(0);
    while (test_t--) {
      s.count(std::to_string(test_t));
    }
  }
}

BENCHMARK(bench_unordered_map_count)->RangeMultiplier(10)->Range(1, 100000);
