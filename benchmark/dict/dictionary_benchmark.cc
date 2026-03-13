#include <benchmark/benchmark.h>

#include <cstdio>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "dicts/dictionary.h"
#include "dicts/leveldb.h"
#include "dicts/trie.h"
#include "map.hpp"
#include "misc/process_memory.h"

constexpr const int REPEATATIONS = 10;

// Define RUN_BENCHMARK_WITH_REAL_DATA to use the actual dictionary file
// #define RUN_BENCHMARK_WITH_REAL_DATA

#ifndef RUN_BENCHMARK_WITH_REAL_DATA
static std::string getDataFilePath() {
  std::filesystem::path path(__FILE__);
  path = path.remove_filename().parent_path() / "data/dummy_dict.txt";
  return path.generic_string();
}
constexpr size_t DICT_SIZE = 10;  // Small size for dummy data
#else
static std::string getDataFilePath() {
  return "/Users/hj/Library/Rime/js/data/cedict_fixed.u8";
}
constexpr size_t DICT_SIZE = 119000;  // Actual size
#endif

static const ParseTextFileOptions PARSE_TEXT_FILE_OPTIONS{
    .lines = DICT_SIZE,
    .onDuplicatedKey = OnDuplicatedKey::Concat,
    .concatSeparator = "|",
};

static std::string getDataFolderPath() {
  std::filesystem::path path(__FILE__);
  path = path.remove_filename().parent_path() / "data";
  if (!std::filesystem::exists(path)) {
    std::filesystem::create_directories(path);
  }
  return path.generic_string();
}

// FIXME: the data seems incorrect
static double getRssUsage() {
  size_t vmUsage = 0;
  size_t residentSet = 0;  // memory usage in bytes
  getMemoryUsage(vmUsage, residentSet);
  return static_cast<double>(residentSet) / 1024;  // Convert to kilobytes
}
// --- Benchmarks ---

// Benchmark loading text file into Trie
static void bmLoadTextToTrie(benchmark::State& state) {
  for (auto _ : state) {
    rime::Trie trie;
    trie.loadTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
    benchmark::DoNotOptimize(trie);
  }
}
BENCHMARK(bmLoadTextToTrie)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly();

// Benchmark Trie serialization
static void bmTrieSerialization(benchmark::State& state) {
  rime::Trie trie;
  trie.loadTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);  // Load once before benchmark loop
  auto trieFile = getDataFolderPath() + "/dictionary.trie";

  for (auto _ : state) {
    trie.saveToBinaryFile(trieFile);
  }
  // Cleanup outside the loop
  std::remove(trieFile.c_str());
}
BENCHMARK(bmTrieSerialization)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly();

// Benchmark Trie deserialization
static void bmTrieDeserialization(benchmark::State& state) {
  rime::Trie trie;
  trie.loadTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
  auto trieFile = getDataFolderPath() + "/dictionary.trie";
  trie.saveToBinaryFile(trieFile);  // Create file once before benchmark loop

  for (auto _ : state) {
    rime::Trie trie2;
    trie2.loadBinaryFile(trieFile);
    benchmark::DoNotOptimize(trie2);
  }
  // Cleanup outside the loop
  std::remove(trieFile.c_str());
}
BENCHMARK(bmTrieDeserialization)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly();

// Benchmark findData with Trie
static void bmFindDataTrie(benchmark::State& state) {
  auto trieFile = getDataFolderPath() + "/dictionary.trie";
  {
    rime::Trie trie;
    trie.loadTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
    trie.saveToBinaryFile(trieFile);
  }

  auto memoryStart = getRssUsage();
  rime::Trie trieToTest;
  trieToTest.loadBinaryFile(trieFile);  // Load the trie once before the loop

  for (auto _ : state) {
    auto result1 = trieToTest.find("accord");
    benchmark::DoNotOptimize(result1);
    auto result2 = trieToTest.find("accordion");
    benchmark::DoNotOptimize(result2);
    auto result3 = trieToTest.find("nonexistent-word");
    benchmark::DoNotOptimize(result3);
    auto prefixResults = trieToTest.prefixSearch("accord");
    benchmark::DoNotOptimize(prefixResults);
    state.counters["MemoryUsed(KB)"] = getRssUsage() - memoryStart;
  }
  std::remove(trieFile.c_str());
}
BENCHMARK(bmFindDataTrie)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMicrosecond)
    ->ReportAggregatesOnly();  // Use microseconds for potentially faster search

// Benchmark loading text file into std::unordered_map
static void bmLoadTextToMap(benchmark::State& state) {
  for (auto _ : state) {
    auto map = Dictionary::parseTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
    benchmark::DoNotOptimize(map);
  }
}
BENCHMARK(bmLoadTextToMap)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly();

