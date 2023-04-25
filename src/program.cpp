#include "include/program.h"
#include "ui_program.h"
#include <QDebug>
#include <QDirIterator>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMimeData>
#include <QTextStream>

Program::Program(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AkashiAssetConfigEditor)
{
    // Init GUI, BASS and config names
    ui->setupUi(this);
    BASS_Init(-1, 48000, BASS_DEVICE_LATENCY, 0, 0);
    m_configs.insert("/backgrounds.txt", ui->treebackgrounds);
    m_configs.insert("/characters.txt", ui->treecharacters);
    m_configs.insert("/music.txt", ui->treemusictxt);
    m_configs.insert("/music.json", ui->treemusicjson);

    // File panel signals (Open, save, and etc.)
    connect(ui->actionOpen_config_folder, &QAction::triggered, this, &Program::openConfigFolderClicked);
    connect(ui->actionOpen_base_folder, &QAction::triggered, this, &Program::openBaseFolderClicked);
    connect(ui->actionSave, &QAction::triggered, this, &Program::saveButtonPressed);
    connect(ui->actionAbout, &QAction::triggered, this, &Program::aboutButtonClicked);
    connect(ui->actionExit, &QAction::triggered, this, &QCoreApplication::quit);

    // Buttons, labels, lines signals (Background's positions/Character's animations, search/length lines, play, stop, add, delete buttons and etc)
    connect(ui->animbgList, &QComboBox::currentTextChanged, this, &Program::animBgListChanged);

    connect(ui->clearconfigButton, &QPushButton::clicked, this, &Program::clearConfigButtonPressed);
    connect(ui->createconfigButton, &QPushButton::clicked, this, &Program::createConfigButtonPressed);

    connect(ui->musictxt2jsonButton, &QPushButton::clicked, this, &Program::musicTxtToJsonButtonPressed);
    connect(ui->musicjson2txtButton, &QPushButton::clicked, this, &Program::musicJsonToTxtButtonPressed);

    connect(ui->getlengthButton, &QPushButton::clicked, this, &Program::getLengthButtonPressed);
    connect(ui->getlengthsButton, &QPushButton::clicked, this, &::Program::getLengthsButtonPressed);

    connect(ui->playButton, &QPushButton::clicked, this, &Program::playButtonPressed);
    connect(ui->stopButton, &QPushButton::clicked, this, &Program::stopButtonPressed);

    connect(ui->addCategoryButton, &QPushButton::clicked, this, &Program::addCategoryButtonPressed);
    connect(ui->deleteButton, &QPushButton::clicked, this, &Program::deleteButtonPressed);

    connect(ui->lengthLine, &QLineEdit::editingFinished, this, &Program::lengthEditingFinished);
    connect(ui->searchLine, &QLineEdit::textChanged, this, &Program::searchTextChanged);

    // Click event signals (Select item)
    connect(ui->treebackgrounds, &QTreeWidget::itemClicked, this, &Program::onItemClicked);
    connect(ui->treecharacters, &QTreeWidget::itemClicked, this, &Program::onItemClicked);
    connect(ui->treemusictxt, &QTreeWidget::itemClicked, this, &Program::onItemClicked);
    connect(ui->treemusicjson, &QTreeWidget::itemClicked, this, &Program::onItemClicked);

    // Double click event signals (Edit item's name)
    connect(ui->treebackgrounds, &QTreeWidget::itemDoubleClicked, this, &Program::onItemDoubleClicked);
    connect(ui->treecharacters, &QTreeWidget::itemDoubleClicked, this, &Program::onItemDoubleClicked);
    connect(ui->treemusictxt, &QTreeWidget::itemDoubleClicked, this, &Program::onItemDoubleClicked);
    connect(ui->treemusicjson, &QTreeWidget::itemDoubleClicked, this, &Program::onItemDoubleClicked);

    // Set drag and drop, and selection mode (Drop new files, select items)
    setAcceptDrops(true);
    ui->treebackgrounds->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treebackgrounds->setDragDropMode(QAbstractItemView::InternalMove);
    ui->treecharacters->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treecharacters->setDragDropMode(QAbstractItemView::InternalMove);
    ui->treemusictxt->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treemusictxt->setDragDropMode(QAbstractItemView::InternalMove);
    ui->treemusicjson->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treemusicjson->setDragDropMode(QAbstractItemView::InternalMove);
}

