/* 
 * File:   AppMenuComponent.cpp
 * Author: anthony
 * Created on December 19, 2017, 1:37 PM
 */


#include <set>
#include "../PocketHomeApplication.h"
#include "AppMenuButton/DesktopEntryButton.h"
#include "AppMenuButton/ConfigAppButton.h"
#include "AppMenuButton/AppFolderButton.h"
#include "AppMenuComponent.h"

AppMenuComponent::AppMenuComponent(AppConfigFile& appConfig) :
ConfigurableComponent(ComponentConfigFile::appMenuKey),
loadingAsync(false),
appConfig(appConfig)

{
    applyConfigBounds();
    x_origin = getBounds().getX();
    y_origin = getBounds().getY();
    setWantsKeyboardFocus(false);
    loadingSpinner = new OverlaySpinner();
    loadingSpinner->setAlwaysOnTop(true);

    loadButtons();
}

AppMenuComponent::~AppMenuComponent()
{
    stopWaitingForLoading();
    while (!buttonColumns.empty())closeFolder();
}

/**
 * Loads all app menu buttons
 */
void AppMenuComponent::loadButtons()
{
    while (!buttonColumns.empty())
    {
        closeFolder();
    }
    buttonNameMap.clear();
    buttonColumns.clear();
    columnTops.clear();
    selected.clear();
    selected.push_back(nullptr);
    columnTops.push_back(y_origin);
    buttonColumns.emplace(buttonColumns.begin());

    //read in main page apps from config
    Array<AppConfigFile::AppItem> favorites = appConfig.getFavorites();
    for (const AppConfigFile::AppItem& favorite : favorites)
    {
        DBG(String("AppMenu:Found app in config:") + favorite.name);
        addButton(new ConfigAppButton(favorite,
                buttonColumns[activeColumn()].size(),
                activeColumn(), iconThread));
    }

    //add category buttons
    Array<AppConfigFile::AppFolder> categories = appConfig.getFolders();
    for (const AppConfigFile::AppFolder& category : categories)
    {
        addButton(new AppFolderButton(appConfig, category,
                buttonColumns[activeColumn()].size(),
                activeColumn(), iconThread));
    }
    DBG(String("added ") + String(buttonColumns[activeColumn()].size())
            + " buttons");
    scrollToSelected();
    showLoadingSpinner();
    if (!loadingAsync)
    {
        loadingAsync = true;
        desktopEntries.loadEntries([this](String loadingMsg)
        {
            loadingSpinner->setLoadingText(loadingMsg);
        },
        [this]()
        {
            loadingAsync = false;
            loadingSpinner->setLoadingText("");
            hideLoadingSpinner();
        });
    }
}

/**
 * Open an application category folder, creating AppMenuButtons for all
 * associated desktop applications.
 */
void AppMenuComponent::openFolder(Array<String> categoryNames)
{
    int folderIndex = selected[activeColumn()]->getIndex() -
            appConfig.getFavorites().size();
    AppConfigFile::AppFolder selectedFolder =
            appConfig.getFolders()[folderIndex];
    std::set<DesktopEntry> folderItems =
            desktopEntries.getCategoryListEntries(categoryNames);
    if (folderItems.empty() && selectedFolder.pinnedApps.isEmpty())return;
    int columnTop = y_origin;
    if (selected[activeColumn()] != nullptr)
    {
        columnTop = selected[activeColumn()]->getY();
    }
    selected.push_back(nullptr);
    columnTops.push_back(columnTop);
    buttonColumns.push_back(std::vector<AppMenuButton::Ptr>());
    for (AppConfigFile::AppItem item : selectedFolder.pinnedApps)
    {
        AppMenuButton::Ptr addedButton;
        if (buttonNameMap[item.name] != nullptr &&
                buttonNameMap[item.name]->getParentComponent() == nullptr)
        {
            addedButton = buttonNameMap[item.name];
            addedButton->setIndex(buttonColumns[activeColumn()].size());
            addedButton->setColumn(activeColumn());
        } else
        {
            addedButton = new ConfigAppButton(item,
                    buttonColumns[activeColumn()].size(),
                    activeColumn(), iconThread);
        }
        addButton(addedButton);
    }
    DBG(String("found ") + String(folderItems.size()) + " items in folder");
    for (DesktopEntry desktopEntry : folderItems)
    {
        if (!desktopEntry.getValue(DesktopEntry::hidden)
                && !desktopEntry.getValue(DesktopEntry::hidden))
        {
            String name = desktopEntry.getValue(DesktopEntry::name);
            AppMenuButton::Ptr addedButton;
            if (buttonNameMap[name] != nullptr &&
                    buttonNameMap[name]->getParentComponent() == nullptr)
            {
                addedButton = buttonNameMap[name];
                addedButton->setIndex(buttonColumns[activeColumn()].size());
                addedButton->setColumn(activeColumn());
            } else
            {
                addedButton = new DesktopEntryButton(desktopEntry,
                        buttonColumns[activeColumn()].size(),
                        activeColumn(), iconThread);
            }
            addButton(addedButton);
        }
    }
    selected[activeColumn()] = buttonColumns[activeColumn()][0];
    buttonColumns[activeColumn()][0]->setSelected(true);
    scrollToSelected();
}

