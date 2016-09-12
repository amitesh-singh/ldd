#include <linux/module.h>

MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("module in asm");
MODULE_LICENSE("GPL");

//declared in main.asm
extern int asm_init(void);
extern void asm_exit(void);

static
int main_init(void)
{
   return asm_init();
}

static
void main_exit(void)
{
   asm_exit();
}
module_init(main_init);
module_exit(main_exit);
