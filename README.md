harfbuzz-icu-shaping
====================

I know this is apple vs oranges but hb_shape is very slow. It can be incorrect usage or implementation... feel free to correct it.

```bash
./bin/clang-darwin-4.2.1/release/harfbuzz-icu-test 1000

ICU shaping

881:907:915:910:904:929:911:3:918:929:908:910:911:3:912:916:920:

1449:1471:1478:1396:3:1477:1499:1401:1415:1486:1431:1478:1396:3:1476:1493:1435:1474:

4710:4524:8640:3212:12901:

 0.008791s wall, 0.010000s user + 0.000000s system = 0.010000s CPU (113.8%)

Harfbuzz shaping

881:907:915:910:904:929:911:3:918:929:908:910:911:3:912:916:920:

2254:2025:2157:384:3:2151:1958:1965:3888:4105:3873:3858:384:3:413:2276:2244:2117:

4710:4524:8640:3212:12901:

 0.501237s wall, 0.500000s user + 0.000000s system = 0.500000s CPU (99.8%)
 ```
