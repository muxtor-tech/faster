#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __USE_XOPEN_EXTENDED (1)
#include <unistd.h>

#include "aster/faster_aq.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define QUEUE_SIZE 4096
#define ITEMS_PER_PRODUCER (QUEUE_SIZE * 2)
#define NUM_PRODUCERS 100
#define NUM_CONSUMERS 100

// Define a queue for integers with a size of QUEUE_SIZE and alignment of 4
// bytes
DECLARE_FAST_ATOMIC_FIFO_QUEUE(int, IntQueue, QUEUE_SIZE, 4)

IntQueue queue;

void *producer(void *arg) {
  int id = *(int *)arg;
  for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
    int item = id * ITEMS_PER_PRODUCER + i;
    while (IntQueue_enqueue(&queue, &item) != 0) {
      // Busy-wait if the queue is full
    }
    // printf("Producer %d enqueued %d\n", id, item);
    // printf(".");
  }
  return NULL;
}

void *consumer(void *arg) {
  (void)arg;
  for (int i = 0; i < (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS; ++i) {
    int item;
    while (IntQueue_dequeue(&queue, &item) != 0) {
      // Busy-wait if the queue is empty
    }
    // printf("Consumer dequeued %d\n", item);
    // printf("\b");
  }
  return NULL;
}

void producer2(IntQueue *queue, int id) {
  for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
    int item = id * ITEMS_PER_PRODUCER + i;
    while (IntQueue_enqueue(queue, &item) == -1) {
      ; // Busy-wait until there is space in the queue
    }
    // printf("Producer %d enqueued %d\n", id, item);
  }
}

void consumer2(IntQueue *queue) {
  for (int i = 0; i < (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS; ++i) {
    int item;
    while (IntQueue_dequeue(queue, &item) == -1) {
      ; // Busy-wait until there is an item in the queue
    }
    // printf("Consumer dequeued %d\n", item);
  }
}

int main() {
  // Initialize the queue with a dynamic buffer
  IntQueue_init_dynamic(&queue);

  pthread_t producers[NUM_PRODUCERS];
  pthread_t consumers[NUM_CONSUMERS];
  int producer_ids[NUM_PRODUCERS];

  printf("Starting with processing of %ld items with %ld concurrent "
         "users.\nStage 1: dynamic, pthreads\n: "
         ".........................................................",
         (long int)ITEMS_PER_PRODUCER * NUM_PRODUCERS, (long int)NUM_CONSUMERS + NUM_PRODUCERS);
  fflush(stdout);

  // Create producer threads
  for (int i = 0; i < NUM_PRODUCERS; ++i) {
    producer_ids[i] = i;
    pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
  }

  // Create consumer threads
  for (int i = 0; i < NUM_CONSUMERS; ++i) {
    pthread_create(&consumers[i], NULL, consumer, NULL);
  }

  // Join producer threads
  for (int i = 0; i < NUM_PRODUCERS; ++i) {
    pthread_join(producers[i], NULL);
  }

  // Join consumer threads
  for (int i = 0; i < NUM_CONSUMERS; ++i) {
    pthread_join(consumers[i], NULL);
  }

  printf("\nItems left in queue: %s\n", (IntQueue_is_empty(&queue) ? "NO" : "YES"));
  if (!IntQueue_is_empty(&queue)) {
    exit(EXIT_FAILURE);
  }

  // Free the queue resources
  IntQueue_free(&queue);

  printf("\nStage 2: static in shm, processes\n: "
         ".........................................................");
  fflush(stdout);

  int shm_fd = shm_open("/queue_shm", O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  // Set the size of the shared memory object
  if (ftruncate(shm_fd, (off_t)(sizeof(IntQueue) + sizeof(IntQueue_StaticBuffer))) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  // Map the shared memory object
  void *shm_ptr = mmap(0, sizeof(IntQueue) + sizeof(IntQueue_StaticBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  // initialize the queue with static buffer allocated in the shared memory
  // block
  IntQueue *queue2 = (IntQueue *)shm_ptr;
  IntQueue_StaticBuffer *buffer2 = (IntQueue_StaticBuffer *)(shm_ptr + sizeof(IntQueue));
  IntQueue_init_static(queue2, buffer2);

  pid_t pids[NUM_PRODUCERS + NUM_CONSUMERS];
  for (int i = 0; i < NUM_PRODUCERS; ++i) {
    pids[i] = fork();
    if (pids[i] == 0) {
      producer2(queue2, i);
      exit(EXIT_SUCCESS);
    }
  }

  // Fork consumer processes
  for (int i = 0; i < NUM_CONSUMERS; ++i) {
    pids[NUM_PRODUCERS + i] = fork();
    if (pids[NUM_PRODUCERS + i] == 0) {
      consumer2(queue2);
      exit(EXIT_SUCCESS);
    }
  }

  // Wait for all processes to finish
  for (int i = 0; i < NUM_PRODUCERS + NUM_CONSUMERS; ++i) {
    waitpid(pids[i], NULL, 0);
  }

  printf("\nItems left in queue: %s\n", (IntQueue_is_empty(queue2) ? "NO" : "YES"));

  if (!IntQueue_is_empty(&queue)) {
    exit(EXIT_FAILURE);
  }

  // Unmap the shared memory
  if (munmap(shm_ptr, sizeof(IntQueue) + sizeof(IntQueue_StaticBuffer)) == -1) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }

  // Close and unlink the shared memory object
  if (close(shm_fd) == -1) {
    perror("close");
    exit(EXIT_FAILURE);
  }

  if (shm_unlink("/queue_shm") == -1) {
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }

  return 0;
}