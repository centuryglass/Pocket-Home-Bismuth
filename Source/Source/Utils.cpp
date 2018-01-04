
#include <sys/stat.h>
#include <dirent.h>
#include <locale.h>
#include "Utils.h"
using namespace juce;

File absoluteFileFromPath(const String &path) {
    return File::isAbsolutePath(path) ? File(path)
            : File::getCurrentWorkingDirectory().getChildFile(path);

}


// TODO: allow user overrides of asset files

File assetFile(const String &fileName) {
    auto devFile = absoluteFileFromPath("../../assets/" + fileName);
    File assetFile;

#if JUCE_LINUX
    // are we linux? look in /usr/share/
    // FIXME: don't hardcode this, maybe find it via .deb configuration
    File linuxAssetFile = absoluteFileFromPath("/usr/share/pocket-home/" + fileName);
    // look in relative path, used in development builds
    assetFile = linuxAssetFile.exists() ? linuxAssetFile : devFile;
    if (assetFile == devFile && !devFile.exists())
        assetFile = absoluteFileFromPath(fileName);
#else
    assetFile = devFile;
#endif

    return assetFile;
}

Image createImageFromFile(const File &imageFile) {
    auto image = Image(Image::ARGB, 128, 128, true);
    if (imageFile.getFileExtension() == ".svg") {
        ScopedPointer<Drawable> svgDrawable = createSVGDrawable(imageFile);
        Graphics g(image);
        svgDrawable->drawWithin(g, Rectangle<float>(0, 0, image.getWidth(), image.getHeight()),
                RectanglePlacement::fillDestination, 1.0f);
    } else {
        image = ImageFileFormat::loadFrom(imageFile);
    }
    return image;
}

ImageButton *createImageButton(const String &name, const File &imageFile) {
    auto image = createImageFromFile(imageFile);
    return createImageButton(name, image);
}

ImageButton *createImageButton(const String &name, const Image &image) {
    auto imageButton = new ImageButton("Back");
    // FIXME: to support touch areas of different size from the base image,
    // we need to explicitly size the images within image buttons when necessary,
    // rather than relying on the resizing parameters used here in setImages().
    // Otherwise images are forced to resize whenever we change component size.
    imageButton->setImages(true, true, true,
            image, 1.0f, Colours::transparentWhite, // normal
            image, 1.0f, Colours::transparentWhite, // over
            image, 0.5f, Colours::transparentWhite, // down
            0);
    return imageButton;
}

ImageButton *createImageButtonFromDrawable(const String &name, const Drawable &drawable) {
    auto image = Image(Image::RGB, 128, 128, true);
    Graphics g(image);
    drawable.drawWithin(g, Rectangle<float>(0, 0, image.getWidth(), image.getHeight()),
            RectanglePlacement::fillDestination, 1.0f);
    return createImageButton(name, image);
}

Drawable * createSVGDrawable(const File& svgFile) {
    if (!svgFile.existsAsFile()) {
        DBG(String("createSVGDrawable:") + svgFile.getFileName() + " not found!");
        return nullptr;
    }
    ScopedPointer<XmlElement> svgElement = XmlDocument::parse(svgFile);
    if (svgElement == nullptr) {
        DBG(String("createSVGDrawable:") + svgFile.getFileName() + " not a valid svg!");
        return nullptr;
    }
    return Drawable::createFromSVG(*svgElement);
}

void fitRectInRect(Rectangle<int> &rect, int x, int y, int width, int height,
        Justification justification, const bool onlyReduceInSize) {
    // it's no good calling this method unless both the component and
    // target rectangle have a finite size.
    jassert(rect.getWidth() > 0 && rect.getHeight() > 0 && width > 0 && height > 0);

    if (rect.getWidth() > 0 && rect.getHeight() > 0 && width > 0 && height > 0) {
        int newW, newH;

        if (onlyReduceInSize && rect.getWidth() <= width && rect.getHeight() <= height) {
            newW = rect.getWidth();
            newH = rect.getHeight();
        } else {
            const double imageRatio = rect.getHeight() / (double) rect.getWidth();
            const double targetRatio = height / (double) width;

            if (imageRatio <= targetRatio) {
                newW = width;
                newH = jmin(height, roundToInt(newW * imageRatio));
            } else {
                newH = height;
                newW = jmin(width, roundToInt(newH / imageRatio));
            }
        }

        if (newW > 0 && newH > 0)
            rect = justification.appliedToRectangle(Rectangle<int>(newW, newH),
                Rectangle<int>(x, y, width, height));
    }
}

void fitRectInRect(Rectangle<int> &rect, const Rectangle<int> &container,
        Justification justification, const bool onlyReduceInSize) {
    fitRectInRect(rect, container.getX(), container.getY(), container.getWidth(),
            container.getHeight(), justification, onlyReduceInSize);
}

