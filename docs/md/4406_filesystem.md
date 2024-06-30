# FileSystem

## 概述

    SDK已经移植了LittleFS和FatFS两种文件系统，其中LittleFS使用SPI接口的Flash作为储存介质，FatFS使用SPI接口的SD卡作为存储介质，并对文件系统相关的API进行了封装，提供抽象的接口给用户使用。

## API参考

    对应的头文件 fs.h、fs_file.h

    为用户提供以下接口

- [fs_mount](#fs_mount)
- [fs_format](#fs_format)
- [fs_unmount](#fs_unmount)
- [fs_size](#fs_size)
- [fs_capacity](#fs_capacity)
- [get_curr_fs](#get_curr_fs)
- [fs_get_lock](#fs_get_lock)
- [fs_release_lock](#fs_release_lock)
- [fs_file_open](#fs_file_open)
- [fs_file_close](#fs_file_close)
- [fs_file_sync](#fs_file_sync)
- [fs_file_read](#fs_file_read)
- [fs_file_write](#fs_file_write)
- [fs_file_seek](#fs_file_seek)
- [fs_file_size](#fs_file_size)
- [fs_file_rewind](#fs_file_rewind)
- [fs_dir_open](#fs_dir_open)
- [fs_dir_read](#fs_dir_read)
- [fs_dir_close](#fs_dir_close)
- [fs_remove_file](#fs_remove_file)
- [fs_mkdir](#fs_mkdir)
- [fs_rename](#fs_rename)
- [fs_get_curr_gpath](#fs_get_curr_gpath)

### fs_mount
<div id="fs_mount"></div>

#### 描述

    挂载文件系统。

#### 函数定义
```c
    int fs_mount(fs_cfg_t *cfg)
```
#### 参数

| 参数名称 | 描述               | 输入输出 |
| -------- | ------------------ | -------- |
| cfg      | 配置参数结构体指针 | 输入     |

#### 返回值

    成功返回0，错误返回其他值。

### fs_format
<div id="fs_format"></div>

#### 描述

    格式化文件系统。

#### 函数定义
```c
    int fs_format(void)
```
#### 参数

    无。

#### 返回值

    成功返回0，错误返回其他值。

### fs_unmount
<div id="fs_unmount"></div>

#### 描述

    卸载文件系统。

#### 函数定义
```c
    int fs_unmount(void)
```
#### 参数

    无。

#### 返回值

    成功返回0，错误返回其他值。

### fs_size
<div id="fs_size"></div>

#### 描述

    获取当前文件系统已使用的容量大小。

#### 函数定义
```c
    int fs_size(void)
```
#### 参数

    无。

#### 返回值

    文件系统已使用的容量字节数。

### fs_capacity
<div id="fs_capacity"></div>

#### 描述

    获取当前文件系统总的容量大小。

#### 函数定义
```c
    int fs_capacity(void)
```
#### 参数

    无。

#### 返回值

    文件系统总的容量字节数。

### get_curr_fs
<div id="get_curr_fs"></div>

#### 描述

    获取当前文件系统指针。

#### 函数定义
```c
    fs_t *get_curr_fs(void)
```
#### 参数

    无。

#### 返回值

    当前文件系统指针。

### fs_get_lock
<div id="fs_get_lock"></div>

#### 描述

    锁住文件系统。

#### 函数定义
```c
    int fs_get_lock(void)
```
#### 参数

    无。

#### 返回值

    成功返回0，错误返回其他值。

### fs_release_lock
<div id="fs_release_lock"></div>

#### 描述

    解锁文件系统。

#### 函数定义
```c
    int fs_release_lock(void)
```
#### 参数

    无。

#### 返回值

    成功返回0，错误返回其他值。

### fs_file_open
<div id="fs_file_open"></div>

#### 描述

    打开文件。

#### 函数定义
```c
    fs_file_t * fs_file_open(const char *filename, int flags)
```
#### 参数

| 参数名称 | 描述           | 输入输出 |
| -------- | -------------- | -------- |
| filename | 待打开的文件名 | 输入     |
| flags    | 打开文件的方式 | 输入     |

#### 返回值

    成功返回文件指针，错误返回NULL。

### fs_file_close
<div id="fs_file_close"></div>

#### 描述

    关闭文件。

#### 函数定义
```c
    int fs_file_close(fs_file_t *fp)
```
#### 参数

| 参数名称 | 描述             | 输入输出 |
| -------- | ---------------- | -------- |
| fp       | 待关闭的文件指针 | 输入     |

#### 返回值

    成功返回0，错误返回其他。

### fs_file_sync
<div id="fs_file_sync"></div>

#### 描述

    同步文件内容到储存介质。

#### 函数定义
```c
    int fs_file_sync(fs_file_t *fp)
```
#### 参数

| 参数名称 | 描述             | 输入输出 |
| -------- | ---------------- | -------- |
| fp       | 待同步的文件指针 | 输入     |

#### 返回值

    成功返回0，错误返回其他。

### fs_file_read
<div id="fs_file_read"></div>

#### 描述

    读取文件。

#### 函数定义
```c
    int fs_file_read(fs_file_t *fp, void *buffer, int size)
```
#### 参数

| 参数名称 | 描述               | 输入输出 |
| -------- | ------------------ | -------- |
| fp       | 待读取的文件指针   | 输入     |
| buffer   | 读取数据的存放地址 | 输入     |
| size     | 读取数据的长度     | 输入     |

#### 返回值

    成功返回读取的数据字节数，错误返回负值。

### fs_file_write
<div id="fs_file_write"></div>

#### 描述

    写入文件。

#### 函数定义
```c
    int fs_file_write(fs_file_t *fp, void *buffer, int size)
```
#### 参数

| 参数名称 | 描述               | 输入输出 |
| -------- | ------------------ | -------- |
| fp       | 待写入的文件指针   | 输入     |
| buffer   | 写入数据的存放地址 | 输入     |
| size     | 写入数据的长度     | 输入     |

#### 返回值

    成功返回写入的数据字节数，错误返回负值。

### fs_file_seek
<div id="fs_file_seek"></div>

#### 描述

    设置文件的读写指针。

#### 函数定义
```c
    int fs_file_seek(fs_file_t *fp, int off, int whence)
```
#### 参数

| 参数名称 | 描述                     | 输入输出 |
| -------- | ------------------------ | -------- |
| fp       | 文件指针                 | 输入     |
| off      | 文件读写指针偏移量       | 输入     |
| whence   | 文件读写指针偏移起始地址 | 输入     |

#### 返回值

    成功返回写入的数据字节数，错误返回负值。

### fs_file_size
<div id="fs_file_size"></div>

#### 描述

    获取文件的大小。

#### 函数定义
```c
    int fs_file_size(fs_file_t *fp)
```
#### 参数

| 参数名称 | 描述     | 输入输出 |
| -------- | -------- | -------- |
| fp       | 文件指针 | 输入     |

#### 返回值

    成功返回文件的字节数，错误返回负值。

### fs_file_rewind
<div id="fs_file_rewind"></div>

#### 描述

    设置文件读写指针到文件开头。

#### 函数定义
```c
    int fs_file_rewind(fs_file_t *fp)
```
#### 参数

| 参数名称 | 描述     | 输入输出 |
| -------- | -------- | -------- |
| fp       | 文件指针 | 输入     |

#### 返回值

    成功返回0，错误返回负值。

### fs_dir_open
<div id="fs_dir_open"></div>

#### 描述

    打开目录。

#### 函数定义
```c
    fs_dir_t * fs_dir_open(const char * dirname, int * dirstat)
```
#### 参数

| 参数名称 | 描述             | 输入输出 |
| -------- | ---------------- | -------- |
| dirname  | 目录名           | 输入     |
| dirstat  | 存储错误码的指针 | 输出     |

#### 返回值

    目录信息结构体指针。

### fs_dir_read
<div id="fs_dir_read"></div>

#### 描述

    读取目录。

#### 函数定义
```c
    int fs_dir_read(fs_dir_t * dirp, struct lfs_info * info)
```
#### 参数

| 参数名称 | 描述               | 输入输出 |
| -------- | ------------------ | -------- |
| dirp     | 目录信息结构体指针 | 输入     |
| info     | 文件信息结构体指针 | 输出     |

#### 返回值

    成功返回文件的个数，错误返回负值。

### fs_dir_close
<div id="fs_dir_close"></div>

#### 描述

    关闭目录。

#### 函数定义
```c
    int fs_dir_close(fs_dir_t * dirp)
```
#### 参数

| 参数名称 | 描述               | 输入输出 |
| -------- | ------------------ | -------- |
| dirp     | 目录信息结构体指针 | 输入     |

#### 返回值

    成功返回0，错误返回负值。

### fs_remove_file
<div id="fs_remove_file"></div>

#### 描述

    删除文件或者目录。

#### 函数定义
```c
    int fs_remove_file(const char *path, int len)
```
#### 参数

| 参数名称 | 描述                         | 输入输出 |
| -------- | ---------------------------- | -------- |
| path     | 待删除的文件或者目录名称     | 输入     |
| len      | 待删除的文件或者目录名称长度 | 输入     |

#### 返回值

    成功返回0，错误返回负值。

### fs_mkdir
<div id="fs_mkdir"></div>

#### 描述

    创建目录。

#### 函数定义
```c
    int fs_mkdir(const char *path)
```
#### 参数

| 参数名称 | 描述             | 输入输出 |
| -------- | ---------------- | -------- |
| path     | 待创建的目录名称 | 输入     |

#### 返回值

    成功返回0，错误返回负值。

### fs_rename
<div id="fs_rename"></div>

#### 描述

    修改文件名或者目录名。

#### 函数定义
```c
    int fs_rename(const char *oldpath, const char *newpath)
```
#### 参数

| 参数名称 | 描述         | 输入输出 |
| -------- | ------------ | -------- |
| oldpath  | 旧的文件名称 | 输入     |
| newpath  | 新的文件名称 |          |

#### 返回值

    成功返回0，错误返回负值。

### fs_get_curr_gpath
<div id="fs_get_curr_gpath"></div>

#### 描述

    获取当前的路径。

#### 函数定义
```c
    char *fs_get_curr_gpath(void)
```
#### 参数

    无。

#### 返回值

    当前的路径字符串指针。

## 数据类型

    相关数据类型、结构体如下：

- [fs_cfg_t](#fs_cfg_t): 文件系统配置结构体类型
- [fs_t](#fs_t): 文件系统定义
- [fs_file_t](#fs_file_t)
- [fs_dir_t](#fs_dir_t) :

### fs_cfg_t
<div id="fs_cfg_t"></div>

#### 描述

    文件系统配置结构体类型，用于指定挂载文件系统时的配置参数。

#### 定义

```c
typedef struct _fs_cfg_t {
    flash_type_t flash_type;
    int          spi_num;
    int          spi_chip_sel;
    uint32_t     clk_rate;
    int          start_offset;
} fs_cfg_t;
```

### fs_t
<div id="fs_t"></div>

#### 描述

    文件系统定义。

#### 定义

```c
typedef struct _fs_t {
    fs_cfg_t cfg;
    void    *fs_ptr;
} fs_t;
```

### fs_file_t
<div id="fs_file_t"></div>

#### 描述

    文件定义。

#### 定义

```c
typedef struct _fs_file_t {
    fs_t            *fs;
    lfs_file_t      *file;
} fs_file_t;
```

### fs_dir_t
<div id="fs_dir_t"></div>

#### 描述

    目录定义。

#### 定义

```c
typedef struct _fs_dir_t {
    fs_t            *fs;
    lfs_dir_t      *dir;
} fs_dir_t;
```

## 例程
```c
/**
 * 文件系统操作
*/
void fs_case(void)
{
    LOGI("", "file system test.");
    // fs_mount()           // 挂载文件系统
    // fs_format()          // 格式化文件系统
    // fs_unmount()         // 卸载文件系统
    // fs_get_lock()        // 锁住文件系统
    // fs_release_lock()    // 解锁文件系统
    // 获取当前文件系统已使用的容量大小
    int fs_used_size = fs_size();
    // 获取当前文件系统总的容量大小
    int fs_cap = fs_capacity();
    // 获取当前文件系统指针
    fs_t* cur_fs = get_curr_fs();
    LOGI("fs","fs used size:%d Byte", fs_used_size);
    LOGI("fs","current fs capacity:%d Byte.", fs_cap);
    LOGI("fs","current fs ptr:0x%p.", cur_fs);
}
```

```c
/**
 * 文件相关操作
*/
void fs_file_case(void)
{
    const char* test_file = "test.txt";
    char data[64] = "This is test read write file string\r\n";
    LOGI("", "fs file test start.......");

    // 以读写方式打开文件,如果文件不存在则创建
    fs_file_t* fd = fs_file_open(test_file, FS_O_RDWR|FS_O_CREAT);
    if(fd == NULL) {
        LOGE("fs file", "open file failed.");
        return;
    }

    // 文件中写入一段测试数据
    int ret = fs_file_write(fd, data, sizeof(data)/sizeof(data[0]));
    if(ret<0) {
        LOGE("fs file", "write data to file failed.");
        fs_file_close(fd);
    }
    LOGI("fs file", "write size:%d data to file success.");

    // 获取文件的大小
    int file_sz = fs_file_size(fd);
    if(file_sz < 0) {
        LOGE("fs file", "get file size failed.");
        return;
    }
    LOGI("fs file", "get file size(%d) success.");

    // 设置文件读写指针到文件开头
    fs_file_rewind(fd);
    // 读取文件内容到缓冲区
    char buffer[128] = {0};
    ret = fs_file_read(fd, buffer, 128);
    if(ret < 0) {
        LOGE("fs file", "read file failed.");
        fs_file_close(fd);
        return;
    }
    LOGI("fs file", "read file to buffer size(%d) success.", ret);
    LOGI("fs file", "read data:%s.", buffer);

    // 设置文件读写指针到文件开头
    file_sz = fs_file_seek(fd, 0, LFS_SEEK_END);       // 设置文件的读写指针。 
    LOGI("fs file", "get file size:%d.", file_sz);

    ret = fs_file_sync(fd);
    if(ret != 0) {
        LOGE("fs file", "sync file to flash failed.");
        fs_file_close(fd);
        return;
    }
    LOGI("fs file", "sync file to flash SUCCESS.");

    fs_file_close(fd);      // 关闭文件。
    LOGI("fs file", "fs file test end.......");
}
```

```c
/**
 * 目录操作
*/
void fs_dir_case(void)
{
    LOGI("", "\n");
    LOGI("fs dir", "dir test start.......");
    const char* test_dir = "./test_dir";
    int err = 0;

    // 创建目录
    int ret = fs_mkdir(test_dir);
    if(ret != 0) {
        LOGE("fs dir", "make dir failed.");
        return;
    }
    LOGI("fs dir", "make dir success.");

    fs_dir_t* dir = fs_dir_open(test_dir, &err);
    if(err != LFS_ERR_OK) {
        LOGE("fs dir", "open dir failed.");
        return;
    }
    LOGI("fs dir", "open dir success.");

    struct lfs_info  info;
    fs_dir_read(dir, &info);
    if(LFS_TYPE_DIR == info.type) {
        LOGI("fs dir", "current file type is directory");
    }
    // 关闭打开的目录
    fs_dir_close(dir);
    // 获取当前的路径
    char* g_path = fs_get_curr_gpath();
    LOGI("fs dir", "get current path:%s.", g_path);
    LOGI("fs dir", "fs dir test end.......");
}
```

```c
/**
 * 目录以及文件均适用的操作
*/
void fs_file_dir_case(void)
{
    int ret = 0;
    LOGI("", "dir test.");

    // 测试文件以及测试目录重命名
    ret = fs_rename("test.txt", "test_rename.txt");
    ret |= fs_rename("test_dir", "test_rename_dir");
    if(ret != 0) {
        LOGE("fs rename", "rename file and dir failed.");
        return;
    }
    LOGI("fs rename", "rename file and dir success.");

    // 删除测试文件以及测试目录
    ret = fs_remove_file("test_rename.txt", strlen("test_rename.txt") + 1);
    ret |= fs_remove_file("test_rename_dir", strlen("test_rename_dir") + 1);
    if(ret != 0) {
        LOGE("fs rename", "remove file and dir failed.");
        return;
    }
    LOGI("fs rename", "remove file and dir success.");
}
```
