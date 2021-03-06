#include "table_records.h"

namespace DB {

QString const toString(TableIndex idx)
{
    QString idx_str = "";

    switch(idx)
    {
        case SOUND_FILE:
            idx_str = "sound_file";
            break;
        case CATEGORY:
            idx_str = "category";
            break;
        case SOUND_FILE_CATEGORY:
            idx_str = "sound_file_category";
            break;
        case RESOURCE_DIRECTORY:
            idx_str = "resource_directory";
        default:
            break;
    }

    return idx_str;
}

TableIndex toTableIndex(QString const& idx_str)
{
    if(idx_str.compare("sound_file") == 0) {
        return SOUND_FILE;
    } else if(idx_str.compare("category") == 0) {
        return CATEGORY;
    } else if(idx_str.compare("sound_file_category") == 0) {
        return SOUND_FILE_CATEGORY;
    } else if(idx_str.compare("resource_directory") == 0) {
        return RESOURCE_DIRECTORY;
    } else {\
        return NONE;
    }
}

}
