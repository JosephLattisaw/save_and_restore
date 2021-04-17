import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';

import 'package:controller/widgets/lite_rolling_switch.dart';

class ConfigurationToggleButtons extends StatelessWidget {
  ConfigurationToggleButtons({Key? key}) : super(key: key) {
    for (final i in toggleButtonMap)
      assert(i.length == toggleButtonMap.first.length);
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: List<Widget>.generate(
        toggleButtonMap.first.length,
        (index) => Container(
          padding: _PADDING,
          child: LiteRollingSwitch(
            textOff: _toggleButtonTextOffNames[index],
            textOn: _toggleButtonTextOnNames[index],
            colorOff: _toggleButtonColorOff[index],
            colorOn: Colors.orange.shade600,
            iconOff: _toggleButtonIconDataOff[index],
            iconOn: _toggleButtonIconDataOn[index],
          ),
        ),
      ),
    );
  }

  static List<String> _toggleButtonTextOffNames = [
    "TCTS SIM",
    "ADU VDS",
    "NO ISIM",
  ];

  static List<String> _toggleButtonTextOnNames = [
    "SLE",
    "ADUSIM",
    "ISIM",
  ];

  static List<Color> _toggleButtonColorOff = [
    Colors.blue.shade900,
    Colors.blue.shade900,
    Colors.grey.shade800
  ];

  static List<IconData> _toggleButtonIconDataOff = [
    Icons.title,
    CupertinoIcons.rocket_fill,
    CupertinoIcons.xmark_circle_fill,
  ];

  static List<IconData> _toggleButtonIconDataOn = [
    Icons.public,
    Icons.brightness_auto,
    CupertinoIcons.star_lefthalf_fill,
  ];

  static List<List<dynamic>> toggleButtonMap = [
    _toggleButtonTextOffNames,
    _toggleButtonTextOnNames,
    _toggleButtonColorOff,
    _toggleButtonIconDataOff,
    _toggleButtonIconDataOn,
  ];

  static const EdgeInsetsGeometry _PADDING = const EdgeInsets.all(4.0);
}
