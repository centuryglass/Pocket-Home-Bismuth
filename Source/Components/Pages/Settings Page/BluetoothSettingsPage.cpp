#include "Utils.h"
#include "BluetoothSettingsPage.h"

BluetoothSettingsPage::BluetoothSettingsPage
(BluetoothStatus& bluetoothStatus) :
ConnectionPage<BluetoothDevice::Ptr>(),
bluetoothStatus(bluetoothStatus) { }

/**
 * @return the list of all visible bluetooth devices
 */
Array<BluetoothDevice::Ptr> BluetoothSettingsPage::loadConnectionList()
{
    return {};
}

/**
 * Attempts to connect to a Bluetooth device.
 *
 *  @param device
 */
void BluetoothSettingsPage::connect(BluetoothDevice::Ptr device) { }

/**
 * @param device if the system is currently connected to this
 * bluetooth device, this method closes that connection.
 *  
 * @param connection
 */
void BluetoothSettingsPage::disconnect(BluetoothDevice::Ptr device) { }

/**
 * @param connection
 * 
 * @return true iff the system is connected to this bluetooth device.
 */
bool BluetoothSettingsPage::isConnected(BluetoothDevice::Ptr device)
{
    return device->connected;
}

/**
 * @param button
 * 
 * This is called whenever a button other than the navigation buttons
 * is clicked.
 */
void BluetoothSettingsPage::connectionButtonClicked(Button* button) { }

/**
 * Construct a button component to represent a bluetooth device.
 * This button will display the device name:
 * TODO: design connection button
 * 
 * @param device
 */
Button* BluetoothSettingsPage::getConnectionButton
(BluetoothDevice::Ptr device)
{
    return new BTDeviceButton(device, isConnected(device));
}

/**
 * Get the layout for the Bluetooth device controls.
 * 
 * @param connection The control components will be updated to suit
 *                    this bluetooth device.
 */
RelativeLayoutManager::Layout
BluetoothSettingsPage::getConnectionControlsLayout
(BluetoothDevice::Ptr device)
{
    return {};
}

/**
 * Update connection control components to match the current bluetooth
 * device connection state and the provided Bluetooth device.
 * 
 * @param BluetoothDevice
 */
void BluetoothSettingsPage::updateConnectionControls
(BluetoothDevice::Ptr device) { }

/**
 * When currentlyConnecting, disable bluetooth controls and show a loading
 * spinner.  Otherwise, enable controls and hide the loading spinner.
 * 
 * @param currentlyConnecting Indicates if bluetooth is trying to connect 
 *                             to a device, or is otherwise busy.
 */
void BluetoothSettingsPage::setCurrentlyConnecting
(bool currentlyConnecting) { }

/**
 * Attempts to connect if return is pressed.
 * 
 * @param editor
 */
void BluetoothSettingsPage::textEditorReturnKeyPressed
(TextEditor& editor) { }

/**
 * Set the spinner's bounds within the connection button.
 */
void BluetoothSettingsPage::connectionPageResized() { }

/**
 * Reload the device list, re-select the selected connection, 
 * update and enable connection controls.
 */
void BluetoothSettingsPage::reloadPage() { }

BluetoothSettingsPage::BTDeviceButton::BTDeviceButton(
        BluetoothDevice::Ptr connection,
        bool isConnected) :
Button(connection->name + String("Button")),
deviceLabel("deviceLabel", connection->name) { }

void BluetoothSettingsPage::BTDeviceButton::resized() { }