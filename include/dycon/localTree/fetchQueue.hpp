#ifndef FETCH_QUEUE
#define FETCH_QUEUE
#include "graph.hpp"
#include <vector>
template <typename T> class fetchQueue {
private:
  std::vector<T> content;
  vertex head;

public:
  fetchQueue<T>() : content(std::vector<T>()), head(0) {}
  ~fetchQueue<T>() {}
  //   void push(T x) { content.emplace_back(x); }
  void push(const T &x) { content.emplace_back(std::move(x)); }
  T &front() { return content[head]; }
  bool empty() { return head == content.size(); }
  void pop() { head++; }
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  iterator begin() { return content.begin(); }
  const_iterator begin() const { return content.begin(); }
  iterator end() { return content.end(); }
  const_iterator end() const { return content.end(); }
};
#endif