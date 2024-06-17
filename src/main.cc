#include <sourcemeta/jsontoolkit/jsonschema.h>

#include <algorithm>   // std::min
#include <cstdlib>     // EXIT_FAILURE, EXIT_SUCCESS
#include <filesystem>  // std::filesystem
#include <iostream>    // std::cerr, std::cout
#include <span>        // std::span
#include <string>      // std::string
#include <string_view> // std::string_view
#include <vector>      // std::vector

#include "command.h"
#include "configure.h"

constexpr std::string_view USAGE_DETAILS{R"EOF(
Global Options:

   --verbose, -v    Enable verbose output
   --resolve, -r    Import the given JSON Schema (or directory of schemas)
                    into the resolution context

Commands:

   validate <schema.json> <instance.json> [--http/-h]

       Validate an instance against the given schema.

   metaschema [schemas-or-directories...] [--http/-h]
              [--extension/-e <extension>]
              [--ignore/-i <schemas-or-directories>]

       Validate that a schema or a set of schemas are valid with respect
       to their metaschemas.

   test [schemas-or-directories...] [--http/-h] [--extension/-e <extension>]
        [--ignore/-i <schemas-or-directories>]

       Run a set of unit tests against a schema.

   fmt [schemas-or-directories...] [--check/-c] [--extension/-e <extension>]
       [--ignore/-i <schemas-or-directories>]

       Format the input schemas in-place or check they are formatted.

   lint [schemas-or-directories...] [--fix/-f] [--extension/-e <extension>]
       [--ignore/-i <schemas-or-directories>]

       Lint the input schemas and potentially fix the reported issues.

   bundle <schema.json> [--http/-h]

       Perform JSON Schema Bundling on a schema to inline remote references,
       printing the result to standard output.

   frame <schema.json>

       Frame a schema in-place, displaying schema locations and references
       in a human-readable manner.

For more documentation, visit https://github.com/Intelligence-AI/jsonschema
)EOF"};

auto jsonschema_main(const std::string &program, const std::string &command,
                     const std::span<const std::string> &arguments) -> int {
  if (command == "fmt") {
    return intelligence::jsonschema::cli::fmt(arguments);
  } else if (command == "frame") {
    return intelligence::jsonschema::cli::frame(arguments);
  } else if (command == "bundle") {
    return intelligence::jsonschema::cli::bundle(arguments);
  } else if (command == "lint") {
    return intelligence::jsonschema::cli::lint(arguments);
  } else if (command == "validate") {
    return intelligence::jsonschema::cli::validate(arguments);
  } else if (command == "metaschema") {
    return intelligence::jsonschema::cli::metaschema(arguments);
  } else if (command == "test") {
    return intelligence::jsonschema::cli::test(arguments);
  } else {
    std::cout << "JSON Schema CLI - v"
              << intelligence::jsonschema::cli::PROJECT_VERSION << "\n";
    std::cout << "Usage: " << std::filesystem::path{program}.filename().string()
              << " <command> [arguments...]\n";
    std::cout << USAGE_DETAILS;
    return EXIT_SUCCESS;
  }
}

auto main(int argc, char *argv[]) noexcept -> int {
  try {
    const std::string program{argv[0]};
    const std::string command{argc > 1 ? argv[1] : "help"};
    const std::vector<std::string> arguments{argv + std::min(2, argc),
                                             argv + argc};
    return jsonschema_main(program, command, arguments);
  } catch (const sourcemeta::jsontoolkit::SchemaReferenceError &error) {
    std::cerr << error.what() << ": " << error.id() << "\n";
    std::cerr << "    at schema location \"";
    sourcemeta::jsontoolkit::stringify(error.location(), std::cerr);
    std::cerr << "\"\n";
    return EXIT_FAILURE;
  } catch (const sourcemeta::jsontoolkit::SchemaResolutionError &error) {
    std::cerr << error.what() << ": " << error.id() << "\n";
    return EXIT_FAILURE;
  } catch (const sourcemeta::jsontoolkit::FileParseError &error) {
    std::cerr << error.path().string() << "\n  " << error.what() << " at line "
              << error.line() << " and column " << error.column() << "\n";
    return EXIT_FAILURE;
  } catch (const sourcemeta::jsontoolkit::ParseError &error) {
    std::cerr << error.what() << " at line " << error.line() << " and column "
              << error.column() << "\n";
    return EXIT_FAILURE;
  } catch (const std::exception &error) {
    std::cerr << "Error: " << error.what() << "\n";
    return EXIT_FAILURE;
  }
}
