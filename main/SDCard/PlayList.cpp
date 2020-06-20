
#include "PlayList.hpp"

// C++ stdlib
#include <string>
#include <memory>

// C standard lib
#include <dirent.h>

PlayList::PlayList( const std::string& root_directory )
: 
    m_RootDir( nullptr, root_directory ),
    m_CurrentDir( nullptr ),
    m_CurrentDirFileList(),
    m_CurrentFileIdx(0)
{
    m_CurrentDir = &m_RootDir;
    m_CurrentDirFileList = m_CurrentDir->EntryList();
    m_CurrentFileIdx = searchFileIdx();
}

PlayList::~PlayList()
{}

int PlayList::searchFileIdx()
{
    if( m_CurrentDirFileList.size() == 0 ){
        return PlayList::sk_NoEntry;
    }
    for( int i = 0; i < m_CurrentDirFileList.size(); ++i ){
        if( m_CurrentDirFileList[i]->Type() == I_FileEntry::FileType::File ){
            return i;
        }
    }

    return PlayList::sk_NoEntry;
}

const std::string PlayList::Current() const
{
    return m_CurrentFileIdx != PlayList::sk_NoEntry ? m_CurrentDirFileList[m_CurrentFileIdx]->Name() : std::string();
}

const std::string PlayList::Next() 
{
    if( m_CurrentFileIdx == PlayList::sk_NoEntry ||
        m_CurrentDirFileList.size() <= 0 )
    {
        m_CurrentFileIdx = PlayList::sk_NoEntry;
        return std::string();
    }

    int previdx = m_CurrentFileIdx;
    while( 1 ){
        m_CurrentFileIdx = (m_CurrentFileIdx + 1) % m_CurrentDirFileList.size();
        if( previdx == m_CurrentFileIdx ){
            break;
        }
        if( m_CurrentDirFileList[m_CurrentFileIdx]->Type() == I_FileEntry::FileType::File ){
            break;
        }
    }

    return Current();
}

const std::string PlayList::Prev()
{
    if( m_CurrentFileIdx == PlayList::sk_NoEntry ||
        m_CurrentDirFileList.size() <= 0 )
    {
        m_CurrentFileIdx = PlayList::sk_NoEntry;
        return std::string();
    }

    int previdx = m_CurrentFileIdx;
    while( 1 ){
        m_CurrentFileIdx = (m_CurrentFileIdx - 1) < 0 ? m_CurrentDirFileList.size() - 1 : m_CurrentFileIdx - 1;
        if( previdx == m_CurrentFileIdx ){
            break;
        }
        if( m_CurrentDirFileList[m_CurrentFileIdx]->Type() == I_FileEntry::FileType::File ){
            break;
        }
    }

    return Current();
}

FilePath::FilePath( I_FileEntry* parent, const std::string& filename )
:
    m_Parent( parent ),
    m_FileName( filename )
{}

FilePath::~FilePath()
{}

const std::string FilePath::Name() const
{
    return m_FileName;
}

const std::string FilePath::FullPath() const
{
    if( m_Parent == nullptr ){
        return m_FileName;
    }
    return m_Parent->FullPath() + m_FileName;
}

I_FileEntry::FileType FilePath::Type() const
{
    return I_FileEntry::FileType::File;
}


DirectoryPath::DirectoryPath( I_FileEntry* parent, const std::string& directory_path )
:
    m_Parent( parent ),
    m_DirectoryName( directory_path ),
    m_DirectoryList()
{
    const std::string& fullpath = FullPath();
    DIR* dir = opendir( fullpath.c_str() );

    if( dir != nullptr ){
        while(1){
            struct dirent* dent = readdir( dir );
            if( dir == nullptr ){
                break;
            }
            if( dent->d_name[0] == '\0' ){
                break;
            }
            if( dent->d_name[0] == '.' ){
                continue;
            }

            if( dent->d_type & DT_DIR ){
                I_FileEntryPtr new_ent = std::make_shared<DirectoryPath>( this, fullpath + std::string( dent->d_name ) );
                m_DirectoryList.push_back( new_ent );
            }
            if( dent->d_type & DT_REG ){
                I_FileEntryPtr new_ent = std::make_shared<FilePath>( this, fullpath + std::string( dent->d_name ) );
                m_DirectoryList.push_back( new_ent );
            }
        }
    }
}

DirectoryPath::~DirectoryPath()
{}

const std::string DirectoryPath::Name() const
{
    return m_DirectoryName;
}

const std::string DirectoryPath::FullPath() const
{
    if( m_Parent == nullptr ){
        return m_DirectoryName + "/";
    }

    return m_Parent->FullPath() + m_DirectoryName + "/";
}

const std::vector<I_FileEntryPtr>& DirectoryPath::EntryList() const
{
    return m_DirectoryList;
}

I_FileEntry::FileType DirectoryPath::Type() const
{
    return I_FileEntry::FileType::Directory;
}
