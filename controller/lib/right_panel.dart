import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';

import 'package:controller/minimal_expanding_spacer.dart';
import 'package:controller/controller_floating_action_button.dart';
import 'package:controller/common/colors.dart';

class RightPanel extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(image: _backgroundDecorationImage),
      child: Padding(
        padding: _DEFAULT_PADDING,
        child: Column(
          children: [
            MinimalExpandingSpacer(),
            ControllerFloatingActionButton(
              onPressed: () => null,
              iconData: _TAKE_CONTROL_ICON,
              label: _TAKE_CONTROL_LABEL,
              backgroundColor: primarySwatch,
              elevation: _TAKE_CONTROL_ELEVATION,
              tooltip: _TAKE_CONTROL_TOOLTIP,
            )
          ],
        ),
      ),
    );
  }

  //color filter
  static const String _BACKGROUND_IMAGE_FILENAME = 'assets/mirror44.jpg';
  static const BoxFit _BACKGROUND_BOX_FIT = BoxFit.cover;
  static Color _backgroundColor = Colors.black.withOpacity(0.7);
  static const BlendMode _BACKGROUND_BLEND_MODE = BlendMode.darken;
  static ColorFilter _backgroundColorFilter = ColorFilter.mode(
    _backgroundColor,
    _BACKGROUND_BLEND_MODE,
  );

  //background image
  static DecorationImage _backgroundDecorationImage = DecorationImage(
    image: AssetImage(_BACKGROUND_IMAGE_FILENAME),
    fit: _BACKGROUND_BOX_FIT,
    colorFilter: _backgroundColorFilter,
  );

  static const _DEFAULT_PADDING = const EdgeInsets.all(8.0);

  static const double _TAKE_CONTROL_ELEVATION = 4.0;
  static const IconData _TAKE_CONTROL_ICON = CupertinoIcons.gamecontroller;
  static const String _TAKE_CONTROL_LABEL = "Take Control";
  static const String _TAKE_CONTROL_TOOLTIP = "Take Control of STS Controller";
}