void Program::openConfigFolderClicked()
{
    if (m_base_folder.isEmpty())
        QMessageBox::information(this, tr("Warning!"), tr("Without the base folder some functions are not available! Please, open the base folder too."));

    m_config_folder = QFileDialog::getExistingDirectory();

    if (m_config_folder.isEmpty())
        return;

    qDebug() << "Config folder's path is: " + m_config_folder;

    // Cleaning from loaded configs
    m_music_length.clear();
    ui->treebackgrounds->clear();
    ui->treecharacters->clear();
    ui->treemusictxt->clear();
    ui->treemusicjson->clear();
    ui->animbgList->clear();

    QFile l_file;
    QStringList l_items;
    QStringList l_keys = m_configs.keys();
    for (const QString &l_key : qAsConst(l_keys)) {
        Qt::ItemFlags l_flags;
        if (l_key == l_keys[0] || l_key == l_keys[1])
            l_flags = m_item_flags;
        else
            l_flags = m_category_flags;

        l_file.setFileName(m_config_folder + l_key);
        l_file.open(QIODevice::ReadOnly | QIODevice::Text);
        if (l_key != l_keys[2]) // Load backgrounds.txt, characters.txt and music.txt
            while (!l_file.atEnd())
                l_items.append(l_file.readLine().trimmed());
        else { // Load music.json
            QJsonArray l_list = QJsonDocument::fromJson(l_file.readAll()).array();
            QJsonObject l_object;
            QJsonArray l_array;
            for (int i = 0; i < l_list.size(); i++) {
                l_object = l_list.at(i).toObject();
                QString l_category = l_object["category"].toString();
                if (!l_category.isEmpty()) {
                    l_items.append(l_category);
                    m_music_length.append("category");
                }

                l_array = l_object["songs"].toArray();
                for (int i = 0; i < l_array.size(); i++) {
                    QJsonObject l_music_object = l_array.at(i).toObject();
                    l_items.append(l_music_object["name"].toString());
                    m_music_length.append(QString::number(l_music_object["length"].toVariant().toDouble()));
                }
            }
        }

        QString l_suc = l_file.isReadable() ? "Success!" : "Failure!"; // I think it that works
        qDebug() << "Loading " + l_key + "... " + l_suc;
        l_file.close();
        addItems(l_items, m_configs[l_key], l_flags);
        l_items.clear();
    }
}

void Program::openBaseFolderClicked()
{
    m_base_folder = QFileDialog::getExistingDirectory();
    qDebug() << "Base folder's path is: " + m_base_folder;
}

void Program::saveButtonPressed()
{
    if (m_config_folder.isEmpty())
        m_config_folder = QFileDialog::getExistingDirectory(); // Get directory to save created from scratch configs

    QStringList l_keys = m_configs.keys();
    for (const QString &l_key : qAsConst(l_keys)) {
        QList<QTreeWidgetItem *> l_items = m_configs[l_key]->findItems(
            QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);
        if (l_items.isEmpty())
            continue;

        QFile l_file;
        l_file.setFileName(m_config_folder + l_key);
        l_file.open(QIODevice::WriteOnly);
        l_file.resize(0);
        QTextStream out(&l_file);
        if (l_key != l_keys[2]) // Save backgrounds.txt, characters.txt, music.txt
            for (const QTreeWidgetItem *l_item : qAsConst(l_items))
#if QT_VERSION > QT_VERSION_CHECK(5, 14, 0)
                out << l_item->text(1) << Qt::endl;
#else
                out << l_item->text(1) << endl;
#endif
        else { // Save music.json
            QJsonDocument l_json;
            QJsonObject l_record_object;
            QJsonArray l_category_array;
            QJsonArray l_record_array;
            QString l_last_category;
            for (const QTreeWidgetItem *l_item : qAsConst(l_items)) {
                QString l_name = l_item->text(1);
                QString l_music_name = l_name.left(l_name.lastIndexOf("."));
                l_music_name = l_music_name.right(l_music_name.length() - (l_music_name.lastIndexOf("/") + 1));
                if (l_name == l_music_name && l_last_category != l_name) {
                    if (!l_last_category.isEmpty()) {
                        l_record_array.push_back(l_record_object);
                        l_category_array = QJsonArray();
                    }

                    l_record_object.insert("category", l_name);
                    l_last_category = l_name;
                }
                else {
                    QJsonObject l_music{{"name", l_name}, {"length", m_music_length[l_item->text(0).toInt() - 1]}};
                    l_category_array.push_back(l_music);
                    l_record_object.insert("songs", l_category_array);
                }

                if (l_item == l_items.last())
                    l_record_array.push_back(l_record_object);
            }

            l_json.setArray(l_record_array);
            out << l_json.toJson();
        }

        QString l_suc = l_file.isWritable() ? "Success!" : "Failure!"; // I think it that works
        qDebug() << "Saving " + l_key + "... " + l_suc;
        l_file.close();
    }
}

