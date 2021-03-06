/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/***************************************************************************
                          cpalette.cpp  -  description
                             -------------------
    begin                : Wed Apr 25 2001
    copyright            : (C) 2001 by Franz Schmid
    email                : Franz.Schmid@altmuehlnet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cpalette.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QRect>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStandardItem>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QCheckBox>

#include "colorlistbox.h"
#include "sccombobox.h"
#include "scribusdoc.h"
#include "scrspinbox.h"
#include "gradienteditor.h"
#include "units.h"
#include "page.h"
#include "pageitem.h"
#include "util_icon.h"
#include "commonstrings.h"
#include "linkbutton.h"
#include "sccolorengine.h"
#include "scpattern.h"

GradientVectorDialog::GradientVectorDialog(QWidget* parent) : ScrPaletteBase( parent, "GradientVectorPalette", false, 0 )
{
	freeGradientLayout = new QGridLayout(this);
	freeGradientLayout->setMargin(5);
	freeGradientLayout->setSpacing(5);
	GTextX1 = new QLabel("X1:", this );
	freeGradientLayout->addWidget( GTextX1, 0, 0 );
	GTextY1 = new QLabel("Y1:", this );
	freeGradientLayout->addWidget( GTextY1, 1, 0 );
	gX1 = new ScrSpinBox( -3000, 3000, this, 0);
	freeGradientLayout->addWidget( gX1, 0, 1 );
	gY1 = new ScrSpinBox( -3000, 3000, this, 0);
	freeGradientLayout->addWidget( gY1, 1, 1 );
	GTextX2 = new QLabel("X2:", this );
	freeGradientLayout->addWidget( GTextX2, 0, 2 );
	GTextY2 = new QLabel("Y2:", this );
	freeGradientLayout->addWidget( GTextY2, 1, 2 );
	gX2 = new ScrSpinBox( -3000, 3000, this, 0);
	freeGradientLayout->addWidget( gX2, 0, 3 );
	gY2 = new ScrSpinBox( -3000, 3000, this, 0);
	freeGradientLayout->addWidget( gY2, 1, 3 );
	connect(gX1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gX2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gY1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gY2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	languageChange();
}

void GradientVectorDialog::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
	}
	else
		QWidget::changeEvent(e);
}

void GradientVectorDialog::languageChange()
{
	setWindowTitle( tr( "Gradient Vector" ));
	resize(minimumSizeHint());
}

void GradientVectorDialog::setValues(double x1, double y1, double x2, double y2)
{
	disconnect(gX1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	disconnect(gX2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	disconnect(gY1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	disconnect(gY2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	gX1->setValue(x1);
	gX2->setValue(x2);
	gY1->setValue(y1);
	gY2->setValue(y2);
	connect(gX1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gX2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gY1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gY2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
}

void GradientVectorDialog::changeSpecial()
{
	emit NewSpecial(gX1->value(), gY1->value(), gX2->value(), gY2->value());
}

void GradientVectorDialog::unitChange(int unitIndex)
{
	disconnect(gX1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	disconnect(gX2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	disconnect(gY1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	disconnect(gY2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	gX1->setNewUnit(unitIndex);
	gY1->setNewUnit(unitIndex);
	gX2->setNewUnit(unitIndex);
	gY2->setNewUnit(unitIndex);
	connect(gX1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gX2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gY1, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
	connect(gY2, SIGNAL(valueChanged(double)), this, SLOT(changeSpecial()));
}

Cpalette::Cpalette(QWidget* parent) : QWidget(parent)
{
	Color = "";
	Color3 = "";
	Shade = 100;
	Shade3 = 100;
	currentGradient = 0;
	currentItem = NULL;
	patternList = NULL;
	CGradDia = NULL;
	CGradDia = new GradientVectorDialog(this->parentWidget());
	CGradDia->hide();
	Form1Layout = new QVBoxLayout(this);
	Form1Layout->setMargin(0);
	Form1Layout->setSpacing(0);
	Layout1 = new QHBoxLayout;
	Layout1->setSpacing( 4 );
	Layout1->setMargin( 1 );
	editLineColorSelector = new QToolButton(this);
	editLineColorSelector->setIcon(QIcon(loadIcon("16/color-stroke.png")));
	editLineColorSelector->setCheckable(true);
	editLineColorSelector->setAutoExclusive(true); // #7323
	Layout1->addWidget(editLineColorSelector);
	editFillColorSelector = new QToolButton(this);
	editFillColorSelector->setIcon(QIcon(loadIcon("16/color-fill.png")));
	editFillColorSelector->setCheckable(true);
	editFillColorSelector->setAutoExclusive(true); // #7323
	editFillColorSelector->setChecked(true);
	Layout1->addWidget(editFillColorSelector);
	selectorQSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	Layout1->addItem( selectorQSpacer );
	Mode = 2;
	ShadeTxt = new QLabel( this);
	Layout1->addWidget( ShadeTxt);
	PM1 = new QSpinBox( this );
	PM1->setMinimum(0);
	PM1->setMaximum(100);
	PM1->setSingleStep(10);
	PM1->setValue(100);
	Layout1->addWidget(PM1);
	Form1Layout->addLayout(Layout1);
	GradLayout = new QVBoxLayout;
	GradLayout->setMargin(0);
	GradLayout->setSpacing(5);
	QFont fo = QFont(font());
	gradientQCombo = new ScComboBox( this );
#ifndef Q_WS_WIN 
	fo.setPointSize(fo.pointSize()-1);
	gradientQCombo->setFont(fo);
#endif
	GradLayout->addWidget( gradientQCombo );
	gradEdit = new GradientEditor(this);
	GradLayout->addWidget(gradEdit, Qt::AlignHCenter);
/*	freeGradientQFrame = new QFrame( this );
	freeGradientQFrame->setFrameShape( QFrame::NoFrame );
	freeGradientQFrame->setFrameShadow( QFrame::Plain );
	freeGradientLayout = new QGridLayout( freeGradientQFrame);
	GTextX1 = new QLabel("X1:", freeGradientQFrame );
	freeGradientLayout->addWidget( GTextX1, 0, 0 );
	GTextY1 = new QLabel("Y1:", freeGradientQFrame );
	freeGradientLayout->addWidget( GTextY1, 1, 0 );
	gX1 = new ScrSpinBox( -3000, 3000, freeGradientQFrame, 0);
	freeGradientLayout->addWidget( gX1, 0, 1 );
	gY1 = new ScrSpinBox( -3000, 3000, freeGradientQFrame, 0);
	freeGradientLayout->addWidget( gY1, 1, 1 );
	GTextX2 = new QLabel("X2:", freeGradientQFrame );
	freeGradientLayout->addWidget( GTextX2, 0, 2 );
	GTextY2 = new QLabel("Y2:", freeGradientQFrame );
	freeGradientLayout->addWidget( GTextY2, 1, 2 );
	gX2 = new ScrSpinBox( -3000, 3000, freeGradientQFrame, 0);
	freeGradientLayout->addWidget( gX2, 0, 3 );
	gY2 = new ScrSpinBox( -3000, 3000, freeGradientQFrame, 0);
	freeGradientLayout->addWidget( gY2, 1, 3 );
	gradEditButton = new QToolButton(freeGradientQFrame);
	gradEditButton->setCheckable(true);
	freeGradientLayout->addWidget(gradEditButton, 2, 0, 1, 4);
	GradLayout->addWidget( freeGradientQFrame ); */
	gradEditButton = new QToolButton(this);
	gradEditButton->setCheckable(true);
	GradLayout->addWidget(gradEditButton);
	Form1Layout->setMargin(5);
	Form1Layout->setSpacing(5);
	Form1Layout->addLayout(GradLayout);
	colorListQLBox = new ColorListBox(this);
	colorListQLBox->setMinimumSize( QSize( 150, 30 ) );
	colorListQLBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	Form1Layout->addWidget(colorListQLBox);
	displayAllColors = new QCheckBox( this );
	displayAllColors->setText( tr( "Display only used Colors" ) );
	displayAllColors->setChecked(false);
	Form1Layout->addWidget(displayAllColors);

	patternFrame = new QFrame( this );
	patternFrame->setFrameShape( QFrame::NoFrame );
	frame3Layout = new QVBoxLayout( patternFrame );
	frame3Layout->setMargin(0);
	frame3Layout->setSpacing(2);
    patternBox = new QListWidget(patternFrame);
    patternBox->setFlow(QListView::LeftToRight);
    patternBox->setWrapping(true);
	patternBox->setWordWrap(true);
    patternBox->setResizeMode(QListView::Adjust);
    patternBox->setViewMode(QListView::IconMode);
	patternBox->setMinimumSize( QSize( 150, 30 ) );
	patternBox->setSelectionMode(QAbstractItemView::SingleSelection);
	patternBox->setDragDropMode(QAbstractItemView::NoDragDrop);
	patternBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	frame3Layout->addWidget( patternBox );

	groupOffset = new QGroupBox( patternFrame );
	groupOffsetLayout = new QHBoxLayout( groupOffset );
	groupOffsetLayout->setSpacing( 2 );
	groupOffsetLayout->setMargin( 3 );
	groupOffsetLayout->setAlignment( Qt::AlignTop );
	textLabel1 = new QLabel( groupOffset );
	groupOffsetLayout->addWidget( textLabel1 );
	spinXoffset = new ScrSpinBox( -3000, 3000, groupOffset, 0);
	groupOffsetLayout->addWidget( spinXoffset );
	textLabel2 = new QLabel( groupOffset );
	groupOffsetLayout->addWidget( textLabel2 );
	spinYoffset = new ScrSpinBox( -3000, 3000, groupOffset, 0);
	groupOffsetLayout->addWidget( spinYoffset );
	frame3Layout->addWidget( groupOffset );

	groupScale = new QGroupBox( patternFrame );
	groupScaleLayout = new QGridLayout( groupScale );
	groupScaleLayout->setSpacing( 2 );
	groupScaleLayout->setMargin( 3 );
	groupScaleLayout->setAlignment( Qt::AlignTop );
	textLabel5 = new QLabel( groupScale );
	groupScaleLayout->addWidget( textLabel5, 0, 0 );
	spinXscaling = new ScrSpinBox( 1, 500, groupScale, 0);
	spinXscaling->setValue( 100 );
	groupScaleLayout->addWidget( spinXscaling, 0, 1 );
	textLabel6 = new QLabel( groupScale );
	groupScaleLayout->addWidget( textLabel6, 1, 0 );
	spinYscaling = new ScrSpinBox( 1, 500, groupScale, 0 );
	groupScaleLayout->addWidget( spinYscaling, 1, 1 );
	keepScaleRatio = new LinkButton( groupScale );
	keepScaleRatio->setCheckable( true );
	keepScaleRatio->setAutoRaise( true );
	keepScaleRatio->setMaximumSize( QSize( 15, 32767 ) );
	groupScaleLayout->addWidget( keepScaleRatio, 0, 2, 2, 1 );
	frame3Layout->addWidget( groupScale );

	groupRotation = new QGroupBox( patternFrame );
	groupRotationLayout = new QHBoxLayout( groupRotation );
	groupRotationLayout->setSpacing( 2 );
	groupRotationLayout->setMargin( 3 );
	groupRotationLayout->setAlignment( Qt::AlignTop );
	textLabel7 = new QLabel( groupRotation );
	groupRotationLayout->addWidget( textLabel7 );
	spinAngle = new ScrSpinBox( -180, 180, groupRotation, 6 );
	groupRotationLayout->addWidget( spinAngle );
	frame3Layout->addWidget( groupRotation );
	Form1Layout->addWidget(patternFrame);
	patternFrame->hide();

	TransGroup = new QGroupBox(this);
	Layout1t = new QGridLayout( TransGroup );
	Layout1t->setAlignment( Qt::AlignTop );
	Layout1t->setSpacing( 5 );
	Layout1t->setMargin( 5 );
	TransTxt = new QLabel( TransGroup );
	Layout1t->addWidget( TransTxt, 0, 0 );
	TransSpin = new QSpinBox( TransGroup );
	TransSpin->setMinimum(0);
	TransSpin->setMaximum(100);
	TransSpin->setSingleStep(10);
	TransSpin->setValue(100);
	Layout1t->addWidget(TransSpin, 0, 1);
	TransTxt2 = new QLabel( TransGroup );
	Layout1t->addWidget( TransTxt2, 1, 0 );
	blendMode = new ScComboBox( TransGroup );
	Layout1t->addWidget( blendMode, 1, 1 );
	Form1Layout->addWidget(TransGroup);

	editFillColorSelector->setChecked(true);
	editFillColorSelectorButton();
	GradientMode = false;
	
	setFocusPolicy(Qt::NoFocus);

	languageChange();
	Mode = 2;
	setActGradient(0);

	connect(editLineColorSelector, SIGNAL(clicked()), this, SLOT(editLineColorSelectorButton()));
	connect(editFillColorSelector, SIGNAL(clicked()), this, SLOT(editFillColorSelectorButton()));
	connect(colorListQLBox, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectColor(QListWidgetItem*)));
	connect(displayAllColors, SIGNAL(clicked()), this, SLOT(ToggleColorDisplay()));
	connect(PM1, SIGNAL(valueChanged(int)), this, SLOT(setActShade()));
	connect(gradientQCombo, SIGNAL(activated(int)), this, SLOT(slotGrad(int)));
	connect(TransSpin, SIGNAL(valueChanged(int)), this, SLOT(slotTrans(int)));
	connect(blendMode, SIGNAL(activated(int)), this, SLOT(changeBlendMode(int)));
	connect(spinXoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	connect(spinYoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	connect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	connect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
	connect(keepScaleRatio, SIGNAL(clicked()), this, SLOT(ToggleKette()));
	connect(spinAngle, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	connect(gradEdit->Preview, SIGNAL(selectedColor(QString, int )), this, SLOT(slotColor(QString, int )));
	connect(gradEdit->Preview, SIGNAL(currTrans(double )), this, SLOT(setGradTrans(double )));
	connect(gradEdit, SIGNAL(gradientChanged()), this, SIGNAL(gradientChanged()));
	connect(gradEdit->Preview, SIGNAL(gradientChanged()), this, SIGNAL(gradientChanged()));
	connect(gradEditButton, SIGNAL(clicked()), this, SLOT(editGradientVector()));
	connect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	connect(CGradDia, SIGNAL(NewSpecial(double, double, double, double)), this, SIGNAL(NewSpecial(double, double, double, double)));
	connect(CGradDia, SIGNAL(paletteShown(bool)), this, SLOT(setActiveGradDia(bool)));
}

void Cpalette::setCurrentItem(PageItem* item)
{
	currentItem = item;
}

void Cpalette::setDocument(ScribusDoc* doc)
{
	currentDoc = doc;
	if (doc == NULL)
		colorListQLBox->cList = NULL;
	else
		colorListQLBox->cList = &doc->PageColors;
}

void Cpalette::updateFromItem()
{
	if (currentItem == NULL)
		return;
	if (!currentDoc)
		return;
	gradEditButton->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
	gradEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
	patternFrame->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
	gradientQCombo->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	colorListQLBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	Color = currentItem->lineColor();
	Shade = qRound(currentItem->lineShade());
	Color3 = currentItem->fillColor();
	Shade3 = qRound(currentItem->fillShade());
	setActTrans(currentItem->fillTransparency(), currentItem->lineTransparency());
	setActBlend(currentItem->fillBlendmode(), currentItem->lineBlendmode());
	if (Mode == 1)
	{
		updateCList();
		setActFarben(Color, Color3, Shade, Shade3);
	}
	else
	{
		ChooseGrad(currentItem->GrType);
		gradEdit->Preview->fill_gradient = currentItem->fill_gradient;
		gradientQCombo->setCurrentIndex(currentItem->GrType);
		gradEdit->Preview->updateDisplay();
//		double dur = currentDoc->unitRatio();
//		setSpecialGradient(currentItem->GrStartX * dur, currentItem->GrStartY * dur, currentItem->GrEndX * dur, currentItem->GrEndY * dur);
	}
	if (patternList->count() == 0)
	{
		if (gradientQCombo->count() == 9)		// remove Pattern entry, as there are no Patterns available
			gradientQCombo->removeItem(8);
	}
	else
	{
		if (gradientQCombo->count() < 9)		// readd the Pattern entry
			gradientQCombo->addItem( tr("Pattern"));
	}
//	freeGradientLayout->activate();
	GradLayout->activate();
	Form1Layout->activate();
	layout()->activate();
	updateGeometry();
	repaint();
}

void Cpalette::editLineColorSelectorButton()
{
	if (editLineColorSelector->isChecked())
	{
		Mode = 1;
		editFillColorSelector->setChecked(false);
		gradEditButton->hide();
		gradEditButton->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
		gradientQCombo->hide();
		gradientQCombo->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
		gradEdit->hide();
		gradEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
		patternFrame->hide();
		patternFrame->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
		colorListQLBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
		colorListQLBox->show();
		displayAllColors->show();
		GradientMode = false;
//		freeGradientLayout->activate();
		GradLayout->activate();
		Form1Layout->activate();
		layout()->activate();
		gradEditButton->updateGeometry();
		gradEdit->updateGeometry();
		colorListQLBox->updateGeometry();
//		updateCList();
//		repaint();
	}
	updateFromItem();
	emit modeChanged();
}

void Cpalette::editFillColorSelectorButton()
{
	if (editFillColorSelector->isChecked())
	{
		Mode = 2;
		editLineColorSelector->setChecked(false);
		gradientQCombo->show();
		gradientQCombo->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
		GradientMode = gradientQCombo->currentIndex() != 0 ? true : false;
		if (GradientMode)
		{
			if (gradEdit->isHidden())
			{
				gradEdit->show();
				gradEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
			}
			if (gradientQCombo->currentIndex() > 5)
			{
				gradEditButton->show();
				gradEditButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
			}
			else
			{
				gradEditButton->hide();
				gradEditButton->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
			}
		}
//		freeGradientLayout->activate();
		GradLayout->activate();
		Form1Layout->activate();
		layout()->activate();
		gradEditButton->updateGeometry();
		gradEdit->updateGeometry();
		colorListQLBox->updateGeometry();
		updateGeometry();
//		updateCList();
//		repaint();
	}
	updateFromItem();
	emit modeChanged();
}

void Cpalette::updatePatternList()
{
	disconnect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	patternBox->clear();
	patternBox->setIconSize(QSize(48, 48));
	for (QMap<QString, ScPattern>::Iterator it = patternList->begin(); it != patternList->end(); ++it)
	{
		QPixmap pm;
		if (it.value().getPattern()->width() >= it.value().getPattern()->height())
			pm=QPixmap::fromImage(it.value().getPattern()->scaledToWidth(48, Qt::SmoothTransformation));
		else
			pm=QPixmap::fromImage(it.value().getPattern()->scaledToHeight(48, Qt::SmoothTransformation));
		QPixmap pm2(48, 48);
		pm2.fill(palette().color(QPalette::Base));
		QPainter p;
		p.begin(&pm2);
		p.drawPixmap(24 - pm.width() / 2, 24 - pm.height() / 2, pm);
		p.end();
		QListWidgetItem *item = new QListWidgetItem(pm2, it.key(), patternBox);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
	if (patternList->count() == 0)
	{
		if (gradientQCombo->count() == 9)		// remove Pattern entry, as there are no Patterns available
			gradientQCombo->removeItem(8);
	}
	else
	{
		if (gradientQCombo->count() < 9)		// readd the Pattern entry
			gradientQCombo->addItem( tr("Pattern"));
	}
	patternBox->clearSelection();
	connect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
}

void Cpalette::SetPatterns(QMap<QString, ScPattern> *docPatterns)
{
	patternList = docPatterns;
	updatePatternList();
}

void Cpalette::selectPattern(QListWidgetItem *c)
{
	if (c == NULL)
		return;
	emit NewPattern(c->text());
}

void Cpalette::changePatternProps()
{
	emit NewPatternProps(spinXscaling->value(), spinYscaling->value(), spinXoffset->value(), spinYoffset->value(), spinAngle->value());
}

void Cpalette::ToggleKette()
{
	disconnect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	disconnect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
	if (keepScaleRatio->isChecked())
	{
		spinYscaling->setValue(spinXscaling->value());
		changePatternProps();
		keepScaleRatio->setChecked(true);
	}
	else
		keepScaleRatio->setChecked(false);
	connect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	connect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
}

void Cpalette::HChange()
{
	disconnect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	disconnect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
	if (keepScaleRatio->isChecked())
		spinYscaling->setValue(spinXscaling->value());
	changePatternProps();
	connect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	connect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
}

void Cpalette::VChange()
{
	disconnect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	disconnect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
	if (keepScaleRatio->isChecked())
		spinXscaling->setValue(spinYscaling->value());
	changePatternProps();
	connect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	connect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
}

void Cpalette::SetColors(ColorList newColorList)
{
	colorList.clear();
	colorList = newColorList;
	updateCList();
}

void Cpalette::updateCList()
{
	disconnect(colorListQLBox, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectColor(QListWidgetItem*)));
	colorListQLBox->clear();
	if ((!GradientMode) || (Mode == 1))
		colorListQLBox->addItem(CommonStrings::tr_NoneColor);
	if (displayAllColors->isChecked())
	{
		if (currentDoc != NULL)
			currentDoc->getUsedColors(colorList);
	}
	colorListQLBox->insertItems(colorList, ColorListBox::fancyPixmap);
	if (colorListQLBox->currentItem())
		colorListQLBox->currentItem()->setSelected(false);
	connect(colorListQLBox, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectColor(QListWidgetItem*)));
}

void Cpalette::ToggleColorDisplay()
{
	if (currentDoc != NULL)
	{
		colorListQLBox->cList = &currentDoc->PageColors;
		colorList = currentDoc->PageColors;
		updateFromItem();
	}
}

void Cpalette::selectColor(QListWidgetItem *item)
{
	ColorPixmapItem* c = dynamic_cast<ColorPixmapItem*>(item);
	if (c != NULL)	
		sFarbe = c->colorName();
	else if (! item->data(Qt::DisplayRole).toString().isEmpty()) 
		sFarbe = item->data(Qt::DisplayRole).toString();
	else
		return;
	
	switch (Mode)
	{
	case 1:
		emit NewPen(sFarbe);
		break;
	case 2:
		if (gradientQCombo->currentIndex() == 0)
		{
			Color3 = sFarbe;
			emit NewBrush(sFarbe);
		}
		else if (gradientQCombo->currentIndex() < 8)
		{
			gradEdit->Preview->setActColor(setColor(sFarbe, Shade), sFarbe, Shade);
			Color = sFarbe;
			emit gradientChanged();
		}
		break;
	}
}

QColor Cpalette::setColor(QString colorName, int shad)
{
	const ScColor& color = colorList[colorName];
	return ScColorEngine::getShadeColorProof(color, currentDoc, shad);
}

void Cpalette::updateBoxS(QString colorName)
{
	disconnect(colorListQLBox, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectColor(QListWidgetItem*)));
	if ((colorName != CommonStrings::None) && (!colorName.isEmpty()))
	{
		QList<QListWidgetItem *> cCol = colorListQLBox->findItems(colorName, Qt::MatchExactly);
		if (cCol.count() != 0)
			colorListQLBox->setCurrentItem(cCol[0]);
	}
	else
		colorListQLBox->setCurrentRow(0);
	connect(colorListQLBox, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectColor(QListWidgetItem*)));
}

