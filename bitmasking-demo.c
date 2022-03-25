#include <stdio.h>

/* demo of bit masking and shifting */
int main(int argc, char** argv) {

	int	Page;

	/* Suppose that we have a three level page table:
	 * Level 0:  4 bits
	 * Level 1:  8 bits
	 * Level 2:  8 bits
	 *
	 * offset is 32 - (4 + 8 + 8) = 12 bits
	 * page size is 2^12 = 4096 bytes
	 */

	int	Address = 0xFe98f982;

	/* Suppose we are interested in the level 1 page table index:
	 * We would have to construct a bit mask where the following bits
	 * are set:
	 *
	 * 3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
	 * 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
	 * ===============================================================
	 * 0 0 0 0|1 1 1 1|1 1 1 1|0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0
	 * or in hexadecimal:
	 *    0   |   F   |   F   |   0   |   0   |   0   |   0   |   0
	 *
	 * You will have to construct this in an algorithmic fashion, for
	 * demonstration purposes we'll just use a constant.
	 */
	int	Mask = 0x0FF00000;

	/* Note that since we stored Address as an int instead of an
	 * unsigned int, when we print in decimal we think that this is an
	 * negative number if bit 31 is set.  When printing in hexadecimal,
	 * all numbers are assumed unsigned and it won't make any
	 * difference.  (If we had declared this as an unsigned int, it
	 * would have printed as positive integer.)
	 */
	printf("Address:\t0x%08x (%d decimal)\n", Address, Address);

	/* Pull out the relevant bits by bit-wise and */
	Page = Mask & Address;
	printf("After masking:\t0x%08x\n", Page);

	/* Shift right by the relevant number of bits */
	Page = Page >> 20;

	printf("Page number = 0x%x\n", Page);
}
