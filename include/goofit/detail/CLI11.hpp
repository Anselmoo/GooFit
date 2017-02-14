#pragma once

// Distributed under the MIT license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

// This file was generated using MakeSingleHeader.py in CLI11/scripts
// from: v0.4-3-g2b40b8c

// This has the complete CLI library in one file.

#include <sys/stat.h>
#include <deque>
#include <set>
#include <iostream>
#include <string>
#include <tuple>
#include <locale>
#include <functional>
#include <numeric>
#include <iomanip>
#include <sys/types.h>
#include <exception>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <type_traits>
#include <memory>

// From CLI/Error.hpp

namespace CLI {

// Error definitions

/// All errors derive from this one
struct Error : public std::runtime_error {
    int exit_code;
    bool print_help;
    Error(std::string parent, std::string name, int exit_code=255, bool print_help=true) : runtime_error(parent + ": " + name), exit_code(exit_code), print_help(print_help) {}
};

// Construction errors (not in parsing)

struct ConstructionError : public Error {
    // Using Error::Error constructors seem to not work on GCC 4.7
    ConstructionError(std::string parent, std::string name, int exit_code=255, bool print_help=true) : Error(parent, name, exit_code, print_help) {}
};

/// Thrown when an option is set to conflicting values (non-vector and multi args, for example)
struct IncorrectConstruction : public ConstructionError {
    IncorrectConstruction(std::string name) : ConstructionError("IncorrectConstruction", name, 8) {}
};

/// Thrown on construction of a bad name
struct BadNameString : public ConstructionError {
    BadNameString(std::string name) : ConstructionError("BadNameString", name, 1) {}
};

/// Thrown when an option already exists
struct OptionAlreadyAdded : public ConstructionError {
    OptionAlreadyAdded(std::string name) : ConstructionError("OptionAlreadyAdded", name, 3) {}
};

// Parsing errors

/// Anything that can error in Parse
struct ParseError : public Error {
    ParseError(std::string parent, std::string name, int exit_code=255, bool print_help=true) : Error(parent, name, exit_code, print_help) {}
};

// Not really "errors"

/// This is a successful completion on parsing, supposed to exit
struct Success : public ParseError {
    Success() : ParseError("Success", "Successfully completed, should be caught and quit", 0, false) {}
};

/// -h or --help on command line
struct CallForHelp : public ParseError {
    CallForHelp() : ParseError("CallForHelp", "This should be caught in your main function, see examples", 0) {}
};


/// Thrown when parsing an INI file and it is missing
struct FileError : public ParseError {
    FileError (std::string name) : ParseError("FileError", name, 10) {}
};

/// Thrown when conversion call back fails, such as when an int fails to coerse to a string
struct ConversionError : public ParseError {
    ConversionError(std::string name) : ParseError("ConversionError", name, 2) {}
};

/// Thrown when validation of results fails
struct ValidationError : public ParseError {
    ValidationError(std::string name) : ParseError("ValidationError", name, 2) {}
};

/// Thrown when a required option is missing
struct RequiredError : public ParseError {
    RequiredError(std::string name) : ParseError("RequiredError", name, 5) {}
};

/// Thrown when a requires option is missing
struct RequiresError : public ParseError {
    RequiresError(std::string name, std::string subname) : ParseError("RequiresError", name + " requires " + subname, 13) {}
};

/// Thrown when a exludes option is present
struct ExcludesError : public ParseError {
    ExcludesError(std::string name, std::string subname) : ParseError("ExcludesError", name + " excludes " + subname, 14) {}
};

/// Thrown when too many positionals are found
struct PositionalError : public ParseError {
    PositionalError(std::string name) : ParseError("PositionalError", name, 6) {}
};

/// This is just a safety check to verify selection and parsing match
struct HorribleError : public ParseError {
    HorribleError(std::string name) : ParseError("HorribleError", "(You should never see this error) " + name, 7) {}
};

// After parsing

/// Thrown when counting a non-existent option
struct OptionNotFound : public Error {
    OptionNotFound(std::string name) : Error("OptionNotFound", name, 4) {}
};


}

// From CLI/TypeTools.hpp

namespace CLI {

// Type tools

// Copied from C++14
#if __cplusplus < 201402L
template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B,T>::type;
#else
// If your compiler supports C++14, you can use that definition instead
using std::enable_if_t;
#endif

template <typename T>
struct is_vector {
  static const bool value = false;
};


template<class T, class A>
struct is_vector<std::vector<T, A> > {
  static bool const value = true;
};

template <typename T>
struct is_bool {
  static const bool value = false;
};

template<>
struct is_bool<bool> {
  static bool const value = true;
};


namespace detail {
    // Based generally on https://rmf.io/cxx11/almost-static-if
    /// Simple empty scoped class
    enum class enabler {};

    /// An instance to use in EnableIf
    constexpr enabler dummy = {};