void Cpalette::setActFarben(QString p, QString b, int shp, int shb)
{
	disconnect(PM1, SIGNAL(valueChanged(int)), this, SLOT(setActShade()));
	switch (Mode)
	{
	case 1:
		PM1->setValue(shp);
		updateBoxS(p);
		break;
	case 2:
		Color3 = b;
		Shade3 = shb;
		PM1->setValue(shb);
		updateBoxS(b);
		break;
	}
	connect(PM1, SIGNAL(valueChanged(int)), this, SLOT(setActShade()));
}

void Cpalette::slotColor(QString n, int s)
{
	if ((GradientMode) && (Mode ==2))
	{
		disconnect(PM1, SIGNAL(valueChanged(int)), this, SLOT(setActShade()));
		Color = n;
		Shade = s;
		PM1->setValue(Shade);
		updateBoxS(Color);
		connect(PM1, SIGNAL(valueChanged(int)), this, SLOT(setActShade()));
	}
}

void Cpalette::slotGrad(int number)
{
	if ((number == 8) && (patternList->count() == 0))
	{
		gradientQCombo->setCurrentIndex(currentGradient);
		return;
	}
	int oldgrad = currentGradient;
	ChooseGrad(number);
	if ((currentGradient != 0) && (oldgrad != currentGradient))
	{
		gradEdit->Preview->fill_gradient = currentItem->fill_gradient;
		gradEdit->Preview->updateDisplay();
	}
	blendMode->setEnabled(number <= 0);
	emit NewGradient(number);
}

