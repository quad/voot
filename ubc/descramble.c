#define MAXCHUNK (2048*1024)

static unsigned int seed;

void my_srand(unsigned int n)
{
    seed = n & 0xffff;
}

unsigned int my_rand(void)
{
    seed = (seed * 2109 + 9273) & 0x7fff;
    return (seed + 0xc000) & 0xffff;
}

void load(unsigned char *dest, unsigned long size)
{
    static unsigned char *source;

    if (!size)
    {
        source = dest;
        return;
    }

    memcpy(dest, source, size);
    source += size;
}

void handle_chunk(unsigned char *ptr, unsigned long sz)
{
    static int idx[MAXCHUNK/32];
    int i;

    /* Convert chunk size to number of slices */
    sz /= 32;

    /* Initialize index table with unity,
        so that each slice gets loaded exactly once */
    for(i = 0; i < sz; i++)
        idx[i] = i;

    for(i = sz-1; i >= 0; --i)
    {
        /* Select a replacement index */
        int x = (my_rand() * i) >> 16;

        /* Swap */
        int tmp = idx[i];
        idx[i] = idx[x];
        idx[x] = tmp;

        /* Load resulting slice */
        load(ptr+32*idx[i], 32);
    }
}

void descramble(unsigned char *source, unsigned char *dest, unsigned long size)
{
    unsigned long chunksz;

    load(source, 0);

    my_srand(size);

    /* Descramble 2 meg blocks for as long as possible, then
        gradually reduce the window down to 32 bytes (1 slice) */
    for(chunksz = MAXCHUNK; chunksz >= 32; chunksz >>= 1)
    {
        while(size >= chunksz)
        {
	        handle_chunk(dest, chunksz);
	        size -= chunksz;
	        dest += chunksz;
        }
    }

    /* !!! Load final incomplete slice */
    if(size)
        load(dest, size);
}
