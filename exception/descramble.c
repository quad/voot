#include <string.h>

/* UNMODIFIED */

#define MAXCHUNK (2048*1024)

static unsigned int seed;

static void my_srand(unsigned int n)
{
    seed = n & 0xffff;
}

static unsigned int my_rand(void)
{
    seed = (seed * 2109 + 9273) & 0x7fff;
    return (seed + 0xc000) & 0xffff;
}

/* MODIFIED FUNCTIONS */

static unsigned char *in_point;

static void load_set(unsigned char *fh)
{
    in_point = fh;
}

static void load(unsigned char *ptr, unsigned long sz)
{
    memcpy(ptr, in_point, sz);
    in_point += sz;
}

static void load_chunk(unsigned char *ptr, unsigned long sz)
{
    static int idx[MAXCHUNK/32];
    int i;

    /* Convert chunk size to number of slices */
    sz /= 32;

    /* Initialize index table with unity,
        so that each slice gets loaded exactly once */
    for(i = 0; i < sz; i++)
    {
        idx[i] = i;
    }

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

void descramble(unsigned char *fh, unsigned char *ptr, unsigned long filesz)
{
    unsigned long chunksz;

    load_set(fh);

    my_srand(filesz);

    /* Descramble 2 meg blocks for as long as possible, then
        gradually reduce the window down to 32 bytes (1 slice) */
    for(chunksz = MAXCHUNK; chunksz >= 32; chunksz >>= 1)
    {
        while(filesz >= chunksz)
        {
	        load_chunk(ptr, chunksz);
	        filesz -= chunksz;
	        ptr += chunksz;
        }
    }

    /* Load final incomplete slice */
    if(filesz)
    {
        load(ptr, filesz);
    }
}
