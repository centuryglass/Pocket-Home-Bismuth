#define APPMENU_IMPLEMENTATION_ONLY
#include "ItemData.h"

/*
 * Gets this menu item's parent folder.
 */
AppMenu::ItemData::Ptr 
AppMenu::ItemData::getParentFolder() const
{
    return parent;
}

/*
 * Gets this menu item's index within its parent folder.
 */
int AppMenu::ItemData::getIndex() const
{
    return index;
}

/*
 * Checks if this menu item represents a folder within the menu.
 */
bool AppMenu::ItemData::isFolder() const
{
    return getCommand().isEmpty();
}

/*
 * Gets the number of folder items held by this menu item.
 */
int AppMenu::ItemData::getFolderSize() const
{
    return children.size();
}

/*
 * Gets a menu item contained in a folder menu item.
 */
AppMenu::ItemData::Ptr 
AppMenu::ItemData::getChild(const int childIndex) const
{
    if(childIndex < 0 || childIndex >= children.size())
    {
        return nullptr;
    }
    return children[childIndex];
}

/*
 * Gets all menu items contained in a folder menu item.
 */
juce::Array<AppMenu::ItemData::Ptr> 
AppMenu::ItemData::getChildren() const
{
    return children;
}

/*
 * Attempts to insert a new menu item into this folder menu item's array of 
 * child menu items, saving the change to this folder item's data source.
 */
bool AppMenu::ItemData::insertChild
(const AppMenu::ItemData::Ptr newChild, const int childIndex)
{
    if(childIndex < 0 || childIndex > getMovableChildCount())
    {
        return false;
    }
    children.insert(childIndex, newChild);
    newChild->parent = this;
    newChild->index = childIndex;
    for(int i = childIndex + 1; i < children.size(); i++)
    {
        children[i]->index++;
    }
    newChild->saveChanges();
    foreachListener([this, &childIndex](Listener* listener)
    {
        listener->childAdded(this, childIndex);
    });
    return true;
}

/*
 * Removes this menu item from its folder, deleting it from its data source.
 */
bool AppMenu::ItemData::remove()
{
    if(parent == nullptr || index < 0)
    {
        return false;
    }
    foreachListener([this](Listener* listener)
    {
        listener->removedFromMenu(this);
    });
    parent->children.remove(index);
    for(int i = index; i < parent->children.size(); i++)
    {
        parent->children[i]->index++;
    }
    parent->foreachListener([this](Listener* listener)
    {
        listener->childRemoved(parent, index);
    });
    parent = nullptr;
    index = -1;
    deleteFromSource();
    return true;
}

/*
 * Swaps the positions of two menu items, saving the change to this menu item's
 * data source.
 */
bool AppMenu::ItemData::swapChildren(const int childIdx1, const int childIdx2)
{
    if(index < 0 || index > getMovableChildCount())
    {
        return false;
    }
    children.swap(childIdx1, childIdx2);
    children[childIdx1]->index = childIdx1;
    children[childIdx2]->index = childIdx2;
    foreachListener([this, &childIdx1, &childIdx2](Listener* listener)
    {
        listener->childrenSwapped(this, childIdx1, childIdx2);
    });
    return true;
}

/*
 * Removes all references to this listener from the menu items it tracks.
 */
AppMenu::ItemData::Listener::~Listener()
{
    juce::Array<ItemData::Ptr> trackedListCopy = trackedItemData;
    for(ItemData::Ptr& trackedItem : trackedListCopy)
    {
        trackedItem->removeListener(this);
    }
}

/*
 * Connects a new Listener object to this ItemData.
 */
void AppMenu::ItemData::addListener(Listener* newListener)
{
    if(newListener != nullptr)
    {
        listeners.add(newListener);
        newListener->trackedItemData.add(this);
    }
    else
    {
        DBG("AppMenu::ItemData::" << __func__ << ": Listener was null!");
    }
}

/*
 * Disconnects a Listener object from this ItemData.
 */
void AppMenu::ItemData::removeListener(Listener* toRemove)
{
    if(toRemove != nullptr)
    {
        toRemove->trackedItemData.removeAllInstancesOf(this);
        listeners.removeAllInstancesOf(toRemove);
    }
    else
    {
        DBG("AppMenu::ItemData::" << __func__ << ": Listener was null!");
    }
}

/*
 * Signal to all listeners tracking this ItemData that the item has changed.
 */
void AppMenu::ItemData::signalDataChanged()
{
    foreachListener([this](Listener* listener)
    {
        listener->dataChanged(this);
    });
}

/*
 * Runs an arbitrary operation on each listener tracking this ItemData.
 */
void AppMenu::ItemData::foreachListener
(const std::function<void(Listener*)> listenerAction)
{
    for(Listener* listener : listeners)
    {
        listenerAction(listener);
    }
}