void Cpalette::ChooseGrad(int number)
{
	if (number==-1)
	{
		gradientQCombo->setCurrentIndex(0);
		currentGradient = 0;
	}

	currentGradient = (number > 0) ? number : 0;
	//no need to disconnect as qcombobox only emits from user action
	/* PFJ - 29.02.04 - Removed GradGroup and Gradient mode from switch */
	GradientMode = number == 0 ? false : number == 8 ? false : true;

	if (number > 0)
	{
		blendMode->setEnabled(false);
		if (number == 8)
		{
			PM1->setEnabled(false);
			gradEditButton->hide();
			gradEdit->hide();
			gradEditButton->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
			gradEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
			colorListQLBox->hide();
			colorListQLBox->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
			displayAllColors->hide();
			patternFrame->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
			patternFrame->show();
		}
		else
		{
			PM1->setEnabled(true);
			patternFrame->hide();
			patternFrame->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
			gradEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
			gradEdit->show();
			if (gradientQCombo->currentIndex() > 5)
			{
				gradEditButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
				gradEditButton->show();
			}
			else
			{
				gradEditButton->hide();
				gradEditButton->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
			}
			colorListQLBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
			colorListQLBox->show();
			displayAllColors->show();
		}
	}
	else
	{
		blendMode->setEnabled(true);
		PM1->setEnabled(true);
		patternFrame->hide();
		patternFrame->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
		gradEditButton->hide();
		gradEdit->hide();
//		freeGradientQFrame->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
		gradEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
		colorListQLBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
		colorListQLBox->show();
		displayAllColors->show();
	}
//	freeGradientLayout->activate();
	GradLayout->activate();
	Form1Layout->activate();
	layout()->activate();
	gradEditButton->updateGeometry();
	gradEdit->updateGeometry();
	colorListQLBox->updateGeometry();
	updateGeometry();
	disconnect(PM1, SIGNAL(valueChanged(int)), this, SLOT(setActShade()));
	// JG probably not needed at all and should probably not be here
	updateCList();
	if (number == 0)
	{
		PM1->setValue(Shade3);
		updateBoxS(Color3);
		if (currentItem)
		{
			setActTrans(currentItem->fillTransparency(), currentItem->lineTransparency());
			setActBlend(currentItem->fillBlendmode(), currentItem->lineBlendmode());
		}
	}
	else
	{
		PM1->setValue(Shade);
		updateBoxS(Color);
	}
//	setFocus();
	repaint();
	connect(PM1, SIGNAL(valueChanged(int)), this, SLOT(setActShade()));
}

