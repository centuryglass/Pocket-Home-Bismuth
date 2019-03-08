########################## Settings Module #####################################
SETTINGS_ROOT = Source/Settings
SETTINGS_PREFIX = Settings_
SETTINGS_OBJ := $(JUCE_OBJDIR)/$(SETTINGS_PREFIX)

OBJECTS_SETTINGS := \
  $(SETTINGS_OBJ)ClockModePicker.o \
  $(SETTINGS_OBJ)BrightnessSlider.o \
  $(SETTINGS_OBJ)VolumeSlider.o \
  $(SETTINGS_OBJ)CursorModePicker.o \
  $(SETTINGS_OBJ)CalibrationButton.o \
  $(SETTINGS_OBJ)ConnectionComponent.o \
  $(SETTINGS_OBJ)ListPage.o 

OBJECTS_SETTINGS_TEST :=

ifeq ($(BUILD_TESTS), 1)
    OBJECTS_SETTINGS := $(OBJECTS_SETTINGS) $(OBJECTS_SETTINGS_TEST)
endif

MODULES := $(MODULES) settings

OBJECTS_APP := $(OBJECTS_APP) $(OBJECTS_SETTINGS)

settings : $(OBJECTS_SETTINGS)
	@echo "Built Settings module"

$(SETTINGS_OBJ)ClockModePicker.o : \
    $(SETTINGS_ROOT)/$(SETTINGS_PREFIX)ClockModePicker.cpp
$(SETTINGS_OBJ)BrightnessSlider.o : \
    $(SETTINGS_ROOT)/$(SETTINGS_PREFIX)BrightnessSlider.cpp
$(SETTINGS_OBJ)VolumeSlider.o : \
    $(SETTINGS_ROOT)/$(SETTINGS_PREFIX)VolumeSlider.cpp
$(SETTINGS_OBJ)CursorModePicker.o : \
    $(SETTINGS_ROOT)/$(SETTINGS_PREFIX)CursorModePicker.cpp
$(SETTINGS_OBJ)CalibrationButton.o : \
    $(SETTINGS_ROOT)/$(SETTINGS_PREFIX)CalibrationButton.cpp
$(SETTINGS_OBJ)ConnectionComponent.o : \
    $(SETTINGS_ROOT)/QuickSettingsPage/ConnectionSettingsComponent.cpp
$(SETTINGS_OBJ)ListPage.o : \
    $(SETTINGS_ROOT)/SettingsListPage/SettingsListPage.cpp
