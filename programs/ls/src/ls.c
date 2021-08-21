//ls.c
//File listing
//Bryan E. Topp <betopp@betopp.com> 2021

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

#include <pcmd.h>
bool cmd_list;
bool cmd_all;
static const pcmd_t cmd = 
{
	.title = "ls",
	.desc = "Lists files.",
	.version = BUILDVERSION,
	.date = BUILDDATE,
	.user = BUILDUSER,
	.opts = (pcmd_opt_t[])
	{
		{
			.name = "List Mode",
			.desc = "Outputs one file per line with details.",
			.letters = "l",
			.words = (const char *[]){ "list", NULL },
			.given = &cmd_list,
		},
		{
			.name = "All Files",
			.desc = "Includes hidden files (beginning with \".\") in output.",
			.letters = "a",
			.words = (const char *[]){ "all", NULL },
			.given = &cmd_all,
		},
		{ 0 }
	}
};

void print_info(const char *name, const struct stat *st)
{	
	if(!cmd_list)
	{
		//Simple printout
		printf("%s ", name);
		return;
	}
	
	//Full list format
	
	char *typestr = "?";
	if(S_ISDIR(st->st_mode))
		typestr = "d";
	else if(S_ISCHR(st->st_mode))
		typestr = "c";
	else if(S_ISBLK(st->st_mode))
		typestr = "b";
	else if(S_ISLNK(st->st_mode))
		typestr = "l";
	else if(S_ISREG(st->st_mode))
		typestr = " ";
	
	const char *permletters = "rwxrwxrwx";
	char permstr[10] = {0};
	for(int permbit = 0; permbit < 9; permbit++)
	{
		if(st->st_mode & (1<<(8-permbit)))
			permstr[permbit] = permletters[permbit];
		else
			permstr[permbit] = '-';
	}
			
	
	printf("%10d %s %s %s\n", st->st_size, permstr, typestr, name);
}

void handle_filename(const char *name)
{
	int fd = open(name, O_NOFOLLOW);
	if(fd < 0)
	{
		printf("%s: cannot find.\n", name);
		return;
	}
	
	struct stat st = {0};
	int stat_result = fstat(fd, &st);
	if(stat_result < 0)
	{
		printf("%s: cannot stat.\n", name);
		close(fd);
		return;
	}
	
	if(!S_ISDIR(st.st_mode))
	{
		//Single file. Just print it.
		print_info(name, &st);
		close(fd);
		return;
	}
	
	//Read directory and print all files within.
	DIR *dirp = fdopendir(fd);
	if(dirp == NULL)
	{
		printf("%s: cannot read directory.\n", name);
		close(fd);
		return;
	}
	
	struct dirent *de;
	while( (de = readdir(dirp)) != NULL )
	{
		if(de->d_name[0] == '.' && !cmd_all)
			continue;
		
		struct stat de_sb = {0};
		int de_stat_result = fstatat(fd, de->d_name, &de_sb, AT_SYMLINK_NOFOLLOW);
		if(de_stat_result < 0)
		{
			printf("%s: cannot stat.\n", de->d_name);
		}
		else
		{
			print_info(de->d_name, &de_sb);
		}
	}
		
	closedir(dirp);
}

int main(int argc, char **argv)
{
	pcmd_parse(&cmd, argc, argv);
	
	if(argc <= 1)
	{
		handle_filename(".");
	}
	else
	{
		for(int aa = 1; aa < argc; aa++)
		{
			if(argv[aa][0] == '-')
				continue;
			
			handle_filename(argv[aa]);
		}
	}
	
	if(!cmd_list)
	{
		//Non-list output needs a single newline at the end
		printf("\n");
	}
	
	return 0;
}
