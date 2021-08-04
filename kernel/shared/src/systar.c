//systar.c
//Initial contents of RAMdisk as TAR image
//Bryan E. Topp <betopp@betopp.com> 2021

#include "systar.h"
#include "kassert.h"
#include "kspace.h"
#include "libcstubs.h"
#include "hal_bootfile.h"

#include <sys/stat.h>

//USTAR header block
typedef struct systar_ustar_hdr_s
{
	char filename[100];
	char mode_oct[8];
	char uid_oct[8];
	char gid_oct[8];
	char size_oct[12];
	char mtime_oct[12];
	char checksum_oct[8];
	char type;
	char linked[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmaj[8];
	char devmin[8];
	char prefix[155];
	
} systar_ustar_hdr_t;

//Parses an octal string from a USTAR header
uint64_t systar_ustar_octstr(const char *str, size_t len)
{	
	uint64_t val = 0;
	for(size_t ss = 0; ss < len; ss++)
	{
		KASSERT(str[ss] == ' ' || str[ss] == '\0' || (str[ss] >= '0' && str[ss] <= '7'));
		if(str[ss] >= '0' && str[ss] <= '7')
		{
			val *= 8;
			val += str[ss] - '0';
		}
	}
	return val;
}

void systar_unpack_file(int fnum)
{
	//Map the file into kernel space
	hal_frame_id_t tar_addr = hal_bootfile_addr(fnum);
	size_t tar_size = hal_bootfile_size(fnum);
	
	uint8_t *tar_bytes = kspace_phys_map(tar_addr, tar_size);
	KASSERT(tar_bytes != NULL);
	
	//Work through the TAR file
	const uint8_t *block_bytes = tar_bytes;
	int zero_blocks = 0;
	while(block_bytes < tar_bytes + tar_size)
	{
		//Read block header and advance past
		const systar_ustar_hdr_t *hdr = (systar_ustar_hdr_t*)(block_bytes);
		block_bytes += 512;
		
		//Make sure it's a USTAR header or empty - two empty blocks mean end-of-file
		if(hdr->filename[0] == '\0')
		{
			zero_blocks++;
			if(zero_blocks == 2)
				break;
			else
				continue;
		}
		
		KASSERT(memcmp(hdr->magic, "ustar\0", 6) == 0);
		KASSERT(memcmp(hdr->version, "00", 2) == 0);
		ssize_t file_size = systar_ustar_octstr(hdr->size_oct, sizeof(hdr->size_oct));
		
		//Based on the header, figure out the mode of the file we'll make
		mode_t mode = systar_ustar_octstr(hdr->mode_oct, sizeof(hdr->mode_oct)) & 0777;
		switch(hdr->type)
		{
			case '0':
			case '\0':
				mode |= S_IFREG;
				break;
			case '3':
				mode |= S_IFCHR;
				break;
			case '4':
				mode |= S_IFBLK;
				break;
			case '5':
				mode |= S_IFDIR;
				break;
			default:
				KASSERT(0);
		}
		
		//Read device numbers, if specified.
		uint64_t spec = systar_ustar_octstr(hdr->devmaj, sizeof(hdr->devmaj)) & 0xFFFF;
		spec <<= 16;
		spec |= systar_ustar_octstr(hdr->devmin, sizeof(hdr->devmin)) & 0xFFFF;
		
		//Make and open the file...
		const char *path_remain = hdr->filename;
		
		//Skip leading slashes and dots in the path. We're extracting to the root anyway.
		while( (*path_remain == '/') || (*path_remain == '.') )
		{
			path_remain++;
		}
		
		//Parse each non-final pathname component. Make sure the directories exist. Change into them, starting from root.
		extern int k_px_fd_find();
		int dir_fd = k_px_fd_find(-1, "/");
		KASSERT(dir_fd >= 0);
		while(1)
		{
			const char *slash = strchr(path_remain, '/');
			if(slash == NULL)
			{
				//No further slashes in the pathname.
				break;
			}
			
			//There's a slash. We need to look up the pathname component before it. Copy it out by itself.
			char dirname[100] = {0};
			for(size_t dd = 0; dd < sizeof(dirname) - 1; dd++)
			{
				if(path_remain[dd] == '/')
					break;
				
				dirname[dd] = path_remain[dd];
			}
			
			//Try to look up that directory
			int next_dir_fd = k_px_fd_find(dir_fd, dirname);
			
			//Make it, if it doesn't exist
			if(next_dir_fd < 0)
			{
				extern int k_px_fd_create();
				next_dir_fd = k_px_fd_create(dir_fd, dirname, S_IFDIR | 0755, 0);
			}
			
			KASSERT(next_dir_fd >= 0);
			
			//Close the previous one and advance
			extern int k_px_fd_close();
			k_px_fd_close(dir_fd);
			dir_fd = next_dir_fd;
			
			path_remain = slash + 1;
		}
		
		//Alright, we're in the directory we want. Make the file there.
		KASSERT(strchr(path_remain, '/') == NULL);
		
		//Directories in TAR files can end with a "/", in which case, we're already done.
		if(strlen(path_remain) > 0)
		{
			extern int k_px_fd_create();
			int file_fd = k_px_fd_create(dir_fd, path_remain, mode, spec);
			KASSERT(file_fd >= 0);
			
			//Write the contents into the file
			if(S_ISREG(mode))
			{
				extern int k_px_fd_write();
				ssize_t written = k_px_fd_write(file_fd, block_bytes, file_size);
				KASSERT(written == file_size);
			}
			
			//Close the file
			extern int k_px_fd_close();
			k_px_fd_close(file_fd);
		}
		
		//Close the directory where we made the file
		extern int k_px_fd_close();
		k_px_fd_close(dir_fd);
		
		//Advance past the file contents in the TAR
		size_t file_blocks = (file_size + 511) / 512;
		block_bytes += file_blocks * 512;
	}
	
	//Unmap and free the file as loaded
	kspace_phys_unmap(tar_bytes, tar_size);
	for(hal_frame_id_t ff = tar_addr; ff + hal_frame_size() - 1 < tar_addr + tar_size; ff += hal_frame_size())
	{
		hal_frame_free(ff);
	}
}

void systar_unpack(void)
{
	int nfiles = hal_bootfile_count();
	for(int ff = 0; ff < nfiles; ff++)
	{
		systar_unpack_file(ff);
	}
}
