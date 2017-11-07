// system headers
#include <iostream>
#include <cstring>

// library headers
#include <args.hxx>

// local headers
#include "zsclient.h"
#include "config.h"

using namespace std;

int main(const int argc, const char** argv) {
    args::ArgumentParser parser(
        "zsync2 -- the probably easiest efficient way to update files",
        "Brand new C++11 frontend to zsync."
    );

    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::ValueFlagList<string> seedFiles(parser, "path",
        "Use data from this file during update process. Can be specified more than once.",
        {'i', "seed-file"}
    );

    args::Flag httpInsecureMode(parser, "", "Switch to HTTP insecure mode.", {'I', "insecure"});

    args::Flag checkForChanges(parser, "",
        "Check for changes on the server. Exits with code 1 if the file changed on the server, otherwise 0.",
        {'j', "check-for-changes"}
    );

    args::ValueFlag<string> saveZSyncFilePath(parser, "path",
        "Save copy of .zsync file to given path.",
        {'k', "copy-zsync-file-to"}
    );

    args::Flag resolveRedirect(parser, "",
        "Resolve redirection for given URL and exit.",
        {'r', "resolve-redirection"}
    );

    args::ValueFlag<string> outputFilename(parser, "path",
        "Path to local file which should be created. If not given, file path in .zsync file will be used.",
        {'o', "output"}
    );

    args::Flag quietMode(parser, "", "Quiet mode", {'s', 'q', "silent-mode"});
    args::Flag verbose(parser, "", "Switch on verbose mode", {'v', "verbose"});
    args::Flag showVersion(parser, "", "Print version and exit", {'V', "version"});

    args::Positional<string> pathOrUrl(parser, "path or URL", "Path or URL to .zsync(2) file");

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help&) {
        cerr << parser;
        return 0;
    } catch (args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    // check whether any flag has been specified which wants the application to exit immediately
    if (showVersion) {
        cerr << "zsync2 version " << VERSION << endl;
        return 0;
    }

    if (verbose) {
        putenv(strdup("CURLOPT_VERBOSE=1"));
    }

    if (!pathOrUrl) {
        cerr << parser;
        return 2;
    }

    string outPath;

    if (outputFilename)
        outPath = outputFilename.Get();

    zsync2::ZSyncClient client(pathOrUrl.Get(), outPath);

    if (seedFiles) {
        for (const auto &seedFile : seedFiles.Get()) {
            client.addSeedFile(seedFile);
        }
    }

    if (checkForChanges) {
        cout << "Checking for changes..." << endl;

        bool changesAvailable;

        // return some non-0/1 error code in case the update check itself fails
        if (!client.checkForChanges(changesAvailable))
            return 3;

        if (changesAvailable) {
            cout << "File has changed on the server, update required!" << endl;
            return 1;
        } else {
            cout << "No changes detected, file is up to date!" << endl;
            return 0;
        }
    }

    if (!client.run())
        return 1;

    return 0;
}
