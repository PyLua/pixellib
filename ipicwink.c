//=====================================================================
//
// ipicwink.c - picture loading extra module
//
// NOTE:
// for more information, please see the readme file
//
//=====================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ibitmap.h"
#include "ipicture.h"
#include "ipicwink.h"


//=====================================================================
// Animate Definition
//=====================================================================

//---------------------------------------------------------------------
// memory hook
//---------------------------------------------------------------------
extern void*(*icmalloc)(size_t size);
extern void (*icfree)(void *ptr);



//---------------------------------------------------------------------
// memory hook
//---------------------------------------------------------------------
int iAnimate_ErrCode = 0;


//---------------------------------------------------------------------
// create a new animate
//---------------------------------------------------------------------
iAnimate *iAnimate_New(int w, int h, int bpp, int count, int hotx, int hoty)
{
	iAnimate *ani;
	int failed = 0, i;
	unsigned length;

	assert(bpp == 8 || bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32);
	assert(count >= 0);

	ani = (iAnimate*)icmalloc(sizeof(iAnimate));
	if (ani == NULL) return NULL;
	memset(ani, 0, sizeof(iAnimate));

	ani->w = w;
	ani->h = h;
	ani->bpp = bpp;
	ani->framecount = count;
	ani->hotx = hotx;
	ani->hoty = hoty;

	for (length = 1; length < count; length <<= 1);

	ani->frames = (IBITMAP**)icmalloc(sizeof(IBITMAP*) * length);

	if (ani->frames == NULL) {
		icfree(ani);
		return NULL;
	}

	for (i = 0; i < length; i++) {
		ani->frames[i] = NULL;
	}

	for (i = 0; i < ani->framecount; i++) {
		ani->frames[i] = ibitmap_create(w, h, bpp);
		if (ani->frames[i] == NULL) failed++;
	}

	if (failed) {
		for (i = 0; i < ani->framecount; i++) {
			if (ani->frames[i]) ibitmap_release(ani->frames[i]);
		}
		icfree(ani->frames);
		icfree(ani);
		return NULL;
	}

	return ani;
}


//---------------------------------------------------------------------
// free a animate
//---------------------------------------------------------------------
void iAnimate_Free(iAnimate *ani)
{
	assert(ani);
	if (ani->frames) {
		int i;
		for (i = 0; i < ani->framecount; i++) {
			if (ani->frames[i]) ibitmap_release(ani->frames[i]);
		}
		icfree(ani->frames);
	}
	if (ani->pal) icfree(ani->pal);
	memset(ani, 0, sizeof(iAnimate));
	icfree(ani);
}


//---------------------------------------------------------------------
// frame resize
//---------------------------------------------------------------------
void iAnimate_Resize(iAnimate *ani, int new_frame_count)
{
	IBITMAP **array;
	int nowsize;
	int newsize;
	int i;

	// recaculate array size
	for (nowsize = 1; nowsize < ani->framecount; nowsize <<= 1);
	for (newsize = 1; newsize < new_frame_count; newsize <<= 1);

	// only grow
	if (newsize > nowsize) {
		array = (IBITMAP**)icmalloc(sizeof(IBITMAP*) * newsize);
		assert(array);
		for (i = 0; i < newsize; i++) {
			array[i] = (i < ani->framecount)? ani->frames[i] : NULL;
		}
		icfree(ani->frames);
		ani->frames = array;
	}

	// create additional bitmaps
	for (i = ani->framecount; i < new_frame_count; i++) {
		if (ani->frames[i] == NULL) {
			ani->frames[i] = ibitmap_create(ani->w, ani->h, ani->bpp);
			assert(ani->frames[i]);
		}
	}

	// release spilth bitmaps
	for (i = new_frame_count; i < ani->framecount; i++) {
		if (ani->frames[i] != NULL) {
			ibitmap_release(ani->frames[i]);
			ani->frames[i] = NULL;
		}
	}

	ani->framecount = new_frame_count;
}


//---------------------------------------------------------------------
// Load Gif Stream
//---------------------------------------------------------------------
iAnimate *iAnimate_Load_Gif_Stream(IMDIO *stream)
{
	struct IGIFDESC gif;
	IBITMAP *list = NULL;
	IBITMAP *bitmap;
	iAnimate *ani;
	IRGB *pal;
	int framecount;
	int w, h, i;

	pal = (IRGB*)icmalloc(sizeof(IRGB) * 256);
	if (pal == NULL) return NULL;

	if (ipic_gif_open(&gif, stream, pal, 0) != 0) {
		icfree(pal);
		return NULL;
	}

	w = gif.width;
	h = gif.height;

	for (framecount = 0; ; framecount++) {
		bitmap = ibitmap_create(w, h, 8);
		if (bitmap == NULL) break;
		if (ipic_gif_read_frame(&gif) != 0) break;
		ibitmap_blit(bitmap, 0, 0, gif.bitmap, 0, 0, w, h, 0, 0);
		bitmap->mask = gif.transparent;
		bitmap->mode = gif.delay;
		bitmap->code = (unsigned long)list;
		list = bitmap;
	}

	ani = iAnimate_New(gif.width, gif.height, 8, framecount, 0, 0);

	if (ani) {
		ani->pal = pal;
		ani->hotx = gif.hotx;
		ani->hoty = gif.hoty;
		bitmap = list;
		for (i = framecount - 1; i >= 0 && bitmap; i--) {
			ibitmap_blit(ani->frames[i], 0, 0, bitmap, 0, 0, w, h, 0, 0);
			ani->frames[i]->mask = bitmap->mask;
			ani->frames[i]->mode = bitmap->mode;
			bitmap = (IBITMAP*)bitmap->code;
		}
	}	else {
		icfree(pal);
	}

	ipic_gif_close(&gif);
	
	for (bitmap = list; bitmap != NULL; ) {
		IBITMAP *next = (IBITMAP*)bitmap->code;
		ibitmap_release(bitmap);
		bitmap = next;
	}

	return ani;
}