    // Type name print

    /// Was going to be based on
    ///  http://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
    /// But this is cleaner and works better in this case
    
    template<typename T,
    enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "INT";
	}

    template<typename T,
    enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "UINT";
	}
    
        
    template<typename T,
    enable_if_t<std::is_floating_point<T>::value, detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "FLOAT";
	}
    
    
    /// This one should not be used, since vector types print the internal type
    template<typename T,
    enable_if_t<is_vector<T>::value, detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "VECTOR";
	}


	template<typename T,
    enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value && !is_vector<T>::value
    , detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "STRING";
	}



    // Lexical cast


    /// Integers
    template<typename T, enable_if_t<std::is_integral<T>::value, detail::enabler> = detail::dummy>
    bool lexical_cast(std::string input, T& output) {
        try{
            output = (T) std::stoll(input);
            return true;
        } catch (std::invalid_argument) {
            return false;
        } catch (std::out_of_range) {
            return false;
        }
    }
        
    /// Floats
    template<typename T, enable_if_t<std::is_floating_point<T>::value, detail::enabler> = detail::dummy>
    bool lexical_cast(std::string input, T& output) {
        try{
            output = (T) std::stold(input);
            return true;
        } catch (std::invalid_argument) {
            return false;
        } catch (std::out_of_range) {
            return false;
        }
    }

    /// Vector
    template<typename T, 
    enable_if_t<is_vector<T>::value, detail::enabler> = detail::dummy>
    bool lexical_cast(std::string input, T& output) {
        if(output.size() == input.size())
            output.resize(input.size());
        for(size_t i=0; i<input.size(); i++)
            output[i] = input[i];
        return true;
    }

    /// String and similar
    template<typename T, 
    enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value && !is_vector<T>::value
    , detail::enabler> = detail::dummy>
    bool lexical_cast(std::string input, T& output) {
        output = input;
        return true;
    }


}
}

// From CLI/StringTools.hpp

