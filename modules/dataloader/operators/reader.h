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

#ifndef __OPENSPACE_MODULE_DATALOADER___READER___H__
#define __OPENSPACE_MODULE_DATALOADER___READER___H__

#include <modules/dataloader/operators/operator.h>
#include <openspace/properties/propertyowner.h>
#include <openspace/properties/property.h> // do we need this
#include <openspace/properties/triggerproperty.h>
#include <ghoul/filesystem/directory.h>

namespace openspace::dataloader {

using properties::PropertyOwner;
using Directory = ghoul::filesystem::Directory;

class Reader : public PropertyOwner, public Operator {
  public:
    Reader();

    Directory getVolumeDir();
    void readVolumeDataItems();

  private:
    ghoul::filesystem::Directory _topDir;

    // properties::TriggerProperty _readVolumesTrigger;
};

}

#endif // __OPENSPACE_MODULE_DATALOADER___READER___H__