//---------------------------------------------------------------------
// Load Tcp Stream
//---------------------------------------------------------------------
iAnimate *iAnimate_Load_Tcp_Stream(IMDIO *stream)
{
	iAnimate *tcp;
	long *framepos;
	int direction, frame;
	int c1, c2, n, i, j;
	int w, h;
	int headlen;
	long current;
	#define IOFFSETLEN		64
	int line_buffer[IOFFSETLEN];
	int *line_offset;
	int  line_offset_size;
	IRGB *pal;
	int debug[100];

	c1 = is_getc(stream);
	c2 = is_getc(stream);

	if (c1 != 0x53 || c2 != 0x50) {
		iAnimate_ErrCode = -1;
		return NULL;
	}

	headlen = is_igetw(stream);
	direction = is_igetw(stream);
	frame = is_igetw(stream);

	w = is_igetw(stream);
	h = is_igetw(stream);

	tcp = iAnimate_New(w, h, 32, direction * frame, 0, 0);
	if (tcp == NULL) {
		iAnimate_ErrCode = -2;
		return NULL;
	}

	tcp->hotx = is_igetw(stream);
	tcp->hoty = is_igetw(stream);
	tcp->nrow = direction;
	tcp->ncol = frame;

	is_seekcur(stream, headlen - 12);

	framepos = (long*)icmalloc(sizeof(long) * tcp->framecount);
	tcp->pal = (IRGB*)icmalloc(sizeof(IRGB) * 256);
	
	if (framepos == 0 || tcp->pal == 0) {
		if (framepos) icfree(framepos);
		iAnimate_Free(tcp);
		iAnimate_ErrCode = -3;
		return NULL;
	}

	for (i = 0; i < 256; i++) {
		unsigned short cc;
		c1 = is_getc(stream);
		c2 = is_getc(stream);
		cc = (unsigned short)((c1 + (c2 << 8)) & 0xffff);
		tcp->pal[i].r = _im_getr16(cc);
		tcp->pal[i].g = _im_getg16(cc);
		tcp->pal[i].b = _im_getb16(cc);
	}

	for (i = 0; i < tcp->framecount; i++) {
		framepos[i] = is_igetl(stream) + headlen + 4;
	}

	current = headlen + 4 + 256 * 2 + tcp->framecount * 4;
	pal = tcp->pal;

	line_offset = line_buffer;
	line_offset_size = IOFFSETLEN;

	for (i = 0; i < 100; i++) debug[i] = 0;

	for (n = 0; n < tcp->framecount; n++) {
		IBITMAP *bitmap;
		int frame_width;
		int frame_height;
		short frame_keyx;
		short frame_keyy;
		
		if (current < framepos[n]) {
			is_seekcur(stream, framepos[n] - current);
			current = framepos[n];
		}

		frame_keyx = (short)is_igetl(stream);
		frame_keyy = (short)is_igetl(stream);
		frame_width = is_igetl(stream);
		frame_height = is_igetl(stream);

		current += 16;
		
		if (frame_width < tcp->w && frame_height < tcp->h && 0) {
			bitmap = tcp->frames[n];
		}	else {
			bitmap = ibitmap_create(frame_width, frame_height, 32);
			if (bitmap == NULL) break;
			ibitmap_release(tcp->frames[n]);
			tcp->frames[n] = bitmap;
		}

		ibitmap_fill(bitmap, 0, 0, bitmap->w, bitmap->h, 0, 0);

		bitmap->mask = (unsigned short)frame_keyx;
		bitmap->mask <<= 16;
		bitmap->mask |= (unsigned short)frame_keyy;

		if (frame_height > line_offset_size) {
			int *offset;
			for (; line_offset_size < frame_height; line_offset_size <<= 1);
			offset = (int*)icmalloc(sizeof(int) * line_offset_size);
			if (offset == NULL) break;
			if (line_offset != line_buffer) icfree(line_offset);
			line_offset = offset;
		}

		for (j = 0; j < frame_height; j++) {
			line_offset[j] = is_igetl(stream) + framepos[n];
			current += 4;
		}

		for (j = 0; j < frame_height; j++) {
			unsigned long *lptr = (unsigned long*)bitmap->line[j];

			if (current < line_offset[j]) {
				is_seekcur(stream, line_offset[j] - current);
				current = line_offset[n];
			}

			while (1) {
				unsigned char style = is_getc(stream);
				int flag = style >> 6;
				int b3 = (style >> 5) & 0x01;
				int low5 = style & 31;
				int low6 = style & 63;
				int c, a;

				current ++;

				if (style == 0) break;
				if (flag == 0) {
					if (b3 == 1) {
						c = is_getc(stream);
						current ++;
						a = _iscale_rgb_5[low5];
						*lptr++ = iARGB(a, pal[c].r, pal[c].g, pal[c].b);
					}
					else {
						a = is_getc(stream);
						c = is_getc(stream);
						current += 2;
						a = _iscale_rgb_5[a & 0x1f];
						for (i = low5; i > 0; i--) {
							*lptr++ = iARGB(a, pal[c].r, pal[c].g, pal[c].b);
						}
					}
				} 
				else if (flag == 1) {
					for (i = low6; i > 0; i--) {
						c = is_getc(stream);
						current ++;
						*lptr++ = iARGB(255, pal[c].r, pal[c].g, pal[c].b);
					}
				}
				else if (flag == 2) {
					c = is_getc(stream);
					current ++;
					for (i = low6; i > 0; i--) {
						*lptr++ = iARGB(255, pal[c].r, pal[c].g, pal[c].b);
					}
				}
				else if (flag == 3) {
					for (i = low6; i > 0; i--) *lptr++ = 0;
				}
			}
		}
	}

	icfree(framepos);
	if (line_offset != line_buffer) icfree(line_offset);

	// loading failed
	if (n < tcp->framecount) {
		iAnimate_ErrCode = -4;
		iAnimate_Free(tcp);
		return NULL;
	}

	return tcp;
}


//---------------------------------------------------------------------
// load animate with format detectiing
//---------------------------------------------------------------------
iAnimate *iAnimate_Load_Stream(IMDIO *stream)
{
	iAnimate *ani = NULL;
	int firstch;

	firstch = is_getc(stream);
	is_ungetc(stream, firstch);

	if (firstch == 0x47) {			// for gif format
		ani = iAnimate_Load_Gif_Stream(stream);
	}
	else if (firstch == 0x53) {		// for tcp format
		ani = iAnimate_Load_Tcp_Stream(stream);
	}
	else {
		// unknow
		return NULL;
	}

	return ani;
}


//---------------------------------------------------------------------
// load animate from file
//---------------------------------------------------------------------
iAnimate *iAnimate_Load_File(const char *filename, long pos)
{
	IMDIO stream;
	iAnimate *ani = NULL;

	if (is_open_file(&stream, filename, "rb") != 0) {
		return NULL;
	}

	fseek((FILE*)(stream.data), pos, SEEK_SET);
	ani = iAnimate_Load_Stream(&stream);
	is_close_file(&stream);

	return ani;
}

//---------------------------------------------------------------------
// load animate from memory
//---------------------------------------------------------------------
iAnimate *iAnimate_Load_Memory(const void *ptr, long size)
{
	IMDIO stream;
	iAnimate *ani;
	size = (size <= 0)? 0x7fffffffl : size;
	is_init_mem(&stream, ptr, size);
	ani = iAnimate_Load_Stream(&stream);
	return ani;
}


//---------------------------------------------------------------------
// Save Tga Batch
//---------------------------------------------------------------------
void Animate_Save_Tga(iAnimate *ani, const char *file_name_prefix)
{
	char *prefix = "";
	int numlen, n;
	char format[32];
	char namebuf[128];

	assert(ani);

	if (file_name_prefix != NULL) prefix = (char*)file_name_prefix;
	sprintf(format, "%d", ani->framecount + 1);
	numlen = strlen(format);
	sprintf(format, "%%s%%0%dd.tga", numlen);

	for (n = 0; n < ani->framecount; n++) {
		sprintf(namebuf, format, prefix, n);
		isave_tga_file(namebuf, ani->frames[n], ani->pal);
	}
}



//=====================================================================
// Color Quantization (octree algorithm and median-cut algorithm)
//=====================================================================

