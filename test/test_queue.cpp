#include "queue.h"
#include <gtest.h>

#define SIZE 1023
class TQueueTest : public ::testing::Test 
{
protected:
  TPriorityQueue* queue;
  TQueueTest() :  queue(0) {}

  void CreateQueue(int maxSize = SIZE) 
  {
    queue = new TPriorityQueue(maxSize);  
  }

  void SetUpFullQueue()
  {
    queue->Push(2, "b");
    queue->Push(3, "c");
    queue->Push(5, "e");
  }

  ~TQueueTest()
  {
    delete queue;
  }
};

TEST_F(TQueueTest, throws_when_create_queue_with_size_not_divisible_by_power_of_two)
{
  ASSERT_ANY_THROW(TPriorityQueue q(10));
}

TEST_F(TQueueTest, can_create_queue_with_DefaultQueueSize)
{
  ASSERT_NO_THROW(TPriorityQueue q(DefaultQueueSize));
}

TEST_F(TQueueTest, can_create_queue_with_MaxQueueSize)
{    
  ASSERT_NO_THROW(TPriorityQueue q(MaxQueueSize));
}

TEST_F(TQueueTest, can_create_queue_with_correct_size)
{
  ASSERT_NO_THROW(TPriorityQueue q(SIZE));
}

TEST_F(TQueueTest, can_allocate_memory_for_queue)
{
  ASSERT_NO_THROW(TPriorityQueue q(SIZE));
}

TEST_F(TQueueTest, throws_when_memory_for_queue_not_allocated)
{
  ASSERT_ANY_THROW(TPriorityQueue q(MaxQueueSize * 2 - 1));
}

TEST_F(TQueueTest, can_create_an_empty_queue)
{
  CreateQueue();

  ASSERT_TRUE(queue->IsEmpty());
}

TEST_F(TQueueTest, can_push_element)
{
  CreateQueue();

  queue->Push(1, "a");

  ASSERT_EQ(1, queue->GetSize());
}

TEST_F(TQueueTest,  can_detect_when_queue_is_full)
{
  CreateQueue(3);
  SetUpFullQueue();

  ASSERT_TRUE(queue->IsFull());
}

TEST_F(TQueueTest, can_detect_when_queue_is_not_full)
{
  CreateQueue(3);
  queue->Push(1, "a");
  queue->Push(2, "b");

  ASSERT_FALSE(queue->IsFull());
}

TEST_F(TQueueTest, can_pop_element)
{
  double key;
  void* value;
  char* resValue = "b";
  CreateQueue(3);
  queue->Push(1, "a");
  queue->Push(2, resValue);

  queue->Pop(&key, &value);

  ASSERT_EQ(2, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TQueueTest, throws_when_pop_from_empty_queue)
{
  double key;
  void* value;
  CreateQueue(3);

  ASSERT_ANY_THROW(queue->Pop(&key, &value));
}

TEST_F(TQueueTest, not_doing_push_to_full_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value;
  CreateQueue(3);
  SetUpFullQueue();

  queue->Push(1, "a");
  for(int i = 0; i < 3; i++)
    queue->Pop(&key, &value);

  ASSERT_NE(1, key);
  //ASSERT_EQ(2, key);
}

TEST_F(TQueueTest, can_push_to_full_queue_when_element_with_largest_key)
{
  double key;
  void* value;
  CreateQueue(3);
  SetUpFullQueue();

  queue->Push(6, "f");

  queue->Pop(&key, &value);
  ASSERT_EQ(6, key);
}

TEST_F(TQueueTest, can_push_to_full_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  CreateQueue(3);
  SetUpFullQueue();

  queue->Push(4, "d");
  for(int i = 0; i < 2; i++)
    queue->Pop(&key, &value);

  ASSERT_EQ(4, key);
}

TEST_F(TQueueTest, can_delete_element_with_min_key_when_push_to_full_queue_element_with_greater_key)
{
  double key;
  void* value;
  CreateQueue(3);
  SetUpFullQueue();

  queue->Push(4, "d");
  for(int i = 0; i < 3; i++)
    queue->Pop(&key, &value);

  ASSERT_NE(2, key);
}

TEST_F(TQueueTest, can_push_to_empty_queue)
{
  double key;
  void* value;
  CreateQueue(3);

  queue->PushWithPriority(1, "a");

  ASSERT_FALSE(queue->IsEmpty());
}

TEST_F(TQueueTest, can_push_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  CreateQueue(3);
  queue->PushWithPriority(1, "a");
  queue->PushWithPriority(1, "b");

  queue->PushWithPriority(2, "c");

  queue->Pop(&key, &value);
  ASSERT_EQ(2, key);
}

TEST_F(TQueueTest, can_push_when_element_is_equal_to_min_key)
{
  double key;
  void* value;
  char* resValue = "c";
  CreateQueue(3);
  queue->PushWithPriority(1, "a");
  queue->PushWithPriority(2, "b");

  queue->PushWithPriority(1, resValue);

  queue->Pop(&key, &value);
  queue->Pop(&key, &value);
  ASSERT_EQ(1, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TQueueTest, can_push_with_priority_to_full_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  char* resValue = "d";
  CreateQueue(3);
  SetUpFullQueue();

  queue->PushWithPriority(4, resValue);

  queue->Pop(&key, &value);
  queue->Pop(&key, &value);
  ASSERT_EQ(4, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TQueueTest, can_clear_queue)
{  
  CreateQueue(3);
  SetUpFullQueue();

  queue->Clear();

  ASSERT_TRUE(queue->IsEmpty());
}