void Cpalette::setActTrans(double val, double val2)
{
	disconnect(TransSpin, SIGNAL(valueChanged(int)), this, SLOT(slotTrans(int)));
	TransSpin->setValue(qRound(100 - (Mode == 1 ? val2 * 100 : val * 100)));
	connect(TransSpin, SIGNAL(valueChanged(int)), this, SLOT(slotTrans(int)));
}

void Cpalette::setActBlend(int val, int val2)
{
	disconnect(blendMode, SIGNAL(activated(int)), this, SLOT(changeBlendMode(int)));
	blendMode->setCurrentIndex(Mode == 1 ? val2 : val);
	connect(blendMode, SIGNAL(activated(int)), this, SLOT(changeBlendMode(int)));
}

void Cpalette::setGradTrans(double val)
{
	if ((GradientMode) && (Mode ==2))
	{
		disconnect(TransSpin, SIGNAL(valueChanged(int)), this, SLOT(slotTrans(int)));
		TransSpin->setValue(qRound(val * 100));
		connect(TransSpin, SIGNAL(valueChanged(int)), this, SLOT(slotTrans(int)));
	}
}

void Cpalette::changeBlendMode(int blend)
{
	if (Mode == 1)
		emit NewBlendS(blend);
	else
	{
		if ((gradientQCombo->currentIndex() == 0) || (gradientQCombo->currentIndex() == 8))
			emit NewBlend(blend);
	}
}