namespace CLI {
namespace detail {


/// Simple function to join a string
template <typename T>
std::string join(const T& v, std::string delim = ",") {
    std::ostringstream s;
    size_t start = 0;
    for (const auto& i : v) {
        if(start++ > 0)
            s << delim;
        s << i;
    }
    return s.str();
}

// Based on http://stackoverflow.com/questions/25829143/c-trim-whitespace-from-a-string

/// Trim whitespace from left of string
std::string& ltrim(std::string &str) {
    auto it2 =  std::find_if( str.begin() , str.end() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( str.begin() , it2);
    return str;   
}

/// Trim whitespace from right of string
std::string& rtrim(std::string &str) {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;   
}

/// Trim whitespace from string
std::string& trim(std::string &str) {
    return ltrim(rtrim(str));
}

/// Make a copy of the string and then trim it
std::string trim_copy(const std::string &str) {
    std::string s = str;
    return ltrim(rtrim(s));
}

/// Print a two part "help" string
void format_help(std::stringstream &out, std::string name, std::string description, size_t wid) {
    name = "  " + name;
    out << std::setw(wid) << std::left << name;
    if(description != "") {
        if(name.length()>=wid)
            out << std::endl << std::setw(wid) << "";
        out << description << std::endl;
    }
}

/// Verify the first character of an option
template<typename T>
bool valid_first_char(T c) {
    return std::isalpha(c) || c=='_';
}

/// Verify following characters of an option
template<typename T>
bool valid_later_char(T c) {
    return std::isalnum(c) || c=='_' || c=='.' || c=='-';
}

/// Verify an option name
inline bool valid_name_string(const std::string &str) {
    if(str.size()<1 || !valid_first_char(str[0]))
        return false;
    for(auto c : str.substr(1))
        if(!valid_later_char(c))
            return false;
    return true;
}



}
}

// From CLI/Ini.hpp

namespace CLI {
namespace detail {


/// Internal parsing function
std::vector<std::string> parse_ini(std::istream &input) {
    std::string line;
    std::string section = "default";

    std::vector<std::string> output;

    while(getline(input, line)) {
        detail::trim(line);
        size_t len = line.length();
        if(len > 1 && line[0] == '[' && line[len-1] == ']') {
            section = line.substr(1,len-2);
            std::transform(std::begin(section), std::end(section), std::begin(section), ::tolower);
        } else if (len > 0) {
            if(section == "default")
                output.push_back("--" + line);
            else
                output.push_back("--" + section + "." + line);
        }
    }
    return output;
}

/// Parse an INI file, throw an error (ParseError:INIParseError or FileError) on failure
std::vector<std::string> parse_ini(const std::string &name) {

    std::ifstream input{name};
    if(!input.good())
        throw FileError(name);

    return parse_ini(input);
}


}
}

// From CLI/Split.hpp

namespace CLI {
namespace detail {

// Returns false if not a short option. Otherwise, sets opt name and rest and returns true
inline bool split_short(const std::string &current, std::string &name, std::string &rest) {
    if(current.size()>1 && current[0] == '-' && valid_first_char(current[1])) {
        name = current.substr(1,1);
        rest = current.substr(2);
        return true;
    } else
        return false;
}

// Returns false if not a long option. Otherwise, sets opt name and other side of = and returns true
inline bool split_long(const std::string &current, std::string &name, std::string &value) {
    if(current.size()>2 && current.substr(0,2) == "--" && valid_first_char(current[2])) {
        auto loc = current.find("=");
        if(loc != std::string::npos) {
            name = current.substr(2,loc-2);
            value = current.substr(loc+1);
        } else {
            name = current.substr(2);
            value = "";
        }
        return true;
    } else
        return false;
}

// Splits a string into multiple long and short names
inline std::vector<std::string> split_names(std::string current) {
    std::vector<std::string> output;
    size_t val;
    while((val = current.find(",")) != std::string::npos) {
        output.push_back(current.substr(0,val));
        current = current.substr(val+1);
    }
    output.push_back(current);
    return output;

}

/// Get a vector of short names, one of long names, and a single name
inline std::tuple<std::vector<std::string>,std::vector<std::string>, std::string>
  get_names(const std::vector<std::string> &input) {
    
    std::vector<std::string> short_names;
    std::vector<std::string> long_names;
    std::string pos_name;

    for(std::string name : input) {
        if(name.length() == 0)
            continue;
        else if(name.length() > 1 && name[0] == '-' && name[1] != '-') {
            if(name.length()==2 && valid_first_char(name[1]))
                short_names.push_back(std::string(1,name[1]));
            else
                throw BadNameString("Invalid one char name: "+name);
        } else if(name.length() > 2 && name.substr(0,2) == "--") {
            name = name.substr(2);
            if(valid_name_string(name))
                long_names.push_back(name);
            else
                throw BadNameString("Bad long name: "+name);
        } else if(name == "-" || name == "--") {
            throw BadNameString("Must have a name, not just dashes");
        } else {
            if(pos_name.length() > 0)
                throw BadNameString("Only one positional name allowed, remove: "+name);
            pos_name = name;

        }
    }
      
    return std::tuple<std::vector<std::string>,std::vector<std::string>, std::string>
        (short_names, long_names, pos_name);
}


}
}

// From CLI/Validators.hpp

namespace CLI {


/// Check for an existing file
bool ExistingFile(std::string filename) {
    struct stat buffer;   
    bool exist = stat(filename.c_str(), &buffer) == 0; 
    bool is_dir = buffer.st_mode & S_IFDIR;
    if(!exist) {
        std::cerr << "File does not exist: " << filename << std::endl;
        return false;
    } else if (is_dir) {
        std::cerr << "File is actually a directory: " << filename << std::endl;
        return false;
    } else {
        return true;
    }
}

/// Check for an existing directory
bool ExistingDirectory(std::string filename) {
    struct stat buffer;   
    bool exist = stat(filename.c_str(), &buffer) == 0; 
    bool is_dir = buffer.st_mode & S_IFDIR;
    if(!exist) {
        std::cerr << "Directory does not exist: " << filename << std::endl;
        return false;
    } else if (is_dir) {
        return true;
    } else {
        std::cerr << "Directory is actually a file: " << filename << std::endl;
        return true;
    }
}


/// Check for a non-existing path
bool NonexistentPath(std::string filename) {
    struct stat buffer;   
    bool exist = stat(filename.c_str(), &buffer) == 0; 
    if(!exist) {
        return true;
    } else {
        std::cerr << "Path exists: " << filename << std::endl;
        return false;
    }
}

/// Produce a range validator function
template<typename T>
std::function<bool(std::string)> Range(T min, T max) {
    return [min, max](std::string input){
        T val;
        detail::lexical_cast(input, val);
        return val >= min && val <= max;
    };
}

/// Range of one value is 0 to value
template<typename T>
std::function<bool(std::string)> Range(T max) {
    return Range((T) 0, max);
}

}

// From CLI/Option.hpp

namespace CLI {

typedef std::vector<std::vector<std::string>> results_t;
typedef std::function<bool(results_t)> callback_t;

class App;

class Option {
    friend App;
protected:
    // Config
    std::vector<std::string> snames;
    std::vector<std::string> lnames;
    std::string pname;

    std::string description;
    callback_t callback;

    // These are for help strings
    std::string defaultval;
    std::string typeval;
    std::string _group {"Options"};

    bool _default {false};
    bool _required {false};
    int _expected {1};
    bool allow_vector {false};
    std::vector<std::function<bool(std::string)>> _validators;

    std::set<Option*> _requires;
    std::set<Option*> _excludes;
    std::string _envname;

