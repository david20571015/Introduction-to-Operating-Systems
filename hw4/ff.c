/*
Student No.: 0710734
Student Name: 邱俊耀
Email: david20571015.eed07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MEMORY_POOL_SIZE 20000

void *memory_pool = NULL;

typedef struct block {
  size_t size;
  int free;
  struct block *prev;
  struct block *next;
} block_t;

void init_memory_pool(void) {
  memory_pool = mmap(NULL, MEMORY_POOL_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON, -1, 0);

  block_t *head = (block_t *)memory_pool;
  head->size = MEMORY_POOL_SIZE - sizeof(block_t);
  head->free = 1;
  head->prev = NULL;
  head->next = NULL;
}

void print_result(size_t size) {
  char output[32] = "Max Free Chunk Size = ";

  char num[8];
  sprintf(num, "%zu", size);

  strcat(output, num);
  strcat(output, "\n");

  write(STDOUT_FILENO, output, strlen(output));
}

block_t *find_largest_free_block(void) {
  block_t *head = (block_t *)memory_pool;
  block_t *largest_free_block = &(block_t){0, 1, NULL, NULL};

  while (head != NULL) {
    largest_free_block = (head->free && (head->size > largest_free_block->size))
                             ? head
                             : largest_free_block;
    head = head->next;
  }
  return largest_free_block;
}

block_t *find_fit_block(size_t size) {
  block_t *curr = (block_t *)memory_pool;

  while (curr->free == 0 || curr->size < size) {
    curr = curr->next;
  }

  return curr;
}

void *malloc(size_t size) {
  if (memory_pool == NULL) init_memory_pool();

  void *mem_ptr = NULL;

  if (size == 0) {
    size_t max_space = find_largest_free_block()->size;
    print_result(max_space);

    munmap(memory_pool, MEMORY_POOL_SIZE);
    mem_ptr = NULL;
  } else {
    size = ((size - 1) / 32 + 1) * 32;
    block_t *curr = find_fit_block(size);

    if (curr == NULL) {
      mem_ptr = NULL;
    } else {
      // Create a new free block if there is space left between current block
      // and next block.
      if (curr->size > size) {
        block_t *next_block = curr + (1 + size / sizeof(block_t));
        next_block->free = 1;
        next_block->size = curr->size - sizeof(block_t) - size;
        next_block->prev = curr;
        next_block->next = curr->next;

        curr->next = next_block;
      }

      curr->free = 0;
      curr->size = size;

      mem_ptr = curr + 1;
    }
  }

  return mem_ptr;
}

void merge_block(block_t *front, block_t *back) {
  front->size += back->size + sizeof(block_t);
  front->next = back->next;
  if (back->next != NULL) back->next->prev = front;
}

void free(void *ptr) {
  if (ptr == NULL) return;

  block_t *curr = (block_t *)ptr - 1;
  curr->free = 1;

  // Merge with previous block if it is free.
  if (curr->prev != NULL && curr->prev->free) {
    merge_block(curr->prev, curr);
  }

  // Merge with next block if it is free.
  if (curr->next != NULL && curr->next->free) {
    merge_block(curr, curr->next);
  }
}