// Benchmark MmapStringMap serialization
static void bmMmapSerialization(benchmark::State& state) {
  auto map = Dictionary::parseTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
  auto mmapFile = getDataFolderPath() + "/dictionary.mmap";

  for (auto _ : state) {
    MmapStringMap mmap;
    for (const auto& pair : map) {
      mmap.add(pair.first, pair.second);
    }
    mmap.save(mmapFile);
  }
  // Cleanup outside the loop
  std::remove(mmapFile.c_str());
}
BENCHMARK(bmMmapSerialization)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly();

// Benchmark MmapStringMap deserialization
static void bmMmapDeserialization(benchmark::State& state) {
  // Prepare the mmap file once before the benchmark loop
  auto map = Dictionary::parseTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
  MmapStringMap mmapSetup;
  for (const auto& pair : map) {
    mmapSetup.add(pair.first, pair.second);
  }
  auto mmapFile = getDataFolderPath() + "/dictionary.mmap";
  mmapSetup.save(mmapFile);

  for (auto _ : state) {
    MmapStringMap mmap;
    mmap.load(mmapFile);
    benchmark::DoNotOptimize(mmap);
  }
  // Cleanup outside the loop
  std::remove(mmapFile.c_str());
}
BENCHMARK(bmMmapDeserialization)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly();

// Benchmark findData with MmapStringMap
static void bmFindDataMmap(benchmark::State& state) {
  auto mmapFile = getDataFolderPath() + "/dictionary.mmap";
  {
    auto map = Dictionary::parseTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
    MmapStringMap mmapSetup;
    for (const auto& pair : map) {
      mmapSetup.add(pair.first, pair.second);
    }
    mmapSetup.save(mmapFile);
  }

  auto memoryStart = getRssUsage();
  MmapStringMap mmapToTest;
  mmapToTest.load(mmapFile);  // Load the mmap once before the loop
  auto mapToTest = mmapToTest.exportToMap();

  for (auto _ : state) {
    auto result1 = mapToTest.find("accord");
    benchmark::DoNotOptimize(result1);
    auto result2 = mapToTest.find("accordion");
    benchmark::DoNotOptimize(result2);
    auto result3 = mapToTest.find("nonexistent-word");
    benchmark::DoNotOptimize(result3);
    std::vector<std::pair<std::string, std::string>> prefixResults;
    for (const auto& [key, value] : mapToTest) {
      if (key.find("accord") == 0) {
        prefixResults.emplace_back(key, value);
      }
    }
    benchmark::DoNotOptimize(prefixResults);
    state.counters["MemoryUsed(KB)"] = getRssUsage() - memoryStart;
  }

  std::remove(mmapFile.c_str());
}
BENCHMARK(bmFindDataMmap)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMicrosecond)
    ->ReportAggregatesOnly();

// --- LevelDB Benchmarks ---

// Helper function to remove LevelDB directory
static void removeLevelDbDir(const std::string& dirPath) {
  if (std::filesystem::exists(dirPath)) {
    std::filesystem::remove_all(dirPath);
  }
}

// Benchmark loading text file into LevelDb
static void bmLoadTextToLevelDb(benchmark::State& state) {
  auto levelDbDir = getDataFolderPath() + "/dictionary.leveldb";
  for (auto _ : state) {
    state.PauseTiming();
    removeLevelDbDir(levelDbDir);  // Clean up before each run
    state.ResumeTiming();
    LevelDb db;
    db.loadTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
    db.saveToBinaryFile(levelDbDir);
    benchmark::DoNotOptimize(db);
    state.PauseTiming();
    db.close();  // Ensure DB is closed before potential removal
    state.ResumeTiming();
  }
  // Final cleanup
  removeLevelDbDir(levelDbDir);
}
BENCHMARK(bmLoadTextToLevelDb)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly();

// Benchmark LevelDb deserialization (Loading from existing DB files)
static void bmLevelDbDeserialization(benchmark::State& state) {
  auto levelDbDir = getDataFolderPath() + "/dictionary.leveldb";

  // Create the DB once before the benchmark loop
  removeLevelDbDir(levelDbDir);
  LevelDb dbSetup;
  dbSetup.loadTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
  dbSetup.saveToBinaryFile(levelDbDir);
  dbSetup.close();  // Close after setup

  for (auto _ : state) {
    LevelDb db;
    db.loadBinaryFile(levelDbDir);  // LevelDB loads from its directory
    benchmark::DoNotOptimize(db);
    state.PauseTiming();
    db.close();
    state.ResumeTiming();
  }
  // Cleanup outside the loop
  removeLevelDbDir(levelDbDir);
}
BENCHMARK(bmLevelDbDeserialization)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly();

