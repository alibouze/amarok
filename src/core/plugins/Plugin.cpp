/****************************************************************************************
 * Copyright (c) 2004 Mark Kretschmann <kretschmann@kde.org>                            *
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

#include "core/plugins/Plugin.h"

namespace Plugins {


Plugin::Plugin()
{}


Plugin::~Plugin()
{}


void
Plugin::addPluginProperty( const QString& key, const QString& value )
{
    m_properties[key.toLower()] = value;
}


QString
Plugin::pluginProperty( const QString& key )
{
    if ( m_properties.find( key.toLower() ) == m_properties.end() )
        return "false";

    return m_properties[key.toLower()];
}


bool
Plugin::hasPluginProperty( const QString& key )
{
    return m_properties.find( key.toLower() ) != m_properties.end();
}

}
