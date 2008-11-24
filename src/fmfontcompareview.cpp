//
// C++ Implementation: fmfontcompareview
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmfontcompareview.h"

#include "fontitem.h"

#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsEllipseItem>
#include <QSettings>
#include <QDebug>

/// Item
FMFontCompareItem::FMFontCompareItem()
	:uuid(QUuid::createUuid()), scene(0),font(0),zindex(0),char_code(0),path(0)
{
	// Nothing special :)
// 	qDebug()<< "Create" <<uuid.toString();
}

FMFontCompareItem::FMFontCompareItem(QGraphicsScene * s, FontItem * f, int z)
	:uuid(QUuid::createUuid()), scene(s), font(f), zindex(z),char_code(0),path(0)
{
// 	qDebug()<< "Create" <<uuid.toString();
	color.setRgb(qrand() % 255,qrand() % 255, qrand() % 255, 255);
}

FMFontCompareItem::~ FMFontCompareItem()
{
	clear();
}

void FMFontCompareItem::clear()
{
// 	qDebug()<< "Clearing" <<uuid.toString();
	
	foreach(QGraphicsLineItem* li, lines_controls)
	{
		delete li;
	}
	lines_controls.clear();
	foreach(QGraphicsLineItem* li, lines_metrics)
	{
		delete li;
	}
	lines_metrics.clear();
	foreach(QGraphicsEllipseItem *ri, points)
	{
		delete ri;
	}
	points.clear();
	if(path)
	{
		delete path;
		path = 0;
	}
}

void FMFontCompareItem::toScreen()
{
	if(!scene)
		return;
	foreach(QGraphicsLineItem* li, lines_controls)
	{
		scene->addItem(li);
		li->setZValue(zindex);
		
	}
	foreach(QGraphicsLineItem* li, lines_metrics)
	{
		scene->addItem(li);
		li->setZValue(zindex);
	}
	foreach(QGraphicsEllipseItem *ri, points)
	{
		scene->addItem(ri);
		ri->setZValue(zindex);
	}
	if(path)
	{
		scene->addItem(path);
		path->setZValue(zindex);
	}
}

QRectF FMFontCompareItem::boundingRect()
{
	if(!path)
		return QRectF();
	
	return path->path().boundingRect();
}

void FMFontCompareItem::drawPoint(QPointF point , bool control)
{
	double u = control ? 2.0 : 4.0;

	QRectF r(point.x()-u,point.y()-u,2*u,2*u);
	QGraphicsEllipseItem *ri = new QGraphicsEllipseItem(r);
	ri->setBrush(Qt::NoBrush);
	if(control)
		ri->setPen(FMFontCompareView::pens["control-point"]);
	else
		ri->setPen(FMFontCompareView::pens["point"]);
		
	
	points << ri;
}

void FMFontCompareItem::show(FMFontCompareItem::GElements elems)
{
	// As it’s lightweight graphic, no need to be too much circonvoluted
	clear();
	if(!font)
		return;
	
	path = font->itemFromChar(char_code, 1000.0);
	if(!path)
		return;
	path->setPen(QPen(color));
	
	if(elems.testFlag(Fill))
	{
		path->setBrush(FMFontCompareView::brushes["fill"]);
	}
	else
		path->setBrush(QBrush());
	
	if(elems.testFlag(Points))
	{
		QPointF curPos;
		for (int i = 0; i < path->path().elementCount(); ++i) 
		{
			const QPainterPath::Element &cur = path->path().elementAt(i);
 	
			if(cur.isMoveTo())
			{
				curPos = cur;
			}
			else if(cur.isLineTo())
			{
				drawPoint(curPos,false);
				curPos = cur;
			}
			else if(cur.isCurveTo())
			{
				const QPainterPath::Element &c1 = path->path().elementAt(i + 1);
				const QPainterPath::Element &c2 = path->path().elementAt(i + 2);
				
				drawPoint(curPos,false);
				drawPoint(c2,false);
				if(elems.testFlag(Controls))
				{
					drawPoint(cur,true);
					drawPoint(c1,true);
					QLineF l1(curPos, cur);
					QLineF l2(c2, c1);
					lines_controls << new QGraphicsLineItem(l1);
					lines_controls.last()->setPen(FMFontCompareView::pens["control-line"]);
					lines_controls << new QGraphicsLineItem(l2);
					lines_controls.last()->setPen(FMFontCompareView::pens["control-line"]);
				}
				
				i += 2;
				curPos = c2;
			}
			else
				qDebug()<<"Unknown point type"<<cur.type;
		}
 	
	}
	
	if(elems.testFlag(Metrics))
	{
		// TODO draw metrics in compare fonts.
	}
	
	toScreen();
}


