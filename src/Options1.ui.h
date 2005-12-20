//Released under GPLv2 or later. (C) 2005 Ian Monroe <ian@monroe.nu>
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <config.h>

#include "amarokconfig.h"
#include <kstandarddirs.h>


void Options1::init()
{
#ifndef HAVE_EXSCALIBAR
    moodFrame->hide();
#endif

    QStringList browsers;
    browsers << "konqueror" << "firefox" << "opera" << "galeon" << "epiphany"
             << "safari" << "mozilla";

    // Remove browsers which are not actually installed
    for( QStringList::Iterator it = browsers.begin(), end = browsers.end(); it != end; ) {
        if( KStandardDirs::findExe( *it ) == QString::null )
            it = browsers.erase( it );
        else
            ++it;
    }

    kComboBox_browser->insertStringList( browsers );
    kComboBox_browser->setCurrentItem( browsers.findIndex( AmarokConfig::externalBrowser() ) );
}


void Options1::slotUpdateMoodFrame()
{
    kcfg_MakeMoodier->setEnabled(kcfg_ShowMoodbar->isChecked());
    kcfg_AlterMood->setEnabled(kcfg_ShowMoodbar->isChecked() && kcfg_MakeMoodier->isChecked());
    kcfg_MoodsWithMusic->setEnabled(kcfg_ShowMoodbar->isChecked());
}
