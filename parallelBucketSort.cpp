#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <cstring>
#include <random>
#include <algorithm>

int compare(const void *a, const void *b)
{
  return (*(int *)a - *(int *)b);
}

void sortBucket(int *bucket, int bucketSize)
{
  if (bucketSize > 0)
  {
    qsort(bucket, bucketSize, sizeof(int), compare);
  }
}

std::vector<int> parallelBucketSort(const std::vector<int> &arr, int bucketNum, int threadNum)
{
  if (arr.empty())
    return {};

  int minValue = *std::min_element(arr.begin(), arr.end());
  int maxValue = *std::max_element(arr.begin(), arr.end());
  int range = maxValue - minValue + 1;

  std::vector<int *> buckets(bucketNum);
  std::vector<int> bucketSizes(bucketNum, 0);
  std::vector<int> bucketCapacities(bucketNum, 0);

  int preallocatedSize = arr.size() / bucketNum + 1;
  for (int i = 0; i < bucketNum; ++i)
  {
    buckets[i] = new int[preallocatedSize];
    bucketCapacities[i] = preallocatedSize;
  }

  for (int num : arr)
  {
    int index = (num - minValue) * bucketNum / range;
    if (index >= bucketNum)
      index = bucketNum - 1;
    if (bucketSizes[index] >= bucketCapacities[index])
    {
      bucketCapacities[index] *= 2;
      int *newBucket = new int[bucketCapacities[index]];
      std::memcpy(newBucket, buckets[index], bucketSizes[index] * sizeof(int));
      delete[] buckets[index];
      buckets[index] = newBucket;
    }
    buckets[index][bucketSizes[index]++] = num;
  }

  auto sortBuckets = [&](int start, int end)
  {
    for (int i = start; i < end; ++i)
    {
      sortBucket(buckets[i], bucketSizes[i]);
    }
  };

  std::vector<std::thread> threads;
  int bucketsPerThread = bucketNum / threadNum;
  int remainingBuckets = bucketNum % threadNum;

  int currentStart = 0;
  for (int t = 0; t < threadNum; ++t)
  {
    bool assignExtraBucket = (remainingBuckets > 0);
    if (assignExtraBucket) {
        --remainingBuckets;
    }

  int currentEnd = currentStart + bucketsPerThread + (assignExtraBucket ? 1 : 0);
    threads.emplace_back(sortBuckets, currentStart, currentEnd);
    currentStart = currentEnd;
  }

  for (auto &thread : threads)
  {
    thread.join();
  }

  std::vector<int> sortedArray;
  sortedArray.reserve(arr.size());
  for (int i = 0; i < bucketNum; ++i)
  {
    sortedArray.insert(sortedArray.end(), buckets[i], buckets[i] + bucketSizes[i]);
    delete[] buckets[i];
  }

  return sortedArray;
}

std::vector<int> generateRandomVector(size_t size, int minValue, int maxValue)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(minValue, maxValue);

  std::vector<int> randomVector(size);
  for (size_t i = 0; i < size; ++i) {
    randomVector[i] = dist(gen);
  }

  return randomVector;
}

int main()
{
  size_t size = 200;
  int minValue = 1;
  int maxValue = 1000;

  std::vector<int> data = generateRandomVector(size, minValue, maxValue);

  std::cout << "Original: ";
  for (int num : data)
  {
    std::cout << num << " ";
  }
  std::cout << "\n\n";

  int bucketNum = 4;
  int threadNum = 2;
  std::vector<int> sortedData = parallelBucketSort(data, bucketNum, threadNum);

  std::cout << "Sorted: ";
  for (int num : sortedData)
  {
    std::cout << num << " ";
  }
  std::cout << "\n\n";

  return 0;
}
