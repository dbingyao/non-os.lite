
extern unsigned int *_pg_tb;

int fa626_repo_cntr() {
	int val;

	asm volatile("mrc p15, 0, %0, c1, c0, 0"
		     : "=r" (val): : "memory");

	return val;
}

void fa626_set_cntr(int val) {

	asm volatile("mcr p15, 0, %0, c1, c0, 0\n\t"
				"nop \n\t"
				"nop \n\t"
				"nop \n\t"
		     : : "r" (val) : "memory");

}

int fa626_check_dcache_en() {
	int val;

	asm volatile("mrc p15, 0, %0, c1, c0, 0"
		     : "=r" (val): : "memory");

	if(val & (1 << 4))
		return 1;
	else
		return 0;
}

void fa626_invalid_dcache_all() {

	asm volatile("mcr p15, 0, %0, c7, c6, 0"
		     : : "r" (0) : "memory");
}

void fa626_invalid_icache_all() {

	asm volatile("mcr p15, 0, %0, c7, c5, 0"
		     : : "r" (0) : "memory");
}

void fa626_clean_dcache_all() {

	asm volatile("mcr p15, 0, %0, c7, c10, 0"
		     : : "r" (0) : "memory");
}

void fa626_clean_invalid_dcache_all() {

	asm volatile("mcr p15, 0, %0, c7, c14, 0"
		     : : "r" (0) : "memory");
}

void fa626_invalid_idcache_all() {

	asm volatile("mcr p15, 0, %0, c7, c7, 0"
		     : : "r" (0) : "memory");
}

void fa626_invalid_tlb_all() {

	asm volatile("mcr p15, 0, %0, c8, c7, 0"
		     : : "r" (0) : "memory");

}

void fa626_drain_wr_buffer() {

	asm volatile("mcr p15, 0, %0, c7, c10, 4\t\n"
				"nop \n\t"
				"nop \n\t"
				"nop \n\t"
				"nop \n\t"
				"nop \n\t"
				"nop \n\t"
				"nop \n\t"
		     : : "r" (0) : "memory");
}

void fa626_invalid_iscratchpad() {

	asm volatile("mcr p15, 0, %0, c7, c5, 5"
		     : : "r" (0) : "memory");
}

void fa626_invalid_btb() {

	asm volatile("mcr p15, 0, %0, c7, c5, 6"
		     : : "r" (0) : "memory");
}

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
	val = fa626_repo_cntr();
	val |= ((1 << 12) | (1 << 2) | (1 << 0));

	fa626_set_cntr(val);

	/* Invalid TLB */
	fa626_invalid_tlb_all();

	/* Invalid Dcache, Icache */
	fa626_invalid_idcache_all();

	return 0;
}

int disable_d_cache(int argc, char * const argv[]) {

	int val;

	/* Clean & Invalidate D-cache */
	fa626_clean_invalid_dcache_all();

	/* Invalidate I-cache */
	fa626_invalid_icache_all();

	/* Drain write buffer(SYNC) */
	fa626_drain_wr_buffer();

	/* Invalidate IScratchpad */
	fa626_invalid_iscratchpad();

	/* Invalidate BTB */
	fa626_invalid_btb();

	/* Read back and then Disable cache */
	val = fa626_repo_cntr();
	val &= ~((1 << 12) | (1 << 2) | (1 << 0));

	/* Disable I-cache, D-cache, and MMU */
	fa626_set_cntr(val);

	return 0;
}
