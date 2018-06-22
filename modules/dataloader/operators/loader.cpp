/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <iostream>
#include <ghoul/filesystem/file.h>
#include <ghoul/filesystem/filesystem.h>

#include <ghoul/lua/lua_helper.h>
#include <string>
#include <modules/dataloader/operators/loader.h>
#include <modules/dataloader/dataloadermodule.h>
#include <openspace/scene/scene.h>
#include <ghoul/logging/logmanager.h>
#include <openspace/properties/triggerproperty.h>
#include <openspace/scene/scenegraphnode.h>
#include <modules/dataloader/helpers.cpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
constexpr const char* _loggerCat = "Loader";
constexpr const char* KeyTime = "Time";
const std::string KeyRenderableType = "RenderableTimeVaryingVolume";
const std::string scaleTypeKey = "StaticScale";
const std::string translationTypeKey = "SpiceTranslation";
const std::string volumesGuiPathKey = "/Solar System/Volumes";

const bool guiHidden = false;
} 

namespace {
  static const openspace::properties::Property::PropertyInfo SelectedFilesInfo = {
      "SelectedFiles",
      "List of selected files and ready to load",
      "This list contains names of selected files in char format"
  };

  static const openspace::properties::Property::PropertyInfo UploadDataTriggerInfo = {
      "UploadDataTrigger",
      "Trigger load data files",
      "If this property is triggered it will call the function to load data"
  };

  static const openspace::properties::Property::PropertyInfo UploadedDataDimensionsInfo = {
      "UploadedDataDimensions",
      "The Dimensions of an uploaded data set",
      "This property contains a three dimensional vector with the dimensional "
      "values of the data that is to be uploaded"
  };

  static const openspace::properties::Property::PropertyInfo UploadedDataVariableInfo = {
      "UploadedDataVariable",
      "The variable from the volume data that is to be processed for visualization",
      "This property says what variable is to be read from the volume data "
      "to process for visualization in the engine."
  };

  static const openspace::properties::Property::PropertyInfo UploadedDataFactorRSquaredInfo = {
      "UploadedDataFactorRSquared",
      "Whether or not to factor the uploaded data with the radial component squared",
      "This property says whether or not the processing of the data should "
      "multiply each variable value with the radius squared"
  };

  static const openspace::properties::Property::PropertyInfo UploadedDataLowerDomainBoundsInfo = {
      "UploadedDataLowerDomainBounds",
      "Lower domain bounds for data to be uploaded.",
      "This property contains a vector of the lower domain bounds for the data "
      "that is to be uploaded and processed for visualization."
  };

  static const openspace::properties::Property::PropertyInfo UploadedDataHigherDomainBoundsInfo = {
      "UploadedDataHigherDomainBounds",
      "Higher domain bounds for data to be uploaded.",
      "This property contains a vector of the higher domain bounds for the data "
      "that is to be uploaded and processed for visualization."
  };
}

