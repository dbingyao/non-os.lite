
extern unsigned int *_pg_tb;

int enable_d_cache(int argc, char * const argv[]) {
	int i, val;
	unsigned int *ttb_addr;
	ttb_addr = _pg_tb;

	/* Initial the page table for the whole 4G memory space */
	for (i = 0; i < 4096; i++)
		*(ttb_addr + i) = ((i << 20) | (3 << 10) | (0x12));

	/* Copy the page table address to cp15 */
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		     : : "r" (ttb_addr) : "memory");
	/* Set the access control to all-supervisor */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (~0));

	/* Read back and then Enable cache */
	asm volatile("mrc p15, 0, %0, c1, c0, 0"
		     : "=r" (val): : "memory");

	val |= ((1 << 12) | (1 << 2) | (1 << 0));
	asm volatile("mcr p15, 0, %0, c1, c0, 0\n\t"
				"nop \n\t"
				"nop \n\t"
				"nop \n\t"
		     : : "r" (val) : "memory");

	/* Invalid TLB */
	asm volatile("mcr p15, 0, %0, c8, c7, 0"
		     : : "r" (0) : "memory");

	/* Invalid Dcache, Icache */
	asm volatile("mcr p15, 0, %0, c7, c7, 0"
		     : : "r" (0));

	return 0;
}

int disable_d_cache(int argc, char * const argv[]) {

	int val;

	/* Clean & Invalidate D-cache */
	asm volatile("mcr p15, 0, %0, c7, c14, 0"
		     : : "r" (0) : "memory");

	/* Invalidate I-cache */
	asm volatile("mcr p15, 0, %0, c7, c5, 0"
		     : : "r" (0) : "memory");

	/* Drain write buffer(SYNC) */
	asm volatile("mcr p15, 0, %0, c7, c10, 4"
		     : : "r" (0) : "memory");

	/* Invalidate IScratchpad */
	asm volatile("mcr p15, 0, %0, c7, c5, 5"
		     : : "r" (0) : "memory");

	/* Invalidate BTB */
	asm volatile("mcr p15, 0, %0, c7, c5, 6"
		     : : "r" (0) : "memory");

	/* Read back and then Disable cache */
	asm volatile("mrc p15, 0, %0, c1, c0, 0"
		     : "=r" (val): : "memory");

	/* Disable I-cache, D-cache, and MMU */
	val &= ~((1 << 12) | (1 << 2) | (1 << 0));
	asm volatile("mcr p15, 0, %0, c1, c0, 0\n\t"
				"nop \n\t"
				"nop \n\t"
				"nop \n\t"
		     : : "r" (val) : "memory");

	return 0;
}