//---------------------------------------------------------------------
// Octree Structures
//---------------------------------------------------------------------
typedef struct iOctreeNode
{
	int isleaf;
	unsigned long npixel;
	unsigned long sumr;
	unsigned long sumg;
	unsigned long sumb;
	struct iOctreeNode *next;
	struct iOctreeNode *child[8];
}	iOctreeNode;

typedef struct iOctree
{
	int depth;
	int nleaf;
	int nnode;
	unsigned char *cache;
	struct iOctreeNode *root;
	struct iOctreeNode *reduce[8];
}	iOctree;

static void ioctree_delnode(iOctreeNode *node)
{
	int i;
	assert(node);
	for (i = 0; i < 8; i++) {
		if (node->child[i]) ioctree_delnode(node->child[i]);
		node->child[i] = NULL;
	}
	node->next = NULL;
	free(node);
}

static iOctreeNode *ioctree_newnode(void)
{
	iOctreeNode *node;
	int i;
	node = (iOctreeNode*)malloc(sizeof(iOctreeNode));
	if (node == NULL) return NULL;
	node->isleaf = 0;
	node->npixel = 0;
	node->sumr = 0;
	node->sumg = 0;
	node->sumb = 0;
	node->next = NULL;
	for (i = 0; i < 8; i++) {
		node->child[i] = NULL;
	}
	return node;
}

void ioctree_init(iOctree *tree, int maxbits)
{
	int i;
	assert(tree);
	tree->root = NULL;
	tree->depth = (maxbits <= 8)? ((maxbits < 2)? 2 : maxbits) : 8;
	tree->nleaf = 0;
	tree->nnode = 0;
	for (i = 0; i < 8; i++) {
		tree->reduce[i] = NULL;
	}
	tree->cache = NULL;
}

void ioctree_reset(iOctree *tree)
{
	int i;
	assert(tree);
	if (tree->root) {
		ioctree_delnode(tree->root);
		tree->root = NULL;
	}
	if (tree->cache) {
		free(tree->cache);
		tree->cache = NULL;
	}
	for (i = 0; i < 8; i++) {
		tree->reduce[i] = NULL;
	}
	tree->nleaf = 0;
	tree->nnode = 0;
}

void ioctree_destroy(iOctree *tree)
{
	ioctree_reset(tree);
}

void ioctree_addcolor(iOctree *tree, int r, int g, int b)
{
	iOctreeNode *node, *newnode;
	int index, i, n;
	if (tree->root == NULL) {
		tree->root = ioctree_newnode();
		assert(tree->root);
		tree->nnode++;
	}
	node = tree->root;
	r = ((r > 255)? 255 : ((r < 0)? 0 : r));
	g = ((g > 255)? 255 : ((g < 0)? 0 : g));
	b = ((b > 255)? 255 : ((b < 0)? 0 : b));
	for (i = 0; i <= tree->depth; i++) {
		n = 7 - i;
		index = (((r >> n) & 1) << 2) | 
			(((g >> n) & 1) << 1) | ((b >> n) & 1);
		if (node->child[index] == NULL) {
			newnode = ioctree_newnode();
			assert(newnode);
			if (i == tree->depth) {
				newnode->isleaf = 1;
				tree->nleaf++;
			}	else {
				newnode->isleaf = 0;
				newnode->next = tree->reduce[i];
				tree->reduce[i] = newnode;
			}
			node->child[index] = newnode;
			tree->nnode++;
		}
		node = node->child[index];
		if (node->isleaf) {
			node->npixel++;
			node->sumr += r;
			node->sumg += g;
			node->sumb += b;
			break;
		}
	}
	if (tree->cache) {
		free(tree->cache);
		tree->cache = NULL;
	}
}

void ioctree_reduce(iOctree *tree)
{
	unsigned long sumr, sumg, sumb, npixel;
	iOctreeNode *node, *child;
	int nchild, i;
	for (i = tree->depth - 1; i >= 0; i--) {
		if (tree->reduce[i] != NULL) break;
	}
	if (i < 0) return;
	node = tree->reduce[i];
	tree->reduce[i] = node->next;
	sumr = sumg = sumb = npixel = 0;
	nchild = 0;
	//printf("5");
	for (i = 0; i < 8; i++) {
		child = node->child[i];
		if (child != NULL) {
			sumr += child->sumr;
			sumg += child->sumg;
			sumb += child->sumb;
			npixel += child->npixel;
			ioctree_delnode(child);
			node->child[i] = NULL;
			nchild++;
		}
	}
	node->isleaf = 1;
	node->sumr = sumr;
	node->sumg = sumg;
	node->sumb = sumb;
	node->npixel = npixel;
	tree->nleaf -= (nchild - 1);
	tree->nnode -= nchild;
	//printf("6(%d) ", nchild);
}

int ioctree_get_ncolors(const iOctree *tree)
{
	return tree->nleaf;
}

static int ioctree_recursion(iOctree *tree, iOctreeNode *node, int index)
{
	int count, n, i, r, g, b;
	if (node->isleaf == 0) {
		count = 0;
		for (i = 0; i < 8; i++) {
			if (node->child[i]) {
				n = ioctree_recursion(tree, node->child[i], index);
				count += n;
				index += n;
			}
		}
		return count;
	}
	assert(node->npixel > 0);
	r = (int)(node->sumr / node->npixel);
	g = (int)(node->sumg / node->npixel);
	b = (int)(node->sumb / node->npixel);
	if (index >= tree->nleaf || tree->cache == NULL) {
		printf("error !!\n");
		assert(0);
	}
	tree->cache[index * 3 + 0] = (unsigned char)((r <= 255)? r : 255);
	tree->cache[index * 3 + 1] = (unsigned char)((g <= 255)? g : 255);
	tree->cache[index * 3 + 2] = (unsigned char)((b <= 255)? b : 255);
	return 1;
}

void ioctree_get_color(iOctree *tree, int id, int *r, int *g, int *b)
{
	int n;
	if (tree->root == NULL) {
		return;
	}
	if (tree->cache == NULL) {
		tree->cache = (unsigned char*)malloc(sizeof(char) * tree->nleaf * 3);
		assert(tree->cache);
		n = ioctree_recursion(tree, tree->root, 0);
		assert(n == tree->nleaf);
	}
	if (id < 0 || id >= tree->nleaf) id = 0;
	if (r) *r = tree->cache[id * 3 + 0];
	if (g) *g = tree->cache[id * 3 + 1];
	if (b) *b = tree->cache[id * 3 + 2];
}


int ioctree_generate(const IRGB *in, long len, IRGB *out, int size, int lv)
{
	iOctree tree;
	int i, r, g, b;
	ioctree_init(&tree, lv);
	memset(out, 0, sizeof(IRGB) * size);
	for (i = 0; i < len; i++) {
		ioctree_addcolor(&tree, in[i].r, in[i].g, in[i].b);
		while (tree.nleaf > size) {
			ioctree_reduce(&tree);
		}
	}
	for (i = 0; i < tree.nleaf; i++) {
		ioctree_get_color(&tree, i, &r, &g, &b);
		out[i].r = (unsigned char)(r & 255);
		out[i].g = (unsigned char)(g & 255);
		out[i].b = (unsigned char)(b & 255);
	}
	ioctree_destroy(&tree);
	return i;
}


