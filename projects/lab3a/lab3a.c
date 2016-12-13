#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

#define SUPERBLOCK_SIZE 1024

int disk_fd, super_fd, group_fd, bitmap_fd, inode_fd, directory_fd, indirect_fd;
int num_desc, desc_size;
int dir_num;
struct superblock *sb;
struct group_desc *gd;
struct ext2_inode *inode_arr;
int *dir_inodes;
char *entry_name;
int *indir_arr, *double_arr, *triple_arr;

struct superblock {
	int32_t inodes_count;
	int32_t blocks_count;
	int32_t r_blocks_count;
	int32_t free_blocks_count;
	int32_t free_inodes_count;
	int32_t first_data_block;
	int32_t log_block_size;
	int32_t log_frag_size;
	int32_t blocks_per_group;
	int32_t frags_per_group;
	int32_t inodes_per_group;
	int32_t mtime;
	int32_t wtime;
	uint16_t mnt_count;
	uint16_t max_mnt_count;
	uint16_t magic;
};

struct group_desc {
	int32_t block_bitmap;
	int32_t inode_bitmap;
	int32_t inode_table;
	int16_t free_blocks_count;
	int16_t free_inodes_count;
	int16_t used_dirs_count;
	int16_t padding;
	char reserved[12];
};

struct ext2_inode{
	uint16_t mode;
	uint16_t uid;
	uint32_t size;
	uint32_t atime;
	uint32_t ctime;
	uint32_t mtime;
	uint32_t dtime;
	uint16_t gid;
	uint16_t links_count;
	uint32_t blocks;
	uint32_t flags;	
	uint32_t osd1;
	uint32_t block[15];
	uint32_t generation;
	uint32_t file_acl;
	uint32_t dir_acl;
	uint32_t faddr;
	uint32_t osd2[3];
} inode;	
	
struct ext2_dir{
	uint32_t inode;
	uint16_t rec_len;
	uint8_t name_len;
	uint8_t file_type;
} dir;

int bit_check(char input, int i) {
	if(i > 7) 
		return 0;
	return ((input) & (1 << (i)));
}

void parse_superblock() {
	super_fd = creat("super.csv", 0666);
	sb = malloc(sizeof(struct superblock));
	pread(disk_fd, sb, sizeof(struct superblock), SUPERBLOCK_SIZE);
	sb->log_block_size = SUPERBLOCK_SIZE << sb->log_block_size;
	sb->log_frag_size = SUPERBLOCK_SIZE << sb->log_frag_size;
	if(!((sb->log_block_size & (~sb->log_block_size + 1)) == sb->log_block_size)) {
		fprintf(stderr, "Error: Superblock - invalid block size: %d\n", sb->log_block_size);
		exit(EXIT_FAILURE);
	}
	if(sb->log_block_size < 512 || sb->log_block_size > 65536){
		fprintf(stderr, "Error: Superblock - block size not within 512-64K: %d\n", sb->log_block_size);
		exit(EXIT_FAILURE);
	}
	dprintf(super_fd, "%x,%d,%d,%d,%d,%d,%d,%d,%d\n", sb->magic, sb->inodes_count, sb->blocks_count, 
		sb->log_block_size, sb->log_frag_size, sb->blocks_per_group, sb->inodes_per_group, sb->frags_per_group, sb->first_data_block);
}

void parse_group_desc() {
	group_fd = creat("group.csv", 0666);
	int offset;
	if(sb->first_data_block == 1)
		offset = sb->log_block_size * 2;
	else
		offset = sb->log_block_size;
	num_desc = sb->blocks_count / sb->blocks_per_group;
	desc_size = sizeof(struct group_desc);
	gd = malloc(num_desc * desc_size);

	for(int i = 0; i < num_desc; i++) {
		pread(disk_fd, &(gd[i]), desc_size, offset + i * desc_size);
		dprintf(group_fd, "%d,%d,%d,%d,%x,%x,%x\n", sb->blocks_per_group, gd[i].free_blocks_count, gd[i].free_inodes_count, 
			gd[i].used_dirs_count, gd[i].inode_bitmap, gd[i].block_bitmap, gd[i].inode_table);
	}
}

