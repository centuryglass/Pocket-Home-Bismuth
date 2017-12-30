#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "Pages/LauncherComponent.h"
#include "Pages/PageStackComponent.h"
#include "SettingsPageLogin.h"

class MainContentComponent;

class LoginPage : public Component, public Button::Listener, public TextEditor::Listener{
public:
  LoginPage(std::function<void(void)>);
  ~LoginPage();
  
  virtual void resized() override;
  virtual void paint(Graphics &) override;
  virtual void buttonClicked(Button *button) override;
  virtual bool hasPassword();
  virtual void textFocus();
  virtual void textEditorReturnKeyPressed(TextEditor&) override;
  
private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginPage)
  void displayError();
  
  std::function<void(void)> functionToExecute;
  bool foundPassword;
  ScopedPointer<TextButton> log;
  ScopedPointer<Label> label_password;
  String hashed_password;
  ScopedPointer<DrawableImage> bgImage;
  ScopedPointer<DrawableImage> ntcIcon;
  ScopedPointer<TextEditor> cur_password;
};

class MainContentComponent : public Component {
public:
  ScopedPointer<LauncherComponent> launcher = nullptr;
  ScopedPointer<LookAndFeel> lookAndFeel = nullptr;
  ScopedPointer<PageStackComponent> pageStack = nullptr;

  MainContentComponent();
  ~MainContentComponent();

  void paint(Graphics &) override;
  void resized() override;
  void loggedIn();
  
  void handleMainWindowInactive();

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
  ScopedPointer<LoginPage> loginPage;
};