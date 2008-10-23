/***************************************************************************
 * copyright        : (C) 2007-2008 Ian Monroe <ian@monroe.nu>
 *                    (C) 2007 Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
 *                    (C) 2008 Seb Ruiz <ruiz@kde.org>
 *                    (C) 2008 Soren Harward <stharward@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#define DEBUG_PREFIX "Playlist::Actions"

#include "PlaylistActions.h"

#include "Amarok.h"
#include "amarokconfig.h"
#include "Debug.h"
#include "DynamicModel.h"
#include "EngineController.h"
#include "EngineObserver.h"
#include "navigators/DynamicTrackNavigator.h"
#include "navigators/RandomAlbumNavigator.h"
#include "navigators/RandomTrackNavigator.h"
#include "navigators/RepeatAlbumNavigator.h"
#include "navigators/RepeatTrackNavigator.h"
#include "navigators/StandardTrackNavigator.h"
#include "PlaylistModel.h"
#include "statusbar_ng/StatusBar.h"

#include <QAction>

#include <typeinfo>

Playlist::Actions* Playlist::Actions::s_instance = 0;

Playlist::Actions* Playlist::Actions::instance()
{
    return ( s_instance ) ? s_instance : new Actions( 0 );
}

void
Playlist::Actions::destroy()
{
    if ( s_instance )
    {
        delete s_instance;
        s_instance = 0;
    }
}

Playlist::Actions::Actions( QObject* parent )
        : QObject( parent )
        , EngineObserver( The::engineController() )
        , m_nextTrackCandidate( 0 )
        , m_navigator( 0 )
        , m_stopAfterMode( StopNever )
        , m_trackError( false )
        , m_waitingForNextTrack( false )
{
    DEBUG_BLOCK
    s_instance = this;
    playlistModeChanged(); // sets m_navigator.
    m_nextTrackCandidate = m_navigator->requestNextTrack();
}

Playlist::Actions::~Actions()
{
    DEBUG_BLOCK
    m_navigator->deleteLater();
}


void
Playlist::Actions::requestNextTrack()
{
    m_trackError = false;
    m_nextTrackCandidate = m_navigator->requestNextTrack();
    play( m_nextTrackCandidate, false );
}

void
Playlist::Actions::requestUserNextTrack()
{
    m_trackError = false;
    m_nextTrackCandidate = m_navigator->requestUserNextTrack();
    play( m_nextTrackCandidate );
}

void
Playlist::Actions::requestPrevTrack()
{
    m_trackError = false;
    m_nextTrackCandidate = m_navigator->requestLastTrack();
    play( m_nextTrackCandidate );
}


void
Playlist::Actions::play()
{
    if( 0 == m_nextTrackCandidate )
    {
        m_nextTrackCandidate = Model::instance()->activeId();
        if( 0 == m_nextTrackCandidate )
            m_nextTrackCandidate = m_navigator->requestNextTrack();
    }

    play( m_nextTrackCandidate );
}

void
Playlist::Actions::play( const QModelIndex& index )
{
    m_nextTrackCandidate = index.data( UniqueIdRole ).value<quint64>();
    play( m_nextTrackCandidate );
}

void
Playlist::Actions::play( int row )
{
    m_nextTrackCandidate = Model::instance()->idAt( row );
    play( m_nextTrackCandidate );
}

void
Playlist::Actions::play( quint64 trackid, bool now )
{
    DEBUG_BLOCK

    Model* model = Model::instance();

    if ( model->containsId( trackid ) ) {
        if ( now ) {
            The::engineController()->play( model->trackForId( trackid ) );
        } else {
            The::engineController()->setNextTrack( model->trackForId( trackid ) );
        }
    } else {
        m_trackError = true;
        warning() << "Invalid trackid" << trackid;
    }
}

void
Playlist::Actions::next()
{
    requestUserNextTrack();
}

void
Playlist::Actions::back()
{
    requestPrevTrack();
}

void
Playlist::Actions::playlistModeChanged()
{
    if ( m_navigator )
        m_navigator->deleteLater();

    debug() << "Dynamic mode:   " << AmarokConfig::dynamicMode();
    debug() << "Repeat enabled: " << Amarok::repeatEnabled();
    debug() << "Random enabled: " << Amarok::randomEnabled();
    debug() << "Track mode:     " << ( Amarok::repeatTrack() || Amarok::randomTracks() );
    debug() << "Album mode:     " << ( Amarok::repeatAlbum() || Amarok::randomAlbums() );

    if ( AmarokConfig::dynamicMode() )
    {
        PlaylistBrowserNS::DynamicModel* dm = PlaylistBrowserNS::DynamicModel::instance();

        Dynamic::DynamicPlaylistPtr playlist = dm->activePlaylist();

        if ( !playlist )
            playlist = dm->defaultPlaylist();

        m_navigator = new DynamicTrackNavigator( playlist );

        return;
    }

    m_navigator = 0;

    if ( Amarok::randomEnabled() ) {
        if ( Amarok::randomTracks() )
            m_navigator = new RandomTrackNavigator();
        else if ( Amarok::randomAlbums() )
            m_navigator = new RandomAlbumNavigator();
        else
            m_navigator = new StandardTrackNavigator(); // crap -- something went wrong
    } else if ( Amarok::repeatEnabled() ) {
        if ( Amarok::repeatTrack() )
            m_navigator = new RepeatTrackNavigator();
        else if ( Amarok::repeatAlbum() )
            m_navigator = new RepeatAlbumNavigator();
        else
            m_navigator = new StandardTrackNavigator(); // this navigator handles playlist repeat
    } else {
        m_navigator = new StandardTrackNavigator();
    }
}

void
Playlist::Actions::repopulateDynamicPlaylist()
{
    if ( typeid( *m_navigator ) == typeid( DynamicTrackNavigator ) )
    {
        (( DynamicTrackNavigator* )m_navigator )->repopulate();
    }
}

void
Playlist::Actions::engineStateChanged( Phonon::State currentState, Phonon::State )
{
    static int failures = 0;
    const int maxFailures = 4;

    if ( currentState == Phonon::ErrorState )
    {
        failures++;
        warning() << "Error, can not play this track.";
        warning() << "Failure count: " << failures;
        if ( failures >= maxFailures )
        {
            The::statusBarNG()->longMessage( i18n( "Too many errors encountered in playlist. Playback stopped." ), StatusBarNG::Warning );
            error() << "Stopping playlist.";
            failures = 0;
            m_trackError = true;
        }
    }
    else if ( currentState == Phonon::PlayingState )
    {
        if ( failures > 0 )
        {
            debug() << "Successfully played track. Resetting failure count.";
        }
        failures = 0;
        m_trackError = false;
    }
}


void
Playlist::Actions::engineNewTrackPlaying()
{
    Model* model = Model::instance();
    Meta::TrackPtr track = The::engineController()->currentTrack();
    if ( track ) {
        if ( model->containsId( m_nextTrackCandidate ) && track == model->trackForId( m_nextTrackCandidate ) ) {
            model->setActiveId( m_nextTrackCandidate );
        } else {
            warning() << "engineNewTrackPlaying:" << track->prettyName() << "does not match what the playlist controller thought it should be";
            if ( model->activeTrack() != track )
                model->setActiveRow( model->rowForTrack( track ) ); // this will set active row to -1 if the track isn't in the playlist at all
        }
    } else {
        warning() << "engineNewTrackPlaying: not really a track";
    }

    m_nextTrackCandidate = 0;
}

namespace The
{
    AMAROK_EXPORT Playlist::Actions* playlistActions() { return Playlist::Actions::instance(); }
}