void parse_bitmap_inode() {
	bitmap_fd = creat("bitmap.csv", 0666);
	inode_fd = creat("inode.csv", 0666);
	int block_offset, inode_b_offset, inode_t_offset;
	uint8_t b_buf, i_buf;
	int num_inodes = 0, num_blocks = 0;
	inode_arr = calloc(sb->inodes_count, sizeof(struct ext2_inode));
	dir_num = 0;
	dir_inodes = malloc(1);

	for(int i = 0; i < num_desc; i++){
		block_offset = sb->log_block_size * gd[i].block_bitmap;
		inode_b_offset = sb->log_block_size * gd[i].inode_bitmap;
		inode_t_offset  = sb->log_block_size * gd[i].inode_table;
		for(int j = 0; j < sb->log_block_size; j++){
			pread(disk_fd, &b_buf, 1, block_offset + j);
			pread(disk_fd, &i_buf, 1, inode_b_offset + j);
			for(int k = 0; k < 8; k++){
				num_blocks++;
				if(!bit_check(b_buf, k))
					dprintf(bitmap_fd, "%x,%d\n", gd[i].block_bitmap, num_blocks);
				if((j * 8 + k) >= sb->inodes_per_group)
					continue;
				num_inodes++;
				if(!bit_check(i_buf, k)) {
					int i_pos = j * 8 + k;
					if(i_pos <= sb->inodes_per_group)
						dprintf(bitmap_fd, "%x,%d\n", gd[i].inode_bitmap, num_inodes);
				}
				else {
					char type = '?';
					int i_offset = (j * 8 + k) * sizeof(struct ext2_inode) + inode_t_offset;
					pread(disk_fd, &inode_arr[num_inodes], sizeof(struct ext2_inode), i_offset);
					inode = inode_arr[num_inodes];
					uint16_t mode = inode.mode;
					if(mode & 0xA000)
						type = 's';
					if(mode & 0x8000)
						type = 'f';
					if(mode & 0x4000){
						type = 'd';
						dir_num++;
						dir_inodes = realloc(dir_inodes, dir_num * sizeof(int));
						dir_inodes[dir_num - 1] = num_inodes;
					}
					dprintf(inode_fd, "%d,%c,%o,%d,%d,%d,%x,%x,%x,%d,%d", num_inodes, type, inode.mode, inode.uid, inode.gid, inode.links_count,
																		inode.ctime, inode.mtime, inode.atime, inode.size, inode.blocks/(2<<sb->log_block_size));
					for(int l = 0; l < 15; l++)
						dprintf(inode_fd, ",%x", inode.block[l]);
					dprintf(inode_fd, "\n");
				}
			}
		}
	}	
}