namespace openspace::dataloader {

Loader::Loader() 
    : PropertyOwner({ "Loader" })
    , _filePaths(SelectedFilesInfo)
    , _uploadDataTrigger(UploadDataTriggerInfo)
    , _uploadedDataDimensions(UploadedDataDimensionsInfo)
    , _uploadedDataVariable(UploadedDataVariableInfo)
    , _uploadedDataFactorRSquared(UploadedDataFactorRSquaredInfo)
    , _uploadedDataLowerDomainBounds(UploadedDataLowerDomainBoundsInfo)
    , _uploadedDataHigherDomainBounds(UploadedDataHigherDomainBoundsInfo)
{
    _uploadDataTrigger.onChange([this](){
        uploadData();
    });

    addProperty(_filePaths);
    addProperty(_uploadDataTrigger);

    addProperty(_uploadedDataDimensions);
    addProperty(_uploadedDataVariable);
    addProperty(_uploadedDataFactorRSquared);
    addProperty(_uploadedDataLowerDomainBounds);
    addProperty(_uploadedDataHigherDomainBounds);
}

void Loader::uploadData() {

  // Linux
  #ifdef _linux
  system("thunar /home/mberg");

  // Windows 
  #elif _WIN32

  char filepath[ MAX_PATH ];

  OPENFILENAME ofn;
    ZeroMemory( &filepath, sizeof( filepath ) );
    ZeroMemory( &ofn,      sizeof( ofn ) );
    ofn.lStructSize  = sizeof( ofn );
    ofn.hwndOwner    = NULL;  // If you have a window to center over, put its HANDLE here
    ofn.lpstrFilter  = "Text Files\0*.txt\0Any File\0*.*\0";
    ofn.lpstrFile    = filepath;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrTitle   = "Upload Data";
    ofn.Flags        = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
  
  if (GetOpenFileNameA( &ofn ))
  {
	// ghoul::filesystem::Directory fileDir(filepath);    
    // _filePaths = fileDir.readDirectories(
    //   ghoul::filesystem::Directory::Recursive::No,
    //   ghoul::filesystem::Directory::Sort::Yes
    // );
    _filePaths = filepath;
  }
  else
  {
    // All the below is to print incorrect user input. 
    switch (CommDlgExtendedError())
    {
      case CDERR_DIALOGFAILURE   : std::cout << "CDERR_DIALOGFAILURE\n";   break;
      case CDERR_FINDRESFAILURE  : std::cout << "CDERR_FINDRESFAILURE\n";  break;
      case CDERR_INITIALIZATION  : std::cout << "CDERR_INITIALIZATION\n";  break;
      case CDERR_LOADRESFAILURE  : std::cout << "CDERR_LOADRESFAILURE\n";  break;
      case CDERR_LOADSTRFAILURE  : std::cout << "CDERR_LOADSTRFAILURE\n";  break;
      case CDERR_LOCKRESFAILURE  : std::cout << "CDERR_LOCKRESFAILURE\n";  break;
      case CDERR_MEMALLOCFAILURE : std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
      case CDERR_MEMLOCKFAILURE  : std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
      case CDERR_NOHINSTANCE     : std::cout << "CDERR_NOHINSTANCE\n";     break;
      case CDERR_NOHOOK          : std::cout << "CDERR_NOHOOK\n";          break;
      case CDERR_NOTEMPLATE      : std::cout << "CDERR_NOTEMPLATE\n";      break;
      case CDERR_STRUCTSIZE      : std::cout << "CDERR_STRUCTSIZE\n";      break;
      case FNERR_BUFFERTOOSMALL  : std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
      case FNERR_INVALIDFILENAME : std::cout << "FNERR_INVALIDFILENAME\n"; break;
      case FNERR_SUBCLASSFAILURE : std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
      default                    : std::cout << "You cancelled.\n";
    }
  }
  // MAC
  #elif __APPLE__
  // Still to do
  #endif

;
}

void Loader::createInternalDataItemProperties() {
    module()->validateDataDirectory();
    std::vector<std::string> volumeItems = module()->volumeDataItems();

    LDEBUG("volume items vec size " + std::to_string(volumeItems.size()));

    for (auto item : volumeItems) {
        const std::string dirLeaf = openspace::dataloader::helpers::getDirLeaf(item);
        const openspace::properties::Property::PropertyInfo info = {
            "ItemTrigger_" + dirLeaf,
            dirLeaf,
            ""
        };

        // Initialize trigger property with data item name (are they unique?)
        auto volumeItemTrigger = properties::TriggerProperty(info);

        // Set onChange method to call loadDataItem with the path as argument
        volumeItemTrigger.onChange([this](){
            // loadDataItem(item);
        });

        // addProperty(volumeItemTrigger);
        // LDEBUG("Added property " + dirLeaf);
    }
}

// addDataItemProperty();
// removeDataItemProperties();

// void Loader::createVolumeDataItem(std::string absPath) {}

void Loader::loadDataItem(std::string absPathToItem) {
    std::string sourceDir = absPathToItem;
    const std::string dirLeaf = openspace::dataloader::helpers::getDirLeaf(absPathToItem);

    // TODO: Variables to let user initialize in GUI
    std::string sunKey = "SUN";
    float sunRadiusScale = 695508000;
    const std::string parent = "SolarSystemBarycenter"; // valid for all volume data?
    
    std::string stateFile = openspace::dataloader::helpers::getFileWithExtensionFromItemFolder(absPathToItem, "state");
    ghoul::Dictionary renderableDictionary = ghoul::lua::loadDictionaryFromFile(stateFile);

    std::string tfFilePath = absPathToItem +
        ghoul::filesystem::FileSystem::PathSeparator +
        "transferfunction.txt";

    renderableDictionary.setValue("Type", KeyRenderableType);
    renderableDictionary.setValue("SourceDirectory", sourceDir);
    renderableDictionary.setValue("TransferFunction", tfFilePath);

    std::initializer_list<std::pair<std::string, ghoul::any>> translation = {
        std::make_pair( "Type", translationTypeKey ),
        std::make_pair( "Target", sunKey ),
        std::make_pair( "Observer", sunKey )
    };
    std::initializer_list<std::pair<std::string, ghoul::any>> scale = {
        std::make_pair( "Type", scaleTypeKey ),
        std::make_pair( "Scale", sunRadiusScale )
    };
    ghoul::Dictionary translationDictionary(translation);
    ghoul::Dictionary scaleDictionary(scale);

    std::initializer_list<std::pair<std::string, ghoul::any>> transform = {
        std::make_pair( "Translation", translationDictionary ),
        std::make_pair( "Scale", scaleDictionary )
    };
    ghoul::Dictionary transformDictionary(transform);

    std::initializer_list<std::pair<std::string, ghoul::any>> gui = {
        std::make_pair( "Path", volumesGuiPathKey ),
        std::make_pair( "Hidden", guiHidden ),
    };
    ghoul::Dictionary guiDictionary(gui);

    std::initializer_list<std::pair<std::string, ghoul::any>> completeList = {
        std::make_pair( "Identifier", dirLeaf ),
        std::make_pair( "Parent", parent ),
        std::make_pair( "Renderable", renderableDictionary ),
        std::make_pair( "Transform", transformDictionary ),
        std::make_pair( "GUI", guiDictionary )
    };

    const ghoul::Dictionary completeDictionary(completeList);
    initializeNode(completeDictionary);

    goToFirstTimeStep(absPathToItem);
}

void Loader::initializeNode(ghoul::Dictionary dict) {
    SceneGraphNode* node = scene()->loadNode(dict);
    scene()->initializeNode(node);
}

void Loader::goToFirstTimeStep(std::string absPathToItem) {
    std::string firstDictionaryFilePath = openspace::dataloader::helpers::getFileWithExtensionFromItemFolder(absPathToItem, "dictionary");
    ghoul::Dictionary dict = ghoul::lua::loadDictionaryFromFile(firstDictionaryFilePath);
    std::string firstTimeStep = dict.value<std::string>(KeyTime);
    time().setTime(firstTimeStep);
}

void Loader::processCurrentlySelectedUploadData() {
    // Case 1: _filePaths is a string of a single path to a single CDF file

    // Determine path to new volume item
    std::string itemPathBase = "${DATA}/.internal/volumes_from_cdf/";

    // Create dictionary with properties as values
    ghoul::Dictionary conversionDictionary = createTaskDictionaryForOneVolumeItem(_filePaths, itemPathBase);

    // Schedule tasks? How to loop through several CDF timesteps and run VolumeToRaw for each
    // Create instance of KameleonVolumeToRaw and run
    // KameleonVolumeToRawTask kameleonVolumeToRawTask(conversionDictionary);
    // kameleonVolumeToRawTask.perform();

    // Create state file, transferfunction file in volume item directory
}

// void Loader::createVolumeDataItem(std::string absPath) {}

ghoul::Dictionary Loader::createTaskDictionaryForOneVolumeItem(std::string inputPath, std::string outputBasePath) {

    // defaults
    // const int dimensions[3] = {100, 100, 128};
    // const int lowerDomainBound[3] = {1, -90, 0};
    // const int upperDomainBound[3] = {15, 90, 360};

    // Set item dirLeaf as name
    const std::string itemName = openspace::dataloader::helpers::getDirLeaf(inputPath);
    const std::string itemOutputFilePath = outputBasePath + 
        ghoul::filesystem::FileSystem::PathSeparator +
        itemName;

    const std::string RawVolumeOutput = itemOutputFilePath + ".rawvolume";
    const std::string DictionaryOutput = itemOutputFilePath + ".dictionary";

    std::initializer_list<std::pair<std::string, ghoul::any>> list = {
        std::make_pair( "Type", "KameleonVolumeToRawTask" ),
        std::make_pair( "Input", inputPath ),
        std::make_pair( "Dimensions", _uploadedDataDimensions ),
        std::make_pair( "Variable", _uploadedDataVariable),
        std::make_pair( "FactorRSquared", _uploadedDataFactorRSquared ),
        std::make_pair( "LowerDomainBound", _uploadedDataLowerDomainBounds ),
        std::make_pair( "UpperDomainBound", _uploadedDataHigherDomainBounds ),
        std::make_pair( "RawVolumeOutput", RawVolumeOutput ),
        std::make_pair( "DictionaryOutput", DictionaryOutput)
    };

    return ghoul::Dictionary(list);
}

// ghoul::Dictionary Loader::createVolumeItemDictionary(std::string dataDictionaryPath, std::string dataStatePath) {

// }

}
