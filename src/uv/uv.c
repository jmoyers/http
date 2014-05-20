#include <stdio.h>
#include <uv.h>

int main(){
  printf("Test\n");
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  return 0;
}
