#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <iostream>

using std::uint8_t;
using std::vector;

int main()
{
  uint8_t ui8_buffer[10] = "hello\0";
  unsigned char uc_buffer[10] = "hello\0";

  printf("%s %s\n", ui8_buffer, uc_buffer);

  if (ui8_buffer[1] == 'e')
  {
    printf("ui8 as expected\n");
  }
  else
  {
    printf("ui8 not as expected\n");
  }

  if (uc_buffer[1] == 'e')
  {
    printf("uc as expected\n");
  }
  else
  {
    printf("uc not as expected\n");
  }

  // c++11 only - vector initializer list
  vector<uint8_t> ui8_vec = {'h', 'e', 'l', 'l', 'o', '\0'};
  vector<unsigned char> uc_vec = {'h', 'e', 'l', 'l', 'o', '\0'};

  // no workie
  // vector<uint8_t> ui8_vec_string = "hello\0";
  // vector<unsigned char> uc_vec_string = "hello\0";

  printf("sizeof buffer[10] = %lu %lu\n", sizeof(uc_buffer), sizeof(ui8_buffer)); 
  vector<unsigned char> uc_vec_str(uc_buffer, uc_buffer + sizeof(uc_buffer));
  vector<uint8_t> ui8_vec_str(ui8_buffer, ui8_buffer + sizeof(ui8_buffer));

  // okay - vector contiguous memory
  printf("%s %s\n", uc_vec_str.data(), ui8_vec_str.data());
  // seg fault - treating the value of [0] as pointer (i think?)
  // printf("%s %s\n", uc_vec_str[0], ui8_vec_str[0]);
  // okay - use subscript operator to pass reference to first location
  printf("%s %s\n", &uc_vec_str[0], &ui8_vec_str[0]);

  // hello, hello
  for(auto c : uc_vec_str)
  {
    std::cout << c;
  }

  std::cout << std::endl;

  for (auto c : ui8_vec_str)
  {
    std::cout << c;
  }

  std::cout << std::endl;

  // hello, hello
  std::cout << uc_vec_str.data() << std::endl;
  std::cout << ui8_vec_str.data() << std::endl;

  // hello, hello
  std::cout << &uc_vec_str[0] << std::endl;
  std::cout << &ui8_vec_str[0] << std::endl;

  // 5, 5
  std::cout << strlen((char *)uc_vec_str.data()) << std::endl;
  std::cout << strlen((char *)ui8_vec_str.data()) << std::endl;

  // works? coincidence?
  vector<uint8_t> test = {'n', 'o', ' ', 'n', 'u', 'l', 'l'};
  std::cout << test.data() << std::endl;

  vector<uint8_t> t(1);

  // Still reports size 1, not crashing -- def coincidence
  t[0] = 't';
  t[1] = 'e';
  t[2] = 's';

  std::cout << t.data() << " " << t.size() << std::endl;

  // no sefault, 1 more zero produces a segfault
  for (auto i = 3; i < 1000000; i++)
  {
    t[i] = 't';
  }

  // capcity still 1
  std::cout << t.data() << " " << t.capacity() << std::endl;
  
  return 0;
}