//---------------------------------------------------------------------
// Median Cut Color Quantization
//---------------------------------------------------------------------
typedef struct iMedianPoint
{
	unsigned char x[3];
	unsigned long count;
}	iMedianPoint;

typedef struct iMedianBox
{
	iMedianPoint *points;
	iMedianPoint mincorner;
	iMedianPoint maxcorner;
	unsigned int axis;
	unsigned int distance;
	unsigned long npoints;
	unsigned long ncolors;
}	iMedianBox;

static int imedian_cut_cmp_0(const void *entry1, const void *entry2)
{
	return ((const iMedianPoint*)entry1)->x[0] - 
		((const iMedianPoint*)entry2)->x[0];
}

static int imedian_cut_cmp_1(const void *entry1, const void *entry2)
{
	return ((const iMedianPoint*)entry1)->x[1] - 
		((const iMedianPoint*)entry2)->x[1];
}

static int imedian_cut_cmp_2(const void *entry1, const void *entry2)
{
	return ((const iMedianPoint*)entry1)->x[2] - 
		((const iMedianPoint*)entry2)->x[2];
}

iMedianBox *imedian_box_create(const iMedianPoint *points, int n, long cnt)
{
	long count, npoints, total, inc, size, pos, i;
	iMedianBox *box;

	count = cnt;
	box = (iMedianBox*)malloc(sizeof(iMedianBox));
	if (box == NULL) return NULL;

	for (total = 0, i = 0; i < n; i++) {
		total += points[i].count;
	}

	if (count >= 0) {
		if (count > total) count = total;
		for (npoints = 0, size = 0, i = 0; i < n; i++) {
			npoints += points[i].count;
			size++;
			if (npoints >= count) break;
		}
		inc = 1;
		pos = 0;
	}	else {
		count = -count;
		if (count > total) count = total;
		for (npoints = 0, size = 0, i = n - 1; i >= 0; i--) {
			npoints += points[i].count;
			size++;
			if (npoints >= count) break;
		}
		inc = -1;
		pos = n - 1;
	}

	box->points = (iMedianPoint*)malloc(sizeof(iMedianPoint) * size);
	if (box->points == NULL) {
		free(box);
		return NULL;
	}

	box->npoints = size;
	box->ncolors = 0;
	box->distance = 0;
	box->axis = 0;

	for (i = 0; ; pos += inc) {
		const iMedianPoint *point = &points[pos];
		box->points[i] = points[pos];
		cnt = point->count;
		if (cnt + box->ncolors >= count) cnt = count - box->ncolors;
		box->points[i].count = cnt;
		box->ncolors += cnt;
		assert(cnt > 0);
		i++;
		if (box->ncolors >= count) {
			break;
		}
	}

	if (i != size) {
		printf("error: i=%d size=%d\n", (int)i, (int)size);
	}

	assert(i == size);

	for (i = 0; i < 3; i++) {
		box->mincorner.x[i] = 255;
		box->maxcorner.x[i] = 0;
	}

	return box;
}

void imedian_box_shrink(iMedianBox *box)
{
	iMedianPoint *points;
	int n, i, j;

	points = box->points;
	n = box->npoints;

	assert(box->npoints > 0);

	for (i = 0; i < 3; i++) {
		box->mincorner.x[i] = points[0].x[i];
		box->maxcorner.x[i] = points[0].x[i];
	}

	for (i = 0; i < n; i++) {
		for (j = 0; j < 3; j++) {
			if (points[i].x[j] < box->mincorner.x[j])
				box->mincorner.x[j] = points[i].x[j];
			if (points[i].x[j] > box->maxcorner.x[j])
				box->maxcorner.x[j] = points[i].x[j];
		}
	}
	
	box->distance = 0;
	box->axis = 0xff;

	for (i = 0; i < 3; i++) {
		long distance = box->maxcorner.x[i] - box->mincorner.x[i];
	#if 1
		static long rights[3] = { 30, 59, 11 };
		distance = distance * rights[i];
	#endif
		if (distance >= box->distance) {
			box->distance = distance;
			box->axis = i;
		}
	}

	switch (box->axis)
	{
	case 0: 
		qsort(box->points, box->npoints, sizeof(iMedianPoint), 
			imedian_cut_cmp_0);
		break;
	case 1:
		qsort(box->points, box->npoints, sizeof(iMedianPoint), 
			imedian_cut_cmp_1);
		break;
	case 2:
		qsort(box->points, box->npoints, sizeof(iMedianPoint), 
			imedian_cut_cmp_2);
		break;
	default:
		printf("error %d\n", box->axis);
		break;
	}
	//printf("box dist=%d\n", box->distance);
}


void imedian_box_release(iMedianBox *box)
{
	if (box->points) {
		free(box->points);
		box->points = NULL;
	}
	box->npoints = 0;
	box->ncolors = 0;
	free(box);
}

void imedian_box_color(const iMedianBox *box, int *r, int *g, int *b)
{
	unsigned long sumr, sumg, sumb, count, i;
	unsigned long R, G, B, N;
	sumr = sumg = sumb = count = 0;
	for (i = 0; i < box->npoints; i++) {
		R = box->points[i].x[0];
		G = box->points[i].x[1];
		B = box->points[i].x[2];
		N = box->points[i].count;
		sumr += R * N;
		sumg += G * N;
		sumb += B * N;
		count += N;
	}
	R = sumr / count;
	G = sumg / count;
	B = sumb / count;
	if (R > 255) R = 255;
	if (G > 255) G = 255;
	if (B > 255) B = 255;
	assert(count > 0);
	if (r) *r = (int)R;
	if (g) *g = (int)G;
	if (b) *b = (int)B;
}

static unsigned int imedian_index(const IRGB *pixel, int depth)
{
	int r = pixel->r >> (8 - depth);
	int g = pixel->g >> (8 - depth);
	int b = pixel->b >> (8 - depth);
	return (r << (depth * 2)) | (g << depth) | (b);
}

iMedianBox *imedian_box_initialize(const IRGB *pixels, long count, int depth)
{
	unsigned char mask;
	unsigned int *colors;
	long total = (1ul << (depth * 3));
	int npoints, index, i, k;
	iMedianPoint *points;
	iMedianBox *box;

	colors = (unsigned int*)malloc(sizeof(int) * total);
	assert(colors);

	memset(colors, 0, sizeof(int) * total);
	for (npoints = 0, i = 0; i < count; i++) {
		index = imedian_index(&pixels[i], depth);
		if (colors[index] == 0) {
			npoints++;
		}
		colors[index]++;
	}

	points = (iMedianPoint*)malloc(sizeof(iMedianPoint) * npoints);
	assert(points);

	mask = 0xff ^ ((1 << (8 - depth)) - 1);
	for (i = 0, k = 0; i < count; i++) {
		index = imedian_index(&pixels[i], depth);
		if (colors[index] > 0) {
			points[k].x[0] = pixels[i].r & mask;
			points[k].x[1] = pixels[i].g & mask;
			points[k].x[2] = pixels[i].b & mask;
			points[k].count = colors[index];
			colors[index] = 0;
			k++;
		}
	}

	assert(k == npoints);
	free(colors);

	box = imedian_box_create(points, npoints, count);
	assert(box->npoints = npoints);
	assert(box->ncolors = count);

	free(points);

	return box;
}


