
extern int fa626_repo_cntr();
extern void fa626_set_cntr();
extern int fa626_check_dcache_en();
extern void fa626_invalid_dcache_all();
extern void fa626_invalid_icache_all();
extern void fa626_clean_dcache_all();
extern void fa626_clean_invalid_dcache_all();
extern void fa626_invalid_idcache_all();
extern void fa626_invalid_tlb_all();
extern void fa626_drain_wr_buffer();
extern void fa626_invalid_iscratchpad();
extern void fa626_invalid_btb();

extern int enable_d_cache(int argc, char * const argv[]);
extern int disable_d_cache(int argc, char * const argv[]);