void Program::aboutButtonClicked()
{
    QMessageBox::about(this, tr("About"), tr("<h2>Akashi Asset Config Editor</h2>"
                                             "<p>Simple program for adding/deleting assets on the server.</p>"
                                             "<p>Created by Ddedinya</p>"
                                             "<p>Source code: <a href='https://github.com/Ddedinya/Akashi-Asset-Config-Editor'>https://github.com/Ddedinya/Akashi-Asset-Config-Editor</a></p>"
                                             "<p>The program and the author have nothing to do with the Attorney Online 2 developers!</p>"));
}

void Program::animBgListChanged(QString filename)
{
    ui->animbgLabel->setPixmap(QPixmap(m_base_folder + getCurrentFolder() + getCurrentTree()->currentItem()->text(1) + "/" + filename).scaled(256, 192));
}

void Program::clearConfigButtonPressed()
{
    getCurrentTree()->clear();
}

void Program::createConfigButtonPressed()
{
    if (m_base_folder.isEmpty())
        return;

    getCurrentTree()->clear();

    QDir l_dir(m_base_folder + getCurrentFolder());
    QStringList l_items = l_dir.entryList();
    if (getCurrentTree() == ui->treemusicjson)
        for (const QString &l_item_name : qAsConst(l_items)) {
            QString l_name = l_item_name.left(l_item_name.lastIndexOf("."));
            l_name = l_name.right(l_name.length() - (l_name.lastIndexOf("/") + 1));
            if (l_name == l_item_name)
                m_music_length.append("category");
            else
                m_music_length.append("0");
        }

    addItems(l_items, getCurrentTree(), m_item_flags);
}

void Program::musicTxtToJsonButtonPressed()
{
    QList<QTreeWidgetItem *> l_items = ui->treemusictxt->findItems(
        QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);

    if (l_items.isEmpty())
        return;

    ui->treemusicjson->clear();
    m_music_length.clear();

    QStringList l_items_name;

    for (const QTreeWidgetItem *l_item : qAsConst(l_items)) {
        QString l_item_name = l_item->text(1);
        QString l_name = l_item_name.left(l_item_name.lastIndexOf("."));
        l_name = l_name.right(l_name.length() - (l_name.lastIndexOf("/") + 1));
        if (l_name == l_item_name)
            m_music_length.append("category");
        else
            m_music_length.append("0");

        l_items_name.append(l_item_name);
    }

    addItems(l_items_name, ui->treemusicjson, m_category_flags);
}

void Program::musicJsonToTxtButtonPressed()
{
    QList<QTreeWidgetItem *> l_items = ui->treemusicjson->findItems(
        QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);
    if (l_items.isEmpty())
        return;

    ui->treemusictxt->clear();

    QStringList l_items_name;
    for (const QTreeWidgetItem *l_item : qAsConst(l_items))
        l_items_name.append(l_item->text(1));

    addItems(l_items_name, ui->treemusictxt, m_category_flags);
}