int imedian_cut(const IRGB *in, long len, IRGB *out, int size, int depth)
{
	iMedianBox **boxes;
	int nboxes, sum, i;

	if (len < 1 || size < 1) return 0;
	boxes = (iMedianBox**)malloc(sizeof(iMedianBox*) * (size + 2));
	assert(boxes);

	if (depth > 7) depth = 7;
	if (depth < 4) depth = 4;

	boxes[0] = imedian_box_initialize(in, len, depth);
	assert(boxes[0]);
	assert(boxes[0]->ncolors == len);

	imedian_box_shrink(boxes[0]);

	for (nboxes = 1; nboxes < size; ) {
		iMedianBox *box, *newbox1, *newbox2;
		long maxindex = -1;
		long distance = -1;
		long count = 0;

		for (i = 0; i < nboxes; i++) {
			box = boxes[i];
			if ((long)box->distance > distance && box->npoints >= 2) {
				distance = box->distance;
				maxindex = i;
			}
		}

		if (maxindex < 0) break;
		//printf("max dist=%d index=%d\n", distance, maxindex);

		box = boxes[maxindex];
		count = box->ncolors / 2;
		newbox1 = imedian_box_create(box->points, box->npoints, count);
		count = count - (long)box->ncolors;
		newbox2 = imedian_box_create(box->points, box->npoints, count);
		assert(newbox1 && newbox2);
		assert(newbox1->ncolors + newbox2->ncolors == box->ncolors);

		imedian_box_shrink(newbox1);
		imedian_box_shrink(newbox2);
		
		boxes[maxindex] = newbox1;
		boxes[nboxes] = newbox2;
		nboxes++;

#if 0
		printf("box(%d, %d, %d) = nbox1(%d, %d, %d) + nbox2(%d, %d, %d)\n",
			box->npoints, box->ncolors, box->distance, 
			newbox1->npoints, newbox1->ncolors, newbox1->distance,
			newbox2->npoints, newbox2->ncolors, newbox2->distance);
#endif

		imedian_box_release(box);
	}

	memset(out, 0, sizeof(IRGB) * size);

	for (i = 0, sum = 0; i < nboxes; i++) {
		int r, g, b;
		imedian_box_color(boxes[i], &r, &g, &b);
		out[i].r = (unsigned char)r;
		out[i].g = (unsigned char)g;
		out[i].b = (unsigned char)b;
		sum += boxes[i]->npoints;
	}

	//assert(sum == len);
	//printf("sum=%d len=%d\n", sum, len);

	for (i = 0; i < nboxes; i++) {
		imedian_box_release(boxes[i]);
	}

	free(boxes);
	//printf("%d\n", nboxes);
	return nboxes;
}


//---------------------------------------------------------------------
// Standard Quantization Interface
//---------------------------------------------------------------------

// octree color quantization algorithm
// using less memory and cpu with normal quality
// nbits (4 - 8) (the higher value the better quality with higher cpu)
int iquantize_octree(	const IRGB *input, 
						long input_size, 
						IRGB *output,
						long output_size, 
						int nbits)
{
	return ioctree_generate(input, input_size, output, output_size, nbits);
}


// median-cut color quantization algorithm
// using more memory and cpu with better quality
// nbits (4 - 8) (the higher value the better quality with higher cpu)
int iquantize_median_cut(	const IRGB *input, 
							long input_size, 
							IRGB *output,
							long output_size, 
							int nbits)
{
	return imedian_cut(input, input_size, output, output_size, nbits);
}


