#include <iostream>
#include <vector>

struct Foo {
  std::vector<int> data;
};

int main() {
  Foo f1;
  f1.data.push_back(5);
  f1.data.push_back(2);
  f1.data.push_back(7);
  Foo f2;
  f2.data.push_back(9);
  Foo f3;
  f3.data.push_back(9);
  f3.data.push_back(10);
  f3.data.push_back(11);
  f3.data.push_back(12);
  std::vector<Foo> foos;
  foos.push_back(f1);
  foos.push_back(f2);
  foos.push_back(f3);
  auto* itr = &foos[0]; 
  for (int i = 0; i < 3; ++i) {
    std::cout << itr->data.size() << std::endl;
    ++itr;
  }
  return 0;
}
