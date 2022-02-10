typedef struct
{
	const char *dli_fname;
	void *dli_fbase;
	const char *dli_sname;
	void *dli_saddr;
} Dl_info;

extern "C"
{

int dladdr(void *, Dl_info *)
{
	return 0;
}

int dl_iterate_phdr(
        int (*callback) (struct dl_phdr_info *info, size_t size, void *data),
        void *data)
{
    return -1;
}

}