    // Results
    results_t results;


public:
    Option(std::string name, std::string description = "", std::function<bool(results_t)> callback=[](results_t){return true;}, bool _default=true) :
      description(description), callback(callback), _default(_default) {
        std::tie(snames, lnames, pname) = detail::get_names(detail::split_names(name));
    }


    // This class is "true" if optio passed.
    operator bool() const {
        return results.size() > 0;
    }

    /// Clear the parsed results (mostly for testing)
    void clear() {
        results.clear();
    }

    /// Set the option as required
    Option* required(bool value = true) {
        _required = value;
        return this;
    }

    /// Support Plubmum term
    Option* mandatory(bool value = true) {
        return required(value);
    }

    /// True if this is a required option
    bool get_required() const {
        return _required;
    }

    /// Set the number of expected arguments (Flags bypass this)
    Option* expected(int value) {
        if(value == 0)
            throw IncorrectConstruction("Cannot set 0 expected, use a flag instead");
        if(!allow_vector && value != 1)
            throw IncorrectConstruction("You can only change the Expected arguments for vectors");
        _expected = value;
        return this;
    }

    /// The number of arguments the option expects
    int get_expected() const {
        return _expected;
    }

    /// True if this has a default value
    int get_default() const {
        return _default;
    }

    /// True if the argument can be given directly
    bool get_positional() const {
        return pname.length() > 0;
    }

    /// True if option has at least one non-positional name
    bool nonpositional() const {
        return (snames.size() + lnames.size()) > 0;
    }

    /// True if option has description
    bool has_description() const {
        return description.length() > 0;
    }

    /// Adds a validator
    Option* check(std::function<bool(std::string)> validator) {

        _validators.push_back(validator);
        return this;
    }

    /// Changes the group membership
    Option* group(std::string name) {
        _group = name;
        return this;
    }

    /// Get the group of this option
    const std::string& get_group() const {
        return _group;
    }

    /// Get the description
    const std::string& get_description() const {
        return description;
    }

    /// Sets required options
    Option* requires(Option* opt) {
        auto tup = _requires.insert(opt);
        if(!tup.second)
            throw OptionAlreadyAdded(get_name() + " requires " + opt->get_name());
        return this;
    }

    /// Any number supported
    template<typename... ARG>
    Option* requires(Option* opt, Option* opt1, ARG... args) {
        requires(opt);
        return requires(opt1, args...);
    }

    /// Sets excluded options
    Option* excludes(Option* opt) {
        auto tup = _excludes.insert(opt);
        if(!tup.second)
            throw OptionAlreadyAdded(get_name() + " excludes " + opt->get_name());
        return this;
    }

    /// Any number supported
    template<typename... ARG>
    Option* excludes(Option* opt, Option* opt1, ARG... args) {
        excludes(opt);
        return excludes(opt1, args...);
    }

    /// Sets environment variable to read if no option given
    Option* envname(std::string name) {
        _envname = name;
        return this;
    }

    /// The name and any extras needed for positionals
    std::string help_positional() const {
        std::string out = pname;
        if(get_expected()<1)
            out = out + "x" + std::to_string(get_expected());
        else if(get_expected()==-1)
            out = out + "...";
        out = get_required() ? out : "["+out+"]";
        return out;
    }

    // Just the pname
    std::string get_pname() const {
        return pname;
    }


    /// Process the callback
    void run_callback() const {
        if(!callback(results))
            throw ConversionError(get_name() + "=" + detail::join(flatten_results()));
        if(_validators.size()>0) {
            for(const std::string & result : flatten_results())
                for(const std::function<bool(std::string)> &vali : _validators)
                    if(!vali(result))
                        throw ValidationError(get_name() + "=" + result);
        }
    }

    /// If options share any of the same names, they are equal (not counting positional)
    bool operator== (const Option& other) const {
        for(const std::string &sname : snames)
            for(const std::string &othersname : other.snames)
                if(sname == othersname)
                    return true;
        for(const std::string &lname : lnames)
            for(const std::string &otherlname : other.lnames)
                if(lname == otherlname)
                    return true;
        return false;
    }

    /// Gets a , sep list of names. Does not include the positional name if opt_only=true.
    std::string get_name(bool opt_only=false) const {
        std::vector<std::string> name_list;
        if(!opt_only && pname.length() > 0)
            name_list.push_back(pname);
        for(const std::string& sname : snames)
            name_list.push_back("-"+sname);
        for(const std::string& lname : lnames)
            name_list.push_back("--"+lname);
        return detail::join(name_list);
    }

    /// Check a name. Requires "-" or "--" for short / long, supports positional name
    bool check_name(std::string name) const {

        if(name.length()>2 && name.substr(0,2) == "--")
            return check_lname(name.substr(2));
        else if (name.length()>1 && name.substr(0,1) == "-")
            return check_sname(name.substr(1));
        else
            return name == pname;
    }

