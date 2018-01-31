## 说明

   1.fatfs 文件系统使用了micropython写的oofatfs，该文件里的内容为oofatfs的辅助内容，notused文件夹里的文件并没有使用，可以     
oofatfs里找到相应文件的定义；drivers里编译了sflash_diskio.c文件（因为文件系统是写在flash里的）
    

   2.文件系统的挂载还使用了micropython/extmod文件夹里的 vfs等文件，现在使用的文件系统参考了c3200的使用方法；