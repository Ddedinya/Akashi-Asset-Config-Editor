#ifndef PROGRAM_H
#define PROGRAM_H

#include "include/bass.h"
#include "include/bassmidi.h"
#include "include/bassopus.h" // stfu clangd pls
#include "ui_program.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Program;
}
QT_END_NAMESPACE

class Program : public QMainWindow
{
    Q_OBJECT

  public:
    Program(QWidget *parent = nullptr);
    ~Program();

    /**
     * @brief Open the folder with configs and load them.
     *
     * @see #m_config_folder
     *
     * @see #m_music_length
     */
    void openConfigFolderClicked();

    /**
     * @brief Open the folder with assets. That need for some functions.
     *
     * @see #m_base_folder
     *
     * @see #onItemClicked
     *
     * @see getLengthButtonPressed
     *
     * @see getLengthsButtonPressed
     */
    void openBaseFolderClicked();

    /**
     * @brief Save and override configs.
     */
    void saveButtonPressed();

    /**
     * @brief Get a little information about the program.
     */
    void aboutButtonClicked();

    /**
     * @brief Change displayed Background's position/Character's animation when the user change it in pos/anim list.
     *
     * @details Works only if the base folder is opened
     *
     * @see #onItemClicked
     *
     * @see #m_base_folder
     */
    void animBgListChanged(QString filename);

    /**
     * @brief Clear displayed config.
     */
    void clearConfigButtonPressed();

    /**
     * @brief Create a new config from base folder.
     *
     * @see #m_base_folder
     */
    void createConfigButtonPressed();

    /**
     * @brief Copy all items from music.txt to music.json
     *
     * @details Song lengths will be '0'
     */
    void musicTxtToJsonButtonPressed();

    /**
     * @brief Copy all items from music.json to music.txt
     */
    void musicJsonToTxtButtonPressed();

    /**
     * @brief Get length of selected songs using their music file.
     *
     * @details Works only if the base folder is opened.
     *
     * @see #m_base_folder
     *
     * @see #m_music_length
     */
    void getLengthButtonPressed();

    /**
     * @brief Get length of songs with '0' length using their music file.
     *
     * @details Works only if the base folder is opened.
     *
     * @see #m_base_folder
     *
     * @see #m_music_length
     */
    void getLengthsButtonPressed();

    /**
     * @brief Play selected song.
     *
     * @details Works only if the base folder is opened.
     *
     * @see #m_base_folder
     */
    void playButtonPressed();

    /**
     * @brief Stopping play song.
     */
    void stopButtonPressed();

    /**
     * @brief Add new category.
     *
     * @details Works only when selected music.* configs.
     *
     */
    void addCategoryButtonPressed();

    /**
     * @brief Delete selected items.
     *
     * @details If delete a category, songs also will deleted.
     *
     */
    void deleteButtonPressed();

    /**
     * @brief Change length when the user finished editing.
     *
     * @details To finish, need to press 'Enter'.
     *
     */
    void lengthEditingFinished();

    /**
     * @brief Change visible items when user using search line.
     */
    void searchTextChanged(QString text);

    /**
     * @brief Helper function for detecting dropping files.
     *
     * @see #dropEvent
     */
    void dragEnterEvent(QDragEnterEvent *event);

    /**
     * @brief Helper function for adding dropped files into the config.
     *
     * @see #dragEnterEvent
     */
    void dropEvent(QDropEvent *event);

    /**
     * @brief Helper function for adding new items into the config.
     */
    void addItems(QStringList items, QTreeWidget *widget, Qt::ItemFlags parent_flags);

    /**
     * @brief Helper function for getting items count in the config.
     */
    long itemsCount(QTreeWidget *widget);

    /**
     * @brief Helper function for getting selected config.
     */
    QTreeWidget *getCurrentTree();

    /**
     * @brief Helper function for getting need folder to display pos/anim or music file.
     */
    QString getCurrentFolder();

    /**
     * @brief Helper function for getting music to play or getting length.
     */
    DWORD getMusic(QString dir);

  public slots:
    /**
     * @brief Slot for display pos/anim or get the music file of selected item.
     */
    void onItemClicked(QTreeWidgetItem *item);

    /**
     * @brief Slot for edit the item's name.
     */
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

  private:
    Ui::AkashiAssetConfigEditor *ui;

    /**
     * @brief List of configs and their widgets.
     */
    QMap<QString, QTreeWidget *> m_configs;

    /**
     * @brief Path to the folder with configs.
     */
    QString m_config_folder;

    /**
     * @brief List of song length.
     *
     * @details For music.json
     *
     */
    QStringList m_music_length;

    /**
     * @brief Flags for creating the common item.
     */
    Qt::ItemFlags m_item_flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled |
                                 Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemNeverHasChildren;

    /**
     * @brief Flags for creating the category-item.
     */
    Qt::ItemFlags m_category_flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled |
                                     Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;

    /**
     * @brief Path to the base folder.
     */
    QString m_base_folder;

    /**
     * @brief Channel of selected music.
     */
    DWORD m_channel;
};
#endif // PROGRAM_H