    /// Requires "-" to be removed from string
    bool check_sname(const std::string& name) const {
        return std::find(std::begin(snames), std::end(snames), name) != std::end(snames);
    }

    /// Requires "--" to be removed from string
    bool check_lname(const std::string& name) const {
        return std::find(std::begin(lnames), std::end(lnames), name) != std::end(lnames);
    }


    /// Puts a result at position r
    void add_result(int r, std::string s) {
        results.at(r).push_back(s);
    }

    /// Starts a new results vector (used for r in add_result)
    int get_new() {
        results.emplace_back();
        return results.size() - 1;
    }

    /// Count the total number of times an option was passed
    int count() const {
        int out = 0;
        for(const std::vector<std::string>& vec : results)
            out += vec.size();
        return out;
    }

    /// The first half of the help print, name plus default, etc
    std::string help_name() const {
        std::stringstream out;
        out << get_name(true) << _help_aftername();
        return out.str();
    }
    
    /// pname with type info
    std::string help_pname() const {
        std::stringstream out;
        out << get_pname() << _help_aftername();
        return out.str();
    }

    /// This is the part after the name is printed but before the description
    std::string _help_aftername() const {
        std::stringstream out;

        if(get_expected() != 0) {
            if(typeval != "")
                out << " " << typeval;
            if(defaultval != "")
                out << "=" << defaultval; 
            if(get_expected() > 1)
                out << " x " << get_expected();
            if(get_expected() == -1)
                out << " ...";
        }
        if(_envname != "")
            out << " (env:" << _envname << ")";
        if(_requires.size() > 0) {
            out << " Requires:";
            for(const Option* opt : _requires)
                out << " " << opt->get_name();
        }
        if(_excludes.size() > 0) {
            out << " Excludes:";
            for(const Option* opt : _excludes)
                out << " " << opt->get_name();
        }
        return out.str();

    }

    /// Produce a flattened vector of results, vs. a vector of vectors.
    std::vector<std::string> flatten_results() const {
        std::vector<std::string> output;
        for(const std::vector<std::string> result : results)
            output.insert(std::end(output), std::begin(result), std::end(result));
        return output;
    }

};



}

// From CLI/App.hpp

namespace CLI {

enum class Classifer {NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND};

class App;

typedef std::unique_ptr<Option> Option_p;
typedef std::unique_ptr<App> App_p;

/// Creates a command line program, with very few defaults.
/** To use, create a new Program() instance with argc, argv, and a help description. The templated
*  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
* program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class App {
protected:
    
    std::string name;
    std::string prog_description;
    std::vector<Option_p> options;
    std::vector<std::string> missing_options;
    std::deque<std::string> positionals;
    std::vector<App_p> subcommands;
    bool parsed {false};
    App* subcommand {nullptr};
    bool required_subcommand = false;
    std::string progname {"program"};
    Option* help_flag {nullptr};

    std::function<void()> app_callback;

    std::string ini_file;
    bool ini_required {false};
    Option* ini_setting {nullptr};
   

public:
    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    App(std::string prog_description="", bool help=true)
        : prog_description(prog_description) {

        if(help)
            help_flag = add_flag("-h,--help", "Print this help message and exit");

    }


    /// Set a callback for the end of parsing. Due to a bug in c++11,
    /// it is not possible to overload on std::function (fixed in c++14
    /// and backported to c++11 on newer compilers). Use capture by reference
    /// to get a pointer to App if needed.
    void set_callback(std::function<void()> callback) {
        app_callback = callback;
    }

    /// Reset the parsed data
    void reset() {

        parsed = false;
        subcommand = nullptr;

        for(const Option_p &opt : options) {
            opt->clear();
        }
        for(const App_p &app : subcommands) {
            app->reset();
        }
    }

    /// Get a pointer to the help flag.
    Option* get_help_ptr() {
        return help_flag;
    }

    /// Get a pointer to the config option.
    Option* get_config_ptr() {
        return ini_setting;
    }

    /// Produce a string that could be read in as a config of the current values of the App
    std::string config_to_str() const {
        std::stringstream out;
        for(const Option_p &opt : options) {
            if(opt->lnames.size() > 0 && opt->count() > 0 && opt->get_expected() > 0)
                out << opt->lnames[0] << "=" << detail::join(opt->flatten_results()) << std::endl;
        }
        return out.str();
    }
    
    /// Add a subcommand. Like the constructor, you can override the help message addition by setting help=false
    App* add_subcommand(std::string name, std::string description="", bool help=true) {
        subcommands.emplace_back(new App(description, help));
        subcommands.back()->name = name;
        return subcommands.back().get();
    }

    /// Add an option, will automatically understand the type for common types.
    /** To use, create a variable with the expected type, and pass it in after the name.
     * After start is called, you can use count to see if the value was passed, and
     * the value will be initialized properly. 
     *
     * ->required(), ->default, and the validators are options, 
     * The positional options take an optional number of arguments.
     *
     * For example,
     *
     *     std::string filename
     *     program.add_option("filename", filename, "description of filename");
     */
    Option* add_option(
            std::string name,
            callback_t callback,
            std::string description="", 
            bool defaulted=false
            ) {
        Option myopt{name, description, callback, defaulted};
        if(std::find_if(std::begin(options), std::end(options),
                    [&myopt](const Option_p &v){return *v == myopt;}) == std::end(options)) {
            options.emplace_back();
            Option_p& option = options.back();
            option.reset(new Option(name, description, callback, defaulted));
            return option.get();
        } else
            throw OptionAlreadyAdded(myopt.get_name());

    }

