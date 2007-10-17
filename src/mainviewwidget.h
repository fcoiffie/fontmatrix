/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MAINVIEWWIDGET_H
#define MAINVIEWWIDGET_H

#include <ui_mainview.h>

#include <QStringList>

class QGraphicsScene;
class typotek;
class FontItem;
class FontActionWidget;
class QTextEdit;

/**
MainViewWidget inherits from an ui designed.

	@author Pierre Marchand <pierre@oep-h.com>
*/
class MainViewWidget :  public QWidget, private Ui::MainView
{
		Q_OBJECT

	public:
		MainViewWidget ( QWidget *parent );

		~MainViewWidget();
	private:
		QGraphicsScene *abcScene;
		QGraphicsScene *loremScene;
		QStringList ord;
		QStringList fields;
		typotek *typo;
		QString faceIndex;
		QString lastIndex;
		QList<FontItem*> currentFonts;
		FontActionWidget *currentFaction;
		QString sampleText;
		
		void allActivation(bool act);
		
	public slots:
		void slotOrderingChanged ( QString s );
		void slotfontSelected ( QTreeWidgetItem * item, int column );
		void slotInfoFont();
		void slotView();
		void slotglyphInfo();
		void slotSearch();
		void slotFontAction(QTreeWidgetItem * item, int column );
		void slotEditAll();
		void slotCleanFontAction();
		void slotZoom(int z);
		void slotAppendTag(QString tag);
		void slotFilterTag(QString tag);
		void slotDesactivateAll();
		void slotActivateAll();
		void slotSetSampleText();
	signals:
		void faceChanged();

	public:
		QString defaultOrd() {return ord[0];};

};

#endif