void Program::getLengthButtonPressed()
{
    if (ui->configList->currentIndex() != 3)
        return;

    if (m_base_folder.isEmpty()) {
        QMessageBox::information(this, tr("Warning!"), tr("Without the base folder, this function is not available!"));
        return;
    }

    QList<QTreeWidgetItem *> l_items = ui->treemusicjson->selectedItems();
    for (const QTreeWidgetItem *l_item : qAsConst(l_items)) {
        int l_id = l_item->text(0).toInt() - 1;

        if (m_music_length[l_id] == "category")
            return;

        QString l_length = QString::number(BASS_ChannelBytes2Seconds(m_channel, BASS_ChannelGetLength(m_channel, BASS_POS_BYTE)));
        m_music_length[l_id] = l_length;
    }

    ui->lengthLine->setText(m_music_length[ui->treemusicjson->currentItem()->text(0).toInt() - 1]);
}

void Program::getLengthsButtonPressed()
{
    if (ui->configList->currentIndex() != 3)
        return;

    if (m_base_folder.isEmpty()) {
        QMessageBox::information(this, tr("Warning!"), tr("Without the base folder, this function is not available!"));
        return;
    }

    QList<QTreeWidgetItem *> l_items = ui->treemusicjson->findItems(
        QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);
    for (const QTreeWidgetItem *l_item : qAsConst(l_items)) {
        int l_id = l_item->text(0).toInt() - 1;
        if (m_music_length[l_id] == "0") {
            DWORD l_music = getMusic(m_base_folder + getCurrentFolder() + l_item->text(1));
            m_music_length[l_id] = QString::number(BASS_ChannelBytes2Seconds(l_music, BASS_ChannelGetLength(l_music, BASS_POS_BYTE)));
        }
    }
}

void Program::playButtonPressed()
{
    BASS_ChannelPlay(m_channel, true);
}

void Program::stopButtonPressed()
{
    BASS_ChannelStop(m_channel);
}

void Program::addCategoryButtonPressed()
{
    int l_index = ui->configList->currentIndex();
    if (l_index != 2 && l_index != 3)
        return;
    if (l_index == 3)
        m_music_length.append("category");

    QStringList l_category("New Category");
    addItems(l_category, getCurrentTree(), m_category_flags);
}

void Program::deleteButtonPressed()
{
    QList<QTreeWidgetItem *> l_items = getCurrentTree()->selectedItems();
    for (int i = 0; i < l_items.size(); i++)
        delete l_items[i];
}

void Program::lengthEditingFinished()
{
    if (ui->configList->currentIndex() != 3)
        return;

    bool l_ok;
    double l_new_length = ui->lengthLine->text().toDouble(&l_ok);
    int l_id = getCurrentTree()->currentItem()->text(0).toInt() - 1;
    if (!l_ok || m_music_length[l_id] == "category") {
        ui->lengthLine->setText(m_music_length[l_id]);
        return;
    }

    m_music_length[l_id] = QString::number(l_new_length);
}

void Program::searchTextChanged(QString text)
{
    QTreeWidgetItemIterator l_iter(getCurrentTree());
    while (*l_iter) {
        (*l_iter)->setHidden(text != "");
        ++l_iter;
    }

    if (text != "") {
        QList<QTreeWidgetItem *> l_items = getCurrentTree()->findItems(
            ui->searchLine->text(), Qt::MatchContains | Qt::MatchRecursive, 1);
        foreach (QTreeWidgetItem *item, l_items) {
            if (item->parent() != nullptr)
                item->parent()->setHidden(false);
            item->setHidden(false);
        }
    }
}

void Program::onItemClicked(QTreeWidgetItem *item)
{
    if (ui->configList->currentIndex() == 3) {
        int l_id = item->text(0).toInt();
        if (l_id <= m_music_length.size())
            ui->lengthLine->setText(m_music_length[l_id - 1]);
    }

    if (m_base_folder.isEmpty())
        return;

    ui->animbgList->clear();
    ui->chariconLabel->clear();
    QString l_dir = m_base_folder + getCurrentFolder() + getCurrentTree()->currentItem()->text(1);

    switch (ui->configList->currentIndex()) {
    case 0:
    case 1:
    {
        l_dir = l_dir + "/";
        QDirIterator l_files(l_dir, QDir::Files, QDirIterator::Subdirectories);

        while (l_files.hasNext()) {
            QString l_file = l_files.next().remove(l_dir);
            if (l_file.startsWith("char_icon"))
                ui->chariconLabel->setPixmap(QPixmap(l_dir + l_file).scaled(80, 80));
            else if (l_file.startsWith("emotions"))
                continue;
            else if (l_file.endsWith(".png") || l_file.endsWith(".webp") || l_file.endsWith(".gif") || l_file.endsWith(".apng"))
                ui->animbgList->addItem(l_file);
        }

        break;
    }
    case 2:
    case 3:
    {
        m_channel = getMusic(l_dir);
        break;
    }
    default:
        break;
    }

    qDebug() << "Selected file's path is " + l_dir;
}