    /// Add option for string
    template<typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option* add_option(
            std::string name,
            T &variable,                ///< The variable to set
            std::string description="",
            bool defaulted=false
            ) {

        
        CLI::callback_t fun = [&variable](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            return detail::lexical_cast(res[0][0], variable);
        };

        Option* retval = add_option(name, fun, description, defaulted);
        retval->typeval = detail::type_name<T>();
        if(defaulted) {
            std::stringstream out;
            out << variable;
            retval->defaultval = out.str();
        }
        return retval;
    }

    /// Add option for vector of results
    template<typename T>
    Option* add_option(
            std::string name,
            std::vector<T> &variable,   ///< The variable vector to set
            std::string description="",
            bool defaulted=false
            ) {

        CLI::callback_t fun = [&variable](CLI::results_t res){
            bool retval = true;
            variable.clear();
            for(const auto &a : res)
                for(const auto &b : a) {
                    variable.emplace_back();
                    retval &= detail::lexical_cast(b, variable.back());
                }
            return variable.size() > 0 && retval;
        };

        Option* retval =  add_option(name, fun, description, defaulted);
        retval->allow_vector = true;
        retval->_expected = -1;
        retval->typeval = detail::type_name<T>();
        if(defaulted)
            retval->defaultval =  "[" + detail::join(variable) + "]";
        return retval;
    }


    /// Add option for flag
    Option* add_flag(
            std::string name,
            std::string description=""
            ) {
        CLI::callback_t fun = [](CLI::results_t){
            return true;
        };
        
        Option* opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->_expected = 0;
        return opt;
    }

    /// Add option for flag
    template<typename T,
        enable_if_t<std::is_integral<T>::value && !is_bool<T>::value, detail::enabler> = detail::dummy>
    Option* add_flag(
            std::string name,
            T &count,                   ///< A varaible holding the count
            std::string description=""
            ) {

        count = 0;
        CLI::callback_t fun = [&count](CLI::results_t res){
            count = (T) res.size();
            return true;
        };
        
        Option* opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->_expected = 0;
        return opt;
    }

    /// Bool version only allows the flag once
    template<typename T,
        enable_if_t<is_bool<T>::value, detail::enabler> = detail::dummy>
    Option* add_flag(
            std::string name,
            T &count,                   ///< A varaible holding true if passed
            std::string description=""
            ) {

        count = false;
        CLI::callback_t fun = [&count](CLI::results_t res){
            count = true;
            return res.size() == 1;
        };
        
        Option* opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->_expected = 0;
        return opt;
    }


    /// Add set of options
    template<typename T>
    Option* add_set(
            std::string name,
            T &member,                     ///< The selected member of the set
            std::set<T> _options,           ///< The set of posibilities
            std::string description="",
            bool defaulted=false
            ) {

        CLI::callback_t fun = [&member, _options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            bool retval = detail::lexical_cast(res[0][0], member);
            if(!retval)
                return false;
            return std::find(std::begin(_options), std::end(_options), member) != std::end(_options);
        };

        Option* retval = add_option(name, fun, description, defaulted);
        retval->typeval = detail::type_name<T>();
        retval->typeval += " in {" + detail::join(_options) + "}";
        if(defaulted) {
            std::stringstream out;
            out << member;
            retval->defaultval = out.str();
        }
        return retval;
    }


    /// Add a configuration ini file option
    Option* add_config(std::string name="--config",
                 std::string default_filename="",
                 std::string help="Read an ini file",
                 bool required=false) {

        // Remove existing config if present
        if(ini_setting != nullptr)
            remove_option(ini_setting);
        ini_file = default_filename;
        ini_required = required;
        ini_setting = add_option(name, ini_file, help, default_filename!="");
        return ini_setting;
    }

    /// Removes an option from the App. Takes an option pointer. Returns true if found and removed.
    bool remove_option(Option* opt) {
        auto iterator = std::find_if(std::begin(options), std::end(options),
                [opt](const Option_p &v){return v.get() == opt;});
        if (iterator != std::end(options)) {
            options.erase(iterator);
            return true;
        }
        return false;
    }

