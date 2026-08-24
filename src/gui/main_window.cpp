#include "main_window.h"
#include "ui_main_window.h"
#include "../core/config_manager.h"
#include "../core/name_list.h"
#include "../core/randomizer.h"
#include "../i18n/localizer.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QObject>
#include <QApplication>
#include <QLinearGradient>
#include <QPalette>
#include <QBrush>
#include <QFont>
#include <fstream>
#include <random>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mainWindow),
    translator(new QTranslator(this)),
    timeLabel(new QLabel(this)),
    nextPreviewLabel(new QLabel(this)),
    currentIndex(0),
    showNextPreview(true),
    allDone(false),
    pickMode(1)
{
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(timeLabel);

    nextPreviewLabel->setAlignment(Qt::AlignCenter);
    nextPreviewLabel->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 13px; padding: 2px;");
    int idx = ui->verticalLayout->indexOf(ui->bottomLine);
    ui->verticalLayout->insertWidget(idx, nextPreviewLabel);

    setStyleSheet(R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
        }
        QMenuBar {
            background: rgba(255,255,255,0.15);
            color: white;
            padding: 2px;
            font-size: 13px;
        }
        QMenuBar::item:selected {
            background: rgba(255,255,255,0.25);
            border-radius: 4px;
        }
        QMenu {
            background: #2d2d44;
            color: white;
            border: 1px solid #444;
            border-radius: 6px;
            padding: 4px;
        }
        QMenu::item:selected {
            background: #667eea;
            border-radius: 4px;
            padding: 4px 20px;
        }
        QLabel#titleShower {
            color: white;
            font-size: 18px;
            font-weight: bold;
            padding: 4px;
        }
        QLabel#modeLabel {
            color: rgba(255,255,255,0.85);
            font-size: 13px;
            padding: 4px 12px;
            background: rgba(255,255,255,0.12);
            border-radius: 12px;
        }
        QFrame#headerLine, QFrame#bottomLine {
            color: rgba(255,255,255,0.2);
        }
        QLabel#BrowserName {
            background: rgba(255,255,255,0.92);
            border-radius: 16px;
            padding: 20px;
            color: #2d2d44;
            font-size: 52px;
            font-weight: 600;
            margin: 10px 20px;
        }
        QPushButton#ButtonNext {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #f093fb, stop:1 #f5576c);
            color: white;
            font-size: 16px;
            font-weight: bold;
            border: none;
            border-radius: 20px;
            padding: 8px 24px;
        }
        QPushButton#ButtonNext:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #f5a6ff, stop:1 #f7788a);
        }
        QPushButton#ButtonNext:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #d07de0, stop:1 #d04a5c);
        }
        QProgressBar {
            background: rgba(255,255,255,0.15);
            border: none;
            border-radius: 10px;
            height: 20px;
            text-align: center;
            color: white;
            font-size: 12px;
            font-weight: bold;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #43e97b, stop:1 #38f9d7);
            border-radius: 10px;
        }
        QLabel#creditLabel {
            color: rgba(255,255,255,0.5);
            font-size: 9px;
        }
        QStatusBar {
            background: rgba(0,0,0,0.15);
            color: rgba(255,255,255,0.8);
            font-size: 12px;
        }
    )");

    timer = new QTimer(this);

    QObject::connect(ui->ButtonNext, &QPushButton::clicked, this, &MainWindow::onNextButtonClicked);
    QObject::connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);
    QObject::connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onExitActionTriggered);
    QObject::connect(ui->action_nextprev, &QAction::triggered, this, &MainWindow::onShowNextPreviewActionTriggered);
    QObject::connect(ui->action_m_all, &QAction::triggered, this, &MainWindow::onAllRandomActionTriggered);
    QObject::connect(ui->action_per, &QAction::triggered, this, &MainWindow::onOneByOneActionTriggered);
    QObject::connect(ui->action_restart, &QAction::triggered, this, &MainWindow::onRestartActionTriggered);
    QObject::connect(ui->action_lang_en, &QAction::triggered, this, &MainWindow::onLanguageEnglish);
    QObject::connect(ui->action_lang_zh, &QAction::triggered, this, &MainWindow::onLanguageChinese);
    QObject::connect(ui->action_about, &QAction::triggered, this, &MainWindow::onAbout);
    QObject::connect(ui->action_about_qt, &QAction::triggered, this, &MainWindow::onAboutQt);

    retranslateStrings();

    loadNamesFromFile();
    initializeRandomizer();

    timer->start(1000);

    updateUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::retranslateStrings()
{
    ui->titleShower->setText(tr("Random Name Picker"));
    ui->ButtonNext->setText(tr("Next"));
    ui->creditLabel->setText(tr("Jianyin Li, Powered by Qt Framework"));

    ui->menu_file->setTitle(tr("File"));
    ui->menu_mode->setTitle(tr("Mode"));
    ui->menu_view->setTitle(tr("View"));
    ui->menu_lang->setTitle(tr("Language"));
    ui->menu_about->setTitle(tr("About"));

    ui->action_m_all->setText(tr("All Random"));
    ui->action_per->setText(tr("One by One"));
    ui->action_restart->setText(tr("Restart"));
    ui->actionExit->setText(tr("Exit"));
    ui->action_lang_en->setText(tr("English"));
    ui->action_lang_zh->setText(tr("中文"));
    ui->action_about->setText(tr("About RandomNamePicker"));
    ui->action_about_qt->setText(tr("About Qt"));
}

void MainWindow::showStatus(const QString& message, int timeout)
{
    ui->statusbar->showMessage(message, timeout);
}