void parse_dir_indirect() {
	directory_fd = creat("directory.csv", 0666);
	indirect_fd = creat("indirect.csv", 0666);
	indir_arr = malloc(sb->log_block_size);
	double_arr = malloc(sb->log_block_size);
	triple_arr = malloc(sb->log_block_size);
	
	for(int i = 0; i < dir_num; i++) {	 
		int parent_inode = dir_inodes[i];
		struct ext2_inode curr = inode_arr[parent_inode];
		int entry_num = 0;
		for(int j = 0; j < 12; j++) { 
			if(curr.block[j] == 0)
				break;
			int block_offset = curr.block[j] * sb->log_block_size; 
			int cur_offset = 0;
			while(cur_offset < sb->log_block_size) {	
				pread(disk_fd, &dir, sizeof(struct ext2_dir), block_offset + cur_offset);
				entry_name = malloc(dir.name_len + 1);
				pread(disk_fd, entry_name, dir.name_len, block_offset + cur_offset + sizeof(struct ext2_dir));
				entry_name[dir.name_len] = '\0';
				if(dir.name_len > 0 && dir.inode > 0)
					dprintf(directory_fd, "%d,%d,%d,%d,%d,\"%s\"\n", parent_inode, entry_num, dir.rec_len, dir.name_len, dir.inode, entry_name);
				free(entry_name);
				cur_offset += dir.rec_len;
				entry_num++;
			}
		}	
		if(curr.block[12] > 0) {			
			pread(disk_fd, indir_arr, sb->log_block_size, curr.block[12] * sb->log_block_size);
			for(int j = 0; j < sb->log_block_size/4; j++) {
				if(indir_arr[j] == 0)
					break;
				dprintf(indirect_fd, "%x,%d,%x\n", curr.block[12], j, indir_arr[j]);
				int block_offset = indir_arr[j] * sb->log_block_size; 
				int cur_offset = 0;
				while(cur_offset < sb->log_block_size){ 
					pread(disk_fd, &dir, sizeof(struct ext2_dir), block_offset + cur_offset);
					entry_name = malloc(dir.name_len + 1);
					pread(disk_fd, entry_name, dir.name_len, block_offset + cur_offset + sizeof(struct ext2_dir));
					entry_name[dir.name_len] = '\0';
					if(dir.name_len > 0 && dir.inode > 0)
						dprintf(directory_fd, "%d,%d,%d,%d,%d,\"%s\"\n", parent_inode, entry_num, dir.rec_len, dir.name_len, dir.inode, entry_name);
					free(entry_name);
					cur_offset += dir.rec_len;
					entry_num++;							
				}
			}
		}
		if(curr.block[13] > 0) {
			pread(disk_fd, double_arr, sb->log_block_size, curr.block[13] * sb->log_block_size);
			for(int j = 0; j < sb->log_block_size/4; j++){
				pread(disk_fd, indir_arr, sb->log_block_size, double_arr[j] * sb->log_block_size);
				for(int k = 0; k < sb->log_block_size/4; k++){
					if(indir_arr[k] == 0)
						break;
					dprintf(indirect_fd, "%x,%d,%x\n", double_arr[j], k, indir_arr[k]);
					int block_offset = indir_arr[k] * sb->log_block_size;
					int cur_offset = 0;
					while(cur_offset < sb->log_block_size) {
						pread(disk_fd, &dir, sizeof(struct ext2_dir), block_offset + cur_offset);
						entry_name = malloc(dir.name_len + 1);
						pread(disk_fd, entry_name, dir.name_len, block_offset + cur_offset + sizeof(struct ext2_dir));
						entry_name[dir.name_len] = '\0';
						if(dir.name_len > 0 && dir.inode > 0)
							dprintf(directory_fd, "%d,%d,%d,%d,%d,\"%s\"\n", parent_inode, entry_num, dir.rec_len, dir.name_len, dir.inode, entry_name);
						free(entry_name);
						cur_offset += dir.rec_len;
						entry_num++;	
					}						
				}
			}
		}
		if(curr.block[14] > 0) {
			pread(disk_fd, triple_arr, sb->log_block_size, curr.block[14] * sb->log_block_size);
			for(int j = 0; j < sb->log_block_size/4; j++){
				pread(disk_fd, double_arr, sb->log_block_size, triple_arr[j] * sb->log_block_size);
				for(int k = 0; k < sb->log_block_size/4; k++){
					pread(disk_fd, indir_arr, sb->log_block_size, double_arr[k] * sb->log_block_size);
					for(int l = 0; l < sb->log_block_size/4; l++) {
						if(indir_arr[l] == 0)
							break;
						dprintf(indirect_fd, "%x,%d,%x\n", triple_arr[k], l, indir_arr[l]);
						int block_offset = indir_arr[l] * sb->log_block_size; 
						int cur_offset = 0;
						while(cur_offset < sb->log_block_size) {
							pread(disk_fd, &dir, sizeof(struct ext2_dir), block_offset + cur_offset);
							entry_name = malloc(dir.name_len + 1);
							pread(disk_fd, entry_name, dir.name_len, block_offset + cur_offset + sizeof(struct ext2_dir));
							entry_name[dir.name_len] = '\0';
							if(dir.name_len > 0 && dir.inode > 0)
								dprintf(directory_fd, "%d,%d,%d,%d,%d,\"%s\"\n", parent_inode, entry_num, dir.rec_len, dir.name_len, dir.inode, entry_name);
							free(entry_name);
							cur_offset += dir.rec_len;
							entry_num++;	
						}							
					}
				}
			}
		}
	}
}

int main(int argc, char * argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if((disk_fd = open(argv[1], O_RDONLY)) == -1) {
		fprintf(stderr, "Error: unable to open file\n");
		exit(EXIT_FAILURE);
	}
	parse_superblock();
	parse_group_desc();
	parse_bitmap_inode();
	parse_dir_indirect();
	
	free(triple_arr);
	free(double_arr);
	free(indir_arr);
	free(dir_inodes);
	free(inode_arr);
	free(gd);
	free(sb);
}	
