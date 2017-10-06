#include <mm.h>

int main(void) {

  mm_init();
  for (int i = 0; i < 10; i++) {
    void *tmp = mm_malloc(1 << i);
    mm_free(tmp);
  }
  return 0;
}
