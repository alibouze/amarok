// Author: Max Howell (C) Copyright 2003-4
// Author: Mark Kretschmann (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution
//

#ifndef PLAYLISTLOADER_H
#define PLAYLISTLOADER_H

#include "debug.h"        //stack allocated
#include <kurl.h>         //KURL::List
#include "metabundle.h"   //stack allocated
#include "threadweaver.h" //baseclass

class QDomNode;
class QListViewItem;
class QTextStream;
class PlaylistItem;

namespace KIO { class Job; }


/**
 * @class PlaylistFile
 * @author Max Howell
 * @short Allocate on the stack, the contents are immediately available from bundles()
 *
 * Note, it won't do anything with XML playlists
 *
 * TODO be able to load directories too, it's in the spec
 * TODO and playlists within playlists, remote and local
 */
class PlaylistFile
{
public:
    PlaylistFile( const QString &path );

    enum Format { M3U, PLS, XML, Unknown, NotPlaylist = Unknown };

    /// the bundles from this playlist, they only contain
    /// the information that can be extracted from  the playlists
    BundleList &bundles() { return m_bundles; }

    ///@return true if couldn't load the playlist's contents
    bool isError() const { return !m_error.isEmpty(); }

    /// if start returns false this has a translated error description
    QString error() const { return m_error; }

    static inline bool isPlaylistFile( const KURL &url ) { return isPlaylistFile( url.fileName() ); }
    static inline bool isPlaylistFile( const QString &fileName ) { return format( fileName ) != Unknown; }
    static inline Format format( const QString &fileName );

protected:
    /// make these virtual if you need to
    bool loadM3u( QTextStream& );
    bool loadPls( QTextStream& );

    QString m_path;
    QString m_error;
    BundleList m_bundles;
};

inline PlaylistFile::Format
PlaylistFile::format( const QString &fileName )
{
    const QString ext = fileName.right( 4 ).lower();

    if( ext == ".m3u" ) return M3U;
    else if( ext == ".pls" ) return PLS;
    else if( ext == ".xml" ) return XML;
    else return Unknown;
}



/**
 * @author Max Howell
 * @author Mark Kretschmann
 * @short Populates the Playlist-view with URLs
 *
 * Will load playlists, remote and local, and tags (if local or in db)
 */
class PlaylistLoader : public ThreadWeaver::DependentJob
{
public:
    PlaylistLoader( const KURL::List&, QListViewItem*, bool playFirstUrl = false );
   ~PlaylistLoader();

protected:
    /// reimplemented from ThreadWeaver::Job
    virtual bool doJob();
    virtual void completeJob();
    virtual void customEvent( QCustomEvent* );

    void loadXml( const KURL& );

private:
    KURL::List recurse( const KURL& );

private:
    KURL::List m_badURLs;
    KURL::List m_URLs;

    PlaylistItem *m_markerListViewItem;

    bool m_playFirstUrl;

    Debug::Block m_block;

protected:
    PlaylistLoader( const PlaylistLoader& ); //undefined
    PlaylistLoader &operator=( const PlaylistLoader& ); //undefined
};



/**
 * @author Max Howell
 * @short Populates the Playlist-view using the result of a single SQL query
 *
 * The format of the query must be in a set order, see doJob()
 */
class SqlLoader : public PlaylistLoader
{
    const QString m_sql;

public:
    SqlLoader( const QString &sql, QListViewItem *after );

    virtual bool doJob();
};



/**
 * @author Max Howell
 * @short Fetches a playlist-file from any location, and then loads it into the Playlist-view
 */
class RemotePlaylistFetcher : public QObject
{
    Q_OBJECT

    const KURL m_source;
    KURL m_destination;
    QListViewItem *m_after;

public:
    RemotePlaylistFetcher( const KURL &source, QListViewItem *after, QObject *playlist );

private slots:
    void result( KIO::Job* );
    void abort() { delete this; }
};


#endif
