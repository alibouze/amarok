/****************************************************************************************
 * Copyright (c) 2007 Leo Franchi <lfranchi@gmail.com>                                  *
 * Copyright (c) 2008 Mark Kretschmann <kretschmann@kde.org>                            *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef AMAROK_LYRICS_ENGINE
#define AMAROK_LYRICS_ENGINE

#include "context/DataEngine.h"
#include "context/LyricsManager.h"
#include "core/meta/Meta.h"

/**
    This class provides Lyrics data for use in Context applets. 

NOTE: The QVariant data is structured like this:
           * the key name is lyrics
           * the data is a QVariantList with title, artist, lyricsurl, lyrics
*/

using namespace Context;

class LyricsEngine : public DataEngine, public LyricsObserver
{
    Q_OBJECT

public:
    LyricsEngine( QObject* parent, const QList<QVariant>& args );

    QStringList sources() const;

    // reimplemented from LyricsObserver
    void newLyrics( QStringList& lyrics );
    void newLyricsHtml( QString& lyrics );
    void newSuggestions( const QVariantList &suggest );
    void lyricsMessage( QString& key, QString& val );

protected:
    bool sourceRequestEvent( const QString& name );

private slots:
    void update();

private:
    /**
      * Tests if the lyrics have changed.
      *
      * @param newLyrics The new lyrics.
      * @param oldHtmlLyrics The old (unchanged) HTML lyrics.
      * @param oldPlainLyrics The old plain lyrics (as provided by the LyricsEngine).
      *
      * @return true if the lyrics for the current track have changed, otherwise false.
      */
    bool testLyricsChanged( const QString& newLyrics,
                            const QString& oldHtmlLyrics,
                            QStringList oldPlainLyrics ) const;

    // Cache the title/artist of the current track so we can check against
    // metadata updates. We only want to update the lyrics if either the
    // title or the artist change (but not other attributes like rating, score,
    // composer etc).
    QStringList m_currentLyricsList,m_prevLyricsList;
    QVariantList m_currentSuggestionsList,m_prevSuggestionsList;
    QString m_currentLyrics,m_prevLyrics;
    QString m_title;
    QString m_artist;
};

AMAROK_EXPORT_DATAENGINE( lyrics, LyricsEngine )

#endif
