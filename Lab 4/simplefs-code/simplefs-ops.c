#include "simplefs-ops.h"

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES]; // Array for storing opened files

int simplefs_create(char *filename)
{
	/*
	    Create file with name `filename` from disk
	*/

	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));

	for (int i = 0; i < NUM_INODES; ++i) // check if same filename exists return -1
	{
		simplefs_readInode(i, inode);
		if (strcmp(inode->name, filename) == 0)
			return ERROR;
	}

	int inode_num = simplefs_allocInode();
	if (inode_num == DOES_NOT_EXIST) // In case of error (no inode available) returns -1
		return ERROR;

	// create this inode
	memcpy(inode->name, filename, sizeof(filename));

	inode->status = INODE_IN_USE;
	inode->file_size = 0;
	for (int i = 0; i < MAX_FILE_SIZE; i++) // MAX_FILE_SIZE = 4
		inode->direct_blocks[i] = -1;

	simplefs_writeInode(inode_num, inode);
	free(inode);

	return inode_num;
}

void simplefs_delete(char *filename)
{
	/*
	    delete file with name `filename` from disk
	*/
	int inode_num = -1;
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));

	for (int i = 0; i < NUM_INODES; ++i) // check if filename exists
	{
		simplefs_readInode(i, inode);
		if (strcmp(inode->name, filename) == 0)
		{
			inode_num = i;
			break;
		}
	}

	if (inode_num == DOES_NOT_EXIST) // if file does not exist
		return;

	for (int i = 0; i < MAX_FILE_SIZE; i++)
	{
		if (inode->direct_blocks[i] != DOES_NOT_EXIST)
			simplefs_freeDataBlock(inode->direct_blocks[i]);
	}

	simplefs_freeInode(inode_num);
	free(inode);
}

int simplefs_open(char *filename)
{
	/*
	    open file with name `filename`
	*/
	int inode_num = -1;
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));

	for (int i = 0; i < NUM_INODES; ++i) //check if filename exists return -1
	{
		simplefs_readInode(i, inode);
		if (strcmp(inode->name, filename) == 0)
		{
			inode_num = i;
			break;
		}
	}

	if (inode_num == DOES_NOT_EXIST) // does not exist
		return ERROR;

	int fd = -1;
	for (int i = 0; i < MAX_OPEN_FILES; i++)
	{
		if (file_handle_array[i].inode_number == -1) // unused
		{
			file_handle_array[i].offset = inode->file_size;
			file_handle_array[i].inode_number = inode_num;
			fd = i;
			break;
		}
	}
	free(inode);

	return fd;
}

void simplefs_close(int file_handle)
{
	/*
	    close file pointed by `file_handle`
	*/
	assert(file_handle < MAX_OPEN_FILES);
	file_handle_array[file_handle].offset = 0;
	file_handle_array[file_handle].inode_number = -1;
}

int simplefs_read(int file_handle, char *buf, int nbytes)
{
	/*
	    read `nbytes` of data into `buf` from file pointed by `file_handle` starting at current offset
	*/
	assert(file_handle < MAX_OPEN_FILES);
	int block_id = file_handle_array[file_handle].offset / BLOCKSIZE;
	int final_block_id = (file_handle_array[file_handle].offset + nbytes - 1) / BLOCKSIZE;

	if (final_block_id >= MAX_FILE_SIZE)
		return ERROR;

	// find inode here for direct_blocks
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	simplefs_readInode(file_handle_array[file_handle].inode_number, inode);

	if ((inode->file_size - sizeof(char) * file_handle_array[file_handle].offset - sizeof(char) * nbytes) < 0)
		return ERROR;

	if (inode->direct_blocks[final_block_id] == DOES_NOT_EXIST) // unallocated data block
		return ERROR;

	int offset = file_handle_array[file_handle].offset;
	char *buffer = (char *)malloc(sizeof(char) * (nbytes));

	for (int i = block_id; i <= final_block_id; ++i)
	{
		char *tempBufferBlock = (char *)malloc(sizeof(char) * BLOCKSIZE);
		simplefs_readDataBlock(inode->direct_blocks[i], tempBufferBlock);

		if (i == block_id)
		{
			int size;
			nbytes < (BLOCKSIZE - offset % BLOCKSIZE + 1)
				? (size = nbytes)
				: (size = BLOCKSIZE - offset % BLOCKSIZE + 1);

			strncpy(buffer, tempBufferBlock + (offset % 64), size);
		}
		else if (i == final_block_id)
		{
			int size;
			(nbytes - offset) % BLOCKSIZE == 0 ? (size = BLOCKSIZE) : (size = (nbytes - offset) % BLOCKSIZE);
			strncat(buffer, tempBufferBlock, size);
		}
		else
		{
			strcat(buffer, tempBufferBlock);
		}
	}

	memcpy(buf, buffer, nbytes);

	free(inode);
	free(buffer);

	return SUCCESS;
}

