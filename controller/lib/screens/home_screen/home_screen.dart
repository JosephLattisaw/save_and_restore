import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';

import 'package:controller/screens/home_screen/application_bar.dart';
import 'package:controller/screens/home_screen/panels/right_panel.dart';
import 'package:controller/screens/home_screen/panels/left_panel.dart';
import 'package:controller/screens/home_screen/panels/tab_panel.dart';
import 'package:provider/provider.dart';

class HomeScreen extends StatelessWidget {
  HomeScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: ApplicationBar.appBar(),
      body: Container(
        child: Row(
          children: [
            Expanded(
              flex: _LEFT_AND_MIDDLE_PANEL_FLEX,
              child: Column(
                children: [
                  Expanded(
                    child: Row(
                      children: [
                        Expanded(child: LeftPanel()),
                        Expanded(
                          flex: _TAB_PANEL_FLEX,
                          child: TabPanel(),
                        ),
                      ],
                    ),
                  ),
                ],
              ),
            ),
            Expanded(child: RightPanel())
          ],
        ),
      ),
    );
  }

  static const int _LEFT_AND_MIDDLE_PANEL_FLEX = 8;
  static const int _TAB_PANEL_FLEX = 6;
}
