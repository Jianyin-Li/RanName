#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H

#include <QtCore/QVariant>
#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QtGui/QAction>

QT_BEGIN_NAMESPACE

class Ui_mainWindow
{
public:
    QAction *actionExit;
    QAction *action_nextprev;
    QAction *action_m_all;
    QAction *action_per;
    QAction *action_restart;
    QAction *action_lang_en;
    QAction *action_lang_zh;
    QAction *action_about;
    QAction *action_about_qt;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *headerLayout;
    QLabel *titleShower;
    QSpacerItem *headerSpacer;
    QLabel *modeLabel;
    QFrame *headerLine;
    QLabel *BrowserName;
    QFrame *bottomLine;
    QHBoxLayout *bottomLayout;
    QProgressBar *progressShower;
    QPushButton *ButtonNext;
    QLabel *creditLabel;
    QMenuBar *menubar;
    QMenu *menu_file;
    QMenu *menu_mode;
    QMenu *menu_view;
    QMenu *menu_lang;
    QMenu *menu_about;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *mainWindow)
    {
        if (mainWindow->objectName().isEmpty())
            mainWindow->setObjectName(QString::fromUtf8("mainWindow"));
        mainWindow->resize(550, 450);
        mainWindow->setMinimumSize(400, 350);

        actionExit = new QAction(mainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));

        action_nextprev = new QAction(mainWindow);
        action_nextprev->setObjectName(QString::fromUtf8("action_nextprev"));
        action_nextprev->setCheckable(true);
        action_nextprev->setChecked(true);

        action_m_all = new QAction(mainWindow);
        action_m_all->setObjectName(QString::fromUtf8("action_m_all"));

        action_per = new QAction(mainWindow);
        action_per->setObjectName(QString::fromUtf8("action_per"));

        action_restart = new QAction(mainWindow);
        action_restart->setObjectName(QString::fromUtf8("action_restart"));

        action_lang_en = new QAction(mainWindow);
        action_lang_en->setObjectName(QString::fromUtf8("action_lang_en"));

        action_lang_zh = new QAction(mainWindow);
        action_lang_zh->setObjectName(QString::fromUtf8("action_lang_zh"));

        action_about = new QAction(mainWindow);
        action_about->setObjectName(QString::fromUtf8("action_about"));

        action_about_qt = new QAction(mainWindow);
        action_about_qt->setObjectName(QString::fromUtf8("action_about_qt"));

        centralwidget = new QWidget(mainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(9, 9, 9, 9);

        headerLayout = new QHBoxLayout();
        headerLayout->setObjectName(QString::fromUtf8("headerLayout"));

        titleShower = new QLabel(centralwidget);
        titleShower->setObjectName(QString::fromUtf8("titleShower"));
        QFont font1;
        font1.setPointSize(14);
        font1.setBold(true);
        titleShower->setFont(font1);
        headerLayout->addWidget(titleShower);

        headerSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        headerLayout->addItem(headerSpacer);

        modeLabel = new QLabel(centralwidget);
        modeLabel->setObjectName(QString::fromUtf8("modeLabel"));
        QFont font2;
        font2.setPointSize(12);
        modeLabel->setFont(font2);
        headerLayout->addWidget(modeLabel);

        verticalLayout->addLayout(headerLayout);

        headerLine = new QFrame(centralwidget);
        headerLine->setObjectName(QString::fromUtf8("headerLine"));
        headerLine->setFrameShape(QFrame::HLine);
        headerLine->setFrameShadow(QFrame::Sunken);
        verticalLayout->addWidget(headerLine);

        BrowserName = new QLabel(centralwidget);
        BrowserName->setObjectName(QString::fromUtf8("BrowserName"));
        BrowserName->setAlignment(Qt::AlignCenter);
        QFont font3;
        font3.setPointSize(48);
        BrowserName->setFont(font3);
        verticalLayout->addWidget(BrowserName, 1);

        bottomLine = new QFrame(centralwidget);
        bottomLine->setObjectName(QString::fromUtf8("bottomLine"));
        bottomLine->setFrameShape(QFrame::HLine);
        bottomLine->setFrameShadow(QFrame::Sunken);
        verticalLayout->addWidget(bottomLine);

        bottomLayout = new QHBoxLayout();
        bottomLayout->setObjectName(QString::fromUtf8("bottomLayout"));

        progressShower = new QProgressBar(centralwidget);
        progressShower->setObjectName(QString::fromUtf8("progressShower"));
        progressShower->setMinimumSize(0, 25);
        progressShower->setValue(0);
        bottomLayout->addWidget(progressShower, 1);

        ButtonNext = new QPushButton(centralwidget);
        ButtonNext->setObjectName(QString::fromUtf8("ButtonNext"));
        ButtonNext->setMinimumSize(120, 40);
        ButtonNext->setMaximumSize(200, 50);
        QFont font4;
        font4.setPointSize(14);
        ButtonNext->setFont(font4);
        bottomLayout->addWidget(ButtonNext);

        verticalLayout->addLayout(bottomLayout);

        creditLabel = new QLabel(centralwidget);
        creditLabel->setObjectName(QString::fromUtf8("creditLabel"));
        creditLabel->setAlignment(Qt::AlignCenter);
        QFont font5;
        font5.setPointSize(8);
        creditLabel->setFont(font5);
        verticalLayout->addWidget(creditLabel);

        mainWindow->setCentralWidget(centralwidget);

        menubar = new QMenuBar(mainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 550, 22));

        menu_file = new QMenu(menubar);
        menu_file->setObjectName(QString::fromUtf8("menu_file"));

        menu_mode = new QMenu(menubar);
        menu_mode->setObjectName(QString::fromUtf8("menu_mode"));

        menu_view = new QMenu(menubar);
        menu_view->setObjectName(QString::fromUtf8("menu_view"));

        menu_lang = new QMenu(menubar);
        menu_lang->setObjectName(QString::fromUtf8("menu_lang"));

        menu_about = new QMenu(menubar);
        menu_about->setObjectName(QString::fromUtf8("menu_about"));

        mainWindow->setMenuBar(menubar);

        statusbar = new QStatusBar(mainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        mainWindow->setStatusBar(statusbar);

        menubar->addAction(menu_file->menuAction());
        menubar->addAction(menu_mode->menuAction());
        menubar->addAction(menu_view->menuAction());
        menubar->addAction(menu_lang->menuAction());
        menubar->addAction(menu_about->menuAction());

        menu_file->addAction(action_restart);
        menu_file->addSeparator();
        menu_file->addAction(actionExit);

        menu_mode->addAction(action_m_all);
        menu_mode->addAction(action_per);

        menu_view->addAction(action_nextprev);

        menu_lang->addAction(action_lang_en);
        menu_lang->addAction(action_lang_zh);

        menu_about->addAction(action_about);
        menu_about->addAction(action_about_qt);

        retranslateUi(mainWindow);

        QMetaObject::connectSlotsByName(mainWindow);
    }

    void retranslateUi(QMainWindow *mainWindow)
    {
        mainWindow->setWindowTitle(QCoreApplication::translate("mainWindow", "Random Name Picker - Main Window", nullptr));
        titleShower->setText(QCoreApplication::translate("mainWindow", "Random Name Picker", nullptr));
        modeLabel->setText(QCoreApplication::translate("mainWindow", "All Random", nullptr));
        creditLabel->setText(QCoreApplication::translate("mainWindow", "Jianyin Li, Powered by Qt Framework", nullptr));
        menu_file->setTitle(QCoreApplication::translate("mainWindow", "File", nullptr));
        menu_mode->setTitle(QCoreApplication::translate("mainWindow", "Mode", nullptr));
        menu_view->setTitle(QCoreApplication::translate("mainWindow", "View", nullptr));
        menu_lang->setTitle(QCoreApplication::translate("mainWindow", "Language", nullptr));
        menu_about->setTitle(QCoreApplication::translate("mainWindow", "About", nullptr));
        actionExit->setText(QCoreApplication::translate("mainWindow", "Exit", nullptr));
        action_nextprev->setText(QCoreApplication::translate("mainWindow", "Show Next Person Preview", nullptr));
        action_m_all->setText(QCoreApplication::translate("mainWindow", "All Random", nullptr));
        action_per->setText(QCoreApplication::translate("mainWindow", "One by One", nullptr));
        action_restart->setText(QCoreApplication::translate("mainWindow", "Restart", nullptr));
        action_lang_en->setText(QCoreApplication::translate("mainWindow", "English", nullptr));
        action_lang_zh->setText(QCoreApplication::translate("mainWindow", "中文", nullptr));
        action_about->setText(QCoreApplication::translate("mainWindow", "About RandomNamePicker", nullptr));
        action_about_qt->setText(QCoreApplication::translate("mainWindow", "About Qt", nullptr));
    }
};

namespace Ui {
    class mainWindow: public Ui_mainWindow {};
}

QT_END_NAMESPACE

#endif
