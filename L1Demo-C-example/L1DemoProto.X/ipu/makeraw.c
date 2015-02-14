#include <stdio.h>
#include <stdlib.h>

#include "testimage.c"

int colors[4];
int nextcolor = 0;

int main() {
	unsigned char *head = gimp_image.pixel_data;
	char packin = 0;
	int pb = 0;
	while(head - gimp_image.pixel_data < sizeof(gimp_image.pixel_data)) {
		int color = head[0] << 16 | head[1] << 8 | head[2];
		int x;
		for(x = 0; x < nextcolor; x++) {
			if(colors[x] == color) {
				break;
			}
		}
		if(x > 3) {
			fprintf(stderr, "Too many colors\n");
			for(x = 0; x < nextcolor; x++) {
				fprintf(stderr, "%d = %x\n", x, colors[x]);
			}
			fprintf(stderr, "next = %x\n", color);
			exit(1);
		}
		if(x == nextcolor) {
			colors[x] = color;
			nextcolor++;
		}
		packin |= (x & 0x3) << pb;
		pb += 2;
		if(pb > 6) {
			putchar(packin);
			packin = 0;
			pb = 0;
		}

		head += gimp_image.bytes_per_pixel;
	}

	int x;
	for(x = 0; x < nextcolor; x++) {
		fprintf(stderr, "%d = %x\n", x, colors[x]);
	}
}
