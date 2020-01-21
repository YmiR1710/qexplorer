#include <archive.h>
#include <archive_entry.h>
#include <iostream>
#include <QStringList>
#include <QString>
#include <QDirIterator>
#include <QDebug>
#include <fcntl.h>


bool is_archive(QString ext){
    QStringList supported_formats = {"tar", "pax", "cpio", "zip", "xar", "lha", "ar", "cab", "mtree", "rar"};
    if (supported_formats.contains(ext))
        return true;
    else
        return false;
}


int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return (r);
        }
    }
}

void compress(QStringList filepathes, const QString& outname, int compress) {
    QStringList Files;
    QStringList Entries;
    size_t j = 0;

    for(int i = 0; i < filepathes.length(); ++i) {
        QString filepath = filepathes[i];
        qDebug() << "filepath:" << filepath;

        QDirIterator it(filepath, QDirIterator::Subdirectories);
           if(QDir(filepath).exists()){
              while (it.hasNext()) {
                  it.next();
                  if(it.fileName() == ".." || it.fileName() == ".")
                          continue;
                  Files.append(it.filePath());
              }

              std::string root = QDir(filepath).dirName().toStdString();
              for(j; j < Files.length(); ++j) {
                  size_t position = Files[j].toStdString().find(root);
                  Entries.append(QString::fromStdString(Files[j].toStdString().substr(position)));
                    qDebug() << "entry: "<< Entries[j];
              }
           }
           else{
              Files.append(filepath);
              Entries.append(QFile(filepath).fileName());
           }
       }

       qDebug() << "here";
       struct archive *a;
       struct archive_entry *entry;
       ssize_t len;
       int fd;

       static char buff[16384];


       const std::string filename = outname.toStdString();

       a = archive_write_new();
       switch (compress) {
       case 'j': case 'y':
           archive_write_add_filter_bzip2(a);
           break;
       case 'Z':
           archive_write_add_filter_compress(a);
           break;
       case 'z':
           archive_write_add_filter_gzip(a);
           break;
       default:
           archive_write_add_filter_none(a);
           break;
       }

       archive_write_set_format_ustar(a);
       archive_write_open_filename(a, filename.c_str());

       for (int i = 0; i < Files.length(); ++i){
           std::string f = Files[i].toStdString();
           std::string e = Entries[i].toStdString();

           struct archive *disk = archive_read_disk_new();
           archive_read_disk_set_standard_lookup(disk);
           int r;

           r = archive_read_disk_open(disk, f.c_str());
           if (r != ARCHIVE_OK) {
               qDebug() << archive_error_string(disk);
               return ;
           }

           while(true) {
               entry = archive_entry_new();

               archive_entry_set_pathname(entry, e.c_str());

               r = archive_read_next_header2(disk, entry);
               if (r == ARCHIVE_EOF)
                   break;
               if (r != ARCHIVE_OK) {
                   qDebug() << archive_error_string(disk);
                   return ;
               }
               archive_read_disk_descend(disk);
               archive_entry_set_pathname(entry, e.c_str());

               r = archive_write_header(a, entry);
               if (r == ARCHIVE_FATAL)
                   return ;
               if (r > ARCHIVE_FAILED) {
                   fd = open(archive_entry_sourcepath(entry), O_RDONLY);
                   len = read(fd, buff, sizeof(buff));
                   while (len > 0) {
                       archive_write_data(a, buff, len);
                       len = read(fd, buff, sizeof(buff));
                   }
                   close(fd);
               }
               archive_entry_free(entry);
           }

           archive_read_close(disk);
           archive_read_free(disk);
       }
       archive_write_close(a);
       archive_write_free(a);
//       for(int i = 0; i<Files.size(); i++){
//           qDebug() << Files[i] << "|" << Entries[i];
//       }

}
void extract(const char *filename, QString out_path) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;

    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    archive_read_support_format_all(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    if ((r = archive_read_open_filename(a, filename, 10240)))
        throw std::runtime_error("Archiving Error: failed to read arhive");
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(a));
        if (r < ARCHIVE_WARN)
            throw std::runtime_error("Archiving Error");

        const std::string full_out_path = out_path.toStdString() + (std::string)archive_entry_pathname(entry);
        archive_entry_set_pathname(entry, full_out_path.c_str());

        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(ext));
            if (r < ARCHIVE_WARN)
                throw std::runtime_error("Archiving Error");
        }

        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
        if (r < ARCHIVE_WARN)
            exit(1);
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
}
