#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <archive.h>
#include <archive_entry.h>
#include <QString>

bool is_archive(QString ext);

void extract(const char *filename, QString out_path);

int  copy_data(struct archive *ar, struct archive *aw);

#endif // ARCHIVER_H
