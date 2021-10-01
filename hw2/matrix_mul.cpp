/*
Student No.: 0710734
Student Name: 邱俊耀
Email: david20571015.eed07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>

class Matrix {
 public:
  explicit Matrix(const int order, uint32_t *value = nullptr);
  ~Matrix();
  uint32_t ComputeChecksum() const;
  Matrix Multiply(const Matrix &mat, const int num_processes = 1) const;
  void RowMultiply(const Matrix &mat, uint32_t *const traget, const int idx) const;

 private:
  uint32_t *m_;
  int order_;
};

void MeasureTimeCost(const Matrix &mat, int num_processes);

int main(void) {
  printf("Input the matrix dimension: ");
  int dim;
  scanf("%d", &dim);
  printf("\n");

  Matrix a(dim);

  for (int num_processes = 1; num_processes <= 16; ++num_processes) {
    MeasureTimeCost(a, num_processes);
  }
}

inline Matrix::Matrix(const int order, uint32_t *value) : order_(order) {
  this->m_ = reinterpret_cast<uint32_t *>(malloc(sizeof(uint32_t) * order * order));

  if (value == nullptr) {
    for (uint32_t i = 0; i < order * order; ++i) this->m_[i] = i;
  } else {
    memcpy(this->m_, value, sizeof(uint32_t) * order * order);
  }
}

inline Matrix::~Matrix() { free(this->m_); }

inline uint32_t Matrix::ComputeChecksum() const {
  uint32_t checksum = 0;
  for (int i = 0; i < order_ * order_; ++i) checksum += this->m_[i];
  return checksum;
}

Matrix Matrix::Multiply(const Matrix &mat, const int num_processes) const {
  int shmid_result = shmget(IPC_PRIVATE, sizeof(uint32_t) * this->order_ * this->order_, IPC_CREAT | 0600);
  uint32_t *result = reinterpret_cast<uint32_t *>(shmat(shmid_result, NULL, 0));

  for (int idx = 0; idx < num_processes; ++idx) {
    switch (fork()) {
      case -1: {
        fprintf(stderr, "Fork failed");
        break;
      }
      case 0: { /* child process */
        /* Group rows by congruent modulo indices */
        for (int i = idx; i < this->order_; i += num_processes) RowMultiply(mat, result, i);
        shmdt(result);
        _exit(0);
        break;
      }
      default: { /* parent process */
        break;
      }
    }
  }

  int remain_processes = num_processes;
  while (remain_processes) {
    pid_t pid = wait(NULL);
    remain_processes -= (pid > 0 ? 1 : 0);
  }

  Matrix c(this->order_, result);
  shmctl(shmid_result, IPC_RMID, NULL);
  return c;
}

inline void Matrix::RowMultiply(const Matrix &mat, uint32_t *const target, const int idx) const {
  int order = this->order_;
  for (size_t i = 0; i < order; ++i)
    for (size_t j = 0; j < order; ++j) /* target[idx][i] += this->m_[idx][j] * mat.m_[j][i] */
      target[idx * order + i] += this->m_[idx * order + j] * mat.m_[j * order + i];
}

void MeasureTimeCost(const Matrix &mat, int num_processes) {
  printf("Multiplying matrices using %d processes\n", num_processes);
  struct timeval start, end;
  gettimeofday(&start, 0);
  Matrix result = mat.Multiply(mat, num_processes);
  gettimeofday(&end, 0);
  time_t sec = end.tv_sec - start.tv_sec;
  time_t usec = end.tv_usec - start.tv_usec;
  printf("Elapsed %f ms, Checksum %u\n", sec * 1000 + (usec / 1000.0), result.ComputeChecksum());
}