// reduce bitmap color
IBITMAP *ibitmap_reduce (	const IBITMAP *src, 
							const IRGB *palsrc,
							IRGB *palout,
							int palsize,
							int reserved,
							int method,
							int dither)
{
	static int dithering[8] = { 1, 16, 32, 42 };
	unsigned char *lookup;
	long i, j, k, x, y;
	int *line[3], e[3], factor;
	int *buffer, *endup;
	IBITMAP *dst;
	IRGB *input;

	assert(src && palout);

	input = (IRGB*)malloc(sizeof(IRGB) * src->w * src->h);
	assert(input);

	if (palsrc == NULL)
		palsrc = _ipaletted;

	if (_ibitmap_pixfmt(src) == 0) 
		ibitmap_set_pixfmt((IBITMAP*)src, 0);

	for (j = 0, k = 0; j < src->h; j++) {
		unsigned char *ptr = _ilineptr(src, j);
		int npixelbyte = (src->bpp + 7) / 8;
		int pixfmt = _ibitmap_pixfmt(src);
		for (i = 0; i < src->w; i++, k++) {
			ICOLORD c = 0, r = 0, g = 0, b = 0, a = 0;
			switch (pixfmt) 
			{
			case IPIX_FMT_8:
				c = _im_get1b(ptr);
				r = palsrc[c].r, g = palsrc[c].g, b = palsrc[c].b; 
				break;
			case IPIX_FMT_RGB15:
				c = _im_get2b(ptr); 
				_im_scale_rgb(c, 15, r, g, b);
				break;
			case IPIX_FMT_BGR15:
				c = _im_get2b(ptr); 
				_im_scale_rgb(c, 15, b, g, r);
				break;
			case IPIX_FMT_RGB16:
				c = _im_get2b(ptr); 
				_im_scale_rgb(c, 16, r, g, b);
				break;
			case IPIX_FMT_BGR16:
				c = _im_get2b(ptr); 
				_im_scale_rgb(c, 16, b, g, r);
				break;
			case IPIX_FMT_RGB24:
				c = _im_get3b(ptr); 
				_im_scale_rgb(c, 24, r, g, b);
				break;
			case IPIX_FMT_BGR24:
				c = _im_get3b(ptr); 
				_im_scale_rgb(c, 24, b, g, r);
				break;
			case IPIX_FMT_RGB32:
				c = _im_get4b(ptr); 
				_im_scale_rgb(c, 32, r, g, b);
				break;
			case IPIX_FMT_BGR32:
				c = _im_get4b(ptr); 
				_im_scale_bgr(c, 32, b, g, r);
				break;
			case IPIX_FMT_ARGB32:
				c = _im_get4b(ptr); 
				_im_scale_argb(c, 32, a, r, g, b);
				break;
			case IPIX_FMT_ABGR32:
				c = _im_get4b(ptr); 
				_im_scale_argb(c, 32, a, b, g, r);
				break;
			case IPIX_FMT_RGBA32:
				c = _im_get4b(ptr); 
				_im_scale_argb(c, 32, r, g, b, a);
				break;
			case IPIX_FMT_BGRA32:
				c = _im_get4b(ptr); 
				_im_scale_argb(c, 32, b, g, r, a);
				break;
			case IPIX_FMT_ARGB_4444:
				c = _im_get2b(ptr);
				_im_scale_argb(c, 4444, a, r, g, b);
				break;
			case IPIX_FMT_ABGR_4444:
				c = _im_get2b(ptr);
				_im_scale_argb(c, 4444, a, b, g, r);
				break;
			case IPIX_FMT_RGBA_4444:
				c = _im_get2b(ptr);
				_im_scale_argb(c, 4444, r, g, b, a);
				break;
			case IPIX_FMT_BGRA_4444:
				c = _im_get2b(ptr);
				_im_scale_argb(c, 4444, b, g, r, a);
				break;
			case IPIX_FMT_ARGB_1555:
				c = _im_get2b(ptr);
				_im_scale_argb(c, 1555, a, r, g, b);
				break;
			case IPIX_FMT_ABGR_1555:
				c = _im_get2b(ptr);
				_im_scale_argb(c, 1555, a, b, g, r);
				break;
			case IPIX_FMT_RGBA_5551:
				c = _im_get2b(ptr);
				_im_scale_argb(c, 5551, r, g, b, a);
				break;
			case IPIX_FMT_BGRA_5551:
				c = _im_get2b(ptr);
				_im_scale_argb(c, 5551, r, b, b, r);
				break;
			default:
				r = g = b = a = 0;
				break;
			}
			input[k].r = r;
			input[k].g = g;
			input[k].b = b;
			ptr += npixelbyte;
		}
	}

	if (palsize > 256) palsize = 256;
	if (reserved > palsize) reserved = palsize;
	dither = (dither > 3)? 3 : ((dither < 0)? 0 : dither);
	factor = dithering[dither];

	if (reserved + 4 < palsize) {
		if (method == 0) {
			iquantize_octree(input, k, palout + reserved, 
				palsize - reserved, 6);
		}	else {
			iquantize_median_cut(input, k, palout + reserved, 
				palsize - reserved, 6);
		}
	}

	buffer = (int*)malloc(sizeof(int) * (src->w + 8) * 4 * 3);
	endup = buffer + (src->w + 8) * 4 * 3;
	assert(buffer);

	memset(buffer, 0, sizeof(int) * (src->w + 8) * 4 * 3);

	line[0] = buffer + 4 * 4;
	line[1] = line[0] + (src->w + 8) * 4;
	line[2] = line[1] + (src->w + 8) * 4;

	#define ipixerr(_yy, _xx, _ii) line[_yy][((_xx) * 4) + (_ii)]

	dst = ibitmap_create(src->w, src->h, 8);
	assert(dst);

	lookup = (unsigned char*)malloc((2 << 18) * 2);
	assert(lookup);
	memset(lookup, 0, (2 << 18) * 2);

	for (y = 0, k = 0; y < src->h; y++) {
		unsigned char *ptr1 = _ilineptr(src, y);
		unsigned char *ptr2 = _ilineptr(dst, y);
		int npixelbyte1 = (src->bpp + 7) / 8;
		int npixelbyte2 = 1;
		ICOLORD index, nc, c;
		int nr, ng, nb;
		int r, g, b, *z;
		for (x = 0; x < src->w; x++, k++) {
			r = input[k].r;
			g = input[k].g;
			b = input[k].b;

			r += ipixerr(0, x, 0) / factor;
			g += ipixerr(0, x, 1) / factor;
			b += ipixerr(0, x, 2) / factor;

			r = (r > 255)? 255 : ((r < 0)? 0 : r);
			g = (g > 255)? 255 : ((g < 0)? 0 : g);
			b = (b > 255)? 255 : ((b < 0)? 0 : b);

			index =	((r & 0xfc) << 10) | 
					((g & 0xfc) <<  4) |
					((b & 0xfc) >>  2);
			index = index << 1;

			#if 1
			if (lookup[index] == 0) {
				nc = _ibestfit_color(palout, r, g, b, palsize);
				lookup[index] = 1;
				lookup[index + 1] = nc;
			}	else {
				nc = lookup[index + 1];
			}
			#else
			nc = _ibestfit_color(palout, r, g, b, palsize);
			#endif

			*ptr2 = (unsigned char)(nc & 0xff);
			ptr1 += npixelbyte1;
			ptr2 += npixelbyte2;

			if (dither == 0) continue;

			nr = palout[nc].r;
			ng = palout[nc].g;
			nb = palout[nc].b;

			e[0] = (int)r - nr;
			e[1] = (int)g - ng;
			e[2] = (int)b - nb;

			switch (dither)
			{
			case 1:
				for (c = 0; c < 3; c++) {
					ipixerr(0, x + 1, c) += e[c] * 7;
					ipixerr(1, x - 1, c) += e[c] * 3;
					ipixerr(1, x + 0, c) += e[c] * 5;
					ipixerr(1, x + 1, c) += e[c] * 1;
				}
				break;
			case 2:
				for (c = 0; c < 3; c++) {
					ipixerr(0, x + 1, c) += e[c] * 8;
					ipixerr(0, x + 2, c) += e[c] * 4;
					ipixerr(1, x - 2, c) += e[c] * 2;
					ipixerr(1, x - 1, c) += e[c] * 4;
					ipixerr(1, x + 0, c) += e[c] * 8;
					ipixerr(1, x + 1, c) += e[c] * 4;
					ipixerr(1, x + 2, c) += e[c] * 2;
				}
				break;
			case 3:
				for (c = 0; c < 3; c++) {
					ipixerr(0, x + 1, c) += e[c] * 8;
					ipixerr(0, x + 2, c) += e[c] * 4;
					ipixerr(1, x - 2, c) += e[c] * 2;
					ipixerr(1, x - 1, c) += e[c] * 4;
					ipixerr(1, x + 0, c) += e[c] * 8;
					ipixerr(1, x + 1, c) += e[c] * 4;
					ipixerr(1, x + 2, c) += e[c] * 2;
					ipixerr(2, x - 2, c) += e[c] * 1;
					ipixerr(2, x - 1, c) += e[c] * 2;
					ipixerr(2, x + 0, c) += e[c] * 4;
					ipixerr(2, x + 1, c) += e[c] * 2;
					ipixerr(2, x + 2, c) += e[c] * 1;
				}
				break;
			}
		}

		z = line[0];
		line[0] = line[1]; 
		line[1] = line[2];
		line[2] = z;

		memset(line[2], 0, sizeof(int) * (src->w + 4) * 4);
	}

	free(lookup);
	free(buffer);
	free(input);

	#undef ipixerr

	return dst;
}