//close the topmost open folder, removing all contained buttons

void AppMenuComponent::closeFolder()
{
    if (isLoading())
    {
        return;
    }
    for (int i = buttonColumns[activeColumn()].size() - 1; i >= 0; i--)
    {
        AppMenuButton::Ptr toRemove = buttonColumns[activeColumn()][i];
        toRemove->setVisible(false);
        toRemove->setSelected(false);
        Component * parent = toRemove->getParentComponent();
        if (parent != nullptr)
        {
            parent->removeChildComponent(toRemove);
        }
        buttonColumns[activeColumn()].pop_back();
    }
    selected.pop_back();
    buttonColumns.pop_back();
    columnTops.pop_back();
    scrollToSelected();
}


//handle all AppMenuButton clicks

void AppMenuComponent::buttonClicked(Button * buttonClicked)
{
    if (isLoading())
    {
        return;
    }
    AppMenuButton::Ptr appClicked = dynamic_cast<AppMenuButton*>(buttonClicked);
    if (appClicked->getColumn() < activeColumn())
    {
        while (appClicked->getColumn() < activeColumn())
        {
            closeFolder();
        }
        selectIndex(appClicked->getIndex());
        return;
    }
    if (selected[activeColumn()] == appClicked)
    {
        if (appClicked->isFolder())
        {
            openFolder(appClicked->getCategories());
        } else
        {
            appLauncher.startOrFocusApp(appClicked->getAppName()
                    ,appClicked->getCommand());
        }
    } else
    {
        selectIndex(appClicked->getIndex());
    }
}

void AppMenuComponent::resized()
{
    loadingSpinner->setBounds(getWindowSize());
    if (loadingAsync)
    {
        showLoadingSpinner();
    }
    ComponentConfigFile& config = PocketHomeApplication::getInstance()
            ->getComponentConfig();
    ComponentConfigFile::ComponentSettings menuSettings =
            config.getComponentSettings(ComponentConfigFile::appMenuKey);
    Rectangle<int> menuBounds = menuSettings.getBounds();
    x_origin = menuBounds.getX();
    y_origin = menuBounds.getY();
    //resize all buttons
    Rectangle<int>buttonSize = AppMenuButton::getButtonSize();
    int buttonWidth = buttonSize.getWidth();
    int buttonHeight = buttonSize.getHeight();
    int numColumns = selected.size();
    if (menuBounds.getWidth() < numColumns * buttonWidth)
    {
        menuBounds.setWidth(numColumns * buttonWidth);
    }
    for (int c = 0; c < numColumns; c++)
    {
        if (c > 0)
        {
            columnTops[c] = selected[c - 1]->getY();
        }
        int numRows = buttonColumns[c].size();
        if (menuBounds.getHeight() < numRows * buttonHeight + columnTops[c])
        {
            menuBounds.setHeight(numRows * buttonHeight + columnTops[c]);
        }
        for (int i = 0; i < numRows; i++)
        {
            AppMenuButton * button = buttonColumns[c][i];
            button->setBounds(buttonSize.withPosition(c*buttonWidth,
                    i * buttonHeight + columnTops[c]));
        }
    }
    setBounds(menuBounds);
    if (activeColumn() >= 0 && selected[activeColumn()] != nullptr
            && !Desktop::getInstance().getAnimator().isAnimating())
    {
        scrollToSelected();
    }
}


//if it loses visibility, stop waiting for apps to launch

void AppMenuComponent::visibilityChanged()
{
    if (loadingAsync)
    {
        showLoadingSpinner();
    } else if (!isVisible())
    {
        stopWaitingForLoading();
    }
}

void AppMenuComponent::selectIndex(int index)
{
    if (isLoading())
    {
        return;
    }
    int column = activeColumn();
    if (index >= buttonColumns[column].size()
            || index < 0
            || selected[column] == buttonColumns[column][index])return;
    if (selected[column] != nullptr)
    {
        selected[column]->setSelected(false);
        selected[column]->repaint();
    }
    selected[column] = buttonColumns[column][index];
    selected[column]->setSelected(true);
    selected[column]->repaint();
    //move AppMenu to center the selected button, if it's not near an edge
    scrollToSelected();
}