// Benchmark findData with LevelDb
static void bmFindDataLevelDb(benchmark::State& state) {
  auto levelDbDir = getDataFolderPath() + "/dictionary.leveldb";
  {
    removeLevelDbDir(levelDbDir);
    LevelDb dbSetup;
    dbSetup.loadTextFile(getDataFilePath(), PARSE_TEXT_FILE_OPTIONS);
    dbSetup.saveToBinaryFile(levelDbDir);
  }

  auto memoryStart = getRssUsage();

  LevelDb dbToTest;
  dbToTest.loadBinaryFile(levelDbDir);
  for (auto _ : state) {
    auto result1 = dbToTest.find("accord");
    benchmark::DoNotOptimize(result1);
    auto result2 = dbToTest.find("accordion");
    benchmark::DoNotOptimize(result2);
    auto result3 = dbToTest.find("nonexistent-word");
    benchmark::DoNotOptimize(result3);
    auto prefixResults = dbToTest.prefixSearch("accord");
    benchmark::DoNotOptimize(prefixResults);
    state.counters["MemoryUsed(KB)"] = getRssUsage() - memoryStart;
  }

  // Cleanup outside the loop
  dbToTest.close();
  removeLevelDbDir(levelDbDir);
}
BENCHMARK(bmFindDataLevelDb)
    ->Repetitions(REPEATATIONS)
    ->Unit(benchmark::kMicrosecond)
    ->ReportAggregatesOnly();

// --- Main ---

// Helper to create dummy file if not using real data
struct GlobalSetup {
  GlobalSetup() {
#ifndef RUN_BENCHMARK_WITH_REAL_DATA
    auto txtpath = getDataFilePath();
    std::cout << "Creating a dummy text file for benchmark: " << txtpath << '\n';
    std::ofstream testDict(txtpath);
    if (!testDict) {
      std::cerr << "Error: Could not create dummy file: " << txtpath << '\n';
      return;
    }
    testDict << "点头\t[diǎn tóu]to nod\n";
    testDict << "点头之交\t[diǎn tóu zhī jiāo]nodding acquaintance\n";
    testDict << "点头咂嘴\t[diǎn tóu zā zuǐ]to approve by nodding one's head and smacking one's "
                "lips (idiom)\n";
    testDict << "点头哈腰\t[diǎn tóu hā yāo]to nod one's head and bow (idiom); bowing and "
                "scraping/unctuous fawning\n";
    testDict << "点头招呼\t[diǎn tóu zhāo hū]beckon\n";
    testDict << "点题\t[diǎn tí]to bring out the main theme/to make the point/to bring out the "
                "substance concisely\n";
    testDict
        << "点餐\t[diǎn cān](at a restaurant) to order a meal/(of a waiter) to take an order\n";
    testDict << "点鬼火\t[diǎn guǐ huǒ]to stir up trouble in secret/to instigate\n";
    testDict << "点点\t[diǎn diǎn]Diandian (Chinese microblogging and social networking "
                "website)||[diǎn diǎn]point/speck\n";
    testDict << "点点滴滴\t[diǎn diǎn dī dī]bit by bit/dribs and drabs/the little details/every "
                "aspect\n";
    testDict.close();
#endif
  }

  ~GlobalSetup() {
#ifndef RUN_BENCHMARK_WITH_REAL_DATA
    auto txtpath = getDataFilePath();
    std::cout << "Removing dummy text file: " << txtpath << '\n';
    std::remove(txtpath.c_str());
#endif
  }
};

GlobalSetup globalSetup;  // Create/remove dummy file globally

BENCHMARK_MAIN();

/*
// Benchmark of loading the cn2en dictionary (7.5 MB, 116647 lines)
// +==========================================================================================+
// | Option               | Load from Text | Save to Binary | Load from Binary | Search entry |
// +----------------------+----------------+----------------+------------------+--------------+
// | Trie (with mmap)     |  ~460 ms       |   ~55 ms       |  ~21.4 ms        |  ~5.33 us    |
// | unordered_map + mmap |  ~235 ms       |  ~137 ms       |  ~69.8 ms        |  ~3472 us    |
// | LevelDB              |  ~408 ms       |  N/A           | ~0.868 ms        |  ~13.2 us    |
// +==========================================================================================+
*/