// calculate two bitmap's error
// the size of two bitmap must equal to each other
double ibitmap_error(	const IBITMAP *src, 
						const IBITMAP *dst,
						const IRGB *palsrc,
						const IRGB *paldst,
						int *red_max_error,
						int *green_max_error,
						int *blue_max_error)
{
	long npixelbyte1 = (src->bpp + 7) / 8;
	long npixelbyte2 = (dst->bpp + 7) / 8;
	double sum1, sum2, error;
	long i, j;
	if (src->w != dst->w || src->h != dst->h) {
		return -1;
	}
	sum1 = 0.0;
	sum2 = 0.0;
	error = 0.0;
	for (j = 0; j < src->h; j++) {
		unsigned char *ptr1 = _ilineptr(src, j);
		unsigned char *ptr2 = _ilineptr(dst, j);
		for (i = 0; i < src->w; i++) {
			ICOLORD r1, g1, b1, r2, g2, b2, c1, c2;
			ICOLORD er, eg, eb, p1, p2, pe;
			switch (src->bpp) 
			{
			case 8:
				c1 = _im_get1b(ptr1);
				r1 = palsrc[c1].r; 
				g1 = palsrc[c1].g; 
				b1 = palsrc[c1].b;
				break;
			case 15:
				c1 = _im_get2b(ptr1); 
				r1 = _im_getr15(c1); 
				g1 = _im_getg15(c1); 
				b1 = _im_getb15(c1);
				break;
			case 16:
				c1 = _im_get2b(ptr1); 
				r1 = _im_getr16(c1); 
				g1 = _im_getg16(c1); 
				b1 = _im_getb16(c1);
				break;
			case 24:
				c1 = _im_get3b(ptr1); 
				r1 = _im_getr24(c1); 
				g1 = _im_getg24(c1); 
				b1 = _im_getb24(c1);
				break;
			case 32:
				c1 = _im_get4b(ptr1); 
				r1 = _im_getr32(c1); 
				g1 = _im_getg32(c1); 
				b1 = _im_getb32(c1);
				break;
			default:
				c1 = r1 = g1 = b1 = 0;
				break;
			}
			switch (dst->bpp)
			{
			case 8:
				c2 = _im_get1b(ptr2);
				r2 = paldst[c2].r; 
				g2 = paldst[c2].g; 
				b2 = paldst[c2].b;
				break;
			case 15:
				c2 = _im_get2b(ptr2); 
				r2 = _im_getr15(c2); 
				g2 = _im_getg15(c2); 
				b2 = _im_getb15(c2);
				break;
			case 16:
				c2 = _im_get2b(ptr2); 
				r2 = _im_getr16(c2); 
				g2 = _im_getg16(c2); 
				b2 = _im_getb16(c2);
				break;
			case 24:
				c2 = _im_get3b(ptr2); 
				r2 = _im_getr24(c2); 
				g2 = _im_getg24(c2); 
				b2 = _im_getb24(c2);
				break;
			case 32:
				c2 = _im_get4b(ptr2); 
				r2 = _im_getr32(c2); 
				g2 = _im_getg32(c2); 
				b2 = _im_getb32(c2);
				break;
			default:
				c2 = r2 = g2 = b2 = 0;
				break;
			}
			er = (r1 > r2)? (r1 - r2) : (r2 - r1);
			eg = (g1 > g2)? (g1 - g2) : (g2 - g1);
			eb = (b1 > b2)? (b1 - b2) : (b2 - b1);
			if (red_max_error) {
				if ((int)er > *red_max_error) *red_max_error = (int)er;
			}
			if (green_max_error) {
				if ((int)eg > *green_max_error) *green_max_error = (int)eg;
			}
			if (blue_max_error) {
				if ((int)eb > *blue_max_error) *blue_max_error = (int)eb;
			}
			p1 =	(r1 * r1 * 30 * 30) +
					(g1 * g1 * 59 * 59) +
					(b1 * b1 * 11 * 11);
			p2 =	(r2 * r2 * 30 * 30) +
					(g2 * g2 * 59 * 59) +
					(b2 * b2 * 11 * 11);
			pe =	(er * er * 30 * 30) +
					(eg * eg * 59 * 59) +
					(eb * eb * 11 * 11);
			sum1 += p1;
			sum2 += p2;
			error += pe;
			ptr1 += npixelbyte1;
			ptr2 += npixelbyte2;
		}
	}

	return error / sum2;
}



//=====================================================================
// Color Space Convertion
//=====================================================================

#define ICONV_BITS		10
#define ICONV_HALF		(1 << (ICONV_BITS - 1))
#define ICONV_FIX(F)	((int)((F) * (1 << ICONV_BITS) + 0.5))

#define ICONV_MAX(a, b) (((a) > (b))? (a) : (b))
#define ICONV_MIN(a, b) (((a) < (b))? (a) : (b))
#define ICONV_MID(a, x, b) ICONV_MAX(a, ICONV_MIN(x, b))

#define ICONV_CLAMP(x)	{ if (x < 0) x = 0; else if (x > 255) x = 255; }
#define ICONV_CLAMP1(r, g, b) { \
	r >>= ICONV_BITS; g >>= ICONV_BITS; b >>= ICONV_BITS; \
	if (((unsigned)(r) | (unsigned)(g) | (unsigned)(b)) & (~255u)) { \
		ICONV_CLAMP(r); ICONV_CLAMP(g); ICONV_CLAMP(b); } \
	}
#define ICONV_CLAMP2(r1, g1, b1, r2, g2, b2) { \
	r1 >>= ICONV_BITS; g1 >>= ICONV_BITS; b1 >>= ICONV_BITS; \
	r2 >>= ICONV_BITS; g2 >>= ICONV_BITS; b2 >>= ICONV_BITS; \
	if (((unsigned)r1 | (unsigned)g1 | (unsigned)b1 |	\
		 (unsigned)r2 | (unsigned)g2 | (unsigned)b2) & (~255u)) {	\
		ICONV_CLAMP(r1); ICONV_CLAMP(g1); ICONV_CLAMP(b1); \
		ICONV_CLAMP(r2); ICONV_CLAMP(g2); ICONV_CLAMP(b2); } \
	}

#define ICONV_MUL(r, g, b, x, y, z, w) \
	(ICONV_FIX(x) * r + ICONV_FIX(y) * g + ICONV_FIX(z) * b + w)

#define ICONV_RGB_TO_Y(r, g, b) \
	ICONV_MUL(r, g, b, 0.299, 0.584, 0.117, 0)
#define ICONV_RGB_TO_Cr(r, g, b) \
	ICONV_MUL(r, g, b, 0.5, -0.4187, -0.0813, (128 << ICONV_BITS))
#define ICONV_RGB_TO_Cb(r, g, b) \
	ICONV_MUL(r, g, b, -0.1687, -0.3313, 0.5, (128 << ICONV_BITS))

#define ICONV_SCALE(f, x) (ICONV_FIX(f) * (x - 128) + ICONV_HALF)

#define ICONV_YCrCb_TO_R(Y, Cr, Cb) \
	((Y << ICONV_BITS) + ICONV_SCALE(1.402, Cr))
#define ICONV_YCrCb_TO_G(Y, Cr, Cb) \
	((Y << ICONV_BITS) - ICONV_SCALE(0.34414, Cb) - \
	ICONV_SCALE(0.71414, Cr))
#define ICONV_YCrCb_TO_B(Y, Cr, Cb) \
	((Y << ICONV_BITS) + ICONV_SCALE(1.772, Cb))

#define ICONV_GET_RGB(input, x, y, z) { \
	x = input->r; y = input->g; z = input->b; input++; }