void Cpalette::slotTrans(int val)
{
	if (Mode == 1)
		emit NewTransS(static_cast<double>(100 - val) / 100.0);
	else
	{
		if ((gradientQCombo->currentIndex() == 0) || (gradientQCombo->currentIndex() == 8))
			emit NewTrans(static_cast<double>(100 - val) / 100.0);
		else
		{
			gradEdit->Preview->setActTrans(static_cast<double>(val) / 100.0);
			emit gradientChanged();
		}
	}
//	setFocus();
}

void Cpalette::setActGradient(int typ)
{
	disconnect(gradientQCombo, SIGNAL(activated(int)), this, SLOT(slotGrad(int)));
	if (Mode == 2)
	{
		currentGradient = typ;
		gradientQCombo->setCurrentIndex(typ);
		ChooseGrad(typ);
	}
	connect(gradientQCombo, SIGNAL(activated(int)), this, SLOT(slotGrad(int)));
}

void Cpalette::setSpecialGradient(double x1, double y1, double x2, double y2)
{
	if (CGradDia)
		CGradDia->setValues(x1, y1, x2, y2);
}

void Cpalette::editGradientVector()
{
	if (gradEditButton->isChecked())
	{
		double unitRatio = currentDoc->unitRatio();
		CGradDia->unitChange(currentDoc->unitIndex());
		CGradDia->setValues(currentItem->GrStartX * unitRatio, currentItem->GrStartY * unitRatio, 
		                    currentItem->GrEndX * unitRatio,   currentItem->GrEndY * unitRatio);
		CGradDia->show();
	}
	else
	{
		CGradDia->hide();
	}
	emit editGradient();
}

