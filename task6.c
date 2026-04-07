#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/sysmacros.h> 

void print_ftype(struct stat *info) {
    if (S_ISREG(info->st_mode))
        printf("regular file\n");
    else if (S_ISDIR(info->st_mode))
        printf("directory\n");
    else if (S_ISLNK(info->st_mode))
        printf("symbolic link\n");
    else if (S_ISCHR(info->st_mode))
        printf("character device\n");
    else if (S_ISBLK(info->st_mode))
        printf("block device\n");
    else if (S_ISFIFO(info->st_mode))
        printf("FIFO/pipe\n");
    else if (S_ISSOCK(info->st_mode))
        printf("socket\n");
    else
        printf("unknown\n");
}


void access_str(struct stat *info, char *buf){
    mode_t mode = info->st_mode;
    if (S_ISREG(mode))      buf[0] = '-';
    else if (S_ISDIR(mode)) buf[0] = 'd';
    else if (S_ISCHR(mode)) buf[0] = 'c';
    else if (S_ISBLK(mode)) buf[0] = 'b';
    else if (S_ISFIFO(mode))buf[0] = 'p';
    else if (S_ISLNK(mode)) buf[0] = 'l';
    else if (S_ISSOCK(mode))buf[0] = 's';
    else                    buf[0] = '?';

    //права владельца
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    // Права группы
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    // Права остальных
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? 'x' : '-';
    buf[10] = '\0';
}

void print_time(const char *label, time_t time, long nsec) {
    struct tm tm;
    char buf[64];
    localtime_r(&time, &tm);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    printf("%s: %s.%09ld %+03ld%02ld\n", label, buf, nsec, tm.tm_gmtoff / 3600, (tm.tm_gmtoff % 3600) / 60);
}

int main(int argc, char *argv[]){
    if (argc != 2){
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return 1;
    }
    struct stat info;
    if (stat(argv[1], &info) == -1){
        perror("stat");
        return 1;
    }
    printf("  File :  %s\n", argv[1]);
    printf("  Size: %-10lld Blocks: %-10lld IO Block: %-6ld ", (long long)info.st_size, (long long)info.st_blocks, (long)info.st_blksize);
    print_ftype(&info);
    printf("Device: %d/%-6d Inode: %-6ld Links: %-6ld\n", major(info.st_dev), minor(info.st_dev), (long)info.st_ino, (long)info.st_nlink);

    char access[12];
    access_str(&info, access);
    printf("Access: (%04o/%s)  ", info.st_mode & 07777, access);

    struct passwd *pw = getpwuid(info.st_uid);
    printf("Uid: (%5ld/ %s)  ", (long)info.st_uid, pw ? pw->pw_name : "?");

    struct group *gr = getgrgid(info.st_gid);
    printf("Gid: (%5ld/ %s)\n", (long)info.st_gid, gr ? gr->gr_name : "?");

    print_time("Access", info.st_atime, info.st_atim.tv_nsec);
    print_time("Modify", info.st_mtime, info.st_atim.tv_nsec);
    print_time("Change", info.st_ctime, info.st_atim.tv_nsec);
    return 0;
}