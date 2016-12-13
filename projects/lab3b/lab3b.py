import sys

class Superblock:
	def __init__(self, arg_arr):
		# self.magic = arg_arr[0]
		self.num_inodes = int(arg_arr[1])
		self.num_blocks = int(arg_arr[2])
		self.block_size = int(arg_arr[3])
		self.frag_size = int(arg_arr[4])
		self.blocks_pg = int(arg_arr[5])
		self.inodes_pg = int(arg_arr[6])
		self.frags_pg = int(arg_arr[7])
		# self.first_block = int(arg_arr[8][:-1])

class GroupDescriptor:
	def __init__(self, arg_arr):
		self.contained_blocks = int(arg_arr[0])
		self.free_blocks = int(arg_arr[1])
		self.free_inodes = int(arg_arr[2])
		self.num_dir = int(arg_arr[3])
		self.inode_bitmap_block = int(arg_arr[4], 16)
		self.block_bitmap_block = int(arg_arr[5], 16)
		self.inode_table_start_block = int(arg_arr[6], 16)

class BitmapEntry:
	def __init__(self, arg_arr):
		self.block_num = int(arg_arr[0], 16)
		self.free_bi_num = int(arg_arr[1])
		if self.block_num in [gds.block_bitmap_block for gds in gd]:
			free_blocks.append(self.free_bi_num)

class Indirect:
	def __init__(self, arg_arr):
		self.containing_block_num = int(arg_arr[0], 16)
		self.entry_num_within_block = int(arg_arr[1])
		self.block_ptr_val = int(arg_arr[2], 16)

class Inode:
	def __init__(self, arg_arr):
		self.inode_num = int(arg_arr[0])
		# self.group = arg_arr[4]
		self.link_count = int(arg_arr[5])
		self.actual_count = 0
		self.num_blocks = int(arg_arr[10])
		self.blocks = [int(b, 16) for b in arg_arr[11:26]]
		self.add_blocks()
		self.check_unallocated_blocks();

	def check_unallocated_blocks(self):
		for i in range(0, min(12, self.num_blocks)):
			if self.blocks[i] == 0:
				print "INVALID BLOCK <", self.blocks[i], "> IN INODE <", self.inode_num, "> ENTRY <", i, ">"

	def check_link_count(self):
		if self.link_count != self.actual_count:
			print "LINKCOUNT <", self.inode_num, "> IS <", self.link_count, "> SHOULD BE <", self.actual_count, ">"

	def add_blocks(self):
		for i in range(15):
			if self.blocks[i] == 0: 
				continue
			Block(self.blocks[i], self.inode_num, i)
			if self.blocks[i] in unallocated_blocks:
				unallocated_blocks.remove(self.blocks[i])

class Block:
	def __init__(self, block_num, inode_num, inode_entry):
		found_block = [ab for ab in all_blocks if ab.block_num == block_num]
		if not found_block: 
			self.block_num = block_num
			self.inode_num = []
			self.inode_entry = []
			self.add_pair(inode_num, inode_entry)
			all_blocks.append(self)
		else: 
			[fb.add_pair(inode_num, inode_entry) for fb in found_block]

	def add_pair(self, inode_num, inode_entry):
		self.inode_num.append(inode_num)
		self.inode_entry.append(inode_entry)

	def check_block(self):
		if len(self.inode_num) > 1:
			out_string = "MULTIPLY REFERENCED BLOCK < " + str(self.block_num)
			for i in range(0,len(self.inode_num)):
				if i == 0:
					out_string += " > BY INODE < " + str(self.inode_num[i]) + " > ENTRY < " + str(self.inode_entry[i])
				else:
					out_string += " > INODE < " + str(self.inode_num[i]) + " > ENTRY < " + str(self.inode_entry[i])
			out_string += " >"
			print out_string

class Directory:
	def __init__(self, arg_arr):
		self.parent_inode_num = int(arg_arr[0])
		self.entry_num = int(arg_arr[1])
		# self.entry_length = arg_arr[2]
		# self.name_length = arg_arr[3]
		self.inode_number_file = int(arg_arr[4])
		self.inc_link_count()
		self.remove_from_unused()
		self.name = arg_arr[5][1:-1]
		self.add_to_parents()
		self.check_directory()

	def check_directory(self):
		if self.name == '.' and self.parent_inode_num != self.inode_number_file:
			print "INCORRECT ENTRY IN <", self.parent_inode_num, "> NAME <", self.name, "> LINK TO <", self.inode_number_file, "> SHOULD BE <", self.parent_inode_num, ">"

	def check_directory_2(self):
		if self.name == '..':
			if self.parent_inode_num in parents and self.inode_number_file != parents[self.parent_inode_num]:
				print "INCORRECT ENTRY IN <", self.parent_inode_num, "> NAME <", self.name, "> LINK TO <", self.inode_number_file,"> SHOULD BE <", parents[self.parent_inode_num], ">"

	def add_to_parents(self):
		if self.name != '.' and self.name != '..':
			parents[self.inode_number_file] = self.parent_inode_num

	def remove_from_unused(self):
		if self.inode_number_file in unused_inodes:
			unused_inodes.remove(self.inode_number_file)

	def inc_link_count(self):
		self_inode = [i for i in inodes if i.inode_num == self.inode_number_file]
		if not self_inode: 
			print "UNALLOCATED INODE <", self.inode_number_file, "> REFERENCED BY DIRECTORY <", self.parent_inode_num, "> ENTRY <", self.entry_num, ">"
		for x in self_inode:
			x.actual_count += 1

def main(argv):
	super_csv = open('super.csv', 'rb')
	sb = Superblock(super_csv.read().split(','))
	super_csv.close()

	global gd
	gd = []
	group_csv = open('group.csv', 'rb')
	[gd.append(GroupDescriptor(line[:-1].split(','))) for line in group_csv]
	group_csv.close()

	global free_blocks
	free_blocks = []
	free_bm = []
	bitmap_csv = open('bitmap.csv', 'rb')
	[free_bm.append(BitmapEntry(line[:-1].split(','))) for line in bitmap_csv]
	bitmap_csv.close() 

	global indir 
	indir = []
	indir_csv = open('indirect.csv', 'rb')
	[indir.append(Indirect(line[:-1].split(','))) for line in indir_csv]
	indir_csv.close()

	global inodes, unused_inodes, all_blocks, unallocated_blocks
	inodes = []
	unused_inodes = range(11, sb.num_inodes+1)
	unallocated_blocks = range(0, sb.num_blocks)
	all_blocks = []
	inode_csv = open('inode.csv','rb')
	[inodes.append(Inode(line[:-1].split(','))) for line in inode_csv]
	inode_csv.close()

	global dirs, parents
	dirs = []
	parents = {}
	dir_csv = open('directory.csv','rb')
	[dirs.append(Directory(line[:-1].split(','))) for line in dir_csv]

	[d.check_directory_2() for d in dirs]

	free = [free_bi.free_bi_num for free_bi in free_bm]
	for x in [i for i in unused_inodes if i not in free]:
		print "MISSING INODE <", x, "> SHOULD BE IN FREE LIST <", gd[x/sb.inodes_pg].inode_bitmap_block, ">"

	[i.check_link_count() for i in inodes]

	[b.check_block() for b in all_blocks]

	for bl in [b for b in all_blocks if b.block_num in free_blocks]:
		if bl.block_num not in unallocated_blocks:
			print "UNALLOCATED BLOCK <", bl.block_num, "> REFERENCED BY INODE <", bl.inode_num[0], "> ENTRY <", bl.inode_entry[0], ">"

if __name__ == '__main__' :
	main(sys.argv[1:])
