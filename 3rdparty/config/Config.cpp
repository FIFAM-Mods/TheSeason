#include "Config.h"

using namespace plugin;

bool config_helper::config_extract_one_value(std::string const &strinput, bool &value) {
    if (!strinput.empty()) {
        if (!strinput.compare("0") || !strinput.compare("false") || !strinput.compare("FALSE") || !strinput.compare("no") || !strinput.compare("NO"))
            value = false;
        else
            value = true;
        return true;
    }
    return false;
}

int config_helper::config_extract_values_array(std::string const &strinput, std::vector<bool> &arr) {
    std::istringstream iss(strinput);
    std::string strval;
    while (config_extract_one_value(iss, strval)) {
        bool bval;
        if (!config_extract_one_value(strval, bval))
            break;
        arr.push_back(bval);
    }
    return arr.size();
}

config_parameter::config_parameter() {
    _notInitialised = true;
    _quotes = true;
}

config_parameter::config_parameter(std::string value, bool quotes) {
    _value = value;
    _notInitialised = false;
    _quotes = quotes;
}

bool config_parameter::isEmpty() {
    return _notInitialised;
}

float config_parameter::asFloat(float defaultVal) {
    float fval;
    if (_notInitialised || !config_helper::config_extract_one_value(_value, fval))
        return defaultVal;
    return fval;
}

float config_parameter::asFloat() { return asFloat(0.0f); }

unsigned int config_parameter::asInt(int defaultVal) {
    int ival;
    if (_notInitialised || !config_helper::config_extract_one_value(_value, ival))
        return defaultVal;
    return ival;
}

unsigned int config_parameter::asInt() { return asInt(0); }

std::string config_parameter::asString(std::string defaultVal) {
    if (_notInitialised)
        return defaultVal;
    if (_quotes)
        return _value.substr(1, _value.size() - 2);
    return _value;
}

std::string config_parameter::asString() { return asString(std::string()); }

bool config_parameter::asBool(bool defaultVal) {
    bool bval;
    if (_notInitialised || !config_helper::config_extract_one_value(_value, bval))
        return defaultVal;
    return bval;
}

bool config_parameter::asBool() { return asBool(false); }

std::vector<int> config_parameter::asIntArray() {
    std::vector<int> result;
    config_helper::config_extract_values_array(_value, result);
    return result;
}

std::vector<float> config_parameter::asFloatArray() {
    std::vector<float> result;
    config_helper::config_extract_values_array(_value, result);
    return result;
}

std::vector<bool> config_parameter::asBoolArray() {
    std::vector<bool> result;
    config_helper::config_extract_values_array(_value, result);
    return result;
}

config_parameter &config_parameter::operator=(float n) {
    _value = std::to_string(n);
    _quotes = false;
    _notInitialised = false;
    return *this;
}

config_parameter &config_parameter::operator=(int n) {
    _value = std::to_string(n);
    _quotes = false;
    _notInitialised = false;
    return *this;
}

config_parameter &config_parameter::operator=(std::string s) {
    _value = s;
    _quotes = true;
    _notInitialised = false;
    return *this;
}

config_parameter &config_parameter::operator=(const char *s) {
    _value = '"';
    _value.append(s);
    _value.push_back('"');
    _quotes = true;
    _notInitialised = false;
    return *this;
}

config_parameter &config_parameter::operator=(bool v) {
    if (v)
        _value = "TRUE";
    else
        _value = "FALSE";
    _quotes = false;
    _notInitialised = false;
    return *this;
}

config_param_line::config_param_line::config_param_line(std::string paramName) {
    name = paramName;
    commentOffset = 0;
}

config_param_line::config_param_line(std::string paramName, std::string value, bool useQuotes) {
    _value = value;
    _notInitialised = false;
    _quotes = useQuotes;
    name = paramName;
    commentOffset = 0;
}

bool config_file::pathEmpty() {
#ifdef _MSC_VER
    if (_bWidePath)
        return _widePath.empty();
    else
#endif
        return _path.empty();
}

