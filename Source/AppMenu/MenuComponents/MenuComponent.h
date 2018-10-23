/* Only include this file directly in the AppMenu implementation! */
#ifdef APPMENU_IMPLEMENTATION_ONLY

#pragma once
#include "Controller.h"
#include "AppMenu.h"

/**
 * @file MenuComponent.h
 *
 * @brief  Creates and arranges application menu folder components.
 */
class AppMenu::MenuComponent : public Controller::ControlledMenu
{
public:
    /**
     * @brief  Creates the MenuComponent, linking it with its controller.
     *
     * @param controller  The controller that manages this menu item.
     */
    MenuComponent(Controller& controller);

    virtual ~MenuComponent() { }

    /**
     * @brief  Gets the MenuItem for the current active folder component.
     *
     * @return  An object holding the last opened menu folder's data.
     */
    virtual MenuItem getActiveFolder() final override;

    /**
     * @brief  Represents a list of shortcuts and folders within the application
     *         menu.
     */
    class Folder : public juce::Component
    {
    public:
        /**
         * @brief  Creates a new folder component.
         *
         * @param folderItem     The menu data object used to construct the
         *                       folder.
         *
         * @param menuComponent  The menuComponent that created this folder
         *                       component.
         */
        Folder(MenuItem folderItem, MenuComponent& menuComponent);

        virtual ~Folder() { }

        /**
         * @brief  Gets the MenuItem that defines this folder.
         *
         * @return   The menu data object used to construct the folder.
         */
        MenuItem getFolderItem() const;
        
    protected:
        /**
         * @brief  Signals to this Folder's MenuComponent that a menu item was
         *         clicked.
         *
         * @param clickedItem   The selected menu item.
         *
         * @param rightClicked  Whether the selected item was right-clicked.
         */
        void signalItemClicked(MenuItem clickedItem, const bool rightClicked);

        /**
         * @brief  Signals to this Folder's MenuComponent that the folder was
         *         clicked.
         *
         * @param rightClicked  Whether the folder was right clicked.
         *
         * @param closestIndex  The closest menu index to the spot that was
         *                      clicked.
         */
        void signalFolderClicked
        (const bool rightClicked, const int closestIndex = -1);

    private:
        /* The menu component that owns this folder */
        MenuComponent& menuComponent;
        /* The MenuItem that defines this folder */
        MenuItem folderItem;
    };
protected:
    /**
     * @brief  Creates a new folder component.
     *
     * @param folderItem  The folder item that will define the new folder
     *                    component.
     *
     * @return            The new component, as some concrete subclass of
     *                    MenuComponent::Folder.
     */
    virtual Folder* createFolderComponent(MenuItem folderItem) = 0;

    /**
     * @brief  Updates the positions and sizes of all open folder components.
     */
    virtual void updateMenuLayout() = 0;

    /**
     * @brief  Gets the number of open folder components.
     *
     * @return   The open folder count.
     */
    int openFolderCount() const;

    /**
     * @brief  Gets an open folder component from the list of open folders.
     *
     * @param folderIndex  The index of a folder component in the list of open
     *                     folders.
     *
     * @return             The component's address, or nullptr if the index is
     *                     out of bounds.
     */
    Folder* getOpenFolder(const int folderIndex) const;

private:
    /**
     * @brief  Creates, shows, and activates a new folder component.
     *
     * @param folderItem  The new folder's menu data.
     */
    virtual void openFolder(MenuItem folderItem) final override;

    /**
     * @brief  Closes the current active folder as long as more than one
     *         folder is open. 
     */
    virtual void closeActiveFolder() final override;

    /**
     * @brief  Handles menu navigation key events not handled by the folder
     *         component.
     *
     * @param key  Describes the key or keys that were pressed.
     *
     * @return     True to indicate that the key event was handled, false to
     *             allow the key event to be passed on to other objects.
     */
    virtual bool keyPressed(const juce::KeyPress& key) final override; 

    /**
     * @brief  notifies the controller when the menu is clicked.
     * 
     * @param event  a mouse event that may represent a click on the main menu
     *               component.
     */
    virtual void mouseDown(const juce::MouseEvent& event) final override;

    /**
     * @brief  Updates the menu layout when the component is resized.
     */
    virtual void resized() final override;
    
    /* The list of open folder components */
    juce::OwnedArray<Folder> openFolders;
};

/* Only include this file directly in the AppMenu implementation! */
#endif