// Run on (8 X 2200 MHz CPU s)
// CPU Caches:
//   L1 Data 32 KiB
//   L1 Instruction 32 KiB
//   L2 Unified 256 KiB (x4)
//   L3 Unified 6144 KiB
// Load Average: 6.72, 6.11, 6.07
// -------------------------------------------------------------------------------------
// Benchmark                                           Time             CPU   Iterations
// -------------------------------------------------------------------------------------
// bmLoadTextToTrie/repeats:10_mean                  460 ms          448 ms           10
// bmLoadTextToTrie/repeats:10_median                439 ms          438 ms           10
// bmLoadTextToTrie/repeats:10_stddev               37.3 ms         24.7 ms           10
// bmLoadTextToTrie/repeats:10_cv                   8.12 %          5.50 %            10
// bmTrieSerialization/repeats:10_mean              54.7 ms         46.4 ms           10
// bmTrieSerialization/repeats:10_median            50.1 ms         44.7 ms           10
// bmTrieSerialization/repeats:10_stddev            10.1 ms         3.48 ms           10
// bmTrieSerialization/repeats:10_cv               18.49 %          7.50 %            10
// bmTrieDeserialization/repeats:10_mean            21.4 ms         21.3 ms           10
// bmTrieDeserialization/repeats:10_median          21.4 ms         21.3 ms           10
// bmTrieDeserialization/repeats:10_stddev         0.268 ms        0.281 ms           10
// bmTrieDeserialization/repeats:10_cv              1.25 %          1.32 %            10
// bmFindDataTrie/repeats:10_mean                   5.33 us         5.29 us           10 MemoryUsed(KB)=1.0024k
// bmFindDataTrie/repeats:10_median                 5.29 us         5.28 us           10 MemoryUsed(KB)=306
// bmFindDataTrie/repeats:10_stddev                0.126 us        0.067 us           10 MemoryUsed(KB)=1.51239k
// bmFindDataTrie/repeats:10_cv                     2.36 %          1.26 %            10 MemoryUsed(KB)=150.88%
// bmLoadTextToMap/repeats:10_mean                   235 ms          233 ms           10
// bmLoadTextToMap/repeats:10_median                 231 ms          229 ms           10
// bmLoadTextToMap/repeats:10_stddev                11.7 ms         9.03 ms           10
// bmLoadTextToMap/repeats:10_cv                    4.97 %          3.88 %            10
// bmMmapSerialization/repeats:10_mean               137 ms         96.5 ms           10
// bmMmapSerialization/repeats:10_median             134 ms         96.2 ms           10
// bmMmapSerialization/repeats:10_stddev            10.4 ms         1.93 ms           10
// bmMmapSerialization/repeats:10_cv                7.64 %          2.00 %            10
// bmMmapDeserialization/repeats:10_mean            69.8 ms         69.5 ms           10
// bmMmapDeserialization/repeats:10_median          69.7 ms         69.4 ms           10
// bmMmapDeserialization/repeats:10_stddev          2.09 ms         1.90 ms           10
// bmMmapDeserialization/repeats:10_cv              2.99 %          2.74 %            10
// bmFindDataMmap/repeats:10_mean                   3472 us         3444 us           10 MemoryUsed(KB)=14.6732k
// bmFindDataMmap/repeats:10_median                 3499 us         3481 us           10 MemoryUsed(KB)=14.626k
// bmFindDataMmap/repeats:10_stddev                  156 us          133 us           10 MemoryUsed(KB)=343.442
// bmFindDataMmap/repeats:10_cv                     4.50 %          3.87 %            10 MemoryUsed(KB)=2.34%
// bmLoadTextToLevelDb/repeats:10_mean               408 ms          341 ms           10
// bmLoadTextToLevelDb/repeats:10_median             402 ms          338 ms           10
// bmLoadTextToLevelDb/repeats:10_stddev            15.3 ms         8.94 ms           10
// bmLoadTextToLevelDb/repeats:10_cv                3.75 %          2.62 %            10
// bmLevelDbDeserialization/repeats:10_mean        0.868 ms        0.795 ms           10
// bmLevelDbDeserialization/repeats:10_median      0.864 ms        0.787 ms           10
// bmLevelDbDeserialization/repeats:10_stddev      0.020 ms        0.025 ms           10
// bmLevelDbDeserialization/repeats:10_cv           2.35 %          3.18 %            10
// bmFindDataLevelDb/repeats:10_mean                13.2 us         12.2 us           10 MemoryUsed(KB)=110.8
// bmFindDataLevelDb/repeats:10_median              13.1 us         12.1 us           10 MemoryUsed(KB)=36
// bmFindDataLevelDb/repeats:10_stddev              1.48 us        0.874 us           10 MemoryUsed(KB)=157.83
// bmFindDataLevelDb/repeats:10_cv                 11.16 %          7.16 %            10 MemoryUsed(KB)=142.45%