#define ICONV_PUT_RGB(output, x, y, z) { \
	output->r = (unsigned char)x; \
	output->g = (unsigned char)y; \
	output->b = (unsigned char)z; \
	output++; }

#define ICONV_GET_YCrCb(input, x, y, z) { \
	x = input->Y; y = input->Cr; z = input->Cb; input++; }
#define ICONV_PUT_YCrCb(output, x, y, z) { \
	output->Y = (unsigned char)x; \
	output->Cr = (unsigned char)y; \
	output->Cb = (unsigned char)z; \
	output++; }


void iconv_RGB_to_YCrCb(const IRGB *input, long size, IYCrCb *output)
{
	int Y1, Cr1, Cb1, Y2, Cr2, Cb2;
	int r1, g1, b1, r2, g2, b2;

	#define iconv_rgb_to_YCrCb_x1 {		\
		ICONV_GET_RGB(input, r1, g1, b1); \
		Y1 = ICONV_RGB_TO_Y(r1, g1, b1); \
		Cr1 = ICONV_RGB_TO_Cr(r1, g1, b1); \
		Cb1 = ICONV_RGB_TO_Cb(r1, g1, b1); \
		ICONV_CLAMP1(Y1, Cr1, Cb1); \
		ICONV_PUT_YCrCb(output, Y1, Cr1, Cb1); \
	}

	#define iconv_rgb_to_YCrCb_x2 { \
		ICONV_GET_RGB(input, r1, g1, b1); \
		ICONV_GET_RGB(input, r2, g2, b2); \
		Y1 = ICONV_RGB_TO_Y(r1, g1, b1); \
		Cr1 = ICONV_RGB_TO_Cr(r1, g1, b1); \
		Cb1 = ICONV_RGB_TO_Cb(r1, g1, b1); \
		Y2 = ICONV_RGB_TO_Y(r2, g2, b2); \
		Cr2 = ICONV_RGB_TO_Cr(r2, g2, b2); \
		Cb2 = ICONV_RGB_TO_Cb(r2, g2, b2); \
		ICONV_CLAMP2(Y1, Cr1, Cb1, Y2, Cr2, Cb2); \
		ICONV_PUT_YCrCb(output, Y1, Cr1, Cb1); \
		ICONV_PUT_YCrCb(output, Y2, Cr2, Cb2); \
	}

	ILINS_LOOP_DOUBLE (
			iconv_rgb_to_YCrCb_x1,
			iconv_rgb_to_YCrCb_x2,
			size
		);

	#undef iconv_rgb_to_YCrCb_x1
	#undef iconv_rgb_to_YCrCb_x2
}

void iconv_YCrCb_to_RGB(const IYCrCb *input, long size, IRGB *output)
{
	int r1, g1, b1, r2, g2, b2;
	int Y1, Cr1, Cb1, Y2, Cr2, Cb2;

	#define iconv_YCrCb_to_RGB_x1 { \
		ICONV_GET_YCrCb(input, Y1, Cr1, Cb1); \
		r1 = ICONV_YCrCb_TO_R(Y1, Cr1, Cb1); \
		g1 = ICONV_YCrCb_TO_G(Y1, Cr1, Cb1); \
		b1 = ICONV_YCrCb_TO_B(Y1, Cr1, Cb1); \
		ICONV_CLAMP1(r1, g1, b1); \
		ICONV_PUT_RGB(output, r1, g1, b1); \
	}

	#define iconv_YCrCb_to_RGB_x2 { \
		ICONV_GET_YCrCb(input, Y1, Cr1, Cb1); \
		ICONV_GET_YCrCb(input, Y2, Cr2, Cb2); \
		r1 = ICONV_YCrCb_TO_R(Y1, Cr1, Cb1); \
		g1 = ICONV_YCrCb_TO_G(Y1, Cr1, Cb1); \
		b1 = ICONV_YCrCb_TO_B(Y1, Cr1, Cb1); \
		r2 = ICONV_YCrCb_TO_R(Y2, Cr2, Cb2); \
		g2 = ICONV_YCrCb_TO_G(Y2, Cr2, Cb2); \
		b2 = ICONV_YCrCb_TO_B(Y2, Cr2, Cb2); \
		ICONV_CLAMP2(r1, g1, b1, r2, g2, b2); \
		ICONV_PUT_RGB(output, r1, g1, b1); \
		ICONV_PUT_RGB(output, r2, g2, b2); \
	}

	ILINS_LOOP_DOUBLE (
			iconv_YCrCb_to_RGB_x1,
			iconv_YCrCb_to_RGB_x2,
			size
		);

	#undef iconv_YCrCb_to_RGB_x1
	#undef iconv_YCrCb_to_RGB_x2
}

void iconv_RGB_to_HSV(const IRGB *input, long size, IHSV *output)
{
	float min, max, delta, h, s, v;
	float r, g, b;
	for (; size > 0; size--, input++, output++) {
		r = input->r * (1.0f / 255.0f);
		g = input->g * (1.0f / 255.0f);
		b = input->b * (1.0f / 255.0f);
		min = ICONV_MIN(r, ICONV_MIN(g, b));
		max = ICONV_MAX(r, ICONV_MAX(g, b));
		v = max;
		delta = max - min;
		if (max == 0.0f) {
			s = 0.0f;
			h = 0.0f;
		}	else {
			s = delta / max;
			if (r == max) h = (g - b) / delta;
			else if (g == max) h = 2.0f + (b - r) / delta;
			else h = 4.0f + (r - g) / delta;
			h *= 60.0f;
			if (h < 0.0f) h += 360.0f;
		}
		output->H = ICONV_MID(0.0f, h, 359.0f);
		output->S = ICONV_MID(0.0f, s, 1.0f);
		output->V = ICONV_MID(0.0f, v, 1.0f);
	}
}

void iconv_HSV_to_RGB(const IHSV *input, long size, IRGB *output)
{
	float r, g, b, h, s, v, f, p, q, t;
	int i;
	for (; size > 0; size--, input++, output++) {
		h = input->H;
		s = input->S;
		v = input->V;
		if (s == 0.0f) {
			r = g = b = 0.0f;
		}	else {
			h /= 60.0f;
			i = (int)h;
			f = h - i;
			p = v * (1.0f - s);
			q = v * (1.0f - s * f);
			t = v * (1.0f - s * (1.0f - f));
			switch (i)
			{
			case 0: r = v; g = t; b = p; break;
			case 1: r = q; g = v; b = p; break;
			case 2: r = p; g = v; b = t; break;
			case 3: r = p; g = q; b = v; break;
			case 4: r = t; g = p; b = v; break;
			case 5: r = v; g = p; b = q; break;
			case 6: r = v; g = t; b = p; break;
			default:
				r = g = b = 0;
				printf("i=%d\n", i);
				assert(i < 6);
				break;
			}
		}
		r = ICONV_MID(0.0f, r, 1.0f);
		g = ICONV_MID(0.0f, g, 1.0f);
		b = ICONV_MID(0.0f, b, 1.0f);
		output->r = (unsigned char)(r * 255.0f);
		output->g = (unsigned char)(g * 255.0f);
		output->b = (unsigned char)(b * 255.0f);
	}
}