void Program::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (column == 1)
        getCurrentTree()->editItem(item, column);
}

void Program::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void Program::dropEvent(QDropEvent *event)
{
    QStringList l_items;
    foreach (const QUrl &url, event->mimeData()->urls())
        l_items.append(url.fileName());

    Qt::ItemFlags l_parent_flags;
    QTreeWidget *l_widget = getCurrentTree();
    if (l_widget == ui->treebackgrounds || l_widget == ui->treecharacters)
        l_parent_flags = m_item_flags;
    else
        l_parent_flags = m_category_flags;

    addItems(l_items, l_widget, l_parent_flags);

    if (l_widget == ui->treemusicjson)
        for (int i = 0; i < l_items.size(); i++)
            m_music_length.append("0");
}

long Program::itemsCount(QTreeWidget *widget)
{
    QTreeWidgetItemIterator l_iter(widget);
    long l_count = 0;
    while (*l_iter) {
        l_count++;
        ++l_iter;
    }

    return l_count;
}

void Program::addItems(QStringList items, QTreeWidget *widget, Qt::ItemFlags parent_flags)
{
    int id = 1;
    int l_count = itemsCount(widget);
    QTreeWidgetItem *l_parent = nullptr;
    for (const QString &l_item : qAsConst(items)) {
        if (l_item == "." || l_item == "..")
            continue;

        QString l_item_name = l_item.left(l_item.lastIndexOf("."));
        l_item_name = l_item_name.right(l_item_name.length() - (l_item_name.lastIndexOf("/") + 1));
        QTreeWidgetItem *l_tree_item;
        if (l_item_name != l_item && l_parent != nullptr) {
            l_tree_item = new QTreeWidgetItem(l_parent);
            l_tree_item->setData(0, Qt::DisplayRole, l_count + id);
            l_tree_item->setData(1, Qt::DisplayRole, l_item);
            l_tree_item->setFlags(m_item_flags);
        }
        else {
            l_tree_item = new QTreeWidgetItem(widget);
            l_tree_item->setData(0, Qt::DisplayRole, l_count + id);
            l_tree_item->setData(1, Qt::DisplayRole, l_item);
            l_tree_item->setFlags(parent_flags);
        }
        if (l_item_name == l_item)
            l_parent = l_tree_item;

        id++;
    }
}

QTreeWidget *Program::getCurrentTree()
{
    switch (ui->configList->currentIndex()) {
    case 0:
        return ui->treebackgrounds;
    case 1:
        return ui->treecharacters;
    case 2:
        return ui->treemusictxt;
    case 3:
        return ui->treemusicjson;
    }

    return nullptr;
}

QString Program::getCurrentFolder()
{
    switch (ui->configList->currentIndex()) {
    case 0:
        return "/background/";
    case 1:
        return "/characters/";
    case 2:
    case 3:
        return "/sounds/music/";
    }

    return "what";
}

DWORD Program::getMusic(QString dir)
{
    if (dir.endsWith(".opus"))
        return BASS_OPUS_StreamCreateFile(FALSE, dir.utf16(), 0, 0, BASS_UNICODE);
    else if (dir.endsWith(".mid"))
        return BASS_MIDI_StreamCreateFile(FALSE, dir.utf16(), 0, 0, BASS_UNICODE, 1);
    else
        return BASS_StreamCreateFile(FALSE, dir.utf16(), 0, 0, BASS_UNICODE);
}

Program::~Program()
{
    delete ui;
}
