/*
Student No.: 0710734
Student Name: 邱俊耀
Email: david20571015.eed07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>

struct Job {
  void (*sort_func)(std::vector<int>*, size_t, size_t);
  std::vector<int>* v;
  size_t start;
  size_t end;
  size_t id;
};

std::queue<Job> job_list;
sem_t arrival_job_event, completion_job_event;
sem_t job_list_mutex, job_table_mutex;

// 0: not dispatched yet, 1: dispatched, 2: completed.
unsigned job_table[16] = {0};

std::vector<int> ReadData(const std::string filename);
void WriteData(const std::string filename, const std::vector<int>& v);

void BubbleSort(std::vector<int>* v, size_t start, size_t end);
void Merge(std::vector<int>* v, size_t start, size_t end);
void ParallelSort(std::vector<int>& v, int n_threads);

void* dispatch(void*);
void* work(void*);

int main() {
  std::vector<int> v = ReadData("input.txt");

  for (int n_threads = 1; n_threads <= 8; ++n_threads) {
    std::vector<int> vec(v);
    struct timeval start, end;
    gettimeofday(&start, 0);
    ParallelSort(vec, n_threads);
    gettimeofday(&end, 0);
    time_t sec = end.tv_sec - start.tv_sec;
    time_t usec = end.tv_usec - start.tv_usec;
    std::cout << "worker thread #" << n_threads << ", elapsed "
              << sec * 1e3 + usec * 1e-3 << " ms" << std::endl;
    WriteData("output_" + std::to_string(n_threads) + ".txt", vec);
  }

  return 0;
}

void ParallelSort(std::vector<int>& v, int n_threads) {
  sem_init(&arrival_job_event, 0, 0);
  sem_init(&completion_job_event, 0, 0);
  sem_init(&job_list_mutex, 0, 1);
  sem_init(&job_table_mutex, 0, 1);

  pthread_t* worker = new pthread_t[n_threads];
  for (size_t i = 0; i < n_threads; ++i) {
    pthread_create(&worker[i], NULL, work, NULL);
  }

  pthread_t dispatcher;
  pthread_create(&dispatcher, NULL, dispatch, &v);

  // dispatcher will exit if sort completed.
  pthread_join(dispatcher, NULL);

  for (size_t i = 0; i < n_threads; ++i) {
    pthread_cancel(worker[i]);
  }
  for (size_t i = 0; i < n_threads; ++i) {
    pthread_join(worker[i], NULL);
  }
  delete[] worker;

  sem_destroy(&arrival_job_event);
  sem_destroy(&completion_job_event);
  sem_destroy(&job_list_mutex);
  sem_destroy(&job_table_mutex);

  while (!job_list.empty()) job_list.pop();
  for (size_t i = 0; i < 16; ++i) job_table[i] = 0;
}

void* dispatch(void* args) {
  std::vector<int>* v = (std::vector<int>*)args;

  for (size_t i = 8; i <= 15; ++i) {
    sem_wait(&job_list_mutex);
    job_list.push(Job{BubbleSort, v, (i - 8) * v->size() / 8,
                      (i - 7) * v->size() / 8, i});
    sem_post(&job_list_mutex);
    sem_post(&arrival_job_event);
  }

  while (true) {
    sem_wait(&completion_job_event);

    sem_wait(&job_table_mutex);
    for (size_t i = 1; i <= 7; ++i) {
      if (job_table[i] == 0 && job_table[i * 2] == 2 &&
          job_table[i * 2 + 1] == 2) {
        sem_wait(&job_list_mutex);
        switch (i) {
          case 1:
            job_list.push(Job{Merge, v, 0, v->size(), i});
            break;
          case 2:
          case 3:
            job_list.push(Job{Merge, v, (i - 2) * v->size() / 2,
                              (i - 1) * v->size() / 2, i});
            break;
          case 4:
          case 5:
          case 6:
          case 7:
            job_list.push(Job{Merge, v, (i - 4) * v->size() / 4,
                              (i - 3) * v->size() / 4, i});
            break;
        }
        sem_post(&job_list_mutex);
        job_table[i] = 1;
        sem_post(&arrival_job_event);
        break;
      }
    }

    if (job_table[1] == 2) break;

    sem_post(&job_table_mutex);
  }

  pthread_exit(0);
}

void* work(void* args) {
  while (true) {
    sem_wait(&arrival_job_event);

    sem_wait(&job_list_mutex);
    Job job = job_list.front();
    job_list.pop();
    sem_post(&job_list_mutex);

    job.sort_func(job.v, job.start, job.end);

    sem_wait(&job_table_mutex);
    job_table[job.id] = 2;
    sem_post(&job_table_mutex);

    sem_post(&completion_job_event);
  }

  pthread_exit(0);
}

void BubbleSort(std::vector<int>* v, size_t start, size_t end) {
  for (size_t i = 0; i < end - start; ++i)
    for (size_t j = start; j < end - i - 1; ++j)
      if (*(v->begin() + j) > *(v->begin() + j + 1)) {
        std::iter_swap(v->begin() + j, v->begin() + j + 1);
      }
}

void Merge(std::vector<int>* v, size_t start, size_t end) {
  size_t mid = (start + end) / 2;
  std::vector<int> tmp;
  size_t i = start, j = mid;

  while (i < mid && j < end) {
    if (*(v->begin() + i) <= *(v->begin() + j))
      tmp.emplace_back(*(v->begin() + i++));
    else
      tmp.emplace_back(*(v->begin() + j++));
  }

  while (i < mid) tmp.emplace_back(*(v->begin() + i++));
  while (j < end) tmp.emplace_back(*(v->begin() + j++));

  std::move(tmp.begin(), tmp.end(), v->begin() + start);
}

std::vector<int> ReadData(const std::string filename) {
  std::ifstream input_file(filename, std::ios::in);
  size_t len;
  input_file >> len;
  std::vector<int> v;

  for (size_t i = 0; i < len; ++i) {
    int tmp;
    input_file >> tmp;
    v.emplace_back(tmp);
  }
  input_file.close();

  return v;
}

void WriteData(const std::string filename, const std::vector<int>& v) {
  std::ofstream output_file(filename, std::ios::out);
  for (int n : v) {
    output_file << n << ' ';
  }
  output_file.close();
}