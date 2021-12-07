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

#if defined(__GNUG__)
#include <ext/hash_map>
template <class K, class V>
using unordered_map = __gnu_cxx::hash_map<K, V>;
#elif defined(_MSC_VER)
#include <hash_map>
template <class K, class V>
using unordered_map = __gnu_cxx::hash_map<K, V>;
#else
#include <unordered_map>
template <class K, class V>
using unordered_map = std::unordered_map<K, V>;
#endif

template <class Value>
struct Node {
  Value value;
  Node *prev, *next;

  Node(Value value) : value(value), prev(nullptr), next(nullptr) {}

  void Insert(Node *node) {
    this->prev = node->prev;
    this->next = node;
    this->prev->next = this;
    this->next->prev = this;
  }

  void Remove() {
    this->prev->next = this->next;
    this->next->prev = this->prev;
  }
};

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
  LfuCache(int frame_size) : frame_size_(frame_size) {
    head_.next = &tail_;
    tail_.prev = &head_;
  }

  ~LfuCache() {
    while (head_.next != &tail_) {
      auto ptr = head_.next;
      ptr->Remove();
      delete ptr;
    }
  }

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
      auto target_page = lookup_[page];
      target_page->value.first++;

      // Move target_page to the head of the sub-list whose ref count is
      // target_page's ref count + 1.
      auto new_place = target_page;
      while (new_place->prev->value.first <= target_page->value.first &&
             new_place->prev != &head_) {
        new_place = new_place->prev;
      }

      if (new_place != target_page) {
        target_page->Remove();
        target_page->Insert(new_place);
      }

    } else {
      // Remove LFU page from list if cache is full.
      if (lookup_.size() == static_cast<size_t>(frame_size_)) {
        auto victim_page = tail_.prev;
        victim_page->Remove();
        lookup_.erase(victim_page->value.second);
        delete victim_page;
      }
      // Add new page to list.
      auto new_page = new Node<PageInfo>(std::make_pair(1, page));

      Node<PageInfo> *new_place = &tail_;
      while (new_place->prev->value.first <= 1 && new_place->prev != &head_) {
        new_place = new_place->prev;
      }
      new_page->Insert(new_place);

      lookup_[page] = new_page;
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
  using PageInfo = std::pair<int, int>;  // (ref count, page)

  int frame_size_, hit_count_ = 0, miss_count_ = 0;
  Node<PageInfo> head_ = Node<PageInfo>(std::make_pair(-1, -1)),
                 tail_ = Node<PageInfo>(std::make_pair(-1, -1));
  unordered_map<int, Node<PageInfo> *> lookup_;
};

class LruCache {
 public:
  LruCache(int frame_size) : frame_size_(frame_size) {
    head_.next = &tail_;
    tail_.prev = &head_;
  }

  ~LruCache() {
    while (head_.next != &tail_) {
      auto ptr = head_.next;
      ptr->Remove();
      delete ptr;
    }
  }

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
      auto target_page = lookup_[page];
      target_page->Remove();
      target_page->Insert(&tail_);
    } else {
      // Remove LRU page from list if cache is full.
      if (lookup_.size() == static_cast<size_t>(frame_size_)) {
        auto victim_page = head_.next;
        victim_page->Remove();
        lookup_.erase(victim_page->value);
        delete victim_page;
      }
      auto new_page = new Node<int>(page);
      new_page->Insert(&tail_);

      lookup_[page] = new_page;
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
  Node<int> head_ = Node<int>(-1), tail_ = Node<int>(-1);
  unordered_map<int, Node<int> *> lookup_;
};

int main(int argc, char const *argv[]) {
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