//Select the next appMenuButton in the active button column.

void AppMenuComponent::selectNext()
{
    if (selected[activeColumn()] == nullptr)
    {
        selectIndex(0);
    } else
    {
        selectIndex(selected[activeColumn()]->getIndex() + 1);
    }
}

//Select the previous appMenuButton in the active button column.

void AppMenuComponent::selectPrevious()
{
    if (selected[activeColumn()] == nullptr)
    {
        selectIndex(0);
    } else
    {
        selectIndex(selected[activeColumn()]->getIndex() - 1);
    }
}

//Trigger a click for the selected button.

void AppMenuComponent::clickSelected()
{
    if (selected[activeColumn()] != nullptr && !isLoading())
    {
        selected[activeColumn()]->triggerClick();
    }
}

/**
 * Returns a popup editor component for updating the selected button.
 */
PopupEditorComponent* AppMenuComponent::getEditorForSelected()
{
    if (selected[activeColumn()] != nullptr && !isLoading())
    {
        return selected[activeColumn()]->getEditor();
    }
    return nullptr;
}

//return the index of the active button column.

int AppMenuComponent::activeColumn()
{
    return selected.size() - 1;
}

void AppMenuComponent::scrollToSelected()
{
    int column = activeColumn();
    if (column < 0)
    {
        return;
    }
    AppMenuButton::Ptr selectedButton = selected[column];
    Rectangle<int>dest = getBounds();

    Rectangle<int>buttonSize = AppMenuButton::getButtonSize();
    int buttonWidth = buttonSize.getWidth();
    int buttonHeight = buttonSize.getHeight();
    if (selectedButton != nullptr && selectedButton->isVisible())
    {
        int buttonPos = selectedButton->getY();
        int screenHeight = getWindowSize().getHeight();
        int distanceFromCenter = abs(buttonPos - getY() + screenHeight / 2);
        //only scroll vertically if selected button is outside the center 3/5 
        if (distanceFromCenter > screenHeight / 5 * 3)
        {
            dest.setY(y_origin - buttonPos + screenHeight / 2 - buttonHeight / 2);
        }
        if (dest.getY() > y_origin)
        {
            dest.setY(y_origin);
        }
    } else dest.setY(y_origin - columnTops[column]);
    if (column == 0)dest.setX(x_origin);
    else
    {
        dest.setX(x_origin - column * buttonWidth + buttonHeight);
    }
    ComponentAnimator& animator = Desktop::getInstance().getAnimator();
    if (animator.isAnimating(this))
    {
        animator.cancelAnimation(this, false);
    }
    animator.animateComponent(this, dest, 1, 100, false, 1, 1);
}

void AppMenuComponent::addButton(AppMenuButton::Ptr appButton)
{
    String name = appButton->getAppName();
    Rectangle<int>buttonSize = AppMenuButton::getButtonSize();
    int buttonWidth = buttonSize.getWidth();
    int buttonHeight = buttonSize.getHeight();
    if (buttonNameMap[name] == nullptr)
    {
        buttonNameMap[name] = appButton;
    }
    int index = appButton->getIndex();
    int column = appButton->getColumn();
    int x = column*buttonWidth;
    int y = columnTops[column] + buttonHeight * buttonColumns[column].size();
    appButton->setBounds(x, y, buttonWidth, buttonHeight);
    addAndMakeVisible(appButton);
    appButton->setEnabled(true);
    appButton->setVisible(true);
    appButton->addListener(this);
    this->buttonColumns[column].push_back(appButton);
    if ((x + buttonWidth) > getWidth())
    {
        setBounds(getX(), getY(), x + buttonWidth, getHeight());
    }
    if ((y + buttonHeight) > getHeight())
    {
        setBounds(getX(), getY(), getWidth(), y + buttonHeight);
    }
}

//################## Application Launching #################################

/**
 * @return true if currently loading information or a new child process.
 */
bool AppMenuComponent::isLoading()
{
    return loadingSpinner->isShowing();
}

/**
 * Makes the menu stop waiting to load something, re-enabling
 * user input.
 */
void AppMenuComponent::stopWaitingForLoading()
{
    hideLoadingSpinner();
    appLauncher.stopTimer();
}

void AppMenuComponent::showLoadingSpinner()
{
    Component * parentPage = getParentComponent();
    if (parentPage != nullptr)
    {
        parentPage->addAndMakeVisible(loadingSpinner);
    }
}

void AppMenuComponent::hideLoadingSpinner()
{
    if (!loadingAsync)
    {
        Component * parentPage = getParentComponent();
        if (parentPage != nullptr)
        {
            parentPage->removeChildComponent(loadingSpinner);
        }
    }
}