void config_file::prepareData() {
    if (_dataRead || pathEmpty())
        return;
    std::ifstream in;
#ifdef _MSC_VER
    if (_bWidePath)
        in.open(_widePath);
    else
#endif
        in.open(_path);
    if (in.is_open()) {
        unsigned int lineId = 0;
        for (std::string line; getline(in, line); ) {
            if (line.empty()) {
                emptyLines.push_back(lineId++);
                continue;
            }
            bool usesQuotes = false;
            std::string name;
            std::string value;
            std::string comment;
            unsigned int commentOffset = 0;

            bool scanBeforeName = true;
            bool scanName = false;
            bool scanAfterName = false;
            bool scanValue = false;
            bool scanAfterValue = false;
            bool isCommentLine = false;

            for (unsigned int i = 0; i < line.size(); i++) {
                if (scanBeforeName) {
                    if (line[i] == '#' || line[i] == ';') {
                        isCommentLine = true;
                        break;
                    }
                    if (line[i] == '=') {
                        // incorrect line
                        continue;
                    }
                    if (line[i] != ' ' && line[i] != '\t') {
                        name.push_back(line[i]);
                        scanBeforeName = false;
                        scanName = true;
                    }
                }
                else if (scanName) {
                    if (line[i] == '#' || line[i] == ';') {
                        commentOffset = i;
                        comment = line.substr(i);
                        break;
                    }
                    if (line[i] == ' ' || line[i] == '\t' || line[i] == '=') {
                        scanName = false;
                        scanAfterName = true;
                    }
                    else
                        name.push_back(line[i]);
                }
                else if (scanAfterName) {
                    if (line[i] == '#' || line[i] == ';') {
                        commentOffset = i;
                        comment = line.substr(i);
                        break;
                    }
                    if (line[i] == '=') {
                        _useEqualitySign = true;
                        continue;
                    }
                    else if (line[i] != ' ' && line[i] != '\t') {
                        scanAfterName = false;
                        scanValue = true;
                        value.push_back(line[i]);
                    }
                }
                else if (scanValue) {
                    if (line[i] == '#' || line[i] == ';') {
                        commentOffset = i;
                        comment = line.substr(i);
                        break;
                    }
                    value.push_back(line[i]);
                }
            }

            if (isCommentLine) {
                comments.push_back(std::make_pair(lineId, line));
                continue;
            }

            if (value.size() > 0) {
                size_t l = value.find_last_not_of(" \t");
                if (l != std::string::npos) {
                    value = value.substr(0, l + 1);
                    size_t valSize = value.size();
                    if (valSize > 1 && value[0] == '"' && value[valSize - 1] == '"')
                        usesQuotes = true;
                }
                else
                    value.clear();
            }

            paramLines.emplace_back(name, value, usesQuotes);
            config_param_line &param = paramLines.back();
            param.comment = comment;
            param.commentOffset = commentOffset;

            //printf("%s \"%s\"\n", param.name.c_str(), param._value.c_str());

            lineId++;
        }
    }
    _dataRead = true;
}

void config_file::writeData() {
    if (pathEmpty())
        return;
    std::ofstream out;
#ifdef _MSC_VER
    if (_bWidePath)
        out.open(_widePath);
    else
#endif
        out.open(_path);
    if (out.is_open()) {
        unsigned int maxStrLen = 0;
        for (config_param_line &param : paramLines) {
            if (!param.isEmpty() && !param.name.empty()) {
                unsigned int strSz = param.name.size();
                if (strSz > maxStrLen)
                    maxStrLen = strSz;
            }
        }
        unsigned int numCommentsLeft = comments.size();
        for (config_param_line &param : paramLines) {
            if (!param.isEmpty() && !param.name.empty()) {
                out << param.name;
                unsigned int numSpaces = maxStrLen - param.name.size() + 1;
                for (unsigned int i = 0; i < numSpaces; i++)
                    out << ' ';
                if (_useEqualitySign)
                    out << "= ";
                out << param._value;
                out << '\n';
            }
        }
    }
}

config_file::config_file() {
#ifdef _MSC_VER
    _bWidePath = false;
#endif
    _dataRead = false;
    _useAlignment = true;
    _useEqualitySign = false;
}

void config_file::open(std::string fileName) {
    _path = fileName;
#ifdef _MSC_VER
    _bWidePath = false;
#endif
    prepareData();
}

void config_file::open(std::string fileName, bool readOnly, bool equalitySign, bool alignment) {
    _path = fileName;
#ifdef _MSC_VER
    _bWidePath = false;
#endif
    prepareData();
}

config_file::config_file(std::string fileName) {
    _dataRead = false;
    _useAlignment = true;
    _useEqualitySign = false;
    open(fileName);
}

#ifdef _MSC_VER
void config_file::open(std::wstring fileName) {
    _widePath = fileName;
    _bWidePath = true;
    prepareData();
}

void config_file::open(std::wstring fileName, bool readOnly, bool equalitySign, bool alignment) {
    _widePath = fileName;
    _bWidePath = true;
    prepareData();
}

config_file::config_file(std::wstring fileName) {
    _dataRead = false;
    _useAlignment = true;
    _useEqualitySign = false;
    open(fileName);
}
#endif

void config_file::save() {
    if (_dataRead)
        writeData();
}

config_parameter &config_file::operator[](std::string name) {
    for (config_param_line &param : paramLines) {
        if (!param.name.compare(name))
            return param;
    }
    paramLines.emplace_back(name);
    return paramLines.back();
}

void config_file::setUseEqualitySign(bool enable) {
    _useEqualitySign = enable;
}

void config_file::setUseAlignment(bool enable) {
    _useAlignment = enable;
}
