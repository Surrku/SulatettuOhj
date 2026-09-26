#ifndef TIMEPARSER_H
#define TIMEPARSER_H

// Error codes
#define TIME_LEN_ERROR      -1  //aikamerkkijonossa väärä pituus
#define TIME_ARRAY_ERROR    -2  //merkkijono/osoitin virheellinen
#define TIME_VALUE_ERROR    -3  //virheellinen aika-arvo

using namespace std;

int time_parse(char *time);

#endif
