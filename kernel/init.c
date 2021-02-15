#include <stdio.h>
#include <string.h>
#include <mmu.h>
#include <io.h>

static int data = 0;

IMPORT_SYM(unsigned long, _image_start, image_start);
IMPORT_SYM(unsigned long, _image_end, image_end);

void early_init(void)
{
	printf("img start %lx end %lx\n", image_start, image_end);
	printf("%lx %lx\n", early_init, printf);
	add_map("all",  image_start, image_start, image_end - image_start,
		MT_NS | MT_NORMAL | MT_RW);
	add_map("uart", 0x09000000, 0x09000000, 0x2000, MT_NS | MT_NORMAL |MT_RW);
	printf("after map\n");
	enable_mmu();
	printf("after enable\n");
}

int main()
{
	early_init();
	data = data + 1023;
	printf("end of main\n");
	return 0;
}