void Cpalette::setActiveGradDia(bool active)
{
	if (!active)
	{
		gradEditButton->setChecked(false);
		emit editGradient();
	}
}

void Cpalette::setActShade()
{
	int b = PM1->value();
	switch (Mode)
	{
	case 1:
		emit NewPenShade(b);
		break;
	case 2:
		if (gradientQCombo->currentIndex() == 0)
		{
			Shade3 = b;
			emit NewBrushShade(b);
		}
		else
		{
			gradEdit->Preview->setActColor(setColor(Color, b), Color, b);
			Shade = b;
			emit gradientChanged();
		}
		break;
	}
}


void Cpalette::setActPattern(QString pattern, double scaleX, double scaleY, double offsetX, double offsetY, double rotation)
{
	disconnect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	disconnect(spinXoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	disconnect(spinYoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	disconnect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	disconnect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
	disconnect(spinAngle, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	QList<QListWidgetItem*> itl = patternBox->findItems(pattern, Qt::MatchExactly);
	if (itl.count() != 0)
	{
		QListWidgetItem *it = itl[0];
		patternBox->setCurrentItem(it);
	}
	else
		patternBox->clearSelection();
	spinXoffset->setValue(offsetX);
	spinYoffset->setValue(offsetY);
	spinXscaling->setValue(scaleX);
	spinYscaling->setValue(scaleY);
	spinAngle->setValue(rotation);
	if (scaleX == scaleY)
		keepScaleRatio->setChecked(true);
	else
		keepScaleRatio->setChecked(false);
	connect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	connect(spinXoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	connect(spinYoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	connect(spinXscaling, SIGNAL(valueChanged(double)), this, SLOT(HChange()));
	connect(spinYscaling, SIGNAL(valueChanged(double)), this, SLOT(VChange()));
	connect(spinAngle, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
}

void Cpalette::unitChange(double oldUnitRatio, double newUnitRatio, int unitIndex)
{
	disconnect(spinXoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	disconnect(spinYoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	spinXoffset->setNewUnit(unitIndex);
	spinYoffset->setNewUnit(unitIndex);
	connect(spinXoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	connect(spinYoffset, SIGNAL(valueChanged(double)), this, SLOT(changePatternProps()));
	if (CGradDia)
		CGradDia->unitChange(unitIndex);
}

void Cpalette::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
	}
	else
		QWidget::changeEvent(e);
}

void Cpalette::languageChange()
{
	QString ptSuffix=tr(" pt");
	QString pctSuffix=tr(" %");
	PM1->setSuffix(pctSuffix);
	TransSpin->setSuffix(pctSuffix);
	groupOffset->setTitle( tr( "Offsets" ) );
	textLabel1->setText( tr( "X:" ) );
	spinXoffset->setSuffix( ptSuffix );
	textLabel2->setText( tr( "Y:" ) );
	spinYoffset->setSuffix( ptSuffix );
	groupScale->setTitle( tr( "Scaling" ) );
	textLabel5->setText( tr( "X-Scale:" ) );
	spinXscaling->setSuffix( pctSuffix );
	textLabel6->setText( tr( "Y-Scale:" ) );
	spinYscaling->setSuffix( pctSuffix );
	groupRotation->setTitle( tr( "Rotation" ) );
	textLabel7->setText( tr( "Angle" ) );

	ShadeTxt->setText( tr( "Shade:" ) );
	TransTxt->setText( tr( "Opacity:" ) );
	gradEditButton->setText( tr("Move Vector"));

	int oldGradient=gradientQCombo->currentIndex();
	gradientQCombo->clear();
	gradientQCombo->addItem( tr("Normal"));
	gradientQCombo->addItem( tr("Horizontal Gradient"));
	gradientQCombo->addItem( tr("Vertical Gradient"));
	gradientQCombo->addItem( tr("Diagonal Gradient"));
	gradientQCombo->addItem( tr("Cross Diagonal Gradient"));
	gradientQCombo->addItem( tr("Radial Gradient"));
	gradientQCombo->addItem( tr("Free linear Gradient"));
	gradientQCombo->addItem( tr("Free radial Gradient"));
	gradientQCombo->addItem( tr("Pattern"));
	gradientQCombo->setCurrentIndex(oldGradient);
	TransGroup->setTitle( tr( "Transparency Settings" ));
	TransTxt2->setText( tr( "Blend Mode:" ) );
	blendMode->clear();
	blendMode->addItem( tr("Normal"));
	blendMode->addItem( tr("Darken"));
	blendMode->addItem( tr("Lighten"));
	blendMode->addItem( tr("Multiply"));
	blendMode->addItem( tr("Screen"));
	blendMode->addItem( tr("Overlay"));
	blendMode->addItem( tr("Hard Light"));
	blendMode->addItem( tr("Soft Light"));
	blendMode->addItem( tr("Difference"));
	blendMode->addItem( tr("Exclusion"));
	blendMode->addItem( tr("Color Dodge"));
	blendMode->addItem( tr("Color Burn"));
	blendMode->addItem( tr("Hue"));
	blendMode->addItem( tr("Saturation"));
	blendMode->addItem( tr("Color"));
	blendMode->addItem( tr("Luminosity"));
	displayAllColors->setText( tr( "Display only used Colors" ));

	editLineColorSelector->setToolTip( tr( "Edit Line Color Properties" ) );
	editFillColorSelector->setToolTip( tr( "Edit Fill Color Properties" ) );
	PM1->setToolTip( tr( "Saturation of color" ) );
	gradientQCombo->setToolTip( tr( "Normal or gradient fill method" ) );
	TransSpin->setToolTip( tr( "Set the transparency for the color selected" ) );
	gradEditButton->setToolTip( "<qt>" + tr( "Move the start of the gradient vector with the left mouse button pressed and move the end of the gradient vector with the right mouse button pressed" ) + "</qt>");
	displayAllColors->setToolTip( "<qt>" + tr( "Display all colors from the document color list, or only the already used colors" ) + "</qt>");
}