int simplefs_write(int file_handle, char *buf, int nbytes)
{
	/*
	    write `nbytes` of data from `buf` to file pointed by `file_handle` starting at current offset
	*/
	char *buff = buf;
	assert(file_handle < MAX_OPEN_FILES);
	int block_id = file_handle_array[file_handle].offset / BLOCKSIZE;
	int final_block_id = (file_handle_array[file_handle].offset - 1 + nbytes) / BLOCKSIZE;

	if (final_block_id >= MAX_FILE_SIZE)
		return ERROR;

	//find inode here for direct_blocks
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	simplefs_readInode(file_handle_array[file_handle].inode_number, inode);

	int offset = file_handle_array[file_handle].offset;

	if (MAX_FILE_SIZE * BLOCKSIZE < inode->file_size + sizeof(buf) / sizeof(char))
	{
		return ERROR;
	}

	for (int i = block_id; i <= final_block_id; i++)
	{
		if (inode->direct_blocks[i] == DOES_NOT_EXIST)
			inode->direct_blocks[i] = simplefs_allocDataBlock();

		if (inode->direct_blocks[i] == DOES_NOT_EXIST)
			return ERROR;
	}
	for (int i = block_id; i <= final_block_id; i++)
	{
		if (i == block_id && offset % BLOCKSIZE != 0)
		{
			char *tempBufferBlock = (char *)malloc(sizeof(char) * BLOCKSIZE);
			simplefs_readDataBlock(inode->direct_blocks[i], tempBufferBlock);
			char *buffer = (char *)malloc(sizeof(char) * (BLOCKSIZE));
			strncat(buffer, tempBufferBlock, offset % BLOCKSIZE);
			strncat(buffer, buff, BLOCKSIZE - (offset % BLOCKSIZE));

			buff += sizeof(char) * (BLOCKSIZE - offset % BLOCKSIZE);

			simplefs_writeDataBlock(inode->direct_blocks[block_id], buffer);
			free(buffer);
		}
		else if (i == final_block_id)
		{
			char *buffer = (char *)malloc(sizeof(char) * BLOCKSIZE);
			strcat(buffer, buff);
			simplefs_writeDataBlock(inode->direct_blocks[i], buffer);
			free(buffer);
		}
		else
		{
			char *buffer = (char *)malloc(sizeof(char) * BLOCKSIZE);
			strncat(buffer, buff, BLOCKSIZE);
			buff += sizeof(char) * BLOCKSIZE;
			simplefs_writeDataBlock(inode->direct_blocks[i], buffer);
			free(buffer);
		}
	}

	inode->file_size += nbytes;
	simplefs_writeInode(file_handle_array[file_handle].inode_number, inode);
	free(inode);
	return SUCCESS;
}

int simplefs_seek(int file_handle, int nseek)
{
	/*
	   increase `file_handle` offset by `nseek`
	*/

	if (file_handle_array[file_handle].offset + nseek < 0 ||
		file_handle_array[file_handle].offset + nseek >= MAX_FILE_SIZE * BLOCKSIZE)

		return ERROR;

	file_handle_array[file_handle].offset += nseek;

	return SUCCESS;
}