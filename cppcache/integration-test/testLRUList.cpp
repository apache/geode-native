/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>

#include <vector>

#include "fw_helper.hpp"

#ifdef WIN32

BEGIN_TEST(NotOnWindows)
  { LOG("Not everything is exported on windows."); }
END_TEST(NotOnWindows)

#else

//#define BUILD_CPPCACHE

#include <LRUList.cpp>
#include <geode/CacheableKey.hpp>

using apache::geode::client::CacheableKey;
using apache::geode::client::LRUEntryProperties;
using apache::geode::client::LRUList;

class MyNode : public LRUEntryProperties {
 public:
  static MyNode *create(const std::shared_ptr<CacheableKey> & = nullptr) {
    return new MyNode();
  }
  virtual ~MyNode() {}
  LRUEntryProperties &getLRUProperties() { return *this; }
  void setValue(int value) { m_value = value; }
  int getValue() { return m_value; }

 private:
  MyNode() : m_value(0) {}

  int m_value;
};

/**
 * @brief Test the LRU-ness of the LRUList.
 */
BEGIN_TEST(LRUListTest)
  {
    LRUList<MyNode, MyNode> lruList;
    // Create 10 Nodes to keep track of.
    std::vector<std::shared_ptr<MyNode> > tenNodes(10);

    for (int i = 0; i < 10; i++) {
      tenNodes[i] = std::shared_ptr<MyNode>(MyNode::create());
      tenNodes[i]->setValue(i);
      // add each node to list
      lruList.appendEntry(tenNodes[i]);
      auto myPtr = std::dynamic_pointer_cast<MyNode>(tenNodes[i]);
      std::cout << "  appendEntry( " << myPtr->getValue() << " )" << std::endl;
    }

    // now mark odd indexed nodes recently used.
    for (int j = 1; j < 10; j += 2) {
      tenNodes[j]->setRecentlyUsed();
    }

    std::shared_ptr<MyNode> aNode;
    char msgbuf[100];
    // check that we get the unmarked entries first. 0,2,4,6,8
    for (int k = 0; k < 10; k += 2) {
      lruList.getLRUEntry(aNode);
      sprintf(msgbuf, "expected node %d", k);
      std::cout << " gotEntry( " << aNode->getValue() << " )" << std::endl;
      ASSERT(aNode == tenNodes[k], msgbuf);
    }

    // now check that we get the remaining entries. 1,3,5,7,9
    for (int m = 1; m < 10; m += 2) {
      lruList.getLRUEntry(aNode);
      sprintf(msgbuf, "expected node %d", m);
      ASSERT(aNode == tenNodes[m], msgbuf);
    }
  }
END_TEST(LRUListTest)

BEGIN_TEST(TestEndOfList)
  {
    LRUList<MyNode, MyNode> lruList;
    // add ten entries.
    for (int i = 0; i < 10; i++) {
      std::shared_ptr<MyNode> tmp(MyNode::create());
      tmp->setValue(i);
      lruList.appendEntry(tmp);
    }
    // drain the list...
    int k = 0;
    char msgbuf[100];
    while (k < 10) {
      std::shared_ptr<MyNode> nodePtr;
      lruList.getLRUEntry(nodePtr);
      sprintf(msgbuf, "expected node %d", k);
      ASSERT(std::dynamic_pointer_cast<MyNode>(nodePtr)->getValue() == k,
             msgbuf);
      k++;
    }
    // now list should be empty...
    std::shared_ptr<MyNode> emptyPtr;
    lruList.getLRUEntry(emptyPtr);
    ASSERT(emptyPtr == nullptr, "expected nullptr");
    // do it again...
    emptyPtr = nullptr;
    ASSERT(emptyPtr == nullptr, "expected nullptr");
    lruList.getLRUEntry(emptyPtr);
    ASSERT(emptyPtr == nullptr, "expected nullptr");
    // now add something to the list... and retest...
    {
      std::shared_ptr<MyNode> tmp(MyNode::create());
      tmp->setValue(100);
      lruList.appendEntry(tmp);
    }
    std::shared_ptr<MyNode> hundredPtr;
    lruList.getLRUEntry(hundredPtr);
    ASSERT(hundredPtr != nullptr, "expected to not be nullptr");
    ASSERT(std::dynamic_pointer_cast<MyNode>(hundredPtr)->getValue() == 100,
           "expected value of 100");
    lruList.getLRUEntry(emptyPtr);
    ASSERT(emptyPtr == nullptr, "expected nullptr");
  }
END_TEST(TestEndOfList)

/**
 * @brief Test all the states of the LRUListEntry
 */
BEGIN_TEST(LRUListEntryTest)
  {
    std::shared_ptr<MyNode> node(MyNode::create());
    node->setValue(20);
    // test initial state.
    ASSERT(node->testRecentlyUsed() == false, "should not be marked used.");
    ASSERT(node->testEvicted() == false, "should not yet be evicted.");

    // set used, and retest state.
    node->setRecentlyUsed();
    ASSERT(node->testRecentlyUsed() == true, "should be marked used.");
    ASSERT(node->testEvicted() == false, "should not yet be evicted.");

    // clear used, and retest state.
    node->clearRecentlyUsed();
    ASSERT(node->testRecentlyUsed() == false, "should not be marked used.");
    ASSERT(node->testEvicted() == false, "should not yet be evicted.");

    // set evicted, and retest state.
    node->setEvicted();
    ASSERT(node->testRecentlyUsed() == false, "should not be marked used.");
    ASSERT(node->testEvicted() == true, "should be evicted.");

    // clear evicted, and retest state.
    node->clearEvicted();
    ASSERT(node->testRecentlyUsed() == false, "should not be marked used.");
    ASSERT(node->testEvicted() == false, "should not yet be evicted.");

    // set both bits.
    node->setRecentlyUsed();
    node->setEvicted();
    ASSERT(node->testRecentlyUsed() == true, "should be marked used.");
    ASSERT(node->testEvicted() == true, "should be evicted.");

    // clear evicted, and retest state.
    node->clearEvicted();
    ASSERT(node->testRecentlyUsed() == true, "should be marked used.");
    ASSERT(node->testEvicted() == false, "should not yet be evicted.");

    // set both bits.
    node->setEvicted();
    node->clearRecentlyUsed();
    ASSERT(node->testRecentlyUsed() == false, "should not be marked used.");
    ASSERT(node->testEvicted() == true, "should be evicted.");
  }
END_TEST(LRUListEntryTest)

#endif