/// View
QMap<QString, QPen> FMFontCompareView::pens;
QMap<QString, QBrush> FMFontCompareView::brushes;

FMFontCompareView::FMFontCompareView(QWidget * parent)
	:QGraphicsView(parent)
{
	setScene(new QGraphicsScene(this));
	setRenderHint(QPainter::Antialiasing,true);
	initPensAndBrushes();
}

FMFontCompareView::~ FMFontCompareView()
{
}

void FMFontCompareView::changeFont(int level, FontItem * font)
{
	glyphs[level] = new FMFontCompareItem(scene(), font, level);
	updateGlyphs();
}

void FMFontCompareView::removeFont(int level)
{
	if(glyphs.contains(level))
	{
		if(glyphs[level])
			delete glyphs[level];
		glyphs.remove(level);
	}
	elements.remove(level);
	int maxLevel(0);
	foreach(int l, glyphs.keys())
	{
		maxLevel = qMax(maxLevel, l);
	}
	for(int i(level+1); i <= maxLevel; ++i)
	{
		if(glyphs.contains(i))
		{
			glyphs[i-1] = glyphs[i];
			glyphs.remove(i);
			elements[i-1] = elements[i];
			elements.remove(i);
		}
	}
	
}

void FMFontCompareView::changeChar(uint ccode)
{
	thechar = ccode;
	foreach(int l, glyphs.keys())
	{
		glyphs[l]->setChar(thechar);
	}
	updateGlyphs();
}

void FMFontCompareView::setElements(int level, FMFontCompareItem::GElements elems)
{
	qDebug()<<"FMFontCompareView::setElements"<<level<<elems;
	elements[level] = elems;
	updateGlyphs();
}

FMFontCompareItem::GElements FMFontCompareView::getElements(int level)
{
	return elements[level];
}

void FMFontCompareView::initPensAndBrushes()
{
	// Controls
	pens["control-line"] = QPen(QColor(20,20,20), 1.0, Qt::DotLine , Qt::FlatCap, Qt::MiterJoin);
	pens["control-point"] = QPen(Qt::black);
	pens["point"] = QPen(Qt::red);
	
	// Fill
	brushes["fill"] = QBrush(QColor(0,0,0,32));
	pens["contour"] = QPen(QColor(20,0,200), 1.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	
// 	QString cat("Compare/%1");
// 	QSettings settings;
// 	foreach(QString attr, pens.keys())
// 	{
// 		pens[attr].setColor( QColor(settings.value(cat.arg(attr),pens[attr].color().name()).toString()) );
// 		settings.setValue(cat.arg(attr),pens[attr].color().name());
// 	}
// 	foreach(QString attr, brushes.keys())
// 	{
// 		brushes[attr].setColor( QColor(settings.value(cat.arg(attr),brushes[attr].color().name()).toString()) );
// 		settings.setValue(cat.arg(attr),pens[attr].color().name());
// 	}
}

void FMFontCompareView::updateGlyphs()
{
	QRectF maxrect;
	foreach(int l, glyphs.keys())
	{
		glyphs[l]->show(elements[l]);
		maxrect = maxrect.united(glyphs[l]->boundingRect());
	}
	double hratio(static_cast<double>(width()) / maxrect.width());
	double vratio(static_cast<double>(height()) / maxrect.height());
	double shape_ratio(qMin(hratio, vratio) * 0.9);
	double view_ratio(qMin(hratio, vratio) * 1.5);

	setMatrix(QMatrix(),false);
	scale(shape_ratio, shape_ratio);
	
	QMatrix m;
	m.scale(view_ratio, view_ratio );
	QRectF vr(m.mapRect(maxrect));
	ensureVisible(vr);
	
// 	qDebug()<<"M"<<maxrect;
// 	qDebug()<<"S"<<vr;
// 	qDebug()<<"V"<<rect();
// 	qDebug()<<"VR"<<vr;
// 	qDebug()<<"MR"<<maxrect;
	
}

QColor FMFontCompareView::getColor(int level)
{
	if(glyphs.contains(level))
		return glyphs[level]->getColor();
	return QColor();
}