    /// This allows subclasses to inject code before callbacks but after parse
    /// This does not run if any errors or help is thrown.
    virtual void pre_callback() {}

    /// Parses the command line - throws errors
    /// This must be called after the options are in but before the rest of the program.
    void parse(int argc, char **argv) {
        progname = argv[0];
        std::vector<std::string> args;
        for(int i=argc-1; i>0; i--)
            args.push_back(argv[i]);
        parse(args);
    }

    /// The real work is done here. Expects a reversed vector
    void parse(std::vector<std::string> &args) {
        return _parse(args);
    }


    /// Print a nice error message and return the exit code
    int exit(const Error& e) const {
        if(e.exit_code != 0) {
            std::cerr << "ERROR: ";
            std::cerr << e.what() << std::endl;
            if(e.print_help)
                std::cerr << help();
        } else {
            if(e.print_help)
                std::cout << help();
        }
        return e.exit_code;
    }

    /// Counts the number of times the given option was passed.
    int count(std::string name) const {
        for(const Option_p &opt : options) {
            if(opt->check_name(name)) {
                return opt->count();
            }
        }
        throw OptionNotFound(name);
    }

    /// Makes a help message, with a column `wid` for column 1
    std::string help(size_t wid=30, std::string prev="") const {
        // Delegate to subcommand if needed
        if(prev == "")
            prev = progname;
        else
            prev += " " + name;

        if(subcommand != nullptr)
            return subcommand->help(wid, prev);

        std::stringstream out;
        out << prog_description << std::endl;
        out << "Usage: " << prev;
        
        // Check for options
        bool npos = false;
        std::set<std::string> groups;
        for(const Option_p &opt : options) {
            if(opt->nonpositional()) {
                npos = true;
                groups.insert(opt->get_group());
            }
        }

        if(npos)
            out << " [OPTIONS]";

        // Positionals
        bool pos=false;
        for(const Option_p &opt : options)
            if(opt->get_positional()) {
                out << " " << opt->help_positional();
                if(opt->has_description())
                    pos=true;
            }

        if(subcommands.size() > 0) {
            if(required_subcommand)
                out << " SUBCOMMAND";
            else
                out << " [SUBCOMMAND]";
        }

        out << std::endl << std::endl;

        // Positional descriptions
        if(pos) {
            out << "Positionals:" << std::endl;
            for(const Option_p &opt : options)
                if(opt->get_positional() && opt->has_description())
                    detail::format_help(out, opt->help_pname(), opt->get_description(), wid);
            out << std::endl;

        }


        // Options
        if(npos) {
            for (const std::string& group : groups) {
                out << group << ":" << std::endl;
                for(const Option_p &opt : options) {
                    if(opt->nonpositional() && opt->get_group() == group)
                        detail::format_help(out, opt->help_name(), opt->get_description(), wid);
                    
                }
                out << std::endl;
            }
        }

        // Subcommands
        if(subcommands.size()> 0) {
            out << "Subcommands:" << std::endl;
            for(const App_p &com : subcommands)
                detail::format_help(out, com->get_name(), com->prog_description, wid);
        }
        return out.str();
    }
    
    /// Get a subcommand pointer to the currently selected subcommand (after parsing)
    App* get_subcommand() {
        return subcommand;
    }
    
    /// Get the name of the current app
    std::string get_name() const {
        return name;
    }

    /// Require a subcommand to be given (does not affect help call)
    void require_subcommand(bool value = true) {
        required_subcommand = value;
    }

protected:

    /// Internal function to run (App) callback
    void run_callback() {
        if(app_callback)
            app_callback();
    }

    /// Selects a Classifer enum based on the type of the current argument
    Classifer _recognize(std::string current) const {
        std::string dummy1, dummy2;

        if(current == "--")
            return Classifer::POSITIONAL_MARK;
        for(const App_p &com : subcommands) {
            if(com->name == current)
                return Classifer::SUBCOMMAND;
        }
        if(detail::split_long(current, dummy1, dummy2))
            return Classifer::LONG;
        if(detail::split_short(current, dummy1, dummy2))
            return Classifer::SHORT;
        return Classifer::NONE;
    }