void MainWindow::switchLanguage(const QString& langCode)
{
    QString qmPath = QString(":/i18n/RandomNamePicker_%1.qm").arg(langCode);
    QFileInfo qmInfo(qmPath);

    qApp->removeTranslator(translator);

    if (qmInfo.exists() && translator->load(qmPath)) {
        qApp->installTranslator(translator);
    }

    i18n::Localizer::setLanguage(langCode.toStdString());

    ui->retranslateUi(this);
    retranslateStrings();
    updateUI();

    config::ConfigManager cfg;
    cfg.loadFromFile("data/config.conf");
    cfg.setLanguage(langCode.toStdString());
    cfg.saveToFile("data/config.conf");

    QString langName = (langCode == "zh_CN")
        ? QString::fromUtf8("中文") : QString("English");
    showStatus(tr("Language switched to %1").arg(langName));
}

void MainWindow::onLanguageEnglish()
{
    switchLanguage("en_US");
}

void MainWindow::onLanguageChinese()
{
    switchLanguage("zh_CN");
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("About RandomNamePicker"),
        tr("<h3>RandomNamePicker %1</h3>"
           "<p>A random name picker for classrooms.</p>"
           "<p>Copyright &copy; 2025 Jianyin Li</p>"
           "<p>Built with Qt %2</p>")
        .arg(QApplication::applicationVersion())
        .arg(QT_VERSION_STR));
}

void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::loadNamesFromFile()
{
    QFileInfo fileInfo(QString::fromStdString("data/namelist.txt"));
    if (!fileInfo.exists()) {
        showStatus(tr("Name list file not found."), 8000);
        QMessageBox::information(this, tr("Setup Required"),
            tr("No name list found at data/namelist.txt.\n"
               "Please add names to start picking."));
        return;
    }

    data::NameList dataList;
    dataList.loadFromFile("data/namelist.txt");

    names.clear();
    for (size_t i = 0; i < dataList.getCount(); i++) {
        names.push_back(dataList.getNameAt(i));
    }

    if (names.empty()) {
        showStatus(tr("Name list is empty! Please configure it first."), 8000);
        QMessageBox::warning(this, tr("Warning"), tr("The name list is empty. Please add some names first!"));
    }
}

void MainWindow::initializeRandomizer()
{
    randomIndices.clear();
    currentIndex = 0;
    allDone = false;

    for (size_t i = 0; i < names.size(); i++) {
        randomIndices.push_back(i);
    }

    try {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(randomIndices.begin(), randomIndices.end(), g);
    } catch (...) {
        std::mt19937 g(static_cast<unsigned int>(
            std::chrono::system_clock::now().time_since_epoch().count()));
        std::shuffle(randomIndices.begin(), randomIndices.end(), g);
    }
}

void MainWindow::onNextButtonClicked()
{
    if (allDone) {
        onRestartActionTriggered();
        return;
    }

    if (currentIndex < names.size() - 1) {
        currentIndex++;
        updateUI();
        showStatus(tr("Progress: %1/%2").arg(currentIndex + 1).arg(names.size()));
    } else {
        allDone = true;
        updateUI();
    }
}

void MainWindow::onTimerTimeout()
{
    updateTime();
}

void MainWindow::onExitActionTriggered()
{
    close();
}

void MainWindow::onShowNextPreviewActionTriggered()
{
    showNextPreview = !showNextPreview;
    ui->action_nextprev->setChecked(showNextPreview);
    nextPreviewLabel->setVisible(showNextPreview);
    showStatus(showNextPreview
        ? tr("Next person preview shown.")
        : tr("Next person preview hidden."));
}

void MainWindow::onAllRandomActionTriggered()
{
    pickMode = 1;
    initializeRandomizer();
    updateUI();
    showStatus(tr("Mode changed to All Random"));
}

void MainWindow::onOneByOneActionTriggered()
{
    pickMode = 2;
    initializeRandomizer();
    updateUI();
    showStatus(tr("Mode changed to One by One"));
}

void MainWindow::onRestartActionTriggered()
{
    initializeRandomizer();
    updateUI();
    showStatus(tr("Restarted"));
}

void MainWindow::updateUI()
{
    QString modeText = (pickMode == 1) ? tr("All Random") : tr("One by One");
    ui->modeLabel->setText(modeText);
    setWindowTitle(tr("Random Name Picker - %1").arg(modeText));

    if (names.empty() || randomIndices.empty()) {
        ui->BrowserName->setText(tr("List is empty"));
        ui->progressShower->setValue(0);
        return;
    }

    if (allDone) {
        ui->BrowserName->setText(tr("All done! Click Next to restart."));
        ui->progressShower->setValue(100);
        nextPreviewLabel->clear();
    } else if (currentIndex < randomIndices.size()) {
        QString currentName = QString::fromStdString(names[randomIndices[currentIndex]]);
        ui->BrowserName->setText(currentName);

        size_t nextIdx = currentIndex + 1;
        if (nextIdx < randomIndices.size()) {
            QString nextName = QString::fromStdString(names[randomIndices[nextIdx]]);
            nextPreviewLabel->setText(tr("Next: %1").arg(nextName));
        } else {
            nextPreviewLabel->setText(tr("Next: ---"));
        }
    }

    updateProgress();
    updateTime();
}

void MainWindow::updateProgress()
{
    if (names.empty()) {
        ui->progressShower->setValue(0);
        return;
    }

    int progress = (currentIndex + 1) * 100 / names.size();
    if (progress > 100) progress = 100;
    ui->progressShower->setValue(progress);
}

void MainWindow::updateTime()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeString = currentTime.toString("HH:mm:ss");
    timeLabel->setText(tr("Current time: %1").arg(timeString));
}
