#include "Parser.h"

using namespace EpisodesParser;

int main(int argc, char *argv[]) {
    Parser * parser;
    parser = new Parser();
    parser->parse("/Users/wimleers/School/masterthesis/logs/driverpacks.net.episodes.log");
}


