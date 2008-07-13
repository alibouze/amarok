/***************************************************************************
 * copyright            : (C) 2007 Leo Franchi <lfranchi@gmail.com>        *
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "WikipediaApplet.h"

#include "Amarok.h"
#include "Debug.h"
#include "context/Svg.h"
#include <plasma/theme.h>

#include <QGraphicsTextItem>
#include <QGraphicsSimpleTextItem>
#include <QPainter>

WikipediaApplet::WikipediaApplet( QObject* parent, const QVariantList& args )
    : Context::Applet( parent, args )
    , m_theme( 0 )
    , m_header( 0 )
    , m_aspectRatio( 0 )
    , m_headerAspectRatio( 0.0 )
    , m_size( QSizeF() )
    , m_wikipediaLabel( 0 )
    , m_webView( 0 )
{

    setHasConfigurationInterface( false );

}

WikipediaApplet::~ WikipediaApplet()
{
    //hacky stuff to keep QWebView from causing a crash
   /* m_wikiPage->setWidget( 0 );
    delete m_wikiPage;
    m_wikiPage = 0;
    delete m_webView;*/
    delete m_webView;
}

void WikipediaApplet::init()
{

    m_header = new Context::Svg( this );
    m_header->setImagePath( "widgets/amarok-wikipedia" );
    m_header->setContainsMultipleImages( false );

    m_header->resize();
    m_aspectRatio = (qreal)m_header->size().height()
        / (qreal)m_header->size().width();
    m_size = m_header->size();

    m_wikipediaLabel = new QGraphicsSimpleTextItem( this );

    m_webView = new Plasma::WebContent( this );


    //make background transparent

    QPalette p = m_webView->palette();
    p.setColor( QPalette::Dark, QColor( 255, 255, 255, 0)  );
    m_webView->setPalette( p );

    
    p = m_webView->page()->palette();
    p.setColor( QPalette::Window, QColor( 255, 255, 255, 0)  );
    m_webView->page()->setPalette( p );


    //m_wikiPage = new QGraphicsProxyWidget( this );
    //m_wikiPage->setWidget( m_webView );

    QFont labelFont;
    labelFont.setBold( true );
    labelFont.setPointSize( labelFont.pointSize() + 3 );
    m_wikipediaLabel->setBrush( Plasma::Theme::defaultTheme()->color( Plasma::Theme::TextColor ) );
    m_wikipediaLabel->setFont( labelFont );
    m_wikipediaLabel->setText( i18n( "Wikipedia" ) );

    dataEngine( "amarok-wikipedia" )->connectSource( "wikipedia", this );
    
    constraintsEvent();

}

void WikipediaApplet::constraintsEvent( Plasma::Constraints constraints )
{
    prepareGeometryChange();
    if ( constraints & Plasma::SizeConstraint && m_header )
    {
        m_header->resize(size().toSize());
    }

    float textWidth = m_wikipediaLabel->boundingRect().width();
    float totalWidth = m_header->elementRect( "wikipedialabel" ).width();
    float offsetX =  ( totalWidth - textWidth ) / 2;

    m_wikipediaLabel->setPos( offsetX, m_header->elementRect( "wikipedialabel" ).topLeft().y() );
    m_wikipediaLabel->setFont( shrinkTextSizeToFit( "Wikipedia", m_header->elementRect( "wikipedialabel" ) ) );

    m_webView->setPos( m_header->elementRect( "wikipediainformation" ).topLeft() );

    QSizeF infoSize( m_header->elementRect( "wikipediainformation" ).bottomRight().x() - m_header->elementRect( "wikipediainformation" ).topLeft().x(), m_header->elementRect( "wikipediainformation" ).bottomRight().y() - m_header->elementRect( "wikipediainformation" ).topLeft().y() );

    if ( infoSize.isValid() ) {
        m_webView->resize( infoSize );
    }

        //m_webView->show();
    }

bool WikipediaApplet::hasHeightForWidth() const
{
    return true;
}

qreal WikipediaApplet::heightForWidth( qreal width ) const
{
    return width * m_aspectRatio;;
}


void WikipediaApplet::dataUpdated( const QString& name, const Plasma::DataEngine::Data& data ) // SLOT
{

    kDebug() << "WikipediaApplet::dataUpdated: " << name;
    Q_UNUSED( name )

    if( data.size() == 0 ) return;

    if( data.contains( "page" ) ) {
        m_webView->setHtml( data[ "page" ].toString().toUtf8(), KUrl( QString() ) );
    } else {
        m_webView->setHtml( data[ data.keys()[ 0 ] ].toString().toUtf8(), KUrl( QString() ) ); // set data

    }

    if( data.contains( "label" ) )
        m_label = data[ "label" ].toString() + ':';
    else
        m_label = QString();

    if( data.contains( "title" ) )
        m_title = data[ "title" ].toString();
    else
        m_title = QString();
}

void WikipediaApplet::paintInterface(  QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect )
{
    Q_UNUSED( option );

    m_header->resize(size().toSize());

    p->save();
    m_header->paint( p, contentsRect/*, "header" */);
    p->restore();
}



#include "WikipediaApplet.moc"


QSizeF WikipediaApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if( constraint.height() == -1 && constraint.width() > 0 ) // asking height for given width basically
    {
        return QSizeF( constraint.width(), m_aspectRatio * constraint.width() );
    } else
    {
        return constraint;
    }
}

