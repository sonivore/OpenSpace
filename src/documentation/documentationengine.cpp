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

#include <openspace/documentation/documentationengine.h>

#include <openspace/openspace.h>
#include <openspace/documentation/core_registration.h>
#include <openspace/documentation/verifier.h>

#include <ghoul/misc/assert.h>
#include <ghoul/filesystem/filesystem.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <streambuf>

#include <ghoul/fmt.h>

namespace {
    const char* HandlebarsFilename = "${WEB}/documentation/handlebars-v4.0.5.js";
    const char* BootstrapFilename = "${WEB}/common/bootstrap.min.css";
    const char* CssFilename = "${WEB}/documentation/style.css";
    const char* JsFilename = "${WEB}/documentation/script.js";
} // namespace


namespace openspace::documentation {

DocumentationEngine* DocumentationEngine::_instance = nullptr;

DocumentationEngine::DuplicateDocumentationException::DuplicateDocumentationException(
                    Documentation doc)
    : ghoul::RuntimeError(fmt::format(
        "Duplicate Documentation with name '{}' and id '{}'",
        doc.name,
        doc.id
    ))
    , documentation(std::move(doc))
{}

DocumentationEngine::DocumentationEngine()
    : DocumentationGenerator(
     "Top Level",
     "toplevel",
        {
            { "toplevelTemplate", "${WEB}/documentation/toplevel.hbs"},
        }
    )
{}


DocumentationEngine& DocumentationEngine::ref() {
    if (_instance == nullptr) {
        _instance = new DocumentationEngine;
        registerCoreClasses(*_instance);
    }
    return *_instance;
}

std::string generateJsonDocumentation(const Documentation& d) {
    std::stringstream result;
    result << "{";

    result << "\"name\": \"" << d.name << "\",";
    result << "\"id\": \"" << d.id << "\",";
    result << "\"entries\": [";
    for (const auto& p : d.entries) {
        result << "{";
        result << "\"key\": \"" << p.key << "\",";
        result << "\"optional\": " << (p.optional ? "true" : "false") << ",";
        result << "\"type\": \"" << p.verifier->type() << "\",";
        result << "\"documentation\": \"" << escapedJson(p.documentation) << "\",";
        TableVerifier* tv = dynamic_cast<TableVerifier*>(p.verifier.get());
        ReferencingVerifier* rv = dynamic_cast<ReferencingVerifier*>(p.verifier.get());

        if (rv) {
            std::vector<Documentation> documentations = DocEng.documentations();
            auto it = std::find_if(
                documentations.begin(),
                documentations.end(),
                [rv](const Documentation& doc) { return doc.id == rv->identifier; }
            );

            if (it == documentations.end()) {
                result << "\"reference\": { \"found\": false }";
            } else {
                result << "\"reference\": {"
                    << "\"found\": true,"
                    << "\"name\": \"" << it->name << "\","
                    << "\"identifier\": \"" << rv->identifier << "\""
                    << "}";
            }
        }
        else if (tv) {
            std::string json = generateJsonDocumentation({ "", "", tv->documentations });
            // We have a TableVerifier, so we need to recurse
            result << "\"restrictions\": " << json;
        }
        else {
            result << "\"description\": \"" << p.verifier->documentation() << "\"";
        }
        result << "}";
        if (&p != &d.entries.back()) {
            result << ", ";
        }

    }

    result << ']';
    result << "}";

    return result.str();
}

std::string DocumentationEngine::generateJson() const {
    std::stringstream json;
    json << "[";

    for (const Documentation& d : _documentations) {
        json << generateJsonDocumentation(d);
        if (&d != &_documentations.back()) {
            json << ", ";
        }
    }

    json << "]";

    return json.str();
}

void DocumentationEngine::addDocumentation(Documentation doc) {
    if (doc.id.empty()) {
        _documentations.push_back(std::move(doc));
    }
    else {
        auto it = std::find_if(
            _documentations.begin(),
            _documentations.end(),
            [doc](const Documentation& d) { return doc.id == d.id; }
        );

        if (it != _documentations.end()) {
            throw DuplicateDocumentationException(std::move(doc));
        }
        else {
            _documentations.push_back(std::move(doc));
        }
    }
}

void DocumentationEngine::addHandlebarTemplates(std::vector<HandlebarTemplate> templates) {
    _handlebarTemplates.insert(std::end(_handlebarTemplates), std::begin(templates), std::end(templates));
}
    
std::vector<Documentation> DocumentationEngine::documentations() const {
    return _documentations;
}
    
void DocumentationEngine::writeDocumentationHtml(const std::string path, const std::string data) {
    
    std::ifstream handlebarsInput;
    handlebarsInput.exceptions(~std::ofstream::goodbit);
    handlebarsInput.open(absPath(HandlebarsFilename));
    const std::string handlebarsContent = std::string(
                                                      std::istreambuf_iterator<char>(handlebarsInput),
                                                      std::istreambuf_iterator<char>()
                                                      );
    std::ifstream jsInput;
    jsInput.exceptions(~std::ofstream::goodbit);
    jsInput.open(absPath(JsFilename));
    const std::string jsContent = std::string(
                                              std::istreambuf_iterator<char>(jsInput),
                                              std::istreambuf_iterator<char>()
                                              );
    
    std::ifstream bootstrapInput;
    bootstrapInput.exceptions(~std::ofstream::goodbit);
    bootstrapInput.open(absPath(BootstrapFilename));
    const std::string bootstrapContent = std::string(
                                                     std::istreambuf_iterator<char>(bootstrapInput),
                                                     std::istreambuf_iterator<char>()
                                                     );
    
    std::ifstream cssInput;
    cssInput.exceptions(~std::ofstream::goodbit);
    cssInput.open(absPath(CssFilename));
    const std::string cssContent = std::string(
                                               std::istreambuf_iterator<char>(cssInput),
                                               std::istreambuf_iterator<char>()
                                               );
    
    std::string filename = path + ("index.html");
    std::ofstream file;
    file.exceptions(~std::ofstream::goodbit);
    file.open(filename);
    
    // We probably should escape backslashes here?
    file << "<!DOCTYPE html>"                                            << '\n'
    << "<html>"                                                          << '\n'
    << "\t"   << "<head>"                                                << '\n';
    
    //write handlebar templates to htmlpage as script elements (as per hb)
    for (const HandlebarTemplate& t : _handlebarTemplates) {
        const char* Type = "text/x-handlebars-template";
        file << "\t\t<script id=\"" << t.name << "\" type=\"" << Type << "\">";
        file << '\n';
        
        std::ifstream templateFilename(absPath(t.filename));
        std::string templateContent(
                                    std::istreambuf_iterator<char>{templateFilename},
                                    std::istreambuf_iterator<char>{}
                                    );
        file << templateContent << "\n\t\t</script>"                    << '\n';
    }
    
    //write main template
    file << "\t\t<script id=\"mainTemplate\" type=\"text/x-handlebars-template\">";
    file << '\n';
    std::ifstream templateFilename(absPath("${WEB}/documentation/main.hbs"));
    std::string templateContent(
                                std::istreambuf_iterator<char>{templateFilename},
                                std::istreambuf_iterator<char>{}
                                );
    file << templateContent << "\t\t</script>"                          << '\n';
    
    //write scripte to register templates dynamically
    file << "\t\t<script type=\"text/javascript\">"                     << '\n';
    file << "\t\t\ttemplates = [];"                                     << '\n';
    file << "\t\t\tregisterTemplates = function() {"                    << '\n';

    for (const HandlebarTemplate& t : _handlebarTemplates) {
        std::string nameOnly = t.name.substr(0,t.name.length() - 8); //-8 for Template
        file << "\t\t\t\tvar " << t.name;
        file << "Element = document.getElementById('" << t.name <<"');" << '\n';

        file << "\t\t\t\tHandlebars.registerPartial('" << nameOnly << "', ";
        file << t.name << "Element.innerHTML);" << '\n';

        file << "\t\t\t\ttemplates['"<< nameOnly << "'] = Handlebars.compile(";
        file << t.name << "Element.innerHTML);"                        << '\n';
    }
    file << "\t\t\t}"                                                   << '\n';
    file << "\t\t</script>"                                             << '\n';
    
    
    const std::string DataId = "data";
    
    const std::string Version =
    "[" +
    std::to_string(OPENSPACE_VERSION_MAJOR) + "," +
    std::to_string(OPENSPACE_VERSION_MINOR) + "," +
    std::to_string(OPENSPACE_VERSION_PATCH) +
    "]";
    
    file
    << "\t\t" << "<script id=\"" << DataId
    << "\" type=\"text/application/json\">" << '\n'
    << "\t\t\t" << data << '\n'
    << "\t\t" << "</script>" << '\n';
    
    
    file
    << "\t"   << "<script>"                                                  << '\n'
    << "\t\t" << jsContent                                                   << '\n'
    << "\t\t" << "var documentation = parseJson('" << DataId << "').documentation;"  << '\n'
    << "\t\t" << "var version = " << Version << ";"                          << '\n'
    << "\t\t" << "var currentDocumentation = documentation[0];"              << '\n'
    << "\t\t" << handlebarsContent                                           << '\n'
    << "\t"   << "</script>"                                                 << '\n'
    << "\t"   << "<style type=\"text/css\">"                                 << '\n'
    << "\t\t" << cssContent                                                  << '\n'
    << "\t\t" << bootstrapContent                                            << '\n'
    << "\t"   << "</style>"                                                  << '\n'
    << "\t\t" << "<title>OpenSpace Documentation</title>"                    << '\n'
    << "\t"   << "</head>"                                                   << '\n'
    << "\t"   << "<body>"                                                    << '\n'
    << "\t"   << "</body>"                                                   << '\n'
    << "</html>"                                                   << '\n';
}

} // namespace openspace::documentation