    /// Internal parse function
    void _parse(std::vector<std::string> &args) {
        parsed = true;

        bool positional_only = false;
        
        while(args.size()>0) {


            Classifer classifer = positional_only ? Classifer::NONE : _recognize(args.back());
            switch(classifer) {
            case Classifer::POSITIONAL_MARK:
                args.pop_back();
                positional_only = true;
                break;
            case Classifer::SUBCOMMAND:
                _parse_subcommand(args);
                break;
            case Classifer::LONG:
                _parse_long(args);
                break;
            case Classifer::SHORT:
                _parse_short(args);
                break;
            case Classifer::NONE:
                positionals.push_back(args.back());
                args.pop_back();
            }
        }

        if (help_flag != nullptr && help_flag->count() > 0) {
            throw CallForHelp();
        }


        // Collect positionals
        for(const Option_p& opt : options) {
            while (opt->get_positional() && opt->count() < opt->get_expected() && positionals.size() > 0) {
                opt->get_new();
                opt->add_result(0, positionals.front());
                positionals.pop_front();
            }
        }

        // Process an INI file
        if (ini_setting != nullptr && ini_file != "") {
            try {
                std::vector<std::string> values = detail::parse_ini(ini_file);
                while(values.size() > 0) {
                    _parse_long(values, false);
                }
                
            } catch (const FileError &e) {
                if(ini_required)
                    throw;
            }
        }

        
        // Get envname options if not yet passed
        for(const Option_p& opt : options) {
            if (opt->count() == 0 && opt->_envname != "") {
                char *ename = std::getenv(opt->_envname.c_str());
                if(ename != nullptr) {
                    opt->get_new();
                    opt->add_result(0, std::string(ename));
                }
            }
        }

        // Process callbacks
        for(const Option_p& opt : options) {
            if (opt->count() > 0) {
                opt->run_callback();
            }
        }

        // Verify required options 
        for(const Option_p& opt : options) {
            // Required
            if (opt->get_required()
                    && (opt->count() < opt->get_expected() || opt->count() == 0))
                throw RequiredError(opt->get_name());
            // Requires
            for (const Option* opt_req : opt->_requires)
                if (opt->count() > 0 && opt_req->count() == 0)
                    throw RequiresError(opt->get_name(), opt_req->get_name());
            // Excludes
            for (const Option* opt_ex : opt->_excludes)
                if (opt->count() > 0 && opt_ex->count() != 0)
                    throw ExcludesError(opt->get_name(), opt_ex->get_name());
        }

        if(required_subcommand && subcommand == nullptr)
            throw RequiredError("Subcommand required");

        if(positionals.size()>0)
            throw PositionalError("[" + detail::join(positionals) + "]");

        pre_callback();
        run_callback();
    }


    void _parse_subcommand(std::vector<std::string> &args) {
        for(const App_p &com : subcommands) {
            if(com->name == args.back()){ 
                args.pop_back();
                subcommand = com.get();
                com->parse(args);
                return;
            }
        }
        throw HorribleError("Subcommand");
    }
 
    /// Parse a short argument, must be at the top of the list
    void _parse_short(std::vector<std::string> &args) {
        std::string current = args.back();

        std::string name;
        std::string rest;
        if(!detail::split_short(current, name, rest))
            throw HorribleError("Short");
        args.pop_back();

        auto op_ptr = std::find_if(std::begin(options), std::end(options), [name](const Option_p &opt){return opt->check_sname(name);});

        if(op_ptr == std::end(options)) {
            missing_options.push_back("-" + name);
            return;
        }

        // Get a reference to the pointer to make syntax bearable
        Option_p& op = *op_ptr;

        int vnum = op->get_new();
        int num = op->get_expected();
       
        if(num == 0)
            op->add_result(vnum, "");
        else if(rest!="") {
            if (num > 0)
                num--;
            op->add_result(vnum, rest);
            rest = "";
        }


        if(num == -1) {
            while(args.size()>0 && _recognize(args.back()) == Classifer::NONE) {
                op->add_result(vnum, args.back());
                args.pop_back();
            }
        } else while(num>0 && args.size() > 0) {
            num--;
            std::string current = args.back();
            args.pop_back();
            op->add_result(vnum,current);
        }

        if(rest != "") {
            rest = "-" + rest;
            args.push_back(rest);
        }
    }

    /// Parse a long argument, must be at the top of the list
    void _parse_long(std::vector<std::string> &args, bool overwrite=true) {
        std::string current = args.back();

        std::string name;
        std::string value;
        if(!detail::split_long(current, name, value))
            throw HorribleError("Long");
        args.pop_back();

        auto op_ptr = std::find_if(std::begin(options), std::end(options), [name](const Option_p &v){return v->check_lname(name);});

        if(op_ptr == std::end(options)) {
            missing_options.push_back("--" + name);
            return;
        }

        // Get a reference to the pointer to make syntax bearable
        Option_p& op = *op_ptr;


        // Stop if not overwriting options (for ini parse)
        if(!overwrite && op->count() > 0)
            return;

        int vnum = op->get_new();
        int num = op->get_expected();
        

        if(value != "") {
            if(num!=-1) num--;
            op->add_result(vnum, value);
        } else if (num == 0) {
            op->add_result(vnum, "");
        }

        if(num == -1) {
            while(args.size() > 0 && _recognize(args.back()) == Classifer::NONE) {
                op->add_result(vnum, args.back());
                args.pop_back();
            }
        } else while(num>0 && args.size()>0) {
            num--;
            op->add_result(vnum,args.back());
            args.pop_back();
        }
        return;
    }

};


}
