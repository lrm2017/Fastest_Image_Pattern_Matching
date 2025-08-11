/********************************************************************************
** Form generated from reading UI file 'MatchToolDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MATCHTOOLDIALOG_H
#define UI_MATCHTOOLDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MatchToolDialog
{
public:
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QWidget *leftWidget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBoxSource;
    QVBoxLayout *sourceLayout;
    QGraphicsView *sourceView;
    QPushButton *loadSrcButton;
    QWidget *rightWidget;
    QVBoxLayout *rightLayout;
    QHBoxLayout *horizontalLayout_2;
    QGroupBox *groupBoxTemplate;
    QVBoxLayout *templateLayout;
    QGraphicsView *templateView;
    QPushButton *loadDstButton;
    QGroupBox *groupBoxParameters;
    QGridLayout *parametersLayout;
    QLabel *labelMaxPositions;
    QSpinBox *maxPositionsSpinBox;
    QLabel *labelMinReduceArea;
    QLabel *labelMaxOverlap;
    QDoubleSpinBox *maxOverlapSpinBox;
    QLabel *labelScore;
    QDoubleSpinBox *scoreSpinBox;
    QLabel *labelToleranceAngle;
    QDoubleSpinBox *toleranceAngleSpinBox;
    QSpinBox *minReduceAreaSpinBox;
    QCheckBox *useSIMDCheckBox;
    QCheckBox *subPixelCheckBox;
    QGroupBox *groupBoxResults;
    QVBoxLayout *resultsLayout;
    QTableWidget *resultsTable;
    QHBoxLayout *resultsButtonLayout;
    QPushButton *executeButton;
    QPushButton *clearButton;
    QStatusBar *statusBar;

    void setupUi(QDialog *MatchToolDialog)
    {
        if (MatchToolDialog->objectName().isEmpty())
            MatchToolDialog->setObjectName(QString::fromUtf8("MatchToolDialog"));
        MatchToolDialog->resize(1200, 800);
        verticalLayout = new QVBoxLayout(MatchToolDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        splitter = new QSplitter(MatchToolDialog);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        leftWidget = new QWidget(splitter);
        leftWidget->setObjectName(QString::fromUtf8("leftWidget"));
        horizontalLayout = new QHBoxLayout(leftWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        groupBoxSource = new QGroupBox(leftWidget);
        groupBoxSource->setObjectName(QString::fromUtf8("groupBoxSource"));
        sourceLayout = new QVBoxLayout(groupBoxSource);
        sourceLayout->setObjectName(QString::fromUtf8("sourceLayout"));
        sourceView = new QGraphicsView(groupBoxSource);
        sourceView->setObjectName(QString::fromUtf8("sourceView"));
        sourceView->setMinimumSize(QSize(400, 200));

        sourceLayout->addWidget(sourceView);

        loadSrcButton = new QPushButton(groupBoxSource);
        loadSrcButton->setObjectName(QString::fromUtf8("loadSrcButton"));

        sourceLayout->addWidget(loadSrcButton);


        horizontalLayout->addWidget(groupBoxSource);

        splitter->addWidget(leftWidget);
        rightWidget = new QWidget(splitter);
        rightWidget->setObjectName(QString::fromUtf8("rightWidget"));
        rightLayout = new QVBoxLayout(rightWidget);
        rightLayout->setObjectName(QString::fromUtf8("rightLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        groupBoxTemplate = new QGroupBox(rightWidget);
        groupBoxTemplate->setObjectName(QString::fromUtf8("groupBoxTemplate"));
        templateLayout = new QVBoxLayout(groupBoxTemplate);
        templateLayout->setObjectName(QString::fromUtf8("templateLayout"));
        templateView = new QGraphicsView(groupBoxTemplate);
        templateView->setObjectName(QString::fromUtf8("templateView"));
        templateView->setMinimumSize(QSize(400, 300));

        templateLayout->addWidget(templateView);

        loadDstButton = new QPushButton(groupBoxTemplate);
        loadDstButton->setObjectName(QString::fromUtf8("loadDstButton"));

        templateLayout->addWidget(loadDstButton);


        horizontalLayout_2->addWidget(groupBoxTemplate);

        groupBoxParameters = new QGroupBox(rightWidget);
        groupBoxParameters->setObjectName(QString::fromUtf8("groupBoxParameters"));
        parametersLayout = new QGridLayout(groupBoxParameters);
        parametersLayout->setObjectName(QString::fromUtf8("parametersLayout"));
        labelMaxPositions = new QLabel(groupBoxParameters);
        labelMaxPositions->setObjectName(QString::fromUtf8("labelMaxPositions"));

        parametersLayout->addWidget(labelMaxPositions, 0, 0, 1, 1);

        maxPositionsSpinBox = new QSpinBox(groupBoxParameters);
        maxPositionsSpinBox->setObjectName(QString::fromUtf8("maxPositionsSpinBox"));
        maxPositionsSpinBox->setMinimum(1);
        maxPositionsSpinBox->setMaximum(200);
        maxPositionsSpinBox->setValue(70);

        parametersLayout->addWidget(maxPositionsSpinBox, 0, 1, 1, 1);

        labelMinReduceArea = new QLabel(groupBoxParameters);
        labelMinReduceArea->setObjectName(QString::fromUtf8("labelMinReduceArea"));

        parametersLayout->addWidget(labelMinReduceArea, 4, 0, 1, 1);

        labelMaxOverlap = new QLabel(groupBoxParameters);
        labelMaxOverlap->setObjectName(QString::fromUtf8("labelMaxOverlap"));

        parametersLayout->addWidget(labelMaxOverlap, 2, 0, 1, 1);

        maxOverlapSpinBox = new QDoubleSpinBox(groupBoxParameters);
        maxOverlapSpinBox->setObjectName(QString::fromUtf8("maxOverlapSpinBox"));
        maxOverlapSpinBox->setDecimals(2);
        maxOverlapSpinBox->setMaximum(1.000000000000000);
        maxOverlapSpinBox->setValue(0.100000000000000);

        parametersLayout->addWidget(maxOverlapSpinBox, 2, 1, 1, 1);

        labelScore = new QLabel(groupBoxParameters);
        labelScore->setObjectName(QString::fromUtf8("labelScore"));

        parametersLayout->addWidget(labelScore, 1, 0, 1, 1);

        scoreSpinBox = new QDoubleSpinBox(groupBoxParameters);
        scoreSpinBox->setObjectName(QString::fromUtf8("scoreSpinBox"));
        scoreSpinBox->setDecimals(2);
        scoreSpinBox->setMaximum(1.000000000000000);
        scoreSpinBox->setValue(0.700000000000000);

        parametersLayout->addWidget(scoreSpinBox, 1, 1, 1, 1);

        labelToleranceAngle = new QLabel(groupBoxParameters);
        labelToleranceAngle->setObjectName(QString::fromUtf8("labelToleranceAngle"));

        parametersLayout->addWidget(labelToleranceAngle, 3, 0, 1, 1);

        toleranceAngleSpinBox = new QDoubleSpinBox(groupBoxParameters);
        toleranceAngleSpinBox->setObjectName(QString::fromUtf8("toleranceAngleSpinBox"));
        toleranceAngleSpinBox->setDecimals(1);
        toleranceAngleSpinBox->setMaximum(360.000000000000000);
        toleranceAngleSpinBox->setValue(180.000000000000000);

        parametersLayout->addWidget(toleranceAngleSpinBox, 3, 1, 1, 1);

        minReduceAreaSpinBox = new QSpinBox(groupBoxParameters);
        minReduceAreaSpinBox->setObjectName(QString::fromUtf8("minReduceAreaSpinBox"));
        minReduceAreaSpinBox->setMinimum(1);
        minReduceAreaSpinBox->setMaximum(10000);
        minReduceAreaSpinBox->setValue(256);

        parametersLayout->addWidget(minReduceAreaSpinBox, 4, 1, 1, 1);

        useSIMDCheckBox = new QCheckBox(groupBoxParameters);
        useSIMDCheckBox->setObjectName(QString::fromUtf8("useSIMDCheckBox"));
        useSIMDCheckBox->setChecked(true);

        parametersLayout->addWidget(useSIMDCheckBox, 5, 0, 1, 2);

        subPixelCheckBox = new QCheckBox(groupBoxParameters);
        subPixelCheckBox->setObjectName(QString::fromUtf8("subPixelCheckBox"));
        subPixelCheckBox->setChecked(true);

        parametersLayout->addWidget(subPixelCheckBox, 6, 0, 1, 1);


        horizontalLayout_2->addWidget(groupBoxParameters);


        rightLayout->addLayout(horizontalLayout_2);

        groupBoxResults = new QGroupBox(rightWidget);
        groupBoxResults->setObjectName(QString::fromUtf8("groupBoxResults"));
        resultsLayout = new QVBoxLayout(groupBoxResults);
        resultsLayout->setObjectName(QString::fromUtf8("resultsLayout"));
        resultsTable = new QTableWidget(groupBoxResults);
        if (resultsTable->columnCount() < 5)
            resultsTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        resultsTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        resultsTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        resultsTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        resultsTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        resultsTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        resultsTable->setObjectName(QString::fromUtf8("resultsTable"));
        resultsTable->setAlternatingRowColors(true);
        resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

        resultsLayout->addWidget(resultsTable);

        resultsButtonLayout = new QHBoxLayout();
        resultsButtonLayout->setObjectName(QString::fromUtf8("resultsButtonLayout"));
        executeButton = new QPushButton(groupBoxResults);
        executeButton->setObjectName(QString::fromUtf8("executeButton"));

        resultsButtonLayout->addWidget(executeButton);

        clearButton = new QPushButton(groupBoxResults);
        clearButton->setObjectName(QString::fromUtf8("clearButton"));

        resultsButtonLayout->addWidget(clearButton);


        resultsLayout->addLayout(resultsButtonLayout);


        rightLayout->addWidget(groupBoxResults);

        rightLayout->setStretch(0, 1);
        rightLayout->setStretch(1, 1);
        splitter->addWidget(rightWidget);

        verticalLayout->addWidget(splitter);

        statusBar = new QStatusBar(MatchToolDialog);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        statusBar->setLayoutDirection(Qt::LeftToRight);

        verticalLayout->addWidget(statusBar);


        retranslateUi(MatchToolDialog);

        QMetaObject::connectSlotsByName(MatchToolDialog);
    } // setupUi

    void retranslateUi(QDialog *MatchToolDialog)
    {
        MatchToolDialog->setWindowTitle(QApplication::translate("MatchToolDialog", "Fastest Image Pattern Matching", nullptr));
        groupBoxSource->setTitle(QApplication::translate("MatchToolDialog", "\346\272\220\345\233\276\345\203\217", nullptr));
        loadSrcButton->setText(QApplication::translate("MatchToolDialog", "\345\212\240\350\275\275\346\272\220\345\233\276\345\203\217", nullptr));
        groupBoxTemplate->setTitle(QApplication::translate("MatchToolDialog", "\346\250\241\346\235\277\345\233\276\345\203\217", nullptr));
        loadDstButton->setText(QApplication::translate("MatchToolDialog", "\345\212\240\350\275\275\346\250\241\346\235\277\345\233\276\345\203\217", nullptr));
        groupBoxParameters->setTitle(QApplication::translate("MatchToolDialog", "\345\217\202\346\225\260\350\256\276\347\275\256", nullptr));
        labelMaxPositions->setText(QApplication::translate("MatchToolDialog", "\346\234\200\345\244\247\344\275\215\347\275\256\346\225\260:", nullptr));
        labelMinReduceArea->setText(QApplication::translate("MatchToolDialog", "\346\234\200\345\260\217\345\207\217\345\260\221\345\214\272\345\237\237:", nullptr));
        labelMaxOverlap->setText(QApplication::translate("MatchToolDialog", "\346\234\200\345\244\247\351\207\215\345\217\240\345\272\246:", nullptr));
        labelScore->setText(QApplication::translate("MatchToolDialog", "\347\233\270\344\274\274\345\272\246\351\230\210\345\200\274:", nullptr));
        labelToleranceAngle->setText(QApplication::translate("MatchToolDialog", "\350\247\222\345\272\246\345\256\271\345\267\256:", nullptr));
        useSIMDCheckBox->setText(QApplication::translate("MatchToolDialog", "\344\275\277\347\224\250SIMD\344\274\230\345\214\226", nullptr));
        subPixelCheckBox->setText(QApplication::translate("MatchToolDialog", "\344\272\232\345\203\217\347\264\240", nullptr));
        groupBoxResults->setTitle(QApplication::translate("MatchToolDialog", "\345\214\271\351\205\215\347\273\223\346\236\234", nullptr));
        QTableWidgetItem *___qtablewidgetitem = resultsTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("MatchToolDialog", "\347\264\242\345\274\225", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = resultsTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("MatchToolDialog", "\345\210\206\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = resultsTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("MatchToolDialog", "\350\247\222\345\272\246", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = resultsTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("MatchToolDialog", "X\345\235\220\346\240\207", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = resultsTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("MatchToolDialog", "Y\345\235\220\346\240\207", nullptr));
        executeButton->setText(QApplication::translate("MatchToolDialog", "\346\211\247\350\241\214\345\214\271\351\205\215", nullptr));
        clearButton->setText(QApplication::translate("MatchToolDialog", "\346\270\205\351\231\244\347\273\223\346\236\234", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MatchToolDialog: public Ui_MatchToolDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MATCHTOOLDIALOG_H
