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

#include "amarok.h"
#include "debug.h"
#include "context/Svg.h"

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
    , m_currentLabel( 0 )
    , m_wikiPage( 0 )
{

    setHasConfigurationInterface( false );

}

WikipediaApplet::~ WikipediaApplet()
{
    //hacky stuff to keep QWebView from causing a crash
    m_wikiPage->setWidget( 0 );
    delete m_wikiPage;
    m_wikiPage = 0;
    delete m_webView;
}

void WikipediaApplet::init()
{
    
    dataEngine( "amarok-wikipedia" )->connectSource( "wikipedia", this );
    
    //m_theme = new Context::Svg( "widgets/amarok-wikipedia", this );
    //m_theme->setContentType( Context::Svg::SingleImage );

    m_header = new Context::Svg( "widgets/amarok-wikipedia", this );
    m_header->setContentType( Context::Svg::SingleImage );
    
    //m_theme->resize();
    m_header->resize();
    m_aspectRatio = (qreal)m_header->size().height()
        / (qreal)m_header->size().width();
    m_size = m_header->size();
    

    m_wikipediaLabel = new QGraphicsSimpleTextItem( this );
    m_currentLabel = new QGraphicsSimpleTextItem( this );
    m_currentTitle = new QGraphicsSimpleTextItem( this );
    
    m_webView = new QWebView();
    m_wikiPage = new QGraphicsProxyWidget( this );
    m_wikiPage->setWidget( m_webView );
    
    QFont labelFont;
    labelFont.setBold( true );
    labelFont.setPointSize( labelFont.pointSize() + 3 );
    m_wikipediaLabel->setBrush( Qt::white );
    m_wikipediaLabel->setFont( labelFont );
    m_wikipediaLabel->setText( i18n( "Wikipedia" ) );
    
    m_currentLabel->setBrush( Qt::white );
    labelFont.setBold( false );
    m_currentLabel->setFont( labelFont );

    m_currentTitle->setBrush( Qt::white );
    m_currentTitle->setFont( labelFont );

    constraintsUpdated();

}

void WikipediaApplet::constraintsUpdated( Plasma::Constraints constraints )
{

    kDebug() << "WikipediaApplet::constraintsUpdated start";
    
    prepareGeometryChange();
    if ( constraints & Plasma::SizeConstraint && m_header )
    {
        m_header->resize(contentSize().toSize());
    }
    
    m_wikipediaLabel->setPos( m_header->elementRect( "wikipedialabel" ).topLeft() );
    m_wikipediaLabel->setFont( shrinkTextSizeToFit( "Wikipedia", m_header->elementRect( "wikipedialabel" ) ) );

    //center it
    float textWidth = m_wikipediaLabel->boundingRect().width();
    float totalWidth = m_header->elementRect( "wikipedialabel" ).width();
    float offsetX =  ( totalWidth - textWidth ) / 2;
    m_wikipediaLabel->setPos( m_header->elementRect( "Wikipedia" ).topLeft() + QPointF ( offsetX, 0 ) );

    
    m_currentLabel->setPos( m_header->elementRect( "titlelabel" ).topLeft() );
    m_currentLabel->setFont( shrinkTextSizeToFit( m_label, m_header->elementRect( "titlelabel" ) ) );
    m_currentLabel->setText( truncateTextToFit( m_label, m_currentLabel->font(), m_header->elementRect( "titlelabel" ) ) );
    
    m_currentTitle->setPos( m_header->elementRect( "title" ).topLeft() );
    m_currentTitle->setFont( shrinkTextSizeToFit( m_title, m_header->elementRect( "title" ) ) );
    m_currentTitle->setText( truncateTextToFit( m_title, m_currentTitle->font(), m_header->elementRect( "title" ) ) );


    kDebug() << "wikipedialabel top left: " <<  m_header->elementRect( "wikipedialabel" ).topLeft();

    m_wikiPage->setPos( m_header->elementRect( "wikipediainformation" ).topLeft() );

    QSizeF infoSize( m_header->elementRect( "wikipediainformation" ).bottomRight().x() - m_header->elementRect( "wikipediainformation" ).topLeft().x(), m_header->elementRect( "wikipediainformation" ).bottomRight().y() - m_header->elementRect( "wikipediainformation" ).topLeft().y() );

    if ( infoSize.isValid() ) {
        m_wikiPage->setMinimumSize( infoSize );
        m_wikiPage->setMaximumSize( infoSize );
    }

    m_wikiPage->show();

    kDebug() << "m_wikiPage top left: " <<  m_wikiPage->boundingRect().topLeft();


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
    DEBUG_BLOCK
    Q_UNUSED( name )

    //debug() << "got data from engine:" << data;
    if( data.size() == 0 ) return;


    
    if( data.contains( "page" ) ) {
        m_webView->setHtml( data[ "page" ].toString() );
    } else {
        m_webView->setHtml( data[ data.keys()[ 0 ] ].toString() ); // set data
    }

    if( data.contains( "label" ) )
        m_label = data[ "label" ].toString() + ':';
    else
        m_label = QString();

    if( data.contains( "title" ) )
        m_title = data[ "title" ].toString();
    else
        m_title = QString();

    
    debug() << "label:" << m_label;
    debug() << "title:" << m_title;


    m_currentLabel->setText( truncateTextToFit( m_label, m_currentLabel->font(), m_header->elementRect( "titlelabel" ) ) );
    m_currentTitle->setText( truncateTextToFit( m_title, m_currentTitle->font(), m_header->elementRect( "title" ) ) );

}

void WikipediaApplet::paintInterface(  QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect )
{
    Q_UNUSED( option );

    kDebug() << "WikipediaApplet::paintInterface start";

   m_header->resize(contentSize().toSize());
    
    p->save();
    m_header->paint( p, contentsRect/*, "header" */);
    p->restore();

    kDebug() << "1";
    
   /* m_wikiPage->setPos( m_header->elementRect( "wikipediainformation" ).topLeft() );
    
    QSizeF infoSize( m_header->elementRect( "wikipediainformation" ).bottomRight().x() - m_header->elementRect( "wikipediainformation" ).topLeft().x(), m_header->elementRect( "wikipediainformation" ).bottomRight().y() - m_header->elementRect( "wikipediainformation" ).topLeft().y() );
    //infoSize.setHeight(  infoSize.height() - 50 );

    kDebug() << "2";
    
    m_wikiPage->setMinimumSize( infoSize );
    m_wikiPage->setMaximumSize( infoSize );
    m_wikiPage->show();*/

    kDebug() << "WikipediaApplet::paintInterface end";

}



#include "WikipediaApplet.moc"
