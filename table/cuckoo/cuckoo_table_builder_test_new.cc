// // table/cuckoo/cuckoo_table_builder_test_new.cc

#include <cassert>
#include <iostream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <fstream>
#include <chrono>

#include "rocksdb/env.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/table.h"

#include "table/cuckoo/cuckoo_table_builder.h"
#include "table/cuckoo/cuckoo_table_factory.h"

using namespace ROCKSDB_NAMESPACE;

std::string make_key(int i) {
    std::ostringstream ss;
    ss << std::setw(8) << std::setfill('0') << i;
    return ss.str();
}

void test(int length) {
    std::string fname = "/tmp/test_cuckoo_table";

    Options options;
    options.table_factory.reset(NewCuckooTableFactory());
    options.allow_mmap_reads = true;
    options.create_if_missing = true;

    DestroyDB(fname, options);

    std::unique_ptr<DB> db;
    Status s = DB::Open(options, fname, &db);
    assert(s.ok());

    const int N = 10000;

    std::string val(length, '1');

    // Insert data
    auto build_start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; i++) {
        std::string key = make_key(i);
        s = db->Put(WriteOptions(), key, val);
        assert(s.ok());
    }
    auto build_end = std::chrono::steady_clock::now();

    db->Flush(FlushOptions());
    DB::Open(options, fname, &db);
    assert(s.ok());

    std::string result;
    std::vector <std::string> keys_to_read;
    
    const int READ = 1000000;
    for (int i = 0; i<READ; ++i) {
        keys_to_read.push_back(make_key(rand() % N));
    }

    // Verify values
    auto read_start = std::chrono::steady_clock::now();
    for (int i = 0; i < READ; i++) {
        result.clear();
        s = db->Get(ReadOptions(), keys_to_read[i], &result);
        assert(s.ok());
    }
    auto read_end = std::chrono::steady_clock::now();


    auto build_duration = std::chrono::duration_cast<std::chrono::microseconds>(build_end - build_start);
    auto read_duration = std::chrono::duration_cast<std::chrono::microseconds>(read_end - read_start);

    std::vector<LiveFileMetaData> metadata;
    db->GetLiveFilesMetaData(&metadata);

    uint64_t size = 0;
    for (auto& file : metadata) {
        size += file.size;
    }
    std::ofstream MyFile("variable_big_results.txt", std::ios::app);

    MyFile << length << ' ' << build_duration.count() << ' ' << read_duration.count() << ' ' << size << '\n';
}

int main() {
    srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = 50; i<=5000; i += 50) {

        for (int j = 0; j<5; ++j) {
            test(i);
        }
    }

    return 0;
}

// table/cuckoo/cuckoo_table_builder_test_new.cc
// #include <cassert>
// #include <iostream>
// #include <string>
// #include <vector>

// #include "rocksdb/env.h"
// #include "rocksdb/options.h"
// #include "rocksdb/slice.h"
// #include "rocksdb/table.h"

// #include "table/cuckoo/cuckoo_table_builder.h"
// #include "table/cuckoo/cuckoo_table_factory.h"

// using namespace ROCKSDB_NAMESPACE;

// std::string make_key(int i) {
//     std::ostringstream ss;
//     ss << std::setw(8) << std::setfill('0') << i;
//     return ss.str();
// }


// int main() {
//   std::string fname = "/tmp/test_cuckoo_table";

//   Options options;

//   options.table_factory.reset(NewCuckooTableFactory());
//   options.allow_mmap_reads = true;
//   options.create_if_missing = true;

//   DestroyDB(fname, options);

//   DB* db;
//   Status s = DB::Open(options, fname, &db);
//   assert(s.ok());

//   const int N = 100000;

//   std::vector<std::string> values;

//   // Insert data
//   for (int i = 1; i < N; i++) {
//     std::string key = make_key(i);
//     std::string val = "value_" + std::to_string(i);

//     values.push_back(val);

//     s = db->Put(WriteOptions(), key, val);
//     assert(s.ok());
//   }

//     db->Flush(FlushOptions());
//     delete db;
//     DB::Open(options, fname, &db);
//     assert(s.ok());

//   // Verify values
//   for (int i = 1; i < N; i++) {
//     std::string key = make_key(i);

//     std::string result;

//     s = db->Get(ReadOptions(), key, &result);

//     assert(s.ok());

//     if (result != values[i - 1]) {
//       std::cerr << "Mismatch at " << i << ' ' << result << ' ' << values[i] << std::endl;
//       exit(1);
//     }
//   }

//   std::cout << "All values verified successfully!" << std::endl;

//   delete db;

//   return 0;
// }