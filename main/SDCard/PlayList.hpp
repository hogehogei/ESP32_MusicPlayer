
#ifndef PLAYLIST_HPP_INCLUDED
#define PLAYLIST_HPP_INCLUDED

#include <string>
#include <vector>

#include "FileSystem.hpp"

class I_FileEntry
{
public:
    enum FileType {
        File,
        Directory
    };

    virtual ~I_FileEntry() {}

    virtual const std::string Name() const = 0;
    virtual const std::string FullPath() const = 0;
    virtual FileType Type() const = 0;
};
using I_FileEntryPtr = std::shared_ptr<I_FileEntry>;


class FilePath : public I_FileEntry
{
public:

    FilePath( I_FileEntry* parent, const std::string& filename );
    virtual ~FilePath();

    virtual const std::string Name() const override;
    virtual const std::string FullPath() const override;
    virtual I_FileEntry::FileType Type() const override;

private:

    I_FileEntry* m_Parent;
    const std::string m_FileName;
};

class DirectoryPath : public I_FileEntry
{
public:

    DirectoryPath( I_FileEntry* parent, const std::string& directory_path );
    virtual ~DirectoryPath();

    virtual const std::string Name() const override;
    virtual const std::string FullPath() const override;
    virtual I_FileEntry::FileType Type() const override;

    const std::vector<I_FileEntryPtr>& EntryList() const;

private:

    I_FileEntry* m_Parent;
    const std::string m_DirectoryName;
    std::vector<I_FileEntryPtr> m_DirectoryList;
};

class PlayList
{
public:
    
    PlayList( const std::string& root_directory );
    ~PlayList() noexcept;

    const std::string Current() const;
    const std::string Next();
    const std::string Prev();

private:

    static constexpr int sk_NoEntry = -1;

    int searchFileIdx();
    
    DirectoryPath  m_RootDir;
    const DirectoryPath* m_CurrentDir;
    std::vector<I_FileEntryPtr> m_CurrentDirFileList;
    int m_CurrentFileIdx;
};


#endif  // PLAYLIST_HPP_INCLUDED