float smoothstep(float edge0, float edge1, float x) {
    x = std::min(std::max((x - edge0) / (edge1 - edge0), 0.0f), 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

float mix(float a, float b, float t) {
    return t * (b - a) + a;
}

float mapLinear(float x, float imin, float imax, float omin, float omax) {
    return mix(omin, omax, (x - imin) / (imax - imin));
}

void animateTranslation(Component *component, int x, int y, float alpha, int durationMillis) {
    const auto &bounds = component->getBounds();
    auto destBounds = bounds.translated(x - bounds.getX(), y - bounds.getY());
    Desktop::getInstance().getAnimator().animateComponent(component, destBounds, alpha,
            durationMillis, true, 0, 0);
}

std::vector<String> split(const String &orig, const String &delim) {
    std::vector<String> elems;
    int index = 0;
    auto remainder = orig.substring(index);
    while (remainder.length()) {
        index = remainder.indexOf(delim);
        if (index < 0) {
            elems.push_back(remainder);
            break;
        }
        elems.push_back(remainder.substring(0, index));
        remainder = remainder.substring(index + 1);
    }
    return elems;
};

bool fileExists(const String& path) {
    struct stat buffer;
    return (stat(path.toRawUTF8(), &buffer) == 0);
}

String getLocale() {
    std::locale l("");
    std::string localeName = l.name();
    std::string::size_type cutIndex = localeName.find(".");
    if (cutIndex != std::string::npos)localeName.erase(cutIndex);
    return String(localeName);
}

String getHomePath() {
    return String(std::getenv("HOME"));
}

//perform function(struct dirent*) on all files in path

void foreachFile(const String& path, std::function<void(struct dirent*) > fn) {
    DIR * dir = opendir(path.toRawUTF8());
    if (dir != nullptr) {
        struct dirent *dirdata = nullptr;
        do {
            dirdata = readdir(dir);
            if (dirdata != nullptr) {
                fn(dirdata);
            }
        } while (dirdata != nullptr);
        closedir(dir);
    }
}

//List all non-directory files in path

std::vector<String> listFiles(const String& path) {
    std::vector<String> files;
    foreachFile(path, [&files](struct dirent * dirdata) {
        if (dirdata->d_type != DT_DIR && dirdata->d_name != "") {
            files.push_back(dirdata->d_name);
        }
    });
    return files;
}

//list all directory files in path, ignoring ./ and ../

std::vector<String> listDirectoryFiles(const String& path) {
    std::vector<String> directories;
    foreachFile(path, [&directories](struct dirent * dirdata) {
        String name = dirdata->d_name;
        if (dirdata->d_type == DT_DIR
                && name != "." && name != ".." && !name.isEmpty()) {
            directories.push_back(name);
        }
    });
    return directories;
}

//Print debug info about the component tree

void componentTrace() {
    highlightFocus.setFill(FillType(Colour(0x0))); 
    highlightFocus.setStrokeFill(FillType(Colour(0xff00ff00)));
    highlightFocus.setStrokeType(PathStrokeType(4));
    std::function<void(Component*, int) > recursiveInfo;
    recursiveInfo = [&recursiveInfo](Component* component, int depth) {
        String indent;
        for (int i = 0; i < depth; i++) {
            indent += "\t";
        }
        DBG(indent + String("Component:") + component->getName());
        indent += " ";
        DBG(indent + String("Position: (") + String(component->getX()) + String(",") +
                String(component->getY()) + String(")"));
        DBG(indent + String("Size: ") + String(component->getWidth()) + String("x") +
                String(component->getHeight()));
        String properties;
        if (component->getWantsKeyboardFocus()) {
            properties += "wantsKeyFocus,";
        }
        if (component->hasKeyboardFocus(false)) {
            properties += "hasKeyFocus,";
            highlightFocus.setBounds(component->getBounds());
        }
        properties += component->isShowing() ? "showing" : "not showing";
        DBG(indent + properties);
        int numChildren = component->getNumChildComponents();
        if (numChildren > 0) {
            DBG(indent + String("Children:") + String(numChildren));
        }
        for (int i = 0; i < numChildren; i++) {
            recursiveInfo(component->getChildComponent(i), depth + 1);
        }
    };
    Component * rootComponent = Desktop::getInstance().getComponent(0);
    recursiveInfo(rootComponent, 0);
    rootComponent->addAndMakeVisible(highlightFocus);
}

Rectangle<int> getWindowSize() {
    return Desktop::getInstance().getComponent(0)->getBounds().withPosition(0, 0);
}

//resizes a font to fit in a containing rectangle.
//If fitting it in would require mangling the font size too much, the
//font gets set to size zero.

Font fontResizedToFit(Font font, String text, Rectangle<int>container) {
    float currentHeight = font.getHeight();
    float currentWidth = font.getStringWidth(text);
    int newHeight = currentHeight * container.getWidth() / currentWidth;
    if (newHeight > container.getHeight()) {
        newHeight = container.getHeight();
    }
    //DBG(String("setting font height to ")+String(newHeight));
    font.setHeight(newHeight);
    return font;
}