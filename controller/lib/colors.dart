import 'package:flutter/material.dart';

Map<int, Color> primarySwatchColorCodes = {
  50: Color.fromRGBO(0, 59, 112, .1),
  100: Color.fromRGBO(0, 59, 112, .2),
  200: Color.fromRGBO(0, 59, 112, .3),
  300: Color.fromRGBO(0, 59, 112, .4),
  400: Color.fromRGBO(0, 59, 112, .5),
  500: Color.fromRGBO(0, 59, 112, .6),
  600: Color.fromRGBO(0, 59, 112, .7),
  700: Color.fromRGBO(0, 59, 112, .8),
  800: Color.fromRGBO(0, 59, 112, .9),
  900: Color.fromRGBO(0, 59, 112, 1),
};

Map<int, Color> scaffoldBackgroundColorCodes = {
  50: Color.fromRGBO(0, 27, 51, .1),
  100: Color.fromRGBO(0, 27, 51, .2),
  200: Color.fromRGBO(0, 27, 51, .3),
  300: Color.fromRGBO(0, 27, 51, .4),
  400: Color.fromRGBO(0, 27, 51, .5),
  500: Color.fromRGBO(0, 27, 51, .6),
  600: Color.fromRGBO(0, 27, 51, .7),
  700: Color.fromRGBO(0, 27, 51, .8),
  800: Color.fromRGBO(0, 27, 51, .9),
  900: Color.fromRGBO(0, 27, 51, 1),
};

MaterialColor primarySwatch =
    MaterialColor(0xFF003b70, primarySwatchColorCodes);
MaterialColor scaffoldBackgroundColor =
    MaterialColor(0xFF001b33, scaffoldBackgroundColorCodes);
