import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

import 'package:controller/common/colors.dart';
import 'package:controller/widgets/minimal_expanding_spacer.dart';
import 'package:controller/widgets/controller_floating_action_button.dart';

class ApplicationBar extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return AppBar(
      elevation: _APP_BAR_ELEVATION,
      title: Row(
        children: [
          Text(_TITLE),
          Image.asset(
            _LOGO_ASSET_FILENAME,
            fit: _LOGO_FIT,
            height: _LOGO_HEIGHT,
            width: _LOGO_WIDTH,
          ),
          MinimalExpandingSpacer(),
          ControllerFloatingActionButton(
            onPressed: () => null,
            label: _START_LABEL,
            iconData: _START_ICON,
            backgroundColor: _floatingActionButtonBackgroundColor,
            tooltip: _START_TOOLTIP,
          ),
          _appBarSpacer(),
          ControllerFloatingActionButton(
            onPressed: () => null,
            label: _STOP_LABEL,
            iconData: _STOP_ICON,
            backgroundColor: _floatingActionButtonBackgroundColor,
            tooltip: _STOP_TOOLTIP,
          ),
          SizedBox(
            width: _VERTICAL_DIVIDER_BOX_WIDTH,
            height: AppBar().preferredSize.height *
                _VERTICAL_DIVIDER_HEIGHT_PERCENTAGE,
            child: VerticalDivider(
              color: Colors.black.withOpacity(_VERTICAL_DIVIDER_OPACITY),
              width: _VERTICAL_DIVIDER_WIDTH,
              thickness: _VERTICAL_DIVIDER_THICKNESS,
            ),
          ),
          ControllerFloatingActionButton(
            onPressed: () => null,
            iconData: _PAUSE_ICON,
            backgroundColor: _floatingActionButtonBackgroundColor,
            tooltip: _PAUSE_TOOLTIP,
          ),
          _appBarSpacer(),
          ControllerFloatingActionButton(
            onPressed: () => null,
            iconData: _PLAY_ICON,
            backgroundColor: _floatingActionButtonBackgroundColor,
            tooltip: _PLAY_TOOLTIP,
          ),
        ],
      ),
    );
  }

  static SizedBox _appBarSpacer() {
    return SizedBox(width: _APP_BAR_SPACER_WIDTH);
  }

  //logo
  static const String _LOGO_ASSET_FILENAME = "assets/honeycomb.png";
  static const BoxFit _LOGO_FIT = BoxFit.scaleDown;
  static const double _LOGO_HEIGHT = 35;
  static const double _LOGO_WIDTH = 35;

  //start
  static const IconData _START_ICON = CupertinoIcons.infinite;
  static const String _START_LABEL = "Start";
  static const String _START_TOOLTIP = "Starts Simulation";

  //stop
  static const IconData _STOP_ICON = Icons.stop;
  static const String _STOP_LABEL = "Stop";
  static const String _STOP_TOOLTIP = "Stops Simulation";

  //pause
  static const IconData _PAUSE_ICON = Icons.pause;
  static const String _PAUSE_TOOLTIP = "Pauses Simics";

  //play
  static const IconData _PLAY_ICON = Icons.play_arrow;
  static const String _PLAY_TOOLTIP = "Starts Simics";

  //divider
  static const double _VERTICAL_DIVIDER_BOX_WIDTH = 20.0;
  static const double _VERTICAL_DIVIDER_OPACITY = 0.2;
  static const double _VERTICAL_DIVIDER_THICKNESS = 2.0;
  static const double _VERTICAL_DIVIDER_HEIGHT_PERCENTAGE = .70;
  static const double _VERTICAL_DIVIDER_WIDTH = 7.0;

  //app bar
  static const double _APP_BAR_ELEVATION = 10.0;
  static const double _APP_BAR_SPACER_WIDTH = 2.0;

  static Color _floatingActionButtonBackgroundColor = scaffoldBackgroundColor;
  static const String _TITLE = "STS Controller";
}
