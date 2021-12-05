/*
Student No.: 0710734
Student Name: 邱俊耀
Email: david20571015.eed07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <sys/time.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>

class TraceLoader {
 public:
  TraceLoader(std::string file_name) { stream_.open(file_name, std::ios::in); };
  ~TraceLoader() { stream_.close(); };

  int NextPage() {
    int page;
    stream_ >> page;
    return stream_.eof() ? EOF : page;
  };

 private:
  std::ifstream stream_;
};

class LfuCache {
 public:
  LfuCache(int frame_size) : frame_size_(frame_size){};
  ~LfuCache() = default;

  void Test(TraceLoader &loader) {
    int page;
    while ((page = loader.NextPage()) != EOF) {
      Request(page);
    }
    PrintResults();
  }

  bool Request(const int page) {
    bool is_hit = (lookup_.find(page) != lookup_.end());

    if (is_hit) {
      int ref_count = lookup_[page]->first;
      auto hint = frame_.erase(lookup_[page]);
      lookup_[page] = frame_.emplace_hint(hint, ref_count + 1, page);
    } else {
      // Remove LFU page from list if cache is full.
      if (frame_.size() == static_cast<size_t>(frame_size_)) {
        int victim_page = frame_.cbegin()->second;

        frame_.erase(lookup_[victim_page]);
        lookup_.erase(victim_page);
      }
      // Add new page to list.
      lookup_[page] = frame_.emplace_hint(frame_.cend(), 1, page);
    }

    is_hit ? ++hit_count_ : ++miss_count_;
    return is_hit;
  }

  void PrintResults() const {
    std::cout << frame_size_ << "\t" << hit_count_ << "\t\t" << miss_count_
              << "\t\t" << std::fixed << std::setprecision(10)
              << 1.0 * miss_count_ / (miss_count_ + hit_count_) << "\n";
  }

 private:
  int frame_size_, hit_count_ = 0, miss_count_ = 0;
  std::multimap<int, int> frame_;  // (ref count, page)
  std::unordered_map<int, decltype(frame_.begin())> lookup_;
};

class LruCache {
 public:
  LruCache(int frame_size) : frame_size_(frame_size){};
  ~LruCache() = default;

  void Test(TraceLoader &loader) {
    int page;
    while ((page = loader.NextPage()) != EOF) {
      Request(page);
    }
    PrintResults();
  }

  bool Request(const int page) {
    bool is_hit = (lookup_.find(page) != lookup_.end());

    if (is_hit) {
      // Remove page from list.
      frame_.erase(lookup_.at(page));
    } else if (frame_.size() == static_cast<size_t>(frame_size_)) {
      // Remove LRU page from list if cache is full.
      lookup_.erase(frame_.back());
      frame_.pop_back();
    }

    frame_.push_front(page);
    lookup_[page] = frame_.begin();

    is_hit ? ++hit_count_ : ++miss_count_;
    return is_hit;
  }

  void PrintResults() const {
    std::cout << frame_size_ << "\t" << hit_count_ << "\t\t" << miss_count_
              << "\t\t" << std::fixed << std::setprecision(10)
              << 1.0 * miss_count_ / (miss_count_ + hit_count_) << "\n";
  }

 private:
  int frame_size_, hit_count_ = 0, miss_count_ = 0;
  std::list<int> frame_;
  std::unordered_map<int, decltype(frame_.begin())> lookup_;
};

int main(int argc, char const *argv[]) {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  timeval start, end;
  double elapsed_seconds;

  int test_frame_sizes[] = {64, 128, 256, 512};

  // LFU
  gettimeofday(&start, NULL);

  std::cout << "LFU policy:\n"
            << "Frame\t"
            << "Hit\t\t"
            << "Miss\t\t"
            << "Page fault retio\n";
  for (int frame_size : test_frame_sizes) {
    TraceLoader loader(argv[1]);
    LfuCache lfu_cache(frame_size);
    lfu_cache.Test(loader);
  }

  gettimeofday(&end, NULL);
  elapsed_seconds =
      (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6;
  std::cout << "Total elapsed time " << std::fixed << std::setprecision(4)
            << elapsed_seconds << " sec" << std::endl;

  std::cout << std::endl;

  // LRU
  gettimeofday(&start, NULL);

  std::cout << "LRU policy:\n"
            << "Frame\t"
            << "Hit\t\t"
            << "Miss\t\t"
            << "Page fault retio\n";
  for (int frame_size : test_frame_sizes) {
    TraceLoader loader(argv[1]);
    LruCache lru_cache(frame_size);
    lru_cache.Test(loader);
  }

  gettimeofday(&end, NULL);
  elapsed_seconds =
      (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6;
  std::cout << "Total elapsed time " << std::fixed << std::setprecision(4)
            << elapsed_seconds << " sec" << std::endl;

  return 0;
}